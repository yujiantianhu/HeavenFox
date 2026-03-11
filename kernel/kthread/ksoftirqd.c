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
static DECLARE_LIST_HEAD(sgtc_ksoftirqd_percpus);

/*!< API functions */
/*!
 * @brief	wake up ksoftirqd thread
 * @param  	none
 * @retval 	none
 * @note   	none
 */
void wake_up_ksoftirqd(void)
{
    kuint32_t cpuid;
    
    mr_preempt_disable();
    cpuid = get_cpu_id();
    schedule_cpu_thread_wakeup(cpuid, &sgtc_ksoftirqd_percpus);
    mr_preempt_enable();
}

/*!
 * @brief	softirq thread entry
 * @param  	args: NULL normally
 * @retval 	none
 * @note   	none
 */
static void *ksoftirqd_entry(void *args)
{
    print_info("%s (cpuid: %u) is enter, which tid is: %d\r\n", __FUNCTION__, get_cpu_id(), mr_current->tid);

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
    struct thread_attr sgtc_attr = {};
    kuint32_t cpuid = get_cpu_id();
    struct kthread_percpu *sptr_kth;

    sptr_kth = kmalloc(sizeof(*sptr_kth), GFP_KERNEL);
    if (!isValid(sptr_kth))
        return -ER_NOMEM;

    sptr_kth->cpuid = cpuid;
    init_list_head(&sptr_kth->sgtc_link);

	sgtc_attr.detachstate = THREAD_CREATE_JOINABLE;
	sgtc_attr.inheritsched	= THREAD_INHERIT_SCHED;
	sgtc_attr.schedpolicy = THREAD_SCHED_FIFO;

    /*!< thread stack */
    thread_attr_setstacksize(&sgtc_attr, KSOFTIRQD_THREAD_STACK_SIZE);
    /*!< lowest priority */
	thread_set_priority(&sgtc_attr, THREAD_PROTY_KSOFTIRQD);
    /*!< default time slice */
    thread_set_time_slice(&sgtc_attr, THREAD_TIME_DEFAULT);
    /*!< bind cpu affinity */
    thread_set_cpuaffinity(&sgtc_attr, CPU_AFFINITY_SINGEL(cpuid));

    /*!< register thread */
    sptr_kth->tid = kernel_thread_create(-1, &sgtc_attr, ksoftirqd_entry, mr_nullptr);
    if (sptr_kth->tid  >= 0)
    {
        thread_set_name_args(sptr_kth->tid , "ksoftirqd/%u", cpuid);
        list_head_add_tail(&sgtc_ksoftirqd_percpus, &sptr_kth->sgtc_link);

        return ER_NORMAL;
    }

    kfree(sptr_kth);
    return -ER_FAILD;
}

/*!< end of file */
