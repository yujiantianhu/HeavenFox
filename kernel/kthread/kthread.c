/*
 * Kernel Thread Instance (kernel task) Interface
 *
 * File Name:   kthread.c
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
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/instance.h>
#include <kernel/spinlock.h>

/*!< The defines */
#define KERL_THREAD_STACK_SIZE                          THREAD_STACK_PAGE(1)   /*!< 1 page (4 kbytes) */

/*!< The globals */
extern kuint32_t g_asm_sched_flag;

static struct thread_attr sgtc_kthread_attr;
static kuint32_t g_kthread_stack[KERL_THREAD_STACK_SIZE];
static struct timer_list sgtc_kthread_timer;
static struct spin_lock sgtc_kthread_spinlock;

/*!< API functions */
/*!
 * @brief	kernel thread scheduler
 * @param  	args: timer
 * @retval 	none
 * @note   	check priority and time slice, set g_asm_sched_flag to true or false
 */
static void kthread_schedule_timeout(kuint32_t args)
{
    struct timer_list *sptr_tim = (struct timer_list *)args;
    struct thread *sptr_work, *sptr_ready;
    kuint32_t work_prio, next_prio;

    sptr_work = mr_current;
    
    /*!< --------------------------------------------------------- */
    /*!< check time slice (when the time slice is not exhuasted, current cannot be preempted) */
    if (mr_time_before(jiffies, sptr_work->expires))
        goto END;

    /*!< automatic tracking of time-slice */
    sptr_work->expires = ((JIFFIES_MAX - jiffies) <= THREAD_PREEMPT_PERIOD) ? 0 : jiffies;
    
    /*!< --------------------------------------------------------- */
    /*!< check priority */
    sptr_ready = get_first_ready_thread();
    if (!sptr_ready)
        goto END;

    work_prio = thread_get_priority(sptr_work->sptr_attr);
    next_prio = thread_get_priority(sptr_ready->sptr_attr);
   
    /*!< there is a higher priority thread ready */
    if (__THREAD_IS_LOW_PRIO(work_prio, next_prio))
        g_asm_sched_flag = true;
    
END:
    mod_timer(sptr_tim, jiffies + msecs_to_jiffies(THREAD_PREEMPT_PERIOD));
}

/*!
 * @brief	record system run ticks
 * @param  	none
 * @retval 	none
 * @note   	none
 */
static void kthread_systime_record(void)
{
    static kutime_t systime = 0;

    if (systime != jiffies)
    {
        msecs_to_timeclock(&sgtc_systime_clock, jiffies_to_msecs(systime));
        systime = SYS_RUNTICK;
    }
}

/*!
 * @brief	manage zombie thread
 * @param  	none
 * @retval 	none
 * @note   	none
 */
static void kthread_kill_zombie(void)
{
    struct thread *sptr_thread = mr_nullptr;

    while ((sptr_thread = next_sleep_thread(sptr_thread)))
        thread_destory(sptr_thread->tid);
}

/*!
 * @brief	kernel thread entry
 * @param  	args: NULL normally
 * @retval 	none
 * @note   	create all kernel thread
 */
static void *kthread_entry(void *args)
{
    struct timer_list *sptr_tim = &sgtc_kthread_timer;
    tid_t tid = mr_current->tid;

    mr_preempt_disable();
    
    thread_set_self_name(__FUNCTION__);
    spin_lock_init(&sgtc_kthread_spinlock);

#if CONFIG_PREEMPT
    setup_timer(sptr_tim, kthread_schedule_timeout, (kuint32_t)sptr_tim);
    sptr_tim->expires = jiffies + msecs_to_jiffies(THREAD_PREEMPT_PERIOD);
    add_timer(sptr_tim);
#endif

    print_info("%s is enter, which tid is: %d\r\n", __FUNCTION__, tid);
    mr_preempt_enable();

    /* platform initcall */
    run_platform_initcall();
    print_info("platform initialization finished\r\n");

    term_init();                            /*!< create term task */
    kworker_init();                         /*!< create kworker task */

    /*!< build application */
    init_proc_init();                       /*!< create init task */

    for (;;)
    {
        kthread_systime_record();
        kthread_kill_zombie();              /*!< kill zombie thread */

        kprintf();
        msleep(50);
    }

    return args;
}

/*!
 * @brief	create kernel thread
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t kthread_init(void)
{
    struct thread_attr *sptr_attr = &sgtc_kthread_attr;

	sptr_attr->detachstate = THREAD_CREATE_JOINABLE;
	sptr_attr->inheritsched	= THREAD_INHERIT_SCHED;
	sptr_attr->schedpolicy = THREAD_SCHED_FIFO;

    /*!< thread stack */
	thread_set_stack(sptr_attr, mr_nullptr, g_kthread_stack, sizeof(g_kthread_stack));
    /*!< lowest priority */
	thread_set_priority(sptr_attr, THREAD_PROTY_KERNEL);
    /*!< default time slice */
    thread_set_time_slice(sptr_attr, THREAD_TIME_DEFUALT);

    /*!< register thread */
    return kernel_thread_base_create(sptr_attr, kthread_entry, mr_nullptr);
}

/*!< end of file */
