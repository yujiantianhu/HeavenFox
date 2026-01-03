/*
 * Mutex Lock Interface
 *
 * File Name:   mutex.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.05
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <kernel/kernel.h>
#include <kernel/mutex.h>
#include <kernel/sched.h>

/*!< The defines */
#if defined(CONFIG_INHERIT) &&(CONFIG_INHERIT)
#define CONFIG_MUTEX_INHERIT                1
#else
#define CONFIG_MUTEX_INHERIT                0
#endif

#if defined(CONFIG_LOCK_WAKEALL) && (CONFIG_LOCK_WAKEALL)
#define CONFIG_MUTEX_WAKEALL                1
#else
#define CONFIG_MUTEX_WAKEALL                0
#endif

/*!< The functions */


/*!< API functions */
/*!
 * @brief   initial mutex lock
 * @param   sptr_lock
 * @retval  none
 * @note    set count = 0
 */
void mutex_init(struct mutex_lock *sptr_lock)
{
    if (sptr_lock)
    {
        atomic_set_val(&sptr_lock->sgtc_atc, 0);
        lock_context_init(&sptr_lock->sgtc_owner);
    }
}

/*!
 * @brief   mutex lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, schedule another thread; otherwise, lock it
 */
void mutex_lock(struct mutex_lock *sptr_lock)
{
    struct thread *sptr_self;
    struct lock_owner *sptr_owner;

    sptr_self = mr_current;
    if (mr_unlikely(!sptr_self))
        return;

    sptr_owner = &sptr_lock->sgtc_owner;
    mr_preempt_disable();
    spin_lock(&sptr_owner->sgtc_lock);

    /*!< If the lock is reentrant, increment the counter and return directly */
    if (sptr_owner->sptr_self == sptr_self)
    {   
        atomic_inc(&sptr_lock->sgtc_atc);
        mr_barrier();

        spin_unlock(&sptr_owner->sgtc_lock);
        mr_preempt_enable();
        return;
    }

    /*!< Check lock (essentially: atomic_get_val(&sptr_lock->sgtc_atc)) */
    while (mutex_is_locked(sptr_lock))
    {
        spin_unlock(&sptr_owner->sgtc_lock);

        /*!
         * The lock is held by another thread; 
         * check for priority inheritance and add the current thread to the lock's "wait chain."
         * Since the spin lock has been released at this point, it is necessary to re-evaluate the lock count value within the function
         */
        if (mr_unlikely(lock_compete(sptr_owner, &sptr_lock->sgtc_atc, CONFIG_MUTEX_INHERIT)))
            goto loop;

        /*!< Suspend self */
        mr_preempt_enable();
        schedule_self_suspend();
        mr_preempt_disable();

loop:
        spin_lock(&sptr_owner->sgtc_lock);
    }

    atomic_inc(&sptr_lock->sgtc_atc);
    sptr_owner->sptr_self = sptr_self;
    mr_barrier();

    /*!< Acquire the lock, make a mark, and add the current lock to the "lock holding linked list" of this thread */
    lock_context_save(sptr_owner);

    spin_unlock(&sptr_owner->sgtc_lock);
    mr_preempt_enable();
}

/*!
 * @brief   mutex wait
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, schedule another thread
 */
void mutex_wait(struct mutex_lock *sptr_lock)
{
    if (!mr_current)
        return;

    while (mutex_is_locked(sptr_lock))
        schedule_thread();
}

/*!
 * @brief   mutex lock
 * @param   sptr_lock
 * @retval  0: success
 * @note    if it has been locked, return directly
 */
