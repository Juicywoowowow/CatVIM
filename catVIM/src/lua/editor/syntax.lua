-- catVIM Syntax Highlighting
local colors = require("ui.colors")

local M = {}

-- Highlight styles
M.styles = {
    keyword = { fg = colors.colors.purple, bold = true },
    type = { fg = colors.colors.cyan },
    string = { fg = colors.colors.green },
    number = { fg = colors.colors.orange },
    comment = { fg = colors.colors.fg_dim, italic = true },
    operator = { fg = colors.colors.yellow },
    function_name = { fg = colors.colors.blue },
    preprocessor = { fg = colors.colors.orange, bold = true },
    register = { fg = colors.colors.red },
    label = { fg = colors.colors.yellow },
    instruction = { fg = colors.colors.purple },
    constant = { fg = colors.colors.orange },
}

-- Language definitions
M.languages = {}

-- Lua syntax
M.languages.lua = {
    keywords = {
        "and", "break", "do", "else", "elseif", "end", "false", "for",
        "function", "goto", "if", "in", "local", "nil", "not", "or",
        "repeat", "return", "then", "true", "until", "while"
    },
    types = {
        "string", "number", "boolean", "table", "function", "thread",
        "userdata", "nil"
    },
    patterns = {
        { pattern = "%-%-.-$", style = "comment" },  -- Single line comment
        { pattern = "%-%-%[%[.-%]%]", style = "comment" },  -- Multi-line comment
        { pattern = '".-"', style = "string" },
        { pattern = "'.-'", style = "string" },
        { pattern = "%[%[.-%]%]", style = "string" },  -- Long strings
        { pattern = "0x[%da-fA-F]+", style = "number" },
        { pattern = "%d+%.?%d*", style = "number" },
        { pattern = "[%+%-%%%*/%^#=<>~]", style = "operator" },
        { pattern = "function%s+([%w_]+)", style = "function_name", capture = 1 },
    },
}

-- C/C++ syntax
M.languages.c = {
    keywords = {
        "auto", "break", "case", "const", "continue", "default", "do",
        "else", "enum", "extern", "for", "goto", "if", "inline",
        "register", "restrict", "return", "sizeof", "static", "struct",
        "switch", "typedef", "union", "volatile", "while",
        -- C++ extras
        "alignas", "alignof", "and", "and_eq", "asm", "bitand", "bitor",
        "catch", "class", "compl", "constexpr", "const_cast", "decltype",
        "delete", "dynamic_cast", "explicit", "export", "false", "friend",
        "mutable", "namespace", "new", "noexcept", "not", "not_eq",
        "nullptr", "operator", "or", "or_eq", "private", "protected",
        "public", "reinterpret_cast", "static_assert", "static_cast",
        "template", "this", "throw", "true", "try", "typeid", "typename",
        "using", "virtual", "xor", "xor_eq", "override", "final"
    },
    types = {
        "void", "char", "short", "int", "long", "float", "double",
        "signed", "unsigned", "bool", "size_t", "int8_t", "int16_t",
        "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t",
        "char16_t", "char32_t", "wchar_t", "string", "vector", "map",
        "set", "list", "array", "unique_ptr", "shared_ptr"
    },
    patterns = {
        { pattern = "//.-$", style = "comment" },
        { pattern = "/%*.-%*/", style = "comment" },
        { pattern = '".-"', style = "string" },
        { pattern = "'.-'", style = "string" },
        { pattern = "^%s*#%w+", style = "preprocessor" },
        { pattern = "0x[%da-fA-F]+[uUlL]*", style = "number" },
        { pattern = "0b[01]+[uUlL]*", style = "number" },
        { pattern = "%d+%.?%d*[fFlLuU]*", style = "number" },
        { pattern = "[%+%-%%%*/%^&|=<>!~]", style = "operator" },
    },
}
M.languages.cpp = M.languages.c
M.languages.h = M.languages.c
M.languages.hpp = M.languages.c

