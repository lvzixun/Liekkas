#include "../lk_decode.h"

#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef SUPPORT_AUDIO_VORBIS
#include <vorbis/vorbisfile.h>

static bool
_decode_ogg(const char* filepath, struct oal_info* out) {
  struct util_fp* fp = util_file_open(filepath);
  if(!fp){
    util_file_close(fp);
    return false;
  }

  OggVorbis_File ogg_file;
  ov_callbacks callbacks = {
    (size_t (*)(void *, size_t, size_t, void *))  util_file_read,
    (int (*)(void *, ogg_int64_t, int))           util_file_seek,
    (int (*)(void *))                             util_file_close,
    (long (*)(void *))                            util_file_size,
  };
  int status = ov_open_callbacks(fp, &ogg_file, 0, 0, callbacks);
  if(status != 0){
    util_file_close(fp);
    ov_clear(&ogg_file);
    ad_error("open ogg:%s error", filepath);
    return false;
  }

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
  if(fp) util_file_close(fp);
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




