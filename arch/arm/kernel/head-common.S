/*
 * Kernel Boot Assembly File
 *
 * File Name:   head-common.S
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

    .extern start_kernel
    .global _kernel_start

_kernel_start:
    push { r4 - r6 }
    ldr r1, =setup_thread
    blx r1

    pop { r4 - r6 }

    READ_CP15_MPIDR(r0)                             @ get cpu id
    and r0, r0, #CP15_MPIDR_CPU_ID_MASK
    cmp r0, #CONFIG_CORE_MASTER
    beq 1f

    ldr r12, =secondary_start_kernel
    mov pc, r12

1:
    b _mmu_page_init

/*!< ----------------------------------------------------------- */
/*!< mmu page configure */
ENTRY(_mmu_page_init)
_mmu_page_init:
    mov r0, r4
    mov r1, r5
    mov r2, r6

    ldr r12, =start_kernel
    mov pc, r12

ENDPROC(_mmu_page_init)

/*!< end of file */
