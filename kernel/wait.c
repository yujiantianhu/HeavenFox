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
/*!
 * @brief   initial wait queue head
 * @param   sptr_wqh
 * @retval  none
 * @note    none
 */
void init_waitqueue_head(struct wait_queue_head *sptr_wqh)
{
    spin_lock_init(&sptr_wqh->sgtc_lock);
    init_list_head(&sptr_wqh->sgtc_task);
}

/*!
 * @brief   add a new wait_queue to wait_queue_head
 * @param   sptr_wqh, sptr_wq
 * @retval  none
 * @note    none
 */
void add_wait_queue(struct wait_queue_head *sptr_wqh, struct wait_queue *sptr_wq)
{
    if (!mr_list_empty(&sptr_wq->sgtc_link))
        return;

    spin_lock_irqsave(&sptr_wqh->sgtc_lock);
    list_head_add_tail(&sptr_wqh->sgtc_task, &sptr_wq->sgtc_link);
    spin_unlock_irqrestore(&sptr_wqh->sgtc_lock);
}

/*!
 * @brief   remove a wait_queue from wait_queue_head
 * @param   sptr_wqh, sptr_wq
 * @retval  none
 * @note    none
 */
void remove_wait_queue(struct wait_queue_head *sptr_wqh, struct wait_queue *sptr_wq)
{
    spin_lock_irqsave(&sptr_wqh->sgtc_lock);
    list_head_del(&sptr_wq->sgtc_link);
    spin_unlock_irqrestore(&sptr_wqh->sgtc_lock);
}

/*!
 * @brief   wake up thread
 * @param   sptr_thread, state
 * @retval  none
 * @note    none
 */
static void __wake_up_common(struct thread *sptr_thread, kuint32_t state)
{
    if (!mr_thread_is_flags(state, sptr_thread))
        return;
    
    thread_state_signal(sptr_thread, NR_THREAD_SIG_WAKEUP, true);
}

/*!
 * @brief   wake up thread
 * @param   sptr_wqh, state
 * @retval  none
 * @note    none
 */
void wake_up_common(struct wait_queue_head *sptr_wqh, kuint32_t state)
{
    struct wait_queue *sptr_wq, *sptr_temp;

    if (mr_list_empty(&sptr_wqh->sgtc_task))
        return;

    spin_lock_irqsave(&sptr_wqh->sgtc_lock);

    foreach_list_next_entry_safe(sptr_wq, sptr_temp, &sptr_wqh->sgtc_task, sgtc_link)
    {
        if (sptr_wq->sptr_task)
            __wake_up_common(sptr_wq->sptr_task, state);
    }

    spin_unlock_irqrestore(&sptr_wqh->sgtc_lock);
}

/*!< end of file */
