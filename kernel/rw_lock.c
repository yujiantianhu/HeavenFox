/*
 * Read/Write Lock Interface
 *
 * File Name:   rw_lock.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.01.21
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <kernel/kernel.h>
#include <kernel/preempt.h>
#include <kernel/rw_lock.h>
#include <kernel/sched.h>
#include <term/term.h>

/*!< The defines */
#if defined(CONFIG_INHERIT) &&(CONFIG_INHERIT)
#define CONFIG_RW_LOCK_INHERIT              1
#else
#define CONFIG_RW_LOCK_INHERIT              0
#endif

#if defined(CONFIG_LOCK_WAKEALL) && (CONFIG_LOCK_WAKEALL)
#define CONFIG_RW_LOCK_WAKEALL              1
#else
#define CONFIG_RW_LOCK_WAKEALL              0
#endif

/*!< The globals */
static kint32_t g_rd_lock_monitor;
static kint32_t g_wr_lock_monitor;

static struct term_variable sgtc_rw_lock_monitor[] = 
{ 
    { .name = "g_rd_lock_monitor", .var = &g_rd_lock_monitor, .num = 1 },
    { .name = "g_wr_lock_monitor", .var = &g_wr_lock_monitor, .num = 1 },
};
static const kusize_t g_num_rw_lock_monitor = ARRAY_SIZE(sgtc_rw_lock_monitor);

/*!< The functions */


/*!< API functions */
/*!
 * @brief   initial rw_lock lock
 * @param   sptr_lock
 * @retval  none
 * @note    set count = 0
 */
void rw_lock_init(struct rw_lock *sptr_lock)
{
    if (sptr_lock)
    {
        ATOMIC_SET(&sptr_lock->sgtc_read, 0);
        ATOMIC_SET(&sptr_lock->sgtc_write, 0);

        lock_context_init(&sptr_lock->sgtc_rds);
        lock_context_init(&sptr_lock->sgtc_wrs);
    }
}

#if 1
/*!
 * @brief   check if read_lock is locked
 * @param   sptr_lock
 * @retval  locked(true) / unlocked(false)
 * @note    none
 */
static __unused 
kbool_t __rd_is_locked(void *lock)
{
    return rd_is_locked((struct rw_lock *)lock);
}

/*!
 * @brief   check if write_lock is locked
 * @param   sptr_lock
 * @retval  locked(true) / unlocked(false)
 * @note    none
 */
static __unused 
kbool_t __wr_is_locked(void *lock)
{
    return wr_is_locked((struct rw_lock *)lock);
}

/*!
 * @brief   check if rw_lock is locked
 * @param   sptr_lock
 * @retval  locked(true) / unlocked(false)
 * @note    none
 */
static kbool_t __rw_is_locked(void *lock)
{
    return rw_is_locked((struct rw_lock *)lock);
}

/*!
 * @brief   Wake up threads which are pending, and detach pending lists
 * @param   sptr_rds
 * @retval  none
 * @note    It should be used by "lock destroy"
 */
static void __read_pending_wakeup_nolock(struct lock_owner *sptr_rds)
{
    struct list_head *sptr_head;

    sptr_head = &sptr_rds->sgtc_pendings;

    if (!mr_list_empty(sptr_head))
    {
        struct lock_waiter *sptr_pwait, *sptr_temp;
        struct thread *sptr_pert;
        struct list_head sgtc_lists;

        init_list_head(&sgtc_lists);

        /*!< Transfer the waiting list and clear the old list */
        list_head_splice_init(&sgtc_lists, sptr_head);

        foreach_list_next_entry_safe(sptr_pwait, sptr_temp, &sgtc_lists, sgtc_link) 
        {
            sptr_pert = mr_container_of(sptr_pwait, struct thread, sgtc_wait);

            sptr_pwait->sptr_wait = mr_nullptr;
            list_head_del(&sptr_pwait->sgtc_link);

            schedule_thread_wakeup(sptr_pert->tid);
        }
    }
}

/*!
 * @brief   Wake up threads which are pending, and detach pending lists
 * @param   sptr_rds
 * @param   sptr_wrs
 * @retval  none
 * @note    It should be used by "lock destroy"
 */
