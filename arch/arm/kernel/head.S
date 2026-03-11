/*
 * ARM V7 Kernel Boot Assembly File
 *
 * File Name:   head.S
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
    .section ".head", "ax"

    .global kernel_entry

/*!< for 32-bits (4 bytes), 4 = 2 ^ ARCH_PER_OFFSET */
#define ARCH_PER_OFFSET                             2

/*!<
 * it can set cpu mode by changing the lower 5 bits of the cpsr register:
 *  USE: 0x10;  FIQ: 0x11;  IRQ: 0x12;  SVC: 0x13   MON: 0x16
 *  ABT: 0x17;  HYP: 0x1a;  UND: 0x1b;  SYS: 0x1f
 */

kernel_entry:
    mov r4, r0                                      @ save variable
    mov r5, r1
    mov r6, r2

    cpsid i                                         @ close IRQ
    cpsid f                                         @ close FIQ

    b _kernel_reset

/*!< ----------------------------------------------------------- */
/*!< Defines variable for stack; It needs combine board_init_r.c to analysis */
/*!< ----------------------------------------------------------- */
.global _UND_MODE_STACK_BASE
_UND_MODE_STACK_BASE:
    .word 0x0123457c

.global _ABT_MODE_STACK_BASE
_ABT_MODE_STACK_BASE:
    .word 0x01234578

.global _FIQ_MODE_STACK_BASE
_FIQ_MODE_STACK_BASE:
    .word 0x01234574

.global _IRQ_MODE_STACK_BASE
_IRQ_MODE_STACK_BASE:
    .word 0x01234570

.global _SYS_MODE_STACK_BASE
_SYS_MODE_STACK_BASE:
    .word 0x0123456c

.global _SVC_MODE_STACK_BASE
_SVC_MODE_STACK_BASE:
    .word 0x01234568

