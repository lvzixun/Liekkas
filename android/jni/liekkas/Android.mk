
include $(CLEAR_VARS)
LOCAL_MODULE := lk

LOCAL_CFLAGS     := -DSUPPORT_AUDIO_MP3
LOCAL_C_INCLUDES := $(LUA_SOURCE_DIR) $(MPG123_SOURCE_DIR)

LIEKKAS_SOURCE_DIR := $(LOCAL_PATH)/liekkas/src/
LOCAL_SRC_FILES := $(addprefix $(LIEKKAS_SOURCE_DIR), \
    opensl/opensl.c \
    opensl/lopensl.c \
    decode/ad_mp3.c \
    decode/ad_ogg.c \
    decode/ad_tools.c \
    decode/ad_wav.c \
    bgm/bgm_android.c \
    lk_util.c \
    lk_bgm.c \
    lk_decode.c \
    )

include $(BUILD_STATIC_LIBRARY)