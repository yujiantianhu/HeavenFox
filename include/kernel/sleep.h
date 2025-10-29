/*
 * Kernel Time Defines
 *
 * File Name:   sleep.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.01
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __KERNEL_SLEEP_H_
#define __KERNEL_SLEEP_H_

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/time.h>
#include <kernel/kernel.h>

/*!< The defines */
struct thread;

#define KTIME_EVENT_JIFFIES                     (0)
#define KTIME_EVENT_HRTIME                      (1)

/*!< Timer event */
struct ktime_event
{
    /*!< Thread pointer */
    struct thread *sptr_cur;

    union {
        struct timer_list sgtc_tm;
        struct hrtimer_list sgtc_hrtm;
    } u;
    
    kuint32_t type;
};

/*!< The functions */
extern void schedule_timeout(kutime_t count);
extern void khrtime_schedule(khrtime_t tick);

extern void thread_sleep_quit(struct ktime_event *sptr_event);

extern kuint32_t sleep(kuint32_t seconds);
extern kuint32_t msleep(kuint32_t milseconds);
extern kint32_t usleep(kuint32_t useconds);

extern kuint32_t sleep_tick(kutime_t tick);
extern void khrt_sleep_tick(khrtime_t tick);

#ifdef __cplusplus
    }
#endif

#endif /* __KERNEL_SLEEP_H_ */
