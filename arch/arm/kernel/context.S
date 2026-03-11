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

@ 提供返回路径函数: ret_with_first_schedule
@ 情况1: 首次调度, 加锁, 新线程为首次调度, 指定从ret_with_first_schedule返回, 解锁, 然后跳转到新线程入口;
@ 情况2: 旧线程主动发起调度schedule_thread, 加锁, 新线程为首次调度, 指定从ret_with_first_schedule返回, 解锁, 然后跳转到新线程入口;
@ 情况3: 旧线程主动发起调度schedule_thread, 加锁, 新线程此前调度过(也是通过schedule_thread), 恢复新线程上下文, 新线程从它之前运行过的schedule_thread返回, 解锁;
@ 情况4: 旧线程被抢占__schedule_thread_irq, 加锁, 新线程为首次调度, 指定从ret_with_first_schedule返回, 解锁, 然后跳转到新线程入口;
@ 情况5: 旧线程被抢占__schedule_thread_irq, 加锁, 新线程此前调度过(通过schedule_thread主动发起), 恢复新线程上下文, 新线程从它之前运行过的schedule_thread返回, 解锁;
@ 情况6: 旧线程被抢占__schedule_thread_irq, 加锁, 新线程此前调度过(也是被抢占的: __schedule_thread_irq), 恢复新线程上下文, 新线程从它之前运行过的__schedule_thread_irq返回, 解锁, 最后由rfeia回到线程上下文(之前被打断的地方);
@ 情况7: 旧线程主动发起调度schedule_thread, 加锁, 新线程此前调度过(是被抢占的: __schedule_thread_irq), 恢复新线程上下文, 新线程从它之前运行过的__schedule_thread_irq返回, 解锁, 最后由rfeia回到线程上下文(之前被打断的地方);

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
    stmdb sp!, { r0 - r12, lr }                     @ 保存r0 ~ r12、lr (r0 = &sgtc_context, 含旧线程、新线程的信息)

    mrs r12, cpsr
    add r8, sp, #ARCH_OFFSET_LR
    stmia r8, { r12, lr }                           @ 保存lr和cpsr (lr未来会作为pc弹出)

    /* sp从低到高分别存储: r0, r1, ..., r12, lr, cpsr, lr(未来的pc) */

    add r2, r0, #CONTEXT_FIRST_OFFSET               @ 地址: r2 = &sgtc_context.first
    ldr r1, [r2]                                    @ 读值: r1 = *(&sgtc_context.first)
    cmp r1, #0                                      @ 如果first为0, 那就是本核心的整个调度器首次调度
    beq _context_save_first                         @ 首次调度特别处理

    b _context_save

_context_save_first:
    mov r12, #1
    str r12, [r2]                                   @ 将1写给*(&sgtc_context.first), first = true, 永久标记

    b _context_restore

