local audio = require "audio"

local function _delay()
  for i=1,100000000 do
  end
  print("delay~~")
end


local function _never_goto_world_end()
  while true do
  end
end


local _sound = {
  "sound/air_trap.mp3",
  "sound/alliance_build_drop_04.caf",
  "sound/alliance_jingle_v2.mp3",
}

local _music = {
  "sound/winwinwin.mp3",
  "sound/battle_lost_02.mp3",
}



for i,v in ipairs(_sound) do
  audio:load(v)
end



audio:play(_sound[3])
_delay()
audio:play(_sound[1])
_delay()
audio:play(_sound[2])

audio:background_music_play(_music[1], true)
_delay()
audio:background_music_stop()
_delay()
audio:background_music_play(_music[2], true)





--never breakup
_never_goto_world_end()