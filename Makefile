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

TARGET			:=	real-kernel

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
LINK_SCRIPT		:=	$(OUTPUT_PATH)/$(TARGET).lds

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
export PROJECT_DIR LINK_SCRIPT OBJECT_PATH INCLUDE_DIRS
export OBJECT_EXEC TARGET_EXEC TARGET_IMGE TARGET_NASM

obj-y			+=	$(SOURCE_DIRS)

VPATH			:= 	$(SOURCE_DIRS)
.PHONY:			all clean debug

obj: $(OBJECT_EXEC)

all : $(OBJECT_EXEC)
	$(Q)$(MAKE) -C $(ARCH_DIRS)

$(OBJECT_EXEC):
	$(Q)$(MAKE) -C $(PROJECT_DIR) -f $(PROJECT_DIR)/scripts/Makefile.build

clean:
	$(Q)$(MAKE) -C $(ARCH_DIRS) clean
	$(Q)$(MAKE) -f $(PROJECT_DIR)/scripts/Makefile.build clean

debug:
	$(Q)$(MAKE) -f $(PROJECT_DIR)/scripts/Makefile.build debug

# end of file
