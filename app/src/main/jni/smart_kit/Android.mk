# Copyright (C) 2009 The Android Open Source Project
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
MY_LOCAL_PATH := $(LOCAL_PATH)
SMART_DIR = $(LOCAL_PATH)

#------------------Pull prebuild library into the build------------------#
include $(CLEAR_VARS)
MY_SMART_LIBDIR := $(MY_LOCAL_PATH)/fftw
define add_smartkit_module
	include $(CLEAR_VARS)
	LOCAL_MODULE:=$1
	LOCAL_SRC_FILES:=lib$1.a
	include $(PREBUILT_STATIC_LIBRARY)  
endef
SMARTKIT_LIBS := iconv ivc_zxing
LOCAL_PATH := $(MY_SMART_LIBDIR)
$(foreach module,$(SMARTKIT_LIBS),$(eval $(call add_smartkit_module,$(module))))
LOCAL_PATH := $(MY_LOCAL_PATH)
#------------------OpenCV configuration-----------------------------------#
OPENCV_INSTALL_MODULE :=on
OPENCV_LIB_TYPE := STATIC 
include $(CLEAR_VARS)
#ifeq ("$(wildcard $(OPENCV_MK_PATH))","")
#	#include D:/OpenCV-2.4.9-android-sdk/sdk/native/jni/OpenCV.mk
#	include E:/OpenCV-2410-android-sdk/sdk/native/jni/OpenCV.mk
#else  
#	include $(OPENCV_MK_PATH)  
#endif

include /home/cefengxu/cetool/openCV-31-android-sdk/sdk/native/jni/OpenCV.mk

#LOCAL_CFLAGS := -march=armv7-a -mfpu=neon -mfloat-abi=softfp

#------------------smart recognition configuration-------------------------#
SMART_RECOGNITION 		:=false
SMART_FOOD 				:=false
SMART_MOTION 			:=false
SMART_DOCUMENT 			:=false
SMART_CODE 				:=false
SMART_LANDSCAPE 		:=false
SMART_PORTRAIT 			:=false
SMART_PORTRAITLIT 		:=false
SMART_LIGHT 			:=false
SMART_AR 				:=false
#------------------smart composition configuration-------------------------#
SMART_COMPOSITION 		:=false
SMART_PORTRAIT_GUIDE 	:=false
SMART_LANDSCAPE_GUIDE 	:=false
SMART_FOOD_GUIDE 		:=false
#------smart recognition build------
ifeq ($(SMART_RECOGNITION),true)
	LOCAL_CFLAGS +=-DSMART_RECOGNITION
