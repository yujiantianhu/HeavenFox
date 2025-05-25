#!/bin/sh

# sudo /opt/SEGGER/JLink/JLinkGDBServerCLExe -device MCIMX6Y2 -JLinkScriptFile ./scripts/jlink/MCIMX6Y2/jlink_init.jlinkscript
# cd scripts/jlink
# arm-linux-gnueabihf-gdb
# source jlink.sh

target remote localhost:2331

set breakpoint auto-hw off
set arm fallback-mode arm

source ../../scripts/jlink/MCIMX6Y2/ddr_init.script
file ../../boot/image/HeavenFox.elf
load ../../boot/image/HeavenFox.elf

b _start
b start_kernel
