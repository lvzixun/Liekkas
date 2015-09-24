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


static struct {
    SLObjectItf engineObject;
    SLEngineItf engineEngine;

    SLObjectItf outputMixObject;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb;
    SLEnvironmentalReverbSettings reverbSettings;

    bool _is_init;
    SLresult last_error_code;

    AAssetManager* asset_mgr;
}ENV;



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
// ------------------  opensl env -------------------------
static bool 
sl_init () {
    if(ENV._is_init)
        return true;

    SLresult result;

    memset(&ENV, 0, sizeof(ENV));

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
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
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
    result = (*ENV.engineEngine)->CreateAudioPlayer(ENV.engineEngine, &ret->bqPlayerObject, &audioSrc, &audioSnk,
            3, ids, req);
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

    return ret;

ERROR:
    sl_source_free(ret);
    return NULL;
}



void
sl_source_free(struct sl_source* source) {
    if(source) {
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

