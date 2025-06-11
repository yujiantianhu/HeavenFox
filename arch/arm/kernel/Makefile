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

obj-y	+=	head-common.o

ifeq ($(CONFIG_CONTEXT_EX),y)
obj-y	+=	context2.o
else
obj-y	+=	context.o
endif

obj-y	+=	kernel.o

# end of file
