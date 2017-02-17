LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS) 
 
LOCAL_MODULE    := libiconv 
 
LOCAL_CFLAGS    := \
	-Wno-multichar \
	-D_ANDROID \
	-DBUILDING_LIBICONV \
	-DIN_LIBRARY \
	-DLIBDIR="\"c\"" \
    -I$(LOCAL_PATH)/iconv-android/ \
    -I$(LOCAL_PATH)/iconv-android/include/ \
    -I$(LOCAL_PATH)/iconv-android/lib/ \
	-I$(LOCAL_PATH)/iconv-android/libcharset/include \

LOCAL_SRC_FILES := \
     iconv-android/lib/iconv.c \
     iconv-android/lib/relocatable.c \
     iconv-android/libcharset/lib/localcharset.c \

include $(BUILD_STATIC_LIBRARY) 
 
LOCAL_LDLIBS    := -llog -lcharset
