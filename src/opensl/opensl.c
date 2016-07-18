#include <assert.h>
#include <string.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "opensl.h"

struct sl_buffer {
    const unsigned char* pcm_buffer;
    size_t pcm_size;
};


struct sl_source {
    SLObjectItf bqPlayerObject;
    SLPlayItf bqPlayerPlay;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
    SLVolumeItf bqPlayerVolume;
    SLPitchItf  bqPlayerPitch;

    bool is_loop;
    struct sl_buffer* buffer;
};


struct sl_bgm {
    SLObjectItf fdPlayerObject;
    SLPlayItf fdPlayerPlay;
    SLVolumeItf fdPlayerVolume;
    SLSeekItf fdPlayerSeek;

    char* file_name;
};

struct source_buffer {
    struct sl_source* raw[2];
    struct sl_source** p;
    size_t cap;
    size_t len;
};

static struct {
    SLObjectItf engineObject;
    SLEngineItf engineEngine;

    SLObjectItf outputMixObject;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb;
    SLEnvironmentalReverbSettings reverbSettings;

    bool _is_init;
    SLresult last_error_code;

    struct sl_bgm bgm;

    struct source_buffer source_record;

    AAssetManager* asset_mgr;
}ENV = {0};


#define _check_result(result) do { \
                                if(result != SL_RESULT_SUCCESS) \
                                    goto ERROR; \
                              }while(0)

#define _return_result(exp) do { \
                              SLresult ret = (exp); \
                              if(ret != SL_RESULT_SUCCESS) { \
                                ENV.last_error_code = ret; \
                                return false; \
                              } \
                          }while(0); \
                          return true


static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);
static void  sl_destory();
static bool  sl_init();
static void _bgm_free(struct sl_bgm* bgm);

static void
source_buffer_init(struct source_buffer* sp) {
    assert(sp->p==NULL);
    sp->p = sp->raw;
    sp->len = 0;
    sp->cap = sizeof(sp->raw) / sizeof(sp->raw[0]);
}

static void
source_buffer_free(struct source_buffer* sp) {
    if(sp->p && sp->p != sp->raw) {
        free(sp->p);
    }
    memset(sp, 0, sizeof(*sp));
}


static void
source_buffer_add(struct source_buffer* sp, struct sl_source* v) {
    if(sp->cap <= sp->len) {
        size_t new_cap = sp->cap*2;
        if(sp->p == sp->raw) {
            sp->p = malloc(new_cap*sizeof(struct sl_source*));
            memcpy(sp->p, sp->raw, sizeof(sp->raw));
        }else {
            sp->p = realloc(sp->p, new_cap*sizeof(struct sl_source*));
        }
        sp->cap = new_cap;
    }

    sp->p[sp->len++] = v;
}

static void
source_buffer_remove(struct source_buffer* sp, struct sl_source* v) {
    size_t i=0;
    size_t idx = sp->len;

    if(sp->cap == 0)
        return;

    for(i; i<sp->len; i++) {
        if(sp->p[i] == v) {
            idx = i;
            sp->len--;
            break;
        }
    }

    for(i=idx; i<sp->len; i++) {
        sp->p[i] = sp->p[i+1];
    }
}


// ------------------  opensl env -------------------------
static bool 
sl_init () {
    if(ENV._is_init)
        return true;

    SLresult result;

    memset(&ENV, 0, sizeof(ENV));

    source_buffer_init(&ENV.source_record);

    ENV.last_error_code = SL_RESULT_SUCCESS;
    SLEnvironmentalReverbSettings v = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
    ENV.reverbSettings = v;

    // create engine
    result = slCreateEngine(&ENV.engineObject, 0, NULL, 0, NULL, NULL);
    _check_result(result);

    // realize the engine
    result = (*ENV.engineObject)->Realize(ENV.engineObject, SL_BOOLEAN_FALSE);
    _check_result(result);

    // get the engine interface, which is needed in order to create other objects
    result = (*ENV.engineObject)->GetInterface(ENV.engineObject, SL_IID_ENGINE, &ENV.engineEngine);
    _check_result(result);

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*ENV.engineEngine)->CreateOutputMix(ENV.engineEngine, &ENV.outputMixObject, 1, ids, req);
    _check_result(result);

    // realize the output mix
    result = (*ENV.outputMixObject)->Realize(ENV.outputMixObject, SL_BOOLEAN_FALSE);
    _check_result(result);

    result = (*ENV.outputMixObject)->GetInterface(ENV.outputMixObject, SL_IID_ENVIRONMENTALREVERB,
            &ENV.outputMixEnvironmentalReverb);
    _check_result(result);

    if (SL_RESULT_SUCCESS == result) {
        result = (*ENV.outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                ENV.outputMixEnvironmentalReverb, &ENV.reverbSettings);
        _check_result(result);
    }


    ENV._is_init = true;
    return true;

