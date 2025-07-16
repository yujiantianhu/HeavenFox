/*
 * Kernel Schedualer Interface
 *
 * File Name:   sched.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.09
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <kernel/kernel.h>
#include <kernel/sched.h>

/*!< The globals */
/*!< TCB */
struct scheduler_table sgtc_scheduler_table =
{
    .max_tidarr		= 0,
    .max_tids		= THREAD_MAX_NUM,
    .max_tidset		= 0,
    .ref_tidarr		= 0,
    .sgtc_cnt		= {},

    .sgtc_ready		= LIST_HEAD_INIT(&sgtc_scheduler_table.sgtc_ready),
    .sgtc_suspend	= LIST_HEAD_INIT(&sgtc_scheduler_table.sgtc_suspend),
    .sgtc_sleep		= LIST_HEAD_INIT(&sgtc_scheduler_table.sgtc_sleep),

    .sptr_work		= mr_nullptr,
    .sptr_tids		= mr_nullptr,
    .sptr_tid_array	= { mr_nullptr },
    .sgtc_lock		= SPIN_LOCK_INIT(),
};

/*!< save to (*.data) section, do not defines in stack */
static struct scheduler_context sgtc_context;
static kuint32_t thread_schedule_ref = 0;

struct atomic sgtc_sched_preempt_cnt = ATOMIC_INIT();

/*!< The defines */
#define SCHED_MANAGER()                         (&sgtc_scheduler_table)

#define SCHED_THREAD_HANDLER(tid)               __THREAD_HANDLER(&sgtc_scheduler_table, tid)
#define SCHED_RUNNING_THREAD                    (sgtc_scheduler_table.sptr_work)
#define SCHED_READY_LIST                        (&sgtc_scheduler_table.sgtc_ready)
#define SCHED_SUSPEND_LIST                      (&sgtc_scheduler_table.sgtc_suspend)
#define SCHED_SLEEP_LIST                        (&sgtc_scheduler_table.sgtc_sleep)
#define __SCHED_LOCK                            (sgtc_scheduler_table.sgtc_lock)

#define SCHED_READY_HASH                        __THREAD_READY_HASH(&sgtc_scheduler_table)
#define SCHED_SUSPEND_HASH                      __THREAD_SUSPEND_HASH(&sgtc_scheduler_table)
#define SCHED_SLEEP_HASH                        __THREAD_SLEEP_HASH(&sgtc_scheduler_table)

/*!< The functions */
static kint32_t __find_thread_from_scheduler(tid_t tid, struct list_head *sptr_head);

static kint32_t __schedule_add_status_list(struct thread *sptr_thread, 
                                struct list_head *sptr_head, struct thread_list *sptr_hash);
static void __schedule_del_status_list(struct thread *sptr_thread, 
                                struct list_head *sptr_head, struct thread_list *sptr_hash);

static kint32_t schedule_despoil_work_role(struct thread *sptr_thread);
static kint32_t schedule_reinstall_work_role(void);
static kint32_t schedule_add_ready_list(struct thread *sptr_thread);
static kint32_t schedule_detach_ready_list(struct thread *sptr_thread);
static kint32_t schedule_add_suspend_list(struct thread *sptr_thread);
static kint32_t schedule_detach_suspend_list(struct thread *sptr_thread);
static kint32_t schedule_add_sleep_list(struct thread *sptr_thread);
static kint32_t schedule_detach_sleep_list(struct thread *sptr_thread);

/* -------------------------------------------------------------------------- */
/*!< API functions */
/*!
 * @brief	get current thread from tcb
 * @param  	tid
 * @retval 	running thread
 * @note   	none
 */
struct thread *get_current_thread(void)
{
    return SCHED_RUNNING_THREAD;
}

/*!
 * @brief	get ready thread list head from tcb
 * @param  	tid
 * @retval 	ready thread list
 * @note   	none
 */
struct list_head *get_ready_thread_table(void)
{
    return SCHED_READY_LIST;
}

/*!
 * @brief	get thread from tcb
 * @param  	tid
 * @retval 	thread
 * @note   	none
 */
struct thread *get_thread_handle(tid_t tid)
{
    if (tid >= THREAD_MAX_NUM)
        return mr_nullptr;
    
    return SCHED_THREAD_HANDLER(tid);
}

/*!
 * @brief	set name to thread
 * @param  	name: thread name
 * @retval 	none
 * @note   	none
 */
void thread_set_name(tid_t tid, const kchar_t *name)
{
    struct thread *sptr_thread;

    if (!name || !(*name))
        return;

    sptr_thread = SCHED_THREAD_HANDLER(tid);

    memset(sptr_thread->name, 0, THREAD_NAME_SIZE);
    kstrlcpy(sptr_thread->name, name, THREAD_NAME_SIZE);
}

/*!
 * @brief	set name to current thread
 * @param  	name: thread name
 * @retval 	none
 * @note   	none
 */
