# Camel 🐪

A collection of experimental projects exploring systems programming, terminal interfaces, and language design.

---

## catVIM

A fast, lightweight terminal IDE built with **LuaJIT** and **C++**. Combines Vim's modal editing philosophy with modern discoverability.

### Features

| Category | Features |
|----------|----------|
| **Editing** | Modal editing (Normal, Insert, Visual, Command), syntax highlighting |
| **Languages** | Lua, C/C++, x86/ARM64 Assembly |
| **Navigation** | Search (`/`), jump to line (`:42`), word motion (`w`/`b`) |
| **Mouse** | Click to move cursor, scroll, clickable toolbar |
| **File Ops** | Explorer (`Ctrl+E`), save/load, file tree navigation |

### Quick Start

```bash
cd catVIM
make
./catvim                    # Welcome screen
./catvim path/to/file.lua   # Open file
```

### Key Bindings

**Normal Mode**
| Key | Action |
|-----|--------|
| `i` / `a` | Insert before/after cursor |
| `o` / `O` | New line below/above |
| `h j k l` | Move cursor |
| `w` / `b` | Word forward/backward |
| `gg` / `G` | Top/bottom of file |
| `dd` | Delete line |
| `yy` | Yank (copy) line |
| `p` / `P` | Paste after/before |
| `u` | Undo |
| `Ctrl+R` | Redo |
| `/` | Search forward |
| `n` / `N` | Next/previous match |
| `:w` | Save |
| `:q` | Quit |

### Architecture

```
catVIM/
├── src/core/          # C++ (terminal I/O, rendering, input parsing)
│   ├── terminal.cpp   # Raw mode, mouse tracking, screen control
│   ├── renderer.cpp   # Double-buffered ANSI rendering
│   ├── input.cpp      # Keyboard/mouse event parsing
│   └── lua_bindings.cpp
├── src/lua/           # LuaJIT (editor logic)
│   ├── editor/        # Buffer, cursor, modes, syntax
│   └── ui/            # Statusline, explorer, buttons
└── Makefile
```

### Dependencies

- **LuaJIT** (or Lua 5.4)
- **C++17** compiler (clang++ or g++)
- POSIX terminal (macOS, Linux)

---

## License

MIT
