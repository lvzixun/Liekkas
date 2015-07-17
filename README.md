# Liekkas
a simple audio wrapper libary base on OpenAL. inspired by [Liekkas](http://music.163.com/#/song?id=3413895). ;)

## support platform

| platform | status |
|:-------:|:-------:|
| MAC OSX |  done |
| window | done |
| ios | done |
| android |  working |
| ubuntu |  working |


## support audio format 
| audio format | status |
|:-------:|:-------:|
| CAF | done |
| mp3 | done |
| wav | done |
| ogg | working |

## require 

1. OpenAL
2. mpg123  (if you want support mp3)
3. mingw32 (window need)
4. lua (>=lua5.2)


## tutorial
```
$ make
$ lua t.lua
```
read [`t.lua`](https://github.com/lvzixun/Liekkas/blob/master/t.lua) for more detail.

## LUA API
 simple effect sound API
~~~.lua
  local sound = require "sound"

  sound:load(file_path [, file_type])                     -- load a audio file
  sound:unload(file_path)                                 -- unload a audio file
  sound:play(file_path, [loop, [pitch, [pan, [gain]]]])   -- play a effect sound
~~~

 background music API
~~~.lua
  local music = require "music"

  music.load(file_path)          -- preload a music sound
  music.play(file_path, loop)    -- play a music sound, will load audio file when not load.
  music.stop()                   -- stop the current playing music sound
~~~

  group effect sound API
~~~.lua
  local audio = require "audio"

  local group1 = audio:create_group([count]) -- create a sound group has count source
  local handle = group1:add(file_path, [loop, [pitch, [pan, [gain]]]]) -- add sound file to group

  group1:play(handle)    -- play sound
  group1:stop(handle)    -- stop sound
  group1:rewind(handle)  -- rewind sound
  group1:pause(handle)  -- pause sound
~~~

read [`audio.lua`](https://github.com/lvzixun/Liekkas/blob/master/audio.lua) wrapper file for more detail.

## interrupt interface
~~~.c
// call after interruption starts
void oal_interrupted();

// call after interruption ends
void oal_resumed();
~~~