void thread_set_self_name(const kchar_t *name)
{
    struct thread *sptr_work;

    if (!name || !(*name))
        return;

    sptr_work = SCHED_RUNNING_THREAD;

    memset(sptr_work->name, 0, THREAD_NAME_SIZE);
    kstrlcpy(sptr_work->name, name, THREAD_NAME_SIZE);
}

/*!
 * @brief	get thread'name
 * @param  	tid: thread id
 * @retval 	name
 * @note   	none
 */
kchar_t *thread_get_name(tid_t tid)
{
    struct thread *sptr_thread;

    sptr_thread = SCHED_THREAD_HANDLER(tid);
    return sptr_thread->name;
}

/*!
 * @brief	get current thread'name
 * @param  	none
 * @retval 	name
 * @note   	none
 */
kchar_t *thread_get_self_name(void)
{
    struct thread *sptr_work;

    sptr_work = SCHED_RUNNING_THREAD;
    return sptr_work->name;
}

/*!
 * @brief	set state to thread
 * @param  	sptr_thread: name handler
 * @retval 	none
 * @note   	none
 */
void thread_set_state(struct thread *sptr_thread, kuint32_t state)
{
    __SET_THREAD_TARGET_STATE(sptr_thread, state);
}

/*!
 * @brief   get the scheduler lock
 * @param   none
 * @retval  lock
 * @note    none
 */
struct spin_lock *scheduler_lock(void)
{
    return &__SCHED_LOCK;
}

/*!
 * @brief	find a free tid
 * @param  	i_start: base
 * @param	count: limit
 * @retval 	none
 * @note   	none
 */
tid_t get_unused_tid_from_scheduler(kuint32_t i_start, kuint32_t count)
{
    kuint32_t i;

    spin_lock_irqsave(&__SCHED_LOCK);
    for (i = i_start; i < (i_start + count); i++)
    {
        if (!SCHED_THREAD_HANDLER(i))
        {
            spin_unlock_irqrestore(&__SCHED_LOCK);
            return i;
        }
    }

    spin_unlock_irqrestore(&__SCHED_LOCK);

    return -ER_MORE;
}

/*!
 * @brief	stat on the number of scheduling
 * @param  	none
 * @retval 	none
 * @note   	none
 */
static void scheduler_record(void)
{
    struct scheduler_table *sptr_tab = SCHED_MANAGER();

    if ((sptr_tab->sgtc_cnt.sched_cnt++) >= __THREAD_MAX_STATS)
    {
        sptr_tab->sgtc_cnt.sched_cnt = 0;
        sptr_tab->sgtc_cnt.cnt_out++;
    }
}

/*!
 * @brief	get the stats of scheduling
 * @param  	none
 * @retval 	stats
 * @note   	none
 */
kuint64_t scheduler_stats_get(void)
{
    struct scheduler_table *sptr_tab = SCHED_MANAGER();
    kuint64_t sum;

    spin_lock_irqsave(&__SCHED_LOCK);
    sum = (__THREAD_MAX_STATS * sptr_tab->sgtc_cnt.cnt_out + sptr_tab->sgtc_cnt.sched_cnt);
    spin_unlock_irqrestore(&__SCHED_LOCK);

    return sum;
}

/*!
 * @brief	suspend current thread
 * @param  	none
 * @retval 	0: fail; 1: succuess
 * @note   	suspend current thread, and switch to next
 */
void schedule_self_suspend(void)
{
    struct thread *sptr_cur = SCHED_RUNNING_THREAD;

    spin_lock_irqsave(&sptr_cur->sgtc_lock);
    
    /*!< Avoid preempting while the function running */
    if (mr_likely(__GET_THREAD_STATE(sptr_cur) == NR_THREAD_RUNNING))
        __SET_THREAD_TARGET_STATE(sptr_cur, NR_THREAD_SUSPEND);

    spin_unlock_irqrestore(&sptr_cur->sgtc_lock);
    schedule_thread();
}

/*!
 * @brief	kill current thread
 * @param  	none
 * @retval 	0: fail; 1: succuess
 * @note   	kill current thread, and switch to next
 */
void schedule_self_sleep(void)
{
    struct thread *sptr_cur = SCHED_RUNNING_THREAD;

    spin_lock_irqsave(&sptr_cur->sgtc_lock);
    
    /*!< Avoid preempting while the function running */
    if (mr_likely(__GET_THREAD_STATE(sptr_cur) == NR_THREAD_RUNNING))
        __SET_THREAD_TARGET_STATE(sptr_cur, NR_THREAD_SLEEP);

    spin_unlock_irqrestore(&sptr_cur->sgtc_lock);
    schedule_thread();
}

/*!
 * @brief	suspend another thread
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	none
 */
