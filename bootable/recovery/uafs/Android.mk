
ifeq ($(GMT_FOTA_SUPPORT),yes)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := uafs.cpp 
LOCAL_SRC_FILES += RB_Android_FSUpdate.cpp
LOCAL_SRC_FILES += RB_FWUpdate.cpp
LOCAL_SRC_FILES += ../ui.cpp
LOCAL_SRC_FILES += ../default_device.cpp
LOCAL_SRC_FILES += ../screen_ui.cpp

commands_uafs_local_path := $(LOCAL_PATH)

LOCAL_MODULE := uafs

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES:= 
LOCAL_STATIC_LIBRARIES += libmincrypt 
LOCAL_STATIC_LIBRARIES += libminui libpng libz libpixelflinger_static 
LOCAL_STATIC_LIBRARIES += libcutils
LOCAL_STATIC_LIBRARIES += libfsua
LOCAL_STATIC_LIBRARIES += libstdc++ libc
LOCAL_C_INCLUDES += mediatek/custom/$(TARGET_PRODUCT)/recovery/inc
include $(BUILD_EXECUTABLE)

include $(commands_uafs_local_path)/libuafs/Android.mk
include $(commands_uafs_local_path)/flashutils/Android.mk

commands_uafs_local_path := 
endif
