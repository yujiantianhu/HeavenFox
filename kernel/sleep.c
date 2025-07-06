/*
 * Kernel Time Interface
 *
 * File Name:   time.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.01
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/spinlock.h>

/*!< The defines */
#define KTIME_EVENT_JIFFIES                     (0)
#define KTIME_EVENT_HRTIME                      (1)

struct ktime_event
{
    struct thread *sptr_cur;

    union {
        struct timer_list sgtc_tm;
        struct hrtimer_list sgtc_hrtm;
    } u;
    
    kuint32_t type;
};

/*!< The functions */


/*!< API functions */
/*!
 * @brief   suspend status over, wake it up
 * @param   args
 * @retval  none
 * @note    timeout function
 */
static void thread_sleep_timeout(kuint32_t args)
{
    struct ktime_event *sptr_event = (struct ktime_event *)args;
    struct thread *sptr_thread = sptr_event->sptr_cur;
    kuint32_t status;

    spin_lock_irqsave(&sptr_thread->sgtc_lock);
    status = __GET_THREAD_STATE(sptr_thread);
    spin_unlock_irqrestore(&sptr_thread->sgtc_lock);

    /*!< Only schedule_thread() is finished, state will be NR_THREAD_SUSPEND */
    if (status == NR_THREAD_SUSPEND)
        schedule_thread_wakeup(sptr_thread->tid);

    spin_lock_irqsave(&sptr_thread->sgtc_lock);
    status = __GET_THREAD_STATE(sptr_thread);
    spin_unlock_irqrestore(&sptr_thread->sgtc_lock);

    /*!< Wake up failed */ 
    if ((status != NR_THREAD_READY) &&
        (status != NR_THREAD_RUNNING))
    {
        if (sptr_event->type == KTIME_EVENT_JIFFIES)
            mod_timer(&sptr_event->u.sgtc_tm, jiffies + 1);
        else
            mod_hrtimer(&sptr_event->u.sgtc_hrtm, khrtime_ticks() + MSEC_TO_HRTICK(1));
    }
}

/*!
 * @brief   timeout callback with high time tick
 * @param   tick: interval
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
void khrtime_schedule(khrtime_t tick)
{    
    struct ktime_event sgtc_event;
    struct hrtimer_list *sptr_tm = &sgtc_event.u.sgtc_hrtm;
    struct thread *sptr_cur = mr_current;

    /*!< schedule but not suspend (just add to ready list) */
    if (tick < USEC_TO_HRTICK(THREAD_SWITCH_TIME)) {
        schedule_thread();
        return;
    }
	
    sgtc_event.type = KTIME_EVENT_HRTIME;
    sgtc_event.sptr_cur = sptr_cur;
    setup_hrtimer(sptr_tm, thread_sleep_timeout, (kuint32_t)&sgtc_event);

    /*!< suspend current thread, and schedule others */
    spin_lock_irqsave(&sptr_cur->sgtc_lock);
    __SET_THREAD_TARGET_STATE(sptr_cur, NR_THREAD_SUSPEND);
    spin_unlock_irqrestore(&sptr_cur->sgtc_lock);

    mod_hrtimer(sptr_tm, khrtime_ticks() + tick);
    if (mr_likely(__GET_THREAD_TARGET_STATE(sptr_cur) == NR_THREAD_SUSPEND))
        schedule_thread();

    del_hrtimer(sptr_tm);
}

/*!
 * @brief   setup timer for sleeping
 * @param   count
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
void schedule_timeout(kutime_t count)
{
    struct ktime_event sgtc_event;
    struct timer_list *sptr_tm = &sgtc_event.u.sgtc_tm;
    struct thread *sptr_cur = mr_current;

    /*!< schedule but not suspend (just add to ready list) */
    if (!count) {
        schedule_thread();
        return;
    }
	
    sgtc_event.type = KTIME_EVENT_JIFFIES;
    sgtc_event.sptr_cur = sptr_cur;
    setup_timer(sptr_tm, thread_sleep_timeout, (kuint32_t)&sgtc_event);

    /*!< suspend current thread, and schedule others */
    spin_lock_irqsave(&sptr_cur->sgtc_lock);
    __SET_THREAD_TARGET_STATE(sptr_cur, NR_THREAD_SUSPEND);
    spin_unlock_irqrestore(&sptr_cur->sgtc_lock);

    mod_timer(sptr_tm, jiffies + count);
    schedule_thread();

    del_timer(sptr_tm);
}