kint32_t schedule_thread_suspend(tid_t tid)
{
    struct thread *sptr_thread;
    kint32_t retval;

    sptr_thread = SCHED_THREAD_HANDLER(tid);
    if (mr_unlikely(!sptr_thread))
        return -ER_NODEV;

    spin_lock_irqsave(&sptr_thread->sgtc_lock);
    __SET_THREAD_TARGET_STATE(sptr_thread, NR_THREAD_SUSPEND);

    if (mr_unlikely(__GET_THREAD_STATE(sptr_thread) == NR_THREAD_RUNNING))
    {
        spin_unlock_irqrestore(&sptr_thread->sgtc_lock);

        /*!< Self suspend */
        schedule_thread();
        return ER_NORMAL;
    }

    spin_unlock_irqrestore(&sptr_thread->sgtc_lock);

    spin_lock_irqsave(&__SCHED_LOCK);
    retval = schedule_thread_switch(tid);
    spin_unlock_irqrestore(&__SCHED_LOCK);
    
    return retval;
}

/*!
 * @brief	kill another thread
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	none
 */
kint32_t schedule_thread_sleep(tid_t tid)
{
    struct thread *sptr_thread;
    kint32_t retval;

    sptr_thread = SCHED_THREAD_HANDLER(tid);
    if (mr_unlikely(!sptr_thread))
        return -ER_NODEV;

    spin_lock_irqsave(&sptr_thread->sgtc_lock);
    __SET_THREAD_TARGET_STATE(sptr_thread, NR_THREAD_SLEEP);

    if (mr_unlikely(__GET_THREAD_STATE(sptr_thread) == NR_THREAD_RUNNING))
    {
        spin_unlock_irqrestore(&sptr_thread->sgtc_lock);

        /*!< Self suspend */
        schedule_thread();
        return ER_NORMAL;
    }

    spin_unlock_irqrestore(&sptr_thread->sgtc_lock);

    spin_lock_irqsave(&__SCHED_LOCK);
    retval = schedule_thread_switch(tid);
    spin_unlock_irqrestore(&__SCHED_LOCK);

    return retval;
}

/*!
 * @brief	wake up another thread
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	if target thread is suspending, wake it up and add to ready list
 */
kint32_t schedule_thread_wakeup(tid_t tid)
{
    struct thread *sptr_thread;
    kuint32_t state;
    kint32_t retval;

    sptr_thread = SCHED_THREAD_HANDLER(tid);
    if (mr_unlikely(!sptr_thread))
        return -ER_NODEV;

    spin_lock_irqsave(&sptr_thread->sgtc_lock);

    if (sptr_thread == SCHED_RUNNING_THREAD)
    {
        __SYNC_THREAD_STATE(sptr_thread, NR_THREAD_RUNNING);
        spin_unlock_irqrestore(&sptr_thread->sgtc_lock);

        return -ER_FORBID;
    }

    state = __GET_THREAD_STATE(sptr_thread);
    if ((state != NR_THREAD_SUSPEND) &&
        (state != NR_THREAD_SLEEP))
    {
        spin_unlock_irqrestore(&sptr_thread->sgtc_lock);
        return -ER_INVALID;
    }

    __SET_THREAD_TARGET_STATE(sptr_thread, NR_THREAD_READY);
    spin_unlock_irqrestore(&sptr_thread->sgtc_lock);

    spin_lock_irqsave(&__SCHED_LOCK);
    retval = schedule_thread_switch(tid);
	spin_unlock_irqrestore(&__SCHED_LOCK);

    return retval;
}

/*!
 * @brief	check if ready list is empty
 * @param  	none
 * @retval 	1: empty; 0: not empty
 * @note   	none
 */
kbool_t is_ready_thread_empty(void)
{
    return !!mr_list_empty(SCHED_READY_LIST);
}

/*!
 * @brief	check if suspend list is empty
 * @param  	none
 * @retval 	1: empty; 0: not empty
 * @note   	none
 */
kbool_t is_suspend_thread_empty(void)
{
    return !!mr_list_empty(SCHED_SUSPEND_LIST);
}

/*!
 * @brief	check if sleep list is empty
 * @param  	none
 * @retval 	1: empty; 0: not empty
 * @note   	none
 */
kbool_t is_sleep_thread_empty(void)
{
    return !!mr_list_empty(SCHED_SLEEP_LIST);
}

/*!
 * @brief	get the highest ready thread (if ready list is not empty)
 * @param  	none
 * @retval 	first thread
 * @note   	none
 */
struct thread *get_first_ready_thread(void)
{
    kbool_t existed = mr_list_empty(SCHED_READY_LIST);
    return existed ? mr_nullptr : mr_list_first_entry(SCHED_READY_LIST, struct thread, sgtc_link);
}

/*!
 * @brief	get the highest suspend thread (if suspend list is not empty)
 * @param  	none
 * @retval 	first thread
 * @note   	none
 */
struct thread *get_first_suspend_thread(void)
{
    kbool_t existed = mr_list_empty(SCHED_SUSPEND_LIST);
    return existed ? mr_nullptr : mr_list_first_entry(SCHED_SUSPEND_LIST, struct thread, sgtc_link);
}

