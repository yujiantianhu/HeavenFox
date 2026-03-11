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
#include <kernel/rw_lock.h>
#include <kernel/instance.h>
#include <term/term.h>

/*!< The defines */
#define DEBUG_THREAD_STACK_SIZE                     THREAD_STACK_PAGE(1)    /*!< 1 page (4kbytes) */

#define DEBUG_MUTEX_ON                              (1)
#define DEBUG_RD_LOCK_ON                            (1)
#define DEBUG_WR_LOCK_ON                            (1)

#define DEBUG_TASK_TYPE                             (8)
#define DEBUG_TASK_GROUP                            (16)

struct debug_test
{
    struct mutex_lock sgtc_mutex;
    struct rw_lock sgtc_lock;

    kint32_t mutex_value;
    kint32_t rw_value;
};

#define mr_debug_test_init(_sptr_test) \
    do {    \
        mutex_init(&((_sptr_test)->sgtc_mutex));    \
        rw_lock_init(&((_sptr_test)->sgtc_lock));    \
    } while (0)

#define mr_thread_set_simple(_sptr_th, _index)  \
    do {    \
        thread_set_self_name_args("debug_test%d", _index);  \
        thread_set_priority((_sptr_th)->sptr_attr, 80 + ((_index) & (DEBUG_TASK_TYPE - 1)) + 1);    \
    } while (0)

/*!< The globals */
static tid_t g_debug_tid;
static THREAD_STACK_DEFINE(g_debug_stack, DEBUG_THREAD_STACK_SIZE);

static struct debug_test sgtc_debug_test[DEBUG_TASK_TYPE];
static kint32_t g_debug_test_stats[DEBUG_TASK_TYPE * DEBUG_TASK_GROUP];
static struct term_variable sgtc_debug_test_stats = 
{ 
    .name = "g_debug_test_stats", 
    .var = &g_debug_test_stats[0], 
    .num = DEBUG_TASK_TYPE * DEBUG_TASK_GROUP 
};

/*!< API functions */
static void *debug_test1_func(void *args)
{
    struct thread *sptr_self = mr_current;
    kint32_t index = (kint32_t)args;
    struct debug_test *sptr_tst = &sgtc_debug_test[0];
    kint32_t temp = 0;

    mr_thread_set_simple(sptr_self, index);

    for (;;)
    {
#if DEBUG_MUTEX_ON
        for (kint32_t i = 0; i < DEBUG_TASK_TYPE; i++)
        {
            mutex_lock(&sptr_tst[i].sgtc_mutex);
            sptr_tst[i].mutex_value += (i + 1);
            msleep(20);
            mutex_unlock(&sptr_tst[i].sgtc_mutex);
        }
#endif

#if DEBUG_RD_LOCK_ON
        for (kint32_t i = 0; i < DEBUG_TASK_TYPE; i++)
        {
            rd_lock(&sptr_tst[i].sgtc_lock);
            temp = sptr_tst[i].rw_value;
            temp++;
            msleep(30);
            rd_unlock(&sptr_tst[i].sgtc_lock);
        }
#endif

#if DEBUG_WR_LOCK_ON
        for (kint32_t i = 0; i < DEBUG_TASK_TYPE; i++)
        {
            wr_lock(&sptr_tst[i].sgtc_lock);
            sptr_tst[i].rw_value += (i + 1);
            msleep(45);
            wr_unlock(&sptr_tst[i].sgtc_lock);
        }
#endif

#if !DEBUG_MUTEX_ON && !DEBUG_RD_LOCK_ON && !DEBUG_WR_LOCK_ON
        msleep(100 + 20 * ((index & (DEBUG_TASK_TYPE - 1)) + 1));
#endif

        g_debug_test_stats[index]++;
    }

    return args;
}

