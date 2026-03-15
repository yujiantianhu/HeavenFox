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
#include <kernel/asm_text.h>

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
@ .global g_asm_sched_flag
@ g_asm_sched_flag:
@     .word 0x00000000

/* -------------------------------------------------------------------------------
 * Exception Handler
 * -----------------------------------------------------------------------------*/
_undefined_handler:
    _exception_save_params                          @ save current context

    mov r0, sp                                      @ save context base stack
    sub r1, lr, #4                                  @ get instrcution causing undefined abort
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

    mov r0, sp                                      @ save context base stack
    sub r1, lr, #4                                  @ get instrcution causing prefetch abort
    bl exec_prefetch_abort_handler                  @ exception handlers, please jump to exception.c

    _exception_restore_params                       @ restore previous context

    /*!< The instruction causing abort (last instruction) needs to run again. So pc = lr - 4 */
    subs pc, lr, #0x04

_data_abort_handler:
    _exception_save_params                          @ save current context

    mov r0, sp                                      @ save context base stack
    sub r1, lr, #8                                  @ get instrcution causing data abort
    bl exec_data_abort_handler                      @ exception handlers, please jump to exception.c

    _exception_restore_params                       @ restore previous context

    /*!< 
     * 1) PC will pointer next instruction before calling Data abort;
     * 2) The instruction causing abort (last instruction) needs to run again;
     * 3) So pc = lr - 4 - 4
     */
    subs pc, lr, #0x08

_unused_handler:
    _exception_save_params                          @ save current context

    mov r0, sp                                      @ save context base stack
    mov r1, #0                                      @ not used
    bl exec_unused_handler                          @ exception handlers, please jump to exception.c

_irq_handler:
    /*!< PC will pointer next instruction before calling IRQ; So it must retrun "lr - 4" */
    sub lr, lr, #0x04
    _exception_save_params                          @ save current context

    bl exec_irq_handler                             @ exception handlers, please jump to interrupt.c
    _exception_restore_params                       @ restore previous context

    push { r0, r1 }

    READ_CP15_TPIDRPRW(r0)                          @ get tpidrprw
    cmp r0, #0                                      @ no data (no thread is running)
    beq 1f

    and r1, r0, #THREAD_MASK
    cmp r1, #THREAD_SCHED_BIT                       @ 0b01: invalid; 0b10: schedule enable (preemption is occurring)
    beq 2f

1:
    pop { r0, r1 }
    movs pc, lr

2:                                                  @ deal with preemption
    bic r1, r0, #THREAD_MASK
    WRITE_CP15_TPIDRPRW(r1)                         @ set tpidrprw

    pop { r0, r1 }

    srsdb sp!, #ARCH_SVC_MODE                       @ save lr_irq and spsr to sp_svc
    cpsid i, #ARCH_SVC_MODE                         @ atomic: switch svc mode and disable irq
    
    b __schedule_from_irq                           @ jump to "context.S"

_fiq_handler:
    _exception_save_params                          @ save current context

    bl exec_fiq_handler                             @ exception handlers, please jump to interrupt.c
    _exception_restore_params

    /*!< PC will pointer next instruction before calling FIQ; So it must retrun "lr - 4" */
    subs pc, lr, #0x04

/* end of file */