static void __read_pending_wakeup(struct lock_owner *sptr_rds, struct lock_owner *sptr_wrs)
{
    struct list_head *sptr_head;
    kutype_t flags;

    sptr_head = &sptr_rds->sgtc_pendings;
    spin_lock(&sptr_wrs->sgtc_lock);

    if (mr_list_empty(sptr_head))
        spin_unlock(&sptr_wrs->sgtc_lock);
    else
    {
        struct lock_waiter *sptr_pwait, *sptr_temp;
        struct lock_owners *sptr_owns;
        struct thread *sptr_pert;
        struct list_head sgtc_lists;

        init_list_head(&sgtc_lists);

        /*!< Transfer the waiting list and clear the old list */
        list_head_splice_init(&sgtc_lists, sptr_head);

        spin_unlock(&sptr_wrs->sgtc_lock);

        foreach_list_next_entry_safe(sptr_pwait, sptr_temp, &sgtc_lists, sgtc_link) 
        {
            sptr_pert = mr_container_of(sptr_pwait, struct thread, sgtc_wait);
            sptr_owns = &sptr_pert->sgtc_owners;

            spin_lock_irqsave(&sptr_owns->sgtc_lock, &flags);
            sptr_pwait->sptr_wait = mr_nullptr;
            list_head_del(&sptr_pwait->sgtc_link);
            spin_unlock_irqrestore(&sptr_owns->sgtc_lock, flags);

            schedule_thread_wakeup(sptr_pert->tid);
        }
    }
}

/*!
 * @brief   read lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, schedule another thread; otherwise, lock it
 */
void rd_lock(struct rw_lock *sptr_lock)
{
    struct thread *sptr_self;
    struct lock_owners *sptr_owns;
    struct lock_owner *sptr_rds;
    struct lock_owner *sptr_wrs;
    kutype_t flags;

    sptr_self = mr_current;
    if (mr_unlikely(!sptr_self))
        return;

    sptr_owns = &sptr_self->sgtc_owners;
    sptr_rds = &sptr_lock->sgtc_rds;
    sptr_wrs = &sptr_lock->sgtc_wrs;

    mr_preempt_disable();
    spin_lock(&sptr_wrs->sgtc_lock);

    /*!< Check lock (essentially: atomic_get_val(&sptr_lock->sgtc_atc)) */
    while (wr_is_locked(sptr_lock))
    {
        /*!
         * The lock is held by another thread; 
         * check for priority inheritance and add the current thread to the lock's "wait chain."
         * Since the spin lock has been released at this point, it is necessary to re-evaluate the lock count value within the function
         */
        spin_lock_irqsave(&sptr_owns->sgtc_lock, &flags);

        /*!
         * If the sptr_self has already been added to pending list, remove it first; 
         * because the lock will be continuously requested by this thread, 
         * this function will be frequently entered to prevent duplicate additions 
         */
        lock_pending_del(sptr_self);
        lock_pending_add(sptr_rds, sptr_self);

        spin_unlock_irqrestore(&sptr_owns->sgtc_lock, flags);
        spin_unlock(&sptr_wrs->sgtc_lock);

        g_rd_lock_monitor++;

        /*!< Suspend self */
        mr_preempt_enable();
        schedule_self_suspend();
        mr_preempt_disable();

        spin_lock(&sptr_wrs->sgtc_lock);
    }

    atomic_inc(&sptr_lock->sgtc_read);
    mr_smp_mb();

    /*! @warning
     * We don't care which read-locks are held by current thread, 
     * because sptr_rds->sgtc_link is only one, it can not be add to multiple threads! 
     */
    if (mr_unlikely(in_lock_pending(&sptr_self->sgtc_wait)))
    {
        spin_lock_irqsave(&sptr_owns->sgtc_lock, &flags);
        lock_pending_del(sptr_self);
        spin_unlock_irqrestore(&sptr_owns->sgtc_lock, flags);
    }

    spin_unlock(&sptr_wrs->sgtc_lock);
    mr_preempt_enable();
}

/*!
 * @brief   rw_lock lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, return directly
 */
kint32_t rd_try_lock(struct rw_lock *sptr_lock)
{
    struct thread *sptr_self;
    struct lock_owner *sptr_wrs;
    kutype_t flags;

    sptr_self = mr_current;
    if (mr_unlikely(!sptr_self))
        return -ER_FORBID;

    sptr_wrs = &sptr_lock->sgtc_wrs;

    /*!< In IRQ_Handler */
    if (IN_INTERRUPT())
    {
        local_irq_save(&flags);
        if (wr_is_locked(sptr_lock))
        {
            local_irq_restore(&flags);
            return -ER_BUSY;
        }

        atomic_inc(&sptr_lock->sgtc_read);
        mr_smp_mb();

        local_irq_restore(&flags);
        return ER_NORMAL;
    }

    mr_preempt_disable();
    spin_lock(&sptr_wrs->sgtc_lock);

    /*!< Check lock (essentially: atomic_get_val(&sptr_lock->sgtc_atc)) */
    if (wr_is_locked(sptr_lock))
    {
        g_rd_lock_monitor++;
        spin_unlock(&sptr_wrs->sgtc_lock);
        mr_preempt_enable();
        return -ER_BUSY;
    }

    atomic_inc(&sptr_lock->sgtc_read);
    mr_smp_mb();

    spin_unlock(&sptr_wrs->sgtc_lock);
    mr_preempt_enable();

    return ER_NORMAL;
}

