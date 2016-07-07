#include "../lk_decode.h"
#include "../lk_util.h"

#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


#ifdef SUPPORT_AUDIO_MP3
#include <mpg123.h>

static struct oal_info*
_get_info(mpg123_handle* handle, struct oal_info* out_info) {
  int channels = 0;
  int encoding = 0;
  long rate = 0;
  int err = mpg123_getformat(handle, &rate, &channels, &encoding);
  if (MPG123_OK != err) {
    ad_error("mpg123_getformat error status[%d]", err);
    return NULL;
  }

  int freq = rate;
  int format;
  if (encoding == MPG123_ENC_UNSIGNED_8) {
    format = (channels==1)?(FORMAT_MONO8):(FORMAT_STEREO8);
  } else {
    format = (channels==1)?(FORMAT_MONO16):(FORMAT_STEREO16);
  }

  out_info->format = format;
  out_info->freq =freq;
  return out_info;
}

static mpg123_handle*
_get_handle() {
  static mpg123_handle* handle = NULL;
  if(!handle) {
    mpg123_init();
    int err=0;
    handle = mpg123_new(NULL, &err);
    if(MPG123_OK != mpg123_format(handle, 44100, MPG123_MONO | MPG123_STEREO,
      MPG123_ENC_UNSIGNED_8 | MPG123_ENC_SIGNED_16)) {
      mpg123_delete(handle);
      handle = NULL;
    }
  }
  return handle;
}

static void*
_read(mpg123_handle* handle, size_t size, size_t *out_done) {
  unsigned char* out_buffer = malloc(size);
  unsigned char* head = out_buffer;
  size_t head_sz = size;
  size_t read_sz = 0;

  *out_done = 0;
  do {
    int err = mpg123_decode(handle, NULL, 0, head, head_sz, &read_sz);
    switch(err) {
      case MPG123_ERR:
        goto EXIT;
      case MPG123_NEED_MORE:
        *out_done += read_sz;
        return out_buffer;
      default:
        *out_done += read_sz;
        size = size*2;
        out_buffer = (unsigned char*)realloc(out_buffer, size);
        head = out_buffer + *out_done;
        head_sz = size - *out_done;
        if(!out_buffer)
          goto EXIT;
        break;
    }
  }while(true);

EXIT:
  if(out_buffer) free(out_buffer);
  return NULL;
}

static bool
_decode_mp3(const char* filepath, struct oal_info* out) {
  bool ret = false;
  unsigned char* in_buffer = NULL;

  mpg123_handle* handle = _get_handle();
  if(!handle){
    ad_error("cannot set specified mpg123 format, file: %s", filepath);
    return false;
  }

  struct util_fp* util_handle = util_file_open(filepath);

  if(!util_handle || MPG123_OK != mpg123_open_feed(handle)) {
    ad_error("open file: %s error.", filepath);
    goto EXIT;
  }

  size_t sz = util_file_size(util_handle);
  in_buffer = (unsigned char*)malloc(sz);
  if(!in_buffer) {
    ad_error("prepare in buffer error.");
    goto EXIT;
  }

  util_file_readall(util_handle, in_buffer, sz);
  if(MPG123_OK != mpg123_feed(handle, (const unsigned char*)in_buffer, sz)) {
    ad_error("set feed: %s error.", filepath);
    goto EXIT;
  }

  if(!_get_info(handle, out)) {
    mpg123_close(handle);
    goto EXIT;
  }

  size_t size = sz;
  if(out->format == FORMAT_MONO16 || out->format == FORMAT_STEREO16) {
    size *= 2;
  }

  size_t done = 0;
  unsigned char* buffer = _read(handle, size, &done);
  if(!buffer) {
    ad_error("mpg123_read error: %s", filepath);
    goto EXIT;
  }else {
    strcpy(out->type, "mp3");
    out->data = buffer;
    out->size = done;    
  }

  ret = true;

EXIT:
  mpg123_close(handle);
  if(in_buffer) free(in_buffer);
  if(util_handle) util_file_close(util_handle);
  return ret;
}

#else
static bool
_decode_mp3(const char* filepath, struct oal_info* out) {
  ad_error("mp3 not support");
  return false;
}
#endif


int
adl_decode_mp3(lua_State* L) {
  const char* file = lua_tostring(L, 1);
  struct oal_info out = {0};
  if(_decode_mp3(file, &out)){
    return ad_new_info(L, &out);
  } else {
    luaL_error(L, ad_last_error());
  }
  return 0;
}




