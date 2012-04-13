LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ardrone

# Do not use thumb because of VLIB assembly parts
LOCAL_CFLAGS := -Wall -marm
#LOCAL_CFLAGS := -DANDROID_NDK -DDISABLE_IMPORTGL

LOCAL_SRC_FILES := \
    app.c \
    video.c \
	at_cmds.c \
	navdata.c \
	stream.c \
	android.c \
	vlib.c \
	default.c

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog

include $(BUILD_SHARED_LIBRARY)
