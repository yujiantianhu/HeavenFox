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
#include <kernel/lock_common.h>
#include <kernel/spinlock.h>

/*!< The defines */
struct mailbox;

#define THREAD_NAME_SIZE                        (32)

struct thread
{
    /*!< thread name */
    kchar_t name[THREAD_NAME_SIZE];

    /*!< thread id */
    tid_t tid;

    /*!< refer to "__ERT_THREAD_BASIC_STATUS" */
    kuint32_t state;
    kuint32_t to_state;

    /*!< thread entry */
    void *(*start_routine) (void *);
    struct thread_attr *sptr_attr;
    void *ptr_args;

    /*!< thread list (to ready/suspend/sleep list) */
    struct list_head sgtc_link;

    /*!< hash list */
    struct list_head sgtc_hash;

    /*!< thread time slice (period = sptr_attr->sgtc_param.init_budget) */
    kutime_t expires;

    /*!< refer to "__ERT_THREAD_SIGNALS" */
    kuint32_t flags;

    /*!< last cpu that thread is running */
    kint32_t last_cpu;
    /*!< current cpu that thread is running */
    kint32_t cpu;
    /*!< scheudler status */
    kutype_t lock_flags;

    /*!< preempt count */
    struct atomic sgtc_preempt;
    /*!< irq count */
    kuint32_t irq_count;

    struct spin_lock sgtc_lock;
    struct mailbox *sptr_mb;

    /*!< Used to indicate which locks are held, as one thread may hold multiple locks */
    struct lock_owners sgtc_owners;     
    /*!
     * Indicating a lock that is being waited for; at any given time, only one lock will be waited for by threads 
     * (because when the thread cannot acquire a lock, it cannot request another lock) 
     */
    struct lock_waiter sgtc_wait;   

    void *time_event;
};

/*!< Set thread state */
#define __SET_THREAD_STATE(sptr_th, value)  \
    do {    \
        (sptr_th)->state = (value); \
    } while (0)

#define __SET_THREAD_TARGET_STATE(sptr_th, value)   \
    do {    \
        (sptr_th)->to_state = (value);  \
    } while (0)

#define __SYNC_THREAD_STATE(sptr_th, value) \
    do {    \
        (sptr_th)->state = (value); \
        (sptr_th)->to_state = NR_THREAD_NONE;   \
    } while (0)

/*!< Get thread state */
#define __GET_THREAD_STATE(sptr_th)                 ((sptr_th)->state)
#define __GET_THREAD_TARGET_STATE(sptr_th)          ((sptr_th)->to_state)

/*!< Signal flags */
#define mr_thread_set_flags(signal, sptr_tsk)   \
    do {    \
        (sptr_tsk)->flags |= mr_bit(signal);    \
    } while (0)

#define mr_thread_clr_flags(signal, sptr_tsk)   \
    do {    \
        (sptr_tsk)->flags &= ~mr_bit(signal);   \
    } while (0)

#define mr_thread_is_flags(signal, sptr_tsk)        (!!((sptr_tsk)->flags & mr_bit(signal)))

/*!< -------------------------------------------------------------------------- */
/*!< Scheduler */
struct thread_hash
{
    struct list_head sgtc_list;
    struct thread *sptr_tail;
};

struct thread_list
{
    kuint64_t ffs_l;
    kuint64_t ffs_h;

    struct thread_hash sgtc_hash[THREAD_PROTY_NUM];
    struct spin_lock sgtc_lock;

#define __THREAD_HASH_EMPTY(hash)                   (!(hash)->ffs_l && !(hash->ffs_h))           
};

/*!< Per-CPU private */
struct scheduler_core
{
    struct thread *sptr_work;                       /*!< current thread (status is running) */

    struct list_head sgtc_lready;                   /*!< ready list head (manage all ready thread) */
    struct list_head sgtc_lsuspend;                 /*!< suspend list head (manage all suspend thread) */
    struct list_head sgtc_lsleep;                   /*!< sleep list head (manage all sleepy thread) */
    struct list_head sgtc_lzombie;                  /*!< zombie list head (manage all zombie thread) */

    kuint32_t ready_num;                            /*!< length of ready lists */
    kuint32_t suspend_num;                          /*!< length of suspend lists */
    kuint32_t sleep_num;                            /*!< length of sleep lists */
    kuint32_t zombie_num;                           /*!< length of zombie lists */