kint32_t mutex_try_lock(struct mutex_lock *sptr_lock)
{
    struct thread *sptr_self;
    struct lock_owner *sptr_owner;
    
    sptr_self = mr_current;
    if (mr_unlikely(!sptr_self))
        return -ER_FORBID;

    sptr_owner = &sptr_lock->sgtc_owner;

    /*!< In IRQ_Handler */
    if (IS_IN_EXCEPTION())
    {
        kutype_t flags;

        local_irq_save(&flags);
        if (mutex_is_locked(sptr_lock))
        {
            local_irq_restore(&flags);
            return -ER_BUSY;
        }

        atomic_inc(&sptr_lock->sgtc_atc);
        sptr_owner->sptr_self = mr_nullptr;
        mr_barrier();

        local_irq_restore(&flags);
        return ER_NORMAL;
    }

    mr_preempt_disable();
    spin_lock(&sptr_owner->sgtc_lock);

    /*!< If the lock is reentrant, increment the counter and return directly */
    if (sptr_owner->sptr_self == sptr_self)
    {   
        atomic_inc(&sptr_lock->sgtc_atc);
        mr_barrier();

        spin_unlock(&sptr_owner->sgtc_lock);
        mr_preempt_enable();
        return ER_NORMAL;
    }

    /*!< Check lock (essentially: atomic_get_val(&sptr_lock->sgtc_atc)) */
    if (mutex_is_locked(sptr_lock))
    {
        spin_unlock(&sptr_owner->sgtc_lock);
        return -ER_BUSY;
    }

    atomic_inc(&sptr_lock->sgtc_atc);
    sptr_owner->sptr_self = sptr_self;
    mr_barrier();

    /*!< Acquire the lock, make a mark, and add the current lock to the "lock holding linked list" of this thread */
    lock_context_save(sptr_owner);

    spin_unlock(&sptr_owner->sgtc_lock);
    mr_preempt_enable();

    return ER_NORMAL;
}

/*!
 * @brief   mutex unlock
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void mutex_unlock(struct mutex_lock *sptr_lock)
{
    struct thread *sptr_self;
    struct lock_owner *sptr_owner;

    sptr_self = mr_current;
    if (mr_unlikely(!sptr_self))
        return;

    sptr_owner = &sptr_lock->sgtc_owner;

    /*!< In IRQ_Handler */
    if (IS_IN_EXCEPTION())
    {
        kutype_t flags;

        local_irq_save(&flags);
        if (sptr_owner->sptr_self || 
            !mutex_is_locked(sptr_lock))
        {
            local_irq_restore(&flags);
            return;
        }

        atomic_dec(&sptr_lock->sgtc_atc);
        mr_barrier();

        /*!< After decrementing the count, the lock is still locked, indicating that the lock is still in a reentrant state */
        if (mutex_is_locked(sptr_lock))
        {
            local_irq_restore(&flags);
            return;
        }

        /*!< This lock has been released, and the thread with the highest priority on the request list will be woken up */
        unlock_pending_wakeup_nolock(sptr_owner, CONFIG_MUTEX_WAKEALL);
        local_irq_restore(&flags);
        return;
    }

    mr_preempt_disable();
    spin_lock(&sptr_owner->sgtc_lock);

    /*!< Only the holder can release the lock; the lock can only be released when it is held */
    if (mr_unlikely(sptr_owner->sptr_self != sptr_self) ||
        mr_unlikely(!mutex_is_locked(sptr_lock)))
    {
        spin_unlock(&sptr_owner->sgtc_lock);
        mr_preempt_enable();
        return;
    }

    atomic_dec(&sptr_lock->sgtc_atc);
    mr_barrier();

    /*!< After decrementing the count, the lock is still locked, indicating that the lock is still in a reentrant state */
    if (mutex_is_locked(sptr_lock))
    {
        spin_unlock(&sptr_owner->sgtc_lock);
        mr_preempt_enable();
        return;
    }

    /*!< Remove this lock from the lock list held by this thread. Be sure to call it before the "unlock_release" function */
    unlock_context_restore(sptr_owner);
    spin_unlock(&sptr_owner->sgtc_lock);

    /*!< Released one lock, recalculated the priority of this thread */
    unlock_release(sptr_self, CONFIG_MUTEX_INHERIT);

    /*!< This lock has been released, and the thread with the highest priority on the request list will be woken up */
    unlock_pending_wakeup(sptr_owner, CONFIG_MUTEX_WAKEALL);
    mr_preempt_enable();
}

/*!
 * @brief   mutex destroy
 * @param   sptr_lock
 * @retval  none
 * @note    If you want to destroy the lock, please be sure to call this function first
 */
void mutex_destroy(struct mutex_lock *sptr_lock)
{
    unlock_context_restore(&sptr_lock->sgtc_owner);
    unlock_pending_del_all(&sptr_lock->sgtc_owner);
}

/*!< end of file */
