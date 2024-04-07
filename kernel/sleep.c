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

/*!< The defines */


/*!< The functions */


/*!< API functions */
/*!
 * @brief   suspend status over, wake it up
 * @param   args
 * @retval  none
 * @note    timeout function
 */
static void real_thread_sleep_timeout(kuint32_t args)
{
    struct kel_thread *sprt_thread = (struct kel_thread *)args;

    if (sprt_thread->status == NR_THREAD_SUSPEND)
        kel_sched_thread_wakeup(sprt_thread->tid);
}

/*!
 * @brief   setup timer for sleeping
 * @param   count
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
void schedule_for_timeout(kutime_t count)
{
    struct timer_list sgrt_tm;
    struct kel_thread *sprt_thread = mrt_current;
	
	mrt_preempt_disable();

    setup_timer(&sgrt_tm, real_thread_sleep_timeout, (kuint32_t)sprt_thread);
    mod_timer(&sgrt_tm, count);

    mrt_preempt_enable();
    
    /*!< suspend current thread, and schedule others */
    kel_sched_self_suspend();
    
    del_timer(&sgrt_tm);
}

/*!
 * @brief   sleep (unit: s)
 * @param   seconds
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
void schedule_delay(kuint32_t seconds)
{
    kutime_t count = jiffies + secs_to_jiffies(seconds);
    
    if (mrt_time_after(count, jiffies))
        schedule_for_timeout(count);
}

/*!
 * @brief   sleep (unit: ms)
 * @param   milseconds
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
void schedule_delay_ms(kuint32_t milseconds)
{
    kutime_t count = jiffies + msecs_to_jiffies(milseconds);
    
    if (mrt_time_after(count, jiffies))
        schedule_for_timeout(count);
}

/*!
 * @brief   sleep (unit: us)
 * @param   useconds
 * @retval  none
 * @note    delay and schedule (current thread may convert to suspend status)
 */
void schedule_delay_us(kuint32_t useconds)
{
    kutime_t count = jiffies + usecs_to_jiffies(useconds);
    
    if (mrt_time_after(count, jiffies))
        schedule_for_timeout(count);
}

/*!< end of file */
