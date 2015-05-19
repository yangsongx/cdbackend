# Android makefile for peer_detect
#
# put the whole source dir under Android code location
# and type `mm' to build out target binary.
#
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= peer_detect.c \
                  scan_peer.c \
                  setup_server.c
LOCAL_CFLAGS += -DANDROID_ENV
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := liblog libcutils

LOCAL_MODULE:= peer_detect

include $(BUILD_EXECUTABLE)
