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
struct ktime_event
{
    struct thread *sptr_cur;
    struct timer_list sgtc_tmf;
    struct timer_list sgtc_tmr;

    kutime_t remain_tick;
    kutime_t jiffies_cnt;
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
    struct thread *sptr_thread = (struct thread *)args;

    /*!< Only schedule_thread() is finished, state will be NR_THREAD_SUSPEND */
    if (sptr_thread->state == NR_THREAD_SUSPEND)
        schedule_thread_wakeup(sptr_thread->tid);
}

/*!
 * @brief   setup timer for sleeping
 * @param   count
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
void schedule_timeout(kutime_t count)
{
    struct timer_list sgtc_tm;
    struct thread *sptr_cur = mr_current;

    /*!< schedule but not suspend (just add to ready list) */
    if (!count) {
        schedule_thread();
        return;
    }
	
    setup_timer(&sgtc_tm, thread_sleep_timeout, (kuint32_t)sptr_cur);

    /*!< suspend current thread, and schedule others */
    spin_lock_irqsave(&sptr_cur->sgtc_lock);
    __SET_THREAD_STATE(sptr_cur, NR_THREAD_SUSPEND);
    spin_unlock_irqrestore(&sptr_cur->sgtc_lock);

    mod_timer(&sgtc_tm, jiffies + count);
    schedule_thread();

    del_timer(&sgtc_tm);
}

/*!
 * @brief   sleep (unit: s)
 * @param   seconds
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
kuint32_t sleep(kuint32_t seconds)
{
    kutime_t count = secs_to_jiffies(seconds);
    kutime_t expires = jiffies + count;

    if (mr_likely(mr_current))
    {
    #if CONFIG_ROLL_POLL
        while (mr_time_after(expires, jiffies))
            schedule_thread();

    #else
        if (mr_time_after(expires, jiffies))
            schedule_timeout(count);
        
    #endif
    }
    else
    {
        /*!< wait_secs(seconds); */
        while (mr_time_after(expires, jiffies));
    }

    return (kuint32_t)count;
}

/*!
 * @brief   sleep (unit: ms)
 * @param   milseconds
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
kuint32_t msleep(kuint32_t milseconds)
{
    kutime_t count = msecs_to_jiffies(milseconds);
    kutime_t expires = jiffies + count;
    
    if (mr_likely(mr_current))
    {
    #if CONFIG_ROLL_POLL
        while (mr_time_after(expires, jiffies))
            schedule_thread();

    #else
        if (mr_time_after(expires, jiffies))
            schedule_timeout(count);
        
    #endif
    }
    else
    {
        /*!< wait_msecs(milseconds); */
        while (mr_time_after(expires, jiffies));
    }

    return (kuint32_t)count;
}

/*!
 * @brief   usleep (unit: us)
 * @param   useconds
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
kint32_t usleep(kuint32_t useconds)
{
    kutime_t count = usecs_to_jiffies(useconds);
    kutime_t expires = jiffies + count;
    
    if (mr_likely(mr_current))
    {
    #if CONFIG_ROLL_POLL
        while (mr_time_after(expires, jiffies))
            schedule_thread();

    #else
        if (mr_time_after(expires, jiffies))
            schedule_timeout(count);
        
    #endif
    }
    else
    {
        /*!< wait_usecs(useconds); */
        while (mr_time_after(expires, jiffies));
    }

    return (kint32_t)count;
}

/*!
 * @brief   timeout callback in jiffies IRQ
 * @param   args: struct ktime_event *
 * @retval  none
 * @note    unused!!!
 */
static void thread_ktimeout_f(kuint32_t args)
{
    struct ktime_event *sptr_tv;
    kutime_t cur_tick;

    sptr_tv = (struct ktime_event *)args;
    cur_tick = ktime_systick();

    if (sptr_tv->remain_tick <= cur_tick)
        thread_sleep_timeout((kuint32_t)sptr_tv->sptr_cur);
    else
    {
        mod_htimer(&sptr_tv->sgtc_tmr, sptr_tv->remain_tick);
        sptr_tv->remain_tick = 0;
        sptr_tv->sgtc_tmr.expires = (JIFFIES_MAX + 1);
    }    
}

/*!
 * @brief   timeout callback with high time tick
 * @param   tick: interval
 * @retval  none
 * @note    unused!!!
 */
void schedule_ktimeout(kutime_t tick)
{    
    struct ktime_event sgtc_tv;
    struct thread *sptr_cur = mr_current;
    kutime_t jiffies_tick = SYSTICK_FREQ / TICK_HZ;
    kutime_t cur_tick, jiffies_cnt;

    /*!< Scheduling will cast 10tick */
    if (tick < 10)
        schedule_thread();

    sgtc_tv.sptr_cur = sptr_cur;

    setup_timer(&sgtc_tv.sgtc_tmf, thread_ktimeout_f, (kuint32_t)&sgtc_tv);
    setup_htimer(&sgtc_tv.sgtc_tmr, thread_sleep_timeout, (kuint32_t)sptr_cur);

    cur_tick = ktime_systick();
    jiffies_cnt = (cur_tick + tick) / jiffies_tick;
    sgtc_tv.remain_tick = (cur_tick + tick) - (jiffies_cnt * jiffies_tick);

    /*!< suspend current thread, and schedule others */
    spin_lock_irqsave(&sptr_cur->sgtc_lock);
    __SET_THREAD_STATE(sptr_cur, NR_THREAD_SUSPEND);
    spin_unlock_irqrestore(&sptr_cur->sgtc_lock);

    if (jiffies_cnt)
        mod_timer(&sgtc_tv.sgtc_tmf, jiffies + jiffies_cnt);
    else
        mod_htimer(&sgtc_tv.sgtc_tmr, cur_tick + tick);

    schedule_thread();

    del_htimer(&sgtc_tv.sgtc_tmf);
    del_htimer(&sgtc_tv.sgtc_tmr);
}

/*!< end of file */
