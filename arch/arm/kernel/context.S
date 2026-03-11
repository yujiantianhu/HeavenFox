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

/*!
 * Provide return path function: ret_with_first_schedule
 *
 * Case 1: First schedule, lock, if new thread is first schedule:
 *         specify return from ret_with_first_schedule, unlock, then jump to new thread entry;
 * Case 2: Old thread actively initiates schedule_thread, lock, if new thread is first schedule:
 *         specify return from ret_with_first_schedule, unlock, then jump to new thread entry;
 * Case 3: Old thread actively initiates schedule_thread, lock, if new thread has been scheduled before (also via schedule_thread):
 *         restore new thread context, new thread returns from its previous schedule_thread, unlock;
 * Case 4: Old thread is preempted by __schedule_thread_irq, lock, if new thread is first schedule:
 *         specify return from ret_with_first_schedule, unlock, then jump to new thread entry;
 * Case 5: Old thread is preempted by __schedule_thread_irq, lock, if new thread has been scheduled before (actively via schedule_thread):
 *         restore new thread context, new thread returns from its previous schedule_thread, unlock;
 * Case 6: Old thread is preempted by __schedule_thread_irq, lock, if new thread has been scheduled before (also preempted: __schedule_thread_irq):
 *         restore new thread context, new thread returns from its previous __schedule_thread_irq, unlock, finally rfeia returns to thread context (where it was interrupted);
 * Case 7: Old thread actively initiates schedule_thread, lock, if new thread has been scheduled before (preempted: __schedule_thread_irq):
 *         restore new thread context, new thread returns from its previous __schedule_thread_irq, unlock, finally rfeia returns to thread context (where it was interrupted);
*/

/*!< The includes */
#include <common/linkage.h>
#include <configs/mach_configs.h>
#include <kernel/asm_text.h>

    .text
    .arm

/*!< The defines */

/*!< The globals */

/*!< API functions  */
/*!
 * @brief	save and restore context
 * @param  	r0: &sgtc_context
 * @retval 	none
 * @note   	none
 */
ENTRY(__switch_to)
__switch_to:
    sub sp, #8
    stmdb sp!, { r0 - r12, lr }                     @ save r0 ~ r12, lr (r0 = &sgtc_context, contains info of old and new threads)

    mrs r12, cpsr
    add r8, sp, #ARCH_OFFSET_LR
    stmia r8, { r12, lr }                           @ save lr and cpsr (lr will be used as pc later)

    /*!< sp stores from low to high: r0, r1, ..., r12, lr, cpsr, lr (future pc) */

    add r2, r0, #CONTEXT_FIRST_OFFSET               @ address: r2 = &sgtc_context.first
    ldr r1, [r2]                                    @ read value: r1 = *(&sgtc_context.first)
    cmp r1, #0                                      @ if first is 0, it's the first schedule of this core's scheduler
    beq _context_save_first                         @ special handling for first schedule

    b _context_save

_context_save_first:
    mov r12, #1
    str r12, [r2]                                   @ write 1 to *(&sgtc_context.first), first = true, permanent mark

    b _context_restore

