SDK_PATH:=$(shell pwd)/../../ARDrone_API/ARDroneLib
PC_TARGET=yes

ifdef ALL_SOURCES
   include $(ARDRONE_CUSTOM_CONFIG)
   include $(ARDRONE_BUILD_CONFIG)
else
   include $(SDK_PATH)/Soft/Build/custom.makefile
   include $(SDK_PATH)/Soft/Build/config.makefile
endif

ifeq "$(RELEASE_BUILD)" "yes"
   ARDRONE_TARGET_DIR=$(SDK_PATH)/Version/Release
else
   ARDRONE_TARGET_DIR=$(SDK_PATH)/Version/Debug
endif

TARGET=linux_sdk_demo

SRC_DIR:=$(shell pwd)/../Sources

# Define application source files
GENERIC_BINARIES_SOURCE_DIR:=$(SRC_DIR)

GENERIC_BINARIES_COMMON_SOURCE_FILES+=			\
   UI/ui.c  \
   UI/gamepad.cpp \
   UI/keyboard.cpp \
   Navdata/navdata.c \
   Navdata/NavDataContainer.cpp \
   Video/video_stage.c \
   UI/planner.cpp \
   xbee/xbee.cpp \
   Tools/coopertools.cpp \
   Tools/medianfilter.cpp \
   Tools/smoothingmedianfilter.cpp 

GENERIC_INCLUDES+=					\
	$(SRC_DIR) \
	$(LIB_DIR) \
	$(SDK_PATH)/Soft/Common \
	$(SDK_PATH)/Soft/Lib

GENERIC_TARGET_BINARIES_PREFIX=

GENERIC_TARGET_BINARIES_DIR=$(ARDRONE_TARGET_DIR)

GENERIC_BINARIES_SOURCE_ENTRYPOINTS+=			\
   ardrone_testing_tool.c

GENERIC_INCLUDES:=$(addprefix -I,$(GENERIC_INCLUDES))

GENERIC_LIB_PATHS=-L$(GENERIC_TARGET_BINARIES_DIR)
GENERIC_LIBS=-lpc_ardrone -lgtk-x11-2.0 -lrt -lhighgui -lfftw3 -lml -lcxcore -lcv

SDK_FLAGS+="USE_APP=yes"
SDK_FLAGS+="APP_ID=linux_sdk_demo"

export GENERIC_CFLAGS
export GENERIC_LIBS
export GENERIC_LIB_PATHS
export GENERIC_INCLUDES
export GENERIC_BINARIES_SOURCE_DIR
export GENERIC_BINARIES_COMMON_SOURCE_FILES
export GENERIC_TARGET_BINARIES_PREFIX
export GENERIC_TARGET_BINARIES_DIR
export GENERIC_BINARIES_SOURCE_ENTRYPOINTS

# Bug fix ...
export GENERIC_LIBRARY_SOURCE_DIR=$(GENERIC_BINARIES_SOURCE_DIR)

.PHONY: $(TARGET) $(MAKECMDGOALS)

all: $(GEN_CUSTOM_HEADER) $(TARGET)

$(GEN_CUSTOM_HEADER): custom.makefile
	@echo "#ifndef _GENERATED_CUSTOM_CONFIGURATION_H_" > $@
	@echo "#define _GENERATED_CUSTOM_CONFIGURATION_H_" >> $@
	@echo >> $@
	@echo "#if  defined(BR2_PACKAGE_BCM4318_AP)" >> $@
	@echo "#  define AP" >> $@
	@echo "#else" >> $@
	@echo "#  define STA" >> $@
	@echo "#endif" >> $@
	@echo "#define CURRENT_NUM_VERSION_SOFT \"$(MAJOR_VERSION).$(MINOR_VERSION).$(MODIF_VERSION)\"" >> $@
	@echo "#define CURRENT_BUILD_DATE \"$(shell date +%F\ %H:%M)\"" >> $@
	@echo >> $@
ifeq ("$(VIDEO_YUV)","yes")
	@echo "#define USE_VIDEO_YUV" >> $@
endif
ifeq ("$(RECORD_VISION_DATA)","yes")
	@echo "#define RECORD_VISION_DATA" >> $@
endif
	@echo >> $@
	@echo "#define WIFI_NETWORK_NAME \"$(WIFI_NETWORK_NAME)\"" >> $@
	@echo "#define WIFI_BROADCAST \"$(WIFI_BROADCAST)\"" >> $@
	@echo "#define WIFI_ARDRONE_IP \"$(WIFI_ARDRONE_IP)\"" >> $@
	@echo >> $@
	@echo "#if defined(__linux__) || defined(USE_MINGW32)" >> $@
	@echo "# define WIFI_MOBILE_IP \"$(WIFI_MOBILE_IP)\"" >> $@
	@echo "# define WIRED_ITFNAME \"$(WIRED_ITFNAME)\"" >> $@
	@echo "#endif // ! __linux__" >> $@
	@echo >> $@
	@echo >> $@
	@echo "#endif // ! _GENERATED_CUSTOM_CONFIGURATION_H_" >> $@

$(TARGET):
	@$(MAKE) -C $(SDK_PATH)/VP_SDK/Build $(TMP_SDK_FLAGS) $(SDK_FLAGS) $(MAKECMDGOALS)
	mv $(ARDRONE_TARGET_DIR)/ardrone_testing_tool $(TARGET)
	cp $(TARGET) $(ARDRONE_TARGET_DIR)

$(MAKECMDGOALS):
	@$(MAKE) -C $(SDK_PATH)/VP_SDK/Build $(TMP_SDK_FLAGS) $(SDK_FLAGS) $(MAKECMDGOALS)

