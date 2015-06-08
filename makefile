.PHONY: clean

UNAME = $(shell uname)

OAL_SOURCE = oal.c \
	oal_decode.c \
	decode/ad_caf.c \
	decode/ad_mp3.c \
	decode/ad_wav.c \
	decode/ad_hardware_mac_ios.m

ifeq ($(UNAME), Darwin)
	CC = clang
	FLAG = -fPIC -Wall -g -dynamiclib -Wl,-undefined,dynamic_lookup
	MACRO = -DSUPPORT_AUDIO_MP3 -DSUPPORT_AUDIO_CAF
	FRAMEWORK = -framework OpenAL -framework AudioToolbox  -framework AVFoundation  -lmpg123
	LIB_SUFFIX = so
	TARGET = $(OAL_LIB)
else
	CC = gcc
	MACRO = -DSUPPORT_AUDIO_MP3
	FLAG = -Wall -g -xc -I../lua-5.2.4/src -I../_3lib/mpg123-1.22.0-x86 -I../_3lib/openal32/include \
		-L../_3lib/openal32 -L../_3lib/mpg123-1.22.0-x86 -L../lua-5.2.4/src  -llua52 -lOpenAL32 -lmpg123-0
	LIB_SUFFIX = dll
	TEST_EXE = test.exe
	TARGET = $(TEST_EXE) bin
endif

OAL_LIB = oal.$(LIB_SUFFIX)


all: $(TARGET)


$(TEST_EXE): $(OAL_SOURCE) test.c
	$(CC) $(FLAG) -o $@ $^ $(MACRO)


$(OAL_LIB): $(OAL_SOURCE)
	$(CC) $(FLAG)  --shared -o $@  $^ $(FRAMEWORK) $(MACRO)


bin:
	-mkdir ../bin
	-cp ../lua-5.2.4/src/lua.exe ../bin 
	-cp ../lua-5.2.4/src/lua52.dll ../bin
	-cp ../_3lib/openal32/OpenAL32.dll ../bin
	-cp -r ../_3lib/mpg123-1.22.0-x86/*.dll ../bin
	-cp $(TEST_EXE) ../bin


clean:
	-rm -rf $(OAL_LIB)
	-rm -rf ../bin

