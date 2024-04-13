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
OBJECT_MANAGE	:=	"true"

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
					-Wundef -Wstrict-prototypes -Wno-trigraphs \
                    -fno-strict-aliasing -fno-common \
                    -Werror-implicit-function-declaration \
                    -fno-tree-scev-cprop

MACROS			:=	-DCONFIG_DEBUG_JTAG	\
					-DCONFIG_MACH_IMX6ULL_TOPPET

PROJECT_DIR		:=	$(shell pwd)
OUTPUT_PATH		:=	$(PROJECT_DIR)/boot

ifneq ($(OBJECT_MANAGE), "true")
OBJECT_PATH		:=	$(PROJECT_DIR)/objects/
else
OBJECT_PATH		:=	
endif

export ARCH TYPE CC CXX LD AR OBJDUMP OBJCOPY READELF
export LIBS_PATH LIBS OUTPUT_FLAGS BUILD_CFLAGS MACROS

TARGET_EXEC		:=	$(PROJECT_DIR)/boot/$(TARGET).elf
TARGET_IMGE		:=	$(PROJECT_DIR)/boot/$(TARGET).img
TARGET_NASM		:=	$(PROJECT_DIR)/boot/$(TARGET).dis

LINK_SCRIPT		:=	$(PROJECT_DIR)/boot/$(TARGET).lds

INCLUDE_DIRS	:= 	$(PROJECT_DIR)/	\
					$(PROJECT_DIR)/arch/$(ARCH)/include	\
					$(PROJECT_DIR)/include	\
					$(PROJECT_DIR)/board/$(VENDOR)	\
					$(PROJECT_DIR)/rootfs	\
					$(PROJECT_DIR)/rootfs/fatfs

ASSEMBLY_DIRS	:=	$(PROJECT_DIR)/arch/$(ARCH)/lib	\
					$(PROJECT_DIR)/arch/$(ARCH)/cpu/$(TYPE) \
					$(PROJECT_DIR)/arch/$(ARCH)/kernel
					
COMMON_DIRS     :=  $(PROJECT_DIR)/common/generic	\
					$(PROJECT_DIR)/common/mempool
ARCH_DIRS       :=  $(PROJECT_DIR)/arch/$(ARCH)/lib	\
					$(PROJECT_DIR)/arch/$(ARCH)/cpu/$(TYPE) \
					$(PROJECT_DIR)/arch/$(ARCH)/cpu/$(TYPE)/$(VENDOR)
BOOT_DIRS       :=  $(PROJECT_DIR)/boot	\
					$(PROJECT_DIR)/boot/dynamic
BOARD_DIRS      :=  $(PROJECT_DIR)/board/mach-$(VENDOR)
PLATFORM_DIRS   :=  $(PROJECT_DIR)/platform	\
					$(PROJECT_DIR)/platform/mem	\
					$(PROJECT_DIR)/platform/mmc	\
					$(PROJECT_DIR)/platform/irq	\
					$(PROJECT_DIR)/platform/of	\
					$(PROJECT_DIR)/platform/bus_type	\
					$(PROJECT_DIR)/platform/chardev	\
					$(PROJECT_DIR)/platform/block_device	\
					$(PROJECT_DIR)/platform/net_device	\
					$(PROJECT_DIR)/platform/gpio_desc	\
					$(PROJECT_DIR)/platform/framebuffer	\
					$(PROJECT_DIR)/platform/usb	\
					$(PROJECT_DIR)/platform/usb/host
KERNEL_DIRS     :=  $(PROJECT_DIR)/kernel	\
					$(PROJECT_DIR)/kernel/kthread
ROOTFS_DIRS     :=  $(PROJECT_DIR)/rootfs	\
					$(PROJECT_DIR)/rootfs/fatfs	\
					$(PROJECT_DIR)/rootfs/option
DRIVER_DIRS     :=  $(PROJECT_DIR)/drivers/clk	\
					$(PROJECT_DIR)/drivers/gpio	\
					$(PROJECT_DIR)/drivers/video	\
					$(PROJECT_DIR)/drivers/usb/gadget
INIT_DIRS       :=  $(PROJECT_DIR)/example	\
					$(PROJECT_DIR)/init

SOURCE_DIRS		:=	$(COMMON_DIRS) $(ARCH_DIRS) $(BOOT_DIRS) $(BOARD_DIRS)  \
                    $(PLATFORM_DIRS) $(KERNEL_DIRS) $(ROOTFS_DIRS) $(DRIVER_DIRS) $(INIT_DIRS)

INCLUDE_DIRS	:= 	$(patsubst %, -I %, $(INCLUDE_DIRS))
SOURCES_ABS		:= 	$(foreach dir, $(SOURCE_DIRS), $(wildcard $(dir)/*.c))
SOURCES		    := 	$(notdir $(SOURCES_ABS))
SRC_OBJECTS		:=	$(patsubst %, $(OBJECT_PATH)%, $(SOURCES:.c=.o))
ASSEMBLY_ABS	:=	$(foreach dir, $(ASSEMBLY_DIRS), $(wildcard $(dir)/*.S))
ASSEMBLIES		:=	$(notdir $(ASSEMBLY_ABS))
ASM_OBJECTS		:=	$(patsubst %, $(OBJECT_PATH)%, $(ASSEMBLIES:.S=.o))
OBJECTS		    :=	$(ASM_OBJECTS) $(SRC_OBJECTS)

VPATH			:= 	$(ASSEMBLY_DIRS) $(SOURCE_DIRS)
.PHONY:			all clean debug

all: $(TARGET_IMGE)
$(TARGET_IMGE) : $(TARGET_EXEC)
	$(OBJCOPY) -O binary -S $(TARGET_EXEC) $@
	$(OBJDUMP) -D -m $(ARCH) $(TARGET_EXEC) > $(TARGET_NASM)

$(TARGET_EXEC): $(OBJECTS)
	$(LD) -T$(LINK_SCRIPT) -o $(TARGET_EXEC) $(OBJECTS) $(LIBS_PATH) $(LIBS)

$(ASM_OBJECTS): $(OBJECT_PATH)%.o : %.S
	$(CC) -c $(BUILD_CFLAGS) $(MACROS) $(INCLUDE_DIRS) $(OUTPUT_FLAGS) -o $@ $<

$(SRC_OBJECTS): $(OBJECT_PATH)%.o : %.c
	$(CC) -c $(BUILD_CFLAGS) $(MACROS) $(INCLUDE_DIRS) $(OUTPUT_FLAGS) -o $@ $<

clean:
	rm -rf $(TARGET_EXEC) $(TARGET_IMGE) $(TARGET_NASM)
	rm -rf $(OBJECTS)

debug:
	@echo $(INCLUDE_DIRS)

# end of file