/*!
 * @brief   rw_lock unlock
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void rd_unlock(struct rw_lock *sptr_lock)
{
    struct thread *sptr_self;
//  struct lock_owner *sptr_rds;
    struct lock_owner *sptr_wrs;

    sptr_self = mr_current;
    if (mr_unlikely(!sptr_self))
        return;

//  sptr_rds = &sptr_lock->sgtc_rds;
    sptr_wrs = &sptr_lock->sgtc_wrs;

    /*!< In IRQ_Handler */
    if (IN_INTERRUPT())
    {
        kutype_t flags;

        local_irq_save(&flags);
        if (!rd_is_locked(sptr_lock))
        {
            local_irq_restore(&flags);
            return;
        }

        atomic_dec(&sptr_lock->sgtc_read);
        mr_smp_mb();

        /*!< After decrementing the count, the lock is still locked, indicating that the lock is still in a reentrant state */
        if (rd_is_locked(sptr_lock))
        {
            local_irq_restore(&flags);
            return;
        }

        /*!< This lock has been released, and the thread with the highest priority on the request list will be woken up */
        if (!rw_is_locked(sptr_lock))
            unlock_pending_wakeup_nolock(sptr_wrs, CONFIG_RW_LOCK_WAKEALL);

        local_irq_restore(&flags);
        return;
    }

    mr_preempt_disable();
    spin_lock(&sptr_wrs->sgtc_lock);

    /*!< The lock can only be released when it is held */
    if (mr_unlikely(!rd_is_locked(sptr_lock)))
    {
        spin_unlock(&sptr_wrs->sgtc_lock);
        mr_preempt_enable();
        return;
    }

    atomic_dec(&sptr_lock->sgtc_read);
    mr_smp_mb();

    /*!< After decrementing the count, the lock is still locked, indicating that the lock is still in a locked state */
    if (rd_is_locked(sptr_lock))
    {
        spin_unlock(&sptr_wrs->sgtc_lock);
        mr_preempt_enable();
        return;
    }

    /*!< Remove this lock from the lock list held by this thread. Be sure to call it before the "unlock_release" function */
//  unlock_context_restore(sptr_rds);
    spin_unlock(&sptr_wrs->sgtc_lock);

    /*!< This lock has been released, and the thread with the highest priority on the request list will be woken up */
    if (!rw_is_locked(sptr_lock))
        unlock_pending_wakeup(sptr_wrs, CONFIG_RW_LOCK_WAKEALL);

    mr_preempt_enable();
}

/*!
 * @brief   write lock lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, schedule another thread; otherwise, lock it
 */
void wr_lock(struct rw_lock *sptr_lock)
{
    struct thread *sptr_self;
    struct lock_owner *sptr_wrs;

    sptr_self = mr_current;
    if (mr_unlikely(!sptr_self))
        return;

    sptr_wrs = &sptr_lock->sgtc_wrs;
    mr_preempt_disable();
    spin_lock(&sptr_wrs->sgtc_lock);

    /*!< If the lock is reentrant, increment the counter and return directly */
    if (sptr_wrs->sptr_self == sptr_self)
    {   
        atomic_inc(&sptr_lock->sgtc_write);
        mr_smp_mb();

        spin_unlock(&sptr_wrs->sgtc_lock);
        mr_preempt_enable();
        return;
    }

    /*!< Check lock (essentially: atomic_get_val(&sptr_lock->sgtc_write)) */
    while (rw_is_locked(sptr_lock))
    {
        spin_unlock(&sptr_wrs->sgtc_lock);

        /*!
         * The lock is held by another thread; 
         * check for priority inheritance and add the current thread to the lock's "wait chain."
         * Since the spin lock has been released at this point, it is necessary to re-evaluate the lock count value within the function
         */
        if (mr_unlikely(lock_compete(sptr_wrs, __rw_is_locked, sptr_lock, CONFIG_RW_LOCK_INHERIT)))
            goto loop;

        g_wr_lock_monitor++;

        /*!< Suspend self */
        mr_preempt_enable();
        schedule_self_suspend();
        mr_preempt_disable();

loop:
        spin_lock(&sptr_wrs->sgtc_lock);
    }

    atomic_inc(&sptr_lock->sgtc_write);
    sptr_wrs->sptr_self = sptr_self;
    mr_smp_mb();

    /*!< Acquire the lock, make a mark, and add the current lock to the "lock holding linked list" of this thread */
    lock_context_save(sptr_wrs);

    spin_unlock(&sptr_wrs->sgtc_lock);
    mr_preempt_enable();
}

