#ifndef _LK_UTIL_H_
#define _LK_UTIL_H_

#include <stdlib.h>

struct util_fp;
struct util_fp* util_file_open(const char* path);
void util_file_close(struct util_fp* handle);
size_t util_file_size(struct util_fp* handle);

size_t util_file_readall(struct util_fp* handle, unsigned char* buffer, size_t size);
size_t util_file_read(void* out_buffer, size_t size, size_t ntimes, struct util_fp* handle);
int util_file_seek(struct util_fp* handle, long offset, int whence);


#ifdef __ANDROID__

// the formart is not need used at android
#define FORMAT_MONO8      1
#define FORMAT_STEREO8    2
#define FORMAT_MONO16     3
#define FORMAT_STEREO16   4

#else

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#define FORMAT_MONO8      AL_FORMAT_MONO8
#define FORMAT_STEREO8    AL_FORMAT_STEREO8
#define FORMAT_MONO16     AL_FORMAT_MONO16
#define FORMAT_STEREO16   AL_FORMAT_STEREO16

#endif

#endif