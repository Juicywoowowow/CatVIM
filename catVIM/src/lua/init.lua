-- catVIM Main Entry Point
local Buffer = require("editor.buffer")
local Cursor = require("editor.cursor")
local Modes = require("editor.modes")
local Syntax = require("editor.syntax")
local colors = require("ui.colors")
local icons = require("ui.icons")
local Button = require("ui.button")
local StatusLine = require("ui.statusline")
local Explorer = require("ui.explorer")
local Cmdline = require("ui.cmdline")
local Autocomplete = require("editor.autocomplete") 

-- Global editor state
local State = {
    width = 80,
    height = 24,
    buffer = nil,
    cursor = nil,
    scroll_y = 0,
    explorer = nil,
    statusline = nil,
    cmdline = nil,
    show_line_numbers = true,
    gutter_width = 4,
    mouse_x = 0,
    mouse_y = 0,
}

-- Initialize state
function State:init()
    self.buffer = Buffer:new()
    self.cursor = Cursor:new(self.buffer)
    
    -- Get terminal size
    local size = catvim.term.size()
    self.width = size.width
    self.height = size.height
    
    -- Initialize UI components
    self.statusline = StatusLine:new()
    self.statusline.on_save = function() self:save() end
    self.statusline.on_open = function() 
        self.explorer:toggle()
    end
    self.statusline.on_quit = function() 
        if self.buffer.modified then
            self:show_message("Unsaved changes! Use :q! to quit", "error")
        else
            catvim.quit()
        end
    end
    
    self.explorer = Explorer:new({
        width = 25,
        cwd = ".",
        on_select = function(path)
            self:open_file(path)
        end
    })
    
    self.cmdline = Cmdline:new()
    self.autocomplete = Autocomplete:new()
    
    -- Set mode change callback
    Modes.on_change = function(new_mode, old_mode)
        if new_mode == "command" then
            self.cmdline:show(":")
        elseif new_mode == "search" then
            local prefix = Modes.search.direction == 1 and "/" or "?"
            self.cmdline:show(prefix)
        else
            self.cmdline:hide()
        end
    end
    
    self:resize()
end

function State:resize()
    local size = catvim.term.size()
    self.width = size.width
    self.height = size.height
    
    catvim.render.resize()
    
    -- Position UI elements
    local editor_x = 1
    if self.explorer.visible then
        self.explorer:set_bounds(1, 1, 25, self.height - 2)
        editor_x = 26
    end
    
    self.statusline:set_pos(self.height, self.width)
    self.cmdline:set_pos(self.height, self.width)
end

function State:open_file(path)
    local ok, err = self.buffer:load(path)
    if ok then
        self.cursor:set_buffer(self.buffer)
        self.cursor:file_start()
        self.scroll_y = 0
        self:show_message("Opened: " .. path, "info")
    else
        self:show_message("Error: " .. (err or "Unknown error"), "error")
    end
end

function State:save()
    if not self.buffer.filepath then
        self:show_message("No filename. Use :w <filename>", "warning")
        return
    end
    
    local ok, err = self.buffer:save()
    if ok then
        self:show_message("Saved: " .. self.buffer.filepath, "info")
    else
        self:show_message("Error saving: " .. (err or "Unknown"), "error")
    end
end

function State:show_message(msg, msg_type)
    self.statusline:show_message(msg, msg_type)
end

function State:editor_bounds()
    local x = 1 + self.gutter_width
    local y = 1
    local w = self.width - self.gutter_width
    local h = self.height - 2  -- Leave room for statusline and toolbar
    
    if self.explorer.visible then
        x = x + self.explorer.width
        w = w - self.explorer.width
    end
    
    return x, y, w, h
end

