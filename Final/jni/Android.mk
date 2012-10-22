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

LOCAL_MODULE    := ndk1
LOCAL_SRC_FILES := Tom_fast_score.cpp


#TARGET_CXX := $(TARGET_CXX) -I/home/winston/Android/STLport-5.1.7/stlport

LOCAL_ARM_NEON = true

#CLFAGS = -I/Users/twd/Android/stlport/stlport


#ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_CFLAGS := -DHAVE_NEON=1 \
    				-DTOON_DEFAULT_PRECISION=float\
    				-flax-vector-conversions -O3
    				
    LOCAL_SRC_FILES += 	halfsize_neon.cpp.neon \
    					Tom_fast_9.cpp.neon \
    					native.cpp.neon \
    					pointset.cpp.neon \
    					orbdetector.cpp.neon \
    					imagepoint.cpp.neon \
    					descriptor.cpp \
    					commands.cpp \
    					comms/ClientSocket.cpp.neon \
    					comms/Socket.cpp.neon
#endif


LOCAL_LDLIBS := -lGLESv1_CM -llog

include $(BUILD_SHARED_LIBRARY)