static void *debug_test2_func(void *args)
{
    struct thread *sptr_self = mr_current;
    kint32_t index = (kint32_t)args;
    struct debug_test *sptr_tst = &sgtc_debug_test[0];
    kint32_t temp = 0;

    mr_thread_set_simple(sptr_self, index);

    for (;;)
    {
#if DEBUG_RD_LOCK_ON
        for (kint32_t i = 0; i < DEBUG_TASK_TYPE; i++)
        {
            rd_lock(&sptr_tst[i].sgtc_lock);
            temp = sptr_tst[i].rw_value;
            temp++;
            msleep(30);
            rd_unlock(&sptr_tst[i].sgtc_lock);
        }
#endif

#if DEBUG_WR_LOCK_ON
        for (kint32_t i = 0; i < DEBUG_TASK_TYPE; i++)
        {
            wr_lock(&sptr_tst[i].sgtc_lock);
            sptr_tst[i].rw_value += (i + 1);
            msleep(45);
            wr_unlock(&sptr_tst[i].sgtc_lock);
        }
#endif

#if DEBUG_MUTEX_ON
        for (kint32_t i = 0; i < DEBUG_TASK_TYPE; i++)
        {
            mutex_lock(&sptr_tst[i].sgtc_mutex);
            sptr_tst[i].mutex_value += (i + 1);
            msleep(20);
            mutex_unlock(&sptr_tst[i].sgtc_mutex);
        }
#endif

#if !DEBUG_MUTEX_ON && !DEBUG_RD_LOCK_ON && !DEBUG_WR_LOCK_ON
        msleep(100 + 20 * ((index & (DEBUG_TASK_TYPE - 1)) + 1));
#endif

        g_debug_test_stats[index]++;
    }

    return args;
}

static void *debug_test3_func(void *args)
{
    struct thread *sptr_self = mr_current;
    kint32_t index = (kint32_t)args;
    struct debug_test *sptr_tst = &sgtc_debug_test[0];
    kint32_t temp = 0;

    mr_thread_set_simple(sptr_self, index);

    for (;;)
    {
#if DEBUG_WR_LOCK_ON
        for (kint32_t i = 0; i < DEBUG_TASK_TYPE; i++)
        {
            wr_lock(&sptr_tst[i].sgtc_lock);
            sptr_tst[i].rw_value += (i + 1);
            msleep(45);
            wr_unlock(&sptr_tst[i].sgtc_lock);
        }
#endif

#if DEBUG_MUTEX_ON
        for (kint32_t i = 0; i < DEBUG_TASK_TYPE; i++)
        {
            mutex_lock(&sptr_tst[i].sgtc_mutex);
            sptr_tst[i].mutex_value += (i + 1);
            msleep(20);
            mutex_unlock(&sptr_tst[i].sgtc_mutex);
        }
#endif

#if DEBUG_RD_LOCK_ON
        for (kint32_t i = 0; i < DEBUG_TASK_TYPE; i++)
        {
            rd_lock(&sptr_tst[i].sgtc_lock);
            temp = sptr_tst[i].rw_value;
            temp++;
            msleep(30);
            rd_unlock(&sptr_tst[i].sgtc_lock);
        }
#endif

#if !DEBUG_MUTEX_ON && !DEBUG_RD_LOCK_ON && !DEBUG_WR_LOCK_ON
        msleep(100 + 20 * ((index & (DEBUG_TASK_TYPE - 1)) + 1));
#endif

        g_debug_test_stats[index]++;
    }

    return args;
}

static void *debug_test4_func(void *args)
{
    struct thread *sptr_self = mr_current;
    kint32_t index = (kint32_t)args;
    struct debug_test *sptr_tst = &sgtc_debug_test[0];
    kint32_t temp = 0;

    mr_thread_set_simple(sptr_self, index);

    for (;;)
    {
#if DEBUG_MUTEX_ON
        mutex_lock(&sptr_tst[4].sgtc_mutex);
        sptr_tst[4].mutex_value += 4;
        msleep(88);
        mutex_unlock(&sptr_tst[4].sgtc_mutex);
#endif

#if DEBUG_RD_LOCK_ON
        rd_lock(&sptr_tst[5].sgtc_lock);
        temp = sptr_tst[5].rw_value;
        temp++;
        msleep(66);
        rd_unlock(&sptr_tst[5].sgtc_lock);
#endif

#if DEBUG_WR_LOCK_ON
        wr_lock(&sptr_tst[6].sgtc_lock);
        sptr_tst[6].rw_value += 4;
        msleep(77);
        wr_unlock(&sptr_tst[6].sgtc_lock);
#endif

#if !DEBUG_MUTEX_ON && !DEBUG_RD_LOCK_ON && !DEBUG_WR_LOCK_ON
        msleep(100 + 20 * ((index & (DEBUG_TASK_TYPE - 1)) + 1));
#endif

        g_debug_test_stats[index]++;
    }

    return args;
}

