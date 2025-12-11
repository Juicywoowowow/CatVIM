#include "lua_bindings.hpp"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <libgen.h>

extern bool catvim_should_quit();

int main(int argc, char* argv[]) {
    catvim::LuaBindings app;
    
    if (!app.init()) {
        fprintf(stderr, "Failed to initialize catVIM\n");
        return 1;
    }
    
    // Find Lua scripts path
    // First try: ./src/lua/init.lua (development)
    // Second try: ~/.config/catvim/init.lua (installed)
    const char* lua_paths[] = {
        "./src/lua/init.lua",
        nullptr  // Will be set to home config path
    };
    
    char config_path[512];
    const char* home = getenv("HOME");
    if (home) {
        snprintf(config_path, sizeof(config_path), "%s/.config/catvim/init.lua", home);
        lua_paths[1] = config_path;
    }
    
    // Set package path for Lua modules
    lua_getglobal(app.state(), "package");
    lua_getfield(app.state(), -1, "path");
    std::string path = lua_tostring(app.state(), -1);
    
    path += ";./src/lua/?.lua;./src/lua/?/init.lua";
    if (home) {
        path += ";";
        path += home;
        path += "/.config/catvim/?.lua;";
        path += home;
        path += "/.config/catvim/?/init.lua";
    }
    
    lua_pushstring(app.state(), path.c_str());
    lua_setfield(app.state(), -3, "path");
    lua_pop(app.state(), 2);
    
    // Load init.lua
    bool loaded = false;
    for (const char* p : lua_paths) {
        if (p && access(p, R_OK) == 0) {
            if (app.load_file(p)) {
                loaded = true;
                break;
            }
        }
    }
    
    if (!loaded) {
        fprintf(stderr, "Could not find init.lua\nTry running from catVIM directory or install with 'make install'\n");
        return 1;
    }
    
    // Pass command line args to Lua
    lua_newtable(app.state());
    for (int i = 1; i < argc; i++) {
        lua_pushstring(app.state(), argv[i]);
        lua_rawseti(app.state(), -2, i);
    }
    lua_setglobal(app.state(), "arg");
    
    // Call init()
    app.call_function("init");
    
    // Main loop - poll() in term.read() handles timing
    while (!catvim_should_quit()) {
        app.call_function("update");
    }
    
    // Cleanup is handled by LuaBindings destructor
    return 0;
}
