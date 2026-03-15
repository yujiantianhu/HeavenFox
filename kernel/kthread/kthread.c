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
#include <kernel/preempt.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/instance.h>
#include <kernel/spinlock.h>

/*!< The defines */
#define KERL_THREAD_STACK_SIZE                          THREAD_STACK_PAGE(2)   /*!< 2 page (8 kbytes) */

/*!< The globals */
// kuint32_t g_sched_flag = false;

static kuint8_t g_kthread_log_buffer[4096];

/*!< API functions */
/*!
 * @brief	kernel thread scheduler
 * @param  	args: timer
 * @retval 	none
 * @note   	check priority and time slice, set sptr_sched->sched_flag to true or false
 */
static void kthread_schedule_timeout(kuint32_t args)
{
    struct timer_list *sptr_tim = (struct timer_list *)args;
    kuint32_t cpuid = get_cpu_id();
    struct spin_lock *sptr_lock = scheduler_cpu_lock(cpuid);
    struct percpu_sched_data *sptr_sched = &sgtc_sched_data[cpuid];
    struct thread *sptr_work, *sptr_ready;
    kuint32_t work_prio, next_prio;
    kutype_t flags;

#if CONFIG_SMP
    /*!< balance scheduler */
    if (check_scheduler_load() >= 0)
        wake_up_migration();
#endif

    /*!< disable global scheduler */
    spin_lock_irqsave(sptr_lock, &flags);
    sptr_work = mr_current;
    
    /*!< --------------------------------------------------------- */
    /*!< reduce time-slice */
    if (sptr_work->expires)
        sptr_work->expires--;

    /*!< if not in thread context or preempt is disable */
    if (mr_preempt_cnt() > 1)
        goto END;
    
    /*!< --------------------------------------------------------- */
    /*!< check priority */
    sptr_ready = get_first_ready_thread(cpuid);
    if (!sptr_ready)
        goto END;

    work_prio = thread_get_rt_priority(sptr_work->sptr_attr);
    next_prio = thread_get_rt_priority(sptr_ready->sptr_attr);
   
#if CONFIG_PREEMPT
    /*!< there is a higher priority thread ready, or time slice is zero && the same priority thread ready */
    if (__THREAD_IS_LOW_PRIO(work_prio, next_prio) ||
        (!sptr_work->expires && (work_prio == next_prio)))
        sptr_sched->sched_flag = true;

#else
    /*!< when the time slice is not exhuasted, current cannot be preempted */
    if (!sptr_work->expires &&
        (__THREAD_IS_LOW_PRIO(work_prio, next_prio) ||
        (work_prio == next_prio)))
        sptr_sched->sched_flag = true;
#endif
    
END:
    spin_unlock_irqrestore(sptr_lock, flags);
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
    kuint64_t cur_time = JIFFIES_COUNT();

    if (systime != cur_time)
    {
        systime = cur_time;
        msecs_to_timeclock(&sgtc_systime_clock, jiffies_to_msecs(systime));
    }
}

/*!
 * @brief	manage sleep thread
 * @param  	none
 * @retval 	none
 * @note   	none
 */
static void kthread_deal_sleep(void)
{
    struct thread *sptr_thread = mr_nullptr;
    kuint32_t cpuid = get_cpu_id();

    while ((sptr_thread = next_sleep_thread(cpuid, sptr_thread)))
        thread_quit(sptr_thread->tid);
}

/*!
 * @brief	manage zombie thread
 * @param  	none
 * @retval 	none
 * @note   	none
 */
static void kthread_kill_zombie(void)
{
    struct thread *sptr_thread, *sptr_next;
    kuint32_t cpuid = get_cpu_id();

    for (sptr_thread = get_first_zombie_thread(cpuid), sptr_next = mr_nullptr;
         sptr_thread;
         sptr_thread = sptr_next)
    {
        sptr_next = next_zombie_thread(cpuid, sptr_thread);
        thread_destroy(sptr_thread->tid);
    }
}

/*!
 * @brief	kernel thread entry
 * @param  	args: NULL normally
 * @retval 	none
 * @note   	create all kernel thread
 */
static void *kthread_entry(void *args)
{
    struct timer_list *sptr_tim;
    tid_t tid = mr_current->tid;
    kuint32_t cpuid = get_cpu_id();

    mr_preempt_disable();
    thread_set_self_name_args("kthread/%u", cpuid);

#if CONFIG_SCHED_SLICE
    sptr_tim = kzalloc(sizeof(*sptr_tim), GFP_KERNEL);
    if (!isValid(sptr_tim))
    {
        print_err("kthread: allocate kthread timer error!\r\n");
        while (1);
    }

    setup_timer(sptr_tim, kthread_schedule_timeout, (kuint32_t)sptr_tim);
    sptr_tim->expires = jiffies + 1;
    add_timer(sptr_tim);
#endif

    print_info("%s (cpu: %d) is enter, which tid is: %d\r\n", __FUNCTION__, cpuid, tid);
    mr_preempt_enable();

    /*!< just for master core */
    if (cpuid == CONFIG_CORE_MASTER)
    {
        /* platform initcall */
        run_platform_initcall();
        print_info("platform initialization finished\r\n");

        term_init();                        /*!< create term task */
        kmemp_init();                       /*!< create kmemp task */
    }
    else
    {
    }

    ksoftirqd_init();                       /*!< create ksoftirqd task */
    kworker_init();                         /*!< create kworker task */
    migration_init();                       /*!< create migration task */

    if (cpuid == CONFIG_CORE_MASTER)
    {
#if CONFIG_SMP
        /*!< wake up another cpu */
        smp_slave_init();
#endif

        /*!< build application */
        init_proc_init();                   /*!< create init task */

#ifdef CONFIG_TEST
        debug_init();                       /*!< create debug test task */
#endif
    }

    print_info("kernel thread initial finished\r\n");

    for (;;)
    {
        kthread_deal_sleep();               /*!< deal with sleep thread */
        kthread_kill_zombie();              /*!< kill zombie thread */
        
        /*!< private */
        if (cpuid == CONFIG_CORE_MASTER)
        {
            kthread_systime_record();

            /*!< Print logs */
            io_stream_logs_print(g_kthread_log_buffer, sizeof(g_kthread_log_buffer));
        }

        /*!< Sleep for a while */
        msleep(103);
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
    struct thread_attr sgtc_attr = {};

	sgtc_attr.detachstate = THREAD_CREATE_JOINABLE;
	sgtc_attr.inheritsched	= THREAD_INHERIT_SCHED;
	sgtc_attr.schedpolicy = THREAD_SCHED_FIFO;

    /*!< thread stack */
	thread_attr_setstacksize(&sgtc_attr, KERL_THREAD_STACK_SIZE);
    /*!< lowest priority */
	thread_set_priority(&sgtc_attr, THREAD_PROTY_KERNEL);
    /*!< default time slice */
    thread_set_time_slice(&sgtc_attr, THREAD_TIME_DEFAULT);

    /*!< bind cpu affinity */
    thread_set_cpuaffinity(&sgtc_attr, CPU_AFFINITY_SINGEL(get_cpu_id()));

    /*!< register thread */
    return kernel_thread_base_create(&sgtc_attr, kthread_entry, mr_nullptr);
}

/*!< end of file */
