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

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/atomic_types.h>
#include <kernel/kernel.h>

/*!< The defines */
typedef struct semaphore
{
	struct atomic sgtc_atc;

} srt_semaphore_t;

#define SEMAPHORE_INITIALIZE()	\
{ 	\
	.sgtc_atc = ATOMIC_INIT()	\
}

/*!< The functions */
extern void sema_init(struct semaphore *sptr_sem, kuint32_t val);
extern void sema_down(struct semaphore *sptr_sem);
extern kint32_t sema_down_try_lock(struct semaphore *sptr_sem);
extern void sema_up(struct semaphore *sptr_sem);

/*!< API functions */
/*!
 * @brief   check if semaphore is locked
 * @param   sptr_sem
 * @retval  locked(true) / unlocked(false)
 * @note    none
 */
static inline kbool_t sema_is_locked(struct semaphore *sptr_sem)
{
	return (0 == atomic_get_val(&sptr_sem->sgtc_atc));
}

#ifdef __cplusplus
    }
#endif

#endif /* __SEMAPHORE_H */
