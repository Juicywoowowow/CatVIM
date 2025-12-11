-- catVIM Modes - Modal editing state machine
local M = {}

M.current = "normal"
M.handlers = {}
M.on_change = nil  -- Callback when mode changes

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
    end
    
    if self.pending == "d" then
        self.pending = nil
        if char == "d" then
            state.buffer:delete_line(state.cursor.line)
            state.cursor:clamp()
            return true
        end
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
    return setmetatable({}, Insert)
end

function Insert:on_enter() end
function Insert:on_exit() end

function Insert:handle(event, state)
    if event.type ~= "key" then return false end
    
    local key = event.key
    local char = event.char
    local ctrl = event.ctrl
    
    -- Escape to normal mode
    if key == KEY.ESCAPE or (ctrl and (key == 3 or char == "c")) then
        state.cursor:move(-1, 0)
        M.switch("normal")
        return true
    end
    
    -- Backspace
    if key == KEY.BACKSPACE then
        local ok, new_col = state.buffer:delete_char(state.cursor.line, state.cursor.col)
        if ok then
            if new_col then
                state.cursor:move_to(state.cursor.line - 1, new_col)
            else
                state.cursor:move(-1, 0)
            end
        end
        return true
    end
    
    -- Enter
    if key == KEY.ENTER then
        state.buffer:split_line(state.cursor.line, state.cursor.col)
        state.cursor:move_to(state.cursor.line + 1, 1)
        return true
    end
    
    -- Tab
    if key == KEY.TAB then
        for _ = 1, 4 do
            state.buffer:insert_char(state.cursor.line, state.cursor.col, " ")
            state.cursor:move(1, 0)
        end
        return true
    end
    
    -- Arrows in insert mode
    if key == 260 then state.cursor:move(-1, 0) return true end
    if key == 261 then state.cursor:move(1, 0) return true end
    if key == 258 then state.cursor:move(0, 1) return true end
    if key == 259 then state.cursor:move(0, -1) return true end
    
    -- Printable characters
    if char and #char == 1 and key >= 32 and key < 127 then
        state.buffer:insert_char(state.cursor.line, state.cursor.col, char)
        state.cursor:move(1, 0)
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
    elseif cmd:match("^e%s+") then
        local path = cmd:match("^e%s+(.+)$")
        state:open_file(path)
    elseif cmd:match("^%d+$") then
        state.cursor:goto_line(tonumber(cmd))
    else
        state:show_message("Unknown command: " .. cmd, "error")
    end
end

-- Register default modes
M.register("normal", Normal:new())
M.register("insert", Insert:new())
M.register("visual", Visual:new())
M.register("command", Command:new())

return M