ERROR:
    ENV._is_init = true;
    sl_destory();
    return false;
}

void
sl_pause() {
    size_t i=0;
    struct source_buffer* sp;
    if(!ENV._is_init)
        return;

    sp = &ENV.source_record;
    for(i; i<sp->len; i++) {
        struct sl_source* v = sp->p[i];
        if(sl_source_get_state(v) == SL_SOURCE_PLAY) {
            sl_source_set_state(v, SL_SOURCE_PAUSE);
        }
    }
}

void
sl_resume() {
    size_t i=0;
    struct source_buffer* sp;
    if(!ENV._is_init)
        return;

    sp = &ENV.source_record;
    for(i; i<sp->len; i++) {
        struct sl_source* v = sp->p[i];
        if(sl_source_get_state(v) == SL_SOURCE_PAUSE) {
            sl_source_set_state(v, SL_SOURCE_PLAY);
        }
    }   
}

AAssetManager*
sl_get_asset_mgr() {
    return (ENV._is_init)?(ENV.asset_mgr):(NULL);
}

bool
sl_isinit() {
    return ENV._is_init;
}

static void 
sl_destory() {
    if(ENV._is_init){
        if(ENV.outputMixObject) {
            (*ENV.outputMixObject)->Destroy(ENV.outputMixObject);
            ENV.outputMixObject = NULL;
        }

        if(ENV.engineObject) {
            (*ENV.engineObject)->Destroy(ENV.engineObject);
            ENV.engineEngine = NULL;
            ENV.engineObject = NULL;
        }

        source_buffer_free(&ENV.source_record);
        _bgm_free(&ENV.bgm);
        ENV._is_init = false;
    }
}


int 
sl_get_last_errcode() {
    return ENV.last_error_code;
}

// ------------------  opensl buffer -------------------------

struct sl_buffer*
sl_buffer_gen() {
    struct sl_buffer* ret = (struct sl_buffer*)malloc(sizeof(*ret));
    if(!ret)
        return NULL;

    ret->pcm_buffer = NULL;
    ret->pcm_size = 0;
    return ret;
}

void 
sl_buffer_data(struct sl_buffer* buffer, const void* data, size_t size) {
    if(buffer->pcm_buffer)
        free((void*)buffer->pcm_buffer);

    unsigned char* p = (unsigned char*)malloc(size);
    if(!p)
        return;

    buffer->pcm_buffer = p;
    memcpy((void*)p, data, size);
    buffer->pcm_size = size;
}


void
sl_buffer_free(struct sl_buffer* p) {
    if(p) {
        free((void*)p->pcm_buffer);
        free(p);
    }
}


// ------------------  opensl source -------------------------

