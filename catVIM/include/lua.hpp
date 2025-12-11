#pragma once

// Lua/LuaJIT compatibility header
#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#ifdef __cplusplus
}
#endif

// Helper macros for Lua 5.1/5.4 compatibility
#if LUA_VERSION_NUM < 502
    #define lua_rawlen lua_objlen
    #define luaL_setfuncs(L, funcs, n) luaL_register(L, NULL, funcs)
#endif
