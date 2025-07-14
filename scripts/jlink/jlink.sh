#!/bin/sh

# sudo /opt/SEGGER/JLink/JLinkGDBServerCLExe -device MCIMX6Y2 -JLinkScriptFile ./scripts/jlink/MCIMX6Y2/jlink_init.jlinkscript
# cd scripts/jlink
# arm-none-eabi-gdb
# source jlink.sh
# sudo luit -encoding gb2312 picocom /dev/ttyUSB0 -b 115200

target remote localhost:2331

set breakpoint auto-hw off
set arm fallback-mode arm

source ../../scripts/jlink/MCIMX6Y2/ddr_init.script
file ../../boot/image/HeavenFox.elf
load ../../boot/image/HeavenFox.elf

b _start
b start_kernel
