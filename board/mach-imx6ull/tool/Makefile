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

# Usage: make IMG_PATH="\"img_path\"" IMAGE="\"test.img\"" BIN_PATH="\"bin_path\"" BIN="\"test.bin\""

DDR	        ?=	512
BIN_OFFSET  ?=  3072
IMAGE       ?=  "\"HeavenFox.img\""
IMG_PATH    ?=  "\"../image\""
BIN         ?=  "\"HeavenFox.bin\""
BIN_PATH    ?=  "\"../../../boot/image\""

ifeq ($(DDR),256)
IVTDCD	    :=	imx6ull_256mb_ivtdcd.c
else
IVTDCD	    :=	imx6ull_512mb_ivtdcd.c
endif

BUILD_OPTION	:=	-DBIN_OFFSET=$(BIN_OFFSET)	\
					-DIMAGE_NAME=$(IMAGE)	\
					-DIMAGE_PATH=$(IMG_PATH)	\
					-DBINARY_NAME=$(BIN)	\
					-DBINARY_PATH=$(BIN_PATH)

all:
	g++ $(IVTDCD) image_tool.cpp -I. -O2 $(BUILD_OPTION) -o image_tool

clean:
	rm -f image_tool

# end of file
