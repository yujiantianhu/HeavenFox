/*
 * Assembly File For Memory Operation
 *
 * File Name:   memory.S
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.11.12
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#include <common/linkage.h>
#include <configs/mach_configs.h>

    .text
    .arm
    .align 2

/*!
 * @param:  r0 (&start_addr), r7 (start_addr)
 * @param:  r1 (end_addr)
 * @param:  r2 (data)
 */
ENTRY(__memset_ex)
__memset_ex:
    push {r5 - r7, lr}
    ldr r7, [r0]                                @ Get start_addr
    nop
    bic r6, r1, #0x3                            @ r6 is 4 bytes alignment
    mov r5, #24

1:
    cmp r7, r6                                  @ Loop1: write [r7] with 4 bytes
    bhs 2f                                      @ r7 >= r6 ?
    str r2, [r7]                                @ *r7 = r2
    add r7, r7, #4
    b 1b
    
2:
    cmp r7, r1                                  @ Loop2: write [r7] with 1 byte (only loop 3 times)
    bhs 3f
    asr r6, r2, r5                              @ r6 = r2 >> r5
    strb r6, [r7]                               @ *r7 = r6 & 0x000000ff
    add r7, r7, #1
    sub r5, r5, #8                              @ r5 = 24/16/8/0
    b 2b

3:
    nop
    str r7, [r0]
    pop {r5 - r7, lr}
    bx lr

ENDPROC(__memset_ex)

/*  end of file */