-- x86/ARM64 Assembly syntax
M.languages.asm = {
    keywords = {},  -- Instructions handled separately
    types = {},
    instructions_x86 = {
        -- Data movement
        "mov", "movzx", "movsx", "lea", "push", "pop", "xchg",
        -- Arithmetic
        "add", "sub", "mul", "imul", "div", "idiv", "inc", "dec", "neg",
        -- Logic
        "and", "or", "xor", "not", "shl", "shr", "sar", "rol", "ror",
        -- Compare/test
        "cmp", "test",
        -- Jumps
        "jmp", "je", "jne", "jz", "jnz", "jg", "jge", "jl", "jle",
        "ja", "jae", "jb", "jbe", "jo", "jno", "js", "jns",
        -- Call/return
        "call", "ret", "leave", "enter",
        -- Stack
        "pusha", "popa", "pushf", "popf",
        -- String
        "rep", "movs", "cmps", "scas", "lods", "stos",
        -- System
        "syscall", "sysenter", "int", "iret", "cli", "sti", "hlt", "nop",
        -- SSE/AVX (common)
        "movaps", "movups", "addps", "subps", "mulps", "divps",
    },
    instructions_arm64 = {
        -- Data movement
        "mov", "mvn", "ldr", "str", "ldp", "stp", "adr", "adrp",
        -- Arithmetic
        "add", "adds", "sub", "subs", "mul", "madd", "msub", "neg",
        -- Logic
        "and", "ands", "orr", "eor", "bic", "lsl", "lsr", "asr", "ror",
        -- Compare
        "cmp", "cmn", "tst",
        -- Branch
        "b", "bl", "br", "blr", "ret", "cbz", "cbnz", "tbz", "tbnz",
        "b.eq", "b.ne", "b.lt", "b.le", "b.gt", "b.ge",
        -- System
        "svc", "hvc", "smc", "nop", "wfi", "wfe",
    },
    registers_x86 = {
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp",
        "ax", "bx", "cx", "dx", "si", "di", "bp", "sp",
        "al", "bl", "cl", "dl", "ah", "bh", "ch", "dh",
        "rip", "eip", "ip", "cs", "ds", "es", "fs", "gs", "ss",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
    },
    registers_arm64 = {
        "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
        "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18",
        "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27",
        "x28", "x29", "x30", "sp", "lr", "xzr", "wzr",
        "w0", "w1", "w2", "w3", "w4", "w5", "w6", "w7", "w8", "w9",
        "v0", "v1", "v2", "v3", "q0", "q1", "q2", "q3",
    },
    patterns = {
        { pattern = "[;#].-$", style = "comment" },  -- Comment
        { pattern = '".-"', style = "string" },
        { pattern = "'.-'", style = "string" },
        { pattern = "0x[%da-fA-F]+", style = "number" },
        { pattern = "0b[01]+", style = "number" },
        { pattern = "%$?%d+", style = "number" },
        { pattern = "^%s*%.%w+", style = "preprocessor" },  -- Directives
        { pattern = "^%s*[%w_]+:", style = "label" },  -- Labels
    },
}
M.languages.s = M.languages.asm
M.languages.S = M.languages.asm

-- Build keyword lookup tables for fast matching
local function build_lookup(lang)
    lang.keyword_set = {}
    lang.type_set = {}
    
    if lang.keywords then
        for _, kw in ipairs(lang.keywords) do
            lang.keyword_set[kw] = true
        end
    end
    
    if lang.types then
        for _, t in ipairs(lang.types) do
            lang.type_set[t] = true
        end
    end
    
    -- Assembly-specific
    if lang.instructions_x86 then
        lang.instr_set = {}
        for _, instr in ipairs(lang.instructions_x86) do
            lang.instr_set[instr:lower()] = true
        end
        for _, instr in ipairs(lang.instructions_arm64 or {}) do
            lang.instr_set[instr:lower()] = true
        end
    end
    
    if lang.registers_x86 then
        lang.reg_set = {}
        for _, reg in ipairs(lang.registers_x86) do
            lang.reg_set[reg:lower()] = true
        end
        for _, reg in ipairs(lang.registers_arm64 or {}) do
            lang.reg_set[reg:lower()] = true
        end
    end
end

for _, lang in pairs(M.languages) do
    build_lookup(lang)
end

-- Highlight a single line, returns list of {start, len, style}
function M.highlight_line(line, filetype)
    local lang = M.languages[filetype]
    if not lang then return {} end
    
    local highlights = {}
    local covered = {}  -- Track which positions are already highlighted
    
    -- Mark a range as highlighted
    local function mark(s, e, style)
        if s and e and s <= e then
            table.insert(highlights, {start = s, finish = e, style = style})
            for i = s, e do covered[i] = true end
        end
    end
    
    -- Apply pattern-based highlights first
    if lang.patterns then
        for _, pat in ipairs(lang.patterns) do
            local s, e, cap = line:find(pat.pattern)
            while s do
                if pat.capture and cap then
                    -- Highlight only the capture group
                    local cap_start = line:find(cap, s, true)
                    if cap_start then
                        mark(cap_start, cap_start + #cap - 1, pat.style)
                    end
                else
                    mark(s, e, pat.style)
                end
                s, e, cap = line:find(pat.pattern, e + 1)
            end
        end
    end
    
    -- Highlight keywords, types, instructions, registers
    for word_start, word in line:gmatch("()([%w_]+)") do
        local word_end = word_start + #word - 1
        
        -- Skip if already covered (e.g., in a string or comment)
        if not covered[word_start] then
            local lower_word = word:lower()
            
            if lang.keyword_set and lang.keyword_set[word] then
                mark(word_start, word_end, "keyword")
            elseif lang.type_set and lang.type_set[word] then
                mark(word_start, word_end, "type")
            elseif lang.instr_set and lang.instr_set[lower_word] then
                mark(word_start, word_end, "instruction")
            elseif lang.reg_set and lang.reg_set[lower_word] then
                mark(word_start, word_end, "register")
            end
        end
    end
    
    return highlights
end

-- Get style for a character position
function M.get_style(highlights, pos)
    for _, hl in ipairs(highlights) do
        if pos >= hl.start and pos <= hl.finish then
            return M.styles[hl.style]
        end
    end
    return nil
end

return M
