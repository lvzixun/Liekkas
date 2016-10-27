#ifndef _OPENSL_H_
#define _OPENSL_H_

#include <stdbool.h>
#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#define sl_log(...) __android_log_print(ANDROID_LOG_INFO, "ejoy", __VA_ARGS__)


AAssetManager* sl_get_asset_mgr();
bool sl_isinit();
void sl_pause();
void sl_resume();


enum sl_source_state {
    SL_SOURCE_INVALID,

    SL_SOURCE_STOP,
    SL_SOURCE_PLAY,
    SL_SOURCE_PAUSE,
};

struct sl_source;
struct sl_buffer;


struct sl_source* sl_source_gen();
void sl_source_free(struct sl_source* source);
bool sl_source_bind(struct sl_source* source, struct sl_buffer* buffer);
void sl_source_loop(struct sl_source* source, bool is_loop);
void sl_source_clear(struct sl_source* source);

bool sl_source_volume(struct sl_source* source, float volume);
bool sl_source_pitch(struct sl_source* source, float pitch);
bool sl_source_set_state(struct sl_source* source, enum sl_source_state op);
enum sl_source_state sl_source_get_state(struct sl_source* source);

struct sl_buffer* sl_buffer_gen();
void sl_buffer_data(struct sl_buffer* buffer, const void* data, size_t size);

void sl_buffer_free(struct sl_buffer* buffer);

bool sl_bgm_load(const char* file_name);
void sl_bgm_play(bool is_loop);
void sl_bgm_stop();
void sl_bgm_pause();
bool sl_bgm_volume(float volume);

#endif