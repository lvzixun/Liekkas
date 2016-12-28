#include "lua.h"
#include "lauxlib.h"
#include "../lk_decode.h"

#include <stdbool.h>

#import <AVFoundation/AVFoundation.h>

@class ADAudioSource;
static ADAudioSource* _instance = nil;

@interface ADAudioSource : NSObject <AVAudioPlayerDelegate> {
  AVAudioPlayer* _source_player;
  NSString* _source_filepath;
  float _volume;
  bool _loop;
  bool _isplaying;
}

@property (readonly) bool isload;
@end


static void setMode(NSString* mode) {
    NSError* error = nil;
    [[AVAudioSession sharedInstance] setCategory:mode error:&error];
    if(error) {
        NSLog(@"setmode error: %@", error);
    }
}


@implementation ADAudioSource

-(id) init {
  if((self = [super init])) {
    _source_filepath = nil;
    _source_player = nil;
    _loop = false;
    _isplaying = false;
  }
  return self;
}

-(bool)isload {
  return _source_player != nil;
}


+(id) sharedInstance {
  if(!_instance) {
    _instance = [[ADAudioSource alloc] init];
  }
  return _instance;
}



-(bool) load:(NSString*) filepath {
  if(_source_filepath == nil || ![filepath isEqualToString:_source_filepath]) {
    _source_filepath = [filepath copy];
    NSError *error = nil;
    if(_isplaying){
      [self stop];
    }
    _source_player = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:_source_filepath] error:&error];
    
    if(error == nil) {
      _source_player.delegate = self;
      [_source_player prepareToPlay];
    }else {
      NSString* err = [NSString stringWithFormat:@"load %@ error[%@]", filepath, error];
      ad_error([err UTF8String]);
      return false;
    }
  }
  return true;
}


-(void)loop:(bool) v {
  _loop = v;
}

-(void) play {
  [_source_player play];
  _isplaying = true;
}


-(void) stop {
  _isplaying = false;
  [_source_player stop];
}

-(void) pause {
  _isplaying = false;
  [_source_player pause];
}

-(void)setVolume:(float) volume {
  _source_player.volume = volume;
}


- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag {
  _isplaying = false;
  if(_loop) {
    [self play];
  }
}

- (void)audioPlayerBeginInterruption:(AVAudioPlayer *)player {
  // nothing todo it.
}

- (void)audioPlayerEndInterruption:(AVAudioPlayer *)player {
  if(_isplaying) {
    [self play];
  }
}

- (void) applicationWillResignActive:(NSNotification *) notification {
  if(_isplaying) {
    [self pause];
  }
}

- (void) applicationDidBecomeActive:(NSNotification *) notification {
  if(_isplaying) {
    [self play];
  }
}

- (void) applicationWillTerminate:(NSNotification *) notification {
  if(_isplaying) {
    [self pause];
  }
}

@end

static int
l_load(lua_State* L) {
  const char* filepath = lua_tostring(L, 1);
  bool success = [[ADAudioSource sharedInstance] load:[NSString stringWithFormat:@"%s", filepath]];
  if(!success) {
    luaL_error(L, ad_last_error());
  }
  return 0;
}

static int
l_play(lua_State* L) {
  bool loop = lua_toboolean(L, 1);
  ADAudioSource* source = [ADAudioSource sharedInstance];
  if([source isload]){
    setMode(AVAudioSessionCategorySoloAmbient);
    [source loop:loop];
    [source play];
  }
  return 0;
}

static int
l_stop(lua_State* L) {
  ADAudioSource* source = [ADAudioSource sharedInstance];
  if([source isload]){
    setMode(AVAudioSessionCategoryAmbient);
    [source stop];
  }
  return 0;
}

static int
l_pause(lua_State* L) {
 ADAudioSource* source = [ADAudioSource sharedInstance];
  if([source isload]){
    setMode(AVAudioSessionCategoryAmbient);
    [source pause];
  }
  return 0; 
}

static int
l_volume(lua_State* L) {
  ADAudioSource* sourcer = [ADAudioSource sharedInstance];
  lua_Number v = lua_tonumber(L, 1);
  [sourcer setVolume:(float)v];
  return 0;
}

int
bgm_ios(lua_State* L) {
  luaL_checkversion(L);
  luaL_Reg l[] = {
    {"load", l_load},
    {"play", l_play},
    {"stop", l_stop},
    {"pause", l_pause},
    {"volume", l_volume},
    {NULL, NULL},
  };
    
  setMode(AVAudioSessionCategoryAmbient);
  luaL_newlib(L, l);
  return 1;
}

