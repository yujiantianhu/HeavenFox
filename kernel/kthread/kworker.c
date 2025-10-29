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

/*!< The globals */
static tid_t g_kworker_tid = -1;
static struct thread_attr sgtc_kworker_attr;
static THREAD_STACK_DEFINE(g_kworker_stack, KWORKER_THREAD_STACK_SIZE);

static DECLARE_WORKQUEUE(sgtc_kworker_wqh);

/*!< API functions */
/*!
 * @brief	add sptr_wq to sgtc_kworker_wqh
 * @param  	sptr_wq: new work
 * @retval 	none
 * @note   	none
 */
void schedule_work(struct workqueue *sptr_wq)
{
    queue_work(&sgtc_kworker_wqh, sptr_wq);

    if (g_kworker_tid != (-1))
        schedule_thread_wakeup(g_kworker_tid);
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
    struct workqueue *sptr_wq;
    struct workqueue *sptr_temp;

    print_info("%s is enter, which tid is: %d\r\n", __FUNCTION__, mr_current->tid);

    for (;;)
    {
        if (is_workqueue_empty(&sgtc_kworker_wqh))
            goto END;

        work_splice_and_init(&sgtc_kworker_wqh, &sgtc_copy);

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
    struct thread_attr *sptr_attr = &sgtc_kworker_attr;

	sptr_attr->detachstate = THREAD_CREATE_JOINABLE;
	sptr_attr->inheritsched	= THREAD_INHERIT_SCHED;
	sptr_attr->schedpolicy = THREAD_SCHED_FIFO;

    /*!< thread stack */
	thread_set_stack(sptr_attr, mr_nullptr, g_kworker_stack, sizeof(g_kworker_stack));
    /*!< lowest priority */
	thread_set_priority(sptr_attr, THREAD_PROTY_KWORKER);
    /*!< default time slice */
    thread_set_time_slice(sptr_attr, THREAD_TIME_DEFUALT);

    /*!< register thread */
    g_kworker_tid = kernel_thread_create(-1, sptr_attr, kworker_entry, mr_nullptr);
    if (g_kworker_tid >= 0)
    {
        thread_set_name(g_kworker_tid, "kworker");
        return ER_NORMAL;
    }

    return -ER_FAILD;
}

/*!< end of file */
