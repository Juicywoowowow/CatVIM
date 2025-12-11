-- catVIM Status Line - Bottom bar with mode, file info, and buttons
local colors = require("ui.colors")
local icons = require("ui.icons")
local Button = require("ui.button")

local StatusLine = {}
StatusLine.__index = StatusLine

function StatusLine:new(opts)
    local self = setmetatable({}, StatusLine)
    self.y = 1  -- Will be set to bottom of screen
    self.width = 80
    self.mode = "normal"
    self.filename = "[No Name]"
    self.modified = false
    self.line = 1
    self.col = 1
    self.total_lines = 1
    self.filetype = "text"
    self.message = nil
    self.message_type = "info"
    self.message_time = 0
    
    -- Create toolbar buttons
    self.btn_save = Button:new({
        text = " Save ",
        style = colors.styles.button,
        on_click = function() if self.on_save then self.on_save() end end
    })
    
    self.btn_open = Button:new({
        text = " Open ",
        style = colors.styles.button,
        on_click = function() if self.on_open then self.on_open() end end
    })
    
    self.btn_quit = Button:new({
        text = " Quit ",
        style = { fg = colors.colors.red, bg = colors.colors.btn_normal },
        style_hover = { fg = colors.colors.red, bg = colors.colors.btn_hover, bold = true },
        on_click = function() if self.on_quit then self.on_quit() end end
    })
    
    return self
end

function StatusLine:set_pos(y, width)
    self.y = y
    self.width = width
    
    -- Position toolbar buttons (right side of line above status)
    local btn_y = y - 1
    self.btn_quit:set_pos(width - 7, btn_y)
    self.btn_open:set_pos(width - 14, btn_y)
    self.btn_save:set_pos(width - 21, btn_y)
end

function StatusLine:update(state)
    self.mode = state.mode or "normal"
    self.filename = state.filename or "[No Name]"
    self.modified = state.modified or false
    self.line = state.line or 1
    self.col = state.col or 1
    self.total_lines = state.total_lines or 1
    self.filetype = state.filetype or "text"
end

function StatusLine:show_message(msg, msg_type)
    self.message = msg
    self.message_type = msg_type or "info"
    self.message_time = os.time()
end

function StatusLine:render()
    -- Check if message should expire (3 seconds)
    if self.message and os.time() - self.message_time > 3 then
        self.message = nil
    end
    
    -- Mode indicator
    local mode_text = " " .. self.mode:upper():sub(1, 1) .. " "
    local mode_colors = {
        normal = { fg = colors.colors.bg, bg = colors.colors.blue },
        insert = { fg = colors.colors.bg, bg = colors.colors.green },
        visual = { fg = colors.colors.bg, bg = colors.colors.purple },
        command = { fg = colors.colors.bg, bg = colors.colors.orange },
    }
    local mode_style = mode_colors[self.mode] or mode_colors.normal
    mode_style.bold = true
    
    catvim.render.string(1, self.y, mode_text, mode_style)
    
    -- Filename + modified indicator
    local name_part = " " .. self.filename
    if self.modified then
        name_part = name_part .. " " .. icons.modified
    end
    name_part = name_part .. " "
    
    local name_style = colors.styles.statusline
    if self.modified then
        name_style = { fg = colors.colors.yellow, bg = colors.colors.bg_dark }
    end
    catvim.render.string(#mode_text + 1, self.y, name_part, name_style)
    
    -- Message or spacer
    local left_len = #mode_text + #name_part
    
    if self.message then
        local msg_style = colors.styles[self.message_type] or colors.styles.statusline
        msg_style.bg = colors.colors.bg_dark
        local space = self.width - left_len - 20
        local msg = self.message:sub(1, space)
        catvim.render.string(left_len + 1, self.y, " " .. msg, msg_style)
        left_len = left_len + #msg + 1
    end
    
    -- Fill middle
    local right_part = self.filetype .. " | " .. self.line .. ":" .. self.col .. "/" .. self.total_lines .. " "
    local fill_len = self.width - left_len - #right_part
    if fill_len > 0 then
        catvim.render.string(left_len + 1, self.y, string.rep(" ", fill_len), colors.styles.statusline)
    end
    
    -- Right side info
    catvim.render.string(self.width - #right_part + 1, self.y, right_part, colors.styles.statusline)
    
    -- Render buttons (they're on line above)
    self.btn_save:render()
    self.btn_open:render()
    self.btn_quit:render()
end

return StatusLine
