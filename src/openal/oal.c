#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <stdio.h>

#include <stdbool.h>
#include <lua.h>
#include <lauxlib.h>

#include "../lk_decode.h"

#define OAL_RATE_HIGH 44100
#define OAL_RATE_MID  22050
#define OAL_RATE_LOW  16000
#define OAL_RATE_BASIC 8000
#define OAL_RATE_DEFAULT 44100

struct _oal_state {
  ALCcontext* context;
  bool is_load;
} OAL_STATE = {0};


#define CHECK_ERROR(tag) do{ \
                            int error = alGetError(); \
                            if(error != AL_NO_ERROR) { \
                              luaL_error(L, "%s error:%d\n", tag, error); \
                            } \
                          }while(0)


#ifdef _MSC_VER
typedef ALvoid (*alcMacOSXMixerOutputRateProcPtr) (const ALdouble value);
#else
typedef ALvoid  AL_APIENTRY (*alcMacOSXMixerOutputRateProcPtr) (const ALdouble value);
#endif // _MSC_VER


// --------------------  openal state  --------------------
static ALvoid  
alcMacOSXMixerOutputRateProc(const ALdouble value) {
  static  alcMacOSXMixerOutputRateProcPtr proc = NULL;
  if (proc == NULL) {
    proc = (alcMacOSXMixerOutputRateProcPtr) alcGetProcAddress(NULL, (const ALCchar*) "alcMacOSXMixerOutputRate");
  }

  if (proc)
    proc(value);

  return;
}


static void
_init_openal(lua_State* L) {
  if(OAL_STATE.is_load)
    return;

  alcMacOSXMixerOutputRateProc(OAL_RATE_DEFAULT);


  ALCcontext* context = NULL;
  ALCdevice* new_device = NULL;
  // Create a new OpenAL Device
  // Pass NULL to specify the system's default output device
  new_device = alcOpenDevice(NULL);
  if(new_device) {
    // Create a new OpenAL Context
    // The new context will render to the OpenAL Device just created
    context = alcCreateContext(new_device, 0);
    if(context){
      // Make the new context the Current OpenAL Context
      alcMakeContextCurrent(context);
    }else {
      luaL_error(L, "openal context error");
    }
  }else {
    luaL_error(L, "no device");
  }

  OAL_STATE.is_load = true;
  OAL_STATE.context = context;
}


// maybe never call
static void
_free_oal() {
  if(OAL_STATE.is_load) {
    ALCcontext  *context = OAL_STATE.context;
    ALCdevice *device = alcGetContextsDevice(context); 

    alcDestroyContext(context);
    alcCloseDevice(device);
  }
}


void
oal_interrupted() {
  if(OAL_STATE.is_load) {
    alcMakeContextCurrent(NULL);
  }
}

void
oal_resumed() {
  if(OAL_STATE.is_load) {
    alcMakeContextCurrent(OAL_STATE.context);
  }
}

static int
_id2string(lua_State* L) {
  ALuint id = *((ALuint*)lua_touserdata(L, 1));
  char buffer[24] = {0};
  sprintf(buffer, "%d", id);
  lua_pushstring(L, buffer);
  return 1;
}

static int
l_listen_position(lua_State* L) {
  lua_Number x = lua_tonumber(L, 1);
  lua_Number y = lua_tonumber(L, 2);
  lua_Number z = lua_tonumber(L, 3);

  alListener3f(AL_POSITION, x, y, z);
  CHECK_ERROR("listen_position");
  return 0;
}

static int
l_set(lua_State* L) {
  ALuint source_id = *((ALuint*)lua_touserdata(L, 1));
  ALuint buffer_id = *((ALuint*)lua_touserdata(L, 2));
  lua_Number pitch = lua_tonumber(L, 3);
  lua_Number max_distance = lua_tonumber(L, 4);
  lua_Number gain = lua_tonumber(L, 5);
  int loop = lua_toboolean(L, 6);

  ALint state;
  alGetSourcei(source_id, AL_SOURCE_STATE, &state);
  CHECK_ERROR("oal_set getsource");

  alSourceStop(source_id);
  CHECK_ERROR("oal_set sourcestop");

  alSourcef(source_id, AL_PITCH, pitch);
  CHECK_ERROR("oal_set pitch");

  alSourcei(source_id, AL_LOOPING, loop);
  CHECK_ERROR("oal_set loop");

  alSourcef(source_id, AL_GAIN, gain);
  CHECK_ERROR("oal_set gain");

  alSourcei(source_id, AL_MAX_DISTANCE, max_distance);
  CHECK_ERROR("oal_set max_distance");

  alSourcei(source_id, AL_BUFFER, buffer_id);
  CHECK_ERROR("oal_set");
  return 0;
}


// --------------------  audio source  --------------------
static int
l_volume_source(lua_State* L) {
  ALuint source_id = *((ALuint*)lua_touserdata(L, 1));
  lua_Number v = lua_tonumber(L, 2);

  alSourcef(source_id, AL_GAIN, v);
  CHECK_ERROR("volume_source");
  return 0;
}

static int
l_position_source(lua_State* L) {
  ALuint source_id = *((ALuint*)lua_touserdata(L, 1));
  lua_Number x = lua_tonumber(L, 2);
  lua_Number y = lua_tonumber(L, 3);
  lua_Number z = lua_tonumber(L, 4);

  alSource3f(source_id, AL_POSITION, x, y, z);
  CHECK_ERROR("position_source");
  return 0;
}


