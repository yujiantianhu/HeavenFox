/*
 * Lock Common Interface
 *
 * File Name:   lock_common.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.12.14
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <kernel/kernel.h>
#include <kernel/lock_common.h>
#include <kernel/sched.h>
#include <term/term.h>

/*!< The defines */
#define INHERIT_RECURSION_DEPTH                         (10)

/*!< The globals */
static kint32_t g_inherit_monitor;
static kint32_t g_inherit_recur_monitor;

static struct term_variable sgtc_inherit_monitor[] = 
{ 
    { .name = "g_inherit_monitor", .var = &g_inherit_monitor, .num = 1 },
    { .name = "g_inherit_recur_monitor", .var = &g_inherit_recur_monitor, .num = 1 },
};
static const kusize_t g_num_inherit_monitor = ARRAY_SIZE(sgtc_inherit_monitor);

/*!< The functions */


/*!< API functions */
/*!
 * @brief   lock initial
 * @param   sptr_owner
 * @retval  none
 * @note    none
 */
void lock_context_init(struct lock_owner *sptr_owner)
{
    sptr_owner->sptr_self = mr_nullptr;

    spin_lock_init(&sptr_owner->sgtc_lock);
    init_list_head(&sptr_owner->sgtc_link);
    init_list_head(&sptr_owner->sgtc_pendings);
}

/*!
 * @brief   Add new lock
 * @param   sptr_owner
 * @retval  none
 * @note    Request lock success: Save thread information; called by mutex_lock function
 */
void lock_context_save(struct lock_owner *sptr_owner)
{
    struct thread *sptr_self;
    struct lock_waiter *sptr_waiter;
    struct lock_owners *sptr_owners;
    kutype_t flags;

    sptr_self = mr_current;

    /*!< It should be assigned before use */
//  if (mr_unlikely(!sptr_owner->sptr_self))
//      print_warn("%s %d: please set owner before using, current tid is: %d\r\n", __FUNCTION__, __LINE__, sptr_self->tid);

    sptr_waiter = &sptr_self->sgtc_wait;
    sptr_owners = &sptr_self->sgtc_owners;

    spin_lock_irqsave(&sptr_owners->sgtc_lock, &flags);

    /*!< This lock is exactly what it has been waiting for. No need to wait any longer */
    if (sptr_waiter->sptr_wait == sptr_owner)
    {
        sptr_waiter->sptr_wait = mr_nullptr;
        list_head_del(&sptr_waiter->sgtc_link);
    }
    
    /*!< 
     * This thread gets a new lock. 
     * The order is consistent with the sequence of lock acquisition, 
     * the lock acquired first is listed first
     */
    list_head_add_tail(&sptr_owners->sgtc_gets, &sptr_owner->sgtc_link);
    spin_unlock_irqrestore(&sptr_owners->sgtc_lock, flags);

    /*!< Mark the lock owner */
    sptr_owner->sptr_self = sptr_self;
}

/*!
 * @brief   Delete lock held
 * @param   sptr_owner
 * @retval  none
 * @note    Relese lock (will be called by mutex_unlock)
 */
void unlock_context_restore(struct lock_owner *sptr_owner)
{
    struct thread *sptr_self;
    struct lock_owners *sptr_owners;
    kutype_t flags;

    /*!< The lock is not held by any thread, so it does not need to be released */
    if (mr_unlikely(!sptr_owner->sptr_self))
        return;

    sptr_self = sptr_owner->sptr_self;
    sptr_owners = &sptr_self->sgtc_owners;

    /*!< Detached from this thread, the lock is no longer held by this thread */
    spin_lock_irqsave(&sptr_owners->sgtc_lock, &flags);
    list_head_del(&sptr_owner->sgtc_link);
    spin_unlock_irqrestore(&sptr_owners->sgtc_lock, flags);

    sptr_owner->sptr_self = mr_nullptr;
}

/*!
 * @brief   Get the first thread
 * @param   sptr_owner
 * @retval  none
 * @note    Obtain the thread of the first request (which is the requester with the highest priority)
 */
struct thread *first_request_thread(struct lock_owner *sptr_owner)
{
    struct lock_waiter *sptr_wait;
    
    if (mr_list_empty(&sptr_owner->sgtc_pendings))
        return mr_nullptr;

    sptr_wait = mr_list_first_entry(&sptr_owner->sgtc_pendings, struct lock_waiter, sgtc_link);
    return mr_container_of(sptr_wait, struct thread, sgtc_wait);
}

