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
#include <kernel/sched.h>
#include <kernel/semaphore.h>

/*!< The defines */


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
        ATOMIC_SET(&sptr_sem->sgtc_atc, val);
}

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

/*!< end of file */