    struct thread_list sgtc_hready;                 /*!< ready hash list */
    struct thread_list sgtc_hsuspend;               /*!< suspend hash list */
    struct thread_list sgtc_hsleep;                 /*!< sleep hash list */
    struct thread_list sgtc_hzombie;                /*!< zombie hash list */

    struct {
        kutype_t cnt_out;                           /*!< when sched_cnt is over (~0), cnt_out++ */
        kutype_t sched_cnt;                         /*!< schedule counter, max is ~0 */
    } sgtc_cnt;

    struct spin_lock sgtc_lock;
//  kutype_t lock_flags;
};

/*!< Common lists */
struct scheduler_share
{
    /*!< no members */

    struct spin_lock sgtc_lock;
};

/*!< thread manage table */
struct scheduler_table
{
    kint32_t max_tidarr;                            /*!< = THREAD_MAX_NUM */
    kint32_t max_tids;                              /*!< = THREAD_MAX_NUM + count of sptr_tids */
    kint32_t max_tidset;                            /*!< the max tid */
    kint32_t ref_tidarr;                            /*!< number of allocated descriptors in sptr_tid_array */

    struct thread **sptr_tids;                      /*!< if sptr_tid_array is up to max, new thread form mempool */
    struct thread *sptr_tid_array[THREAD_MAX_NUM];  /*!< thread maximum, tid = 0 ~ THREAD_MAX_NUM */
    
    struct scheduler_share sgtc_share;              /*!< thread lists */
    struct spin_lock sgtc_lock;

    /*!< per-cpu lists */
    struct scheduler_core sgtc_core[CONFIG_CORE_NUM];

#define __THREAD_MAX_STATS                          ((kutype_t)(~0))
#define __THREAD_HANDLER(ptr, tid)                  ((ptr)->sptr_tid_array[(tid)])

#define __THREAD_RUNNING(_sptr_core)                ((_sptr_core)->sptr_work)
#define __THREAD_READY_LIST(_sptr_core)             (&((_sptr_core)->sgtc_lready))
#define __THREAD_SUSPEND_LIST(_sptr_core)           (&((_sptr_core)->sgtc_lsuspend))
#define __THREAD_SLEEP_LIST(_sptr_core)             (&((_sptr_core)->sgtc_lsleep))
#define __THREAD_ZOMBIE_LIST(_sptr_core)            (&((_sptr_core)->sgtc_lzombie))

#define __THREAD_READY_HASH(_sptr_core)             (&((_sptr_core)->sgtc_hready))
#define __THREAD_SUSPEND_HASH(_sptr_core)           (&((_sptr_core)->sgtc_hsuspend))
#define __THREAD_SLEEP_HASH(_sptr_core)             (&((_sptr_core)->sgtc_hsleep))
#define __THREAD_ZOMBIE_HASH(_sptr_core)            (&((_sptr_core)->sgtc_hzombie))

#define __THREAD_CORE_LOCK(_sptr_core)              (&((_sptr_core)->sgtc_lock))
};

/*!< The globals */

/*!< The functions */
extern struct scheduler_core *get_scheduler_core(kuint32_t cpuid);
extern struct thread *get_current_thread(void);
extern struct thread *get_cpu_current_thread(kuint32_t cpuid);
extern struct list_head *get_ready_thread_table(void);
extern struct thread *get_thread_handle(tid_t tid);
extern void thread_set_name(tid_t tid, const kchar_t *name);
extern void thread_set_name_args(tid_t tid, const kchar_t *name, ...);
extern void thread_set_self_name(const kchar_t *name);
extern void thread_set_self_name_args(const kchar_t *name, ...);
extern kchar_t *thread_get_name(tid_t tid);
extern kchar_t *thread_get_self_name(void);
extern void thread_set_state(struct thread *sptr_thread, kuint32_t state);
extern struct spin_lock *scheduler_lock(void);
extern struct spin_lock *scheduler_cpu_lock(kuint32_t cpuid);
extern tid_t get_unused_tid_from_scheduler(kuint32_t i_start, kuint32_t count);
extern void schedule_self_suspend(void);
extern void schedule_self_sleep(void);
extern void schedule_self_zombie(void);
extern kint32_t schedule_thread_suspend(tid_t tid);
extern kint32_t schedule_thread_sleep(tid_t tid);
extern kint32_t schedule_thread_zombie(tid_t tid);
extern kint32_t schedule_thread_wakeup(tid_t tid);

