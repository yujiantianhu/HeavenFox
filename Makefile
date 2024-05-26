#
# Kernel Top Makefile
#
# File Name:   Makefile
# Author:      Yang Yujun
# E-mail:      <yujiantianhu@163.com>
# Created on:  2023.09.09
#
# Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
#

TARGET			:=	HeavenFox

MAKE			:=	make
Q				:=	
ARCH			:= 	arm
TYPE			:=	armv7
VENDOR			:=	imx6ull
COMPILER_VER	:=	gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf

COMPILER_BIN	:=  
COMPILER		:= 	$(COMPILER_BIN)arm-linux-gnueabihf-
CC				:= 	$(COMPILER)gcc
CXX				:=  $(COMPILER)g++
LD				:= 	$(COMPILER)ld
AR				:=	$(COMPILER)ar
OBJDUMP			:= 	$(COMPILER)objdump
OBJCOPY			:= 	$(COMPILER)objcopy
READELF			:= 	$(COMPILER)readelf

COMPILER_PATH	:=  /usr/local/arm/$(COMPILER_VER)
COMPILER_LIBC	:=	$(COMPILER_PATH)/arm-linux-gnueabihf/libc

LIBS_PATH		:=  -L $(COMPILER_LIBC)/usr/lib	\
					-L $(COMPILER_LIBC)/lib

LIBS			:=	--static -lgcc -lm -lc
OUTPUT_FLAGS	:=  -fexec-charset=GB2312

BUILD_CFLAGS   	:=  -g3 -O0 -Wall -nostdlib	\
					-Wundef	\
					-Wstrict-prototypes	\
					-Wno-trigraphs \
                    -fno-strict-aliasing	\
					-fno-common \
                    -Werror-implicit-function-declaration \
                    -fno-tree-scev-cprop

MACROS			:=	-DCONFIG_DEBUG_JTAG	\
					-DCONFIG_MACH_IMX6ULL_TOPPET

PROJECT_DIR		:=	$(shell pwd)
OBJECT_PATH		:=	$(PROJECT_DIR)/objects

OUTPUT_PATH		:=	$(PROJECT_DIR)/boot
LINK_SCRIPT		:=	$(PROJECT_DIR)/arch/$(ARCH)/cpu_ramboot.lds
DTC				:=	$(PROJECT_DIR)/scripts/dtc/dtc
BUILD_SCRIPT	:=	$(PROJECT_DIR)/scripts/Makefile.build

TARGET_EXEC		:=	$(OUTPUT_PATH)/$(TARGET).elf
TARGET_IMGE		:=	$(OUTPUT_PATH)/$(TARGET).img
TARGET_NASM		:=	$(OUTPUT_PATH)/$(TARGET).dis

INCLUDE_DIRS	:= 	$(PROJECT_DIR)/	\
					$(PROJECT_DIR)/arch/$(ARCH)/include	\
					$(PROJECT_DIR)/include	\
					$(PROJECT_DIR)/board/$(VENDOR)	\
					$(PROJECT_DIR)/rootfs	\
					$(PROJECT_DIR)/rootfs/fatfs

ARCH_DIRS       :=  arch/$(ARCH)/
COMMON_DIRS     :=  common/
BOOT_DIRS       :=  boot/
BOARD_DIRS      :=  board/
PLATFORM_DIRS   :=  platform/
KERNEL_DIRS     :=  kernel/
ROOTFS_DIRS     :=  rootfs/
DRIVER_DIRS     :=  drivers/
INIT_DIRS       :=  example/ init/

OBJECT_EXEC		:=	$(OBJECT_PATH)/built-in.o
SOURCE_DIRS		:=	$(ARCH_DIRS) $(COMMON_DIRS) $(BOOT_DIRS) $(BOARD_DIRS) $(PLATFORM_DIRS)	\
					$(KERNEL_DIRS) $(ROOTFS_DIRS) $(DRIVER_DIRS) $(INIT_DIRS)
INCLUDE_DIRS	:= 	$(patsubst %, -I %, $(INCLUDE_DIRS))

export ARCH TYPE VENDOR CC CXX LD AR OBJDUMP OBJCOPY READELF
export LIBS_PATH LIBS OUTPUT_FLAGS BUILD_CFLAGS MACROS
export PROJECT_DIR LINK_SCRIPT DTC BUILD_SCRIPT INCLUDE_DIRS
export OBJECT_PATH OBJECT_EXEC TARGET_EXEC TARGET_IMGE TARGET_NASM

obj-y			+=	$(SOURCE_DIRS)

VPATH			:= 	$(SOURCE_DIRS)
.PHONY:			all clean distclean dtbs debug

all : $(OBJECT_EXEC)
	$(Q)$(MAKE) -C $(ARCH_DIRS)

$(OBJECT_EXEC):
	$(Q)$(MAKE) -C $(PROJECT_DIR) -f $(BUILD_SCRIPT)

clean:
	$(Q)$(MAKE) -C $(ARCH_DIRS) clean
	$(Q)$(MAKE) -f $(BUILD_SCRIPT) clean
	rm -rf $(OBJECT_EXEC)

distclean:
#	$(Q)$(MAKE) -C $(OUTPUT_PATH)/device-tree distclean
	$(Q)$(MAKE) -C $(ARCH_DIRS) distclean
	$(Q)$(MAKE) -f $(BUILD_SCRIPT) distclean

dtbs:
	$(Q)$(MAKE) -C $(OUTPUT_PATH)/device-tree

debug:
	$(Q)$(MAKE) -f $(BUILD_SCRIPT) debug

# end of file
