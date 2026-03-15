/*
 * Kernel Generic Interface Defines
 *
 * File Name:   kernel.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.09
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __KERNEL_H
#define __KERNEL_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/generic.h>
#include <common/list_types.h>
#include <configs/configs.h>
#include <boot/core.h>
#include <kernel/asm_text.h>

/*!< The defines */
#define THREAD_USER										(0x1)	/*!< user thread */

/*!< Thread type */
typedef kint32_t tid_t;

/*!<
 * status move:
 *	ready ---> running
 *	running ---> ready/suspend/sleep
 *	suspend ---> ready
 *	sleep ---> ready/suspend (need to be awaked by another thread)
 */
enum __ERT_THREAD_BASIC_STATUS
{
    NR_THREAD_NONE = 0,
    
    /*!< running status: thread is running  */
    NR_THREAD_RUNNING,

    /*!< ready status: thread is waitting for running */
    NR_THREAD_READY,

    /*!<
     * suspend status: when thread is in delay or message-blocking stage, it is suspended;
     * and returns to the ready state when the delay ends and the message is normal 
     */
    NR_THREAD_SUSPEND,

    /*!<
     * sleep status: thread is sleeping, e.g. thread is completely dormant, 
     * and even if the ready condition is met, it will not be inserted into the ready list Can be awakened */
    NR_THREAD_SLEEP,

    /*!< zombie list: thread will be destroyed */
    NR_THREAD_ZOMBIE,

    /*!< numbers */
    NR_THREAD_STATUS_MAX
};

enum __ERT_THREAD_SIGNALS
{
    NR_THREAD_SIG_NORMAL = NR_THREAD_STATUS_MAX,
    NR_THREAD_SIG_WAKEUP,
    NR_THREAD_SIG_SUSPEND,
    NR_THREAD_SIG_SLEEP,
    NR_THREAD_SIG_KILL,
    NR_THREAD_SIG_INTR,

    /*!< NR_THREAD_SIG_MAX <= 32 */
    NR_THREAD_SIG_MAX,
};

/*!< Per-CPUs private thread */
struct kthread_percpu
{
    tid_t tid;
    kuint32_t cpuid;
    struct list_head sgtc_link;
};

#define schedule_cpu_thread_wakeup(_cpuid, _head)   \
    do { \
        struct kthread_percpu *_sptr_kth;  \
        foreach_list_next_entry(_sptr_kth, _head, sgtc_link)   \
        {   \
            if ((_cpuid) == (_sptr_kth)->cpuid)   \
            {   \
                schedule_thread_wakeup((_sptr_kth)->tid);  \
                break;  \
            }   \
        }   \
    } while (0)

/*!< The globals */

/*!< The functions */
extern void wake_up_migration(void);

#ifdef __cplusplus
    }
#endif

#endif /* __KERNEL_H */
