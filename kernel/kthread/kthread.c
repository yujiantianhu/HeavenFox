/*
 * Kernel Thread Instance (kernel task) Interface
 *
 * File Name:   kthread.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.01
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/instance.h>

/*!< The defines */
#define KERL_THREAD_PREEMPT_PERIOD                      (20)                        /*!< 20ms */
#define KERL_THREAD_STACK_SIZE                          KEL_THREAD_STACK_HALF(1)    /*!< 1/2 page (2 kbytes) */

/*!< The globals */
TARGET_EXT kuint32_t g_asm_sched_flag;

static srt_kel_thread_attr_t sgrt_kthread_attr;
static kuint32_t g_kthread_stack[KERL_THREAD_STACK_SIZE];
static struct timer_list sgrt_kthread_timer;

/*!< API functions */
/*!
 * @brief	kernel thread scheduler
 * @param  	args: timer
 * @retval 	none
 * @note   	check priority and time slice, set g_asm_sched_flag to true or false
 */
static void kthread_schedule_timeout(kuint32_t args)
{
    struct timer_list *sprt_tim = (struct timer_list *)args;
    struct kel_thread *sprt_work, *sprt_ready;
    kuint32_t work_prio, next_prio;
    
    sprt_work = mrt_current;
    
    /*!< --------------------------------------------------------- */
    /*!< check time slice (when the time slice is not exhuasted, current cannot be preempted) */
    if (mrt_time_before(jiffies, sprt_work->expires))
        goto END;
    
    /*!< --------------------------------------------------------- */
    /*!< check priority */
    sprt_ready = get_first_ready_thread();
    if (!sprt_ready)
        goto END;

    work_prio = real_thread_get_priority(sprt_work->sprt_attr);
    next_prio = real_thread_get_priority(sprt_ready->sprt_attr);
   
    /*!< there is a higher priority thread ready */
    if (work_prio <= next_prio)
    {
        g_asm_sched_flag = true;
        goto END;
    }
    
END:
    mod_timer(sprt_tim, jiffies + msecs_to_jiffies(KERL_THREAD_PREEMPT_PERIOD));
}

/*!
 * @brief	kernel thread entry
 * @param  	args: NULL normally
 * @retval 	none
 * @note   	create all kernel thread
 */
static void *kthread_entry(void *args)
{
    struct timer_list *sprt_tim = &sgrt_kthread_timer;
    
    setup_timer(sprt_tim, kthread_schedule_timeout, (kuint32_t)sprt_tim);

    /*!< create init task */
    init_proc_init();

    for (;;)
    {
        struct kel_thread *sprt_tid = mrt_current;
        print_info("%s: tid = %d\n", __FUNCTION__, sprt_tid->tid);
        
        /*!< start timer */
        if (!sprt_tim->expires)
            mod_timer(sprt_tim, jiffies + msecs_to_jiffies(KERL_THREAD_PREEMPT_PERIOD));

        schedule_delay_ms(200);
    }

    return args;
}

/*!
 * @brief	create kernel thread
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
ksint32_t kthread_init(void)
{
    srt_kel_thread_attr_t *sprt_attr = &sgrt_kthread_attr;

	sprt_attr->detachstate = KEL_THREAD_CREATE_JOINABLE;
	sprt_attr->inheritsched	= KEL_THREAD_INHERIT_SCHED;
	sprt_attr->schedpolicy = KEL_THREAD_SCHED_FIFO;

    /*!< thread stack */
	real_thread_set_stack(sprt_attr, mrt_nullptr, g_kthread_stack, sizeof(g_kthread_stack));
    /*!< lowest priority */
	real_thread_set_priority(sprt_attr, KEL_THREAD_PROTY_KERNEL);
    /*!< default time slice */
    real_thread_set_time_slice(sprt_attr, KEL_THREAD_TIME_DEFUALT);

    /*!< register thread */
    return kernel_thread_base_create(sprt_attr, kthread_entry, mrt_nullptr);
}

/*!< end of file */
