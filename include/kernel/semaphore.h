/*
 * Semaphore Interface Defines
 *
 * File Name:   semaphore.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.06.01
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H

/*!< The includes */
#include <common/atomic_types.h>
#include <kernel/kernel.h>

/*!< The defines */
typedef struct semaphore
{
	struct atomic sgrt_atc;

} srt_semaphore_t;

#define SEMAPHORE_INITIALIZE()	\
{ 	\
	.sgrt_atc = ATOMIC_INIT()	\
}

/*!< The functions */
TARGET_EXT void sema_init(struct semaphore *sprt_sem, kuint32_t val);
TARGET_EXT void sema_down(struct semaphore *sprt_sem);
TARGET_EXT kint32_t sema_down_try_lock(struct semaphore *sprt_sem);
TARGET_EXT void sema_up(struct semaphore *sprt_sem);

/*!< API functions */
static inline kbool_t sema_is_locked(struct semaphore *sprt_sem)
{
	return (0 == ATOMIC_READ(&sprt_sem->sgrt_atc));
}

#endif /* __SEMAPHORE_H */