/*!
 * @brief	get the highest sleep thread (if sleep list is not empty)
 * @param  	none
 * @retval 	first thread
 * @note   	none
 */
struct thread *get_first_sleep_thread(void)
{
    kbool_t existed = mr_list_empty(SCHED_SLEEP_LIST);
    return existed ? mr_nullptr : mr_list_first_entry(SCHED_SLEEP_LIST, struct thread, sgtc_link);
}

/*!
 * @brief	check if thread is valid
 * @param  	none
 * @retval 	1: valid; 0: unvalid
 * @note   	none
 */
kbool_t is_thread_valid(tid_t tid)
{
    struct thread *sptr_thread = get_thread_handle(tid);
    return ((sptr_thread->state != NR_THREAD_SLEEP) && (sptr_thread->to_state != NR_THREAD_SLEEP));
}

/*!
 * @brief	get next ready thread
 * @param  	sptr_prev
 * @retval 	next
 * @note   	none
 */
struct thread *next_ready_thread(struct thread *sptr_prev)
{
    if (!sptr_prev)
        return get_first_ready_thread();

    if (mr_list_empty(SCHED_READY_LIST) ||
        mr_list_empty(&sptr_prev->sgtc_link) ||
        mr_list_head_until(sptr_prev, SCHED_READY_LIST, sgtc_link))
        return mr_nullptr;

    return mr_list_next_entry(sptr_prev, sgtc_link);
}

/*!
 * @brief	get next suspend thread
 * @param  	sptr_prev
 * @retval 	next
 * @note   	none
 */
struct thread *next_suspend_thread(struct thread *sptr_prev)
{
    if (!sptr_prev)
        return get_first_suspend_thread();

    if (mr_list_empty(SCHED_SUSPEND_LIST) ||
        mr_list_empty(&sptr_prev->sgtc_link) ||
        mr_list_head_until(sptr_prev, SCHED_SUSPEND_LIST, sgtc_link))
        return mr_nullptr;

    return mr_list_next_entry(sptr_prev, sgtc_link);
}

/*!
 * @brief	get next sleep thread
 * @param  	sptr_prev
 * @retval 	next
 * @note   	none
 */
struct thread *next_sleep_thread(struct thread *sptr_prev)
{
    if (!sptr_prev)
        return get_first_sleep_thread();

    if (mr_list_empty(SCHED_SLEEP_LIST) ||
        mr_list_empty(&sptr_prev->sgtc_link) ||
        mr_list_head_until(sptr_prev, SCHED_SLEEP_LIST, sgtc_link))
        return mr_nullptr;

    return mr_list_next_entry(sptr_prev, sgtc_link);
}

/*!
 * @brief	add new thread to hash
 * @param  	sptr_list: current hash
 * @param   sptr_thread: current thread
 * @retval 	prev thread (sptr_thread will be add to it's next)
 * @note   	none
 */
static struct list_head *__thread_hash_add(struct thread_list *sptr_list, struct thread *sptr_thread)
{
    kint32_t prio = thread_get_priority(sptr_thread->sptr_attr);
    struct thread_hash *sptr_hash = sptr_list->sgtc_hash + prio;
    struct thread *sptr_tail = sptr_hash->sptr_tail;
    struct list_head *sptr_prev = mr_nullptr;

    /*!< Here are other threads with the same priority; add to the tail of them, and update sptr_tail = new-thread */
    if (sptr_tail)
    {
        list_head_add_head(&sptr_tail->sgtc_hash, &sptr_thread->sgtc_hash);

        sptr_prev = &sptr_tail->sgtc_link;
        sptr_hash->sptr_tail = sptr_thread;
    }
    else
    {
        kint32_t prev_prio = THREAD_PROTY_NUM;
        
        /*!< No thread exists in the hash */
        list_head_add_head(&sptr_hash->sgtc_list, &sptr_thread->sgtc_hash);
        sptr_hash->sptr_tail = sptr_thread;

        if (prio < 64)
        {
            kuint64_t ffs_l = sptr_list->ffs_l;
            kuint64_t bit_nr = (1ULL << prio);
            kuint64_t mask = (bit_nr - 1) & ffs_l;

            sptr_list->ffs_l |= bit_nr;

            /*!< The higher priority thread exists, get the offset in ffs_l */
            if (mask)
                prev_prio = mr_flsll(mask) - 1;
        }
        else
        {
            kuint64_t ffs_h = sptr_list->ffs_h;
            kuint64_t ffs_l = sptr_list->ffs_l;
            kuint64_t bit_nr = (1ULL << (prio - 64U));
            kuint64_t mask = (bit_nr - 1) & ffs_h;

            sptr_list->ffs_h |= bit_nr;

            /*!< The higher priority thread exists, get the offset in ffs_h */
            if (mask)
                prev_prio = 64U + mr_flsll(mask) - 1;
            else if (ffs_l)
                prev_prio = mr_flsll(ffs_l) - 1;
        }

        if (prev_prio < THREAD_PROTY_NUM)
        {
            struct thread_hash *sptr_prevhash = sptr_list->sgtc_hash + prev_prio;
            sptr_prev = &sptr_prevhash->sptr_tail->sgtc_link;
        }
    }