function State:render()
    catvim.render.clear()
    
    local editor_x, editor_y, editor_w, editor_h = self:editor_bounds()
    local gutter_x = editor_x - self.gutter_width
    
    -- Ensure cursor is visible
    if self.cursor.line < self.scroll_y + 1 then
        self.scroll_y = self.cursor.line - 1
    elseif self.cursor.line > self.scroll_y + editor_h then
        self.scroll_y = self.cursor.line - editor_h
    end
    
    -- Render editor lines
    for i = 1, editor_h do
        local line_num = self.scroll_y + i
        local y = editor_y + i - 1
        
        -- Gutter (line numbers)
        if self.show_line_numbers then
            local num_style = colors.styles.line_number
            if line_num == self.cursor.line then
                num_style = colors.styles.line_number_current
            end
            
            local num_str
            if line_num <= self.buffer:line_count() then
                num_str = string.format("%3d ", line_num)
            else
                num_str = "  ~ "
            end
            catvim.render.string(gutter_x, y, num_str, num_style)
        end
        
        -- Line content
        if line_num <= self.buffer:line_count() then
            local line = self.buffer:get_line(line_num)
            local base_style = colors.styles.normal
            
            -- Highlight cursor line background
            if line_num == self.cursor.line then
                base_style = colors.styles.cursor_line
            end
            
            -- Get syntax highlights for this line
            local highlights = Syntax.highlight_line(line, self.buffer.filetype)
            
            -- Render line character by character with syntax highlighting
            local display_line = line:sub(1, editor_w)
            for col = 1, #display_line do
                local char = display_line:sub(col, col)
                local char_style = Syntax.get_style(highlights, col)
                
                if char_style then
                    -- Merge syntax style with base style (keep bg from cursor line)
                    local merged = {
                        fg = char_style.fg or base_style.fg,
                        bg = base_style.bg,
                        bold = char_style.bold,
                        italic = char_style.italic
                    }
                    catvim.render.set(editor_x + col - 1, y, char, merged)
                else
                    catvim.render.set(editor_x + col - 1, y, char, base_style)
                end
            end
            
            -- Fill rest of line
            local fill = editor_w - #display_line
            if fill > 0 then
                catvim.render.string(editor_x + #display_line, y, string.rep(" ", fill), base_style)
            end
            
            -- Render cursor
            if line_num == self.cursor.line then
                local cursor_x = editor_x + self.cursor.col - 1
                local cursor_char = line:sub(self.cursor.col, self.cursor.col)
                if cursor_char == "" then cursor_char = " " end
                
                local cursor_style = { 
                    fg = colors.colors.bg, 
                    bg = colors.colors.cursor,
                    bold = true 
                }
                if Modes.current == "insert" then
                    cursor_style.bg = colors.colors.green
                end
                
                catvim.render.string(cursor_x, y, cursor_char, cursor_style)
            end
        else
            -- Empty line indicator
            catvim.render.string(editor_x, y, string.rep(" ", editor_w), colors.styles.normal)
        end
    end
    
    -- Render explorer
    self.explorer:render()
    
    -- Toolbar row (above status line)
    local toolbar_y = self.height - 1
    catvim.render.string(1, toolbar_y, string.rep(" ", self.width), { bg = colors.colors.bg_light })
    
    -- Toolbar hint text
    local hint = " Press <Space>e for explorer | <Space>f for files | :w to save | :q to quit "
    catvim.render.string(1, toolbar_y, hint, { fg = colors.colors.fg_dim, bg = colors.colors.bg_light })
    
    -- Render buttons
    Button.render_all()
    
    -- Update and render status line
    self.statusline:update({
        mode = Modes.current,
        filename = self.buffer.name,
        modified = self.buffer.modified,
        line = self.cursor.line,
        col = self.cursor.col,
        total_lines = self.buffer:line_count(),
        filetype = self.buffer.filetype,
    })
    self.statusline:render()
    
    -- Command/Search line
    if Modes.current == "command" then
        self.cmdline:set_input(Modes.handlers.command.input)
        self.cmdline:render()
    elseif Modes.current == "search" then
        self.cmdline:set_input(Modes.handlers.search.input)
        self.cmdline:render()
    end
    
    -- Render autocomplete popup (on top)
    if self.autocomplete then
        self.autocomplete:render(self)
    end
    
    catvim.render.flush()
end

function State:handle_event(event)
    if not event then return end
    
    -- Handle mouse events first
    if event.type == "mouse" then
        self.mouse_x = event.x
        self.mouse_y = event.y
        
        Button.update_hover(event.x, event.y)
        
        if Button.handle_mouse(event) then
            return
        end
        
        -- Autocomplete click
        if self.autocomplete and self.autocomplete:handle_click(event.x, event.y, event.action, self) then
            return
        end
        
        -- Explorer click
        if self.explorer:handle_click(event.x, event.y, event.action) then
            return
        end
        
        -- Editor click - move cursor
        if event.action == "press" and event.button == 0 then
            local editor_x, editor_y, editor_w, editor_h = self:editor_bounds()
            
            if event.x >= editor_x and event.x < editor_x + editor_w
               and event.y >= editor_y and event.y < editor_y + editor_h then
                local click_line = self.scroll_y + (event.y - editor_y + 1)
                local click_col = event.x - editor_x + 1
                
                click_line = math.min(click_line, self.buffer:line_count())
                self.cursor:move_to(click_line, click_col)
            end
        end
        
        -- Scroll wheel
        if event.action == "scroll" then
            if event.button == 64 then  -- Scroll up
                self.scroll_y = math.max(0, self.scroll_y - 3)
            elseif event.button == 65 then  -- Scroll down
                local max_scroll = math.max(0, self.buffer:line_count() - 10)
                self.scroll_y = math.min(max_scroll, self.scroll_y + 3)
            end
        end
        
        return
    end
    
    -- Handle keyboard events
    if event.type == "key" then
        -- Space-prefixed shortcuts (leader key)
        if event.char == " " and Modes.current == "normal" then
            -- Wait for next key
            -- For now, we'll handle immediate space commands
        end
        
        -- Explorer keyboard (if focused)
        if self.explorer.visible and self.explorer:handle_key(event) then
            return
        end
        
        -- Toggle explorer with Space+e in normal mode
        if Modes.current == "normal" and event.char == "e" and event.ctrl then
            self.explorer:toggle()
            self:resize()
            return
        end
        
        -- Mode handlers
        Modes.handle(event, self)
    end
end

-- Global functions called by C++
function init()
    State:init()
    
    -- Load file from command line if provided
    if arg and #arg > 0 then
        State:open_file(arg[1])
    else
        -- Default welcome message
        State.buffer.lines = {
            "",
            "  Welcome to catVIM!",
            "",
            "  A terminal IDE with Vim philosophy.",
            "",
            "  Quick Start:",
            "    i          Enter insert mode",
            "    Esc        Return to normal mode",
            "    :w         Save file",
            "    :q         Quit",
            "",
            "  Mouse Support:",
            "    Click      Move cursor",
            "    Scroll     Navigate document",
            "    Buttons    Use toolbar buttons",
            "",
            "  Navigation:",
            "    h j k l    Move cursor",
            "    w b        Word forward/backward", 
            "    gg G       Top/bottom of file",
            "",
            "  Open a file:",
            "    :e <path>  Open file",
            "    Ctrl+E     Toggle file explorer",
            "",
        }
        State.buffer.name = "[Welcome]"
    end
    
    State:render()
end

function update()
    local event = catvim.term.read()
    
    if event then
        State:handle_event(event)
        State:render()
    end
end
