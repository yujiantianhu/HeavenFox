/*
 * Kernel Thread Instance (migration task) Interface
 *
 * File Name:   migration.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2026.03.05
 *
 * Copyright (c) 2026   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/mutex.h>
#include <kernel/instance.h>

/*!< The defines */
#define MIGRATION_THREAD_STACK_SIZE                     THREAD_STACK_PAGE(1)    /*!< 1 page (4kbytes) */

/*!< The globals */
static DECLARE_LIST_HEAD(sgtc_migration_percpus);
struct mutex_lock sgtc_migration_mutex;

/*!< API functions */
/*!
 * @brief	wake up migration thread
 * @param  	none
 * @retval 	none
 * @note   	when current cpu is idle, request migration
 */
void wake_up_migration(void)
{
    kuint32_t cpuid;
    
    mr_preempt_disable();
    cpuid = get_cpu_id();
    schedule_cpu_thread_wakeup(cpuid, &sgtc_migration_percpus);
    mr_preempt_enable();
}

/*!
 * @brief	migration thread entry
 * @param  	args: NULL normally
 * @retval 	none
 * @note   	only preempting and time slice polling can jump it out
 */
static void *migration_entry(void *args)
{
    struct mutex_lock *sptr_mutex = &sgtc_migration_mutex;

    print_info("%s (cpuid: %u) is enter, which tid is: %d\r\n", __FUNCTION__, get_cpu_id(), mr_current->tid);

    for (;;)
    {   
        schedule_self_suspend();

        mutex_lock(sptr_mutex);
        check_and_balance_scheduler();
        mutex_unlock(sptr_mutex);
    }

    return args;
}

/*!
 * @brief	create migration thread
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t migration_init(void)
{
    struct thread_attr sgtc_attr = {};
    kuint32_t cpuid = get_cpu_id();
    struct kthread_percpu *sptr_kth;

    sptr_kth = kmalloc(sizeof(*sptr_kth), GFP_KERNEL);
    if (!isValid(sptr_kth))
        return -ER_NOMEM;

    sptr_kth->cpuid = cpuid;
    init_list_head(&sptr_kth->sgtc_link);

	sgtc_attr.detachstate = THREAD_CREATE_JOINABLE;
	sgtc_attr.inheritsched = THREAD_INHERIT_SCHED;
	sgtc_attr.schedpolicy = THREAD_SCHED_FIFO;

    /*!< thread stack */
	thread_attr_setstacksize(&sgtc_attr, MIGRATION_THREAD_STACK_SIZE);
    /*!< higher priority */
	thread_set_priority(&sgtc_attr, THREAD_PROTY_KERNEL);
    /*!< default time slice */
    thread_set_time_slice(&sgtc_attr, THREAD_TIME_DEFAULT);

    /*!< bind cpu affinity */
    thread_set_cpuaffinity(&sgtc_attr, CPU_AFFINITY_SINGEL(cpuid));

    /*!< register migration thread */
    sptr_kth->tid = kernel_thread_create(-1, &sgtc_attr, migration_entry, mr_nullptr);
    if (sptr_kth >= 0)
    {
        thread_set_name_args(sptr_kth->tid, "migration/%u", cpuid);
        list_head_add_tail(&sgtc_migration_percpus, &sptr_kth->sgtc_link);

        return ER_NORMAL;
    }

    kfree(sptr_kth);
    return -ER_FAILD;
}

/*!< ------------------------------------------------------------------------- */
/*!
 * @brief   kernel thread migration init
 * @param   none
 * @retval  errno
 * @note    none
 */
static kint32_t __plat_init kernel_migration_init(void)
{
    mutex_init(&sgtc_migration_mutex);
    return ER_NORMAL;
}

/*!
 * @brief   kernel thread migration exit
 * @param   none
 * @retval  none
 * @note    none
 */
static void __plat_exit kernel_migration_exit(void)
{

}

IMPORT_KERNEL_INIT(kernel_migration_init);
IMPORT_KERNEL_EXIT(kernel_migration_exit);

/*!< end of file */
