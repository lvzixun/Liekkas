#include "../oal_decode.h"

#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef SUPPORT_AUDIO_VORBIS
#include <vorbis/vorbisfile.h>

static bool
_decode_ogg(const char* filepath, struct oal_info* out) {
  FILE* fp = fopen(filepath, "r");
  if(!fp){
    fclose(fp);
    return false;
  }

  OggVorbis_File ogg_file;
  int status = ov_test(fp, &ogg_file, 0, 0);
  if(status != 0){
    fclose(fp);
    ov_clear(&ogg_file);
    ad_error("test decode ogg:%s error", filepath);
    return false;
  }

  status = ov_test_open(&ogg_file);
  if(status != 0) {
    fclose(fp);
    ad_error("test open ogg:%s error", filepath);
    return false;
  }

  // As vorbis documentation says, we should not fclose() file
  // after successful opening by vorbis functions.
  vorbis_info* info = ov_info(&ogg_file, -1);
  ALenum  format = (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

  size_t size = ov_pcm_total(&ogg_file, -1) * info->channels * 2;
  char* data = (char*)malloc(size);

  size_t sz = 0;
  int section = 0;
  while(sz < size) {
    status = ov_read(&ogg_file, data+sz, size-sz, 0, 2, 1, &section);
    if(status > 0){
      sz += status;
    }else if(status < 0) {
      ad_error("decode ogg: %s stop status[%d]", filepath, status);
      goto EXIT;
    }else {
      break;
    }
  }
  if(sz == 0) {
    ad_error("unabel to read ogg: %s", filepath);
    goto EXIT;
  }

  out->freq = info->rate;
  out->data = data;
  out->format = format;
  strcpy(out->type, "ogg");
  out->size = size;
  return true;

EXIT:
  if(data) free(data);
  return false;
}
#else

static bool
_decode_ogg(const char* filepath, struct oal_info* out) {
  ad_error("unspport decode ogg.");
  return false;
}
#endif



int
adl_decode_ogg(lua_State* L) {
  const char* file = lua_tostring(L, 1);
  struct oal_info out = {0};
  if(_decode_ogg(file, &out)){
    return ad_new_info(L, &out);
  } else {
    luaL_error(L, ad_last_error());
  }
  return 0;
}




