LOCAL_PATH:= $(call my-dir)
#############################################################
# build into .so
include $(CLEAR_VARS)

OPENCV_CAMERA_MODULES:=off
OPENCV_LIB_TYPE:=STATIC
include /home/yang/opencv/OpenCV-2.4.9-android-sdk/sdk/native/jni/OpenCV.mk

LOCAL_MODULE:= libmyocr
LOCAL_SRC_FILES:= myocr.cpp

# NOTE - need specify SDK location
LOCAL_CFLAGS += -DANDROID_ENV -D_JNISO
LOCAL_SHARED_LIBRARIES := liblog libcutils

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
