/*
 * Kernel Thread Instance (debug task) Interface
 *
 * File Name:   debug.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.12.14
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/mutex.h>
#include <kernel/instance.h>

/*!< The defines */
#define DEBUG_THREAD_STACK_SIZE                      THREAD_STACK_PAGE(1)    /*!< 1 page (4kbytes) */

/*!< The globals */
static tid_t g_debug_tid;
static struct thread_attr sgtc_debug_attr;
static THREAD_STACK_DEFINE(g_debug_stack, DEBUG_THREAD_STACK_SIZE);

static struct mutex_lock sgtc_debug_mutex1;
static struct mutex_lock sgtc_debug_mutex2;
static struct mutex_lock sgtc_debug_mutex3;
static struct mutex_lock sgtc_debug_mutex4;
static struct mutex_lock sgtc_debug_mutex5;
static struct mutex_lock sgtc_debug_mutex6;
static kint32_t g_debug_index = 0;

/*!< API functions */
static void *debug_test1_func(void *args)
{
    struct thread *sptr_self = mr_current;
    kint32_t index = g_debug_index++;

//  thread_set_self_name("debug_test1");
    thread_set_priority(sptr_self->sptr_attr, 80 - index);

    for (;;)
    {
//      mutex_lock(&sgtc_debug_mutex1);
//      msleep(10);
//      mutex_lock(&sgtc_debug_mutex2);
//      msleep(20);
//      mutex_lock(&sgtc_debug_mutex3);
//      msleep(30);
//      mutex_lock(&sgtc_debug_mutex4);
//      msleep(40);
//      mutex_lock(&sgtc_debug_mutex5);
//      msleep(50);
        mutex_lock(&sgtc_debug_mutex6);
        msleep(100);

        printk("thread %d is running, cur_prio: %d\r\n", index, thread_get_rt_priority(sptr_self->sptr_attr));

        mutex_unlock(&sgtc_debug_mutex6);
        msleep(400);
//      mutex_unlock(&sgtc_debug_mutex5);
//      msleep(50);
//      mutex_unlock(&sgtc_debug_mutex4);
//      msleep(40);
//      mutex_unlock(&sgtc_debug_mutex3);
//      msleep(30);
//      mutex_unlock(&sgtc_debug_mutex2);
//      msleep(20);
//      mutex_unlock(&sgtc_debug_mutex1);
//      msleep(10);
    }

    return args;
}

static void *debug_test2_func(void *args)
{
    struct thread *sptr_self = mr_current;

    thread_set_self_name("debug_test2");
    thread_set_priority(sptr_self->sptr_attr, 80 - 1);

    for (;;)
    {
        mutex_lock(&sgtc_debug_mutex2);
        msleep(100);
        mutex_unlock(&sgtc_debug_mutex2);
        msleep(100);
    }

    return args;
}

static void *debug_test3_func(void *args)
{
    struct thread *sptr_self = mr_current;

    thread_set_self_name("debug_test3");
    thread_set_priority(sptr_self->sptr_attr, 80 - 2);

    for (;;)
    {
        mutex_lock(&sgtc_debug_mutex3);
        msleep(100);
        mutex_unlock(&sgtc_debug_mutex3);
        msleep(100);
    }

    return args;
}

static void *debug_test4_func(void *args)
{
    struct thread *sptr_self = mr_current;

    thread_set_self_name("debug_test4");
    thread_set_priority(sptr_self->sptr_attr, 80 - 3);

    for (;;)
    {
        mutex_lock(&sgtc_debug_mutex4);
        msleep(100);
        mutex_unlock(&sgtc_debug_mutex4);
        msleep(100);
    }

    return args;
}

static void *debug_test5_func(void *args)
{
    struct thread *sptr_self = mr_current;

    thread_set_self_name("debug_test5");
    thread_set_priority(sptr_self->sptr_attr, 80 - 4);

    for (;;)
    {
        mutex_lock(&sgtc_debug_mutex5);
        msleep(100);
        mutex_unlock(&sgtc_debug_mutex5);
        msleep(100);
    }

    return args;
}

static void *debug_test6_func(void *args)
{
    struct thread *sptr_self = mr_current;

    thread_set_self_name("debug_test6");
    thread_set_priority(sptr_self->sptr_attr, 80 - 5);

    for (;;)
    {
        mutex_lock(&sgtc_debug_mutex6);
        msleep(100);
        mutex_unlock(&sgtc_debug_mutex6);
        msleep(100);
    }

    return args;
}

/*!
 * @brief	debug thread entry
 * @param  	args: NULL normally
 * @retval 	none
 * @note   	only preempting and time slice polling can jump it out
 */
static void *debug_entry(void *args)
{
    thread_create(mr_nullptr, mr_nullptr, debug_test1_func, mr_nullptr);
    thread_create(mr_nullptr, mr_nullptr, debug_test1_func, mr_nullptr);
    thread_create(mr_nullptr, mr_nullptr, debug_test1_func, mr_nullptr);
    thread_create(mr_nullptr, mr_nullptr, debug_test1_func, mr_nullptr);
    thread_create(mr_nullptr, mr_nullptr, debug_test1_func, mr_nullptr);
    thread_create(mr_nullptr, mr_nullptr, debug_test1_func, mr_nullptr);

    mutex_init(&sgtc_debug_mutex1);
    mutex_init(&sgtc_debug_mutex2);
    mutex_init(&sgtc_debug_mutex3);
    mutex_init(&sgtc_debug_mutex4);
    mutex_init(&sgtc_debug_mutex5);
    mutex_init(&sgtc_debug_mutex6);

    for (;;)
    {   
        schedule_self_suspend();
    }

    return args;
}

/*!
 * @brief	create debug thread
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t debug_init(void)
{
    struct thread_attr *sptr_attr = &sgtc_debug_attr;

	sptr_attr->detachstate = THREAD_CREATE_JOINABLE;
	sptr_attr->inheritsched	= THREAD_INHERIT_SCHED;
	sptr_attr->schedpolicy = THREAD_SCHED_FIFO;

    /*!< thread stack */
	thread_set_stack(sptr_attr, mr_nullptr, g_debug_stack, sizeof(g_debug_stack));
    /*!< lowest priority */
	thread_set_priority(sptr_attr, THREAD_PROTY_DEFAULT);
    /*!< default time slice */
    thread_set_time_slice(sptr_attr, THREAD_TIME_DEFUALT);

    /*!< register thread */
    g_debug_tid = kernel_thread_create(-1, sptr_attr, debug_entry, mr_nullptr);
    if (g_debug_tid >= 0)
    {
        thread_set_name(g_debug_tid, "debug");
        return ER_NORMAL;
    }

    return -ER_FAILD;
}

/*!< end of file */
