/*
 * User Thread Instance (tsc task) Interface
 *
 * File Name:   tsc_app.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.21
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <common/io_stream.h>
#include <platform/fwk_fcntl.h>
#include <platform/input/fwk_input.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/mutex.h>
#include <kernel/mailbox.h>

#include "thread_table.h"

/*!< The defines */
#define TSCAPP_THREAD_STACK_SIZE                          KEL_THREAD_STACK_HALF(1)    /*!< 1/2 page (1kbytes) */

/*!< The globals */
static real_thread_t g_tsc_app_tid;
static struct kel_thread_attr sgrt_tsc_app_attr;
static kuint32_t g_tsc_app_stack[TSCAPP_THREAD_STACK_SIZE];

/*!< API functions */
/*!
 * @brief  tsc task
 * @param  none
 * @retval none
 * @note   trun on/off led by timer
 */
static void *tsc_app_entry(void *args)
{
    kint32_t fd;
    struct fwk_input_event sgrt_event[4] = {};
    kssize_t retval;

    for (;;)
    {
        fd = virt_open("/dev/input/event0", 0);
        if (fd < 0)
            goto END1;

        memset(&sgrt_event[0], 0, sizeof(sgrt_event));

        retval = virt_read(fd, &sgrt_event[0], sizeof(sgrt_event));
        if ((retval < 0))
            goto END2;

        print_info("%s: key: %d, abs_x: %d, abs_y: %d\n", __FUNCTION__, 
                        sgrt_event[0].value, sgrt_event[1].value, sgrt_event[2].value);
        
END2:
        virt_close(fd);
END1:
        schedule_delay_ms(200);
    }

    return args;
}

/*!
 * @brief	create tsc app task
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t tsc_app_init(void)
{
    struct kel_thread_attr *sprt_attr = &sgrt_tsc_app_attr;
    kint32_t retval;

	sprt_attr->detachstate = KEL_THREAD_CREATE_JOINABLE;
	sprt_attr->inheritsched	= KEL_THREAD_INHERIT_SCHED;
	sprt_attr->schedpolicy = KEL_THREAD_SCHED_FIFO;

    /*!< thread stack */
	real_thread_set_stack(sprt_attr, mrt_nullptr, g_tsc_app_stack, sizeof(g_tsc_app_stack));
    /*!< lowest priority */
	real_thread_set_priority(sprt_attr, KEL_THREAD_PROTY_DEFAULT);
    /*!< default time slice */
    real_thread_set_time_slice(sprt_attr, KEL_THREAD_TIME_DEFUALT);

    /*!< register thread */
    retval = real_thread_create(&g_tsc_app_tid, sprt_attr, tsc_app_entry, mrt_nullptr);
    return (retval < 0) ? retval : 0;
}

/*!< end of file */