_context_save:
    ldr r2, [r0, #CONTEXT_PREV_SP]                  @ r2 = &sgtc_context.prev_stack_addr, stack_addr的值是当前线程的栈顶地址
    ldr r1, [r2]                                    @ r1 = 当前线程的栈顶地址

    add r8, r1, #ARCH_OFFSET_LR                     @ 栈顶的上方有一块特别区域, 同样按r0 ~ r15的方式定义, 这里偏移到r14的位置
    str sp, [r8, #-4]                               @ 将当前线程的sp保存到栈上方偏移r13的位置, 这样sp就可在下次轻易获取: 系统知道它就在栈上方偏移r13的位置

    b _context_restore                              @ r0 ~ r15已经全都保存完, 当前线程可以终结了; 下面是新线程的恢复

_context_restore:
    ldr r2, [r0, #CONTEXT_NEXT_SP]                  @ r2 = &sgtc_context.next_stack_addr, stack_addr的值是新线程的栈顶地址
    ldr r1, [r2]                                    @ r1 = 新线程的栈顶地址

    ldr sp, [r1, #ARCH_OFFSET_SP]                   @ 我们知道, 栈顶上方有一块预留区域, 其偏移r13的地址就保存着sp的值; 取出更新为当前sp, 此即新线程的栈
    cmp sp, #0                                      @ 这个位置为空值 ? 说明新线程此前还没有被调度 (还没有执行过_context_save), 这是首次要运行
    beq _sched_first                                @ 首次运行的新线程特殊处理, 因为, 它无需恢复上下文   

    ldr r1, =__thread_init_before                   @ 对新线程的时间片进行初始化, 趁现在新线程的上下文还没复原, 寄存器正是随意使用的时期
    blx r1                                          @ 建议sp更新为新线程后再调用__thread_init_before, 这样用的栈就是新线程的

    ldmia sp!, { r0 - r12, lr }                     @ 新线程是之前运行过的, 它有上下文要恢复: 复原r0 ~ r12、lr, 但是cpsr和pc先留着, 还没到结束的时候
    b _sched_init_before                            @ 执行启动前的最后一次准备工作

_sched_init_before:
    push { r12 }                                    @ r12准备用来暂存cpsr
    ldr r12, [sp, #4]                               @ sp现在指向r12, 往上偏4字节, 就是上次存的cpsr
    msr cpsr, r12                                   @ 更新到cpsr, 注意: 这个cpsr也是处于关中断状态的, 不用担心会立即发生中断, 因为进调度器前都会关中断
    pop { r12 }                                     @ sp += 4, 弹出r12
    add sp, #4                                      @ sp += 4, 现在指向lr的位置

    b _switch_to_next                               @ 一切就绪

_switch_to_next:
    pop { pc }                                      @ 返回到schedule_thread或schedule_thread_irq

/*!< ---------------------------------------------------------------------------------------------------- */
/*!< ---------------------------------------------------------------------------------------------------- */
_sched_first:
    ldr r1, [r0, #CONTEXT_NEXT_SP]                  @ r1 = &sgtc_context.next_stack_addr, stack_addr的值是新线程的栈顶地址
    ldr r8, [r1]                                    @ r8 = 新线程的栈顶地址
    mov sp, r8                                      @ 作为首次运行的线程, r8就可以直接当成sp: *((unsigned int *)sgtc_context.next_stack_addr)

    ldr r1, [r0, #CONTEXT_ENTRY_OFFSET]             @ r1 = &sgtc_context.next_entry, next_entry的值是新线程的入口地址
    ldr r8, [r1]                                    @ r8 = 线程入口的地址
    push { r8 }                                     @ 等价于: push { lr }, 这是下次作为pc的值

    ldr r1, [r0, #CONTEXT_ARGS_OFFSET]              @ r1 = &sgtc_context.next_args, args的值是新线程的入口参数
    ldr r8, [r1]                                    @ r8 = 线程入口的参数值, 将作为函数的参数
    push { r8 }                                     @ r8保存着入口参数args, 需要先暂存

    ldr r1, =__thread_init_before                   @ 对新线程的时间片进行初始化, 新线程是首次调度, 寄存器随意使用
    blx r1                                          

    mrs r0, cpsr
    ldr r1, =ret_with_first_schedule                @ 做一些准备工作, 包括自旋锁解锁、cpsr预备
    blx r1
    mov r1, r0                                      @ 返回值: cpsr默认值
    
    pop { r0 }                                      @ 还原args给r0, 作为线程入口参数
    msr cpsr, r1                                    @ 更新cpsr

    pop { pc }                                      @ 弹出线程入口的地址作为pc, 新线程运行

ENDPROC(__switch_to)

/*!< ---------------------------------------------------------------------------------------------------- */
/*!< ---------------------------------------------------------------------------------------------------- */
/*!
 * @brief	schedule because of preemptetion
 * @param  	none
 * @retval 	none
 * @note   	none
 */
ENTRY(__schedule_from_irq)
__schedule_from_irq:
    stmdb sp!, { r0 - r12, lr }                     @ 先压栈，这里的lr是svc模式的lr，而irq模式的lr已经保存在栈里

    /*  当前栈区状态(从sp开始, 低到高): r0, r1, ..., r12, lr_svc, spsr, lr_irq(未来的pc) */

    ldr r1, =schedule_thread_irq                    @ 跳到C语言, 旧线程加锁, 切换新线程, 然后由新线程解锁
    blx r1                                          @ lr_svc会被更新

    ldmia sp!, { r0 - r12, lr }                     @ 还原之前的栈
    rfeia sp!                                       @ 从这里返回: 将spsr给cpsr, lr_irq赋值给pc

ENDPROC(__schedule_from_irq)

/* end of file */
