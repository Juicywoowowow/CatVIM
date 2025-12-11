#pragma once

#include "lua.hpp"
#include "terminal.hpp"
#include "input.hpp"
#include "renderer.hpp"
#include <memory>

namespace catvim {

class LuaBindings {
public:
    LuaBindings();
    ~LuaBindings();
    
    bool init();
    bool load_file(const char* path);
    bool call_function(const char* name, int nargs = 0, int nresults = 0);
    
    lua_State* state() { return L_; }
    Terminal& terminal() { return terminal_; }
    Renderer& renderer() { return renderer_; }
    InputParser& input() { return input_; }
    
    // Singleton access for Lua callbacks
    static LuaBindings* instance();

private:
    lua_State* L_ = nullptr;
    Terminal terminal_;
    Renderer renderer_;
    InputParser input_;
    
    void register_functions();
    
    // Lua-exposed functions
    static int lua_term_size(lua_State* L);
    static int lua_term_write(lua_State* L);
    static int lua_term_read(lua_State* L);
    static int lua_term_clear(lua_State* L);
    static int lua_term_move(lua_State* L);
    static int lua_term_show_cursor(lua_State* L);
    static int lua_term_hide_cursor(lua_State* L);
    
    static int lua_render_set(lua_State* L);
    static int lua_render_string(lua_State* L);
    static int lua_render_clear(lua_State* L);
    static int lua_render_flush(lua_State* L);
    static int lua_render_box(lua_State* L);
    static int lua_render_resize(lua_State* L);
    
    static int lua_fs_read(lua_State* L);
    static int lua_fs_write(lua_State* L);
    static int lua_fs_list(lua_State* L);
    static int lua_fs_exists(lua_State* L);
    static int lua_fs_isdir(lua_State* L);
    
    static int lua_exec(lua_State* L);
    static int lua_quit(lua_State* L);
};

}  // namespace catvim
