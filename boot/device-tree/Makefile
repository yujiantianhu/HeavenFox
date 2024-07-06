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

dtb-y 			:=

include $(VENDOR)/Makefile

TARGET			:=	$(dtb-y)
DT_DIRS			:=	$(shell pwd)
DT_FILE			:=	$(DT_DIRS)/$(VENDOR)

INCLUDES		:=	$(DT_DIRS)/	\
					$(DT_DIRS)/$(VENDOR)/	\
					$(DT_DIRS)/dt-bindings/

DTC_INCLUDES	:= 	$(patsubst %, -i %, $(INCLUDES))
CXX_INCLUDES	:=	$(patsubst %, -I %, $(INCLUDES))

DTC				:=	$(PROJECT_DIR)/scripts/dtc/dtc
BUILD_FLAGS		:=	-nostdinc -undef -D__DTS__ -x assembler-with-cpp

all:
	cpp -Wp,-MD,$(DT_FILE)/.$(TARGET).dts.d $(BUILD_FLAGS) $(CXX_INCLUDES) -o $(DT_FILE)/.$(TARGET).dts.temp $(DT_FILE)/$(TARGET).dts
	$(DTC) -I dts -O dts $(DTC_INCLUDES) -o $(DT_FILE)/.$(TARGET).dts $(DT_FILE)/.$(TARGET).dts.temp
	$(DTC) -I dts -O dtb -o $(DT_FILE)/$(TARGET).dtb $(DT_FILE)/.$(TARGET).dts
	cp $(DT_FILE)/$(TARGET).dtb ../firmware.dtb

clean:
	rm -rf $(DT_FILE)/.$(TARGET).dts.temp 
	rm -rf $(DT_FILE)/.$(TARGET).dts.d 
	rm -rf $(DT_FILE)/.$(TARGET).dts

distclean:
	rm -rf ../firmware.dtb
	rm -rf $(DT_FILE)/$(TARGET).dtb 
	rm -rf $(DT_FILE)/.$(TARGET).dts.temp 
	rm -rf $(DT_FILE)/.$(TARGET).dts.d 
	rm -rf $(DT_FILE)/.$(TARGET).dts

check:
	$(DTC) -q -I dts -O dtb $(DTC_INCLUDES) $(TARGET).dts

version:
	$(DTC) -v

# end of file