/*!
 * @brief   write lock lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, return directly
 */
kint32_t wr_try_lock(struct rw_lock *sptr_lock)
{
    struct thread *sptr_self;
    struct lock_owner *sptr_wrs;

    sptr_self = mr_current;
    if (mr_unlikely(!sptr_self))
        return -ER_FORBID;

    sptr_wrs = &sptr_lock->sgtc_wrs;

    /*!< In IRQ_Handler */
    if (IN_INTERRUPT())
    {
        kutype_t flags;

        local_irq_save(&flags);
        if (rw_is_locked(sptr_lock))
        {
            local_irq_restore(&flags);
            return -ER_BUSY;
        }

        atomic_inc(&sptr_lock->sgtc_write);
        sptr_wrs->sptr_self = mr_nullptr;
        mr_smp_mb();

        local_irq_restore(&flags);
        return ER_NORMAL;
    }

    mr_preempt_disable();
    spin_lock(&sptr_wrs->sgtc_lock);

    /*!< If the lock is reentrant, increment the counter and return directly */
    if (sptr_wrs->sptr_self == sptr_self)
    {   
        atomic_inc(&sptr_lock->sgtc_write);
        mr_smp_mb();

        spin_unlock(&sptr_wrs->sgtc_lock);
        mr_preempt_enable();
        return ER_NORMAL;
    }

    /*!< Check lock (essentially: atomic_get_val(&sptr_lock->sgtc_write)) */
    if (rw_is_locked(sptr_lock))
    {
        g_wr_lock_monitor++;
        spin_unlock(&sptr_wrs->sgtc_lock);
        mr_preempt_enable();
        return -ER_BUSY;
    }

    atomic_inc(&sptr_lock->sgtc_write);
    sptr_wrs->sptr_self = sptr_self;
    mr_smp_mb();

    /*!< Acquire the lock, make a mark, and add the current lock to the "lock holding linked list" of this thread */
    lock_context_save(sptr_wrs);

    spin_unlock(&sptr_wrs->sgtc_lock);
    mr_preempt_enable();

    return ER_NORMAL;
}

/*!
 * @brief   write lock unlock
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void wr_unlock(struct rw_lock *sptr_lock)
{
    struct thread *sptr_self;
    struct lock_owner *sptr_rds;
    struct lock_owner *sptr_wrs;

    sptr_self = mr_current;
    if (mr_unlikely(!sptr_self))
        return;

    sptr_rds = &sptr_lock->sgtc_rds;
    sptr_wrs = &sptr_lock->sgtc_wrs;

    /*!< In IRQ_Handler */
    if (IN_INTERRUPT())
    {
        kutype_t flags;

        local_irq_save(&flags);
        if (sptr_wrs->sptr_self || 
            !wr_is_locked(sptr_lock))
        {
            local_irq_restore(&flags);
            return;
        }

        atomic_dec(&sptr_lock->sgtc_write);
        mr_smp_mb();

        /*!< After decrementing the count, the lock is still locked, indicating that the lock is still in a reentrant state */
        if (wr_is_locked(sptr_lock))
        {
            local_irq_restore(&flags);
            return;
        }

        /*!< This lock has been released, and the thread with the highest priority on the request list will be woken up */
        unlock_pending_wakeup_nolock(sptr_wrs, CONFIG_RW_LOCK_WAKEALL);
        __read_pending_wakeup_nolock(sptr_rds);

        local_irq_restore(&flags);
        return;
    }

    mr_preempt_disable();
    spin_lock(&sptr_wrs->sgtc_lock);

    /*!< Only the holder can release the lock; the lock can only be released when it is held */
    if (mr_unlikely(sptr_wrs->sptr_self != sptr_self) ||
        mr_unlikely(!wr_is_locked(sptr_lock)))
    {
        spin_unlock(&sptr_wrs->sgtc_lock);
        mr_preempt_enable();
        return;
    }

    atomic_dec(&sptr_lock->sgtc_write);
    mr_smp_mb();

    /*!< After decrementing the count, the lock is still locked, indicating that the lock is still in a reentrant state */
    if (wr_is_locked(sptr_lock))
    {
        spin_unlock(&sptr_wrs->sgtc_lock);
        mr_preempt_enable();
        return;
    }

    /*!< Remove this lock from the lock list held by this thread. Be sure to call it before the "unlock_release" function */
    unlock_context_restore(sptr_wrs);
    spin_unlock(&sptr_wrs->sgtc_lock);

    /*!< Released one lock, recalculated the priority of this thread */
    unlock_release(sptr_self, CONFIG_RW_LOCK_INHERIT);

    /*!< This lock has been released, and the thread with the highest priority on the request list will be woken up */
    unlock_pending_wakeup(sptr_wrs, CONFIG_RW_LOCK_WAKEALL);
    __read_pending_wakeup(sptr_rds, sptr_wrs);
    mr_preempt_enable();
}