    return sptr_prev;
}

/*!
 * @brief	del thread from hash
 * @param  	sptr_list: current hash
 * @param   sptr_thread: current thread
 * @retval 	none
 * @note   	none
 */
static void __thread_hash_remove(struct thread_list *sptr_list, struct thread *sptr_thread)
{
    kint32_t prio = thread_get_priority(sptr_thread->sptr_attr);
    struct thread_hash *sptr_hash = sptr_list->sgtc_hash + prio;
    struct thread *sptr_tail = sptr_hash->sptr_tail;

    /*!< Remove the tail */
    if (sptr_tail == sptr_thread)
    {
        if (mr_list_reach_head(&sptr_hash->sgtc_list, &sptr_thread->sgtc_hash))
        {
            /*!< It's the last thread */
            sptr_hash->sptr_tail = mr_nullptr;

            if (prio < 64)
                sptr_list->ffs_l &= ~(1ULL << prio);
            else
                sptr_list->ffs_h &= ~(1ULL << (prio - 64));
        }
        else
        {
            sptr_hash->sptr_tail = mr_list_prev_entry(sptr_thread, sgtc_hash);
        }
    }

    list_head_del(&sptr_thread->sgtc_hash);
}

/*!
 * @brief	switch thread from one state to another state
 * @param  	tid: target thread
 * @param	src: current state
 * @param	dst: target state
 * @retval 	err code
 * @note   	only the running thread need to save context; and only the ready thread maybe need to restore context
 */ 
kint32_t schedule_thread_switch(tid_t tid)
{
    struct thread *sptr_thread;
    kuint32_t src, dst;
    kint32_t retval;
    
    /*!< Protected by caller, do not disable again */
//  mr_preempt_disable();

    sptr_thread = SCHED_THREAD_HANDLER(tid);
    if (mr_unlikely(!sptr_thread))
        return -ER_NODEV;

    src = sptr_thread->state;
    dst = sptr_thread->to_state;

    /*!<
     * thread switch:
     * (At all times, it is necessary to ensure that at least one thread (including idle threads) is running)
     * running ---> ready/suspend/sleep
     * ready ---> running/suspend/sleep
     * suspend ---> ready/sleep
     * sleep ---> ready/suspend
     *
     * (only running and ready state can be switched to any state)
     */
    if (mr_unlikely(src == dst) || 
        mr_unlikely(dst >= NR_THREAD_STATUS_MAX))
        goto fail;

    /*!< for idle thread, only ready and running state can be chosen */
    if ((tid == THREAD_TID_IDLE) && 
        mr_unlikely((dst != NR_THREAD_RUNNING) && (dst != NR_THREAD_READY)))
        goto fail;

    /*!< detached from current list */
    switch (src)
    {
        case NR_THREAD_RUNNING:
            retval = schedule_reinstall_work_role();
            if (mr_unlikely(retval))
                goto fail;
            
            break;

        case NR_THREAD_READY:
            schedule_detach_ready_list(sptr_thread);
            break;

        case NR_THREAD_SUSPEND:
            schedule_detach_suspend_list(sptr_thread);
            break;

        case NR_THREAD_SLEEP:
            schedule_detach_sleep_list(sptr_thread);
            break;

        default:
            break;
    }

    mr_barrier();

    /*!< add to new list */
    switch (dst)
    {
        case NR_THREAD_RUNNING:
            retval = schedule_despoil_work_role(sptr_thread);
            break;

        case NR_THREAD_READY:
            retval = schedule_add_ready_list(sptr_thread);
            break;

        case NR_THREAD_SUSPEND:
            retval = schedule_add_suspend_list(sptr_thread);
            break;

        case NR_THREAD_SLEEP:
            retval = schedule_add_sleep_list(sptr_thread);
            break;

        default:
            retval = -ER_ERROR;
            break;
    }

    if (mr_unlikely(retval < 0))
    {
        print_warn("switch thread failed ! current and target state is : %d, %d\r\n", src, dst);
        goto fail;
    }

    if (mr_unlikely(!SCHED_RUNNING_THREAD))
    {
        print_err("no thread is running !!! dangerous action !!!\r\n");
        goto fail;
    }

    /*!< update thread state */
    __SYNC_THREAD_STATE(sptr_thread, dst);

//  mr_preempt_enable();

    return ER_NORMAL;
    
fail:
    __SYNC_THREAD_STATE(sptr_thread, src);
//  mr_preempt_enable();

    return -ER_INVALID;
}

/*!
 * @brief	change thread state to running
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	only ready state can be switched to running!!!
 */
