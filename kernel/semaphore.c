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
 * @param   sprt_sem
 * @retval  none
 * @note    set count = sem
 */
void sema_init(struct semaphore *sprt_sem, kuint32_t val)
{
    if (sprt_sem)
        ATOMIC_SET(&sprt_sem->sgrt_atc, val);
}

/*!
 * @brief   semaphore lock
 * @param   sprt_sem
 * @retval  none
 * @note    if it has been locked, schedule another thread
 */
void sema_down(struct semaphore *sprt_sem)
{
    if (!mrt_current)
        return;

    while (sema_is_locked(sprt_sem))
        schedule_thread();
    
    atomic_dec(&sprt_sem->sgrt_atc);
}

/*!
 * @brief   semaphore lock
 * @param   sprt_sem
 * @retval  none
 * @note    if it has been locked, return directly
 */
kint32_t sema_down_try_lock(struct semaphore *sprt_sem)
{
    if (!mrt_current)
        return -ER_FORBID;

    if (sema_is_locked(sprt_sem))
        return -ER_BUSY;
    
    atomic_dec(&sprt_sem->sgrt_atc);

    return ER_NORMAL;
}

/*!
 * @brief   semaphore unlock
 * @param   sprt_sem
 * @retval  none
 * @note    none
 */
void sema_up(struct semaphore *sprt_sem)
{
    if (!mrt_current || !sema_is_locked(sprt_sem))
        return;
    
    atomic_inc(&sprt_sem->sgrt_atc);
}

/*!< end of file */
