-- catVIM Cursor - Cursor position and movement
local Cursor = {}
Cursor.__index = Cursor

function Cursor:new(buffer)
    local self = setmetatable({}, Cursor)
    self.line = 1
    self.col = 1
    self.target_col = 1  -- For vertical movement memory
    self.buffer = buffer
    return self
end

function Cursor:set_buffer(buffer)
    self.buffer = buffer
    self:clamp()
end

function Cursor:clamp()
    if not self.buffer then return end
    
    -- Clamp line
    local max_line = self.buffer:line_count()
    self.line = math.max(1, math.min(self.line, max_line))
    
    -- Clamp column
    local line_len = #self.buffer:get_line(self.line)
    self.col = math.max(1, math.min(self.col, line_len + 1))
end

function Cursor:move(dx, dy)
    self.line = self.line + dy
    self.col = self.col + dx
    
    if dx ~= 0 then
        self.target_col = self.col
    end
    
    self:clamp()
    
    -- Restore target column on vertical movement
    if dy ~= 0 and dx == 0 then
        local line_len = #self.buffer:get_line(self.line)
        self.col = math.min(self.target_col, line_len + 1)
    end
end

function Cursor:move_to(line, col)
    self.line = line
    self.col = col
    self.target_col = col
    self:clamp()
end

function Cursor:line_start()
    self.col = 1
    self.target_col = 1
end

function Cursor:line_end()
    local line_len = #self.buffer:get_line(self.line)
    self.col = line_len + 1
    self.target_col = self.col
end

function Cursor:first_non_blank()
    local line = self.buffer:get_line(self.line)
    local pos = line:find("%S") or 1
    self.col = pos
    self.target_col = pos
end

function Cursor:word_forward()
    local line = self.buffer:get_line(self.line)
    local rest = line:sub(self.col)
    
    -- Find next word start
    local _, word_end = rest:find("^%s*%w+")
    if word_end then
        self.col = self.col + word_end
    elseif self.line < self.buffer:line_count() then
        self.line = self.line + 1
        self.col = 1
        self:first_non_blank()
    else
        self:line_end()
    end
    self.target_col = self.col
end

function Cursor:word_backward()
    if self.col <= 1 then
        if self.line > 1 then
            self.line = self.line - 1
            self:line_end()
        end
        return
    end
    
    local line = self.buffer:get_line(self.line)
    local before = line:sub(1, self.col - 1)
    
    -- Find previous word start
    local word_start = before:match(".*()%w+%s*$") or before:match(".*()%S+%s*$")
    self.col = word_start or 1
    self.target_col = self.col
end

function Cursor:goto_line(n)
    self.line = n
    self:clamp()
    self:first_non_blank()
end

function Cursor:file_start()
    self.line = 1
    self.col = 1
    self.target_col = 1
end

function Cursor:file_end()
    self.line = self.buffer:line_count()
    self:first_non_blank()
end

return Cursor