struct sl_source*
sl_source_gen() {
    struct sl_source* ret = (struct sl_source*)malloc(sizeof(*ret));
    if(!ret)
        return NULL;

    SLresult result;

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, ENV.outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
    sl_log("sl_source_gen!!!! begin!!!\n");
    result = (*ENV.engineEngine)->CreateAudioPlayer(ENV.engineEngine, &ret->bqPlayerObject, &audioSrc, &audioSnk,
            3, ids, req);
    sl_log("sl_source_gen!!!! end : %d!!!\n", result);
    _check_result(result);

    // realize the player
    result = (*ret->bqPlayerObject)->Realize(ret->bqPlayerObject, SL_BOOLEAN_FALSE);
    _check_result(result);


    // get the play interface
    result = (*ret->bqPlayerObject)->GetInterface(ret->bqPlayerObject, SL_IID_PLAY, &ret->bqPlayerPlay);
    _check_result(result);

    // get the buffer queue interface
    result = (*ret->bqPlayerObject)->GetInterface(ret->bqPlayerObject, SL_IID_BUFFERQUEUE,
            &ret->bqPlayerBufferQueue);
    _check_result(result);

    // register callback on the buffer queue
    result = (*ret->bqPlayerBufferQueue)->RegisterCallback(ret->bqPlayerBufferQueue, bqPlayerCallback, (void*)ret);
    _check_result(result);

    // get the volume interface
    result = (*ret->bqPlayerObject)->GetInterface(ret->bqPlayerObject, SL_IID_VOLUME, &ret->bqPlayerVolume);
    _check_result(result);

    // get the pitch interface, may be feature unsupported
    result = (*ret->bqPlayerObject)->GetInterface(ret->bqPlayerObject, SL_IID_PITCH, &ret->bqPlayerPitch);

    // set the player's state to playing
    result = (*ret->bqPlayerPlay)->SetPlayState(ret->bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    _check_result(result);

    source_buffer_add(&ENV.source_record, ret);
    return ret;

ERROR:
    sl_source_free(ret);
    return NULL;
}



void
sl_source_free(struct sl_source* source) {
    if(source) {
        source_buffer_remove(&ENV.source_record, source);
        if(source->bqPlayerObject) {
            (*source->bqPlayerObject)->Destroy(source->bqPlayerObject);
        }
        free(source);
    }
}


static bool
_source_enqueue(struct sl_source* source) {
    SLresult result;

    _return_result(
        (*source->bqPlayerBufferQueue)->Enqueue(source->bqPlayerBufferQueue, 
            source->buffer->pcm_buffer, source->buffer->pcm_size)
        );
}

bool
sl_source_bind(struct sl_source* source, struct sl_buffer* buffer) {
    source->buffer = buffer;
    return _source_enqueue(source);
}


void
sl_source_clear(struct sl_source* source) {
    sl_source_set_state(source, SL_SOURCE_STOP);
    (*source->bqPlayerBufferQueue)->Clear(source->bqPlayerBufferQueue);
    source->buffer = NULL;
    source->is_loop = false;
}


void
sl_source_loop(struct sl_source* source, bool is_loop) {
    source->is_loop = is_loop;
}

bool
sl_source_set_state(struct sl_source* source, enum sl_source_state op) {
    switch(op) {
        case SL_SOURCE_STOP:
            _return_result((*source->bqPlayerPlay)->SetPlayState(source->bqPlayerPlay, SL_PLAYSTATE_STOPPED));
            break;
        case SL_SOURCE_PLAY:
            _return_result((*source->bqPlayerPlay)->SetPlayState(source->bqPlayerPlay, SL_PLAYSTATE_PLAYING));
            break;
        case SL_SOURCE_PAUSE:
            _return_result((*source->bqPlayerPlay)->SetPlayState(source->bqPlayerPlay, SL_PLAYSTATE_PAUSED));
            break;
        default:
            return false;
    }
    return false;
}


enum sl_source_state
sl_source_get_state(struct sl_source* source) {
    SLuint32 state;
    SLresult result = (*source->bqPlayerPlay)->GetPlayState(source->bqPlayerPlay, &state);
    if(result != SL_RESULT_SUCCESS)
        return SL_SOURCE_INVALID;

    switch(state) {
        case SL_PLAYSTATE_STOPPED:
            return SL_SOURCE_STOP;
        case SL_PLAYSTATE_PAUSED:
            return SL_SOURCE_PAUSE;
        case SL_PLAYSTATE_PLAYING:
            return SL_SOURCE_PLAY;
        default:
            return SL_SOURCE_INVALID;
    }
    return SL_SOURCE_INVALID;
}


bool
sl_source_volume(struct sl_source* source, float volume) {
    SLmillibel max;
    SLresult result = (*source->bqPlayerVolume)->GetMaxVolumeLevel(source->bqPlayerVolume, &max);
    if(result == SL_RESULT_SUCCESS) {
        SLmillibel level = max*volume;
         _return_result((*source->bqPlayerVolume)->SetVolumeLevel(source->bqPlayerVolume, level));
    }
    return false;
}


bool
sl_source_pitch(struct sl_source* source, float pitch) {
    if(source->bqPlayerPitch) {
        SLpermille max, min;
        SLresult reslut = (*source->bqPlayerPitch)->GetPitchCapabilities(source->bqPlayerPitch, &max, &min);
        if(reslut == SL_RESULT_SUCCESS) {
            SLpermille v = (SLpermille)((max - min)*pitch);
            v = min + v;
            v = (v > max)?(max):(v);
            _return_result((*source->bqPlayerPitch)->SetPitch(source->bqPlayerPitch, v));
        }
    }
    return false;    
}

static void
bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    sl_log("bqPlayerCallback!!!\n");
    struct sl_source* source = (struct sl_source*)context;
    if(source->is_loop) {
        sl_log("bqPlayerCallback!!! loop it!!!\n");
        _source_enqueue(source);
    }
}

// -------------------- for background music --------------------
static void
_bgm_free(struct sl_bgm* bgm) {
    if(bgm->file_name) {
        free(bgm->file_name);
        bgm->file_name = NULL;
    }

    if(bgm->fdPlayerObject) {
        (*bgm->fdPlayerObject)->Destroy(bgm->fdPlayerObject);
        bgm->fdPlayerObject = NULL;
    }
}

static inline bool
_bgm_is_init() {
    return ENV._is_init && ENV.bgm.file_name && ENV.bgm.fdPlayerObject;
}


