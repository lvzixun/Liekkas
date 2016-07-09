

all: install run log

log:
	adb logcat | grep ejoy

build:
	mkdir -p assets
	cp -r jni/liekkas/src/bind/*.lua assets/
	cp -r ../test/sound ../test/android_test.lua  assets/
	cd jni && ndk-build clean && ndk-build
	# cd jni && ndk-build
	ant debug

clean:
	ant clean
	rm -rf assets
	rm -rf ./libs/


uninstall:
	adb uninstall com.example.testaudio


run:
	adb shell am start -a android.intent.action.MAIN -n com.example.testaudio.TestAudio.MainActivity

install: clean uninstall build
	adb install ./bin/TestAudio-debug.apk


.PHONY: all build clean uninstall install