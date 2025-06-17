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
#include <platform/irq/fwk_irq_types.h>
#include <kernel/kernel.h>
#include <kernel/spinlock.h>
#include <kernel/sched.h>

/*!< The defines */


/*!< The functions */


/*!< API functions */
/*!
 * @brief   initial spin lock
 * @param   sptr_lock
 * @retval  none
 * @note    set count = 0
 */
void spin_lock_init(struct spin_lock *sptr_lock)
{
    if (isValid(sptr_lock))
        ATOMIC_SET(&sptr_lock->sgtc_atc, 0);
}

/*!
 * @brief   spin lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, schedule another thread
 */
void spin_lock(struct spin_lock *sptr_lock)
{
    kutype_t flags;

    while (spin_is_locked(sptr_lock));
    
    local_irq_save(&flags);

    mr_preempt_disable();
    atomic_inc(&sptr_lock->sgtc_atc);
    local_irq_restore(&flags);
}

/*!
 * @brief   spin unlock
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void spin_unlock(struct spin_lock *sptr_lock)
{
    kutype_t flags;

    local_irq_save(&flags);
    if (!spin_is_locked(sptr_lock))
    {
        local_irq_restore(&flags);
        return;
    }

    atomic_dec(&sptr_lock->sgtc_atc);
    mr_preempt_enable();
    local_irq_restore(&flags);
}

/*!
 * @brief   try spin lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, return right away
 */
kint32_t spin_try_lock(struct spin_lock *sptr_lock)
{
    kutype_t flags;

    local_irq_save(&flags);
    if (spin_is_locked(sptr_lock))
    {
        local_irq_restore(&flags);
        return -ER_LOCKED;
    }

    mr_preempt_disable();
    atomic_inc(&sptr_lock->sgtc_atc);
    local_irq_restore(&flags);
    
    return ER_NORMAL;
}

/*!< for SMP (if is single core, do not need to use them) */
/*!
 * @brief   spin lock and disable irq
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void spin_lock_irq(struct spin_lock *sptr_lock)
{
    while (spin_is_locked(sptr_lock));
    
    mr_disable_cpu_irq();
    mr_preempt_disable();
    mr_barrier();

    atomic_inc(&sptr_lock->sgtc_atc);
}

/*!
 * @brief   try spin lock and disable irq
 * @param   sptr_lock
 * @retval  1: lock success; 0: lock fail
 * @note    none
 */
kint32_t spin_try_lock_irq(struct spin_lock *sptr_lock)
{
    if (spin_is_locked(sptr_lock))
        return -ER_LOCKED;

    mr_disable_cpu_irq();
    mr_preempt_disable();
    mr_barrier();
    
    atomic_inc(&sptr_lock->sgtc_atc);
    
    return ER_NORMAL;
}

/*!
 * @brief   spin unlock and enable irq
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void spin_unlock_irq(struct spin_lock *sptr_lock)
{
    if (!spin_is_locked(sptr_lock))
        return;
    
    spin_unlock(sptr_lock);
    mr_barrier();
    mr_enable_cpu_irq();
}

/*!
 * @brief   spin lock and save current irq status
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void spin_lock_irqsave(struct spin_lock *sptr_lock)
{
    while (spin_is_locked(sptr_lock));
    
    sptr_lock->flag = __get_cpsr();
    mr_disable_cpu_irq();

    mr_barrier();
    mr_preempt_disable();
    atomic_inc(&sptr_lock->sgtc_atc);
}

/*!
 * @brief   try spin lock and save current irq status
 * @param   sptr_lock
 * @retval  1: lock success; 0: lock fail
 * @note    none
 */
kint32_t spin_try_lock_irqsave(struct spin_lock *sptr_lock)
{
    if (spin_is_locked(sptr_lock))
        return -ER_LOCKED;

    sptr_lock->flag = __get_cpsr();
    mr_disable_cpu_irq();

    mr_barrier();
    mr_preempt_disable();
    atomic_inc(&sptr_lock->sgtc_atc);

    return ER_NORMAL;
}

/*!
 * @brief   spin unlock and restore irq status
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void spin_unlock_irqrestore(struct spin_lock *sptr_lock)
{
    if (!spin_is_locked(sptr_lock))
        return;
    
    spin_unlock(sptr_lock);
    mr_barrier();

    /*!< bit4 ~ bit0 is mode bit, which are not equaled to 0 */
    if (!(sptr_lock->flag & CPSR_BIT_I))
        mr_enable_cpu_irq();

    sptr_lock->flag = 0;
}

/*!
 * @brief   spin lock and disable softirq
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void spin_lock_bh(struct spin_lock *sptr_lock)
{
    local_bh_disable();
    spin_lock(sptr_lock);
}

/*!
 * @brief   spin unlock and enable softirq
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void spin_unlock_bh(struct spin_lock *sptr_lock)
{
    spin_unlock(sptr_lock);
    local_bh_enable();
}

/*!< end of file */
