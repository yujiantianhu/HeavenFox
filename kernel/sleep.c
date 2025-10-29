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

/*!< The functions */


/*!< API functions */
/*!
 * @brief   stop sleep and destroy timer event
 * @param   sptr_event
 * @retval  none
 * @note    perhaps called by thread_destroy
 */
void thread_sleep_quit(struct ktime_event *sptr_event)
{
    sptr_event->sptr_cur->time_event = mr_nullptr;
    sptr_event->sptr_cur = mr_nullptr;

    if (sptr_event->type == KTIME_EVENT_JIFFIES)
        del_timer(&sptr_event->u.sgtc_tm);
    else
        del_hrtimer(&sptr_event->u.sgtc_hrtm);
}

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
    kuint32_t status, to_status;

    if (mr_unlikely(!sptr_thread))
        return;

    spin_lock_irqsave(&sptr_thread->sgtc_lock);
    status = __GET_THREAD_STATE(sptr_thread);
    to_status = __GET_THREAD_TARGET_STATE(sptr_thread);

    /*!< Only schedule_thread() is finished, state will be NR_THREAD_SUSPEND */
    if (status == NR_THREAD_SUSPEND)
    {
        spin_unlock_irqrestore(&sptr_thread->sgtc_lock);

        /*!< NODEV: thread has been released */
        if ((-ER_NODEV) == schedule_thread_wakeup(sptr_thread->tid))
            return;

        spin_lock_irqsave(&sptr_thread->sgtc_lock);
        status = __GET_THREAD_STATE(sptr_thread);
    }
    /*!< Timeout is triggered before scheduling, to_status is not equal to NR_THREAD_NONE */
    else if (to_status == NR_THREAD_SUSPEND)
    {
    	__SET_THREAD_TARGET_STATE(sptr_thread, NR_THREAD_NONE);
    	spin_unlock_irqrestore(&sptr_thread->sgtc_lock);

    	return;
    }

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
    sptr_cur->time_event = (void *)&sgtc_event;
    __SET_THREAD_TARGET_STATE(sptr_cur, NR_THREAD_SUSPEND);
    mr_preempt_disable();
    spin_unlock_irqrestore(&sptr_cur->sgtc_lock);

    mod_hrtimer(sptr_tm, khrtime_ticks() + tick);
    mr_preempt_enable();
    
    if (mr_likely(__GET_THREAD_TARGET_STATE(sptr_cur) == NR_THREAD_SUSPEND))
        schedule_thread();

    sptr_cur->time_event = mr_nullptr;
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
    sptr_cur->time_event = (void *)&sgtc_event;
    __SET_THREAD_TARGET_STATE(sptr_cur, NR_THREAD_SUSPEND);
    mr_preempt_disable();
    spin_unlock_irqrestore(&sptr_cur->sgtc_lock);

    mod_timer(sptr_tm, jiffies + count);
    mr_preempt_enable();

    if (mr_likely(__GET_THREAD_TARGET_STATE(sptr_cur) == NR_THREAD_SUSPEND))
        schedule_thread();

    sptr_cur->time_event = mr_nullptr;
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
#if CONFIG_ROLL_POLL
    khrtime_t expires = khrtime_ticks() + tick;

    while (expires > khrtime_ticks())
        schedule_thread();

#else
//  if (expires > khrtime_ticks())
        khrtime_schedule(tick);
#endif
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

#if CONFIG_ROLL_POLL
    while (mr_time_after(expires, jiffies))
        schedule_thread();

#else
    if (mr_time_after(expires, jiffies))
        schedule_timeout(tick);
    
#endif

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
    if (mr_likely(mr_current))
    {
    /*!< TICK_HZ >= 100, jiffies can satisfy */
// #if defined(CONFIG_HRTIMER_ENBALE) && (CONFIG_HRTIMER_ENBALE)
//      khrt_sleep_tick(SEC_TO_HRTICK(seconds));
//      return ER_NORMAL;

// #else
        return sleep_tick(secs_to_jiffies(seconds));
// #endif
    }

    delay(seconds);
    return ER_NORMAL;
}

/*!
 * @brief   sleep (unit: ms)
 * @param   milseconds
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
kuint32_t msleep(kuint32_t milseconds)
{
    if (mr_likely(mr_current))
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

    mdelay(milseconds);
    return ER_NORMAL;
}

/*!
 * @brief   usleep (unit: us)
 * @param   useconds
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
kint32_t usleep(kuint32_t useconds)
{
    if (mr_likely(mr_current))
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
    
    /*!< Scheduling not starts, use general delay */
    udelay(useconds);
    return ER_NORMAL;
}

/*!< end of file */
