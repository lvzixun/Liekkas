local audio = require "audio"

local M = {}
local sound_group = audio:create_group(16)


function M:load(file_path)
  return audio:load(file_path)
end


function M:unload(file_path)
  return audio:unload(file_path)
end

function M:play(file_path, loop, pitch, pan, gain)
  local handle  = sound_group:add(file_path, loop, pitch, pan, gain)
  sound_group:play(handle)
end



return M