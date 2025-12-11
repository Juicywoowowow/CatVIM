#include "lua_bindings.hpp"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

namespace catvim {

static LuaBindings* g_instance = nullptr;
static bool g_should_quit = false;

LuaBindings* LuaBindings::instance() {
    return g_instance;
}

LuaBindings::LuaBindings() {
    g_instance = this;
}

LuaBindings::~LuaBindings() {
    if (L_) {
        lua_close(L_);
    }
    g_instance = nullptr;
}

bool LuaBindings::init() {
    L_ = luaL_newstate();
    if (!L_) return false;
    
    luaL_openlibs(L_);
    register_functions();
    
    // Initialize terminal
    terminal_.enter_raw_mode();
    terminal_.enable_alternate_screen();
    terminal_.enable_mouse();
    terminal_.hide_cursor();
    
    // Initialize renderer with terminal size
    Vec2 size = terminal_.get_size();
    renderer_.resize(size.x, size.y);
    
    return true;
}

void LuaBindings::register_functions() {
    // Create catvim table
    lua_newtable(L_);
    
    // catvim.term
    lua_newtable(L_);
    lua_pushcfunction(L_, lua_term_size); lua_setfield(L_, -2, "size");
    lua_pushcfunction(L_, lua_term_write); lua_setfield(L_, -2, "write");
    lua_pushcfunction(L_, lua_term_read); lua_setfield(L_, -2, "read");
    lua_pushcfunction(L_, lua_term_clear); lua_setfield(L_, -2, "clear");
    lua_pushcfunction(L_, lua_term_move); lua_setfield(L_, -2, "move");
    lua_pushcfunction(L_, lua_term_show_cursor); lua_setfield(L_, -2, "show_cursor");
    lua_pushcfunction(L_, lua_term_hide_cursor); lua_setfield(L_, -2, "hide_cursor");
    lua_setfield(L_, -2, "term");
    
    // catvim.render
    lua_newtable(L_);
    lua_pushcfunction(L_, lua_render_set); lua_setfield(L_, -2, "set");
    lua_pushcfunction(L_, lua_render_string); lua_setfield(L_, -2, "string");
    lua_pushcfunction(L_, lua_render_clear); lua_setfield(L_, -2, "clear");
    lua_pushcfunction(L_, lua_render_flush); lua_setfield(L_, -2, "flush");
    lua_pushcfunction(L_, lua_render_box); lua_setfield(L_, -2, "box");
    lua_pushcfunction(L_, lua_render_resize); lua_setfield(L_, -2, "resize");
    lua_setfield(L_, -2, "render");
    
    // catvim.fs
    lua_newtable(L_);
    lua_pushcfunction(L_, lua_fs_read); lua_setfield(L_, -2, "read");
    lua_pushcfunction(L_, lua_fs_write); lua_setfield(L_, -2, "write");
    lua_pushcfunction(L_, lua_fs_list); lua_setfield(L_, -2, "list");
    lua_pushcfunction(L_, lua_fs_exists); lua_setfield(L_, -2, "exists");
    lua_pushcfunction(L_, lua_fs_isdir); lua_setfield(L_, -2, "isdir");
    lua_setfield(L_, -2, "fs");
    
    // catvim.exec, catvim.quit
    lua_pushcfunction(L_, lua_exec); lua_setfield(L_, -2, "exec");
    lua_pushcfunction(L_, lua_quit); lua_setfield(L_, -2, "quit");
    
    lua_setglobal(L_, "catvim");
}

bool LuaBindings::load_file(const char* path) {
    if (luaL_loadfile(L_, path) != 0) {
        fprintf(stderr, "Error loading %s: %s\n", path, lua_tostring(L_, -1));
        lua_pop(L_, 1);
        return false;
    }
    if (lua_pcall(L_, 0, 0, 0) != 0) {
        fprintf(stderr, "Error running %s: %s\n", path, lua_tostring(L_, -1));
        lua_pop(L_, 1);
        return false;
    }
    return true;
}

bool LuaBindings::call_function(const char* name, int nargs, int nresults) {
    lua_getglobal(L_, name);
    if (!lua_isfunction(L_, -1)) {
        lua_pop(L_, 1);
        return false;
    }
    if (lua_pcall(L_, nargs, nresults, 0) != 0) {
        fprintf(stderr, "Error calling %s: %s\n", name, lua_tostring(L_, -1));
        lua_pop(L_, 1);
        return false;
    }
    return true;
}

// Terminal functions
int LuaBindings::lua_term_size(lua_State* L) {
    Vec2 size = instance()->terminal().get_size();
    lua_newtable(L);
    lua_pushinteger(L, size.x); lua_setfield(L, -2, "width");
    lua_pushinteger(L, size.y); lua_setfield(L, -2, "height");
    return 1;
}

int LuaBindings::lua_term_write(lua_State* L) {
    const char* str = luaL_checkstring(L, 1);
    instance()->terminal().write(str);
    return 0;
}

int LuaBindings::lua_term_read(lua_State* L) {
    auto& term = instance()->terminal();
    auto& parser = instance()->input();
    
    std::string buf;
    int c;
    while ((c = term.read_byte()) != -1) {
        buf += static_cast<char>(c);
    }
    
    if (buf.empty()) {
        lua_pushnil(L);
        return 1;
    }
    
    Event evt;
    size_t consumed;
    if (!parser.parse(buf, evt, consumed)) {
        lua_pushnil(L);
        return 1;
    }
    
    lua_newtable(L);
    
    if (evt.type == EventType::KEY) {
        lua_pushstring(L, "key"); lua_setfield(L, -2, "type");
        lua_pushinteger(L, evt.key.key); lua_setfield(L, -2, "key");
        lua_pushboolean(L, evt.key.ctrl); lua_setfield(L, -2, "ctrl");
        lua_pushboolean(L, evt.key.alt); lua_setfield(L, -2, "alt");
        lua_pushboolean(L, evt.key.shift); lua_setfield(L, -2, "shift");
        
        // Also provide a char representation for printable keys
        if (evt.key.key >= 32 && evt.key.key < 127) {
            char ch[2] = {static_cast<char>(evt.key.key), 0};
            lua_pushstring(L, ch); lua_setfield(L, -2, "char");
        }
    } else if (evt.type == EventType::MOUSE) {
        lua_pushstring(L, "mouse"); lua_setfield(L, -2, "type");
        lua_pushinteger(L, evt.mouse.x); lua_setfield(L, -2, "x");
        lua_pushinteger(L, evt.mouse.y); lua_setfield(L, -2, "y");
        lua_pushinteger(L, static_cast<int>(evt.mouse.button)); lua_setfield(L, -2, "button");
        lua_pushboolean(L, evt.mouse.ctrl); lua_setfield(L, -2, "ctrl");
        lua_pushboolean(L, evt.mouse.alt); lua_setfield(L, -2, "alt");
        lua_pushboolean(L, evt.mouse.shift); lua_setfield(L, -2, "shift");
        
        const char* action = "unknown";
        switch (evt.mouse.action) {
            case MouseAction::PRESS: action = "press"; break;
            case MouseAction::RELEASE: action = "release"; break;
            case MouseAction::DRAG: action = "drag"; break;
            case MouseAction::SCROLL: action = "scroll"; break;
        }
        lua_pushstring(L, action); lua_setfield(L, -2, "action");
    }
    
    return 1;
}

int LuaBindings::lua_term_clear(lua_State*) {
    instance()->terminal().clear();
    return 0;
}

int LuaBindings::lua_term_move(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    instance()->terminal().move_cursor(x, y);
    return 0;
}

int LuaBindings::lua_term_show_cursor(lua_State*) {
    instance()->terminal().show_cursor();
    return 0;
}

int LuaBindings::lua_term_hide_cursor(lua_State*) {
    instance()->terminal().hide_cursor();
    return 0;
}

// Render functions
int LuaBindings::lua_render_set(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    const char* ch = luaL_checkstring(L, 3);
    
    Style style;
    if (lua_gettop(L) >= 4 && lua_istable(L, 4)) {
        lua_getfield(L, 4, "fg");
        if (lua_istable(L, -1)) {
            lua_rawgeti(L, -1, 1); int r = lua_tointeger(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 2); int g = lua_tointeger(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 3); int b = lua_tointeger(L, -1); lua_pop(L, 1);
            style.fg = Color::RGB(r, g, b);
        }
        lua_pop(L, 1);
        
        lua_getfield(L, 4, "bg");
        if (lua_istable(L, -1)) {
            lua_rawgeti(L, -1, 1); int r = lua_tointeger(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 2); int g = lua_tointeger(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 3); int b = lua_tointeger(L, -1); lua_pop(L, 1);
            style.bg = Color::RGB(r, g, b);
        }
        lua_pop(L, 1);
        
        lua_getfield(L, 4, "bold");
        if (lua_toboolean(L, -1)) style.attrs = style.attrs | Attr::BOLD;
        lua_pop(L, 1);
        
        lua_getfield(L, 4, "italic");
        if (lua_toboolean(L, -1)) style.attrs = style.attrs | Attr::ITALIC;
        lua_pop(L, 1);
        
        lua_getfield(L, 4, "underline");
        if (lua_toboolean(L, -1)) style.attrs = style.attrs | Attr::UNDERLINE;
        lua_pop(L, 1);
    }
    
    instance()->renderer().set_cell(x - 1, y - 1, ch[0], style);
    return 0;
}

int LuaBindings::lua_render_string(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    const char* str = luaL_checkstring(L, 3);
    
    Style style;
    // Parse style if provided (same as lua_render_set)
    if (lua_gettop(L) >= 4 && lua_istable(L, 4)) {
        lua_getfield(L, 4, "fg");
        if (lua_istable(L, -1)) {
            lua_rawgeti(L, -1, 1); int r = lua_tointeger(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 2); int g = lua_tointeger(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 3); int b = lua_tointeger(L, -1); lua_pop(L, 1);
            style.fg = Color::RGB(r, g, b);
        }
        lua_pop(L, 1);
        
        lua_getfield(L, 4, "bg");
        if (lua_istable(L, -1)) {
            lua_rawgeti(L, -1, 1); int r = lua_tointeger(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 2); int g = lua_tointeger(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, -1, 3); int b = lua_tointeger(L, -1); lua_pop(L, 1);
            style.bg = Color::RGB(r, g, b);
        }
        lua_pop(L, 1);
        
        lua_getfield(L, 4, "bold");
        if (lua_toboolean(L, -1)) style.attrs = style.attrs | Attr::BOLD;
        lua_pop(L, 1);
    }
    
    instance()->renderer().set_string(x - 1, y - 1, str, style);
    return 0;
}

int LuaBindings::lua_render_clear(lua_State*) {
    instance()->renderer().clear();
    return 0;
}

int LuaBindings::lua_render_flush(lua_State*) {
    std::string output = instance()->renderer().flush();
    instance()->terminal().write(output);
    instance()->terminal().flush();
    return 0;
}

int LuaBindings::lua_render_box(lua_State* L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int w = luaL_checkinteger(L, 3);
    int h = luaL_checkinteger(L, 4);
    Style style;
    instance()->renderer().draw_box(x - 1, y - 1, w, h, style);
    return 0;
}

int LuaBindings::lua_render_resize(lua_State* L) {
    Vec2 size = instance()->terminal().get_size();
    instance()->renderer().resize(size.x, size.y);
    lua_pushinteger(L, size.x);
    lua_pushinteger(L, size.y);
    return 2;
}

// File system functions
int LuaBindings::lua_fs_read(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    std::ifstream file(path);
    if (!file) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to open file");
        return 2;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    lua_pushstring(L, ss.str().c_str());
    return 1;
}

int LuaBindings::lua_fs_write(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    const char* data = luaL_checkstring(L, 2);
    std::ofstream file(path);
    if (!file) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Failed to open file for writing");
        return 2;
    }
    file << data;
    lua_pushboolean(L, true);
    return 1;
}