extern kuint64_t scheduler_stats_get(kuint32_t cpuid);
extern kbool_t is_ready_thread_empty(kuint32_t cpuid);
extern kbool_t is_suspend_thread_empty(kuint32_t cpuid);
extern kbool_t is_sleep_thread_empty(kuint32_t cpuid);
extern struct thread *get_first_ready_thread(kuint32_t cpuid);
extern struct thread *get_first_suspend_thread(kuint32_t cpuid);
extern struct thread *get_first_sleep_thread(kuint32_t cpuid);
extern struct thread *get_first_zombie_thread(kuint32_t cpuid);
extern struct thread *get_last_ready_thread(kuint32_t cpuid);
extern struct thread *get_last_suspend_thread(kuint32_t cpuid);
extern struct thread *get_last_sleep_thread(kuint32_t cpuid);
extern struct thread *get_last_zombie_thread(kuint32_t cpuid);
extern kbool_t is_thread_valid(tid_t tid);
extern struct thread *next_ready_thread(kuint32_t cpuid, struct thread *sptr_prev);
extern struct thread *next_suspend_thread(kuint32_t cpuid, struct thread *sptr_prev);
extern struct thread *next_sleep_thread(kuint32_t cpuid, struct thread *sptr_prev);
extern struct thread *next_zombie_thread(kuint32_t cpuid, struct thread *sptr_prev);

extern kint32_t schedule_thread_switch(struct thread *sptr_thread);
extern kint32_t switch_thread_cpu(struct thread *sptr_thread, kint32_t target_cpu);
extern void __setup_thread(struct thread *sptr_thread);
extern kint32_t register_new_thread(struct thread *sptr_thread, tid_t *ptr_tid);
extern struct thread *unregister_thread(tid_t tid);
extern kint32_t check_scheduler_load(void);
extern void check_and_balance_scheduler(void);
extern void __thread_init_before(void);
extern struct scheduler_context *__schedule_thread(void);
extern void schedule_thread(void);
extern void scheduler_init(void);

/*!< API functions */
/*!
 * @brief   get current
 * @param   none
 * @retval  sptr_thread
 * @note    none
 */
static inline struct thread *current_thread(void)
{
#ifdef PERCPU_CURRENT
    return (struct thread *)(PERCPU_CURRENT() & (~THREAD_MASK));
#else
    return get_current_thread();
#endif
}

/*!
 * @brief   check if thread is valid
 * @param   none
 * @retval  status
 * @note    none
 */
static inline kbool_t current_valid(void)
{
#ifdef PERCPU_CURRENT
    kutype_t cur = PERCPU_CURRENT();
    return !!(cur & (~THREAD_MASK)) && (!(cur & THREAD_VIRTUAL_BIT));
#else
    return !!((kutype_t)get_current_thread());
#endif
}

/*!< The defines */
#define mr_current                              current_thread()
#define THREAD_VALID()                          (mr_current && (mr_current->tid >= 0))

#ifndef SET_PERCPU_CURRENT
#define SET_PERCPU_CURRENT(x)                   do { } while (0)
#endif

/*!< The defines */
#define mr_tid_handle(tid)                      get_thread_handle(tid)
#define mr_tid_attr(tid)                        thread_attr_get(tid)

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
    kutype_t flags;

    spin_lock_irqsave(&sptr_thread->sgtc_lock, &flags);
    is_wakeup = mr_thread_is_flags(NR_THREAD_SIG_WAKEUP, sptr_thread);
    is_killed = mr_thread_is_flags(NR_THREAD_SIG_KILL, sptr_thread);

    mr_barrier();

    mr_thread_clr_flags(NR_THREAD_SIG_WAKEUP, sptr_thread);
    mr_thread_clr_flags(NR_THREAD_SIG_KILL, sptr_thread);
    spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

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
    kutype_t flags;

    spin_lock_irqsave(&sptr_thread->sgtc_lock, &flags);

    if (mode)
        mr_thread_set_flags(state, sptr_thread);
    else
        mr_thread_clr_flags(state, sptr_thread);
    
    spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);
}

#ifdef __cplusplus
    }
#endif

#endif /* __KERNEL_SCHED_H_ */
