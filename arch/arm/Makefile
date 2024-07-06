#
# Makefile
#
# File Name:   Makefile
# Author:      Yang Yujun
# E-mail:      <yujiantianhu@163.com>
# Created on:  2024.04.12
#
# Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
#

obj-y		+=	cpu/
obj-y		+=	kernel/
obj-y		+=	lib/

vector-y	:= 	lib/vectors.o
start-y		:=	cpu/$(TYPE)/start.o
head-y		:=	kernel/head.o

# get current path
arch_path	:= 	$(shell pwd)
arch_output	:=	$(patsubst $(PROJECT_DIR)%, $(OBJECT_PATH)%, $(arch_path))

head-objs	:=	$(vector-y) $(start-y) $(head-y)
depd-objs	:=	$(foreach file, $(head-objs), $(addsuffix .d, $(file)))

head-objs	:=	$(addprefix $(arch_path)/, $(head-objs))
head-objs	:=	$(patsubst $(PROJECT_DIR)%, $(OBJECT_PATH)%, $(head-objs))

ifneq ($(depd-objs),)
depd-objs	:=	$(addprefix $(arch_path)/, $(depd-objs))
depd-objs	:=	$(patsubst $(PROJECT_DIR)%, $(OBJECT_PATH)%, $(depd-objs))
depd-objs	:=	$(wildcard $(depd-objs))

include $(depd-objs)
endif

all: $(TARGET_IMGE)

$(TARGET_IMGE) : $(TARGET_EXEC)
	$(OBJCOPY) -O binary -S $(TARGET_EXEC) $@
	$(OBJDUMP) -D -m $(ARCH) $(TARGET_EXEC) > $(TARGET_NASM)

$(TARGET_EXEC) : $(head-objs) $(OBJECT_EXEC)
	$(CC) -T$(LINK_SCRIPT) $(BUILD_CFLAGS) $(OUTPUT_FLAGS) -o $@ $^ $(LIBS_PATH) $(LIBS)

$(arch_output)/%.o : %.c
	if [ ! -d $(arch_output) ]; then mkdir -p $(arch_output); fi
	$(CC) -Wp,-MD,$@.d -c $(BUILD_CFLAGS) $(MACROS) $(INCLUDE_DIRS) $(OUTPUT_FLAGS) -o $@ $<

$(arch_output)/%.o : %.S
	if [ ! -d $(arch_output) ]; then mkdir -p $(arch_output); fi
	$(CC) -Wp,-MD,$@.d -c $(BUILD_CFLAGS) $(MACROS) $(INCLUDE_DIRS) $(OUTPUT_FLAGS) -o $@ $<

clean:
	rm -rf $(head-objs) $(depd-objs)

distclean:
	rm -rf $(head-objs) $(depd-objs)
	rm -rf $(TARGET_EXEC) $(TARGET_IMGE) $(TARGET_NASM)

# end of file
