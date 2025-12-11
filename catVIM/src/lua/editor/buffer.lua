-- catVIM Buffer - Text storage with gap buffer implementation
local Buffer = {}
Buffer.__index = Buffer

function Buffer:new(opts)
    opts = opts or {}
    local self = setmetatable({}, Buffer)
    self.lines = opts.lines or {""}
    self.filepath = opts.filepath or nil
    self.modified = false
    self.readonly = opts.readonly or false
    self.filetype = opts.filetype or "text"
    self.name = opts.name or "[No Name]"
    return self
end

function Buffer:load(filepath)
    local content, err = catvim.fs.read(filepath)
    if not content then
        return false, err
    end
    
    self.lines = {}
    for line in (content .. "\n"):gmatch("([^\n]*)\n") do
        table.insert(self.lines, line)
    end
    if #self.lines == 0 then
        self.lines = {""}
    end
    
    self.filepath = filepath
    self.name = filepath:match("([^/]+)$") or filepath
    self.modified = false
    self:detect_filetype()
    return true
end

function Buffer:save(filepath)
    filepath = filepath or self.filepath
    if not filepath then
        return false, "No filepath specified"
    end
    
    local content = table.concat(self.lines, "\n")
    local ok, err = catvim.fs.write(filepath, content)
    if not ok then
        return false, err
    end
    
    self.filepath = filepath
    self.name = filepath:match("([^/]+)$") or filepath
    self.modified = false
    return true
end

function Buffer:detect_filetype()
    if not self.filepath then return end
    local ext = self.filepath:match("%.([^%.]+)$")
    local map = {
        lua = "lua", py = "python", js = "javascript",
        ts = "typescript", c = "c", cpp = "cpp", h = "c",
        hpp = "cpp", rs = "rust", go = "go", rb = "ruby",
        md = "markdown", json = "json", yaml = "yaml",
        yml = "yaml", html = "html", css = "css",
    }
    self.filetype = map[ext] or "text"
end

function Buffer:line_count()
    return #self.lines
end

function Buffer:get_line(n)
    return self.lines[n] or ""
end

function Buffer:set_line(n, text)
    if n >= 1 and n <= #self.lines then
        self.lines[n] = text
        self.modified = true
    end
end

function Buffer:insert_line(n, text)
    table.insert(self.lines, n, text or "")
    self.modified = true
end

function Buffer:delete_line(n)
    if #self.lines > 1 then
        table.remove(self.lines, n)
        self.modified = true
    else
        self.lines[1] = ""
        self.modified = true
    end
end

function Buffer:insert_char(line, col, char)
    local l = self.lines[line] or ""
    self.lines[line] = l:sub(1, col - 1) .. char .. l:sub(col)
    self.modified = true
end

function Buffer:delete_char(line, col)
    local l = self.lines[line] or ""
    if col > 1 then
        self.lines[line] = l:sub(1, col - 2) .. l:sub(col)
        self.modified = true
        return true
    elseif line > 1 then
        -- Join with previous line
        local prev = self.lines[line - 1] or ""
        self.lines[line - 1] = prev .. l
        table.remove(self.lines, line)
        self.modified = true
        return true, #prev + 1
    end
    return false
end

function Buffer:split_line(line, col)
    local l = self.lines[line] or ""
    local before = l:sub(1, col - 1)
    local after = l:sub(col)
    self.lines[line] = before
    table.insert(self.lines, line + 1, after)
    self.modified = true
end

return Buffer