bool
sl_bgm_load(const char* file_name) {
    if(!file_name) {
        return false;
    }

    struct sl_bgm* bgm = &(ENV.bgm);
    if(bgm->file_name && strcmp(file_name, bgm->file_name) == 0) {
        SLresult result = (*bgm->fdPlayerPlay)->SetPlayState(bgm->fdPlayerPlay, SL_PLAYSTATE_STOPPED);
        return result == SL_RESULT_SUCCESS;
    }

    _bgm_free(bgm);
    bgm->file_name = strdup(file_name);
    if(bgm->file_name == NULL) {
        return false;
    }

    AAsset* asset = AAssetManager_open(ENV.asset_mgr, file_name, AASSET_MODE_UNKNOWN);
    if(asset == NULL) {
        goto ERROR;
    }

    // open asset as file descriptor
    off_t start, length;
    int fd = AAsset_openFileDescriptor(asset, &start, &length);
    AAsset_close(asset);
    if(fd < 0) {
        goto ERROR;
    }


    // configure audio source
    SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
    SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
    SLDataSource audioSrc = {&loc_fd, &format_mime};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, ENV.outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    SLresult result;
    const SLInterfaceID ids[3] = {SL_IID_SEEK, SL_IID_MUTESOLO, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*ENV.engineEngine)->CreateAudioPlayer(ENV.engineEngine, &bgm->fdPlayerObject, &audioSrc, &audioSnk,
            3, ids, req);
    _check_result(result);


    // realize the player
    result = (*bgm->fdPlayerObject)->Realize(bgm->fdPlayerObject, SL_BOOLEAN_FALSE);
    _check_result(result);

    // get the play interface
    result = (*bgm->fdPlayerObject)->GetInterface(bgm->fdPlayerObject, SL_IID_PLAY, &bgm->fdPlayerPlay);
    _check_result(result);

    // get the seek interface
    result = (*bgm->fdPlayerObject)->GetInterface(bgm->fdPlayerObject, SL_IID_SEEK, &bgm->fdPlayerSeek);
    _check_result(result);

    // get the volume interface
    result = (*bgm->fdPlayerObject)->GetInterface(bgm->fdPlayerObject, SL_IID_VOLUME, &bgm->fdPlayerVolume);
    _check_result(result);

    // enable whole file looping
    result = (*bgm->fdPlayerSeek)->SetLoop(bgm->fdPlayerSeek, SL_BOOLEAN_FALSE, 0, SL_TIME_UNKNOWN);
    _check_result(result);

    // set bgm stop
    result = (*bgm->fdPlayerPlay)->SetPlayState(bgm->fdPlayerPlay, SL_PLAYSTATE_STOPPED);
    _check_result(result);

    return true;

ERROR:
    _bgm_free(bgm);
    return false;
}

void
sl_bgm_play(bool is_loop) {
    struct sl_bgm* bgm = &ENV.bgm;
    if(_bgm_is_init()) {
        (*bgm->fdPlayerSeek)->SetLoop(bgm->fdPlayerSeek, 
            (is_loop)?(SL_BOOLEAN_TRUE):(SL_BOOLEAN_FALSE), 0, SL_TIME_UNKNOWN);
        (*bgm->fdPlayerPlay)->SetPlayState(bgm->fdPlayerPlay, SL_PLAYSTATE_STOPPED);
        (*bgm->fdPlayerPlay)->SetPlayState(bgm->fdPlayerPlay, SL_PLAYSTATE_PLAYING);
    }
}

void
sl_bgm_stop() {
    struct sl_bgm* bgm = &ENV.bgm;
    if(_bgm_is_init()) {
        (*bgm->fdPlayerPlay)->SetPlayState(bgm->fdPlayerPlay, SL_PLAYSTATE_STOPPED);
    }
}


void
sl_bgm_pause() {
    struct sl_bgm* bgm = &ENV.bgm;
    if(_bgm_is_init()) {
        (*bgm->fdPlayerPlay)->SetPlayState(bgm->fdPlayerPlay, SL_PLAYSTATE_PAUSED);
    }
}

// -------------------- for JNI --------------------
JNIEXPORT jboolean JNICALL
Java_com_liekkas_Liekkas_engineInit(JNIEnv* env, jclass clazz, jobject assetManager) {
    bool ret = sl_init();
    ENV.asset_mgr = AAssetManager_fromJava(env, assetManager);
    return (ret)?(JNI_TRUE):(JNI_FALSE);
}


JNIEXPORT void JNICALL
Java_com_liekkas_Liekkas_engineDestory(JNIEnv* env, jclass clazz) {
    sl_destory();
}


JNIEXPORT void JNICALL
Java_com_liekkas_Liekkas_enginePause(JNIEnv* env, jclass clazz) {
    sl_pause();
}


JNIEXPORT void JNICALL
Java_com_liekkas_Liekkas_engineResume(JNIEnv* env, jclass clazz) {
    sl_resume();
}

