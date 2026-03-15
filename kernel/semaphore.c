/*
 * Semaphore Interface
 *
 * File Name:   semaphore.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.06.01
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <kernel/kernel.h>
#include <kernel/preempt.h>
#include <kernel/sched.h>
#include <kernel/semaphore.h>
#include <term/term.h>

/*!< The defines */


/*!< The globals */
static kint32_t g_sema_monitor;
static struct term_variable sgtc_sema_monitor = { .name = "g_sema_monitor", .var = &g_sema_monitor, .num = 1 };

/*!< The functions */


/*!< API functions */
/*!
 * @brief   initial semaphore
 * @param   sptr_sem
 * @retval  none
 * @note    set count = sem
 */
void sema_init(struct semaphore *sptr_sem, kuint32_t val)
{
    if (sptr_sem)
    {
        sptr_sem->init_val = val;
        atomic_set_val(&sptr_sem->sgtc_atc, val);
        lock_context_init(&sptr_sem->sgtc_rec);
    }
}

#if 1
/*!
 * @brief   semaphore lock
 * @param   sptr_sem
 * @retval  none
 * @note    if it has been locked, schedule another thread
 */
void sema_down(struct semaphore *sptr_sem)
{
    struct thread *sptr_self;
    struct lock_owners *sptr_owns;
    struct lock_owner *sptr_rec;
    kutype_t flags;

    sptr_self = mr_current;
    if (mr_unlikely(!sptr_self))
        return;

    sptr_owns = &sptr_self->sgtc_owners;
    sptr_rec = &sptr_sem->sgtc_rec;

    mr_preempt_disable();
    spin_lock(&sptr_rec->sgtc_lock);

    /*!< Check lock (essentially: atomic_get_val(&sptr_sem->sgtc_atc)) */
    while (sema_is_locked(sptr_sem))
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
        lock_pending_add(sptr_rec, sptr_self);

        spin_unlock_irqrestore(&sptr_owns->sgtc_lock, flags);
        spin_unlock(&sptr_rec->sgtc_lock);

        g_sema_monitor++;

        /*!< Suspend self */
        mr_preempt_enable();
        schedule_self_suspend();
        mr_preempt_disable();

        spin_lock(&sptr_rec->sgtc_lock);
    }

    mr_sem_dec(sptr_sem);

    /*! @warning
     * We don't care which read-locks are held by current thread, 
     * because sptr_rec->sgtc_link is only one, it can not be add to multiple threads! 
     */
    if (mr_unlikely(in_lock_pending(&sptr_self->sgtc_wait)))
    {
        spin_lock_irqsave(&sptr_owns->sgtc_lock, &flags);
        lock_pending_del(sptr_self);
        spin_unlock_irqrestore(&sptr_owns->sgtc_lock, flags);
    }

    spin_unlock(&sptr_rec->sgtc_lock);
    mr_preempt_enable();
}

/*!
 * @brief   semaphore lock
 * @param   sptr_sem
 * @retval  none
 * @note    if it has been locked, return directly
 */
kint32_t sema_down_try_lock(struct semaphore *sptr_sem)
{
    struct thread *sptr_self;
    struct lock_owner *sptr_rec;

    sptr_self = mr_current;
    if (mr_unlikely(!sptr_self))
        return -ER_FORBID;

    sptr_rec = &sptr_sem->sgtc_rec;

    /*!< In IRQ_Handler */
    if (mr_unlikely(IN_INTERRUPT()))
        return -ER_FORBID;

    mr_preempt_disable();
    spin_lock(&sptr_rec->sgtc_lock);

    /*!< Check lock (essentially: atomic_get_val(&sptr_sem->sgtc_atc)) */
    if (sema_is_locked(sptr_sem))
    {
        g_sema_monitor++;
        spin_unlock(&sptr_rec->sgtc_lock);
        mr_preempt_enable();
        return -ER_BUSY;
    }

    mr_sem_dec(sptr_sem);
    spin_unlock(&sptr_rec->sgtc_lock);
    mr_preempt_enable();

    return ER_NORMAL;
}

