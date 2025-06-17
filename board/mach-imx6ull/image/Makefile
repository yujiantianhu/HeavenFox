#
# Makefile
#
# File Name:   Makefile
# Author:      Yang Yujun
# E-mail:      <yujiantianhu@163.com>
# Created on:  2024.05.12
#
# Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
#

DDR	?=	512

IVTDCD_512MB	:=	imx6ull_512mb_ivtdcd.c
IVTDCD_256MB	:=	imx6ull_256mb_ivtdcd.c

BUILD_OPTION	:=	-DBINARY_NAME="\"HeavenFox.bin\""	\
					-DIMAGE_NAME="\"HeavenFox.img\""	\
					-DBINARY_PATH="\"../../../boot/image\""

ifeq ($(DDR),256)
all:
	g++ $(IVTDCD_256MB) image_tool.cpp -I. -O2 $(BUILD_OPTION) -o image_tool
else
all:
	g++ $(IVTDCD_512MB) image_tool.cpp -I. -O2 $(BUILD_OPTION) -o image_tool
endif

clean:
	rm -f image_tool

# end of file