#else
/*!
 * @brief   read lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, schedule another thread; otherwise, lock it
 */
void rd_lock(struct rw_lock *sptr_lock)
{
    kutype_t flags;

    if (!mr_current)
        return;

    local_irq_save(&flags);
    while (wr_is_locked(sptr_lock))
    {
        local_irq_restore(&flags);
        schedule_thread();
    }
    
    atomic_inc(&sptr_lock->sgtc_read);
    local_irq_restore(&flags);
}

/*!
 * @brief   rw_lock lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, return directly
 */
kint32_t rd_try_lock(struct rw_lock *sptr_lock)
{
    kutype_t flags;

    if (!mr_current)
        return -ER_FORBID;

    local_irq_save(&flags);
    if (wr_is_locked(sptr_lock))
    {
        local_irq_restore(&flags);
        return -ER_BUSY;
    }
    
    atomic_inc(&sptr_lock->sgtc_read);
    local_irq_restore(&flags);

    return ER_NORMAL;
}

/*!
 * @brief   rw_lock unlock
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void rd_unlock(struct rw_lock *sptr_lock)
{
    kutype_t flags;

    local_irq_save(&flags);
    if (!mr_current || !rd_is_locked(sptr_lock))
    {
        local_irq_restore(&flags);
        return;
    }
    
    atomic_dec(&sptr_lock->sgtc_read);
    local_irq_restore(&flags);
}

/*!
 * @brief   write lock lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, schedule another thread; otherwise, lock it
 */
void wr_lock(struct rw_lock *sptr_lock)
{
    kutype_t flags;

    if (!mr_current)
        return;

    local_irq_save(&flags);
    while (rw_is_locked(sptr_lock))
    {
        local_irq_restore(&flags);
        schedule_thread();
    }
    
    atomic_inc(&sptr_lock->sgtc_write);
    local_irq_restore(&flags);
}

/*!
 * @brief   write lock lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, return directly
 */
kint32_t wr_try_lock(struct rw_lock *sptr_lock)
{
    kutype_t flags;

    if (!mr_current)
        return -ER_FORBID;

    local_irq_save(&flags);
    if (rw_is_locked(sptr_lock))
    {
        local_irq_restore(&flags);
        return -ER_BUSY;
    }
    
    atomic_inc(&sptr_lock->sgtc_write);
    local_irq_restore(&flags);

    return ER_NORMAL;
}

/*!
 * @brief   write lock unlock
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void wr_unlock(struct rw_lock *sptr_lock)
{
    kutype_t flags;

    local_irq_save(&flags);
    
    if (!mr_current || !rw_is_locked(sptr_lock))
    {
        local_irq_restore(&flags);
        return;
    }
    
    atomic_dec(&sptr_lock->sgtc_write);
    local_irq_restore(&flags);
}

#endif

/*!< ------------------------------------------------------------------------- */
/*!
 * @brief   rw_lock init
 * @param   none
 * @retval  errno
 * @note    none
 */
static kint32_t __plat_init kernel_rw_lock_init(void)
{
    struct term_variable *sptr_var = &sgtc_rw_lock_monitor[0];
    kint32_t index;

    for (index = 0; index < g_num_rw_lock_monitor; index++)
        init_list_head(&sptr_var[index].sgtc_link);

    term_variable_add_more(sptr_var, g_num_rw_lock_monitor);
    return ER_NORMAL;
}

/*!
 * @brief   rw_lock exit
 * @param   none
 * @retval  none
 * @note    none
 */
static void __plat_exit kernel_rw_lock_exit(void)
{
    term_variable_del_more(&sgtc_rw_lock_monitor[0], g_num_rw_lock_monitor);
}

IMPORT_KERNEL_INIT(kernel_rw_lock_init);
IMPORT_KERNEL_EXIT(kernel_rw_lock_exit);

/*!< end of file */
