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
#include <kernel/rw_lock.h>
#include <kernel/sched.h>

/*!< The defines */


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
    }
}

/*!
 * @brief   rw_lock lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, schedule another thread; otherwise, lock it
 */
void rw_lock(struct rw_lock *sptr_lock)
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
    
    atomic_inc(&sptr_lock->sgtc_read);
    atomic_inc(&sptr_lock->sgtc_write);

    local_irq_restore(&flags);
}

/*!
 * @brief   rw_lock lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, return directly
 */
kint32_t rw_try_lock(struct rw_lock *sptr_lock)
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
    
    atomic_inc(&sptr_lock->sgtc_read);
    atomic_inc(&sptr_lock->sgtc_write);

    local_irq_restore(&flags);
    return ER_NORMAL;
}

/*!
 * @brief   rw_lock unlock
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void rw_unlock(struct rw_lock *sptr_lock)
{
    kutype_t flags;

    local_irq_save(&flags);
    if (!mr_current || !rw_is_locked(sptr_lock))
    {
        local_irq_restore(&flags);
        return;
    }
    
    atomic_dec(&sptr_lock->sgtc_read);
    atomic_dec(&sptr_lock->sgtc_write);

    local_irq_restore(&flags);
}

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

/*!< end of file */
