/*
 * Spin Lock Interface Defines
 *
 * File Name:   spinlock.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.05
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __SPIN_LOCK_H
#define __SPIN_LOCK_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/atomic_types.h>
#include <kernel/kernel.h>

/*!< The defines */
#define CONFIG_SPIN_LOCK_ASSERT                     1

typedef struct spin_lock
{
    kutype_t owner;
    struct atomic sgtc_atc;

#if CONFIG_SPIN_LOCK_ASSERT
    kchar_t *__file;
    kuint32_t __line;
    kchar_t *__function;
#endif

} srt_spin_lock_t;

#if CONFIG_SPIN_LOCK_ASSERT
#define DECLARE_SPIN_LOCK(lock)	\
    struct spin_lock lock = { .owner = 0, .sgtc_atc = ATOMIC_INIT(), .__file = mr_nullptr, .__line = 0, .__function = mr_nullptr }

#define SPIN_LOCK_INIT()	\
    { .owner = 0, .sgtc_atc = ATOMIC_INIT(), .__file = mr_nullptr, .__line = 0, .__function = mr_nullptr }

#else
#define DECLARE_SPIN_LOCK(lock)	\
    struct spin_lock lock = { .owner = 0, .sgtc_atc = ATOMIC_INIT() }

#define SPIN_LOCK_INIT()	\
    { .owner = 0, .sgtc_atc = ATOMIC_INIT() }
#endif

/*!< The functions */
extern void spin_lock_init(struct spin_lock *sptr_lock);
extern void spin_lock(struct spin_lock *sptr_lock);
extern void spin_unlock(struct spin_lock *sptr_lock);
extern kint32_t spin_try_lock(struct spin_lock *sptr_lock);
extern void spin_lock_irq(struct spin_lock *sptr_lock);
extern kint32_t spin_try_lock_irq(struct spin_lock *sptr_lock);
extern void spin_unlock_irq(struct spin_lock *sptr_lock);
extern void spin_lock_irqsave(struct spin_lock *sptr_lock, kutype_t *flags);
extern kint32_t spin_try_lock_irqsave(struct spin_lock *sptr_lock, kutype_t *flags);
extern void spin_unlock_irqrestore(struct spin_lock *sptr_lock, kutype_t flags);
extern void spin_lock_bh(struct spin_lock *sptr_lock);
extern void spin_unlock_bh(struct spin_lock *sptr_lock);

/*!< API functions */
#if CONFIG_SPIN_LOCK_ASSERT
extern void spin_lock_assert(struct spin_lock *sptr_lock, const kchar_t *__file, kuint32_t __line, const kchar_t *__function);
extern void spin_unlock_assert(struct spin_lock *sptr_lock);
extern void spin_lock_irqsave_assert(struct spin_lock *sptr_lock, kutype_t *flags, 
                    const kchar_t *__file, kuint32_t __line, const kchar_t *__function);
extern void spin_unlock_irqrestore_assert(struct spin_lock *sptr_lock, kutype_t flags);

#else
/*!
 * @brief   spin lock
 * @param   sptr_lock
 * @retval  none
 * @note    if it has been locked, waitting for unlocking
 */
static inline void spin_lock_assert(struct spin_lock *sptr_lock, const kchar_t *__file, kuint32_t __line, const kchar_t *__function)
{
    spin_lock(sptr_lock);
}

/*!
 * @brief   spin unlock
 * @param   sptr_lock
 * @retval  none
 * @note    none
 */
static inline void spin_unlock_assert(struct spin_lock *sptr_lock)
{
    spin_unlock(sptr_lock);
}

/*!
 * @brief   spin lock and save current irq status
 * @param   sptr_lock, flags
 * @retval  none
 * @note    just call spin_lock_irqsave
 */
static inline void spin_lock_irqsave_assert(struct spin_lock *sptr_lock, kutype_t *flags, 
                    const kchar_t *__file, kuint32_t __line, const kchar_t *__function)
{
    spin_lock_irqsave(sptr_lock, flags);
}

/*!
 * @brief   spin unlock and restore irq status
 * @param   sptr_lock
 * @retval  none
 * @note    just call spin_unlock_irqrestore
 */
static inline void spin_unlock_irqrestore_assert(struct spin_lock *sptr_lock, kutype_t flags)
{
    spin_unlock_irqrestore(sptr_lock, flags);
}
#endif

/*!
 * @brief   check if spinlock is locked
 * @param   sptr_lock
 * @retval  locked(true) / unlocked(false)
 * @note    none
 */
static inline kbool_t spin_is_locked(struct spin_lock *sptr_lock)
{
    return !!atomic_get_val(&sptr_lock->sgtc_atc);
}

#ifdef __cplusplus
    }
#endif

#endif /* __SPIN_LOCK_H */
