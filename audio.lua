local oal = require "oal"
local ad = require "oal.decode"

local SOURCE_LIMIT = 32


local M = {
  load_map = {},

  source_handle = {},
  _cur_source_idx = 0,

  background_music_source = false,
}

local function _suffix(s)
  return string.gsub(s, "^.+%.(.+)$", "%1")
end


function M:_init()
  -- init source handle
  local handles = self.source_handle
  for i=1,SOURCE_LIMIT do
    handles[i] = {
    source_id = oal.create_source(),
    load_key = false,
  }
  end

  self.background_music_source = oal.create_source()
end

local support_type = {
  ["caf"] = function (file_path)
    return ad.decode_caf(file_path)
  end,
  ["mp3"] = function (file_path)
    return ad.decode_mp3(file_path)
  end,
  ["wav"] = function (file_path)
    return ad.decode_wav(file_path)
  end,
}

function M:load(file_path, file_type)
  file_type = file_type or _suffix(file_path)
  local func = support_type[file_type]
  if not func then
    error("cannot load "..file_path)
  end

  local entry = self.load_map[file_path]
  if not entry then
    local info = func(file_path)
    print("load: "..tostring(info))
    local buffer_id = oal.create_bufferid()
    oal.buffer_bind(buffer_id, info)
    entry = {
      info = info,
      buffer_id = buffer_id,
    }
    self.load_map[file_path] = entry
  end
end


function M:unload(file_path)
  local entry = self.load_map[file_path]
  if not entry then
    return
  end

  for i,v in ipairs(self.source_handle) do
    if v.load_key == file_path then
      local source_id = v.source_id
      source_id:clear()
      v.load_key = false
    end
  end

  self.load_map[file_path] = nil
end



function M:_get_source()
  self._cur_source_idx = self._cur_source_idx % SOURCE_LIMIT + 1
  return self.source_handle[self._cur_source_idx]
end


function M:play(file_path, loop, pitch, pan, gain)
  pitch = pitch or 1.0
  pan = pan or 0.0
  gain = gain or 1.0
  loop = loop or false
  local entry = self.load_map[file_path]
  if entry then
    local source = self:_get_source()
    source.load_key = file_path
    local source_id = source.source_id
    oal.source_set(source_id, entry.buffer_id, pitch, pan, gain, loop)
    source_id:play()
  end
end



--------------- background music -------------------


local function _gen_ios_hd_func()
  local cur_file_path = false
  local hd_ios = ad.decode_hardware_ios
  local m = {}

  function m.load(self, file_path)
    hd_ios.load(file_path)
    cur_file_path = file_path
  end

  function m.play(self, file_path, loop)
    if file_path ~= cur_file_path then
      m.load(self, file_path)
    end
    hd_ios.play(loop or false)
  end

  function m.stop(self)
    hd_ios.stop()
  end
  return m
end


local bg_t = ad.decode_hardware_ios and "ios_hd" or "oal"
local _bg_music_handles = {
  ["ios_hd"] = _gen_ios_hd_func(),

  ["oal"] = {
    load = function (self, file_path)
      self:load(file_path)
    end,

    play = function (self, file_path, loop)
      self:load(file_path)
      local entry = self.load_map[file_path]
      local source = self.background_music_source
      oal.source_set(source, entry.buffer_id, 1.0, 0.0, 1.0, loop or false)
      source:play()
    end,

    stop = function (self)
      local source = self.background_music_source
      if source then
        source:stop()
      end
    end,
  },
}

local _cur_bg_handle = _bg_music_handles[bg_t]
assert(_cur_bg_handle)


function M:background_music_load(file_path)
  _cur_bg_handle.load(self, file_path)
end

function M:background_music_play(file_path, loop)
  _cur_bg_handle.play(self, file_path, loop)
end

function M:background_music_stop()
  _cur_bg_handle.stop(self)
end

M:_init()
return M