static kint32_t schedule_despoil_work_role(struct thread *sptr_thread)
{
    struct thread *sptr_running;
    kint32_t retval;

    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< only ready state can be switched to running */
    if (mr_unlikely(NR_THREAD_READY != sptr_thread->state))
        return -ER_INVALID;

    /*!< get current */
    sptr_running = SCHED_RUNNING_THREAD;
    if (mr_likely(sptr_running))
    {
        /*!< current thread add to ready list */
        retval = schedule_add_ready_list(sptr_running);
        if (mr_unlikely(retval < 0))
            return retval;

        __SYNC_THREAD_STATE(sptr_thread, NR_THREAD_READY);
    }

    /*!< update current */
    SCHED_RUNNING_THREAD = sptr_thread;

    return ER_NORMAL;
}

/*!
 * @brief	change current thread state
 * @param  	none
 * @retval 	err code
 * @note   	running ---> xxx
 */
static kint32_t schedule_reinstall_work_role(void)
{
    struct thread *sptr_new;
    struct list_head *sptr_ready = SCHED_READY_LIST;

    /*!< no thread ready; current should be set to idle thread */
//  if (mr_list_empty(sptr_ready))
//      return -ER_FAULT;

    /*!< get the first ready thread */
    sptr_new = mr_list_first_valid_entry(sptr_ready, struct thread, sgtc_link);
    if (mr_likely(sptr_new))
    {
        /*!< detached from ready list */
        schedule_detach_ready_list(sptr_new);

        /*!< ready thread ---> running */
        SCHED_RUNNING_THREAD = sptr_new;
        mr_barrier();
        __SYNC_THREAD_STATE(sptr_new, NR_THREAD_RUNNING);

        return ER_NORMAL;
    }

    return -ER_FAILD;
}

/*!
 * @brief	change target thread state to ready
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	add to ready list
 */
static kint32_t schedule_add_ready_list(struct thread *sptr_thread)
{
    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< avoid duplicate additions */
    if (mr_unlikely(NR_THREAD_READY == sptr_thread->state))
        return -ER_INVALID;

    return __schedule_add_status_list(sptr_thread, SCHED_READY_LIST, SCHED_READY_HASH);
}

/*!
 * @brief	change target thread state from ready
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	del from ready list. it must be called by schedule_thread_switch, do not use alone !!!
 * 			(after detaching from the ready list, the thread will appear in a free state, 
 * 			so this function prohibits external calls to prevent the thread from leaving management and causing memory leakage)
 */
static kint32_t schedule_detach_ready_list(struct thread *sptr_thread)
{
    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< check if is in ready state */
    if (mr_unlikely(NR_THREAD_READY != sptr_thread->state))
        return -ER_INVALID;

    /*!< delete it */
    __schedule_del_status_list(sptr_thread, SCHED_READY_LIST, SCHED_READY_HASH);

    return ER_NORMAL;
}

/*!
 * @brief	change target thread state to suspend
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	add to suspend list
 */
static kint32_t schedule_add_suspend_list(struct thread *sptr_thread)
{
    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< avoid duplicate additions */
    if (mr_unlikely(NR_THREAD_SUSPEND == sptr_thread->state))
        return -ER_INVALID;

    return __schedule_add_status_list(sptr_thread, SCHED_SUSPEND_LIST, SCHED_SUSPEND_HASH);
}

/*!
 * @brief	change target thread state from suspend
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	del from suspend list. it must be called by schedule_thread_switch, do not use alone !!!
 * 			(after detaching from the suspend list, the thread will appear in a free state, 
 * 			so this function prohibits external calls to prevent the thread from leaving management and causing memory leakage)
 */
static kint32_t schedule_detach_suspend_list(struct thread *sptr_thread)
{
    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< check if is in ready state */
    if (mr_unlikely(NR_THREAD_SUSPEND != sptr_thread->state))
        return -ER_INVALID;

    /*!< delete it */
    __schedule_del_status_list(sptr_thread, SCHED_SUSPEND_LIST, SCHED_SUSPEND_HASH);

    return ER_NORMAL;
}

/*!
 * @brief	change target thread state to sleep
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	add to sleep list
 */
static kint32_t schedule_add_sleep_list(struct thread *sptr_thread)
{
    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< avoid duplicate additions */
    if (mr_unlikely(NR_THREAD_SLEEP == sptr_thread->state))
        return -ER_INVALID;

    return __schedule_add_status_list(sptr_thread, SCHED_SLEEP_LIST, SCHED_SLEEP_HASH);
}

/*!
 * @brief	change target thread state from sleep
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	del from sleep list. it must be called by schedule_thread_switch, do not use alone !!!
 * 			(after detaching from the sleep list, the thread will appear in a free state, 
 * 			so this function prohibits external calls to prevent the thread from leaving management and causing memory leakage)
 */
