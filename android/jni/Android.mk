# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := Liekkas

# for lua code
LUA_SOURCE := $(addprefix src/lua/src/,\
	lapi.c \
	lauxlib.c \
	lbaselib.c \
	lbitlib.c \
	lcode.c \
	lcorolib.c \
	lctype.c \
	ldblib.c \
	ldebug.c \
	ldo.c \
	ldump.c \
	lfunc.c \
	lgc.c \
	linit.c \
	liolib.c \
	llex.c \
	lmathlib.c \
	lmem.c \
	loadlib.c \
	lobject.c \
	lopcodes.c \
	loslib.c \
	lparser.c \
	lstate.c \
	lstring.c \
	lstrlib.c \
	ltable.c \
	ltablib.c \
	ltm.c \
	lundump.c \
	lutf8lib.c \
	lvm.c \
	lzio.c \
	)

# fot opensl and decode
LIEKKAS_SOURCE := src/lk_decode.c \
	src/lk_util.c \
	src/opensl/opensl.c \
	src/opensl/lopensl.c \
	src/decode/ad_mp3.c \
	src/decode/ad_ogg.c \
	src/decode/ad_tools.c \
	src/decode/ad_wav.c


# fot test audio
TEST_AUDIO_SOURCE := test_audio_jni.c

LOCAL_SRC_FILES := $(LUA_SOURCE) $(LIEKKAS_SOURCE) $(TEST_AUDIO_SOURCE)


# for native audio
LOCAL_LDLIBS    += -lOpenSLES
# for logging
LOCAL_LDLIBS    += -llog
# for native asset manager
LOCAL_LDLIBS    += -landroid

LOCAL_CFLAGS += -D"lua_getlocaledecpoint()='.'"

LOCAL_C_INCLUDES += src/lua/src/

include $(BUILD_SHARED_LIBRARY)
