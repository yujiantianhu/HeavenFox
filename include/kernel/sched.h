/*
 * Kernel Scheduler Defines
 *
 * File Name:   sched.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.09
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __KERNEL_SCHED_H_
#define __KERNEL_SCHED_H_

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <kernel/kernel.h>
#include <kernel/context.h>
#include <kernel/thread.h>
#include <kernel/spinlock.h>

/*!< The defines */
struct spin_lock;
struct mailbox;

#define THREAD_NAME_SIZE                        (32)

struct thread
{
    /*!< thread name */
    kchar_t name[THREAD_NAME_SIZE];

    /*!< thread id */
    kuint32_t tid;

    /*!< refer to "__ERT_THREAD_BASIC_STATUS" */
    kuint32_t status;
    kuint32_t to_status;

    /*!< thread entry */
    void *(*start_routine) (void *);
    struct thread_attr *sptr_attr;
    void *ptr_args;

    /*!< thread list (to ready/suspend/sleep list) */
    struct list_head sgtc_link;

    /*!< thread time slice (period = sptr_attr->sgtc_param.mr_sched_init_budget) */
    kutime_t expires;

    /*!< refer to "__ERT_THREAD_SIGNALS" */
    kuint32_t flags;

    struct spin_lock sgtc_lock;
    struct mailbox *sptr_mb;
};

#define mr_thread_set_flags(signal, sptr_tsk)	\
    do {	\
        sptr_tsk->flags |= mr_bit(signal);	\
    } while (0)

#define mr_thread_clr_flags(signal, sptr_tsk)	\
    do {	\
        sptr_tsk->flags &= ~mr_bit(signal);	\
    } while (0)

#define mr_thread_is_flags(signal, sptr_tsk)						(!!((sptr_tsk)->flags & mr_bit(signal)))

/*!< thread manage table */
struct scheduler_table
{
    kint32_t max_tidarr;											/*!< = THREAD_MAX_NUM */
    kint32_t max_tids; 												/*!< = THREAD_MAX_NUM + count of sptr_tids */
    kint32_t max_tidset;											/*!< the max tid */
    kint32_t ref_tidarr; 											/*!< number of allocated descriptors in sptr_tid_array */

    struct {
        kutype_t cnt_out;											/*!< when sched_cnt is over (~0), cnt_out++ */
        kutype_t sched_cnt;											/*!< schedule counter, max is ~0 */
    } sgtc_cnt;

    struct list_head sgtc_ready;									/*!< ready list head (manage all ready thread) */
    struct list_head sgtc_suspend;									/*!< suspend list head (manage all suspend thread) */
    struct list_head sgtc_sleep;									/*!< sleep list head (manage all sleepy thread) */

    struct thread *sptr_work;									    /*!< current thread (status is running) */

    struct thread **sptr_tids;									    /*!< if sptr_tid_array is up to max, new thread form mempool */
    struct thread *sptr_tid_array[THREAD_MAX_NUM];		            /*!< thread maximum, tid = 0 ~ THREAD_MAX_NUM */

    struct spin_lock sgtc_lock;

#define __THREAD_MAX_STATS					((kutype_t)(~0))
#define __THREAD_HANDLER(ptr, tid)			((ptr)->sptr_tid_array[(tid)])
#define __THREAD_RUNNING_LIST(ptr)			((ptr)->sptr_work)
#define __THREAD_READY_LIST(ptr)			(&((ptr)->sgtc_ready))
#define __THREAD_SUSPEND_LIST(ptr)			(&((ptr)->sgtc_suspend))
#define __THREAD_SLEEP_LIST(ptr)			(&((ptr)->sgtc_sleep))
};

/*!< The globals */

/*!< The functions */
extern struct thread *get_current_thread(void);
extern struct list_head *get_ready_thread_table(void);
extern struct thread *get_thread_handle(tid_t tid);
extern void thread_set_name(tid_t tid, const kchar_t *name);
extern void thread_set_self_name(const kchar_t *name);
extern void thread_set_state(struct thread *sptr_thread, kuint32_t state);
extern struct spin_lock *scheduler_lock(void);
extern tid_t get_unused_tid_from_scheduler(kuint32_t i_start, kuint32_t count);
extern kuint64_t scheduler_stats_get(void);
extern void schedule_self_suspend(void);
extern void schedule_self_sleep(void);
extern kint32_t schedule_thread_suspend(tid_t tid);
extern kint32_t schedule_thread_sleep(tid_t tid);
extern kint32_t schedule_thread_wakeup(tid_t tid);

extern kbool_t is_ready_thread_empty(void);
extern kbool_t is_suspend_thread_empty(void);
extern kbool_t is_sleep_thread_empty(void);
extern struct thread *get_first_ready_thread(void);
extern struct thread *get_first_suspend_thread(void);
extern struct thread *get_first_sleep_thread(void);
extern kbool_t is_thread_valid(tid_t tid);
extern struct thread *next_ready_thread(struct thread *sptr_prev);
extern struct thread *next_suspend_thread(struct thread *sptr_prev);
extern struct thread *next_sleep_thread(struct thread *sptr_prev);

extern kint32_t schedule_thread_switch(tid_t tid);
extern kint32_t register_new_thread(struct thread *sptr_thread, tid_t tid);
extern struct thread *unregister_thread(tid_t tid);
extern void __thread_init_before(void);
extern struct scheduler_context *__schedule_thread(void);
extern void schedule_thread(void);

/*!< The defines */
#define mr_current                             get_current_thread()
#define mr_tid_handle(tid)                     get_thread_handle(tid)
#define mr_tid_attr(tid)                       thread_attr_get(tid)

/*!< API functions */
/*!
 * @brief   get thread state
 * @param   sptr_thread
 * @retval  status
 * @note    none
 */
static inline kbool_t thread_state_pending(struct thread *sptr_thread)
{
    kbool_t is_wakeup, is_killed;

    spin_lock_irqsave(&sptr_thread->sgtc_lock);
    is_wakeup = mr_thread_is_flags(NR_THREAD_SIG_WAKEUP, sptr_thread);
    is_killed = mr_thread_is_flags(NR_THREAD_SIG_KILL, sptr_thread);

    mr_barrier();

    mr_thread_clr_flags(NR_THREAD_SIG_WAKEUP, sptr_thread);
    mr_thread_clr_flags(NR_THREAD_SIG_KILL, sptr_thread);
    spin_unlock_irqrestore(&sptr_thread->sgtc_lock);

    return (is_wakeup || is_killed);
}

/*!
 * @brief   set thread state
 * @param   sptr_thread, state, mode
 * @retval  none
 * @note    none
 */
static inline void thread_state_signal(struct thread *sptr_thread, kuint32_t state, kbool_t mode)
{
    spin_lock_irqsave(&sptr_thread->sgtc_lock);

    if (mode)
        mr_thread_set_flags(state, sptr_thread);
    else
        mr_thread_clr_flags(state, sptr_thread);
    
    spin_unlock_irqrestore(&sptr_thread->sgtc_lock);
}

#ifdef __cplusplus
    }
#endif

#endif /* __KERNEL_SCHED_H_ */