static kint32_t schedule_detach_sleep_list(struct thread *sptr_thread)
{
    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< check if is in ready state */
    if (mr_unlikely(NR_THREAD_SLEEP != sptr_thread->state))
        return -ER_INVALID;

    /*!< delete it */
    __schedule_del_status_list(sptr_thread, SCHED_SLEEP_LIST, SCHED_SLEEP_HASH);

    return ER_NORMAL;
}

/*!
 * @brief	check if target thread is in the target list
 * @param  	tid: target thread
 * @param	sptr_head: ready/suspend/sleep list
 * @retval 	err code
 * @note   	find thread if it is exsited
 */
__unused
static kint32_t __find_thread_from_scheduler(tid_t tid, struct list_head *sptr_head)
{
    struct thread *sptr_anyTask;
    struct thread *sptr_thread = SCHED_THREAD_HANDLER(tid); 

    if (mr_list_empty(sptr_head))
        return -ER_NOTFOUND;

    foreach_list_next_entry(sptr_anyTask, sptr_head, sgtc_link)
    {
        if (sptr_anyTask == sptr_thread)
            return tid;
    }

    return -ER_NOTFOUND;
}

/*!
 * @brief	add to target list
 * @param  	sptr_thread: target thread
 * @param	sptr_head: ready/suspend/sleep list
 * @retval 	err code
 * @note   	every thread will be sorted by priority
 */
static kint32_t __schedule_add_status_list(struct thread *sptr_thread, 
                            struct list_head *sptr_head, struct thread_list *sptr_hash)
{
    struct list_head *sptr_last;

    if (mr_unlikely(!mr_list_empty(&sptr_thread->sgtc_link)))
        return -ER_EXISTED;

    /*!< Update priority */
    thread_sync_priority(sptr_thread->sptr_attr);

    /*!< Add to hash, and return thread list */
    sptr_last = __thread_hash_add(sptr_hash, sptr_thread);
    if (!sptr_last)
        sptr_last = sptr_head;

    list_head_add_head(sptr_last, &sptr_thread->sgtc_link);

    return ER_NORMAL;
}

/*!
 * @brief	delete from target list
 * @param  	sptr_thread: target thread
 * @param	sptr_head: ready/suspend/sleep list
 * @retval 	err code
 * @note   	none
 */
static void __schedule_del_status_list(struct thread *sptr_thread, 
                            struct list_head *sptr_head, struct thread_list *sptr_hash)
{
    __thread_hash_remove(sptr_hash, sptr_thread);

    /*!< check if target link is in the list before deleting, but needs lot of time */
//  list_head_del_safe(sptr_head, &sptr_thread->sgtc_link);
    list_head_del(&sptr_thread->sgtc_link);
}

/*!
 * @brief	register a new thread, which will be added to ready list
 * @param  	sptr_thread: target thread
 * @param	tid: global tcb index
 * @retval 	err code
 * @note   	all new threads should be added to ready list at first
 */
kint32_t register_new_thread(struct thread *sptr_thread, tid_t tid)
{
    struct thread_attr *sptr_it_attr;
    kint32_t retval;

    sptr_it_attr = sptr_thread->sptr_attr;

    if (SCHED_THREAD_HANDLER(tid))
        return -ER_INVALID;

    /*!< stack must be valid */
    if (!sptr_it_attr->stack_addr)
        return -ER_NOMEM;

    spin_lock_irqsave(&__SCHED_LOCK);
    /*!< saved to tcb */
    SCHED_THREAD_HANDLER(tid) = sptr_thread;

    /*!< initial link */
    init_list_head(&sptr_thread->sgtc_link);
    init_list_head(&sptr_thread->sgtc_hash);

    /*!< initial spinlock */
    spin_lock_init(&sptr_thread->sgtc_lock);

    /*!< set name */
    sprintk(sptr_thread->name, "thread-%d", tid);

    /*!< add and sorted by priority */
    retval = schedule_add_ready_list(sptr_thread);
    if (retval < 0)
    {
        SCHED_THREAD_HANDLER(tid) = mr_nullptr;
        spin_unlock_irqrestore(&__SCHED_LOCK);
        return retval;
    }

    /*!< set to ready state */
    __SYNC_THREAD_STATE(sptr_thread, NR_THREAD_READY);
    spin_unlock_irqrestore(&__SCHED_LOCK);

    return ER_NORMAL;
}

/*!
 * @brief	unregister a thread, which will be deleted
 * @param	tid: global tcb index
 * @retval 	sptr_thread
 * @note   	all threads should be deleted from sleep list
 */
struct thread *unregister_thread(tid_t tid)
{
    struct thread *sptr_thread;

    if ((tid < 0) || (tid == mr_current->tid))
        return ERR_PTR(-ER_LOCKED);

    sptr_thread = SCHED_THREAD_HANDLER(tid);
    if (!sptr_thread)
        return mr_nullptr;

