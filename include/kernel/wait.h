/*
 * Kernel Thread Wait Queue Defines
 *
 * File Name:   wakequeue.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.29
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __KERNEL_WAIT_H_
#define __KERNEL_WAIT_H_

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <kernel/kernel.h>
#include <kernel/thread.h>
#include <kernel/sched.h>
#include <kernel/sleep.h>
#include <kernel/spinlock.h>

/*!< The defines */
#define FWK_WAIT_NO_CONDION                                     (0)

typedef struct wait_queue_head
{
    struct spin_lock sgtc_lock;
    struct list_head sgtc_task;

} srt_wait_queue_head_t;

#define __WAITQUEUE_HEAD_INITIALIZER(name)  \
{   \
    .sgtc_lock = SPIN_LOCK_INIT(),  \
    .sgtc_task = LIST_HEAD_INIT(&(name)->sgtc_task) \
}

#define DECALRE_WAITQUEUE_HEAD(name)    \
    srt_wait_queue_head_t name = __WAITQUEUE_HEAD_INITIALIZER(name)

typedef struct wait_queue
{
    struct thread *sptr_task;
    struct list_head sgtc_link;

} srt_wait_queue_t;

#define __WAITQUEUE_INITIALIZER(tsk)  \
{    \
	.sptr_task	= tsk,  \
	.sgtc_link	= { mr_nullptr, mr_nullptr }    \
}

#define DECLARE_WAITQUEUE(name, tsk)    \
    srt_wait_queue_t name = __WAITQUEUE_INITIALIZER(tsk)

/*!< The functions */
extern void init_waitqueue_head(struct wait_queue_head *sptr_wqh);
extern void add_wait_queue(struct wait_queue_head *sptr_wqh, struct wait_queue *sptr_wq);
extern void remove_wait_queue(struct wait_queue_head *sptr_wqh, struct wait_queue *sptr_wq);
extern void wake_up_common(struct wait_queue_head *sptr_wqh, kuint32_t state);

/*!< The defines */
#define __wait_is_interruptible(sptr_wq)                (mr_thread_is_flags(NR_THREAD_SIG_INTR, (sptr_wq)->sptr_task))

#define __wait_event(sptr_wqh, condition, sig_enable, func) \
({  \
    DECLARE_WAITQUEUE(sgtc_wq, mr_current);  \
    const kuint32_t state = (sig_enable) ? NR_THREAD_SIG_INTR : NR_THREAD_SIG_NORMAL;  \
    \
    if (!sgtc_wq.sptr_task)   \
        while (!(condition)); \
    else {  \
        init_list_head(&sgtc_wq.sgtc_link); \
        add_wait_queue(sptr_wqh, &sgtc_wq); \
        thread_state_signal(sgtc_wq.sptr_task, state, true);   \
        \
        for (;;) {   \
            \
            /*!< condition is satisfied */  \
            if (condition)  \
                break;  \
            \
            /*!< signal pending (by calling "wake_up") */   \
            if (__wait_is_interruptible(&sgtc_wq) && thread_state_pending(sgtc_wq.sptr_task)) \
                break;  \
            \
            func; \
            \
        }   \
        \
        thread_state_signal(sgtc_wq.sptr_task, state, false);   \
        remove_wait_queue(sptr_wqh, &sgtc_wq);  \
    }   \
})

#define wait_event(sptr_wqh, condition) \
    do {    \
        if (condition)  \
            break;  \
        (void)__wait_event(sptr_wqh, condition, 0, schedule_thread());    \
    } while (0)

#define wait_event_timeout(sptr_wqh, condition, timeout) \
    do {    \
        if (condition)  \
            break;  \
        (void)__wait_event(sptr_wqh, condition, 0, schedule_timeout(timeout));    \
    } while (0)

#define wait_event_interruptible(sptr_wqh, condition) \
    do {    \
        if (condition)  \
            break;  \
        (void)__wait_event(sptr_wqh, condition, 1, schedule_thread());    \
    } while (0)

#define wait_event_interruptible_timeout(sptr_wqh, condition, timeout) \
    do {    \
        if (condition)  \
            break;  \
        (void)__wait_event(sptr_wqh, condition, 1, schedule_timeout(timeout));    \
    } while (0)

#define wake_up(sptr_wqh)   \
    do {    \
        wake_up_common(sptr_wqh, NR_THREAD_SIG_NORMAL);    \
    } while (0)

#define wake_up_interruptible(sptr_wqh)   \
    do {    \
        wake_up_common(sptr_wqh, NR_THREAD_SIG_INTR);    \
    } while (0)

#ifdef __cplusplus
    }
#endif

#endif /* __KERNEL_WAIT_H_ */
