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
 * @brief   suspend status over, wake it up
 * @param   args
 * @retval  none
 * @note    timeout function
 */
static void thread_sleep_timeout(kuint32_t args)
{
    struct thread *sptr_thread = (struct thread *)args;
    struct spin_lock *sptr_lock = scheduler_lock();

    if (spin_is_locked(sptr_lock))
		return;

    if (sptr_thread->status == NR_THREAD_SUSPEND)
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
    struct thread *sptr_thread = mr_current;
    struct spin_lock *sptr_lock = scheduler_lock();

    if (!count)
        schedule_thread();
	
	spin_lock_irqsave(sptr_lock);
    setup_timer(&sgtc_tm, thread_sleep_timeout, (kuint32_t)sptr_thread);
    mod_timer(&sgtc_tm, jiffies + count);
    spin_unlock_irqrestore(sptr_lock);
    
    /*!< suspend current thread, and schedule others */
    schedule_self_suspend();
    
    spin_lock_irqsave(sptr_lock);
    del_timer(&sgtc_tm);
    spin_unlock_irqrestore(sptr_lock);
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

/*!< end of file */
