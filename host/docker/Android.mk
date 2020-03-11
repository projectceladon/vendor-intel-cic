LOCAL_PATH := $(call my-dir)

# host docker build helper script
include $(CLEAR_VARS)
LOCAL_MODULE := aic-build
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := scripts/aic-build
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_SUFFIX :=
LOCAL_IS_HOST_MODULE := true
LOCAL_BUILT_MODULE_STEM := $(notdir $(LOCAL_SRC_FILES))
include $(BUILD_PREBUILT)

# wrapper tool for the veritysetup
include $(CLEAR_VARS)
LOCAL_MODULE := cic_veritysetup

LOCAL_SRC_FILES:= \
     aic-manager/src/cic_verity/cic_verity_mgr.c \

LOCAL_LDFLAGS := -static
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := liblog \
     libcrypto \
     libcrypto_utils \
     libbase

include $(BUILD_HOST_EXECUTABLE)

# verity img build script
include $(CLEAR_VARS)
LOCAL_MODULE := build_verity_img.py
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := scripts/build_verity_img.py
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_SUFFIX :=
LOCAL_IS_HOST_MODULE := true
LOCAL_BUILT_MODULE_STEM := $(notdir $(LOCAL_SRC_FILES))
include $(BUILD_PREBUILT)
