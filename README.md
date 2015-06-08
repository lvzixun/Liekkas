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
2. mpg123 
3. mingw32 (window need)
4. lua (>=lua5.2)


## tutorial
```
$ make
$ lua t.lua
```
read [`t.lua`](https://github.com/lvzixun/Liekkas/blob/master/t.lua) for more detail.

## LUA API
 effect sound API
~~~.lua
  audio:load(file_path [, file_type])   -- load a audio file
  audio:unload(file_path)               -- unload a audio file
  audio:play(file_path)                 -- play a effect sound
~~~

 background music API
~~~.lua
  audio:background_music_load(file_path)          -- preload a music sound
  audio:background_music_play(file_path, loop)    -- play a music sound, will load audio file when not load.
  audio:background_music_stop()                   -- stop the current playing music sound
~~~
read [`audio.lua`](https://github.com/lvzixun/Liekkas/blob/master/audio.lua) wrapper file for more detail.