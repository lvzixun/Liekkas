local util = require "util"
local liekkas = require "liekkas"
print = function (...)
    local args = {...}
    local ret = {}
    for i,v in ipairs(args) do
        ret[i] = tostring(v)
    end
    local s = table.concat(ret, "\t")
    liekkas.log(s)
end

--- sea loader
local function gen_loader()
    return function (modname)
        local filename = string.gsub(modname, "%.", "/") .. ".lua"
        local source = util.readall(filename)
        if not source then
            return "\n\tno such file:"..filename
        else
            local chunk, err = load(source, "@"..filename)
            if not chunk then
                return error(string.format("error loading module %s from assets file %s :\n\t%s",
                    modname, filename, err))
            end
            return chunk, filename
        end
    end
end
table.insert(package.searchers, 3, gen_loader())


local audio = require "audio"
local sound = require "sound"


print("hello world android liekkas!!", audio, sound)

local function _never_goto_world_end()
  while true do
  end
end


local function _delay()
  for i=1,1000000 do
  end
  print("delay~~")
end


-- for test
local audio_files = {
    "sound/alliance_build_drop_04.wav",
}

for i,v in ipairs(audio_files) do
    sound:load(v)
end

sound:play(audio_files[1], true)

-- -- _delay()
-- sound:play(audio_files[1])
-- -- _delay()
-- sound:play(audio_files[1])
-- sound:play(audio_files[1])
-- sound:play(audio_files[1])
-- sound:play(audio_files[1])


