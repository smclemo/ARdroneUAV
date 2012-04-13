SDK_PATH:=$(shell pwd)/../../../../ARDroneLib
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

TARGET=ardrone_navigation

SRC_DIR:=$(shell pwd)/../Sources


COMMON_MOB_SRC_PATH 	= common
UTILS_SRC_PATH 		= utils
VIDEO_SRC_PATH 		= video
UI_SRC_PATH 		= UI
BT_SRC_PATH 		= BT
REF_SEND_SRC_PATH 	= ref_send
IHM_SRC_PATH 		= ihm
NDC_SRC_PATH 		= navdata_client

# Define application source files

GENERIC_BINARIES_SOURCE_DIR:=$(SRC_DIR)

GENERIC_BINARIES_COMMON_SOURCE_FILES+=			\
	$(COMMON_MOB_SRC_PATH)/mobile_config.c		\
	$(COMMON_MOB_SRC_PATH)/mobile_wifi.c		\
	$(UI_SRC_PATH)/ui.c				\
	$(UI_SRC_PATH)/gamepad.c			\
	$(UI_SRC_PATH)/wiimote.c			\
	$(UTILS_SRC_PATH)/utils.c			\
	$(UTILS_SRC_PATH)/remote_console.c		\
	$(NDC_SRC_PATH)/navdata_client.c		\
	$(NDC_SRC_PATH)/navdata_tablepilotage.c		\
	$(NDC_SRC_PATH)/navdata_ihm.c			\
	$(IHM_SRC_PATH)/view_drone_attitude.c		\
	$(IHM_SRC_PATH)/ihm_vision.c    		\
	$(IHM_SRC_PATH)/ihm_stages_o_gtk.c    		\
	$(IHM_SRC_PATH)/ihm.c

ifeq ($(PC_USE_POLARIS),yes)
GENERIC_BINARIES_COMMON_SOURCE_FILES+=			\
	$(NDC_SRC_PATH)/navdata_polaris.c		
endif

ifeq ($(USE_RAW_CAPTURE),yes)	
GENERIC_BINARIES_COMMON_SOURCE_FILES+=			\
    $(NDC_SRC_PATH)/raw_capture.c
endif

ifeq ($(PC_USE_POLARIS),yes)
   GENERIC_CFLAGS+=-DPC_USE_POLARIS
endif

GENERIC_INCLUDES+=					\
	$(SRC_DIR) \
	$(LIB_DIR) \
	$(SDK_PATH)/Soft/Common \
	$(SDK_PATH)/Soft/Lib

ifeq ($(PC_USE_POLARIS),yes)
   GENERIC_INCLUDES+=					\
       $(ALL_SOURCES)/mykonos/$(SDK_VERSION)/Mykonos/Soft/Lib  \
       $(ALL_SOURCES)/mykonos/$(SDK_VERSION)/Mykonos/Soft/Lib/libPolaris
endif

ifeq ($(USE_ARDRONE_CALIBRATION),yes)
   GENERIC_INCLUDES+=					\
       $(ALL_SOURCES)/mykonos/$(SDK_VERSION)/Mykonos/Soft/Lib   \
       $(ALL_SOURCES)/mykonos/$(SDK_VERSION)/Mykonos/Soft/Lib/libCalibration
endif

ifeq ($(USE_ARDRONE_VISION),yes)
   GENERIC_INCLUDES+=					\
       $(ALL_SOURCES)/mykonos/$(SDK_VERSION)/Mykonos/Soft/Lib   \
       $(ALL_SOURCES)/mykonos/$(SDK_VERSION)/Mykonos/Soft/Lib/Vision
endif

GENERIC_TARGET_BINARIES_PREFIX=

GENERIC_TARGET_BINARIES_DIR=$(ARDRONE_TARGET_DIR)

GENERIC_BINARIES_SOURCE_ENTRYPOINTS+=			\
	$(COMMON_MOB_SRC_PATH)/mobile_main.c

GENERIC_INCLUDES:=$(addprefix -I,$(GENERIC_INCLUDES))

ifeq ($(USE_ARDRONE_CALIBRATION),yes)
   GENERIC_CFLAGS+=-DPC_USE_CALIBRATION
endif

ifeq ($(USE_ARDRONE_VISION),yes)
   GENERIC_CFLAGS+=-DPC_USE_VISION
endif

GENERIC_LIB_PATHS=-L$(GENERIC_TARGET_BINARIES_DIR)
GENERIC_LIBS=-liw -lpc_ardrone -lgtk-x11-2.0 -lrt

ifeq ($(USE_ARDRONE_VISION),yes)
   GENERIC_LIBS+=-lpc_vision
endif

ifeq ($(USE_ARDRONE_CALIBRATION),yes)
   GENERIC_LIBS+=-lcalibration
endif

ifeq ($(PC_USE_POLARIS),yes)
   GENERIC_LIBS+=-lpc_polaris
endif

ifeq ($(WIIMOTE_SUPPORT),yes)
  GENERIC_LIBS+=-lpc_cwiid
endif
ifeq ($(EXTERNAL_CONTROL),yes)
  GENERIC_LIBS+=-lpc_ardrone_control
endif

SDK_FLAGS+="USE_APP=yes"
SDK_FLAGS+="APP_ID=ardrone_navigation"

GENERIC_CFLAGS+=-DRECORD_VIDEO

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


.PHONY: $(TARGET)

all: $(TARGET)

$(TARGET):
	@$(MAKE) -C $(SDK_PATH)/VP_SDK/Build $(TMP_SDK_FLAGS) $(SDK_FLAGS) $(MAKECMDGOALS)
	mv $(GENERIC_TARGET_BINARIES_DIR)/$(COMMON_MOB_SRC_PATH)/mobile_main $(TARGET)
	mv $(TARGET) $(ARDRONE_TARGET_DIR)/

$(MAKECMDGOALS):
	@$(MAKE) -C $(SDK_PATH)/VP_SDK/Build $(TMP_SDK_FLAGS) $(SDK_FLAGS) $(MAKECMDGOALS)

