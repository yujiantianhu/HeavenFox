/*
 * Spin Lock Interface
 *
 * File Name:   spinlock.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.05
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <kernel/kernel.h>
#include <kernel/spinlock.h>
#include <kernel/sched.h>

/*!< The defines */


/*!< The functions */


/*!< API functions */
/*!
 * @brief   initial spin lock
 * @param   sprt_lock
 * @retval  none
 * @note    set count = 0
 */
void spin_lock_init(struct spin_lock *sprt_lock)
{
    if (mrt_isValid(sprt_lock))
        sprt_lock->count = 0;
}

/*!
 * @brief   spin lock
 * @param   sprt_lock
 * @retval  none
 * @note    if it has been locked, schedule another thread
 */
void spin_lock(struct spin_lock *sprt_lock)
{
    while (mrt_spin_is_locked(sprt_lock));
    
    mrt_preempt_disable();
    COUNT_INC(sprt_lock->count);
}

/*!
 * @brief   spin unlock
 * @param   sprt_lock
 * @retval  none
 * @note    none
 */
void spin_unlock(struct spin_lock *sprt_lock)
{
    COUNT_DEC(sprt_lock->count);
    mrt_preempt_enable();
}

/*!
 * @brief   try spin lock
 * @param   sprt_lock
 * @retval  none
 * @note    if it has been locked, return right away
 */
ksint32_t spin_trylock(struct spin_lock *sprt_lock)
{
    if (mrt_spin_is_locked(sprt_lock))
        return -Ert_isExisted;

    mrt_preempt_disable();
    COUNT_INC(sprt_lock->count);
    return 0;
}

/*!< end of file */