/*!< cp15 initial */
ENTRY(kernel_init_cp15)
kernel_init_cp15:
    mov r12, lr

    READ_CP15_SCTLR(r0)
    bic r0, r0, #CP15_SCTLR_BIT_M                   @ disable mmu
    orr r0, r0, #CP15_SCTLR_BIT_C                   @ enable D-Cache
    orr r0, r0, #CP15_SCTLR_BIT_I                   @ enable I-Cache
    orr r0, r0, #CP15_SCTLR_BIT_Z                   @ enable branch prediction
    bic r0, r0, #CP15_SCTLR_BIT_A                   @ disable alignment check
    WRITE_CP15_SCTLR(r0)

    mov r0, #0
    WRITE_CP15_TPIDRPRW(r0)                         @ reset tpidrprw (it will be used to save thread's context)

    mov pc, r12

ENDPROC(kernel_init_cp15)

/*!< ----------------------------------------------------------- */
ENTRY(_kernel_reset)
_kernel_reset:
    cps #ARCH_IRQ_MODE                              @ set the cpu to IRQ mode
@   ldr sp, _IRQ_MODE_STACK_BASE                    @ set IRQ mode stack
    READ_CP15_MPIDR(r0)                             @ get cpu id
    and r0, r0, #CP15_MPIDR_CPU_ID_MASK
    ldr r1, =g_IRQ_MODE_STACK_BASE                  @ get &g_IRQ_MODE_STACK_BASE[0]
    add r1, r1, r0, lsl #ARCH_PER_OFFSET            @ get &g_IRQ_MODE_STACK_BASE[cpuid]
    ldr sp, [r1]                                    @ set IRQ mode stack: g_IRQ_MODE_STACK_BASE[cpuid]
    mrs r2, cpsr                                    @ read cpsr register to r0
    bic r2, r2, #CPSR_BIT_E                         @ set to littile endian
    msr spsr_fsxc, r2

    /*!< Jtag can not run with another mode */
@#if !defined(CONFIG_DEBUG)

    cps #ARCH_FIQ_MODE                              @ set the cpu to FIQ mode
@   ldr sp, _FIQ_MODE_STACK_BASE                    @ set FIQ mode stack
    READ_CP15_MPIDR(r0)                             @ get cpu id
    and r0, r0, #CP15_MPIDR_CPU_ID_MASK
    ldr r1, =g_FIQ_MODE_STACK_BASE                  @ get &g_FIQ_MODE_STACK_BASE[0]
    add r1, r1, r0, lsl #ARCH_PER_OFFSET            @ get &g_FIQ_MODE_STACK_BASE[cpuid]
    ldr sp, [r1]                                    @ set FIQ mode stack: g_FIQ_MODE_STACK_BASE[cpuid]
    mrs r2, cpsr                                    @ read cpsr register to r0
    bic r2, r2, #CPSR_BIT_E                         @ set to littile endian
    msr spsr_fsxc, r2

    cps #ARCH_UND_MODE                              @ set the cpu to UND mode
@   ldr sp, _UND_MODE_STACK_BASE                    @ set UND mode stack
    READ_CP15_MPIDR(r0)                             @ get cpu id
    and r0, r0, #CP15_MPIDR_CPU_ID_MASK
    ldr r1, =g_UND_MODE_STACK_BASE                  @ get &g_UND_MODE_STACK_BASE[0]
    add r1, r1, r0, lsl #ARCH_PER_OFFSET            @ get &g_UND_MODE_STACK_BASE[cpuid]
    ldr sp, [r1]                                    @ set UND mode stack: g_UND_MODE_STACK_BASE[cpuid]
    mrs r2, cpsr                                    @ read cpsr register to r0
    bic r2, r2, #CPSR_BIT_E                         @ set to littile endian
    msr spsr_fsxc, r2

    cps #ARCH_SYS_MODE                              @ set the cpu to SYS mode
@   ldr sp, _SYS_MODE_STACK_BASE                    @ set SYS mode stack
    READ_CP15_MPIDR(r0)                             @ get cpu id
    and r0, r0, #CP15_MPIDR_CPU_ID_MASK
    ldr r1, =g_SYS_MODE_STACK_BASE                  @ get &g_SYS_MODE_STACK_BASE[0]
    add r1, r1, r0, lsl #ARCH_PER_OFFSET            @ get &g_SYS_MODE_STACK_BASE[cpuid]
    ldr sp, [r1]                                    @ set SYS mode stack: g_SYS_MODE_STACK_BASE[cpuid]
    mrs r2, cpsr                                    @ read cpsr register to r0
    bic r2, r2, #CPSR_BIT_E                         @ set to littile endian
    msr spsr_fsxc, r2

@#endif

    cps #ARCH_ABT_MODE                              @ set the cpu to ABT mode
@   ldr sp, _ABT_MODE_STACK_BASE                    @ set ABT mode stack
    READ_CP15_MPIDR(r0)                             @ get cpu id
    and r0, r0, #CP15_MPIDR_CPU_ID_MASK
    ldr r1, =g_ABT_MODE_STACK_BASE                  @ get &g_ABT_MODE_STACK_BASE[0]
    add r1, r1, r0, lsl #ARCH_PER_OFFSET            @ get &g_ABT_MODE_STACK_BASE[cpuid]
    ldr sp, [r1]                                    @ set ABT mode stack: g_ABT_MODE_STACK_BASE[cpuid]
    mrs r2, cpsr                                    @ read cpsr register to r0
    bic r2, r2, #CPSR_BIT_E                         @ set to littile endian
    msr spsr_fsxc, r2

    /*!< switch to SVC mode */
    /*!< kernel will run in SVC mode to initial all resources */
    cps #ARCH_SVC_MODE                              @ set the cpu to SVC mode
@   ldr sp, _SVC_MODE_STACK_BASE                    @ set SVC mode stack
    READ_CP15_MPIDR(r0)                             @ get cpu id
    and r0, r0, #CP15_MPIDR_CPU_ID_MASK
    ldr r1, =g_SVC_MODE_STACK_BASE                  @ get &g_SVC_MODE_STACK_BASE[0]
    add r1, r1, r0, lsl #ARCH_PER_OFFSET            @ get &g_SVC_MODE_STACK_BASE[cpuid]
    ldr sp, [r1]                                    @ set SVC mode stack: g_SVC_MODE_STACK_BASE[cpuid]
    mrs r2, cpsr                                    @ read cpsr register to r0
    bic r2, r2, #CPSR_BIT_E                         @ set to littile endian
    bic r2, r2, #CPSR_BIT_A                         @ enable abort
@   orr r2, r2, #CPSR_BIT_A                         @ disable abort
    msr cpsr, r2

    bl kernel_init_cp15                             @ initialize cp15
    b _kernel_start                                 @ jump to head-common.S (configure mmu)

ENDPROC(_kernel_reset)

/*!< ----------------------------------------------------------- */
ENTRY(_slave_reset)
_slave_reset:
    cpsid i                                         @ close IRQ
    cpsid f                                         @ close FIQ

1:
    wfe                                             @ wait for signal from master cpu
    READ_CP15_MPIDR(r0)                             @ get cpu id
    and r0, r0, #CP15_MPIDR_CPU_ID_MASK
    ldr r1, =g_percpu_slave_wake                    @ get &g_percpu_slave_wake[0]
    add r1, r1, r0, lsl #ARCH_PER_OFFSET            @ get &g_percpu_slave_wake[cpuid]
    ldr r2, [r1]                                    @ get g_percpu_slave_wake[cpuid]
    cmp r2, #1                                      @ check if cpu wake up
    bne 1b                                          @ not wake

@   mov r2, #0
@   str r2, [r1]                                    @ clear wake flag
    b _kernel_reset

ENDPROC(_slave_reset)

/* end of file */
