/*
 * User Thread Instance (display task) Interface
 *
 * File Name:   display_app.c
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
#include <platform/fwk_fcntl.h>
#include <platform/video/fwk_fbmem.h>
#include <platform/video/fwk_rgbmap.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/mutex.h>

#include "thread_table.h"

/*!< The defines */
#define DISPLAY_APP_THREAD_STACK_SIZE                       KEL_THREAD_STACK_HALF(1)    /*!< 1/2 page (1kbytes) */

/*!< The globals */
static real_thread_t g_display_app_tid;
static srt_kel_thread_attr_t sgrt_display_app_attr;
static kuint32_t g_display_app_stack[DISPLAY_APP_THREAD_STACK_SIZE];
static struct mutex_lock sgrt_display_app_lock;

static const kuint32_t g_rgb_color_table[] = { 
    RGB_BLUE, 
    RGB_LIME, 
    RGB_CYAN, 
    RGB_RED,
    RGB_MAGENTA, 
    RGB_YELLOW, 
    RGB_WHITE
};

/*!< API functions */
/*!
 * @brief  display one color on full screen
 * @param  none
 * @retval none
 * @note   do display
 */
static void display_fill_one_screen(kuint32_t *pbuffer, kuint32_t width, kuint32_t height)
{
    static kuint32_t *ptr_color = (kuint32_t *)&g_rgb_color_table[0];
    kuint32_t i, j;

    mutex_lock(&sgrt_display_app_lock);
    
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            *(pbuffer++) = *ptr_color;
        }
    }
    mutex_unlock(&sgrt_display_app_lock);

    if (((++ptr_color) - (&g_rgb_color_table[0])) >= sizeof(g_rgb_color_table))
    {
        ptr_color = (kuint32_t *)&g_rgb_color_table[0];
    }
}

/*!
 * @brief  display task
 * @param  none
 * @retval none
 * @note   do display
 */
static void *display_app_entry(void *args)
{
    ksint32_t fd;
    struct fwk_fb_fix_screen_info sgrt_fix;
	struct fwk_fb_var_screen_info sgrt_var;
    kuint32_t *fbuffer;

    mutex_init(&sgrt_display_app_lock);

    for (;;)
    {
        fd = virt_open("/dev/fb0", 0);
        if (!mrt_isErr(fd))
        {
            virt_ioctl(fd, NR_FB_IOGetVScreenInfo, &sgrt_var);
            virt_ioctl(fd, NR_FB_IOGetFScreenInfo, &sgrt_fix);

            fbuffer = (kuint32_t *)virt_mmap(mrt_nullptr, sgrt_fix.smem_len, 0, 0, fd, 0);
            if (!mrt_isValid(fbuffer))
            {
                virt_close(fd);
            }

            display_fill_one_screen(fbuffer, sgrt_var.xres, sgrt_var.yres);
            
            virt_munmap(fbuffer, sgrt_fix.smem_len);
            virt_close(fd);
        }

        schedule_delay_ms(500);
    }

    return args;
}

/*!
 * @brief	create display app task
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
ksint32_t display_app_init(void)
{
    srt_kel_thread_attr_t *sprt_attr = &sgrt_display_app_attr;
    ksint32_t retval;

	sprt_attr->detachstate = KEL_THREAD_CREATE_JOINABLE;
	sprt_attr->inheritsched	= KEL_THREAD_INHERIT_SCHED;
	sprt_attr->schedpolicy = KEL_THREAD_SCHED_FIFO;

    /*!< thread stack */
	real_thread_set_stack(sprt_attr, mrt_nullptr, g_display_app_stack, sizeof(g_display_app_stack));
    /*!< lowest priority */
	real_thread_set_priority(sprt_attr, KEL_THREAD_PROTY_DEFAULT);
    /*!< default time slice */
    real_thread_set_time_slice(sprt_attr, KEL_THREAD_TIME_DEFUALT);

    /*!< register thread */
    retval = real_thread_create(&g_display_app_tid, sprt_attr, display_app_entry, mrt_nullptr);
    return mrt_isErr(retval) ? retval : 0;
}

/*!< end of file */
