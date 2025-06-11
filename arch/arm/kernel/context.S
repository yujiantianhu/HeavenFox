/*
 * Assembly File About Thread Scheduler
 *
 * File Name:   context.S
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.03.31
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#include <common/linkage.h>
#include <configs/mach_configs.h>
#include <kernel/asm_text.h>

    .text
    .arm

/* -------------------------------------------------------------------------------
 * Global Variables Defines 
 * -----------------------------------------------------------------------------*/
 .global g_asm_context_info
g_asm_context_info: .word 0x00000000

/* -------------------------------------------------------------------------------
 * -------------------------------------------------------------------------------
 *                      Entry1: enter from "schedule_thread"
 * -------------------------------------------------------------------------------
 * -----------------------------------------------------------------------------*/
ENTRY(__switch_to)
__switch_to:
    sub sp, #8
    stmdb sp!, { r0 - r12, lr }                     @ save r0 ~ r12 and lr (r0 = g_asm_context_info = &sgtc_context)

    mrs r12, spsr
    add r8, sp, #ARCH_OFFSET_LR
    stmia r8, { r12, lr }                           @ save lr and spsr (lr will be set to pc, and spsr to cpsr)

    str r0, g_asm_context_info                      @ save &sgtc_context to global variable

    ldr r2, [r0, #CONTEXT_FIRST_OFFSET]
    ldr r1, [r2]
    cmp r1, #0                                      @ judge if the scheduler works for the first time
    beq _context_save_first

1:
    ldr r2, [r0, #CONTEXT_PREV_SP]                  @ r1: prev sp (frame register base)
    ldr r1, [r2]

    add r2, r1, #ARCH_FRAME_SIZE
    ldr r3, [r2]                                    @ r2 = &flags, r3 = flags
    bic r3, #SCHED_FROM_IRQ                         @ scheduled by actively

2:
    mov r10, lr                                     @ r10 = lr
    mov r11, r10                                    @ r11 = pc
    b _context_save

/* -------------------------------------------------------------------------------
 * scheduled for first time
 * -----------------------------------------------------------------------------*/
_context_save_first:
    mov r12, #1
    str r12, [r2]                                  @ set first to true

    push { r0 }
    ldr r1, =__thread_init_before                  @ init thread before running
    blx r1
    pop { r0 }

    b _sched_first_ready

/* -------------------------------------------------------------------------------
 * save curent context
 * -----------------------------------------------------------------------------*/
_context_save:
    orr r3, #SCHED_NOT_FIRST                        @ r3 = flags | 0x02
    str r3, [r2]                                    @ initial flags (marking the previous thread will no longer be scheduled for the first time)  

    add r8, r1, #ARCH_OFFSET_LR
    str sp, [r8, #-4]                               @ save sp
    stmia r8, { r10 - r12 }                         @ save lr(r10), pc(r11), spsr(r12). Just saving, not involved in scheduling                   

    push { r0 }
    ldr r1, =__thread_init_before                   @ init thread before running
    blx r1
    pop { r0 }

    b _context_restore

/* -------------------------------------------------------------------------------
 * restore next context
 * -----------------------------------------------------------------------------*/
_context_restore:
    ldr r2, [r0, #CONTEXT_NEXT_SP]                  @ r0: &sgtc_context
    ldr r1, [r2]                                    @ r1: next sp (frame register base)

    ldr sp, [r1, #ARCH_OFFSET_SP]
    cmp sp, #0
    beq _sched_first_ready                          @ first schedule    

    ldmia sp!, { r0 - r12, lr }                     @ restore r0 ~ r12 and lr, but spsr and pc reserved
    b _sched_init_before

/* -------------------------------------------------------------------------------
 * schedule preparation (first time scheduling)
 * -----------------------------------------------------------------------------*/
_sched_first_ready:
    ldr r1, [r0, #CONTEXT_NEXT_SP]                  @ r1: next sp (frame register base)
    ldr r8, [r1]
    mov sp, r8                                      @ first schedule, sp = frame register base    

    ldr r1, [r0, #CONTEXT_ENTRY_OFFSET]             @ address of entry
    ldr r8, [r1]                                    @ first schedule, lr = entry
    mrs r7, spsr
    push { r7, r8 }                                 @ push { cpsr, lr } ===> pop { spsr }, pop { pc }

    ldr r1, [r0, #CONTEXT_ARGS_OFFSET]              @ args
    ldr r8, [r1]
    mov r0, r8                                      @ r0 = args

    b _sched_init_before

/* -------------------------------------------------------------------------------
 * save curent context
 * -----------------------------------------------------------------------------*/
_sched_init_before:
    push { r0 - r3, r12 }                           @ r1: next sp; r2: first; r7: address of [first]

1:
    ldr r0, g_asm_context_info                      @ get context_info from "__schedule_thread"
    ldr r1, [r0, #CONTEXT_NEXT_SP]                  @ r1: next sp (frame register base)
    ldr r2, [r1]

    add r3, r2, #ARCH_FRAME_SIZE                    @ r3 = &flags
    ldr r12, [r3]                                   @ r12 = flags
    and r12, #SCHED_FROM_IRQ
    cmp r12, #0
    beq 3f                                          @ judge if is from irq_handler

2:
    pop { r0 - r3, r12 }
    b _switch_to_irq                                @ ===> return irq_handler

3:
    ldr r12, [sp, #20]
    msr cpsr, r12                                   @ spsr will be used by current thread
    pop { r0 - r3, r12 }
    add sp, #4

    b _switch_to_next                               @ ===> return thread

/* -------------------------------------------------------------------------------
 * schedule new thread
 * -----------------------------------------------------------------------------*/
_switch_to_next:
    pop { pc }

ENDPROC(__switch_to)


/* -------------------------------------------------------------------------------
 * -------------------------------------------------------------------------------
 *                      Entry2: enter from "irq_handler"
 * -------------------------------------------------------------------------------
 * -----------------------------------------------------------------------------*/
ENTRY(__schedule_before)
__schedule_before:
    stmdb sp!, { r0 - r12, lr }                     @ save r0 ~ r12 and current thread's lr (which is not equaled to lr_irq !!!)

    ldr r1, =__schedule_thread
    blx r1                                          @ schedule threads (r0 = g_asm_context_info = &sgtc_context)

    cmp r0, #0                                      @ __schedule_thread returns NULL ?
    beq _switch_fail                                @ schedule fail, give up scheduling

    add r8, sp, #ARCH_OFFSET_PC                     @ pointer to spsr_irq
    ldr r12, [r8]                                   @ r12 = spsr_irq                    
    cmp r12, #0
    orreq r12, #ARCH_SVC_MODE
    str r12, [r8]                                   @ if spsr is none, set svc-mode

    str r0, g_asm_context_info                      @ save &sgtc_context to global variable

1:
    ldr r2, [r0, #CONTEXT_PREV_SP]                  @ r1: prev sp (frame register base)
    ldr r1, [r2]

    add r2, r1, #ARCH_FRAME_SIZE
    ldr r3, [r2]
    orr r3, #SCHED_FROM_IRQ                         @ scheduling source: by IRQ

2:
    add r8, sp, #ARCH_OFFSET_SP                     @ r8 pointer to lr_svc
    ldmia r8, { r10, r11 }                          @ r10 = lr(lr_svc), r11 = pc(lr_irq)
    b _context_save

/* -------------------------------------------------------------------------------
 * schedule thread failed (by IRQ)
 * -----------------------------------------------------------------------------*/
_switch_fail:
@   ldr r1, =__thread_init_before                   @ schedule failed, init time slice to run again
@   blx r1

    stmia sp!, { r0 - r12, lr }                     @ restore context

/* -------------------------------------------------------------------------------
 * schedule new thread (by IRQ)
 * -----------------------------------------------------------------------------*/
_switch_to_irq:
    rfeia sp!                                       @ if scheduling source is irq, return to kill interrupt

ENDPROC(__schedule_before)

/* end of file */
