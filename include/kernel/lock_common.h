/*
 * Kernel Generic Interface Defines
 *
 * File Name:   common.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2026.01.01
 *
 * Copyright (c) 2026   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __LOCK_COMMON_H
#define __LOCK_COMMON_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/atomic_types.h>
#include <kernel/kernel.h>
#include <kernel/spinlock.h>

/*!< The defines */
struct thread;

/*!< The holder of lock; one lock can only be held by one thread */
struct lock_owner 
{
    struct thread *sptr_self;
    /*!< Related to specific threads; one thread can hold multiple locks. sgtc_link are used to connect these held locks */
    struct list_head sgtc_link;         

    /*!< Protect sgtc_pendings */
    struct spin_lock sgtc_lock;         
    /*!< One lock can be contested by multiple threads, with each thread's requests forming the list */
    struct list_head sgtc_pendings;     
};

struct lock_owners 
{
    /*!< Protect sgtc_gets */
    struct spin_lock sgtc_lock;         
    /*!< Used to indicate which locks are held, as one thread may hold multiple locks */
    struct list_head sgtc_gets;         
};

/*!< Lock request and waiting; one lock can be requested by multiple threads */
struct lock_waiter 
{
    /*!< The lock being requested */
    struct lock_owner *sptr_wait;       
    /*!< Used to related to the lock to be requested; the head of the linked list is struct lock_owner::sgtc_pendings */
    struct list_head sgtc_link;         
};

/*!< The globals */

/*!< The functions */
extern struct thread *first_request_thread(struct lock_owner *sptr_owner);
extern void lock_pending_add(struct lock_owner *sptr_owner, struct thread *sptr_thread);
extern void lock_pending_del(struct lock_owner *sptr_owner, struct thread *sptr_thread);
extern void unlock_pending_del_all(struct lock_owner *sptr_owner);
extern void unlock_pending_wakeup(struct lock_owner *sptr_owner, kbool_t wake_all);
extern void unlock_pending_wakeup_nolock(struct lock_owner *sptr_owner, kbool_t wake_all);
extern kuint32_t lock_find_max_priority(struct lock_owners *sptr_owners);

extern void lock_context_init(struct lock_owner *sptr_owner);
extern void lock_context_save(struct lock_owner *sptr_owner);
extern void unlock_context_restore(struct lock_owner *sptr_owner);
extern kint32_t lock_compete(struct lock_owner *sptr_owner, kbool_t (*is_locked)(void *), void *lock, kbool_t inherit_enable);
extern kint32_t unlock_release(struct thread *sptr_self, kbool_t inherit_enable);

/*!< API functions */
/*!
 * @brief   Judge if is in pending list
 * @param   sptr_waiter
 * @retval  true (yes) or false (no)
 * @note    none
 */
static inline kbool_t in_lock_pending(struct lock_waiter *sptr_waiter)
{
    struct list_head *sptr_link = &sptr_waiter->sgtc_link;  
    return !mr_list_empty(sptr_link);
}

#ifdef __cplusplus
    }
#endif

#endif /* __LOCK_COMMON_H */
