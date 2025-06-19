#!/bin/sh
#
# File Name:   image_load.sh
# Author:      Yang Yujun
# E-mail:      <yujiantianhu@163.com>
# Created on:  2025.06.19
#
# Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
#

# Source image file, write it to device (like "/dev/sda")
image_file=../image/HeavenFox.img

case $# in
    2 )
        image_file=$2;
        ;;
    1 )
        ;;
    * )
        echo "Input parameters are not correct! Please input './image_load [/dev/sd*]' [image]"
        exit;
        ;;
esac

dev_file=$1

if [ ! -c $dev_file -a ! -b $dev_file ]; then echo "$dev_file is not a device file, or not exists!"; exit; fi
if [ ! -f $image_file ]; then echo "Image $image_file not exists!"; exit; fi

sudo dd iflag=dsync oflag=dsync if=$image_file of=$dev_file bs=512 seek=2
echo "Download $image_file to $dev_file finished"
