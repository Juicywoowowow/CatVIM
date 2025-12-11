-- catVIM File Explorer - Tree view sidebar
local colors = require("ui.colors")
local icons = require("ui.icons")
local Button = require("ui.button")

local Explorer = {}
Explorer.__index = Explorer

function Explorer:new(opts)
    local self = setmetatable({}, Explorer)
    self.x = 1
    self.y = 1
    self.width = opts.width or 25
    self.height = 20
    self.visible = opts.visible or false
    self.entries = {}
    self.selected = 1
    self.scroll = 0
    self.cwd = opts.cwd or "."
    self.on_select = opts.on_select or function() end
    self.expanded = {}  -- Track expanded directories
    
    return self
end

function Explorer:toggle()
    self.visible = not self.visible
    if self.visible then
        self:refresh()
    end
end

function Explorer:set_bounds(x, y, width, height)
    self.x = x
    self.y = y
    self.width = width
    self.height = height
end

function Explorer:refresh()
    self.entries = {}
    self:load_dir(self.cwd, 0)
end

function Explorer:load_dir(path, depth)
    local items = catvim.fs.list(path)
    if not items then return end
    
    -- Sort: directories first, then alphabetically
    table.sort(items, function(a, b)
        if a.isdir ~= b.isdir then
            return a.isdir
        end
        return a.name < b.name
    end)
    
    for _, item in ipairs(items) do
        if item.name ~= ".." then
            local full_path = path .. "/" .. item.name
            table.insert(self.entries, {
                name = item.name,
                path = full_path,
                isdir = item.isdir,
                depth = depth,
                expanded = self.expanded[full_path] or false
            })
            
            -- Load subdirectory if expanded
            if item.isdir and self.expanded[full_path] then
                self:load_dir(full_path, depth + 1)
            end
        end
    end
end

function Explorer:select_entry(idx)
    if idx < 1 or idx > #self.entries then return end
    
    local entry = self.entries[idx]
    self.selected = idx
    
    if entry.isdir then
        -- Toggle expand
        self.expanded[entry.path] = not self.expanded[entry.path]
        self:refresh()
    else
        -- Open file
        self.on_select(entry.path)
    end
end

function Explorer:move_selection(delta)
    self.selected = math.max(1, math.min(#self.entries, self.selected + delta))
    
    -- Scroll if needed
    local visible_start = self.scroll + 1
    local visible_end = self.scroll + self.height - 2
    
    if self.selected < visible_start then
        self.scroll = self.selected - 1
    elseif self.selected > visible_end then
        self.scroll = self.selected - (self.height - 2)
    end
end

function Explorer:handle_click(x, y)
    if not self.visible then return false end
    if x < self.x or x >= self.x + self.width then return false end
    if y < self.y + 1 or y >= self.y + self.height - 1 then return false end
    
    local idx = (y - self.y - 1) + self.scroll + 1
    if idx >= 1 and idx <= #self.entries then
        self:select_entry(idx)
        return true
    end
    return false
end

function Explorer:handle_key(event)
    if not self.visible then return false end
    
    local char = event.char
    local key = event.key
    
    if char == "j" or key == 258 then
        self:move_selection(1)
        return true
    elseif char == "k" or key == 259 then
        self:move_selection(-1)
        return true
    elseif key == 13 or char == "l" then  -- Enter or l
        self:select_entry(self.selected)
        return true
    elseif char == "h" then
        -- Collapse directory or go to parent
        local entry = self.entries[self.selected]
        if entry and entry.isdir and self.expanded[entry.path] then
            self.expanded[entry.path] = false
            self:refresh()
        end
        return true
    elseif char == "q" or key == 27 then  -- q or Escape
        self.visible = false
        return true
    end
    
    return false
end

function Explorer:render()
    if not self.visible then return end
    
    -- Background
    for y = self.y, self.y + self.height - 1 do
        catvim.render.string(self.x, y, string.rep(" ", self.width), colors.styles.statusline)
    end
    
    -- Border
    catvim.render.box(self.x, self.y, self.width, self.height)
    
    -- Title
    local title = " Explorer "
    catvim.render.string(self.x + 2, self.y, title, { fg = colors.colors.blue, bg = colors.colors.bg_dark, bold = true })
    
    -- Entries
    local visible_rows = self.height - 2
    for i = 1, visible_rows do
        local entry_idx = self.scroll + i
        local entry = self.entries[entry_idx]
        if not entry then break end
        
        local y = self.y + i
        local indent = string.rep("  ", entry.depth)
        local icon = entry.isdir and (self.expanded[entry.path] and icons.folder_open or icons.folder) or icons.file
        local name = entry.name
        
        local max_name_len = self.width - #indent - 4
        if #name > max_name_len then
            name = name:sub(1, max_name_len - 2) .. ".."
        end
        
        local text = indent .. icon .. " " .. name
        text = text .. string.rep(" ", self.width - #text - 1)
        
        local style = entry.isdir and colors.styles.explorer_dir or colors.styles.explorer_file
        if entry_idx == self.selected then
            style = colors.styles.explorer_selected
        end
        
        catvim.render.string(self.x + 1, y, text, style)
    end
end

return Explorer