static int
l_state_source(lua_State* L) {
  ALuint source_id = *((ALuint*)lua_touserdata(L, 1));
  ALint state;
  alGetSourcei(source_id, AL_SOURCE_STATE, &state);
  CHECK_ERROR("state_source");
  lua_pushinteger(L, state);
  return 1;
}

static int
l_play_source(lua_State* L) {
  ALuint source_id = *((ALuint*)lua_touserdata(L, 1));
  alSourcePlay(source_id);
  CHECK_ERROR("play_source");
  return 0;
}


static int
l_pause_source(lua_State* L) {
  ALuint source_id = *((ALuint*)lua_touserdata(L, 1));
  alSourcePause(source_id);
  CHECK_ERROR("pause_source");
  return 0;
}

static int
l_rewind_source(lua_State* L) {
  ALuint source_id = *((ALuint*)lua_touserdata(L, 1));
  alSourceRewind(source_id);
  CHECK_ERROR("rewind_source");
  return 0;
}

static int
l_stop_source(lua_State* L) {
  ALuint source_id = *((ALuint*)lua_touserdata(L, 1));
  alSourceStop(source_id);
  CHECK_ERROR("stop_source");
  return 0;
}


static int
l_clear_source(lua_State* L) {
  ALuint source_id = *((ALuint*)lua_touserdata(L, 1));
  alSourceStop(source_id);
  alSourcei(source_id, AL_BUFFER, 0);
  CHECK_ERROR("clear_source");
  return 0;
}

static int
l_free_source(lua_State* L) {
  ALuint source_id = *((ALuint*)lua_touserdata(L, 1));
  alSourceStop(source_id);
  alSourcei(source_id, AL_BUFFER, 0);
  alDeleteSources(1, &source_id);
  CHECK_ERROR("free_source");
  return 0;
}


static int
_new_source(lua_State* L, ALuint source_id) {
  ALuint* p = (ALuint*)lua_newuserdata(L, sizeof(source_id));
  *p = source_id;
  if(luaL_newmetatable(L, "mt_source")) {
    lua_pushcfunction(L, l_free_source);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, _id2string);
    lua_setfield(L, -2, "__tostring");

    luaL_Reg l[] = {
      {"clear", l_clear_source},
      {"play", l_play_source},
      {"stop", l_stop_source},
      {"rewind", l_rewind_source},
      {"pause", l_pause_source},
      {"state", l_state_source},
      {"volume", l_volume_source},
      {"position", l_position_source},
      {NULL, NULL},
    };
    luaL_newlib(L, l);
    lua_setfield(L, -2, "__index");
  }

  lua_setmetatable(L, -2);
  return 1;
}


static int
l_create_source(lua_State* L) {
  ALuint source_id;
  alGenSources(1, &source_id);
  CHECK_ERROR("create_source");

  alSourcei(source_id, AL_BUFFER, 0);
  CHECK_ERROR("set_source_null_buffer");  
  
  return _new_source(L, source_id);
}

static void
_source_state(lua_State* L) {
  struct {
    ALint t;
    const char*  s;
  }v[] = {
    { AL_PLAYING, "playing"},
    { AL_INITIAL, "initial"},
    { AL_STOPPED, "stopped"},
    { AL_PAUSED, "paused"},
  };

  lua_newtable(L);
  int i;
  for(i=0; i<sizeof(v) / sizeof(v[0]); i++) {
    lua_pushstring(L, v[i].s);
    lua_pushinteger(L, v[i].t);
    lua_settable(L, -3);
  }
}

// --------------------  aduio buffer  --------------------

static int
l_free_buffer(lua_State* L) {
  ALuint buffer_id = *((ALuint*)lua_touserdata(L, 1));
  alDeleteBuffers(1, &buffer_id);
  CHECK_ERROR("free_buffer");
  return 0;
}


static int
_new_buffer(lua_State* L, ALuint buffer_id) {
  ALuint* p = (ALuint*)lua_newuserdata(L, sizeof(buffer_id));
  *p = buffer_id;
  if(luaL_newmetatable(L, "mt_buffer")) {
    lua_pushcfunction(L, l_free_buffer);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, _id2string);
    lua_setfield(L, -2, "__tostring");
  }

  lua_setmetatable(L, -2);
  return 1;
}

static int
l_create_buffer(lua_State* L) {
  ALuint buffer_id;
  alGenBuffers(1, &buffer_id);
  CHECK_ERROR("create_buffer");
  
  return _new_buffer(L, buffer_id);
}

static int
l_bind_buffer(lua_State* L) {
  ALuint buffer_id = *((ALuint*)lua_touserdata(L, 1));
  struct oal_info* info = (struct oal_info*)lua_touserdata(L, 2);

  alBufferData(buffer_id, info->format, info->data, info->size, info->freq);
  CHECK_ERROR("bind_buffer");

  return 0;
}


// --------------------  lua wraper  --------------------

int
luaopen_liekkas(lua_State* L) {
  unused(_free_oal);  // unused

  _init_openal(L);
  
  // set lib
  luaL_checkversion(L);
  luaL_Reg l[] = {
    {"create_source", l_create_source},
    {"create_buffer", l_create_buffer},
    {"buffer_bind", l_bind_buffer},
    {"source_set", l_set},
    {"listen_position", l_listen_position},
    {NULL, NULL},
  };

  luaL_newlib(L, l);

  // set source state
  _source_state(L);
  lua_setfield(L, -2, "source_state");

  return 1;
}

