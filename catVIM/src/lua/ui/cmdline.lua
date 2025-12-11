-- catVIM Command Line - Bottom input for : commands
local colors = require("ui.colors")

local Cmdline = {}
Cmdline.__index = Cmdline

function Cmdline:new()
    local self = setmetatable({}, Cmdline)
    self.y = 1
    self.width = 80
    self.prefix = ":"
    self.input = ""
    self.visible = false
    return self
end

function Cmdline:set_pos(y, width)
    self.y = y
    self.width = width
end

function Cmdline:show(prefix)
    self.prefix = prefix or ":"
    self.input = ""
    self.visible = true
end

function Cmdline:hide()
    self.visible = false
    self.input = ""
end

function Cmdline:set_input(text)
    self.input = text
end

function Cmdline:render()
    if not self.visible then return end
    
    local style = { fg = colors.colors.fg, bg = colors.colors.bg }
    local line = self.prefix .. self.input .. "_"
    line = line .. string.rep(" ", self.width - #line)
    
    catvim.render.string(1, self.y, line, style)
end

return Cmdline
