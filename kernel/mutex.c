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
        ATOMIC_SET(&sptr_lock->sgtc_atc, 0);
}

/*!
 * @brief   mutex lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, schedule another thread; otherwise, lock it
 */
void mutex_lock(struct mutex_lock *sptr_lock)
{
    if (!mr_current)
        return;

    while (mutex_is_locked(sptr_lock))
        schedule_thread();
    
    atomic_inc(&sptr_lock->sgtc_atc);
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
 * @retval  none
 * @note    if it has been locked, return directly
 */
kint32_t mutex_try_lock(struct mutex_lock *sptr_lock)
{
    if (!mr_current)
        return -ER_FORBID;

    if (mutex_is_locked(sptr_lock))
        return -ER_BUSY;
    
    atomic_inc(&sptr_lock->sgtc_atc);

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
    if (!mr_current || !mutex_is_locked(sptr_lock))
        return;
    
    atomic_dec(&sptr_lock->sgtc_atc);
}

/*!< end of file */
