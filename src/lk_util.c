#include "lk_util.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>


#ifdef __ANDROID__ 
#include "opensl/opensl.h"
#include <fcntl.h>
#include <sys/types.h>

struct util_fp {
    AAsset* asset;

    const unsigned char* data;
    off_t cur_pos;

    off_t length;
};

struct util_fp*
util_file_open(const char* path) {
    struct util_fp* ret = NULL;
    AAssetManager* mgr = sl_get_asset_mgr();
    AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_UNKNOWN);
    if(asset){
        ret = (struct util_fp*)malloc(sizeof(*ret));
        if(!ret)
            goto ERROR;
        ret->cur_pos = 0;
        ret->asset = asset;
        ret->data = AAsset_getBuffer(asset);
        ret->length = AAsset_getLength(asset);
        return ret;
    }

ERROR:
    if(ret) free(ret);
    return NULL;
}


void
util_file_close(struct util_fp* handle) {
    if(!handle)
        return;

    AAsset_close(handle->asset);
    free(handle);
}


size_t
util_file_size(struct util_fp* handle) {
    return (size_t)handle->length;
}

size_t
util_file_readall(struct util_fp* handle, unsigned char* buffer, size_t size) {
    size_t read_size = (size > handle->length)?(handle->length):(size);
    memcpy(buffer, handle->data, read_size);
    return read_size;
}

size_t
util_file_read(void* out_buffer, size_t size, size_t ntimes, struct util_fp* handle) {
    off_t des_pos = handle->cur_pos;
    des_pos += size * ntimes;
    des_pos = (des_pos > handle->length)?(handle->length):(des_pos);

    off_t n = des_pos - handle->cur_pos;
    memcpy(out_buffer, handle->data+des_pos, n);
    handle->cur_pos = des_pos;
    return n;
}


int
util_file_seek(struct util_fp* handle, long offset, int whence) {
    off_t  des_pos = 0;
    switch(whence) {
        case SEEK_CUR:
            des_pos = handle->cur_pos + offset;
            break;
        case SEEK_SET:
            des_pos = offset;
            break;
        case SEEK_END:
            des_pos = handle->length + offset;
            break;
        default:
            return -1;
    }

    if(des_pos < 0 || des_pos > handle->length)
        return -1;
    else {
        handle->cur_pos = des_pos;
        return 0;
    }
}

#else

struct util_fp {
    FILE* fp;
    size_t size;
};

struct util_fp*
util_file_open(const char* path) {
    if(path == NULL)
        return NULL;

    struct util_fp* ret = (struct util_fp*)malloc(sizeof(*ret));
    if(ret) {
        FILE* fp = fopen(path, "rb");
        if(!fp) 
            goto ERROR;
        fseek(fp, 0L, SEEK_END);
        ret->size = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        ret->fp = fp;
        return ret;
    }

ERROR:
    if(ret) free(ret);
    return NULL;
}

void
util_file_close(struct util_fp* handle) {
    printf("util_file_close!!!! %p\n", handle);
    if(!handle)
        return;

    fclose(handle->fp);
    free(handle);
}

size_t
util_file_size(struct util_fp* handle) {
    return handle->size;
}

size_t
util_file_readall(struct util_fp* handle, unsigned char* buffer, size_t size) {
    size_t read_size = (size>handle->size)?(handle->size):(size);
    size_t ret = fread(buffer, 1, read_size, handle->fp);
    assert(ret == read_size);
    return read_size;
}

size_t
util_file_read(void* out_buffer, size_t size, size_t ntimes, struct util_fp* handle) {
    FILE* fp = handle->fp;
    return fread(out_buffer, size, ntimes, fp);
}

int
util_file_seek(struct util_fp* handle, long offset, int whence) {
    if(handle ==NULL) return -1;

    #ifdef __MINGW32__
      return fseeko64(handle->fp, offset, whence);
    #elif defined (_WIN32)
      return _fseeki64(handle->fp, offset, whence);
    #else
      return fseek(handle->fp, offset, whence);
    #endif
}


#endif

