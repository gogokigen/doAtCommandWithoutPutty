# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	doAtCmd.c

LOCAL_CFLAGS := -Wall -Werror

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE:= doAtCmd

LOCAL_MODULE_PATH   := $(PRODUCT_OUT)/system/bin

include $(BUILD_EXECUTABLE)

