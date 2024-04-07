/*
 * ARM V7 Assembly File About Low Level Init
 *
 * File Name:   lowlevel_init.S
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#include <common/linkage.h>
#include <configs/mach_configs.h>

    .text
    .arm

    .global lowlevel_init

lowlevel_init:
    ldr sp, =CONFIG_BOARD_SP_ADDR               @ set stack address
    mov r12, sp
    bic r12, r12, #0x07                         @ 8 bits alignment
    mov sp, r12

    push {lr}                                   @ save current pc

    /*!< s_init: initial clock */
    /*!< only initial basic cpu configuration, it's not related to specfic board. */
    bl s_init                                   @ jump to arch/cpu/$(ARCH)/$(VENDOR)/board.c

    pop {lr}                                    @ restore previous pc
    mov pc, lr                                  @ retrun start.S

/*!< ----------------------------------------------------------- */
/*!< raise, do nothing. for gcc "udiv" ... "__aeabi_uldivmod" */
ENTRY(raise)
raise:
    nop
    mov pc, lr

ENDPROC(raise)

/* end of file */
