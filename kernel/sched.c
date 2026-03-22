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
#include <kernel/preempt.h>
#include <kernel/sched.h>

/*!< The globals */
/*!< TCB */
struct scheduler_table sgtc_scheduler_table =
{
    .max_tidarr     = 0,
    .max_tids       = THREAD_MAX_NUM,
    .max_tidset     = 0,
    .ref_tidarr     = 0,

    .sptr_tids      = mr_nullptr,
    .sptr_tid_array = { mr_nullptr },
    .sgtc_lock      = SPIN_LOCK_INIT(),
};

/*!< save to (*.data) section, do not defines in stack */
DEFINE_PER_CPU(struct percpu_sched_data, sgtc_sched_data);
kbool_t g_kernel_preempt_enable = false;

/*!< The defines */
#define SCHED_MANAGER()                         (&sgtc_scheduler_table)
#define SCHED_MANAGER_CORE(_cpuid)              (&(sgtc_scheduler_table.sgtc_core[_cpuid]))
#define SCHED_MANAGER_SHARE()                   (&(sgtc_scheduler_table.sgtc_share))
#define __SCHED_LOCK                            (SCHED_MANAGER()->sgtc_lock)

#define SCHED_THREAD_HANDLER(tid)               __THREAD_HANDLER( SCHED_MANAGER(), tid )
#define SCHED_CORE_LOCK(_cpuid)                 __THREAD_CORE_LOCK( SCHED_MANAGER_CORE(_cpuid) )

#define SCHED_RUNNING_THREAD(_cpuid)            __THREAD_RUNNING( SCHED_MANAGER_CORE(_cpuid) )
#define SCHED_READY_LIST(_cpuid)                __THREAD_READY_LIST( SCHED_MANAGER_CORE(_cpuid) )
#define SCHED_SUSPEND_LIST(_cpuid)              __THREAD_SUSPEND_LIST( SCHED_MANAGER_CORE(_cpuid) )
#define SCHED_SLEEP_LIST(_cpuid)                __THREAD_SLEEP_LIST( SCHED_MANAGER_CORE(_cpuid) )
#define SCHED_ZOMBIE_LIST(_cpuid)               __THREAD_ZOMBIE_LIST( SCHED_MANAGER_CORE(_cpuid) )

#define SCHED_READY_HASH(_cpuid)                __THREAD_READY_HASH( SCHED_MANAGER_CORE(_cpuid) )
#define SCHED_SUSPEND_HASH(_cpuid)              __THREAD_SUSPEND_HASH( SCHED_MANAGER_CORE(_cpuid) )
#define SCHED_SLEEP_HASH(_cpuid)                __THREAD_SLEEP_HASH( SCHED_MANAGER_CORE(_cpuid) )
#define SCHED_ZOMBIE_HASH(_cpuid)               __THREAD_ZOMBIE_HASH( SCHED_MANAGER_CORE(_cpuid) )

/*!< Scheduler Operations */
struct scheduler_operation
{
    kint32_t (*detach)(kuint32_t, struct thread *);
    kint32_t (*add_new)(kuint32_t, struct thread *);
};

/*!< The functions */
static kint32_t __find_thread_from_scheduler(tid_t tid, struct list_head *sptr_head);

static kint32_t __schedule_add_status_list(struct thread *sptr_thread, 
                                struct list_head *sptr_head, struct thread_list *sptr_hash);
static void __schedule_del_status_list(struct thread *sptr_thread, 
                                struct list_head *sptr_head, struct thread_list *sptr_hash);

static kint32_t schedule_despoil_work_role(kuint32_t cpuid, struct thread *sptr_thread);
static kint32_t schedule_reinstall_work_role(kuint32_t cpuid, struct thread *sptr_thread);
static kint32_t schedule_add_ready_list(kuint32_t cpuid, struct thread *sptr_thread);
static kint32_t schedule_detach_ready_list(kuint32_t cpuid, struct thread *sptr_thread);
static kint32_t schedule_add_suspend_list(kuint32_t cpuid, struct thread *sptr_thread);
static kint32_t schedule_detach_suspend_list(kuint32_t cpuid, struct thread *sptr_thread);
static kint32_t schedule_add_sleep_list(kuint32_t cpuid, struct thread *sptr_thread);
static kint32_t schedule_detach_sleep_list(kuint32_t cpuid, struct thread *sptr_thread);
static kint32_t schedule_add_zombie_list(kuint32_t cpuid, struct thread *sptr_thread);
static kint32_t schedule_detach_zombie_list(kuint32_t cpuid, struct thread *sptr_thread);

/*!< The globals */
/*!< Scheduler Operations */
static const struct scheduler_operation sgtc_scheduler_operations[] =
{
    [NR_THREAD_NONE     ] = { .detach = mr_nullptr,                     .add_new = mr_nullptr                   },
    [NR_THREAD_RUNNING  ] = { .detach = schedule_despoil_work_role,     .add_new = schedule_reinstall_work_role },
    [NR_THREAD_READY    ] = { .detach = schedule_detach_ready_list,     .add_new = schedule_add_ready_list      },
    [NR_THREAD_SUSPEND  ] = { .detach = schedule_detach_suspend_list,   .add_new = schedule_add_suspend_list    },
    [NR_THREAD_SLEEP    ] = { .detach = schedule_detach_sleep_list,     .add_new = schedule_add_sleep_list      },
    [NR_THREAD_ZOMBIE   ] = { .detach = mr_nullptr,                     .add_new = schedule_add_zombie_list     },
};

/* -------------------------------------------------------------------------- */
/*!< API functions */
/*!
 * @brief	get scheduler manager
 * @param  	cpuid
 * @retval 	scheduler manager
 * @note   	none
 */
struct scheduler_core *get_scheduler_core(kuint32_t cpuid)
{
    return SCHED_MANAGER_CORE(cpuid);
}

/*!
 * @brief	get current thread from tcb
 * @param  	tid
 * @retval 	running thread
 * @note   	none
 */
struct thread *get_current_thread(void)
{
    return SCHED_RUNNING_THREAD(get_cpu_id());
}

/*!
 * @brief	get current thread from tcb
 * @param  	tid
 * @retval 	running thread
 * @note   	none
 */
struct thread *get_cpu_current_thread(kuint32_t cpuid)
{
    return SCHED_RUNNING_THREAD(cpuid);
}

/*!
 * @brief	get ready thread list head from tcb
 * @param  	tid
 * @retval 	ready thread list
 * @note   	none
 */
struct list_head *get_ready_thread_table(void)
{
    return SCHED_READY_LIST(get_cpu_id());
}

/*!
 * @brief	get thread from tcb
 * @param  	tid
 * @retval 	thread
 * @note   	none
 */
