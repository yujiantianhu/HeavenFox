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
 * @param   sprt_lock
 * @retval  none
 * @note    set count = 0
 */
void mutex_init(struct mutex_lock *sprt_lock)
{
    if (sprt_lock)
        ATOMIC_SET(&sprt_lock->sgrt_atc);
}

/*!
 * @brief   mutex lock
 * @param   sprt_lock
 * @retval  none
 * @note    if it has been locked, schedule another thread
 */
void mutex_lock(struct mutex_lock *sprt_lock)
{
    if (!mrt_current)
        return;

    if (mutex_is_locked(sprt_lock))
        real_thread_schedule();
    
    atomic_inc(&sprt_lock->sgrt_atc);
}

/*!
 * @brief   mutex unlock
 * @param   sprt_lock
 * @retval  none
 * @note    none
 */
void mutex_unlock(struct mutex_lock *sprt_lock)
{
    if (!mrt_current)
        return;
    
    atomic_dec(&sprt_lock->sgrt_atc);
}

/*!< end of file */
