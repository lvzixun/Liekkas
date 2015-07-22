// the test case for window mingw

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <stdio.h>

#include "oal.h"

static void
_register(lua_State* L, const char* name, lua_CFunction f) {
  luaL_requiref(L, name, f, 0);
  lua_pop(L, 1);
}

int main(int argc, char const *argv[]) {
  lua_State* L =  luaL_newstate();

  luaL_openlibs(L);
  _register(L, "oal", luaopen_oal);
  _register(L, "oal.decode", luaopen_oal_decode);

  if(luaL_dofile(L, "../oal/t.lua")) {
    const char* err = lua_tostring(L, -1);
    printf("error: %s\n", err);
  }

  lua_close(L);
  return 0;
}