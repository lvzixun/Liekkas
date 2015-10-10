#include <jni.h>
#include <android/log.h>

#include "liekkas/src/opensl/lopensl.h"
#include "liekkas/src/opensl/opensl.h"
#include "liekkas/src/lk_util.h"
#include "liekkas/src/lk_decode.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

static lua_State* L = NULL;

static void
_register(lua_State *L, lua_CFunction func, const char * libname) {
    luaL_requiref(L, libname, func, 0);
    lua_pop(L, 1);
}

static int
traceback(lua_State *L) {
    const char *msg = lua_tostring(L, 1);
    if (msg)
        luaL_traceback(L, L, msg, 1);
    else if (!lua_isnoneornil(L, 1)) {
        if (!luaL_callmeta(L, 1, "__tostring"))
            lua_pushliteral(L, "(no error message)");
    }
    return 1;
}


static int
_readall(lua_State* L) {
    const char* path = lua_tostring(L, 1);
    struct util_fp* handle = util_file_open(path);
    if(!handle){
        lua_pushnil(L);
    }else {
        size_t sz = util_file_size(handle);
        char tmp[sz];
        util_file_readall(handle, tmp, sz);
        lua_pushlstring(L, tmp, sz);
        util_file_close(handle);
    }
    return 1;
}



static int
l_util(lua_State* L) {
    luaL_Reg l[] = {
        {"readall", _readall},
        {NULL, NULL},
    };

    luaL_newlib(L, l);
    return 1;
}


JNIEXPORT void JNICALL
Java_com_example_testaudio_TestAudio_testInit(JNIEnv* env, jclass clazz) {
    const char* lua_src = "android_test.lua";
    L =  luaL_newstate();

    struct util_fp* handle = util_file_open(lua_src);
    if(!handle) {
        sl_log("open %s error\n", lua_src);
        return;
    }
    size_t sz = util_file_size(handle);
    char src[sz];
    util_file_readall(handle, src, sz);

    luaL_openlibs(L);
    _register(L, l_util, "util");
    _register(L, luaopen_liekkas, "liekkas");
    _register(L, bgm_android, "liekkas.bgm");
    _register(L, luaopen_liekkas_decode, "liekkas.decode");
    lua_pushcfunction(L, traceback);
    int top = lua_gettop(L);

    if(luaL_loadbuffer(L, (const char*)src, sz, "android_test.lua") != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        sl_log("android jni error: %s\n", err);
        goto EXIT;
    }

    if(lua_pcall(L, 0, 0, top) != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        sl_log("lua error: %s\n", err);
        goto EXIT;
    }

EXIT:
    util_file_close(handle);
}


JNIEXPORT void JNICALL
Java_com_example_testaudio_TestAudio_testDestory(JNIEnv* env, jclass clazz) {
    sl_log("Java_com_example_testaudio_TestAudio_testDestory!!!\n");
    if(L) {
        lua_close(L);
    }
}

