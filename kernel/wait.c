/*
 * Wait Queue Interface
 *
 * File Name:   wait.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.29
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/wait.h>

/*!< The defines */


/*!< The functions */


/*!< API functions */
void init_waitqueue_head(struct wait_queue_head *sprt_wqh)
{
    spin_lock_init(&sprt_wqh->sgrt_lock);
    init_list_head(&sprt_wqh->sgrt_task);
}

void add_wait_queue(struct wait_queue_head *sprt_wqh, struct wait_queue *sprt_wq)
{
    if (!mrt_list_head_empty(&sprt_wq->sgrt_link))
        return;

    spin_lock_irqsave(&sprt_wqh->sgrt_lock);
    list_head_add_tail(&sprt_wqh->sgrt_task, &sprt_wq->sgrt_link);
    spin_unlock_irqrestore(&sprt_wqh->sgrt_lock);
}

void remove_wait_queue(struct wait_queue_head *sprt_wqh, struct wait_queue *sprt_wq)
{
    spin_lock_irqsave(&sprt_wqh->sgrt_lock);
    list_head_del(&sprt_wq->sgrt_link);
    spin_unlock_irqrestore(&sprt_wqh->sgrt_lock);
}

static void __wake_up_common(struct real_thread *sprt_thread, kuint32_t state)
{
    if (!mrt_thread_is_flags(state, sprt_thread))
        return;
    
    real_thread_state_signal(sprt_thread, NR_THREAD_SIG_WAKEUP, true);
}

void wake_up_common(struct wait_queue_head *sprt_wqh, kuint32_t state)
{
    struct wait_queue *sprt_wq, *sprt_temp;

    if (mrt_list_head_empty(&sprt_wqh->sgrt_task))
        return;

    spin_lock_irqsave(&sprt_wqh->sgrt_lock);

    foreach_list_next_entry_safe(sprt_wq, sprt_temp, &sprt_wqh->sgrt_task, sgrt_link)
    {
        if (sprt_wq->sprt_task)
            __wake_up_common(sprt_wq->sprt_task, state);
    }

    spin_unlock_irqrestore(&sprt_wqh->sgrt_lock);
}

/*!< end of file */
