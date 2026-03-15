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
#include <kernel/preempt.h>
#include <kernel/spinlock.h>
#include <kernel/sched.h>
#include <term/term.h>

/*!< The defines */
#define SPIN_LOCK_OWNER_IRQ                     (0x01)
#define SPIN_LOCK_OWNER_TASK                    (0x02)

#define mr_spin_set_owner(_ptr_owner)  \
    do { \
        *(_ptr_owner) = ((kutype_t)mr_current) | (IN_INTERRUPT() ? SPIN_LOCK_OWNER_IRQ : SPIN_LOCK_OWNER_TASK);    \
        mr_smp_mb();    \
    } while (0)

#define mr_spin_clr_owner(_ptr_owner)  \
    do { \
        *(_ptr_owner) = 0;    \
        mr_smp_mb();    \
    } while (0)

/*!< The globals */
static kint32_t g_spin_lock_monitor;
static struct term_variable sgtc_spin_lock_monitor = { .name = "g_spin_lock_monitor", .var = &g_spin_lock_monitor, .num = 1 };

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
    {
        sptr_lock->owner = 0;
        atomic_set_val(&sptr_lock->sgtc_atc, 0);

#if CONFIG_SPIN_LOCK_ASSERT
        sptr_lock->__line = 0;
        sptr_lock->__file = sptr_lock->__function = mr_nullptr;
#endif
    }
}

/*!
 * @brief   spin lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, waitting for unlocking
 */
void spin_lock(struct spin_lock *sptr_lock)
{
    kutype_t flags;

    if (spin_is_locked(sptr_lock))
        g_spin_lock_monitor++;

    local_irq_save(&flags);
    while (atomic_test_and_set_val(&sptr_lock->sgtc_atc, 1));

    mr_smp_mb();
    mr_preempt_disable();
    mr_spin_set_owner(&sptr_lock->owner);
    mr_smp_mb();

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
    if (!atomic_get_val(&sptr_lock->sgtc_atc))
    {
        local_irq_restore(&flags);
        return;
    }

    mr_spin_clr_owner(&sptr_lock->owner);
    mr_preempt_enable();
    mr_barrier();
    atomic_set_val(&sptr_lock->sgtc_atc, 0);
    mr_smp_mb();

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
    if (atomic_test_and_set_val(&sptr_lock->sgtc_atc, 1))
    {
        g_spin_lock_monitor++;
        local_irq_restore(&flags);
        return -ER_LOCKED;
    }

    mr_smp_mb();
    mr_preempt_disable();
    mr_spin_set_owner(&sptr_lock->owner);
    mr_smp_mb();

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
    kutype_t flags;

    if (spin_is_locked(sptr_lock))
        g_spin_lock_monitor++;

    local_irq_save(&flags);
    while (atomic_test_and_set_val(&sptr_lock->sgtc_atc, 1));

    mr_smp_mb();
    mr_preempt_disable();
    mr_spin_set_owner(&sptr_lock->owner);
    mr_smp_mb();
}

/*!
 * @brief   try spin lock and disable irq
 * @param   sptr_lock
 * @retval  1: lock success; 0: lock fail
 * @note    none
 */
kint32_t spin_try_lock_irq(struct spin_lock *sptr_lock)
{
    kutype_t flags;

    local_irq_save(&flags);
    if (atomic_test_and_set_val(&sptr_lock->sgtc_atc, 1))
    {
        g_spin_lock_monitor++;
        local_irq_restore(&flags);
        return -ER_LOCKED;
    }

    mr_smp_mb();
    mr_preempt_disable(); 
    mr_spin_set_owner(&sptr_lock->owner);
    mr_smp_mb();
    
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
    kutype_t temp;

    local_irq_save(&temp);
    if (!atomic_get_val(&sptr_lock->sgtc_atc))
    {
        local_irq_restore(&temp);
        return;
    }
    
    mr_spin_clr_owner(&sptr_lock->owner);
    mr_preempt_enable();
    mr_barrier();
    atomic_set_val(&sptr_lock->sgtc_atc, 0);
    mr_smp_mb();

    mr_enable_cpu_irq();
}

/*!
 * @brief   spin lock and save current irq status
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void spin_lock_irqsave(struct spin_lock *sptr_lock, kutype_t *flags)
{
    /*!< record */
    if (spin_is_locked(sptr_lock))
        g_spin_lock_monitor++;

    local_irq_save(flags);
    while (atomic_test_and_set_val(&sptr_lock->sgtc_atc, 1));

    mr_smp_mb();
    mr_preempt_disable();
    mr_spin_set_owner(&sptr_lock->owner);
    mr_smp_mb();
}

/*!
 * @brief   try spin lock and save current irq status
 * @param   sptr_lock
 * @retval  1: lock success; 0: lock fail
 * @note    none
 */
kint32_t spin_try_lock_irqsave(struct spin_lock *sptr_lock, kutype_t *flags)
{
    local_irq_save(flags);
    if (atomic_test_and_set_val(&sptr_lock->sgtc_atc, 1))
    {
        g_spin_lock_monitor++;
        local_irq_restore(flags);
        return -ER_LOCKED;
    }

    mr_smp_mb();
    mr_preempt_disable();
    mr_spin_set_owner(&sptr_lock->owner);
    mr_smp_mb();

    return ER_NORMAL;
}

