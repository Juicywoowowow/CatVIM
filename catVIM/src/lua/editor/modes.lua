-- catVIM Modes - Modal editing state machine
local M = {}

M.current = "normal"
M.handlers = {}
M.on_change = nil  -- Callback when mode changes

-- Search state
M.search = {
    pattern = "",
    matches = {},
    current_match = 0,
    direction = 1  -- 1 = forward, -1 = backward
}

-- Clipboard for yank/paste
M.clipboard = {
    text = {},      -- Lines of text
    is_line = true  -- true = whole line(s), false = partial
}

-- Key constants
local KEY = {
    ESCAPE = 27,
    ENTER = 13,
    TAB = 9,
    BACKSPACE = 127,
    CTRL_C = 3,
    CTRL_S = 19,
    CTRL_Q = 17,
    CTRL_W = 23,
    CTRL_R = 18,
}

function M.register(name, handler)
    M.handlers[name] = handler
end

function M.switch(new_mode)
    if M.handlers[M.current] and M.handlers[M.current].on_exit then
        M.handlers[M.current]:on_exit()
    end
    
    local old_mode = M.current
    M.current = new_mode
    
    if M.handlers[new_mode] and M.handlers[new_mode].on_enter then
        M.handlers[new_mode]:on_enter()
    end
    
    if M.on_change then
        M.on_change(new_mode, old_mode)
    end
end

function M.handle(event, state)
    local handler = M.handlers[M.current]
    if handler and handler.handle then
        return handler:handle(event, state)
    end
    return false
end

-- Normal mode handler
local Normal = {}
Normal.__index = Normal

function Normal:new()
    return setmetatable({pending = nil}, Normal)
end

function Normal:on_enter()
    self.pending = nil
end

function Normal:on_exit()
    self.pending = nil
end

