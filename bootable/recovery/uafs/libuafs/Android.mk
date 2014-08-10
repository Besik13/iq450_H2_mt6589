
ifeq ($(GMT_FOTA_SUPPORT),yes)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libfsua
LOCAL_MODULE_SUFFIX := .a
#LOCAL_RAW_STATIC_LIBRARY: = true
LOCAL_SRC_FILES := GMT_GMT_ARM11_arm-eabi-4.6_fw_fs.lib
LOCAL_MODULE_CLASS := STATIC_LIBRARIES

include $(BUILD_PREBUILT)

# vDM Shared library
# ============================================================
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libdmc
LOCAL_MODULE_SUFFIX := .so
LOCAL_SRC_FILES := libdmc.so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

include $(BUILD_PREBUILT)

endif

