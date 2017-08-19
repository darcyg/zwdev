CROSSTOOLDIR :=/opt/au/openwrt_7620
export	STAGING_DIR	:= $(CROSSTOOLDIR)/staging_dir
export	PATH :=$(PATH):$(STAGING_DIR)/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin
CROSS 	?= mipsel-openwrt-linux-
ARCH		?= mipsel

GCC 		?= $(CROSS)gcc
CXX			?= $(CROSS)g++
AR			?= $(CROSS)ar
AS			?= $(CROSS)gcc
RANLIB	?= $(CROSS)ranlib
STRIP 	?= $(CROSS)strip
OBJCOPY	?= $(CROSS)objcopy
OBJDUMP ?= $(CROSS)objdump
SIZE		?= $(CROSS)size
LD			?= $(CROSS)ld
MKDIR		?= mkdir -p

CROSS_CFLAGS			:= -I$(CROSSTOOLDIR)/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/usr/include
CROSS_LDFLAGHS		:= -L$(CROSSTOOLDIR)/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/usr/lib


TARGET_CFLAGS 		+= -Wall -g -O2 -I$(ROOTDIR)/inc -I$(ROOTDIR)/inc/ayla -I$(ROOTDIR)/platform -I$(ROOTDIR)/product/zwave/inc  $(CROSS_CFLAGS)

TARGET_CXXFLAGS 	+= $(TARGET_CFLAGS) -std=c++0x

TARGET_LDFLAGS 		+= -L$(ROOTDIR)/lib -lm -lrt -ldl -lpthread -lubus -lblobmsg_json -lubox -ljansson $(CROSS_LDFLAGHS)
#TARGET_LDFLAGS 	+= -L/usr/lib/ -ljansson
#TARGET_LDFLAGS		+= -lstdc++

