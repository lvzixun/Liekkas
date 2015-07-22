local audio = require "audio"
local music = require "music"
local sound = require "sound"

local function _delay()
  for i=1,50000000 do
  end
  print("delay~~")
end


local function _never_goto_world_end()
  while true do
  end
end


local _sound = {
  "test/sound/air_trap.mp3",
  "test/sound/alliance_build_drop_04.caf",
  "test/sound/alliance_jingle_v2.mp3",
}

local _music = {
  "test/sound/winwinwin.mp3",
  "test/sound/battle_lost_02.mp3",
}



for i,v in ipairs(_sound) do
  sound:load(v)
end

sound:play(_sound[1], true)
_delay()
sound:play(_sound[3])


local group1 = audio:create_group(3)
local handle = group1:add(_sound[1])
group1:play(handle)

group1:play(group1:add(_sound[2]))
group1:play(group1:add(_sound[2]))
group1:play(group1:add(_sound[1], true))

_delay()
audio:unload(_sound[1])



music.play(_music[1], true)


--never breakup
_never_goto_world_end()