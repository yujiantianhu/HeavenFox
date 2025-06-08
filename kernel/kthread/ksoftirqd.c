/*
 * Kernel Thread Instance (ksoftirqd task) Interface
 *
 * File Name:   ksoftirqd.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.06.08
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <platform/irq/fwk_irq_types.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/instance.h>
#include <kernel/workqueue.h>

/*!< The defines */
#define KSOFTIRQD_THREAD_STACK_SIZE                     THREAD_STACK_PAGE(1)    /*!< 1 page (4 kbytes) */

/*!< The globals */
static tid_t g_ksoftirqd_tid;
static struct thread_attr sgtc_ksoftirqd_attr;
static kuint8_t g_ksoftirqd_stack[KSOFTIRQD_THREAD_STACK_SIZE];

/*!< API functions */
/*!
 * @brief	wake up ksoftirqd thread
 * @param  	none
 * @retval 	none
 * @note   	none
 */
void wake_up_ksoftirqd(void)
{
    schedule_thread_wakeup(g_ksoftirqd_tid);
}

/*!
 * @brief	softirq thread entry
 * @param  	args: NULL normally
 * @retval 	none
 * @note   	none
 */
static void *ksoftirqd_entry(void *args)
{
    print_info("%s is enter, which tid is: %d\r\n", __FUNCTION__, mr_current->tid);

    for (;;)
    {
        schedule_self_suspend();
        fwk_handle_softirq();
    }

    return args;
}

/*!
 * @brief	create ksoftirqd thread
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t ksoftirqd_init(void)
{
    struct thread_attr *sptr_attr = &sgtc_ksoftirqd_attr;

	sptr_attr->detachstate = THREAD_CREATE_JOINABLE;
	sptr_attr->inheritsched	= THREAD_INHERIT_SCHED;
	sptr_attr->schedpolicy = THREAD_SCHED_FIFO;

    /*!< thread stack */
	thread_set_stack(sptr_attr, mr_nullptr, g_ksoftirqd_stack, sizeof(g_ksoftirqd_stack));
    /*!< lowest priority */
	thread_set_priority(sptr_attr, THREAD_PROTY_KSOFTIRQD);
    /*!< default time slice */
    thread_set_time_slice(sptr_attr, THREAD_TIME_DEFUALT);

    /*!< register thread */
    g_ksoftirqd_tid = kernel_thread_create(-1, sptr_attr, ksoftirqd_entry, mr_nullptr);
    if (g_ksoftirqd_tid >= 0)
    {
        thread_set_name(g_ksoftirqd_tid, "ksoftirqd_entry");
        return ER_NORMAL;
    }

    return -ER_FAILD;
}

/*!< end of file */
