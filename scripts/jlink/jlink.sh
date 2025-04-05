#!/bin/sh

# cd scripts/jlink
# source jlink.sh

target remote localhost:2331

set breakpoint auto-hw off
set arm fallback-mode arm

source ../../scripts/jlink/MCIMX6Y2/ddr_init.script
file ../../boot/image/HeavenFox.elf
load ../../boot/image/HeavenFox.elf

b _start
b start_kernel