/*!
 * @brief   Add new thread (which requests the lock) to pending list
 * @param   sptr_owner
 * @param   sptr_thread
 * @retval  none
 * @note    Add the new thread to the pending linked list of this lock. 
 *          The higher the priority, the earlier it will be placed in the queue
 */
void lock_pending_add(struct lock_owner *sptr_owner, struct thread *sptr_thread)
{
    struct lock_waiter *sptr_waiter;
    struct list_head *sptr_head;

    sptr_head = &sptr_owner->sgtc_pendings;
    if (!mr_list_empty(sptr_head))
    {
        struct thread *sptr_pert;
        struct lock_waiter *sptr_pwait;
        kuint32_t self_prio, per_prio;

        /*!< The priority of "sptr_thread" */
        self_prio = thread_get_rt_priority(sptr_thread->sptr_attr);

        /*!< Search */
        foreach_list_next_entry(sptr_pwait, sptr_head, sgtc_link)
        {
            sptr_pert = mr_container_of(sptr_pwait, struct thread, sgtc_wait);
            per_prio  = thread_get_rt_priority(sptr_pert->sptr_attr);

            /*! @note
             * Add to the chain according to priority, with higher priority items placed earlier.
             * Threads with the same priority should be inserted at the end of the same priority's list 
             * (do not place them at the beginning, as this would be unfair to other threads with the same priority) 
             */
            if (__THREAD_IS_LOW_PRIO(per_prio, self_prio))
            {
                sptr_head = &sptr_pert->sgtc_wait.sgtc_link;
                break;
            }
        }
    }

    sptr_waiter = &sptr_thread->sgtc_wait;

    /*!< Add to the chain, and attach this thread to the pending chain list of this lock for the next round of retrieval */
    list_head_add_tail(sptr_head, &sptr_waiter->sgtc_link);
    /*!< A thread will only wait for one lock at any given time */
    sptr_waiter->sptr_wait = sptr_owner;
}

/*!
 * @brief   Detach a pending item
 * @param   sptr_owner
 * @param   sptr_thread
 * @retval  none
 * @note    none
 */
void lock_pending_del(struct thread *sptr_thread)
{
    struct lock_waiter *sptr_waiter = &sptr_thread->sgtc_wait;

    if (in_lock_pending(sptr_waiter))
        list_head_del(&sptr_waiter->sgtc_link);

    sptr_waiter->sptr_wait = mr_nullptr;
}

/*!
 * @brief   Wake up a thread
 * @param   sptr_pert
 * @param   sptr_owns
 * @param   sptr_pwait
 * @retval  none
 * @note    none
 */
static void __unlock_pending_wakeup(struct thread *sptr_pert, struct lock_owners *sptr_owns, struct lock_waiter *sptr_pwait)
{
    kutype_t flags;

    spin_lock_irqsave(&sptr_owns->sgtc_lock, &flags);
    sptr_pwait->sptr_wait = mr_nullptr;
    list_head_del(&sptr_pwait->sgtc_link);
    spin_unlock_irqrestore(&sptr_owns->sgtc_lock, flags);

    schedule_thread_wakeup(sptr_pert->tid);
}

/*!
 * @brief   Wake up all thread which are pending, and detach all pending list
 * @param   sptr_owner
 * @retval  none
 * @note    It should be used by "lock destroy"
 */
void unlock_pending_del_all(struct lock_owner *sptr_owner)
{
    struct list_head *sptr_head, sgtc_lists;

    init_list_head(&sgtc_lists);

    sptr_head = &sptr_owner->sgtc_pendings;
    while (!mr_list_empty(sptr_head))
    {
        struct lock_waiter *sptr_pwait, *sptr_temp;
        struct thread *sptr_pert;
        struct lock_owners *sptr_owns;

        spin_lock(&sptr_owner->sgtc_lock);
        list_head_splice_init(&sgtc_lists, sptr_head);
        spin_unlock(&sptr_owner->sgtc_lock);

        foreach_list_next_entry_safe(sptr_pwait, sptr_temp, &sgtc_lists, sgtc_link)
        {
            sptr_pert = mr_container_of(sptr_pwait, struct thread, sgtc_wait);
            sptr_owns = &sptr_pert->sgtc_owners;
            __unlock_pending_wakeup(sptr_pert, sptr_owns, sptr_pwait);
        }
    }
}

/*!
 * @brief   Wake up threads which are pending, and detach pending lists
 * @param   sptr_owner
 * @param   wake_all: true ? wake all threads; false ? wake the thread with highest priority
 * @retval  none
 * @note    It should be used by "lock destroy"
 */
