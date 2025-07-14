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
#define KERL_THREAD_STACK_SIZE                          THREAD_STACK_PAGE(2)   /*!< 2 page (8 kbytes) */

/*!< The globals */
kbool_t g_sched_flag = false;

static struct thread_attr sgtc_kthread_attr;
static THREAD_STACK_DEFINE(g_kthread_stack, KERL_THREAD_STACK_SIZE);
static struct timer_list sgtc_kthread_timer;
static struct spin_lock sgtc_kthread_spinlock;

static kuint8_t g_kthread_log_buffer[4096];

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
    spin_lock(&sptr_work->sgtc_lock);
    
    /*!< --------------------------------------------------------- */
    /*!< if not in thread context or preempt is disable */
    if (!sptr_work->expires)
    {
        sptr_work->expires = 1;
        mr_barrier();
    }

    /*!< reduce time slice */
    sptr_work->expires--;

    /*!< not allow preempt */
    if (mr_preempt_is_locked())
        goto END;
    
    /*!< --------------------------------------------------------- */
    /*!< check priority */
    sptr_ready = get_first_ready_thread();
    if (!sptr_ready)
        goto END;

    work_prio = thread_get_priority(sptr_work->sptr_attr);
    next_prio = thread_get_priority(sptr_ready->sptr_attr);
   
#if CONFIG_PREEMPT
    /*!< there is a higher priority thread ready, or time slice is zero && the same priority thread ready */
    if (__THREAD_IS_LOW_PRIO(work_prio, next_prio) ||
        (!sptr_work->expires && (work_prio == next_prio)))
        g_sched_flag = true;

#else
    /*!< when the time slice is not exhuasted, current cannot be preempted */
    if (!sptr_work->expires &&
        (__THREAD_IS_LOW_PRIO(work_prio, next_prio) ||
        (work_prio == next_prio)))
        g_sched_flag = true;
#endif
    
END:
    spin_unlock(&sptr_work->sgtc_lock);
    mod_timer(sptr_tim, jiffies + 1);
}

/*!
 * @brief	record system run ticks
 * @param  	none
 * @retval 	none
 * @note   	none
 */
static void kthread_systime_record(void)
{
    static kuint64_t systime = 0;

    if (systime != jiffies)
    {
        msecs_to_timeclock(&sgtc_systime_clock, jiffies_to_msecs(systime));
        systime = JIFFIES_COUNT();
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
    
    thread_set_self_name("kthread");
    spin_lock_init(&sgtc_kthread_spinlock);

#if CONFIG_SCHED_SLICE
    setup_timer(sptr_tim, kthread_schedule_timeout, (kuint32_t)sptr_tim);
    sptr_tim->expires = jiffies + 1;
    add_timer(sptr_tim);
#endif

    print_info("%s is enter, which tid is: %d\r\n", __FUNCTION__, tid);
    mr_preempt_enable();

    /* platform initcall */
    run_platform_initcall();
    print_info("platform initialization finished\r\n");

    term_init();                            /*!< create term task */
    ksoftirqd_init();                       /*!< create ksoftirqd task */
    kworker_init();                         /*!< create kworker task */
    kmemp_init();                           /*!< create kmemp task */

    /*!< build application */
    init_proc_init();                       /*!< create init task */

    print_info("%s: %d\r\n", __func__, __LINE__);

    for (;;)
    {
        kthread_systime_record();
        kthread_kill_zombie();              /*!< kill zombie thread */
        
        /*!< Print logs */
        io_stream_logs_print(g_kthread_log_buffer, sizeof(g_kthread_log_buffer));

        /*!< Sleep for a while */
        msleep(203);
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