    if (sptr_thread->state != NR_THREAD_SLEEP)
        return ERR_PTR(-ER_BUSY);

    spin_lock_irqsave(&__SCHED_LOCK);
    schedule_detach_sleep_list(sptr_thread);
    SCHED_THREAD_HANDLER(tid) = mr_nullptr;
    spin_unlock_irqrestore(&__SCHED_LOCK);

    return sptr_thread;
}

/*!
 * @brief	init thread (ready to running)
 * @param  	none
 * @retval 	none
 * @note   	reload time slice
 */
void __thread_init_before(void)
{
    struct thread *sptr_thread = SCHED_RUNNING_THREAD;
    kuint32_t milseconds = thread_get_sched_msecs(sptr_thread->sptr_attr);
    
    sptr_thread->expires = msecs_to_jiffies(milseconds);
}

/*!
 * @brief	start schedule (ready to running)
 * @param  	none
 * @retval 	none
 * @note   	select a highest priority thread from ready list, and swicth it to running state
 */
struct scheduler_context *__schedule_thread(void)
{
    struct thread *sptr_thread;
    struct thread *sptr_prev;
    struct thread_list *sptr_hash;
    kint32_t retval;

    sptr_hash = SCHED_READY_HASH;

    /*!< no ready thread here, unable to start or switch */
    if (__THREAD_HASH_EMPTY(sptr_hash))
        goto fail;

    /*!< get the current thread */
    sptr_prev = SCHED_RUNNING_THREAD;
    if (mr_unlikely(!sptr_prev))
    {
        if (mr_unlikely(thread_schedule_ref))
            goto fail;
        else
        {
            /*!< scheduled by "start_kernel" for the first time */
            sptr_prev = mr_list_first_entry(SCHED_READY_LIST, struct thread, sgtc_link);           
            __SET_THREAD_TARGET_STATE(sptr_prev, NR_THREAD_RUNNING);
        }
    }
    
    /*!< if not set target state, default to ready */
    if (NR_THREAD_NONE == sptr_prev->to_state)
        __SET_THREAD_TARGET_STATE(sptr_prev, NR_THREAD_READY);

    /*!< select next valid thread */
    retval = schedule_thread_switch(sptr_prev->tid);
    sptr_thread = SCHED_RUNNING_THREAD;
    if (mr_unlikely(retval < 0) || 
        mr_unlikely(!sptr_thread))
        goto fail;
    
    sgtc_context.first = (kuaddr_t)&thread_schedule_ref;
    sgtc_context.entry = (kuaddr_t)&sptr_thread->start_routine;
    sgtc_context.args = (kuaddr_t)&sptr_thread->ptr_args;
    sgtc_context.prev_sp = 0;
    sgtc_context.next_sp = thread_get_stack(sptr_thread->sptr_attr);

    if (mr_likely(thread_schedule_ref))
        sgtc_context.prev_sp = thread_get_stack(sptr_prev->sptr_attr);

    scheduler_record();

    /*!< address of sgtc_context ===> r0 */
    return &sgtc_context;

fail:
    return mr_nullptr;
}

/*!
 * @brief	start schedule (ready to running)
 * @param  	none
 * @retval 	none
 * @note   	select a highest priority thread from ready list, and swicth it to running state
 */
void schedule_thread(void)
{
    struct scheduler_context *sptr_context;

    mr_preempt_disable();

    /*!< Save cpsr to spsr */
    __push_psr();
    mr_local_irq_disable();
    mr_preempt_enable();

    /*!< Not allow called by IRQ */
    if (mr_unlikely(IS_IN_INTERRUPT()))
    {
        __SYNC_THREAD_STATE(mr_current, NR_THREAD_RUNNING);
        mr_warn(false);

        goto END;
    }
   
    sptr_context = __schedule_thread();
    if (!sptr_context)
        goto END;

    /*!< sptr_context ===> r0 */
    context_switch(sptr_context);
    return;

END:
    __pop_psr();
}

/*!
 * @brief	initial hash
 * @param  	sptr_list: current hash
 * @retval 	none
 * @note   	none
 */
static void __scheduler_init(struct thread_list *sptr_list)
{
    struct thread_hash *sptr_hash;

    sptr_list->ffs_h = sptr_list->ffs_l = 0;
    spin_lock_init(&sptr_list->sgtc_lock);

    for (kint32_t i = 0; i < THREAD_PROTY_NUM; i++)
    {
        sptr_hash = sptr_list->sgtc_hash + i;

        sptr_hash->sptr_tail = mr_nullptr;
        init_list_head(&sptr_hash->sgtc_list);
    }
}

/*!
 * @brief	initial all hash
 * @param  	none
 * @retval 	none
 * @note   	none
 */
void __init scheduler_init(void)
{
    __scheduler_init(SCHED_READY_HASH);
    __scheduler_init(SCHED_SUSPEND_HASH);
    __scheduler_init(SCHED_SLEEP_HASH);
}

/*!< end of file */
