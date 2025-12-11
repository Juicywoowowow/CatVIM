-- catVIM Color Scheme
local M = {}

-- Dark theme colors (RGB)
M.colors = {
    -- Background shades
    bg        = {30, 30, 40},
    bg_dark   = {20, 20, 28},
    bg_light  = {45, 45, 55},
    bg_accent = {50, 50, 65},
    
    -- Foreground
    fg        = {220, 220, 230},
    fg_dim    = {140, 140, 155},
    fg_dark   = {100, 100, 115},
    
    -- Accent colors
    blue      = {100, 150, 255},
    green     = {100, 230, 150},
    yellow    = {255, 220, 100},
    orange    = {255, 160, 100},
    red       = {255, 100, 120},
    purple    = {200, 130, 255},
    cyan      = {80, 220, 230},
    
    -- Semantic colors
    error     = {255, 100, 120},
    warning   = {255, 200, 100},
    info      = {100, 180, 255},
    hint      = {150, 150, 170},
    
    -- UI elements
    border    = {70, 70, 85},
    selection = {60, 80, 120},
    cursor    = {255, 255, 255},
    cursorline = {40, 40, 55},
    
    -- Button states
    btn_normal = {50, 55, 70},
    btn_hover  = {70, 80, 100},
    btn_active = {90, 100, 130},
}

-- Style presets
M.styles = {
    normal = { fg = M.colors.fg, bg = M.colors.bg },
    dim = { fg = M.colors.fg_dim, bg = M.colors.bg },
    
    -- Status line
    statusline = { fg = M.colors.fg, bg = M.colors.bg_dark },
    statusline_mode = { fg = M.colors.bg, bg = M.colors.blue, bold = true },
    
    -- Tabs
    tab = { fg = M.colors.fg_dim, bg = M.colors.bg_dark },
    tab_active = { fg = M.colors.fg, bg = M.colors.bg, bold = true },
    
    -- Buttons
    button = { fg = M.colors.fg, bg = M.colors.btn_normal },
    button_hover = { fg = M.colors.fg, bg = M.colors.btn_hover },
    button_active = { fg = M.colors.fg, bg = M.colors.btn_active, bold = true },
    button_primary = { fg = M.colors.bg, bg = M.colors.blue, bold = true },
    
    -- Explorer
    explorer_file = { fg = M.colors.fg },
    explorer_dir = { fg = M.colors.blue, bold = true },
    explorer_selected = { fg = M.colors.fg, bg = M.colors.selection },
    
    -- Editor
    line_number = { fg = M.colors.fg_dark },
    line_number_current = { fg = M.colors.yellow },
    cursor_line = { bg = M.colors.cursorline },
    selection = { bg = M.colors.selection },
    
    -- Messages
    error = { fg = M.colors.error, bold = true },
    warning = { fg = M.colors.warning },
    info = { fg = M.colors.info },
}

return M
