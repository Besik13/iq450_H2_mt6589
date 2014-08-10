



ifeq ($(GMT_FOTA_SUPPORT),yes)
LOCAL_PATH := $(call my-dir)

ifneq ($(TARGET_SIMULATOR),true)
ifeq ($(TARGET_ARCH),arm)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := flash_image_gmobi.c
LOCAL_MODULE := flash_image_gmobi
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libflashutils_gmobi libmtdutils_gmobi libmmcutils_gmobi libbmlutils_gmobi


LOCAL_STATIC_LIBRARIES += libcutils libc
LOCAL_FORCE_STATIC_EXECUTABLE := true
include $(BUILD_EXECUTABLE)

# libflashutils_gmobi
# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libflashutils_gmobi
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := libflashutils_gmobi.a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES

include $(BUILD_PREBUILT)

# libmtdutils_gmobi
# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libmtdutils_gmobi
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := libmtdutils_gmobi.a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES

include $(BUILD_PREBUILT)

# libmmcutils_gmobi
# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libmmcutils_gmobi
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := libmmcutils_gmobi.a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES

include $(BUILD_PREBUILT)


# libbmlutils_gmobi
# =============================================
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libbmlutils_gmobi
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := libbmlutils_gmobi.a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES

include $(BUILD_PREBUILT)


endif	# TARGET_ARCH == arm
endif	# !TARGET_SIMULATOR
endif
