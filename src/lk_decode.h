#ifndef _LK_DECODE_H_
#define _LK_DECODE_H_

#include <lua.h>
#include <lauxlib.h>
#include "lk_util.h"


struct oal_info {
  void* data;
  unsigned int size;
  int format;
  int freq;
  char type[8]; // caf, mp3 ...etc
};
void od_free_info(struct oal_info* info);

void ad_error(const char* f, ...);
const char*  ad_last_error();

int ad_new_info(lua_State* L, struct oal_info* info);

int luaopen_oal_decode(lua_State* L);
#endif