struct thread *get_thread_handle(tid_t tid)
{   
    return IS_TID_VALID(tid) ? SCHED_THREAD_HANDLER(tid) : mr_nullptr;
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
 * @brief	set name to thread
 * @param  	name: thread name
 * @retval 	none
 * @note   	none
 */
void thread_set_name_args(tid_t tid, const kchar_t *name, ...)
{
    struct thread *sptr_thread;
    va_list sptr_list;

    if (!name || !(*name))
        return;

    sptr_thread = SCHED_THREAD_HANDLER(tid);
    memset(sptr_thread->name, 0, THREAD_NAME_SIZE);

    va_start(sptr_list, name);
    vasprintk_limit(sptr_thread->name, THREAD_NAME_SIZE, name, sptr_list);
    va_end(sptr_list);
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

    sptr_work = current_thread();

    memset(sptr_work->name, 0, THREAD_NAME_SIZE);
    kstrlcpy(sptr_work->name, name, THREAD_NAME_SIZE);
}

/*!
 * @brief	set name to current thread
 * @param  	name: thread name
 * @retval 	none
 * @note   	none
 */
void thread_set_self_name_args(const kchar_t *name, ...)
{
    struct thread *sptr_work;
    va_list sptr_list;

    if (!name || !(*name))
        return;

    sptr_work = current_thread();
    memset(sptr_work->name, 0, THREAD_NAME_SIZE);

    va_start(sptr_list, name);
    vasprintk_limit(sptr_work->name, THREAD_NAME_SIZE, name, sptr_list);
    va_end(sptr_list);
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

    sptr_work = current_thread();
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
 * @brief   get the scheduler per-cpu lock
 * @param   none
 * @retval  lock
 * @note    none
 */
struct spin_lock *scheduler_cpu_lock(kuint32_t cpuid)
{
    return SCHED_CORE_LOCK(cpuid);
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
    kutype_t flags;

    spin_lock_irqsave(&__SCHED_LOCK, &flags);
    for (i = i_start; i < (i_start + count); i++)
    {
        if (!SCHED_THREAD_HANDLER(i))
        {
            spin_unlock_irqrestore(&__SCHED_LOCK, flags);
            return i;
        }
    }

    spin_unlock_irqrestore(&__SCHED_LOCK, flags);
    return -ER_MORE;
}

/*!
 * @brief	stat on the number of scheduling
 * @param  	none
 * @retval 	none
 * @note   	none
 */
static __unused 
void scheduler_record(kuint32_t cpuid)
{
    struct scheduler_core *sptr_core = SCHED_MANAGER_CORE(cpuid);

    if ((sptr_core->sgtc_cnt.sched_cnt++) >= __THREAD_MAX_STATS)
    {
        sptr_core->sgtc_cnt.sched_cnt = 0;
        sptr_core->sgtc_cnt.cnt_out++;
    }
}

/*!
 * @brief	get the stats of scheduling
 * @param  	none
 * @retval 	stats
 * @note   	none
 */
kuint64_t scheduler_stats_get(kuint32_t cpuid)
{
    struct scheduler_core *sptr_core = SCHED_MANAGER_CORE(cpuid);
    kuint64_t sum;
    kutype_t flags;

    spin_lock_irqsave(&sptr_core->sgtc_lock, &flags);
    sum = (__THREAD_MAX_STATS * sptr_core->sgtc_cnt.cnt_out + sptr_core->sgtc_cnt.sched_cnt);
    spin_unlock_irqrestore(&sptr_core->sgtc_lock, flags);

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
    struct thread *sptr_cur = current_thread();
    kutype_t flags;

    mr_preempt_disable();
    spin_lock_irqsave(&sptr_cur->sgtc_lock, &flags);
    
    /*!< Avoid preempting while the function running */
    if (mr_likely(__GET_THREAD_STATE(sptr_cur) == NR_THREAD_RUNNING))
        __SET_THREAD_TARGET_STATE(sptr_cur, NR_THREAD_SUSPEND);

    spin_unlock_irqrestore(&sptr_cur->sgtc_lock, flags);
    schedule_thread();
    mr_preempt_enable();
}

/*!
 * @brief	sleep current thread
 * @param  	none
 * @retval 	0: fail; 1: succuess
 * @note   	sleep current thread, and switch to next
 */
void schedule_self_sleep(void)
{
    struct thread *sptr_cur = current_thread();
    kutype_t flags;

    mr_preempt_disable();
    spin_lock_irqsave(&sptr_cur->sgtc_lock, &flags);
    
    /*!< Avoid preempting while the function running */
    if (mr_likely(__GET_THREAD_STATE(sptr_cur) == NR_THREAD_RUNNING))
        __SET_THREAD_TARGET_STATE(sptr_cur, NR_THREAD_SLEEP);

    spin_unlock_irqrestore(&sptr_cur->sgtc_lock, flags);
    schedule_thread();
    mr_preempt_enable();
}

/*!
 * @brief	kill current thread
 * @param  	none
 * @retval 	0: fail; 1: succuess
 * @note   	kill current thread, and switch to next
 */
void schedule_self_zombie(void)
{
    struct thread *sptr_cur = current_thread();
    kutype_t flags;

    mr_preempt_disable();
    spin_lock_irqsave(&sptr_cur->sgtc_lock, &flags);
    
    /*!< Avoid preempting while the function running */
    if (mr_likely(__GET_THREAD_STATE(sptr_cur) == NR_THREAD_RUNNING))
        __SET_THREAD_TARGET_STATE(sptr_cur, NR_THREAD_ZOMBIE);

    spin_unlock_irqrestore(&sptr_cur->sgtc_lock, flags);
    schedule_thread();
    mr_preempt_enable();
}

/*!
 * @brief	send IPI to another thread
 * @param  	sptr_thread: target thread
 * @retval 	none
 * @note   	none
 */
void send_state_to_thread(struct thread *sptr_thread, kuint32_t state)
{
    kuint32_t cpuid;
    struct spin_lock *sptr_lock;
    kutype_t flags;

    mr_preempt_disable();

loop:
    cpuid = sptr_thread->cpu;
    sptr_lock = scheduler_cpu_lock(cpuid);
    spin_lock_irqsave(sptr_lock, &flags);

    /*!< if cpu has more than 2 cores, sptr_thread may migrate between other cores that are not the current core */
    if (mr_unlikely(cpuid != sptr_thread->cpu))
    {
        cpuid = sptr_thread->cpu;
        spin_unlock_irqrestore(sptr_lock, flags);
        goto loop;
    }

    // send IPI
    // ...

    spin_unlock_irqrestore(sptr_lock, flags);
    mr_preempt_enable();
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
    kuint32_t cpuid;
    struct spin_lock *sptr_lock;
    kutype_t flags;
    kint32_t retval;

    sptr_thread = SCHED_THREAD_HANDLER(tid);
    if (mr_unlikely(!sptr_thread))
        return -ER_NODEV;

    mr_preempt_disable();
    spin_lock_irqsave(&sptr_thread->sgtc_lock, &flags);
    cpuid = sptr_thread->cpu;

    /*!< Only current cpu can schedule */
    if (__GET_THREAD_STATE(sptr_thread) == NR_THREAD_RUNNING)
    {
        if (cpuid != get_cpu_id())
        {
            spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

            /*!< send IPI to cpuid */
            send_state_to_thread(sptr_thread, NR_THREAD_SUSPEND);
            mr_preempt_enable();

            return -ER_FORBID;
        }

        __SET_THREAD_TARGET_STATE(sptr_thread, NR_THREAD_SUSPEND);
        spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);
        
        /*!< Self suspend */
        schedule_thread();
        mr_preempt_enable();

        return ER_NORMAL;
    }

    __SET_THREAD_TARGET_STATE(sptr_thread, NR_THREAD_SUSPEND);
    spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

loop:
    cpuid = sptr_thread->cpu;
    sptr_lock = scheduler_cpu_lock(cpuid);
    spin_lock_irqsave(sptr_lock, &flags);

    /*!< if cpu has more than 2 cores, sptr_thread may migrate between other cores that are not the current core */
    if (mr_unlikely(cpuid != sptr_thread->cpu))
    {
        spin_unlock_irqrestore(sptr_lock, flags);
        goto loop;
    }

    retval = schedule_thread_switch(sptr_thread);
    spin_unlock_irqrestore(sptr_lock, flags);

    mr_preempt_enable();
    return retval;
}

/*!
 * @brief	sleep another thread
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	none
 */
kint32_t schedule_thread_sleep(tid_t tid)
{
    struct thread *sptr_thread;
    kuint32_t cpuid;
    struct spin_lock *sptr_lock;
    kutype_t flags;
    kint32_t retval;

    sptr_thread = SCHED_THREAD_HANDLER(tid);
    if (mr_unlikely(!sptr_thread))
        return -ER_NODEV;

    mr_preempt_disable();
    spin_lock_irqsave(&sptr_thread->sgtc_lock, &flags);
    cpuid = sptr_thread->cpu;

    /*!< Only current cpu can schedule */
    if (mr_unlikely(__GET_THREAD_STATE(sptr_thread) == NR_THREAD_RUNNING))
    {
        if (cpuid != get_cpu_id())
        {
            spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

            /*!< send IPI to cpuid */
            send_state_to_thread(sptr_thread, NR_THREAD_SUSPEND);
            mr_preempt_enable();

            return -ER_FORBID;
        }

        __SET_THREAD_TARGET_STATE(sptr_thread, NR_THREAD_SLEEP);
        spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

        /*!< Self suspend */
        schedule_thread();
        mr_preempt_enable();

        return ER_NORMAL;
    }

    __SET_THREAD_TARGET_STATE(sptr_thread, NR_THREAD_SLEEP);
    spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

loop:
    cpuid = sptr_thread->cpu;
    sptr_lock = scheduler_cpu_lock(cpuid);
    spin_lock_irqsave(sptr_lock, &flags);

    /*!< if cpu has more than 2 cores, sptr_thread may migrate between other cores that are not the current core */
    if (mr_unlikely(cpuid != sptr_thread->cpu))
    {
        spin_unlock_irqrestore(sptr_lock, flags);
        goto loop;
    }

    retval = schedule_thread_switch(sptr_thread);
    spin_unlock_irqrestore(sptr_lock, flags);

    mr_preempt_enable();
    return retval;
}

/*!
 * @brief	kill another thread
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	none
 */
kint32_t schedule_thread_zombie(tid_t tid)
{
    struct thread *sptr_thread;
    kuint32_t cpuid;
    struct spin_lock *sptr_lock;
    kutype_t flags;
    kint32_t retval;

    sptr_thread = SCHED_THREAD_HANDLER(tid);
    if (mr_unlikely(!sptr_thread))
        return -ER_NODEV;

    mr_preempt_disable();
    spin_lock_irqsave(&sptr_thread->sgtc_lock, &flags);
    cpuid = sptr_thread->cpu;

    /*!< Only current cpu can schedule */
    if (mr_unlikely(__GET_THREAD_STATE(sptr_thread) == NR_THREAD_RUNNING))
    {
        if (cpuid != get_cpu_id())
        {
            spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

            /*!< send IPI to cpuid */
            send_state_to_thread(sptr_thread, NR_THREAD_SUSPEND);
            mr_preempt_enable();

            return -ER_FORBID;
        }

        __SET_THREAD_TARGET_STATE(sptr_thread, NR_THREAD_ZOMBIE);
        spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

        /*!< Self suspend */
        schedule_thread();
        mr_preempt_enable();

        return ER_NORMAL;
    }

    __SET_THREAD_TARGET_STATE(sptr_thread, NR_THREAD_ZOMBIE);
    spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

loop:
    cpuid = sptr_thread->cpu;
    sptr_lock = scheduler_cpu_lock(cpuid);
    spin_lock_irqsave(sptr_lock, &flags);

    /*!< if cpu has more than 2 cores, sptr_thread may migrate between other cores that are not the current core */
    if (mr_unlikely(cpuid != sptr_thread->cpu))
    {
        spin_unlock_irqrestore(sptr_lock, flags);
        goto loop;
    }

    retval = schedule_thread_switch(sptr_thread);
    spin_unlock_irqrestore(sptr_lock, flags);

    mr_preempt_enable();
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
    struct spin_lock *sptr_lock;
    kuint32_t state, cpuid;
    kutype_t flags;
    kint32_t retval;

    sptr_thread = SCHED_THREAD_HANDLER(tid);
    if (mr_unlikely(!sptr_thread))
        return -ER_NODEV;

    spin_lock_irqsave(&sptr_thread->sgtc_lock, &flags);
    if (mr_unlikely(sptr_thread == current_thread()))
    {
        __SYNC_THREAD_STATE(sptr_thread, NR_THREAD_RUNNING);
        spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

        return -ER_FORBID;
    }

    /*!< exclude NR_THREAD_ZOMBIE */
    state = __GET_THREAD_STATE(sptr_thread);
    if ((state != NR_THREAD_SUSPEND) &&
        (state != NR_THREAD_SLEEP))
    {
        spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);
        return -ER_INVALID;
    }

