
#########################################################
# Common build definitions (CUSTOM)
#########################################################

SDK_VERSION          = head
RELEASE_BUILD        = no
QUIET_BUILD          = yes

#########################################################
# System utility definitions (STATIC)
#########################################################
define CHECK_DEFINITION
  ifndef $(1)
    $$(warning ERROR : $(1) undefined $(2))
    ERROR=1
  endif
endef

define CHECK_UNDEFINITION
  ifdef $(1)
    $$(warning ERROR : $(1) defined $(2))
    ERROR=1
  endif
endef

define EXIT_IF_ERROR
  ifeq "$$(ERROR)" "1"
    $$(error There has been some errors)
  endif
endef


#########################################################
# Validity control (STATIC)
#########################################################

ifdef PC_TARGET
  $(eval $(call CHECK_UNDEFINITION,CONSOLE_TARGET,(should not be defined when PC_TARGET is defined)))
endif
$(eval $(call CHECK_DEFINITION,SDK_SOURCES,: should be defined as an environment variable.))

$(eval $(call EXIT_IF_ERROR))


#########################################################
# Common definitions (STATIC)
#########################################################

ifeq "$(QUIET_BUILD)" "yes"
  MAKE=@make -s
else
  MAKE=make
endif

ifeq "$(RELEASE_BUILD)" "yes"
  ARDRONE_TARGET_DIR=$(SDK_SOURCES)/Soft/Build/Version/Release
else
  ARDRONE_TARGET_DIR=$(SDK_SOURCES)/Soft/Build/Version/Debug
endif

COMMON_DIR:=../Common

SDK_FLAGS:=-C $(SDK_SOURCES)/VP_SDK/Build $(TMP_SDK_FLAGS)
SDK_FLAGS+="NO_EXAMPLES=yes"
SDK_FLAGS+="USE_SDK=yes"
SDK_FLAGS+="QUIET_BUILD=$(QUIET_BUILD)"
SDK_FLAGS+="RELEASE_BUILD=$(RELEASE_BUILD)"
SDK_FLAGS+="SDK_VERSION=$(SDK_VERSION)"

ifeq ($(filter NO_COM=%,$(TMP_SDK_FLAGS)),)
  SDK_FLAGS+="NO_COM=no"
endif

#########################################################
# PC_TARGET specific definitions (STATIC)
#########################################################
ifdef PC_TARGET
  SDK_FLAGS+="NO_COM=no"

  ifeq ($(ARDRONE_TARGET_OS),Linux)
    OS_DEFINE=GNU_LINUX
  else
   ifeq ($(ARDRONE_TARGET_OS),iphoneos)
    OS_DEFINE=GNU_LINUX
   else
      ifeq ($(ARDRONE_TARGET_OS),iphonesimulator)
         OS_DEFINE=GNU_LINUX
      else
         TARGET:=$(TARGET).exe
         OS_DEFINE=WINDOW
      endif
    endif
  endif

  GENERIC_CFLAGS+=-D_MOBILE

  ifeq ("$(PC_USE_TABLE_PILOTAGE)","yes")
    GENERIC_CFLAGS+=-DUSE_TABLE_PILOTAGE
  endif

  ifeq ("$(RECORD_VIDEO)","yes")
    GENERIC_CFLAGS+=-DRECORD_VIDEO
  endif

  GENERIC_CFLAGS+=-D$(OS_DEFINE)
  ifeq ($(IPHONE_MODE),yes)
     ifeq ($(ARDRONE_TARGET_OS),iphoneos)
        GENERIC_CFLAGS+=-DTARGET_OS_IPHONE
     else
        GENERIC_CFLAGS+=-DTARGET_IPHONE_SIMULATOR
     endif
  endif

  ifneq ("$(USE_MINGW32)","yes")
    GENERIC_CFLAGS+=$(shell pkg-config --cflags gtk+-2.0)
    GENERIC_LIBS+=$(shell pkg-config --libs gtk+-2.0)
  endif

  ifeq ($(ARDRONE_TARGET_OS),Linux)
	SDK_FLAGS+="USE_LINUX=yes"
  else
	SDK_FLAGS+="USE_LINUX=no"
  endif
  
  SDK_FLAGS+="USE_ELINUX=no"
  
  ifneq ($(findstring iphone,$(ARDRONE_TARGET_OS)),)
	SDK_FLAGS+="USE_IPHONE=yes"
  	SDK_FLAGS+="IPHONE_PLATFORM=$(ARDRONE_TARGET_OS)"
  else
	SDK_FLAGS+="USE_IPHONE=no"
  endif
  SDK_FLAGS+="IPHONE_SDK_VERSION=$(CURRENT_IPHONE_SDK_VERSION)"

  SDK_FLAGS+="USE_NDS=no"

  ifeq ($(filter USE_BLUEZ=%,$(TMP_SDK_FLAGS)),)
    SDK_FLAGS+="USE_BLUEZ=no"
  endif

  SDK_FLAGS+="USE_VLIB=yes"
  SDK_FLAGS+="USE_BONJOUR=no"
  SDK_FLAGS+="USE_WIFI=yes"
  
  SDK_FLAGS+="USE_BROADCOM=no"
  SDK_FLAGS+="USE_IWLIB=yes"

  SDK_FLAGS+="FF_ARCH=Intel"

  SDK_FLAGS+="USE_PARROTOS_CORE=no"
  SDK_FLAGS+="USE_PARROTOS_DRIVERS=no"
  SDK_FLAGS+="USE_PARROTOS_DEVS=no"
  SDK_FLAGS+="USE_PARROTOS_CODEC=no"

  
  SDK_FLAGS+="USE_ARDRONELIB=yes"
  SDK_FLAGS+="USE_ARDRONE_VISION=yes"
  SDK_FLAGS+="USE_ARDRONE_POLARIS=no"
  SDK_FLAGS+="USE_ARDRONE_TEST_BENCHS=no"
  SDK_FLAGS+="USE_ARDRONE_CALIBRATION=no"

endif