int LuaBindings::lua_fs_list(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    DIR* dir = opendir(path);
    if (!dir) {
        lua_pushnil(L);
        return 1;
    }
    
    lua_newtable(L);
    int i = 1;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") != 0) {
            lua_newtable(L);
            lua_pushstring(L, entry->d_name);
            lua_setfield(L, -2, "name");
            lua_pushboolean(L, entry->d_type == DT_DIR);
            lua_setfield(L, -2, "isdir");
            lua_rawseti(L, -2, i++);
        }
    }
    closedir(dir);
    return 1;
}

int LuaBindings::lua_fs_exists(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    struct stat st;
    lua_pushboolean(L, stat(path, &st) == 0);
    return 1;
}

int LuaBindings::lua_fs_isdir(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    struct stat st;
    if (stat(path, &st) != 0) {
        lua_pushboolean(L, false);
        return 1;
    }
    lua_pushboolean(L, S_ISDIR(st.st_mode));
    return 1;
}

int LuaBindings::lua_exec(lua_State* L) {
    const char* cmd = luaL_checkstring(L, 1);
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to execute command");
        return 2;
    }
    
    std::string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    int status = pclose(pipe);
    
    lua_pushstring(L, result.c_str());
    lua_pushinteger(L, WEXITSTATUS(status));
    return 2;
}

int LuaBindings::lua_quit(lua_State*) {
    g_should_quit = true;
    return 0;
}

}  // namespace catvim

// Global accessor for quit flag
bool catvim_should_quit() {
    return catvim::g_should_quit;
}