    __SET_THREAD_TARGET_STATE(sptr_thread, NR_THREAD_READY);
    spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

    mr_preempt_disable();

loop:
    cpuid = sptr_thread->cpu;
    sptr_lock = scheduler_cpu_lock(cpuid);
    spin_lock_irqsave(sptr_lock, &flags);

    /*!< if cpu has more than 2 cores, sptr_thread may migrate between other cores that are not the current core */
    if (mr_unlikely(cpuid != sptr_thread->cpu))
    {
        spin_unlock_irqrestore(sptr_lock, flags);
        goto loop;
    }

    retval = schedule_thread_switch(sptr_thread);
	spin_unlock_irqrestore(sptr_lock, flags);

    mr_preempt_enable();
    return retval;
}

/*!
 * @brief	check if ready list is empty
 * @param  	none
 * @retval 	1: empty; 0: not empty
 * @note   	none
 */
kbool_t is_ready_thread_empty(kuint32_t cpuid)
{
    DECLARE_LIST_HEAD_PTR_INIT(sptr_list, SCHED_READY_LIST(cpuid));
    return !!mr_list_empty(sptr_list);
}

/*!
 * @brief	check if suspend list is empty
 * @param  	none
 * @retval 	1: empty; 0: not empty
 * @note   	none
 */
kbool_t is_suspend_thread_empty(kuint32_t cpuid)
{
    DECLARE_LIST_HEAD_PTR_INIT(sptr_list, SCHED_SUSPEND_LIST(cpuid));
    return !!mr_list_empty(sptr_list);
}

/*!
 * @brief	check if sleep list is empty
 * @param  	none
 * @retval 	1: empty; 0: not empty
 * @note   	none
 */
kbool_t is_sleep_thread_empty(kuint32_t cpuid)
{
    DECLARE_LIST_HEAD_PTR_INIT(sptr_list, SCHED_SLEEP_LIST(cpuid));
    return !!mr_list_empty(sptr_list);
}

/*!
 * @brief	get the highest ready thread (if ready list is not empty)
 * @param  	none
 * @retval 	first thread
 * @note   	none
 */
struct thread *get_first_ready_thread(kuint32_t cpuid)
{
    DECLARE_LIST_HEAD_PTR_INIT(sptr_list, SCHED_READY_LIST(cpuid));
    kbool_t existed = mr_list_empty(sptr_list);

    return existed ? mr_nullptr : mr_list_first_entry(sptr_list, struct thread, sgtc_link);
}

/*!
 * @brief	get the highest suspend thread (if suspend list is not empty)
 * @param  	none
 * @retval 	first thread
 * @note   	none
 */
struct thread *get_first_suspend_thread(kuint32_t cpuid)
{
    DECLARE_LIST_HEAD_PTR_INIT(sptr_list, SCHED_SUSPEND_LIST(cpuid));
    kbool_t existed = mr_list_empty(sptr_list);

    return existed ? mr_nullptr : mr_list_first_entry(sptr_list, struct thread, sgtc_link);
}

/*!
 * @brief	get the highest sleep thread (if sleep list is not empty)
 * @param  	none
 * @retval 	first thread
 * @note   	none
 */
struct thread *get_first_sleep_thread(kuint32_t cpuid)
{
    DECLARE_LIST_HEAD_PTR_INIT(sptr_list, SCHED_SLEEP_LIST(cpuid));
    kbool_t existed = mr_list_empty(sptr_list);

    return existed ? mr_nullptr : mr_list_first_entry(sptr_list, struct thread, sgtc_link);
}

/*!
 * @brief	get the highest zombie thread (if zombie list is not empty)
 * @param  	none
 * @retval 	first thread
 * @note   	none
 */
