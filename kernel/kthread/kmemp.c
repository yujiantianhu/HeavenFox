/*
 * Kernel Thread Instance (kmemp task) Interface
 *
 * File Name:   kmemp.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.06.08
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/instance.h>
#include <kernel/workqueue.h>

/*!< The defines */
#define KMEMP_THREAD_STACK_SIZE                         THREAD_STACK_PAGE(1)    /*!< 1/2 page (2 kbytes) */

/*!< The globals */
static tid_t g_kmemp_tid;
static struct thread_attr sgtc_kmemp_attr;
static THREAD_STACK_DEFINE(g_kmemp_stack, KMEMP_THREAD_STACK_SIZE);

static struct fwk_memp_list sgtc_kmemp_list_head = {};
static struct spin_lock sgtc_kmemp_lock = SPIN_LOCK_INIT();

#define IS_KMEMP_LIST_EMPTY()                           (!sgtc_kmemp_list_head.sptr_next)

/*!< API functions */
/*!
 * @brief	wake up kmemp thread
 * @param  	none
 * @retval 	none
 * @note   	none
 */
void wake_up_kmemp_thread(void)
{
    schedule_thread_wakeup(g_kmemp_tid);
}

/*!
 * @brief	add list to head
 * @param  	sptr_memp (sptr_memp->sptr_next)
 * @retval 	none
 * @note   	none
 */
void kmemp_list_add(struct fwk_memp_list *sptr_memp)
{
    struct spin_lock *sptr_lock = &sgtc_kmemp_lock;
    struct fwk_memp_list *sptr_head = &sgtc_kmemp_list_head;

    spin_lock_irqsave(sptr_lock);
    sptr_memp->sptr_next = sptr_head->sptr_next;
    sptr_head->sptr_next = sptr_memp;
    spin_unlock_irqrestore(sptr_lock);
}

/*!
 * @brief	move list and init head
 * @param  	none
 * @retval 	the list moved
 * @note   	none
 */
struct fwk_memp_list *kmemp_list_splice_init(void)
{
    struct spin_lock *sptr_lock = &sgtc_kmemp_lock;
    struct fwk_memp_list *sptr_head = &sgtc_kmemp_list_head;
    struct fwk_memp_list *sptr_memp;

    spin_lock_irqsave(sptr_lock);
    sptr_memp = sptr_head->sptr_next;
    sptr_head->sptr_next = mr_nullptr;
    spin_unlock_irqrestore(sptr_lock);

    return sptr_memp;
}

/*!< Loop (Get every list) */
#define foreach_kmemp_next_list(sptr_memp, sptr_temp)   \
    for (sptr_memp = kmemp_list_splice_init(), sptr_temp = sptr_memp ? sptr_memp->sptr_next : mr_nullptr;    \
         sptr_memp;     \
         sptr_memp = sptr_temp, sptr_temp = sptr_temp ? sptr_temp->sptr_next : mr_nullptr)

/*!
 * @brief	memory manage thread entry
 * @param  	args: NULL normally
 * @retval 	none
 * @note   	none
 */
static void *kmemp_entry(void *args)
{
    struct fwk_memp_list *sptr_memp, *sptr_temp;

    print_info("%s is enter, which tid is: %d\r\n", __FUNCTION__, mr_current->tid);

    for (;;)
    {
        foreach_kmemp_next_list(sptr_memp, sptr_temp)
        {
            if (sptr_memp->release)
                sptr_memp->release(sptr_memp);
        }
        
        if (IS_KMEMP_LIST_EMPTY())
            schedule_self_suspend();
    }

    return args;
}

/*!
 * @brief	create kmemp thread
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t kmemp_init(void)
{
    struct thread_attr *sptr_attr = &sgtc_kmemp_attr;

	sptr_attr->detachstate = THREAD_CREATE_JOINABLE;
	sptr_attr->inheritsched	= THREAD_INHERIT_SCHED;
	sptr_attr->schedpolicy = THREAD_SCHED_FIFO;

    /*!< thread stack */
	thread_set_stack(sptr_attr, mr_nullptr, g_kmemp_stack, sizeof(g_kmemp_stack));
    /*!< lowest priority */
	thread_set_priority(sptr_attr, THREAD_PROTY_KMEMP);
    /*!< default time slice */
    thread_set_time_slice(sptr_attr, THREAD_TIME_KMEMP);

    /*!< register thread */
    g_kmemp_tid = kernel_thread_create(-1, sptr_attr, kmemp_entry, mr_nullptr);
    if (g_kmemp_tid >= 0)
    {
        thread_set_name(g_kmemp_tid, "kmemp_entry");
        return ER_NORMAL;
    }

    return -ER_FAILD;
}

/*!< end of file */
