

all: install run log

log:
	adb logcat | grep ejoy

build:
	cd jni && ndk-build clean && ndk-build
	ant debug

clean:
	ant clean
	rm -rf assets
	rm -rf ./libs/


uninstall:
	adb uninstall com.example.testaudio


run:
	mkdir -p assets
	cp -r jni/src/bind/*.lua assets/
	cp -r ../test/sound assets/
	adb shell am start -a android.intent.action.MAIN -n com.example.testaudio.TestAudio.MainActivity

install: clean uninstall build
	adb install ./bin/TestAudio-debug.apk


.PHONY: all build clean uninstall install