/*!
 * @brief   semaphore unlock
 * @param   sptr_sem
 * @retval  none
 * @note    none
 */
void sema_up(struct semaphore *sptr_sem)
{
    struct thread *sptr_self;
    struct lock_owner *sptr_rec;

    sptr_self = mr_current;
    if (mr_unlikely(!sptr_self))
        return;

    sptr_rec = &sptr_sem->sgtc_rec;

    /*!< In IRQ_Handler */
    if (mr_unlikely(IN_INTERRUPT()))
        return;

    mr_preempt_disable();
    spin_lock(&sptr_rec->sgtc_lock);

    /*!< The lock can only be released when it is held */
    if (mr_unlikely(mr_sem_get(sptr_sem) >= sptr_sem->init_val))
    {
        spin_unlock(&sptr_rec->sgtc_lock);
        mr_preempt_enable();
        return;
    }

    /*!< Release semaphore, but do not over the original value */
    mr_sem_inc(sptr_sem);
    spin_unlock(&sptr_rec->sgtc_lock);

    /*!< This lock has been released, and the thread with the highest priority on the request list will be woken up */
    if (mr_likely(!sema_is_locked(sptr_sem)))
        unlock_pending_wakeup(sptr_rec, mr_sem_get(sptr_sem) > 1);

    mr_preempt_enable();
}

#else
/*!
 * @brief   semaphore lock
 * @param   sptr_sem
 * @retval  none
 * @note    if it has been locked, schedule another thread
 */
void sema_down(struct semaphore *sptr_sem)
{
    kutype_t flags;

    if (!mr_current)
        return;

    local_irq_save(&flags);
    while (sema_is_locked(sptr_sem))
    {
        local_irq_restore(&flags);
        schedule_thread();
    }
    
    atomic_dec(&sptr_sem->sgtc_atc);
    local_irq_restore(&flags);
}

/*!
 * @brief   semaphore lock
 * @param   sptr_sem
 * @retval  none
 * @note    if it has been locked, return directly
 */
kint32_t sema_down_try_lock(struct semaphore *sptr_sem)
{
    kutype_t flags;
    
    if (!mr_current)
        return -ER_FORBID;

    local_irq_save(&flags);
    if (sema_is_locked(sptr_sem))
    {
        local_irq_restore(&flags);
        return -ER_BUSY;
    }
    
    atomic_dec(&sptr_sem->sgtc_atc);
    local_irq_restore(&flags);

    return ER_NORMAL;
}

/*!
 * @brief   semaphore unlock
 * @param   sptr_sem
 * @retval  none
 * @note    none
 */
void sema_up(struct semaphore *sptr_sem)
{
    kutype_t flags;

    local_irq_save(&flags);
    if (!mr_current || !sema_is_locked(sptr_sem))
    {
        local_irq_restore(&flags);
        return;
    }
    
    atomic_inc(&sptr_sem->sgtc_atc);
    local_irq_restore(&flags);
}
#endif

/*!< ------------------------------------------------------------------------- */
/*!
 * @brief   sema init
 * @param   none
 * @retval  errno
 * @note    none
 */
static kint32_t __plat_init kernel_sema_init(void)
{
    struct term_variable *sptr_var = &sgtc_sema_monitor;

    init_list_head(&sptr_var->sgtc_link);
    term_variable_add(sptr_var);

    return ER_NORMAL;
}

/*!
 * @brief   sema exit
 * @param   none
 * @retval  none
 * @note    none
 */
static void __plat_exit kernel_sema_exit(void)
{
    term_variable_del(&sgtc_sema_monitor);
}

IMPORT_KERNEL_INIT(kernel_sema_init);
IMPORT_KERNEL_EXIT(kernel_sema_exit);

/*!< end of file */
