include $(CLEAR_VARS)
LOCAL_MODULE    := mpg123

MPG123_SOURCE_DIR := $(LOCAL_PATH)/libmpg123/src/
LOCAL_ARM_MODE  := arm
LOCAL_CFLAGS    += -O4 -Wall -DHAVE_CONFIG_H  \
	-fomit-frame-pointer -funroll-all-loops -finline-functions -ffast-math
	
MPG123_SRC_FILE := \
	compat.c \
	parse.c \
	frame.c \
	format.c \
	dct64.c \
	equalizer.c \
	id3.c \
	icy.c \
	icy2utf8.c \
	optimize.c \
	readers.c \
	tabinit.c \
	libmpg123.c \
	index.c \
	layer1.c \
	layer2.c \
	layer3.c \
	dither.c \
	feature.c \
	synth.c \
	ntom.c \
	synth_8bit.c \
	stringbuf.c 
	
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
	LOCAL_CFLAGS += -mfloat-abi=softfp -mfpu=neon -DOPT_NEON -DREAL_IS_FLOAT
	MPG123_SRC_FILE += 	synth_neon.S \
						synth_stereo_neon.S \
						dct64_neon.S
else
	LOCAL_CFLAGS += -DOPT_ARM -DREAL_IS_FIXED
	MPG123_SRC_FILE += synth_arm.S
endif


LOCAL_C_INCLUDES  := $(MPG123_SOURCE_DIR)
LOCAL_SRC_FILES   := $(addprefix $(MPG123_SOURCE_DIR), $(MPG123_SRC_FILE))

include $(BUILD_STATIC_LIBRARY)