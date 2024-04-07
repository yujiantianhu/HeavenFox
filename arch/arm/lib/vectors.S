/*
 * Assembly File About Vector Table
 * The first file that will be exec
 *
 * File Name:   vectors.S
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.09
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#include <common/linkage.h>
#include <configs/mach_configs.h>

    .arch armv7-a
    .text
    .arm
    .section ".vectors", "ax"

    .global _start

/* -------------------------------------------------------------------------------
 * Exception vectors defines
 * -----------------------------------------------------------------------------*/
    .global _undefined_handler
    .global _software_irq_handler
    .global _prefetch_abort_handler
    .global _data_abort_handler
    .global _unused_handler
    .global _irq_handler
    .global _fiq_handler

/* -------------------------------------------------------------------------------
 * Exception vectors table
 * -----------------------------------------------------------------------------*/
_start:
    b   _reset                                      @ address offset: 0x00, reset, please jump to start.S
    ldr pc, =_undefined_handler                     @ address offset: 0x04, undefined instruction abort exception
    ldr pc, =_software_irq_handler                  @ address offset: 0x08, soft interrupt
    ldr pc, =_prefetch_abort_handler                @ address offset: 0x0C, prefetch instruction abort exception
    ldr pc, =_data_abort_handler                    @ address offset: 0x10, data abort exception
    ldr pc, =_unused_handler                        @ address offset: 0x14, unused, reserved
    ldr pc, =_irq_handler                           @ address offset: 0x18, general interrupt, like external irq
    ldr pc, =_fiq_handler                           @ address offset: 0x1C, fast interrupt

/* -------------------------------------------------------------------------------
 * Save The Current Filed Context 
 * -----------------------------------------------------------------------------*/
.macro _exception_save_params
    sub sp, sp, #ARCH_FRAME_SIZE                    @ leave frame size of space for registers

    stmia sp, {r0 - r12}                            @ push r0 ~ r12 to sp (basic address)
    add r8, sp, #ARCH_OFFSET_PC                     @ r8 = offset of pc, it is also lr reg's top address
    add r7, sp, #ARCH_OFFSET_PSR                    @ r7 = offset of spsr
    stmdb r8, {sp, lr}                              @ save lr to stack first, and then save sp
    str lr, [r8]                                    @ manually save lr(pc - 4) to stack (offset pc)
    mrs r5, spsr                                    @ get spsr
    str r5, [r7]                                    @ manually save spsr to stack (offset psr)
.endm

/* -------------------------------------------------------------------------------
 * Restore The Current Filed Context 
 * -----------------------------------------------------------------------------*/
.macro _exception_restore_params
    ldmia sp, {r0 - r12}^                           @ pop r0 ~ r12, and resume cpsr (cpsr = spsr)
    ldr lr, [sp, #ARCH_OFFSET_PC]                   @ reload lr, lr = lr(pc - 4)
    add sp, sp, #ARCH_FRAME_SIZE                    @ resume top of stack
.endm

/* -------------------------------------------------------------------------------
 * Global Variables Defines 
 * -----------------------------------------------------------------------------*/
.global g_asm_sched_flag
g_asm_sched_flag:
    .word 0x00000000

/* -------------------------------------------------------------------------------
 * Exception Handler
 * -----------------------------------------------------------------------------*/
_undefined_handler:
    _exception_save_params                          @ save current context

    bl exec_undefined_handler                       @ exception handlers, please jump to exception.c
    _exception_restore_params                       @ restore previous context

    /*!< return pc (when exception handler is called, lr = pc - 4) */
    movs pc, lr

_software_irq_handler:
    push { r12 }

    ldr r12, [lr, #-4]
    bic r12, r12, #0xff000000

    _exception_save_params                          @ save current context
    bl exec_software_irq_handler                    @ exception handlers, please jump to interrupt.c
    _exception_restore_params                       @ restore previous context

    pop { r12 }

    /*!< return pc (when exception handler is called, lr = pc - 4) */
    movs pc, lr

_prefetch_abort_handler:
    _exception_save_params                          @ save current context

    ldr r0, =prefecth_abort_addr                    @ it is defined in "exception.c"
    sub r1, lr, #4                                  @ get instrcution causing prefetch abort
    str r1, [r0]                                    @ save to global var

    bl exec_prefetch_abort_handler                  @ exception handlers, please jump to exception.c
    _exception_restore_params                       @ restore previous context

    /*!< The instruction causing abort (last instruction) needs to run again. So pc = lr - 4 */
    subs pc, lr, #0x04

_data_abort_handler:
    _exception_save_params                          @ save current context

    ldr r0, =data_abort_addr                        @ it is defined in "exception.c"
    sub r1, lr, #8                                  @ get instrcution causing data abort
    str r1, [r0]                                    @ save to global var

    bl exec_data_abort_handler                      @ exception handlers, please jump to exception.c
    _exception_restore_params                       @ restore previous context

    /*!< 
     * 1) PC will pointer next instruction before calling Data abort;
     * 2) The instruction causing abort (last instruction) needs to run again;
     * 3) So pc = lr - 4 - 4
     */
    subs pc, lr, #0x08

_unused_handler:
    bl exec_unused_handler                          @ exception handlers, please jump to exception.c

_irq_handler:
    /*!< PC will pointer next instruction before calling IRQ; So it must retrun "lr - 4" */
    sub lr, lr, #0x04
    _exception_save_params                          @ save current context

    bl exec_irq_handler                             @ exception handlers, please jump to interrupt.c
    _exception_restore_params                       @ restore previous context

    push { r0 }
    ldr r0, g_asm_sched_flag
    cmp r0, #0                                      @ check if preemption is occurring
    bne 1f

    pop { r0 }
    movs pc, lr

1:                                                  @ deal with preemption
    mov r0, #0
    str r0, g_asm_sched_flag
    pop { r0 }

    srsdb sp!, #ARCH_SVC_MODE                       @ save lr_irq and spsr to sp_svc
    cps #ARCH_SVC_MODE
    
    b __schedule_before                             @ jump to "context.S"

_fiq_handler:
    cpsid f
    _exception_save_params                          @ save current context

    bl exec_fiq_handler                             @ exception handlers, please jump to interrupt.c
    _exception_restore_params

    cpsie f

    /*!< PC will pointer next instruction before calling FIQ; So it must retrun "lr - 4" */
    subs pc, lr, #0x04

/* end of file */