struct thread *get_first_zombie_thread(kuint32_t cpuid)
{
    DECLARE_LIST_HEAD_PTR_INIT(sptr_list, SCHED_ZOMBIE_LIST(cpuid));
    kbool_t existed = mr_list_empty(sptr_list);

    return existed ? mr_nullptr : mr_list_first_entry(sptr_list, struct thread, sgtc_link);
}

/*!
 * @brief	get the lowest ready thread (if ready list is not empty)
 * @param  	none
 * @retval 	last thread
 * @note   	none
 */
struct thread *get_last_ready_thread(kuint32_t cpuid)
{
    DECLARE_LIST_HEAD_PTR_INIT(sptr_list, SCHED_READY_LIST(cpuid));
    kbool_t existed = mr_list_empty(sptr_list);

    return existed ? mr_nullptr : mr_list_last_entry(sptr_list, struct thread, sgtc_link);
}

/*!
 * @brief	get the lowest suspend thread (if suspend list is not empty)
 * @param  	none
 * @retval 	last thread
 * @note   	none
 */
struct thread *get_last_suspend_thread(kuint32_t cpuid)
{
    DECLARE_LIST_HEAD_PTR_INIT(sptr_list, SCHED_SUSPEND_LIST(cpuid));
    kbool_t existed = mr_list_empty(sptr_list);

    return existed ? mr_nullptr : mr_list_last_entry(sptr_list, struct thread, sgtc_link);
}

/*!
 * @brief	get the lowest sleep thread (if sleep list is not empty)
 * @param  	none
 * @retval 	last thread
 * @note   	none
 */
struct thread *get_last_sleep_thread(kuint32_t cpuid)
{
    DECLARE_LIST_HEAD_PTR_INIT(sptr_list, SCHED_SLEEP_LIST(cpuid));
    kbool_t existed = mr_list_empty(sptr_list);

    return existed ? mr_nullptr : mr_list_last_entry(sptr_list, struct thread, sgtc_link);
}

/*!
 * @brief	get the lowest zombie thread (if zombie list is not empty)
 * @param  	none
 * @retval 	last thread
 * @note   	none
 */
struct thread *get_last_zombie_thread(kuint32_t cpuid)
{
    DECLARE_LIST_HEAD_PTR_INIT(sptr_list, SCHED_ZOMBIE_LIST(cpuid));
    kbool_t existed = mr_list_empty(sptr_list);

    return existed ? mr_nullptr : mr_list_last_entry(sptr_list, struct thread, sgtc_link);
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
    return ((sptr_thread->state != NR_THREAD_ZOMBIE) && (sptr_thread->to_state != NR_THREAD_ZOMBIE));
}

/*!
 * @brief	get next ready thread
 * @param  	sptr_prev
 * @retval 	next
 * @note   	none
 */
struct thread *next_ready_thread(kuint32_t cpuid, struct thread *sptr_prev)
{
    DECLARE_LIST_HEAD_PTR_INIT(sptr_list, SCHED_READY_LIST(cpuid));

    if (!sptr_prev)
        return get_first_ready_thread(cpuid);

    if (mr_list_empty(sptr_list) ||
        mr_list_empty(&sptr_prev->sgtc_link) ||
        mr_list_head_until(sptr_prev, sptr_list, sgtc_link))
        return mr_nullptr;

    return mr_list_next_entry(sptr_prev, sgtc_link);
}

/*!
 * @brief	get next suspend thread
 * @param  	sptr_prev
 * @retval 	next
 * @note   	none
 */
struct thread *next_suspend_thread(kuint32_t cpuid, struct thread *sptr_prev)
{
    DECLARE_LIST_HEAD_PTR_INIT(sptr_list, SCHED_SUSPEND_LIST(cpuid));

    if (!sptr_prev)
        return get_first_suspend_thread(cpuid);

    if (mr_list_empty(sptr_list) ||
        mr_list_empty(&sptr_prev->sgtc_link) ||
        mr_list_head_until(sptr_prev, sptr_list, sgtc_link))
        return mr_nullptr;

    return mr_list_next_entry(sptr_prev, sgtc_link);
}

/*!
 * @brief	get next sleep thread
 * @param  	sptr_prev
 * @retval 	next
 * @note   	none
 */
struct thread *next_sleep_thread(kuint32_t cpuid, struct thread *sptr_prev)
{
    DECLARE_LIST_HEAD_PTR_INIT(sptr_list, SCHED_SLEEP_LIST(cpuid));

    if (!sptr_prev)
        return get_first_sleep_thread(cpuid);

    if (mr_list_empty(sptr_list) ||
        mr_list_empty(&sptr_prev->sgtc_link) ||
        mr_list_head_until(sptr_prev, sptr_list, sgtc_link))
        return mr_nullptr;

    return mr_list_next_entry(sptr_prev, sgtc_link);
}

/*!
 * @brief	get next zombie thread
 * @param  	sptr_prev
 * @retval 	next
 * @note   	none
 */
struct thread *next_zombie_thread(kuint32_t cpuid, struct thread *sptr_prev)
{
    DECLARE_LIST_HEAD_PTR_INIT(sptr_list, SCHED_ZOMBIE_LIST(cpuid));

    if (!sptr_prev)
        return get_first_zombie_thread(cpuid);

