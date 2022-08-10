#
# Copyright (C) 2023 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := android.hardware.power-service-mediatek
LOCAL_VENDOR_MODULE := true
LOCAL_VINTF_FRAGMENTS := power-mtk.xml
LOCAL_SRC_FILES := Power.cpp

LOCAL_SHARED_LIBRARIES := \
    libbase \
    libbinder_ndk \
    android.hardware.power-V2-ndk

include $(BUILD_SHARED_LIBRARY)