void unlock_pending_wakeup(struct lock_owner *sptr_owner, kbool_t wake_all)
{
    struct list_head *sptr_head;

    sptr_head = &sptr_owner->sgtc_pendings;
    spin_lock(&sptr_owner->sgtc_lock);

    /*!
     * Check whether this lock is held by other threads. 
     * If it is, this thread has no right to operate someone else's lock 
     */
    if (sptr_owner->sptr_self && (sptr_owner->sptr_self != mr_current))
    {
        spin_unlock(&sptr_owner->sgtc_lock);
        return;
    }

    if (mr_list_empty(sptr_head))
        spin_unlock(&sptr_owner->sgtc_lock);
    else
    {
        struct lock_waiter *sptr_pwait, *sptr_temp;
        struct lock_owners *sptr_owns;
        struct thread *sptr_pert;
        struct list_head sgtc_lists;

        init_list_head(&sgtc_lists);

        if (wake_all)
        {
            /*!< Transfer the waiting list and clear the old list */
            list_head_splice_init(&sgtc_lists, sptr_head);
        }
        else
        {
            sptr_pwait = mr_list_first_entry(sptr_head, struct lock_waiter, sgtc_link);

            list_head_del(&sptr_pwait->sgtc_link);
            list_head_add_tail(&sgtc_lists, &sptr_pwait->sgtc_link);
        }

        spin_unlock(&sptr_owner->sgtc_lock);

        foreach_list_next_entry_safe(sptr_pwait, sptr_temp, &sgtc_lists, sgtc_link) 
        {
            sptr_pert = mr_container_of(sptr_pwait, struct thread, sgtc_wait);
            sptr_owns = &sptr_pert->sgtc_owners;
            __unlock_pending_wakeup(sptr_pert, sptr_owns, sptr_pwait);
        }
    }
}

/*!
 * @brief   Wake up threads which are pending, and detach pending lists
 * @param   sptr_owner
 * @param   wake_all: true ? wake all threads; false ? wake the thread with highest priority
 * @retval  none
 * @note    It should be used by IRQ_Handler
 */
void unlock_pending_wakeup_nolock(struct lock_owner *sptr_owner, kbool_t wake_all)
{
    struct list_head *sptr_head;

    sptr_head = &sptr_owner->sgtc_pendings;

    /*!
     * Check whether this lock is held by other threads. 
     * If it is, this thread has no right to operate someone else's lock 
     */
    if (sptr_owner->sptr_self && (sptr_owner->sptr_self != mr_current))
        return;

    if (!mr_list_empty(sptr_head))
    {
        struct lock_waiter *sptr_pwait, *sptr_temp;
        struct lock_owners *sptr_owns;
        struct thread *sptr_pert;
        struct list_head sgtc_lists;

        init_list_head(&sgtc_lists);

        if (wake_all)
        {
            /*!< Transfer the waiting list and clear the old list */
            list_head_splice_init(&sgtc_lists, sptr_head);
        }
        else
        {
            sptr_pwait = mr_list_first_entry(sptr_head, struct lock_waiter, sgtc_link);

            list_head_del(&sptr_pwait->sgtc_link);
            list_head_add_tail(&sgtc_lists, &sptr_pwait->sgtc_link);
        }

        foreach_list_next_entry_safe(sptr_pwait, sptr_temp, &sgtc_lists, sgtc_link) 
        {
            sptr_pert = mr_container_of(sptr_pwait, struct thread, sgtc_wait);
            sptr_owns = &sptr_pert->sgtc_owners;
            __unlock_pending_wakeup(sptr_pert, sptr_owns, sptr_pwait);
        }
    }
}

/*!
 * @brief   Compare the pending linked lists of all locks to obtain the highest priority
 * @param   sptr_owner
 * @retval  none
 * @note    none
 */
kuint32_t lock_find_max_priority(struct lock_owners *sptr_owners)
{
    struct lock_owner *sptr_pero;
    struct thread *sptr_request;
    struct list_head *sptr_head;
    kuint32_t its_prio, max_prio = THREAD_PROTY_MIN;

    sptr_head = &sptr_owners->sgtc_gets;
    if (mr_list_empty(sptr_head))
        return THREAD_PROTY_MIN;

    foreach_list_next_entry(sptr_pero, sptr_head, sgtc_link)
    {
        /*!< Extract the thread with the highest priority from the request linked list of each lock */
        sptr_request = first_request_thread(sptr_pero);
        its_prio = sptr_request ? thread_get_rt_priority(sptr_request->sptr_attr) : THREAD_PROTY_MIN;

        if (__THREAD_IS_LOW_PRIO(max_prio, its_prio))
            max_prio = its_prio;
    }

    return max_prio;
}

