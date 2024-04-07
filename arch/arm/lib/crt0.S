/*
 * Assembly File For Board Initial
 *
 * File Name:   crt0.S
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.09
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#include <common/linkage.h>
#include <configs/mach_configs.h>

    .text
    .arm

    .global _main

_main:
    ldr sp, =CONFIG_BOARD_SP_ADDR               @ set stack address
    mov r12, sp
    bic r12, r12, #0x07                         @ 8 bits alignment
    mov sp, r12

    mov r0, sp
    bl board_init_f_alloc_reserve               @ reserve memory for allocating, and create global data
    mov sp, r0

    mov r9, r0                                  @ global data base address
    bl board_init_f_init_reserve                @ initial stack and global data
    mov r0, #0

    bl _bss_clear                               @ clear section .bss

    mov r0, #0
    bl board_init_f                             @ early initialize

    mov r0, #0
    ldr r10, =kernel_entry                      @ get kernel entry
    ldr lr, =board_init_r                       @ late initialize
    mov pc, lr

/*!< ----------------------------------------------------------- */
ENTRY(_bss_clear)
_bss_clear:
    mov r12, lr
    
    ldr r0, =__bss_start                        @ get __bss_start address, it is from real-kernel.lds
    ldr r1, =__bss_end                          @ get __bss_end address, it is from real-kernel.lds
    mov r2, #0x00

ENDPROC(_bss_clear)

/*!< ----------------------------------------------------------- */
ENTRY(_bss_clear_loop)
_bss_clear_loop:
    cmp r0, r1
    bhs _bss_clear_end                          @ if r0 >= r1, jump to _bss_clear_end
    b 1f
1:
    str r2, [r0]                                @ clear [r0]
    add r0, #0x04                               @ next bss address
    b _bss_clear_loop                           @ loop

ENDPROC(_bss_clear_loop)

/*!< ----------------------------------------------------------- */
ENTRY(_bss_clear_end)
_bss_clear_end:
    mov pc, r12                                 @ back to _main

ENDPROC(_bss_clear_end)

/*  end of file */