    if (mr_list_empty(sptr_list) ||
        mr_list_empty(&sptr_prev->sgtc_link) ||
        mr_list_head_until(sptr_prev, sptr_list, sgtc_link))
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
 * @param  	sptr_thread: target thread
 * @retval 	err code
 * @note   	only the running thread need to save context; and only the ready thread maybe need to restore context
 */ 
kint32_t schedule_thread_switch(struct thread *sptr_thread)
{
//  struct thread *sptr_thread;
//  const struct scheduler_operation *sptr_oprts = &sgtc_scheduler_operations[0];
    kint32_t cpuid = sptr_thread->cpu;
    tid_t tid;
    kuint32_t src, dst;
    kint32_t retval = ER_NORMAL;
    
    /*!< Protected by caller, do not disable again */
//  mr_preempt_disable();

//  sptr_thread = SCHED_THREAD_HANDLER(tid);
    if (mr_unlikely(!sptr_thread) || (cpuid < 0))
        return -ER_NODEV;

    tid = sptr_thread->tid;
    src = sptr_thread->state;
    dst = sptr_thread->to_state;

    /*!<
     * thread switch:
     * (At all times, it is necessary to ensure that at least one thread (including idle threads) is running)
     * running ---> ready/suspend/sleep/zombie
     * ready ---> running/suspend/sleep/zombie
     * suspend ---> ready/sleep/zombie
     * sleep ---> ready/suspend/zombie
     * zombie ---> prohit!!! it will be killed by kthread
     *
     * (only running and ready state can be switched to any state)
     */
    if (mr_unlikely((dst == NR_THREAD_NONE) || (dst >= NR_THREAD_STATUS_MAX)))
        goto fail;

    /*!< for idle thread, only ready and running state can be chosen */
    if ((tid < THREAD_TID_BASE) && 
        mr_unlikely((dst != NR_THREAD_RUNNING) && (dst != NR_THREAD_READY)))
        goto fail;

    /*!< do not suspend self in interrupt, and do not allow to switch context of other CPUs */
    if (mr_unlikely(dst == NR_THREAD_RUNNING) && 
        mr_unlikely(__IN_INTERRUPT(__IRQ_COUNT(sptr_thread)) || (cpuid != get_cpu_id())))
        goto fail;

    /*!< detached from current list */
    switch (src)
    {
        case NR_THREAD_RUNNING:
            /*!< no need to schedule */
            if (mr_unlikely(dst == NR_THREAD_RUNNING))
                goto fail;
            
            /*!< do not allow to switch context of other CPUs */
            if (mr_unlikely(cpuid != get_cpu_id()))
                goto fail;

            retval = schedule_reinstall_work_role(cpuid, sptr_thread);          
            break;

        case NR_THREAD_READY:
            retval = schedule_detach_ready_list(cpuid, sptr_thread);
            break;

        case NR_THREAD_SUSPEND:
            retval = schedule_detach_suspend_list(cpuid, sptr_thread);
            break;

        case NR_THREAD_SLEEP:
            retval = schedule_detach_sleep_list(cpuid, sptr_thread);
            break;

        case NR_THREAD_ZOMBIE:
            /*!< zombie thread can not be scheduled */
            goto fail;
            
        default:
            break;
    }

    mr_barrier();
    if (mr_unlikely(retval))
    {
        print_warn("switch thread (detach old) failed ! current (cpuid: %u) and target state is : %s, %d, %d\r\n", 
                    cpuid, sptr_thread->name, src, dst);
        goto fail;
    }

    /*!< add to new list */
    switch (dst)
    {
        case NR_THREAD_RUNNING:
            retval = schedule_despoil_work_role(cpuid, sptr_thread);
            break;

        case NR_THREAD_READY:
            /*!< clear state */
            __SET_THREAD_STATE(sptr_thread, NR_THREAD_NONE);
            retval = schedule_add_ready_list(cpuid, sptr_thread);
            break;

        case NR_THREAD_SUSPEND:
            /*!< clear state */
            __SET_THREAD_STATE(sptr_thread, NR_THREAD_NONE);
            retval = schedule_add_suspend_list(cpuid, sptr_thread);
            break;

        case NR_THREAD_SLEEP:
            /*!< clear state */
            __SET_THREAD_STATE(sptr_thread, NR_THREAD_NONE);
            retval = schedule_add_sleep_list(cpuid, sptr_thread);
            break;

        case NR_THREAD_ZOMBIE:
            /*!< clear state */
            __SET_THREAD_STATE(sptr_thread, NR_THREAD_NONE);
            retval = schedule_add_zombie_list(cpuid, sptr_thread);
            break;

        default:
            retval = -ER_ERROR;
            break;
    }

    if (mr_unlikely(retval))
    {
        print_warn("switch thread (add new) failed ! current (cpuid: %u) and target state is : %s, %d, %d, error code: %d\r\n", 
                    cpuid, sptr_thread->name, src, dst, retval);

        src = NR_THREAD_SLEEP;
        retval = schedule_add_sleep_list(cpuid, sptr_thread);
        if (retval < 0)
        {
            src = NR_THREAD_NONE;
            print_err("current thread is down and will be about to become kernel garbage! error code: %d\r\n", retval);
        }
        goto fail;
    }

    if (mr_unlikely(!SCHED_RUNNING_THREAD(cpuid)))
    {
        print_err("no thread is running !!! dangerous action !!!\r\n");
        mr_assert(true);
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
 * @brief	change thread cpu
 * @param  	sptr_thread
 * @retval 	errno
 * @note   	none
 */
kint32_t switch_thread_cpu(struct thread *sptr_thread, kint32_t target_cpu)
{
    struct scheduler_core *sptr_core;
    struct scheduler_core *sptr_core2;
    struct scheduler_operation *sptr_oprts;
    kuint32_t cpuid, state, affinity;
    kutype_t flags;
    kint32_t retval;

loop:
    cpuid = sptr_thread->cpu;
    sptr_core = SCHED_MANAGER_CORE(cpuid);
    spin_lock_irqsave(&sptr_core->sgtc_lock, &flags);

    /*!< if cpu has more than 2 cores, sptr_thread may migrate between other cores that are not the current core */
    if (mr_unlikely(cpuid != sptr_thread->cpu))
    {
        spin_unlock_irqrestore(&sptr_core->sgtc_lock, flags);
        goto loop;
    }

    if ((sptr_thread->cpu < 0) || 
        (target_cpu < 0) || 
        (sptr_thread->cpu == target_cpu))
    {
        spin_unlock_irqrestore(&sptr_core->sgtc_lock, flags);
        return -ER_CHECKERR;
    }

    /*!< target cpu is not normal */
    if (!get_cpu_current_thread(target_cpu))
    {
        spin_unlock_irqrestore(&sptr_core->sgtc_lock, flags);
        return -ER_NODEV;
    }

    state = __GET_THREAD_STATE(sptr_thread);
    if (state == NR_THREAD_RUNNING)
    {
        spin_unlock_irqrestore(&sptr_core->sgtc_lock, flags);
        print_warn("thread \'%s\' is running, not allow change cpu\r\n", sptr_thread->name);
        return -ER_FORBID;
    }

    affinity = thread_get_cpuaffinity(sptr_thread->sptr_attr);
    if (!(affinity & CPU_AFFINITY_SINGEL(target_cpu)))
    {
        spin_unlock_irqrestore(&sptr_core->sgtc_lock, flags);
        print_warn("thread \'%s\' is not allowed change to cpu %u, affinity forbid\r\n", sptr_thread->name, target_cpu);
        return -ER_FORBID;
    }

    sptr_oprts = (struct scheduler_operation *)(&sgtc_scheduler_operations[state]);
    if (mr_unlikely(!sptr_oprts->detach || !sptr_oprts->add_new))
    {
        spin_unlock_irqrestore(&sptr_core->sgtc_lock, flags);
        return -ER_FAULT;
    }

    retval = sptr_oprts->detach(sptr_thread->cpu, sptr_thread);
    if (retval || !mr_list_empty(&sptr_thread->sgtc_link))
    {
        spin_unlock_irqrestore(&sptr_core->sgtc_lock, flags);
        return -ER_FAILD;
    }

    sptr_thread->last_cpu = sptr_thread->cpu;
    sptr_thread->cpu = -1;
    mr_smp_mb();
    spin_unlock_irqrestore(&sptr_core->sgtc_lock, flags);

    /*!< add to new cpu */
    sptr_core2 = SCHED_MANAGER_CORE(target_cpu);
    spin_lock_irqsave(&sptr_core2->sgtc_lock, &flags);

    retval = sptr_oprts->add_new(target_cpu, sptr_thread);
    if (retval || mr_list_empty(&sptr_thread->sgtc_link))
    {
        spin_unlock_irqrestore(&sptr_core2->sgtc_lock, flags);
        mr_warn(false);
        return -ER_FAILD;
    }

    /*!< sptr_thread is free (not in queue), no cpu will schedule it; therefore, it is safe */
    sptr_thread->cpu = target_cpu;
    spin_unlock_irqrestore(&sptr_core2->sgtc_lock, flags);

    return ER_NORMAL;
}

/*!
 * @brief	change thread state to running
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	only ready state can be switched to running!!!
 */
static kint32_t schedule_despoil_work_role(kuint32_t cpuid, struct thread *sptr_thread)
{
    struct thread *sptr_running;
    struct scheduler_core *sptr_core = SCHED_MANAGER_CORE(cpuid);
    
    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< only ready state can be switched to running */
    if (mr_unlikely(NR_THREAD_READY != sptr_thread->state))
        return -ER_INVALID;

    /*!< get current */
    sptr_running = __THREAD_RUNNING(sptr_core);
    if (mr_likely(sptr_running))
    {
        kint32_t retval;

        /*!< current thread add to ready list */
        retval = schedule_add_ready_list(cpuid, sptr_running);
        if (mr_unlikely(retval < 0))
            return retval;

        __SYNC_THREAD_STATE(sptr_thread, NR_THREAD_READY);
    }

    /*!< update current */
    __THREAD_RUNNING(sptr_core) = sptr_thread;

    return ER_NORMAL;
}

/*!
 * @brief	change current thread state
 * @param  	none
 * @retval 	err code
 * @note   	running ---> xxx
 */
static kint32_t schedule_reinstall_work_role(kuint32_t cpuid, struct thread *sptr_thread)
{
    struct thread *sptr_new;
    struct scheduler_core *sptr_core = SCHED_MANAGER_CORE(cpuid);
    struct list_head *sptr_ready = __THREAD_READY_LIST(sptr_core);

    /*!< no thread ready; current should be set to idle thread */
//  if (mr_list_empty(sptr_ready))
//      return -ER_FAULT;

    /*!< get the first ready thread */
    sptr_new = mr_list_first_valid_entry(sptr_ready, struct thread, sgtc_link);
    if (mr_likely(sptr_new))
    {
        kint32_t retval;

        /*!< detached from ready list */
        retval = schedule_detach_ready_list(cpuid, sptr_new);
        if (mr_unlikely(retval < 0))
            return retval;

        /*!< ready thread ---> running */
        __THREAD_RUNNING(sptr_core) = sptr_new;
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
static kint32_t schedule_add_ready_list(kuint32_t cpuid, struct thread *sptr_thread)
{
    struct scheduler_core *sptr_core = SCHED_MANAGER_CORE(cpuid);
    kint32_t retval;

    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< avoid duplicate additions */
    if (mr_unlikely((NR_THREAD_READY == sptr_thread->state) &&
        !mr_list_empty(&sptr_thread->sgtc_link)))
        return -ER_INVALID;

    retval = __schedule_add_status_list(sptr_thread, 
                        __THREAD_READY_LIST(sptr_core), __THREAD_READY_HASH(sptr_core));
    if (!retval)
        sptr_core->ready_num++;

    return retval;
}

/*!
 * @brief	change target thread state from ready
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	del from ready list. it must be called by schedule_thread_switch, do not use alone !!!
 * 			(after detaching from the ready list, the thread will appear in a free state, 
 * 			so this function prohibits external calls to prevent the thread from leaving management and causing memory leakage)
 */
static kint32_t schedule_detach_ready_list(kuint32_t cpuid, struct thread *sptr_thread)
{
    struct scheduler_core *sptr_core = SCHED_MANAGER_CORE(cpuid);

    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< check if is in ready state */
    if (mr_unlikely(NR_THREAD_READY != sptr_thread->state))
        return -ER_INVALID;

    /*!< delete it */
    __schedule_del_status_list(sptr_thread, __THREAD_READY_LIST(sptr_core), __THREAD_READY_HASH(sptr_core));
    sptr_core->ready_num--;

    return ER_NORMAL;
}

/*!
 * @brief	change target thread state to suspend
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	add to suspend list
 */
static kint32_t schedule_add_suspend_list(kuint32_t cpuid, struct thread *sptr_thread)
{
    struct scheduler_core *sptr_core = SCHED_MANAGER_CORE(cpuid);
    kint32_t retval;

    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< avoid duplicate additions */
    if (mr_unlikely((NR_THREAD_SUSPEND == sptr_thread->state) &&
        !mr_list_empty(&sptr_thread->sgtc_link)))
        return -ER_INVALID;

    retval = __schedule_add_status_list(sptr_thread, 
                        __THREAD_SUSPEND_LIST(sptr_core), __THREAD_SUSPEND_HASH(sptr_core));
    if (!retval)
        sptr_core->suspend_num++;

    return retval;
}

/*!
 * @brief	change target thread state from suspend
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	del from suspend list. it must be called by schedule_thread_switch, do not use alone !!!
 * 			(after detaching from the suspend list, the thread will appear in a free state, 
 * 			so this function prohibits external calls to prevent the thread from leaving management and causing memory leakage)
 */
static kint32_t schedule_detach_suspend_list(kuint32_t cpuid, struct thread *sptr_thread)
{
    struct scheduler_core *sptr_core = SCHED_MANAGER_CORE(cpuid);

    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< check if is in ready state */
    if (mr_unlikely(NR_THREAD_SUSPEND != sptr_thread->state))
        return -ER_INVALID;

    /*!< delete it */
    __schedule_del_status_list(sptr_thread, __THREAD_SUSPEND_LIST(sptr_core), __THREAD_SUSPEND_HASH(sptr_core));
    sptr_core->suspend_num--;

    return ER_NORMAL;
}

/*!
 * @brief	change target thread state to sleep
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	add to sleep list
 */
static kint32_t schedule_add_sleep_list(kuint32_t cpuid, struct thread *sptr_thread)
{
    struct scheduler_core *sptr_core = SCHED_MANAGER_CORE(cpuid);
    kint32_t retval;

    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< avoid duplicate additions */
    if (mr_unlikely((NR_THREAD_SLEEP == sptr_thread->state) &&
        !mr_list_empty(&sptr_thread->sgtc_link)))
        return -ER_INVALID;

    retval = __schedule_add_status_list(sptr_thread, 
                        __THREAD_SLEEP_LIST(sptr_core), __THREAD_SLEEP_HASH(sptr_core));
    if (!retval)
        sptr_core->sleep_num++;

    return retval;
}

/*!
 * @brief	change target thread state from sleep
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	del from sleep list. it must be called by schedule_thread_switch, do not use alone !!!
 * 			(after detaching from the sleep list, the thread will appear in a free state, 
 * 			so this function prohibits external calls to prevent the thread from leaving management and causing memory leakage)
 */
static kint32_t schedule_detach_sleep_list(kuint32_t cpuid, struct thread *sptr_thread)
{
    struct scheduler_core *sptr_core = SCHED_MANAGER_CORE(cpuid);

    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< check if is in ready state */
    if (mr_unlikely(NR_THREAD_SLEEP != sptr_thread->state))
        return -ER_INVALID;

    /*!< delete it */
    __schedule_del_status_list(sptr_thread, __THREAD_SLEEP_LIST(sptr_core), __THREAD_SLEEP_HASH(sptr_core));
    sptr_core->sleep_num--;

    return ER_NORMAL;
}

/*!
 * @brief	change target thread state to zombie
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	add to zombie list
 */
static kint32_t schedule_add_zombie_list(kuint32_t cpuid, struct thread *sptr_thread)
{
    struct scheduler_core *sptr_core = SCHED_MANAGER_CORE(cpuid);

    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< avoid duplicate additions */
    if (mr_unlikely((NR_THREAD_ZOMBIE == sptr_thread->state) &&
        !mr_list_empty(&sptr_thread->sgtc_link)))
        return -ER_INVALID;

    return __schedule_add_status_list(sptr_thread, 
                    __THREAD_ZOMBIE_LIST(sptr_core), __THREAD_ZOMBIE_HASH(sptr_core));
}

/*!
 * @brief	change target thread state from zombie
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	del from zombie list. it must be called by schedule_thread_switch, do not use alone !!!
 * 			(after detaching from the zombie list, the thread will appear in a free state, 
 * 			so this function prohibits external calls to prevent the thread from leaving management and causing memory leakage)
 */
static kint32_t schedule_detach_zombie_list(kuint32_t cpuid, struct thread *sptr_thread)
{
    struct scheduler_core *sptr_core = SCHED_MANAGER_CORE(cpuid);

    if (mr_unlikely(!sptr_thread))
        return -ER_FAULT;

    /*!< check if is in ready state */
    if (mr_unlikely(NR_THREAD_ZOMBIE != sptr_thread->state))
        return -ER_INVALID;

    /*!< delete it */
    __schedule_del_status_list(sptr_thread, __THREAD_ZOMBIE_LIST(sptr_core), __THREAD_ZOMBIE_HASH(sptr_core));
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
 * @brief	initial thread
 * @param  	sptr_thread: target thread
 * @retval 	none
 * @note   	none
 */
void __setup_thread(struct thread *sptr_thread)
{
    struct lock_owners *sptr_owners;
    struct lock_waiter *sptr_waiter;

    sptr_owners = &sptr_thread->sgtc_owners;
    sptr_waiter = &sptr_thread->sgtc_wait;

    /*!< initial link */
    init_list_head(&sptr_thread->sgtc_link);
    init_list_head(&sptr_thread->sgtc_hash);

    /*!< initial spinlock */
    spin_lock_init(&sptr_thread->sgtc_lock);

    /*!< initial preempt count */
    ATOMIC_SET(&sptr_thread->sgtc_preempt, 0);

    /*!< initial lock struct */
    init_list_head(&sptr_owners->sgtc_gets);
    spin_lock_init(&sptr_owners->sgtc_lock);
    sptr_waiter->sptr_wait = mr_nullptr;
    init_list_head(&sptr_waiter->sgtc_link);
}

/*!
 * @brief	register a new thread, which will be added to ready list
 * @param  	sptr_thread: target thread
 * @param	tid: global tcb index
 * @retval 	err code
 * @note   	all new threads should be added to ready list at first
 */
kint32_t register_new_thread(struct thread *sptr_thread, tid_t *ptr_tid)
{
    kuint32_t cpuid = get_cpu_id();
    struct scheduler_core *sptr_core;
    struct thread_attr *sptr_it_attr;
    tid_t tid = *ptr_tid;
    kutype_t flags;
    kint32_t retval;

    sptr_it_attr = sptr_thread->sptr_attr;

    /*!< stack must be valid */
    if (!sptr_it_attr->stack_addr)
        return -ER_NOMEM;

    /*!< prohibit schedule */
    if (!sptr_it_attr->cpu_affinity)
        sptr_thread->cpu = -1;
    /*!< current cpu first */
    else if (sptr_it_attr->cpu_affinity & mr_bit(cpuid))
        sptr_thread->cpu = cpuid;
    /*!< select the first cpu */
    else
        sptr_thread->cpu = mr_ffs(sptr_it_attr->cpu_affinity) - 1;

    /*!< check again */
    if ((sptr_thread->cpu >= CONFIG_CORE_NUM) || (sptr_thread->cpu < 0))
        return -ER_INVALID;

    sptr_thread->last_cpu = sptr_thread->cpu;
    sptr_core = SCHED_MANAGER_CORE(sptr_thread->cpu);

    /*!< initial thread */
    __setup_thread(sptr_thread);

    /*!< set name */
    sprintk(sptr_thread->name, "thread-%d", tid);

    /*!< saved to tcb */
    spin_lock_irqsave(&__SCHED_LOCK, &flags);
    if (SCHED_THREAD_HANDLER(tid))
    {
        spin_unlock_irqrestore(&__SCHED_LOCK, flags);
        return -ER_INVALID;
    }
    SCHED_THREAD_HANDLER(tid) = sptr_thread;
    spin_unlock_irqrestore(&__SCHED_LOCK, flags);

    /*!< add and sorted by priority */
    spin_lock_irqsave(&sptr_core->sgtc_lock, &flags);
    retval = schedule_add_ready_list(sptr_thread->cpu, sptr_thread);
    if (retval < 0)
    {
        SCHED_THREAD_HANDLER(tid) = mr_nullptr;
        spin_unlock_irqrestore(&sptr_core->sgtc_lock, flags);
        return retval;
    }

    /*!< set to ready state */
    __SYNC_THREAD_STATE(sptr_thread, NR_THREAD_READY);
    spin_unlock_irqrestore(&sptr_core->sgtc_lock, flags);

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
    struct scheduler_core *sptr_core;
    kutype_t flags;

    spin_lock_irqsave(&__SCHED_LOCK, &flags);
    if ((tid < 0) || (tid == mr_current->tid))
    {
        sptr_thread = ERR_PTR(-ER_LOCKED);
        goto fail;
    }

    sptr_thread = SCHED_THREAD_HANDLER(tid);
    if (!sptr_thread)
        goto fail;

    if (sptr_thread->state != NR_THREAD_ZOMBIE)
    {
        sptr_thread = ERR_PTR(-ER_BUSY);
        goto fail;
    }

    SCHED_THREAD_HANDLER(tid) = mr_nullptr;
    spin_unlock_irqrestore(&__SCHED_LOCK, flags);

    sptr_core = SCHED_MANAGER_CORE(sptr_thread->cpu);
    spin_lock_irqsave(&sptr_core->sgtc_lock, &flags);
    schedule_detach_zombie_list(sptr_thread->cpu, sptr_thread);
    spin_unlock_irqrestore(&sptr_core->sgtc_lock, flags);

    goto end;

fail:
    spin_unlock_irqrestore(&__SCHED_LOCK, flags);
end:
    return sptr_thread;
}

/*!
 * @brief	check if scheduler is overload
 * @param  	none
 * @retval 	none
 * @note   	ajust scheduler, called by migration thread
 */
kint32_t check_scheduler_load(void)
{
    kuint32_t cpuid = get_cpu_id();
    struct scheduler_core *sptr_core = SCHED_MANAGER_CORE(0);
    kuint32_t total_load = 0, max_load = 0, max_load_cpu = 0;

    foreach_percpu(kuint32_t, i)
    {
        total_load += sptr_core->ready_num;

        if (sptr_core->ready_num > max_load)
        {
            max_load_cpu = i;
            max_load = sptr_core->ready_num;
        }

        sptr_core++;
    }

    sptr_core = SCHED_MANAGER_CORE(cpuid);
    if ((cpuid == max_load_cpu) || ((sptr_core->ready_num * CONFIG_CORE_NUM) > total_load))
        return -ER_FORBID;

    return max_load_cpu;
}

/*!
 * @brief	check if scheduler is overload
 * @param  	none
 * @retval 	none
 * @note   	ajust scheduler, called by migration thread
 */
void check_and_balance_scheduler(void)
{
    kuint32_t cpuid = get_cpu_id();
    struct scheduler_core *sptr_core;
    struct scheduler_core *sptr_core2;
    DECLARE_LIST_HEAD_PTR(sptr_list);
    struct thread *sptr_thread;
    kint32_t max_load_cpu = 0;
    kutype_t flags;
    kint32_t fail_count = 0, retval;

    /*!< check all the time, until balance */
    while (fail_count < 2)
    {
        kbool_t found = false;

        max_load_cpu = check_scheduler_load();
        if (max_load_cpu < 0)
            return;

        sptr_core = SCHED_MANAGER_CORE(cpuid);
        sptr_core2 = SCHED_MANAGER_CORE(max_load_cpu);
        sptr_list = __THREAD_READY_LIST(sptr_core2);

        spin_lock_irqsave_assert(&sptr_core2->sgtc_lock, &flags, __FILE__, __LINE__, __FUNCTION__);
        foreach_list_prev_entry(sptr_thread, sptr_list, sgtc_link)
        {
            /*!< support moving to current cpu ? */
            if (thread_get_cpuaffinity(sptr_thread->sptr_attr) & CPU_AFFINITY_SINGEL(cpuid))
            {
                found = true;
                break;
            }
        }

        /*!< no thread can be moved */
        if (!found)
        {
            spin_unlock_irqrestore_assert(&sptr_core2->sgtc_lock, flags);
            return;
        }

        retval = schedule_detach_ready_list(max_load_cpu, sptr_thread);
        if (retval || !mr_list_empty(&sptr_thread->sgtc_link))
        {
            spin_unlock_irqrestore_assert(&sptr_core2->sgtc_lock, flags);
            fail_count++;
            continue;
        }

        sptr_thread->last_cpu = sptr_thread->cpu;
        sptr_thread->cpu = -1;
        mr_smp_mb();
        spin_unlock_irqrestore_assert(&sptr_core2->sgtc_lock, flags);

        spin_lock_irqsave_assert(&sptr_core->sgtc_lock, &flags, __FILE__, __LINE__, __FUNCTION__);
        retval = schedule_add_ready_list(cpuid, sptr_thread);
        if (retval || mr_list_empty(&sptr_thread->sgtc_link))
        {
            spin_unlock_irqrestore_assert(&sptr_core->sgtc_lock, flags);
            mr_warn(false);

            return;
        }

        /*!< sptr_thread is free (not in queue), no cpu will schedule it; therefore, it is safe */
        sptr_thread->cpu = cpuid;
        spin_unlock_irqrestore_assert(&sptr_core->sgtc_lock, flags);

        fail_count = 0;
    }
}

/*!
 * @brief	init thread (ready to running)
 * @param  	none
 * @retval 	none
 * @note   	reload time slice
 */
void __thread_init_before(void)
{
    struct thread *sptr_thread = SCHED_RUNNING_THREAD(get_cpu_id());
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
    struct scheduler_context *sptr_context;
    kuint32_t cpuid = get_cpu_id();
    struct scheduler_core *sptr_core = SCHED_MANAGER_CORE(cpuid);
    kint32_t retval;

    sptr_hash = __THREAD_READY_HASH(sptr_core);
    sptr_context = &sgtc_sched_data[cpuid].sgtc_context;

    /*!< no ready thread here, unable to start or switch */
    if (__THREAD_HASH_EMPTY(sptr_hash))
        goto fail;

    /*!< get the current thread */
    sptr_prev = __THREAD_RUNNING(sptr_core);
    if (mr_unlikely(!sptr_prev))
    {
        if (mr_unlikely(sptr_context->first))
            goto fail;
        else
        {
            /*!< scheduled by "start_kernel" for the first time */
            sptr_prev = mr_list_first_entry(__THREAD_READY_LIST(sptr_core), struct thread, sgtc_link);           
            __SET_THREAD_TARGET_STATE(sptr_prev, NR_THREAD_RUNNING);
        }
    }
    
    /*!< if not set target state, default to ready */
    if (NR_THREAD_NONE == sptr_prev->to_state)
        __SET_THREAD_TARGET_STATE(sptr_prev, NR_THREAD_READY);

    /*!< select next valid thread */
    retval = schedule_thread_switch(sptr_prev);
    sptr_thread = __THREAD_RUNNING(sptr_core);
    if (mr_unlikely(retval < 0) || 
        mr_unlikely(!sptr_thread))
        goto fail;

    sptr_context->entry = (kuaddr_t)&sptr_thread->start_routine;
    sptr_context->args = (kuaddr_t)&sptr_thread->ptr_args;
    sptr_context->prev_sp = 0;
    sptr_context->next_sp = thread_get_stack(sptr_thread->sptr_attr);

    if (mr_likely(sptr_context->first))
        sptr_context->prev_sp = thread_get_stack(sptr_prev->sptr_attr);

    /*!< save current thread */
    SET_PERCPU_CURRENT(sptr_thread);

    /*!< record schedule count */
//  scheduler_record(cpuid);

    /*!< address of sgtc_context ===> r0 */
    return sptr_context;

fail:
    return mr_nullptr;
}

/*!
 * @brief	start schedule (ready to running)
 * @param  	none
 * @retval 	none
 * @note   	select a highest priority thread from ready list, and swicth it to running state
 */
void schedule_and_switch(void)
{
    /*!< pick next ready thread */
    struct scheduler_context *sptr_context = __schedule_thread();
    if (sptr_context)
        context_switch(sptr_context);
}

/*!
 * @brief	scheduled by current thread
 * @param  	none
 * @retval 	none
 * @note   	select a highest priority thread from ready list, and swicth it to running state
 */
void schedule_thread(void)
{
    kuint32_t cpuid;
    struct scheduler_core *sptr_core;
    struct thread *sptr_prev, *sptr_new;

    /*!
     * disable preemptetion, otherwise, preempt (schedule_thread_irq) will pause schedule_thread function; 
     * if current thread (with ready status) takes the opportunity to migrate to another cpu, 
     * context will be saved (such as cpuid, sptr_core, ...), but the new cpu does not know, 
     * it will use the old cpu's data (cpuid is old cpu) to continue to run, but it's error !!!
     */
    mr_preempt_disable();

    /*!< for first schedule, mr_current is a virtual thread, sptr_prev will be not NULL */
    /*!< virtual thread ---> schedule_thread() ---> real thread (virtual thread will be not back forever) */
    sptr_prev = mr_current;
    
    /*!< Not allow called by IRQ; because of cpuid (the data of current cpu), the judgement must in behind of preempt_disable */
    if (mr_unlikely(__IN_INTERRUPT(__IRQ_COUNT(sptr_prev))))
    {
        __SYNC_THREAD_STATE(sptr_prev, NR_THREAD_RUNNING);
        mr_preempt_enable();

        mr_warn(false);
        return;
    }

    cpuid = get_cpu_id();
    sptr_core = SCHED_MANAGER_CORE(cpuid);
    spin_lock_irqsave_assert(&sptr_core->sgtc_lock, &sptr_prev->lock_flags, __FILE__, __LINE__, __FUNCTION__);

    /*!< spin_lock will call mr_preempt_disable() again, we can release the first one, but preempt is still disabled */
    mr_preempt_enable();
    schedule_and_switch();
    mr_barrier();

    /*!< perhaps thread will be migrate, cpu is changed */
    sptr_core = SCHED_MANAGER_CORE(get_cpu_id());
    sptr_new = mr_current;
    spin_unlock_irqrestore_assert(&sptr_core->sgtc_lock, sptr_new->lock_flags);
}

/*!
 * @brief	start schedule (ready to running)
 * @param  	none
 * @retval 	none
 * @note   	select a highest priority thread from ready list, and swicth it to running state
 */
void schedule_thread_irq(void)
{
    struct scheduler_core *sptr_core;
    struct thread *sptr_prev, *sptr_new;

    sptr_core = SCHED_MANAGER_CORE(get_cpu_id());
    sptr_prev = mr_current;

    spin_lock_irqsave_assert(&sptr_core->sgtc_lock, &sptr_prev->lock_flags, __FILE__, __LINE__, __FUNCTION__);
    schedule_and_switch();
    mr_barrier();

    /*!< perhaps thread will be migrate, cpu is changed */
    sptr_core = SCHED_MANAGER_CORE(get_cpu_id());
    sptr_new = mr_current;
    spin_unlock_irqrestore_assert(&sptr_core->sgtc_lock, sptr_new->lock_flags);
}

/*!
 * @brief	first schedule preparation
 * @param  	_spsr: super register
 * @retval 	none
 * @note   	called by the tail of "__switch_to"
 */
kutype_t ret_with_first_schedule(kutype_t _spsr)
{
    kuint32_t cpuid = get_cpu_id();
    struct scheduler_core *sptr_core = SCHED_MANAGER_CORE(cpuid);

    /*!< enable interrupt, it will be set to cpsr by the tail of "__switch_to" */
    _spsr &= ~(CPSR_BIT_I | CPSR_BIT_F | CPSR_BIT_A);

    /*!< delay enable interrupt */
    spin_unlock_irqrestore_assert(&sptr_core->sgtc_lock, _spsr | CPSR_BIT_I | CPSR_BIT_F);
    return _spsr;
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
    struct scheduler_table *sptr_sch = SCHED_MANAGER();
    struct scheduler_core *sptr_core;
    struct scheduler_share *sptr_share;

    foreach_percpu(kuint32_t, id)
    {
        sptr_core = SPEC_CPU_READ(sptr_sch->sgtc_core, id);
        
        /*!< core */
        memset(sptr_core, 0, sizeof(*sptr_core));
        spin_lock_init(&sptr_core->sgtc_lock);

        init_list_head(&sptr_core->sgtc_lready);
        init_list_head(&sptr_core->sgtc_lsuspend);
        init_list_head(&sptr_core->sgtc_lsleep);
        init_list_head(&sptr_core->sgtc_lzombie);
        
        __scheduler_init(&sptr_core->sgtc_hready);
        __scheduler_init(&sptr_core->sgtc_hsuspend);
        __scheduler_init(&sptr_core->sgtc_hsleep);
        __scheduler_init(&sptr_core->sgtc_hzombie);
    }

    /*!< share */
    sptr_share = &sptr_sch->sgtc_share;
    spin_lock_init(&sptr_share->sgtc_lock);
}

/*!< end of file */