/*!
 * @brief   Request lock (race condition)
 * @param   sptr_owner
 * @param   sptc_atc: lock counter
 * @param   inherit_enable: true --> enable priority inherit; false --> disable
 * @retval  errno
 * @note    When the lock is already held by another thread, it will be called by mutex_lock
 */
kint32_t lock_compete(struct lock_owner *sptr_owner, kbool_t (*is_locked)(void *), void *lock, kbool_t inherit_enable)
{
    struct thread *sptr_self, *sptr_rival;
    struct lock_owners *sptr_owns;
    kutype_t flags;
    
    /*!< Current thread */
    sptr_self = mr_current;
    sptr_owns = &sptr_self->sgtc_owners;

    spin_lock(&sptr_owner->sgtc_lock);

    /*!< The thread which held the lock */
    sptr_rival = sptr_owner->sptr_self;

    /*!< It's me, there's no competitive relationship; or the lock is already idle. Re-entry is prohibited! */
    if ((sptr_self == sptr_rival) || (is_locked(lock) == 0))
    {
        spin_unlock(&sptr_owner->sgtc_lock);
        return -ER_FORBID;
    }

    /*!< Support inherit ? */
    if (inherit_enable && sptr_rival)
    {
        struct lock_owners *sptr_rlowns;
        kuint32_t cur_prio, ori_prio, max_prio;
        struct spin_lock *sptr_lock;

#if defined(CONFIG_INHERIT_RECURSION) && (CONFIG_INHERIT_RECURSION)
        kuint8_t depth = INHERIT_RECURSION_DEPTH;
#endif

        for (;;) 
        {
            spin_lock_irqsave(&sptr_owns->sgtc_lock, &flags);

            /*!
             * If the list (sptr_self) has already been added, remove it first; 
             * because the lock will be continuously requested by this thread, 
             * this function will be frequently entered to prevent duplicate additions 
             */
            lock_pending_del(sptr_self);

            /*!< Insert into the pending list */
            lock_pending_add(sptr_owner, sptr_self);

            spin_unlock_irqrestore(&sptr_owns->sgtc_lock, flags);
            spin_unlock(&sptr_owner->sgtc_lock);

            /*!< This thread is not the holder of the lock, but a contender */
            sptr_rlowns = &sptr_rival->sgtc_owners;

            /*!< Read-Only */
            /*!< Original priority */
            ori_prio = thread_get_ori_priority(sptr_rival->sptr_attr);
            /*!< Realtime priority */
            cur_prio = thread_get_rt_priority(sptr_rival->sptr_attr);

            /*! @note
             * Find the new maximum priority from the request linked list of all locks
             */
            spin_lock_irqsave(&sptr_rlowns->sgtc_lock, &flags);
            max_prio = lock_find_max_priority(sptr_rlowns);
            spin_unlock_irqrestore(&sptr_rlowns->sgtc_lock, flags);
            
            /*!
             * This is the priority that the thread holding the lock will be adjusted to, 
             * which cannot be lower than its original priority 
             */
            max_prio = __THREAD_IS_LOW_PRIO(ori_prio, max_prio) ? max_prio : ori_prio;

            /*!< Priority changed */
            if (max_prio != cur_prio)
            {
                kuint32_t cpuid;

                g_inherit_monitor++;
                spin_lock_irqsave(&sptr_rival->sgtc_lock, &flags);

                /*!< Set inheritance priority */
                thread_set_inherit_priority(sptr_rival->sptr_attr, max_prio);
                /*!
                 * Immediately adjust the position of the hostile thread in the linked list, 
                 * without altering the thread state or the ownership of the linked list 
                 */
                __SET_THREAD_TARGET_STATE(sptr_rival, __GET_THREAD_STATE(sptr_rival));

                spin_unlock_irqrestore(&sptr_rival->sgtc_lock, flags);
                mr_preempt_disable();

            loop:
                cpuid = sptr_rival->cpu;
                sptr_lock = scheduler_cpu_lock(cpuid);
                spin_lock_irqsave(sptr_lock, &flags);

                if (__GET_THREAD_STATE(sptr_rival) != NR_THREAD_RUNNING)
                {
                    /*!< if cpu has more than 2 cores, sptr_thread may migrate between other cores that are not the current core */
                    if (mr_unlikely(cpuid != sptr_rival->cpu))
                    {
                        cpuid = sptr_rival->cpu;
                        spin_unlock_irqrestore(sptr_lock, flags);
                        goto loop;
                    }

                    schedule_thread_switch(sptr_rival);
                }

                spin_unlock_irqrestore(sptr_lock, flags);
                mr_preempt_enable();
            }

/*!< If support recursive inheritance (frequent retrieval of linked lists may slow down the system, use with caution) */
#if defined(CONFIG_INHERIT_RECURSION) && (CONFIG_INHERIT_RECURSION)
            sptr_self = sptr_rival;
            sptr_owner = sptr_self->sgtc_wait.sptr_wait;
            sptr_rival = sptr_owner ? sptr_owner->sptr_self : mr_nullptr;

            if (sptr_rival && (depth--))
            {
                sptr_owns = &sptr_self->sgtc_owners;
                
                g_inherit_recur_monitor++;
                spin_lock(&sptr_owner->sgtc_lock);

                continue;
            }
#endif

            break;
        };
    }
    else
    {
        spin_lock_irqsave(&sptr_owns->sgtc_lock, &flags);

        /*!
         * If the sptr_self has already been added to pending list, remove it first; 
         * because the lock will be continuously requested by this thread, 
         * this function will be frequently entered to prevent duplicate additions 
         */
        lock_pending_del(sptr_self);
        lock_pending_add(sptr_owner, sptr_self);

        spin_unlock_irqrestore(&sptr_owns->sgtc_lock, flags);
        spin_unlock(&sptr_owner->sgtc_lock);
    }

    return ER_NORMAL;
}

