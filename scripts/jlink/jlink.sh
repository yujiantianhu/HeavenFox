#!/bin/sh

source scripts/jlink/MCIMX6Y2/ddr_init.script
file boot/gardens.elf
load boot/gardens.elf
delete 1-100
b _start
b start_kernel