static void *debug_test5_func(void *args)
{
    struct thread *sptr_self = mr_current;
    kint32_t index = (kint32_t)args;
    struct debug_test *sptr_tst = &sgtc_debug_test[0];
    kint32_t temp = 0;

    mr_thread_set_simple(sptr_self, index);

    for (;;)
    {
#if DEBUG_MUTEX_ON
        mutex_lock(&sptr_tst[5].sgtc_mutex);
        sptr_tst[5].mutex_value += 5;
        msleep(88);
        mutex_unlock(&sptr_tst[5].sgtc_mutex);
#endif

#if DEBUG_RD_LOCK_ON
        rd_lock(&sptr_tst[6].sgtc_lock);
        temp = sptr_tst[6].rw_value;
        temp++;
        msleep(66);
        rd_unlock(&sptr_tst[6].sgtc_lock);
#endif

#if DEBUG_WR_LOCK_ON
        wr_lock(&sptr_tst[4].sgtc_lock);
        sptr_tst[4].rw_value += 5;
        msleep(77);
        wr_unlock(&sptr_tst[4].sgtc_lock);
#endif

#if !DEBUG_MUTEX_ON && !DEBUG_RD_LOCK_ON && !DEBUG_WR_LOCK_ON
        msleep(100 + 20 * ((index & (DEBUG_TASK_TYPE - 1)) + 1));
#endif

        g_debug_test_stats[index]++;
    }

    return args;
}

static void *debug_test6_func(void *args)
{
    struct thread *sptr_self = mr_current;
    kint32_t index = (kint32_t)args;
    struct debug_test *sptr_tst = &sgtc_debug_test[0];
    kint32_t temp = 0;

    mr_thread_set_simple(sptr_self, index);

    for (;;)
    {
#if DEBUG_MUTEX_ON
        mutex_lock(&sptr_tst[6].sgtc_mutex);
        sptr_tst[6].mutex_value += 6;
        msleep(88);
        mutex_unlock(&sptr_tst[6].sgtc_mutex);
#endif

#if DEBUG_RD_LOCK_ON
        rd_lock(&sptr_tst[4].sgtc_lock);
        temp = sptr_tst[4].rw_value;
        temp++;
        msleep(66);
        rd_unlock(&sptr_tst[4].sgtc_lock);
#endif

#if DEBUG_WR_LOCK_ON
        wr_lock(&sptr_tst[5].sgtc_lock);
        sptr_tst[5].rw_value += 6;
        msleep(77);
        wr_unlock(&sptr_tst[5].sgtc_lock);
#endif

#if !DEBUG_MUTEX_ON && !DEBUG_RD_LOCK_ON && !DEBUG_WR_LOCK_ON
        msleep(100 + 20 * ((index & (DEBUG_TASK_TYPE - 1)) + 1));
#endif

        g_debug_test_stats[index]++;
    }

    return args;
}

static void *debug_test7_func(void *args)
{
    struct thread *sptr_self = mr_current;
    kint32_t index = (kint32_t)args;
    struct debug_test *sptr_tst = &sgtc_debug_test[0];
    kint32_t temp = 0;

    mr_thread_set_simple(sptr_self, index);

    for (;;)
    {
        for (kint32_t i = 0; i < DEBUG_TASK_TYPE; i++)
        {
#if DEBUG_MUTEX_ON
            mutex_lock(&sptr_tst[i].sgtc_mutex);
            sptr_tst[i].mutex_value += (i + 1);
            msleep(20);
            mutex_unlock(&sptr_tst[i].sgtc_mutex);
#endif

#if DEBUG_RD_LOCK_ON
            rd_lock(&sptr_tst[i].sgtc_lock);
            temp = sptr_tst[i].rw_value;
            temp++;
            msleep(30);
            rd_unlock(&sptr_tst[i].sgtc_lock);
#endif

#if DEBUG_WR_LOCK_ON
            wr_lock(&sptr_tst[i].sgtc_lock);
            sptr_tst[i].rw_value += (i + 1);
            msleep(45);
            wr_unlock(&sptr_tst[i].sgtc_lock);
#endif
        }

#if !DEBUG_MUTEX_ON && !DEBUG_RD_LOCK_ON && !DEBUG_WR_LOCK_ON
        msleep(100 + 20 * ((index & (DEBUG_TASK_TYPE - 1)) + 1));
#endif

        g_debug_test_stats[index]++;
    }

    return args;
}

