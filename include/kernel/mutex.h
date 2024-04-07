/*
 * Mutex Lock Interface Defines
 *
 * File Name:   mutex.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.05
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __MUTEX_H
#define __MUTEX_H

/*!< The includes */
#include <kernel/kernel.h>

/*!< The defines */
typedef struct mutex_lock
{
	kuint32_t count;

} srt_mutex_lock_t;

#define DECLARE_MUTEX_LOCK(lock)					struct mutex_lock lock = { .count = 0 }
#define mrt_mutex_lock_init(lock)					do { (lock)->count = 0; } while (0)
#define mrt_mutex_is_locked(lock)					(!!(lock)->count)

/*!< The functions */
TARGET_EXT void mutex_init(struct mutex_lock *sprt_lock);
TARGET_EXT void mutex_lock(struct mutex_lock *sprt_lock);
TARGET_EXT void mutex_unlock(struct mutex_lock *sprt_lock);

#endif /* __MUTEX_H */
