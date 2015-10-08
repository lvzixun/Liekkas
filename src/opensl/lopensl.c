#include "opensl.h"
#include <stdio.h>

#include <stdbool.h>

#include "lopensl.h"
#include "../lk_decode.h"


#define _get_value(type, L, idx) *((type *)lua_touserdata(L, idx))
#define _get_source(L, idx) _get_value(struct sl_source*, L, idx)
#define _get_buffer(L, idx) _get_value(struct sl_buffer*, L, idx)


static int
_feature_unsupport(lua_State* L) {
    luaL_error(L, "the feature unsupport");
    return 0;
}

static int
_id2string(lua_State* L) {
    uintptr_t v = _get_value(uintptr_t, L, -1);
    char buffer[24] = {0};
    sprintf(buffer, "0x%.8x", v);
    lua_pushstring(L, buffer);
    return 1;
}


static int
l_free_source(lua_State* L) {
    struct sl_source* source = _get_source(L, -1);
    sl_source_set_state(source, SL_SOURCE_STOP);
    sl_source_free(source);
    return 0;
}

static int
l_clear_source(lua_State* L) {
    struct sl_source* source = _get_source(L, -1);
    sl_source_clear(source);
    return 0;
}


static int
l_play_source(lua_State* L) {
    struct sl_source* source = _get_source(L, -1);
    sl_source_set_state(source, SL_SOURCE_PLAY);
    return 0;
}

static int
l_stop_source(lua_State* L) {
    struct sl_source* source = _get_source(L, -1);
    sl_source_set_state(source, SL_SOURCE_STOP);
    return 0;
}

static int
l_rewind_source(lua_State* L) {
    struct sl_source* source = _get_source(L, -1);
    sl_source_set_state(source, SL_SOURCE_STOP);
    sl_source_set_state(source, SL_SOURCE_PLAY);
    return 0;
}

static int
l_pause_source(lua_State* L) {
    struct sl_source* source = _get_source(L, -1);
    sl_source_set_state(source, SL_SOURCE_PAUSE);
    return 0;
}

static int
l_volume_source(lua_State* L) {
    struct sl_source* source = _get_source(L, -1);
    float volume = lua_tonumber(L, -2);
    sl_source_volume(source, volume);
    return 0;
}

static int
l_source_state(lua_State* L) {
    struct sl_source* source = _get_source(L, -1);
    enum sl_source_state state =  sl_source_get_state(source);
    lua_pushnumber(L, state);
    return 1;
}


static int
_new_source(lua_State* L, struct sl_source* source) {
    struct sl_source** p = (struct sl_source**)lua_newuserdata(L, sizeof(source));
    *p = source;

    if(luaL_newmetatable(L, "mt_source")) {
        lua_pushcfunction(L, l_free_source);
        lua_setfield(L, -2, "__gc");
        lua_pushcfunction(L, _id2string);
        lua_setfield(L, -2, "__tostring");

        luaL_Reg l[] = {
            {"clear", l_clear_source},
            {"play", l_play_source},
            {"stop", l_stop_source},
            {"rewind", l_rewind_source},
            {"pause", l_pause_source},
            {"state", l_source_state},
            {"volume", l_volume_source},
            {"position", _feature_unsupport},
            {NULL, NULL},
        };
        luaL_newlib(L, l);
        lua_setfield(L, -2, "__index");
    }

    lua_setmetatable(L, -2);
    return 1;
}


static int
l_create_source(lua_State* L) {
    struct sl_source* source = sl_source_gen();
    if(source == NULL)
        luaL_error(L, "create source error");
    return _new_source(L, source);
}


static int
l_free_buffer(lua_State* L) {
    struct sl_buffer* buffer = _get_buffer(L, -1);
    sl_buffer_free(buffer);
    return 0;
}


static int
_new_buffer(lua_State* L, struct sl_buffer* buffer) {
    struct sl_buffer** p = (struct sl_buffer**)lua_newuserdata(L, sizeof(buffer));
    *p = buffer;
    if(luaL_newmetatable(L, "mt_buffer")) {
        lua_pushcfunction(L, l_free_buffer);
        lua_setfield(L, -2, "__gc");
        lua_pushcfunction(L, _id2string);
        lua_setfield(L, -2, "__tostring");
    }
    
    lua_setmetatable(L, -2);
    return 1;
}


static int
l_create_buffer(lua_State* L) {
    struct sl_buffer* buffer = sl_buffer_gen();
    if(buffer == NULL)
        luaL_error(L, "create buffer error");
    return _new_buffer(L, buffer);
}


static int
l_bind_buffer(lua_State* L) {
    struct sl_buffer* buffer = _get_buffer(L, 1);
    struct oal_info* info = (struct oal_info*)lua_touserdata(L, 2);

    sl_buffer_data(buffer, info->data, info->size);
    return 0;
}

static int
l_set(lua_State* L) {
    struct sl_source* source = _get_source(L, 1);
    struct sl_buffer* buffer = _get_buffer(L, 2);
    lua_Number pitch = lua_tonumber(L, 3);
    lua_Number max_distance = lua_tonumber(L, 4);
    lua_Number gain = lua_tonumber(L, 5);
    int loop = lua_toboolean(L, 6);

    unused(max_distance);
    unused(gain);

    sl_source_clear(source);
    sl_source_bind(source, buffer);
    sl_source_loop(source, loop);
    sl_source_pitch(source, pitch);

    lua_pushvalue(L, 1);
    return 1;
}

static int
l_log(lua_State* L) {
    const char* str = lua_tostring(L, -1);
    sl_log("%s\n", str);
    return 0;
}


// --------bgm-------------
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


int
luaopen_liekkas(lua_State* L) {
    luaL_checkversion(L);
    if(!sl_isinit()) {
        luaL_error(L, "opensl not init");
    }

    luaL_Reg l[] = {
        {"create_source", l_create_source},
        {"create_buffer", l_create_buffer},
        {"buffer_bind", l_bind_buffer},
        {"source_set", l_set},
        {"listen_position", _feature_unsupport},
        {"log", l_log},
        {NULL, NULL},
    };

    luaL_newlib(L, l);
    return 1;
}

