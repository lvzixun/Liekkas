#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"


#if defined(__APPLE__)
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
        #define PLATFORM_IOS
        int bgm_ios(lua_State* L);
    #endif
#elif __ANDROID__
    #define PLATFORM_ANDROID
    int bgm_android(lua_State* L);
#endif



int
luaopen_liekkas_bgm(lua_State* L) {
    luaL_checkversion(L);
    #ifdef PLATFORM_IOS
    bgm_ios(L);
    #elif PLATFORM_ANDROID 
    bgm_android(L);
    #else
    lua_pushboolean(L, 0);
    #endif

    return 1;
}