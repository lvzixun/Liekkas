
include $(CLEAR_VARS)
LOCAL_MODULE := lua

LUA_SOURCE_DIR := $(LOCAL_PATH)/lua/src/src/

LUA_SOURCE := $(addprefix $(LUA_SOURCE_DIR),\
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

LOCAL_CFLAGS      += -D"lua_getlocaledecpoint()='.'"
LOCAL_C_INCLUDES  := $(LUA_SOURCE_DIR)
LOCAL_SRC_FILES   := $(LUA_SOURCE)

include $(BUILD_STATIC_LIBRARY)