/*!
 * @brief   sleep (unit: tick) by hrtimer
 * @param   tick
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
void khrt_sleep_tick(khrtime_t tick)
{
    khrtime_t expires = khrtime_ticks() + tick;

    if (mr_likely(mr_current))
    {
    #if CONFIG_ROLL_POLL
        while (expires > khrtime_ticks())
            schedule_thread();

    #else
//      if (expires > khrtime_ticks())
            khrtime_schedule(tick);
        
    #endif
    }
    else
    {
        /*!< delay(seconds); */
        while (expires > khrtime_ticks())
            mr_nop();
    }
}

/*!
 * @brief   sleep (unit: tick) by jiffies
 * @param   tick
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
kuint32_t sleep_tick(kutime_t tick)
{
    kutime_t expires = jiffies + tick;

    if (mr_likely(mr_current))
    {
    #if CONFIG_ROLL_POLL
        while (mr_time_after(expires, jiffies))
            schedule_thread();

    #else
        if (mr_time_after(expires, jiffies))
            schedule_timeout(tick);
        
    #endif
    }
    else
    {
        /*!< delay(seconds); */
        while (mr_time_after(expires, jiffies));
    }

    return (kuint32_t)tick;
}

/*!
 * @brief   sleep (unit: s)
 * @param   seconds
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
kuint32_t sleep(kuint32_t seconds)
{
    /*!< TICK_HZ >= 100, jiffies can satisfy */
//#if defined(CONFIG_HRTIMER_ENBALE) && (CONFIG_HRTIMER_ENBALE)
//    khrt_sleep_tick(SEC_TO_HRTICK(seconds));
//    return ER_NORMAL;

//#else
    return sleep_tick(secs_to_jiffies(seconds));

//#endif
}

/*!
 * @brief   sleep (unit: ms)
 * @param   milseconds
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
kuint32_t msleep(kuint32_t milseconds)
{
#if defined(CONFIG_HRTIMER_ENBALE) && (CONFIG_HRTIMER_ENBALE)
    kuint32_t period_ms = 1000U / TICK_HZ;
    kuint32_t remain_msecs = milseconds % period_ms;
    khrtime_t ticks = MSEC_TO_HRTICK(milseconds);

    /*!< Is the multiples of period_ms, or ticks are too large, use jiffies first */
    if (mr_likely(!remain_msecs) || (ticks > HRTIMER_MAX))
        return sleep_tick(msecs_to_jiffies(milseconds));

    khrt_sleep_tick(ticks);
    return ER_NORMAL;

#else
    return sleep_tick(msecs_to_jiffies(milseconds));

#endif
}

/*!
 * @brief   usleep (unit: us)
 * @param   useconds
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
kint32_t usleep(kuint32_t useconds)
{
#if defined(CONFIG_HRTIMER_ENBALE) && (CONFIG_HRTIMER_ENBALE)
    kuint32_t period_us = 1000000U / TICK_HZ;
    kuint32_t remain_usecs = useconds % period_us;
    khrtime_t ticks = USEC_TO_HRTICK(useconds);

    /*!< Is the multiples of period_us, or ticks are too large, use jiffies first */
    if (mr_likely(!remain_usecs) || (ticks > HRTIMER_MAX))
        return sleep_tick(usecs_to_jiffies(useconds));

    khrt_sleep_tick(ticks);
    return ER_NORMAL;

#else
    return sleep_tick(usecs_to_jiffies(useconds));

#endif
}

/*!< end of file */
