/*
 * User Thread Instance (light task) Interface
 *
 * File Name:   light_app.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.01
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <common/io_stream.h>
#include <platform/hal_fcntl.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/mutex.h>

#include "thread_table.h"

/*!< The defines */
#define LIGHTAPP_THREAD_STACK_SIZE                          KEL_THREAD_STACK_HALF(1)    /*!< 1/2 page (1kbytes) */

/*!< The globals */
static real_thread_t g_light_app_tid;
static srt_kel_thread_attr_t sgrt_light_app_attr;
static kuint32_t g_light_app_stack[LIGHTAPP_THREAD_STACK_SIZE];

/*!< API functions */
/*!
 * @brief  light task
 * @param  none
 * @retval none
 * @note   trun on/off led by timer
 */
static void *light_app_entry(void *args)
{
    static kbool_t iLightStatus = 0;
    ksint32_t fd = 0;

    for (;;)
    {
        fd = virt_open("/dev/led-template", 0);
        if (!mrt_isErr(fd))
        {
            virt_write(fd, &iLightStatus, 1);
            virt_close(fd);
        }

        iLightStatus ^= 1;

        schedule_delay_ms(500);
    }

    return args;
}

/*!
 * @brief	create light app task
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
ksint32_t light_app_init(void)
{
    srt_kel_thread_attr_t *sprt_attr = &sgrt_light_app_attr;
    ksint32_t retval;

	sprt_attr->detachstate = KEL_THREAD_CREATE_JOINABLE;
	sprt_attr->inheritsched	= KEL_THREAD_INHERIT_SCHED;
	sprt_attr->schedpolicy = KEL_THREAD_SCHED_FIFO;

    /*!< thread stack */
	real_thread_set_stack(sprt_attr, mrt_nullptr, g_light_app_stack, sizeof(g_light_app_stack));
    /*!< lowest priority */
	real_thread_set_priority(sprt_attr, KEL_THREAD_PROTY_DEFAULT);
    /*!< default time slice */
    real_thread_set_time_slice(sprt_attr, KEL_THREAD_TIME_DEFUALT);

    /*!< register thread */
    retval = real_thread_create(&g_light_app_tid, sprt_attr, light_app_entry, mrt_nullptr);
    return mrt_isErr(retval) ? retval : 0;
}

/*!< end of file */
