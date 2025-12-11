-- catVIM Autocomplete
local colors = require("ui.colors")
local Syntax = require("editor.syntax")

local M = {}
M.__index = M

function M:new()
    local self = setmetatable({}, M)
    self.visible = false
    self.candidates = {}
    self.selection = 1
    self.base_x = 0
    self.base_y = 0
    self.prefix = ""
    return self
end

function M:gather_candidates(buffer, prefix)
    if #prefix < 2 then return {} end
    
    local candidates = {}
    local seen = {}
    
    -- 1. Language keywords
    local lang = Syntax.languages[buffer.filetype]
    if lang and lang.keywords then
        for _, kw in ipairs(lang.keywords) do
            if kw:sub(1, #prefix) == prefix then
                table.insert(candidates, { text = kw, type = "keyword" })
                seen[kw] = true
            end
        end
    end
    
    -- 2. Words in buffer (simple scan)
    -- Scan nearby lines first for performance? Or just full scan.
    -- For now, scan visible lines or whole buffer (if small)
    -- Let's limit to nearby 100 lines
    local start_line = math.max(1, #buffer.lines - 500) -- limit scan
    for i = 1, #buffer.lines do
        local line = buffer.lines[i]
        for word in line:gmatch("[%w_]+") do
            if #word > #prefix and word:sub(1, #prefix) == prefix and not seen[word] then
                table.insert(candidates, { text = word, type = "text" })
                seen[word] = true
            end
        end
    end
    
    table.sort(candidates, function(a, b) return a.text < b.text end)
    return candidates
end

function M:trigger(state)
    local line = state.buffer:get_line(state.cursor.line)
    local col = state.cursor.col
    
    -- Find word prefix before cursor
    local prefix_start = col
    while prefix_start > 1 do
        local c = line:sub(prefix_start - 1, prefix_start - 1)
        if not c:match("[%w_]") then break end
        prefix_start = prefix_start - 1
    end
    
    local prefix = line:sub(prefix_start, col - 1)
    
    self.candidates = self:gather_candidates(state.buffer, prefix)
    
    if #self.candidates > 0 then
        self.visible = true
        self.selection = 1
        self.prefix = prefix
        self.base_x = prefix_start  -- This is relative to line, need screen coords
        self.base_y = state.cursor.line
    else
        self.visible = false
    end
end

function M:hide()
    self.visible = false
    self.candidates = {}
end

function M:accept(state)
    if not self.visible or #self.candidates == 0 then return false end
    
    local selected = self.candidates[self.selection]
    local suffix = selected.text:sub(#self.prefix + 1)
    
    -- Insert suffix
    for i = 1, #suffix do
        state.buffer:insert_char(state.cursor.line, state.cursor.col, suffix:sub(i, i))
        state.cursor:move(1, 0)
    end
    
    self:hide()
    return true
end

function M:cycle(direction)
    self.selection = self.selection + direction
    if self.selection > #self.candidates then self.selection = 1 end
    if self.selection < 1 then self.selection = #self.candidates end
end

function M:render(state)
    if not self.visible then return end
    
    -- Calculate screen position
    -- base_y is buffer line, need buffer coords -> screen coords
    local screen_y = self.base_y - state.scroll_y + 1
    
    -- Check if cursor moved away from trigger point (simple check)
    if state.cursor.line ~= self.base_y then
        self:hide()
        return
    end
    
    -- Calculate x position (gutter offset)
    local gutter_width = state.show_line_numbers and 4 or 0
    local screen_x = self.base_x + gutter_width
    
    -- Don't render if off screen
    if screen_y < 1 or screen_y > state.height then return end
    
    -- Draw popup
    local max_width = 20
    for _, c in ipairs(self.candidates) do
        max_width = math.max(max_width, #c.text + 4)
    end
    local h = math.min(#self.candidates, 10)
    
    -- Draw box background
    local style = colors.styles.popup or { fg = colors.colors.fg, bg = colors.colors.bg_light }
    local sel_style = colors.styles.popup_selected or { fg = colors.colors.bg, bg = colors.colors.blue }
    
    -- Ensure popup doesn't go off screen bottom
    local render_y = screen_y + 1
    if render_y + h > state.height then
        render_y = screen_y - h  -- Flip up
    end
    
    -- Store render bounds for click detection
    self.rect = {
        x = screen_x,
        y = render_y,
        w = max_width,
        h = h
    }
    
    for i = 1, h do
        local idx = i -- scroll offset logic later if needed
        if idx <= #self.candidates then
            local cand = self.candidates[idx]
            local row_style = idx == self.selection and sel_style or style
            
            -- Prepare text with padding
            local text = string.format(" %-15s %s ", cand.text, cand.type:sub(1,1))
            text = text .. string.rep(" ", max_width - #text)
            
            catvim.render.string(screen_x, render_y + i - 1, text, row_style)
        end
    end
end

function M:handle_click(x, y, action, state)
    if not self.visible or not self.rect then return false end
    
    -- Check bounds
    if x >= self.rect.x and x < self.rect.x + self.rect.w and
       y >= self.rect.y and y < self.rect.y + self.rect.h then
        
        -- Calculate index relative to scroll (though we don't scroll yet)
        local rel_y = y - self.rect.y
        local idx = rel_y + 1
        
        if idx >= 1 and idx <= #self.candidates then
            self.selection = idx
            if action == "press" then
                -- Just select on press
                return true
            elseif action == "release" then
                 -- Accept on release (or double click logic if needed, but release is fine)
                 -- Actually, standard behavior is usually click selects. Let's accept on click.
                 self:accept(state)
                 return true
            end
        end
        return true
    end
    
    return false
end

return M
