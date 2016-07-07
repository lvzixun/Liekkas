
LOCAL_PATH := $(call my-dir)


include $(LOCAL_PATH)/lua/Android.mk
include $(LOCAL_PATH)/liekkas/Android.mk
# include $(LOCAL_PATH)/libmpg123/Android.mk


include $(CLEAR_VARS)
LOCAL_MODULE	:= Liekkas
LOCAL_LDLIBS	:= -lOpenSLES -llog -landroid

LOCAL_SRC_FILES := test_audio_jni.c 
LOCAL_STATIC_LIBRARIES	:= \
	lua \
	lk 
	# mpg123 
	
include $(BUILD_SHARED_LIBRARY)