static void *debug_test8_func(void *args)
{
    struct thread *sptr_self = mr_current;
    kint32_t index = (kint32_t)args;
    struct debug_test *sptr_tst = &sgtc_debug_test[0];
    kint32_t temp = 0;

    mr_thread_set_simple(sptr_self, index);

    for (;;)
    {
        for (kint32_t i = 0; i < DEBUG_TASK_TYPE; i++)
        {
#if DEBUG_WR_LOCK_ON
            wr_lock(&sptr_tst[i].sgtc_lock);
            sptr_tst[i].rw_value += (i + 1);
            msleep(45);
            wr_unlock(&sptr_tst[i].sgtc_lock);
#endif

#if DEBUG_MUTEX_ON
            mutex_lock(&sptr_tst[i].sgtc_mutex);
            sptr_tst[i].mutex_value += (i + 1);
            msleep(20);
            mutex_unlock(&sptr_tst[i].sgtc_mutex);
#endif

#if DEBUG_RD_LOCK_ON
            rd_lock(&sptr_tst[i].sgtc_lock);
            temp = sptr_tst[i].rw_value;
            temp++;
            msleep(30);
            rd_unlock(&sptr_tst[i].sgtc_lock);
#endif
        }

#if !DEBUG_MUTEX_ON && !DEBUG_RD_LOCK_ON && !DEBUG_WR_LOCK_ON
        msleep(100 + 20 * ((index & (DEBUG_TASK_TYPE - 1)) + 1));
#endif

        g_debug_test_stats[index]++;
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
    static kint32_t g_debug_index = 0;
    kint32_t retval;

    void *(*debug_test_func[DEBUG_TASK_TYPE])(void *) = 
    {
        debug_test1_func,
        debug_test2_func,
        debug_test3_func,
        debug_test4_func,
        debug_test5_func,
        debug_test6_func,
        debug_test7_func,
        debug_test8_func
    };

    for (g_debug_index = 0; g_debug_index < DEBUG_TASK_TYPE; g_debug_index++)
        mr_debug_test_init(&sgtc_debug_test[g_debug_index]);

    init_list_head(&sgtc_debug_test_stats.sgtc_link);
    term_variable_add(&sgtc_debug_test_stats);

    for (g_debug_index = 0; g_debug_index < (DEBUG_TASK_TYPE * DEBUG_TASK_GROUP); g_debug_index++)
    {
        struct thread_attr sgtc_attr = {};

        thread_attr_setstacksize(&sgtc_attr, THREAD_STACK_PAGE(1));
        thread_set_priority(&sgtc_attr, THREAD_PROTY_DEFAULT);
        thread_set_time_slice(&sgtc_attr, THREAD_TIME_DEFAULT);

        retval = thread_create(mr_nullptr, &sgtc_attr, 
                    debug_test_func[g_debug_index & (DEBUG_TASK_TYPE - 1)], (void *)g_debug_index);
        if (retval)
            print_warn("Create thread which index is %d failed!\r\n", g_debug_index);
    }

    for (;;)
    {   
        schedule_self_sleep();
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
    struct thread_attr sgtc_attr = {};

	sgtc_attr.detachstate = THREAD_CREATE_JOINABLE;
	sgtc_attr.inheritsched	= THREAD_INHERIT_SCHED;
	sgtc_attr.schedpolicy = THREAD_SCHED_FIFO;

    /*!< thread stack */
	thread_set_stack(&sgtc_attr, mr_nullptr, g_debug_stack, sizeof(g_debug_stack));
    /*!< lowest priority */
	thread_set_priority(&sgtc_attr, THREAD_PROTY_DEFAULT);
    /*!< default time slice */
    thread_set_time_slice(&sgtc_attr, THREAD_TIME_DEFAULT);

    /*!< register thread */
    g_debug_tid = kernel_thread_create(-1, &sgtc_attr, debug_entry, mr_nullptr);
    if (g_debug_tid >= 0)
    {
        thread_set_name(g_debug_tid, "debug_task");
        return ER_NORMAL;
    }

    return -ER_FAILD;
}

/*!< end of file */