/*!
 * @brief   lock release
 * @param   sptr_owner
 * @retval  none
 * @note    Release the lock. When releasing, it is also necessary to check whether there is a more suitable priority level
 */
kint32_t unlock_release(struct thread *sptr_self, kbool_t inherit_enable)
{
    if (inherit_enable)
    {
        struct lock_owners *sptr_rlowns;
        kuint32_t cur_prio, ori_prio, max_prio = 0;
        kutype_t flags;

        /*!< The thread that holds the lock */
        sptr_rlowns = &sptr_self->sgtc_owners;

        /*!< Original priority */
        ori_prio = thread_get_ori_priority(sptr_self->sptr_attr);
        /*!< Realtime priority */
        cur_prio = thread_get_rt_priority(sptr_self->sptr_attr);

        /*! @note
         * Retrieve the current maximum priority 
         * (by searching among the remaining locks held, if the thread does not hold any locks, return 0)
         * Before that, the "unlock_context_restore" function must be called first to remove this lock, 
         * so that it will no longer participate in priority calculation
         */
        spin_lock_irqsave(&sptr_rlowns->sgtc_lock, &flags);
        max_prio = lock_find_max_priority(sptr_rlowns);
        spin_unlock_irqrestore(&sptr_rlowns->sgtc_lock, flags);

        /*!
         * This is the priority that this thread will be adjusted to, 
         * which cannot be lower than the inherent priority of the thread 
         */
        max_prio = __THREAD_IS_LOW_PRIO(ori_prio, max_prio) ? max_prio : ori_prio;

        if (max_prio != cur_prio)
        {
            /*!
             * Set the priority, but there is no need to initiate a switch, 
             * because the holder of the lock is the current thread 
             */
            spin_lock_irqsave(&sptr_self->sgtc_lock, &flags);
            thread_set_inherit_priority(sptr_self->sptr_attr, max_prio);
            spin_unlock_irqrestore(&sptr_self->sgtc_lock, flags);
        }
    }

    return ER_NORMAL;
}

/*!< ------------------------------------------------------------------------- */
/*!
 * @brief   inherit kernel init
 * @param   none
 * @retval  errno
 * @note    none
 */
static kint32_t __plat_init kernel_inherit_init(void)
{
    struct term_variable *sptr_var = &sgtc_inherit_monitor[0];
    kint32_t index;

    for (index = 0; index < g_num_inherit_monitor; index++)
        init_list_head(&sptr_var[index].sgtc_link);

    term_variable_add_more(sptr_var, g_num_inherit_monitor);
    return ER_NORMAL;
}

/*!
 * @brief   inherit kernel exit
 * @param   none
 * @retval  none
 * @note    none
 */
static void __plat_exit kernel_inherit_exit(void)
{
    term_variable_del_more(sgtc_inherit_monitor, g_num_inherit_monitor);
}

IMPORT_KERNEL_INIT(kernel_inherit_init);
IMPORT_KERNEL_EXIT(kernel_inherit_exit);

/*!< end of file */