#	SRC_FILES += $(wildcard $(SMART_DIR)/smart_recognition.cpp)
	#------code------
	ifeq ($(SMART_CODE),true)
		LOCAL_CFLAGS +=-DSMART_CODE
		LOCAL_C_INCLUDES += $(SMART_DIR)/zxing
		LOCAL_C_INCLUDES += $(SMART_DIR)/obj_detection/code_detection
		SRC_FILES += $(wildcard $(SMART_DIR)/obj_detection/code_detection/code_detection.cpp)
		SRC_FILES += $(wildcard $(SMART_DIR)/obj_detection/code_detection/zxing_code_detection.cpp)
		SRC_FILES += $(wildcard $(SMART_DIR)/obj_tracking/KLTracking/qrcode_tracker.cpp)
		SRC_FILES += $(wildcard $(SMART_DIR)/zxing/*.cpp)
		LOCAL_STATIC_LIBRARIES += ivc_zxing iconv
	endif
	#------portait------
	ifeq ($(SMART_PORTRAIT),true)
		LOCAL_CFLAGS +=-DSMART_PORTRAIT
	endif
	#------portaitlit------
	ifeq ($(SMART_PORTRAITLIT),true)
		LOCAL_CFLAGS +=-DSMART_PORTRAITLIT
	endif
	#------landscape------
	ifeq ($(SMART_LANDSCAPE),true)
		LOCAL_CFLAGS +=-DSMART_LANDSCAPE
		LOCAL_C_INCLUDES += $(SMART_DIR)/obj_detection/line_detection
		LOCAL_C_INCLUDES += $(SMART_DIR)/feature_extraction/gist_feature
		SRC_FILES += $(wildcard $(SMART_DIR)/feature_extraction/*.cpp)
		SRC_FILES += $(wildcard $(SMART_DIR)/feature_extraction/gist_feature/*.cpp)
		SRC_FILES += $(wildcard $(SMART_DIR)/obj_detection/line_detection/*.cpp)
		LOCAL_LDLIBS +=  -lfftw3 -lfftw3f -lfftw3f_threads
	    LOCAL_LDLIBS += -L$(SMART_DIR)/fftw	
	endif
	#------food------
	ifeq ($(SMART_FOOD),true)
		LOCAL_CFLAGS +=-DSMART_FOOD
		LOCAL_C_INCLUDES += $(SMART_DIR)/obj_detection/ellipse_detection
		SRC_FILES += $(wildcard $(SMART_DIR)/obj_detection/ellipse_detection/*.cpp)
		SRC_FILES += $(wildcard $(SMART_DIR)/obj_tracking/KLTracking/food_tracker.cpp)
	endif
	#------document------
	ifeq ($(SMART_DOCUMENT),true)
		LOCAL_CFLAGS +=-DSMART_DOCUMENT
	endif
	#------motion------
	ifeq ($(SMART_MOTION),true)
		LOCAL_CFLAGS +=-DSMART_MOTION
		SRC_FILES += $(wildcard $(SMART_DIR)/util/motion_util.cpp)
	endif
	#------ar------
	ifeq ($(SMART_AR),true)
		LOCAL_CFLAGS +=-DSMART_AR
		SRC_FILES += $(wildcard $(SMART_DIR)/smart_ar.cpp)
		SRC_FILES += $(wildcard $(SMART_DIR)/smart_ar/*.cpp)
		SRC_FILES += $(wildcard $(SMART_DIR)/smart_ar/gl_matrix/*.cpp)
		SRC_FILES += $(wildcard $(SMART_DIR)/smart_ar/gl_display/*.cpp)
	endif

	#------light------
	ifeq ($(SMART_LIGHT),true)
		LOCAL_CFLAGS +=-DSMART_LIGHT
	endif
endif
#------smart composition build------
ifeq ($(SMART_COMPOSITION),true)
	LOCAL_CFLAGS +=-DSMART_COMPOSITION
#	SRC_FILES += $(wildcard $(SMART_DIR)/smart_composition.cpp)
	#------composition_portrait------
	ifeq ($(SMART_PORTRAIT_GUIDE),true)
		LOCAL_CFLAGS +=-DSMART_PORTRAIT_GUIDE
	endif
	#------composition_landscape------
	ifeq ($(SMART_LANDSCAPE_GUIDE),true)
		LOCAL_CFLAGS +=-DSMART_LANDSCAPE_GUIDE
	endif
	#------composition_food------
	ifeq ($(SMART_FOOD_GUIDE),true)
		LOCAL_CFLAGS +=-DSMART_FOOD_GUIDE
	endif
endif

	LOCAL_C_INCLUDES += $(SMART_DIR)/obj_detection
	LOCAL_C_INCLUDES += $(SMART_DIR)/obj_tracking
	LOCAL_C_INCLUDES += $(SMART_DIR)/obj_tracking/mf_tracking
	LOCAL_C_INCLUDES += $(SMART_DIR)
	LOCAL_C_INCLUDES += $(SMART_DIR)/util
	LOCAL_C_INCLUDES += $(SMART_DIR)/include
	
	LOCAL_C_INCLUDES += $(SMART_DIR)/obj_tracking/tld_tracking/


#ifeq ($(SMART_LANDSCAPE),false)
#	LOCAL_CFLAGS +=-DSMART_LANDSCAPE	
#	LOCAL_C_INCLUDES += $(SMART_DIR)/obj_detection/line_detection
#	LOCAL_C_INCLUDES += $(SMART_DIR)/feature_extraction/gist_feature
#	SRC_FILES += $(wildcard $(SMART_DIR)/feature_extraction/*.cpp)
#	SRC_FILES += $(wildcard $(SMART_DIR)/feature_extraction/gist_feature/*.cpp)
#	SRC_FILES += $(wildcard $(SMART_DIR)/obj_detection/line_detection/*.cpp)
#	LOCAL_LDLIBS +=  -lfftw3 -lfftw3f -lfftw3f_threads
#	LOCAL_LDLIBS += -L$(SMART_DIR)/fftw	
#endif
	
	SRC_FILES += $(wildcard $(SMART_DIR)/obj_detection/obj_detection.cpp)
	SRC_FILES += $(wildcard $(SMART_DIR)/obj_tracking/*.cpp)
	SRC_FILES += $(wildcard $(SMART_DIR)/obj_tracking/mf_tracking/*.cpp)
	SRC_FILES += $(wildcard $(SMART_DIR)/util/face_util.cpp)
	SRC_FILES += $(wildcard $(SMART_DIR)/util/light_util.cpp)
	SRC_FILES += $(wildcard $(SMART_DIR)/util/math_util.cpp)
	SRC_FILES += $(wildcard $(SMART_DIR)/util/util.cpp)
	
	SRC_FILES += $(wildcard $(SMART_DIR)/obj_tracking/KLTracking/general_tracker.cpp)
	SRC_FILES += $(wildcard $(SMART_DIR)/obj_tracking/KLTracking/LKTracker.cpp)
	
	SRC_FILES += $(wildcard $(SMART_DIR)/smart_extern_manager.cpp)
	SRC_FILES += $(wildcard $(SMART_DIR)/smart_process.cpp)
	SRC_FILES += $(wildcard $(SMART_DIR)/smart_recognition.cpp)
	SRC_FILES += $(wildcard $(SMART_DIR)/smart_composition.cpp)
	SRC_FILES += $(wildcard $(SMART_DIR)/smart_always_in_focus.cpp)
#	SRC_FILES += $(wildcard $(SMART_DIR)/smart_gesture_recongnition.cpp)
	SRC_FILES += $(wildcard $(SMART_DIR)/smart_planet.cpp)
	
	SRC_FILES += $(wildcard $(SMART_DIR)/obj_tracking/tld_tracking/*.cpp)
	
	LOCAL_SRC_FILES += $(SRC_FILES:$(LOCAL_PATH)/%=%)
	
	LOCAL_LDLIBS +=  -lz -lm -llog -lGLESv2

#LOCAL_STATIC_LIBRARIES := cpufeatures


LOCAL_MODULE    := smart_kit

include $(BUILD_SHARED_LIBRARY)
#$(call import-module,cpufeatures)
#include ${SMART_DIR}/zxing/Android.mk

