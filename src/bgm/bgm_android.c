#include "../opensl/opensl.h"

#include "lua.h"
#include "lauxlib.h"
#include <stdbool.h>


static int
l_bgm_load(lua_State* L) {
    const char* file_name = luaL_checkstring(L, 1);
    bool success = sl_bgm_load(file_name);
    if(!success) {
        luaL_error(L, "load bgm: %s error", file_name);
    }
    return 0;
}


static int
l_bgm_play(lua_State* L) {
    bool is_loop = lua_toboolean(L, 1);
    sl_bgm_play(is_loop);
    return 0;
}

static int
l_bgm_stop(lua_State* L) {
    sl_bgm_stop();
    return 0;
}

static int
l_bgm_pause(lua_State* L) {
    sl_bgm_pause();
    return 0;
}

int
bgm_android(lua_State* L) {
    luaL_Reg l[] = {
        {"load", l_bgm_load},
        {"play", l_bgm_play},
        {"stop", l_bgm_stop},
        {"pause", l_bgm_pause},

        {NULL, NULL},
    };

    luaL_newlib(L, l);
    return 1;
}