_context_save:
    ldr r2, [r0, #CONTEXT_PREV_SP]                  @ r2 = &sgtc_context.prev_stack_addr, stack_addr is the stack top address of current thread
    ldr r1, [r2]                                    @ r1 = stack top address of current thread

    add r8, r1, #ARCH_OFFSET_LR                     @ there is a special area above the stack top, defined in the same way as r0 ~ r15, offset to r14 here
    str sp, [r8, #-4]                               @ save current thread's sp to the offset r13 above the stack, so sp can be easily obtained next time: the system knows it's at offset r13 above the stack

    b _context_restore                              @ r0 ~ r15 have all been saved, current thread can end; below is restoration of new thread

_context_restore:
    ldr r2, [r0, #CONTEXT_NEXT_SP]                  @ r2 = &sgtc_context.next_stack_addr, stack_addr is the stack top address of new thread
    ldr r1, [r2]                                    @ r1 = stack top address of new thread

    ldr sp, [r1, #ARCH_OFFSET_SP]                   @ we know there is a reserved area above the stack top, whose offset r13 holds the sp value; load and update as current sp, this is the new thread's stack
    cmp sp, #0                                      @ is this location null? it means the new thread has not been scheduled before (has not executed _context_save), this is its first run
    beq _sched_first                                @ special handling for first-run new thread, because it does not need to restore context

    ldr r1, =__thread_init_before                   @ initialize time slice for new thread; now that new thread's context has not been restored, registers are free to use
    blx r1                                          @ it is recommended to call __thread_init_before after updating sp to new thread, so that the stack used is the new thread's

    ldmia sp!, { r0 - r12, lr }                     @ new thread has been run before, it has context to restore: restore r0 ~ r12, lr, but cpsr and pc are kept for later
    b _sched_init_before                            @ perform final preparation before starting

_sched_init_before:
    push { r12 }                                    @ r12 is used to temporarily store cpsr
    ldr r12, [sp, #4]                               @ sp now points to r12, 4 bytes above is the previously stored cpsr
    msr cpsr, r12                                   @ update cpsr; note: this cpsr is also in interrupt-disabled state, no need to worry about immediate interrupts because interrupts are disabled before entering scheduler
    pop { r12 }                                     @ sp += 4, pop r12
    add sp, #4                                      @ sp += 4, now points to lr

    b _switch_to_next                               @ everything ready

_switch_to_next:
    pop { pc }                                      @ return to schedule_thread or schedule_thread_irq

/*!< ---------------------------------------------------------------------------------------------------- */
/*!< ---------------------------------------------------------------------------------------------------- */
_sched_first:
    ldr r1, [r0, #CONTEXT_NEXT_SP]                  @ r1 = &sgtc_context.next_stack_addr, stack_addr is the stack top address of new thread
    ldr r8, [r1]                                    @ r8 = stack top address of new thread
    mov sp, r8                                      @ as the first-run thread, r8 can be directly used as sp: *((unsigned int *)sgtc_context.next_stack_addr)

    ldr r1, [r0, #CONTEXT_ENTRY_OFFSET]             @ r1 = &sgtc_context.next_entry, next_entry is the entry address of new thread
    ldr r8, [r1]                                    @ r8 = entry address of thread
    push { r8 }                                     @ equivalent to: push { lr }, this will be used as pc next

    ldr r1, [r0, #CONTEXT_ARGS_OFFSET]              @ r1 = &sgtc_context.next_args, args is the entry parameter of new thread
    ldr r8, [r1]                                    @ r8 = value of thread entry parameter, will be used as function argument
    push { r8 }                                     @ r8 holds entry parameter args, need to temporarily store

    ldr r1, =__thread_init_before                   @ initialize time slice for new thread; new thread is first schedule, registers are free to use
    blx r1                                          

    mrs r0, cpsr
    ldr r1, =ret_with_first_schedule                @ do some preparation, including spinlock unlock, cpsr preparation
    blx r1
    mov r1, r0                                      @ return value: default cpsr
    
    pop { r0 }                                      @ restore args to r0 as thread entry parameter
    msr cpsr, r1                                    @ update cpsr

    pop { pc }                                      @ pop thread entry address as pc, new thread runs

ENDPROC(__switch_to)

/*!< ---------------------------------------------------------------------------------------------------- */
/*!< ---------------------------------------------------------------------------------------------------- */
/*!
 * @brief	schedule because of preemption
 * @param  	none
 * @retval 	none
 * @note   	none
 */
ENTRY(__schedule_from_irq)
__schedule_from_irq:
    stmdb sp!, { r0 - r12, lr }                     @ push first, here lr is lr of SVC mode, while lr of IRQ mode is already saved on stack

    /*!< current stack state (from sp, low to high): r0, r1, ..., r12, lr_svc, spsr, lr_irq (future pc) */

    ldr r1, =schedule_thread_irq                    @ jump to C function, old thread locks, switch to new thread, then new thread unlocks
    blx r1                                          @ lr_svc will be updated

    ldmia sp!, { r0 - r12, lr }                     @ restore previous stack
    rfeia sp!                                       @ return from here: move spsr to cpsr, lr_irq to pc

ENDPROC(__schedule_from_irq)

/* end of file */