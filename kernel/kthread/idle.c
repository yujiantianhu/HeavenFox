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
#define IDLE_THREAD_STACK_SIZE                      THREAD_STACK_PAGE(1)    /*!< 1 page (4kbytes) */

/*!< The globals */

/*!< API functions */
/*!
 * @brief	idle thread entry
 * @param  	args: NULL normally
 * @retval 	none
 * @note   	only preempting and time slice polling can jump it out
 */
static void *rest_entry(void *args)
{
    kuint32_t cpuid = get_cpu_id();

    thread_set_self_name_args("idle/%u", cpuid);

    for (;;)
    {   
#if (!CONFIG_SCHED_SLICE)
        /*!< check priority */
        struct thread *sptr_ready = get_first_ready_thread(cpuid);
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
    struct thread_attr sgtc_attr = {};

	sgtc_attr.detachstate = THREAD_CREATE_JOINABLE;
	sgtc_attr.inheritsched	= THREAD_INHERIT_SCHED;
	sgtc_attr.schedpolicy = THREAD_SCHED_FIFO;

    /*!< thread stack */
	thread_attr_setstacksize(&sgtc_attr, IDLE_THREAD_STACK_SIZE);
    /*!< lowest priority */
	thread_set_priority(&sgtc_attr, THREAD_PROTY_IDLE);
    /*!< default time slice */
    thread_set_time_slice(&sgtc_attr, THREAD_TIME_DEFAULT);

    /*!< bind cpu affinity */
    thread_set_cpuaffinity(&sgtc_attr, CPU_AFFINITY_SINGEL(get_cpu_id()));

    /*!< register idle thread */
    return kernel_thread_idle_create(&sgtc_attr, rest_entry, mr_nullptr);
}

/*!< end of file */