/*!
 * @brief   spin unlock and restore irq status
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void spin_unlock_irqrestore(struct spin_lock *sptr_lock, kutype_t flags)
{
    kutype_t temp;

    local_irq_save(&temp);
    if (!atomic_get_val(&sptr_lock->sgtc_atc))
    {
        local_irq_restore(&temp);
        return;
    }
    
    mr_spin_clr_owner(&sptr_lock->owner);
    mr_preempt_enable();
    mr_barrier();
    atomic_set_val(&sptr_lock->sgtc_atc, 0);
    mr_smp_mb();

    /*!< bit4 ~ bit0 is mode bit, which are not equaled to 0 */
    local_irq_restore(&flags);
}

#if CONFIG_SPIN_LOCK_ASSERT
/*!
 * @brief   spin lock and record function and line
 * @param   sptr_lock
 * @retval  1: lock success; 0: lock fail
 * @note    none
 */
void spin_lock_assert(struct spin_lock *sptr_lock, const kchar_t *__file, kuint32_t __line, const kchar_t *__function)
{
    kutype_t flags;

    if (spin_is_locked(sptr_lock))
        g_spin_lock_monitor++;

    local_irq_save(&flags);
    while (atomic_test_and_set_val(&sptr_lock->sgtc_atc, 1));

    mr_smp_mb();
    mr_preempt_disable();
    mr_spin_set_owner(&sptr_lock->owner);

    sptr_lock->__file = (kchar_t *)__file;
    sptr_lock->__line = __line;
    sptr_lock->__function = (kchar_t *)__function;

    mr_smp_mb();
    local_irq_restore(&flags);
}

/*!
 * @brief   spin unlock
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void spin_unlock_assert(struct spin_lock *sptr_lock)
{
    kutype_t flags;

    local_irq_save(&flags);
    if (!atomic_get_val(&sptr_lock->sgtc_atc))
    {
        local_irq_restore(&flags);
        return;
    }

    mr_spin_clr_owner(&sptr_lock->owner);
    mr_preempt_enable();

    sptr_lock->__file = sptr_lock->__function = mr_nullptr;
    sptr_lock->__line = 0;

    atomic_set_val(&sptr_lock->sgtc_atc, 0);
    mr_smp_mb();

    local_irq_restore(&flags);
}

/*!
 * @brief   spin lock and save current irq status, and record function and line
 * @param   sptr_lock
 * @retval  1: lock success; 0: lock fail
 * @note    none
 */
void spin_lock_irqsave_assert(struct spin_lock *sptr_lock, kutype_t *flags, 
                    const kchar_t *__file, kuint32_t __line, const kchar_t *__function)
{
    spin_lock_irqsave(sptr_lock, flags);

    sptr_lock->__file = (kchar_t *)__file;
    sptr_lock->__line = __line;
    sptr_lock->__function = (kchar_t *)__function;

    mr_smp_mb();
}

/*!
 * @brief   spin unlock and restore irq status
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void spin_unlock_irqrestore_assert(struct spin_lock *sptr_lock, kutype_t flags)
{
    kutype_t temp;

    local_irq_save(&temp);
    if (!atomic_get_val(&sptr_lock->sgtc_atc))
    {
        local_irq_restore(&temp);
        return;
    }
    
    mr_spin_clr_owner(&sptr_lock->owner);
    mr_preempt_enable();

    sptr_lock->__file = sptr_lock->__function = mr_nullptr;
    sptr_lock->__line = 0;

    atomic_set_val(&sptr_lock->sgtc_atc, 0);
    mr_smp_mb();

    /*!< bit4 ~ bit0 is mode bit, which are not equaled to 0 */
    local_irq_restore(&flags);
}
#endif

/*!
 * @brief   spin lock and disable softirq
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void spin_lock_bh(struct spin_lock *sptr_lock)
{
    kutype_t flags;

    if (spin_is_locked(sptr_lock))
        g_spin_lock_monitor++;

    local_irq_save(&flags);
    while (atomic_test_and_set_val(&sptr_lock->sgtc_atc, 1));

    mr_smp_mb();
    mr_preempt_disable();
    mr_local_bh_disable();
    mr_spin_set_owner(&sptr_lock->owner);
    mr_smp_mb();

    local_irq_restore(&flags);
}

/*!
 * @brief   spin unlock and enable softirq
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
void spin_unlock_bh(struct spin_lock *sptr_lock)
{
    kutype_t flags;

    local_irq_save(&flags);
    if (!atomic_get_val(&sptr_lock->sgtc_atc))
    {
        local_irq_restore(&flags);
        return;
    }

    mr_spin_clr_owner(&sptr_lock->owner);
    mr_local_bh_enable();
    mr_preempt_enable();
    mr_barrier();
    atomic_set_val(&sptr_lock->sgtc_atc, 0);
    mr_smp_mb();

    local_irq_restore(&flags);
}

/*!< ------------------------------------------------------------------------- */
/*!
 * @brief   spin_lock init
 * @param   none
 * @retval  errno
 * @note    none
 */
static kint32_t __plat_init kernel_spin_lock_init(void)
{
    struct term_variable *sptr_var = &sgtc_spin_lock_monitor;

    init_list_head(&sptr_var->sgtc_link);
    term_variable_add(sptr_var);

    return ER_NORMAL;
}

/*!
 * @brief   spin_lock exit
 * @param   none
 * @retval  none
 * @note    none
 */
static void __plat_exit kernel_spin_lock_exit(void)
{
    term_variable_del(&sgtc_spin_lock_monitor);
}

IMPORT_KERNEL_INIT(kernel_spin_lock_init);
IMPORT_KERNEL_EXIT(kernel_spin_lock_exit);

/*!< end of file */
