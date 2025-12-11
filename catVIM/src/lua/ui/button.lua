-- catVIM Button Component - Clickable UI elements
local colors = require("ui.colors")

local Button = {}
Button.__index = Button

-- Registry of all buttons for hit testing
local buttons = {}

function Button.clear_all()
    buttons = {}
end

function Button.hit_test(x, y)
    for _, btn in ipairs(buttons) do
        if btn.visible and btn:contains(x, y) then
            return btn
        end
    end
    return nil
end

function Button:new(opts)
    local self = setmetatable({}, Button)
    self.x = opts.x or 1
    self.y = opts.y or 1
    self.text = opts.text or ""
    self.width = opts.width or (#self.text + 2)
    self.height = opts.height or 1
    self.on_click = opts.on_click or function() end
    self.style = opts.style or colors.styles.button
    self.style_hover = opts.style_hover or colors.styles.button_hover
    self.style_active = opts.style_active or colors.styles.button_active
    self.visible = true
    self.hovered = false
    self.pressed = false
    self.id = opts.id or ("btn_" .. tostring(#buttons + 1))
    
    table.insert(buttons, self)
    return self
end

function Button:contains(x, y)
    return x >= self.x and x < self.x + self.width
       and y >= self.y and y < self.y + self.height
end

function Button:set_pos(x, y)
    self.x = x
    self.y = y
end

function Button:render()
    if not self.visible then return end
    
    local style = self.style
    if self.pressed then
        style = self.style_active
    elseif self.hovered then
        style = self.style_hover
    end
    
    -- Render button background
    local padding = math.floor((self.width - #self.text) / 2)
    local text = string.rep(" ", padding) .. self.text .. string.rep(" ", self.width - padding - #self.text)
    
    catvim.render.string(self.x, self.y, text, style)
end

function Button:on_mouse(event)
    if not self.visible then return false end
    
    local inside = self:contains(event.x, event.y)
    
    if event.action == "press" and inside then
        self.pressed = true
        return true
    elseif event.action == "release" then
        if self.pressed and inside then
            self.on_click()
        end
        self.pressed = false
        return inside
    elseif event.action == "drag" then
        self.hovered = inside
        return inside
    end
    
    self.hovered = inside
    return false
end

function Button:destroy()
    for i, btn in ipairs(buttons) do
        if btn == self then
            table.remove(buttons, i)
            break
        end
    end
end

-- Static methods for handling all buttons
function Button.handle_mouse(event)
    local handled = false
    for _, btn in ipairs(buttons) do
        if btn:on_mouse(event) then
            handled = true
        end
    end
    return handled
end

function Button.update_hover(x, y)
    for _, btn in ipairs(buttons) do
        btn.hovered = btn.visible and btn:contains(x, y)
    end
end

function Button.render_all()
    for _, btn in ipairs(buttons) do
        btn:render()
    end
end

return Button
