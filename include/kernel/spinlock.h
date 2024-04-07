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

/*!< The includes */
#include <kernel/kernel.h>

/*!< The defines */
typedef struct spin_lock
{
	kuint32_t count;

} srt_spin_lock_t;

#define DECLARE_SPIN_LOCK(lock)						srt_spin_lock_t lock = { .count = 0 }
#define mrt_spin_lock_init(lock)					do { (lock)->count = 0; } while (0)
#define mrt_spin_is_locked(lock)					(!!(lock)->count)

#define mrt_spin_lock(lock)	\
	do {	\
		while (mrt_spin_is_locked(lock));	\
		COUNT_INC((lock)->count);	\
	} while (0)

#define mrt_spin_trylock(lock)	\
	do {	\
		if (mrt_spin_is_locked(lock))	\
			return;	\
		COUNT_INC((lock)->count);	\
	} while (0)

#define mrt_spin_unlock(lock)						COUNT_DEC((lock)->count)

/*!< for SMP (if is single core, do not need to use them)*/
#define mrt_spin_lock_irq(lock)	\
	do {	\
		mrt_spin_lock(lock);	\
		mrt_disable_global_irq();	\
	} while (0)

#define mrt_spin_unlock_irq(lock)	\
	do {	\
		mrt_spin_unlock(lock);	\
		mrt_enable_global_irq();	\
	} while (0)

/*!< The functions */
TARGET_EXT void spin_lock_init(struct spin_lock *sprt_lock);
TARGET_EXT void spin_lock(struct spin_lock *sprt_lock);
TARGET_EXT void spin_unlock(struct spin_lock *sprt_lock);
TARGET_EXT ksint32_t spin_trylock(struct spin_lock *sprt_lock);

#endif /* __SPIN_LOCK_H */
