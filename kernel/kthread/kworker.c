/*
 * Kernel Thread Instance (kernel task) Interface
 *
 * File Name:   kworker.c
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
#include <kernel/workqueue.h>

/*!< The defines */
#define KWORKER_THREAD_STACK_SIZE                       THREAD_STACK_PAGE(1)    /*!< 1 page (4 kbytes) */

struct kworker_percpu
{
    struct kthread_percpu sgtc_kth;
    struct workqueue_head sgtc_wqh;
};

/*!< The globals */
static DECLARE_LIST_HEAD(sgtc_kworker_percpus);

/*!< API functions */
/*!
 * @brief	add sptr_wq to sgtc_kworker_wqh
 * @param  	sptr_wq: new work
 * @retval 	none
 * @note   	none
 */
void schedule_work(struct workqueue *sptr_wq)
{
    kuint32_t cpuid;
    struct kthread_percpu *sptr_kth; 
    
    mr_preempt_disable();
    cpuid = get_cpu_id();

    foreach_list_next_entry(sptr_kth, &sgtc_kworker_percpus, sgtc_link) 
    { 
        if (cpuid == sptr_kth->cpuid) 
        {
            struct kworker_percpu *sptr_kworker;

            sptr_kworker = mr_container_of(sptr_kth, struct kworker_percpu, sgtc_kth);
            queue_work(&sptr_kworker->sgtc_wqh, sptr_wq);
            schedule_thread_wakeup(sptr_kth->tid); 

            break; 
        } 
    }

    mr_preempt_enable();
}

/*!
 * @brief	kernel worker thread entry
 * @param  	args: NULL normally
 * @retval 	none
 * @note   	none
 */
static void *kworker_entry(void *args)
{
    DECLARE_WORKQUEUE(sgtc_copy);
    struct kworker_percpu *sptr_kworker = (struct kworker_percpu *)args;
    struct workqueue *sptr_wq;
    struct workqueue *sptr_temp;

    print_info("%s (cpuid: %u) is enter, which tid is: %d\r\n", __FUNCTION__, get_cpu_id(), mr_current->tid);

    for (;;)
    {
        if (is_workqueue_empty(&sptr_kworker->sgtc_wqh))
            goto END;

        work_splice_and_init(&sptr_kworker->sgtc_wqh, &sgtc_copy);

        foreach_workqueue_safe(sptr_wq, sptr_temp, &sgtc_copy)
        {
            if (sptr_wq->func)
                sptr_wq->func(sptr_wq);

            detach_work_safe(&sgtc_copy, sptr_wq);
        }

        continue;

END:
        msleep(208);
    }

    return args;
}

/*!
 * @brief	create kernel thread
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t kworker_init(void)
{
    struct thread_attr sgtc_attr = {};
    kuint32_t cpuid = get_cpu_id();
    struct kthread_percpu *sptr_kth;
    struct kworker_percpu *sptr_kworker;

    sptr_kworker = kmalloc(sizeof(*sptr_kworker), GFP_KERNEL);
    if (!isValid(sptr_kworker))
        return -ER_NOMEM;

    INIT_WORKQUEUE_HEAD(&sptr_kworker->sgtc_wqh);
    sptr_kth = &sptr_kworker->sgtc_kth;
    sptr_kth->cpuid = cpuid;
    init_list_head(&sptr_kth->sgtc_link);

	sgtc_attr.detachstate = THREAD_CREATE_JOINABLE;
	sgtc_attr.inheritsched	= THREAD_INHERIT_SCHED;
	sgtc_attr.schedpolicy = THREAD_SCHED_FIFO;

    /*!< thread stack */
	thread_attr_setstacksize(&sgtc_attr, KWORKER_THREAD_STACK_SIZE);
    /*!< lowest priority */
	thread_set_priority(&sgtc_attr, THREAD_PROTY_KWORKER);
    /*!< default time slice */
    thread_set_time_slice(&sgtc_attr, THREAD_TIME_DEFAULT);

    /*!< bind cpu affinity */
    thread_set_cpuaffinity(&sgtc_attr, CPU_AFFINITY_SINGEL(cpuid));

    /*!< register thread */
    sptr_kth->tid = kernel_thread_create(-1, &sgtc_attr, kworker_entry, sptr_kworker);
    if (sptr_kth->tid >= 0)
    {
        thread_set_name_args(sptr_kth->tid, "kworker/%u", cpuid);
        list_head_add_tail(&sgtc_kworker_percpus, &sptr_kth->sgtc_link);

        return ER_NORMAL;
    }

    kfree(sptr_kworker);
    return -ER_FAILD;
}

/*!< end of file */
