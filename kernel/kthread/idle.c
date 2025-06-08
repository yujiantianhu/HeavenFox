/*
 * Kernel Thread Instance (idle task) Interface
 *
 * File Name:   idle.c
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
#define IDLE_THREAD_STACK_SIZE                      THREAD_STACK_QUAR(1)    /*!< 1/4 page (1kbytes) */

/*!< The globals */
static struct thread_attr sgtc_idle_attr;
static kuint8_t g_idle_stack[IDLE_THREAD_STACK_SIZE];

/*!< API functions */
/*!
 * @brief	idle thread entry
 * @param  	args: NULL normally
 * @retval 	none
 * @note   	only preempting and time slice polling can jump it out
 */
static void *rest_entry(void *args)
{
    thread_set_self_name(__FUNCTION__);

    for (;;)
    {   
#if (!CONFIG_PREEMPT)
        /*!< check priority */
        struct thread *sptr_ready = get_first_ready_thread();
        if (!sptr_ready || (sptr_ready == mr_current))
            continue;
#endif

        schedule_thread();
    }

    return args;
}

/*!
 * @brief	create idle thread
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t rest_init(void)
{
    struct thread_attr *sptr_attr = &sgtc_idle_attr;

	sptr_attr->detachstate = THREAD_CREATE_JOINABLE;
	sptr_attr->inheritsched	= THREAD_INHERIT_SCHED;
	sptr_attr->schedpolicy = THREAD_SCHED_FIFO;

    /*!< thread stack */
	thread_set_stack(sptr_attr, mr_nullptr, g_idle_stack, sizeof(g_idle_stack));
    /*!< lowest priority */
	thread_set_priority(sptr_attr, THREAD_PROTY_IDLE);
    /*!< default time slice */
    thread_set_time_slice(sptr_attr, THREAD_TIME_DEFUALT);

    /*!< register idle thread */
    return kernel_thread_idle_create(sptr_attr, rest_entry, mr_nullptr);
}

/*!< end of file */