function Normal:handle(event, state)
    if event.type ~= "key" then return false end
    
    local key = event.key
    local char = event.char
    local ctrl = event.ctrl
    
    -- Check for count prefix
    if char and char:match("%d") and (self.pending or char ~= "0") then
        self.pending = (self.pending or "") .. char
        return true
    end
    
    local count = tonumber(self.pending) or 1
    self.pending = nil
    
    -- Mode switches
    if char == "i" then
        M.switch("insert")
        return true
    elseif char == "I" then
        state.cursor:first_non_blank()
        M.switch("insert")
        return true
    elseif char == "a" then
        state.cursor:move(1, 0)
        M.switch("insert")
        return true
    elseif char == "A" then
        state.cursor:line_end()
        M.switch("insert")
        return true
    elseif char == "o" then
        local line = state.cursor.line
        state.buffer:insert_line(line + 1, "")
        state.cursor:move_to(line + 1, 1)
        M.switch("insert")
        return true
    elseif char == "O" then
        local line = state.cursor.line
        state.buffer:insert_line(line, "")
        state.cursor:move_to(line, 1)
        M.switch("insert")
        return true
    elseif char == "v" then
        M.switch("visual")
        return true
    elseif char == ":" then
        M.switch("command")
        return true
    end
    
    -- Movement
    if char == "h" or key == 260 then  -- Left
        for _ = 1, count do state.cursor:move(-1, 0) end
        return true
    elseif char == "l" or key == 261 then  -- Right
        for _ = 1, count do state.cursor:move(1, 0) end
        return true
    elseif char == "j" or key == 258 then  -- Down
        for _ = 1, count do state.cursor:move(0, 1) end
        return true
    elseif char == "k" or key == 259 then  -- Up
        for _ = 1, count do state.cursor:move(0, -1) end
        return true
    elseif char == "w" then
        for _ = 1, count do state.cursor:word_forward() end
        return true
    elseif char == "b" then
        for _ = 1, count do state.cursor:word_backward() end
        return true
    elseif char == "0" then
        state.cursor:line_start()
        return true
    elseif char == "$" then
        state.cursor:line_end()
        return true
    elseif char == "^" then
        state.cursor:first_non_blank()
        return true
    elseif char == "G" then
        if count > 1 then
            state.cursor:goto_line(count)
        else
            state.cursor:file_end()
        end
        return true
    end
    
    -- g commands
    if char == "g" then
        self.pending = "g"
        return true
    end
    if self.pending == "g" then
        self.pending = nil
        if char == "g" then
            state.cursor:file_start()
            return true
        end
    end
    
    -- Editing
    if char == "x" then
        state.buffer:save_state()  -- Save for undo
        local line = state.cursor.line
        local col = state.cursor.col
        local text = state.buffer:get_line(line)
        if col <= #text then
            state.buffer:set_line(line, text:sub(1, col - 1) .. text:sub(col + 1))
        end
        return true
    elseif char == "d" then
        self.pending = "d"
        return true
    elseif char == "y" then
        self.pending = "y"
        return true
    end
    
    -- dd - delete line (and yank it)
    if self.pending == "d" then
        self.pending = nil
        if char == "d" then
            state.buffer:save_state()
            -- Yank line before deleting
            M.clipboard.text = { state.buffer:get_line(state.cursor.line) }
            M.clipboard.is_line = true
            state.buffer:delete_line(state.cursor.line)
            state.cursor:clamp()
            state:show_message("1 line deleted", "info")
            return true
        end
    end
    
    -- yy - yank line
    if self.pending == "y" then
        self.pending = nil
        if char == "y" then
            M.clipboard.text = { state.buffer:get_line(state.cursor.line) }
            M.clipboard.is_line = true
            state:show_message("1 line yanked", "info")
            return true
        end
    end
    
    -- p - paste after
    if char == "p" then
        if #M.clipboard.text == 0 then
            state:show_message("Nothing to paste", "warning")
            return true
        end
        state.buffer:save_state()
        if M.clipboard.is_line then
            -- Paste line(s) below current line
            for i, line in ipairs(M.clipboard.text) do
                state.buffer:insert_line(state.cursor.line + i, line)
            end
            state.cursor:move(0, 1)
            state:show_message(#M.clipboard.text .. " line(s) pasted", "info")
        else
            -- Paste text after cursor
            local line = state.buffer:get_line(state.cursor.line)
            local col = state.cursor.col
            local new_line = line:sub(1, col) .. M.clipboard.text[1] .. line:sub(col + 1)
            state.buffer:set_line(state.cursor.line, new_line)
            state.cursor:move(#M.clipboard.text[1], 0)
        end
        return true
    end
    
    -- P - paste before
    if char == "P" then
        if #M.clipboard.text == 0 then
            state:show_message("Nothing to paste", "warning")
            return true
        end
        state.buffer:save_state()
        if M.clipboard.is_line then
            -- Paste line(s) above current line
            for i, line in ipairs(M.clipboard.text) do
                state.buffer:insert_line(state.cursor.line + i - 1, line)
            end
            state:show_message(#M.clipboard.text .. " line(s) pasted", "info")
        else
            -- Paste text before cursor
            local line = state.buffer:get_line(state.cursor.line)
            local col = state.cursor.col
            local new_line = line:sub(1, col - 1) .. M.clipboard.text[1] .. line:sub(col)
            state.buffer:set_line(state.cursor.line, new_line)
        end
        return true
    end
    
    -- Undo/Redo
    if char == "u" then
        if state.buffer:undo() then
            state.cursor:clamp()
            state:show_message("Undo", "info")
        else
            state:show_message("Already at oldest change", "warning")
        end
        return true
    end
    
    if ctrl and (key == KEY.CTRL_R or char == "r") then
        if state.buffer:redo() then
            state.cursor:clamp()
            state:show_message("Redo", "info")
        else
            state:show_message("Already at newest change", "warning")
        end
        return true
    end
    
    -- Search
    if char == "/" then
        M.search.direction = 1
        M.switch("search")
        return true
    elseif char == "?" then
        M.search.direction = -1
        M.switch("search")
        return true
    end
    
    -- Next/prev search match
    if char == "n" then
        M.find_next(state, M.search.direction)
        return true
    elseif char == "N" then
        M.find_next(state, -M.search.direction)
        return true
    end
    
    -- Save/Quit shortcuts (Ctrl)
    if ctrl then
        if key == 19 or char == "s" then  -- Ctrl+S
            state:save()
            return true
        elseif key == 17 or char == "q" then  -- Ctrl+Q
            catvim.quit()
            return true
        end
    end
    
    return false
end

-- Insert mode handler
local Insert = {}
Insert.__index = Insert

function Insert:new()
    return setmetatable({has_edited = false}, Insert)
end

function Insert:on_enter()
    self.has_edited = false
end

function Insert:on_exit()
    -- Autocomplete is hidden in handle() before switch
end

function Insert:save_for_undo(state)
    if not self.has_edited then
        state.buffer:save_state()
        self.has_edited = true
    end
end

function Insert:handle(event, state)
    if event.type ~= "key" then return false end
    
    local key = event.key
    local char = event.char
    local ctrl = event.ctrl
    local ac = state.autocomplete
    
    -- Autocomplete navigation
    if ac and ac.visible then
        if key == 258 then -- Down
            ac:cycle(1)
            return true
        elseif key == 259 then -- Up
            ac:cycle(-1)
            return true
        end
    end
    
    -- Escape to normal mode
    if key == KEY.ESCAPE or (ctrl and (key == 3 or char == "c")) then
        if ac then ac:hide() end
        state.cursor:move(-1, 0)
        M.switch("normal")
        return true
    end
    
    -- Tab (Indent or Autocomplete Accept)
    if key == KEY.TAB then
        if ac and ac.visible then
            self:save_for_undo(state)
            ac:accept(state)
            return true
        end
        
        self:save_for_undo(state)
        for _ = 1, 4 do
            state.buffer:insert_char(state.cursor.line, state.cursor.col, " ")
            state.cursor:move(1, 0)
        end
        return true
    end
    
    -- Backspace
    if key == KEY.BACKSPACE then
        self:save_for_undo(state)
        local ok, new_col = state.buffer:delete_char(state.cursor.line, state.cursor.col)
        if ok then
            if new_col then
                state.cursor:move_to(state.cursor.line - 1, new_col)
            else
                state.cursor:move(-1, 0)
            end
        end
        if ac then ac:trigger(state) end
        return true
    end
    
    -- Enter
    if key == KEY.ENTER then
        if ac then ac:hide() end
        self:save_for_undo(state)
        state.buffer:split_line(state.cursor.line, state.cursor.col)
        state.cursor:move_to(state.cursor.line + 1, 1)
        return true
    end
    
    -- Arrows in insert mode (hide AC on side movement)
    if key == 260 then 
        if ac then ac:hide() end
        state.cursor:move(-1, 0) 
        return true 
    end
    if key == 261 then 
        if ac then ac:hide() end
        state.cursor:move(1, 0) 
        return true 
    end
    if key == 258 then 
        state.cursor:move(0, 1) 
        if ac then ac:hide() end
        return true 
    end
    if key == 259 then 
        state.cursor:move(0, -1) 
        if ac then ac:hide() end
        return true 
    end
    
    -- Printable characters
    if char and #char == 1 and key >= 32 and key < 127 then
        self:save_for_undo(state)
        state.buffer:insert_char(state.cursor.line, state.cursor.col, char)
        state.cursor:move(1, 0)
        
        if ac and char:match("[%w_]") then
            ac:trigger(state)
        else
            if ac then ac:hide() end
        end
        return true
    end
    
    return false
end

-- Visual mode handler (basic)
local Visual = {}
Visual.__index = Visual

function Visual:new()
    return setmetatable({start_line = 1, start_col = 1}, Visual)
end

function Visual:on_enter()
    -- Remember where selection started
    -- This will be set by editor state
end

function Visual:on_exit() end

function Visual:handle(event, state)
    if event.type ~= "key" then return false end
    
    local key = event.key
    local char = event.char
    
    if key == 27 or char == "v" then -- Escape or v to exit
        M.switch("normal")
        return true
    end
    
    -- Movement (same as normal mode)
    if char == "h" then state.cursor:move(-1, 0) return true end
    if char == "l" then state.cursor:move(1, 0) return true end
    if char == "j" then state.cursor:move(0, 1) return true end
    if char == "k" then state.cursor:move(0, -1) return true end
    
    return false
end

-- Command mode handler
local Command = {}
Command.__index = Command

function Command:new()
    return setmetatable({input = ""}, Command)
end

function Command:on_enter()
    self.input = ""
end

function Command:on_exit()
    self.input = ""
end

function Command:handle(event, state)
    if event.type ~= "key" then return false end
    
    local key = event.key
    local char = event.char
    
    if key == KEY.ESCAPE then
        M.switch("normal")
        return true
    end
    
    if key == KEY.ENTER then
        self:execute(state)
        M.switch("normal")
        return true
    end
    
    if key == KEY.BACKSPACE then
        if #self.input > 0 then
            self.input = self.input:sub(1, -2)
        else
            M.switch("normal")
        end
        return true
    end
    
    if char and #char == 1 and key >= 32 and key < 127 then
        self.input = self.input .. char
        return true
    end
    
    return false
end

function Command:execute(state)
    local cmd = self.input:match("^%s*(.-)%s*$")  -- Trim
    
    if cmd == "w" or cmd == "write" then
        state:save()
    elseif cmd == "q" or cmd == "quit" then
        if state.buffer.modified then
            state:show_message("Unsaved changes! Use :q! to force quit", "error")
        else
            catvim.quit()
        end
    elseif cmd == "q!" then
        catvim.quit()
    elseif cmd == "wq" or cmd == "x" then
        state:save()
        catvim.quit()
    elseif cmd == "e" or cmd == "edit" then
        if state.buffer.filepath then
            state:open_file(state.buffer.filepath)
        else
            state:show_message("No file to reload", "warning")
        end
    elseif cmd:match("^e%s+") or cmd:match("^edit%s+") then
        local path = cmd:match("^e%s+(.+)$")
        if not path then path = cmd:match("^edit%s+(.+)$") end
        state:open_file(path)
    elseif cmd:match("^%d+$") then
        state.cursor:goto_line(tonumber(cmd))
    else
        state:show_message("Unknown command: " .. cmd, "error")
    end
end

-- Search mode handler
local Search = {}
Search.__index = Search

function Search:new()
    return setmetatable({input = ""}, Search)
end

function Search:on_enter()
    self.input = ""
end

function Search:on_exit()
    -- Keep pattern for n/N navigation
end

function Search:handle(event, state)
    if event.type ~= "key" then return false end
    
    local key = event.key
    local char = event.char
    
    if key == KEY.ESCAPE then
        M.search.pattern = ""
        M.switch("normal")
        return true
    end
    
    if key == KEY.ENTER then
        M.search.pattern = self.input
        M.do_search(state)
        M.switch("normal")
        return true
    end
    
    if key == KEY.BACKSPACE then
        if #self.input > 0 then
            self.input = self.input:sub(1, -2)
        else
            M.switch("normal")
        end
        return true
    end
    
    if char and #char == 1 and key >= 32 and key < 127 then
        self.input = self.input .. char
        return true
    end
    
    return false
end

-- Search functions
function M.do_search(state)
    local pattern = M.search.pattern
    if pattern == "" then return end
    
    M.search.matches = {}
    
    -- Find all matches
    for line_num = 1, state.buffer:line_count() do
        local line = state.buffer:get_line(line_num)
        local start = 1
        while true do
            local s, e = line:find(pattern, start, true)  -- Plain text search
            if s then
                table.insert(M.search.matches, {line = line_num, col = s, end_col = e})
                start = e + 1
            else
                break
            end
        end
    end
    
    if #M.search.matches == 0 then
        state:show_message("Pattern not found: " .. pattern, "warning")
        return
    end
    
    -- Find first match after cursor
    local cursor_line = state.cursor.line
    local cursor_col = state.cursor.col
    
    for i, match in ipairs(M.search.matches) do
        if match.line > cursor_line or (match.line == cursor_line and match.col > cursor_col) then
            M.search.current_match = i
            state.cursor:move_to(match.line, match.col)
            state:show_message("/" .. pattern .. " [" .. i .. "/" .. #M.search.matches .. "]", "info")
            return
        end
    end
    
    -- Wrap to first match
    M.search.current_match = 1
    local match = M.search.matches[1]
    state.cursor:move_to(match.line, match.col)
    state:show_message("/" .. pattern .. " [1/" .. #M.search.matches .. "] (wrapped)", "info")
end

function M.find_next(state, direction)
    if M.search.pattern == "" or #M.search.matches == 0 then
        if M.search.pattern == "" then
            state:show_message("No search pattern", "warning")
        else
            state:show_message("Pattern not found: " .. M.search.pattern, "warning")
        end
        return
    end
    
    local next_idx = M.search.current_match + direction
    if next_idx > #M.search.matches then
        next_idx = 1
    elseif next_idx < 1 then
        next_idx = #M.search.matches
    end
    
    M.search.current_match = next_idx
    local match = M.search.matches[next_idx]
    state.cursor:move_to(match.line, match.col)
    
    local wrap_msg = ""
    if (direction > 0 and next_idx == 1) or (direction < 0 and next_idx == #M.search.matches) then
        wrap_msg = " (wrapped)"
    end
    state:show_message("/" .. M.search.pattern .. " [" .. next_idx .. "/" .. #M.search.matches .. "]" .. wrap_msg, "info")
end

-- Register default modes
M.register("normal", Normal:new())
M.register("insert", Insert:new())
M.register("visual", Visual:new())
M.register("command", Command:new())
M.register("search", Search:new())

return M

