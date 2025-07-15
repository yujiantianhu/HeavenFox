/*
 * User Thread Instance (display task) Interface
 *
 * File Name:   lvgl_task.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.11.17
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <common/io_stream.h>
#include <platform/base/fwk_fcntl.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/mutex.h>
#include <kernel/mailbox.h>

#include "../../task.h"
#include "../demo_task.h"

#include "porting/lv_port_disp.h"
#include "porting/lv_port_fs.h"
#include "app/app.h"

using namespace tsk;

/*!< The defines */
#define LVGL_TASK_STACK_SIZE                    THREAD_STACK_PAGE(1)    /*!< 1 page (4kbytes) */

/*!< The globals */

/*!< API functions */
/*!
 * @brief  display task
 * @param  none
 * @retval none
 * @note   do display
 */
static void *lvgl_task_entry(void *args)
{
    struct fwk_disp_ctrl sgtc_dctrl;
    struct fwk_disp_info sgtc_disp;

    sgtc_dctrl.sptr_di = &sgtc_disp;

    lv_init();
    lv_port_disp_init(&sgtc_dctrl);
    lv_port_fs_init();

    lvgl_task_setup(&sgtc_dctrl);
    msleep(1);

    for (;;)
    {
        lvgl_task(&sgtc_dctrl);
        msleep(100);
    }

    return args;
}

/*!
 * @brief	create display app task
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t lvgl_task_init(void)
{
    static THREAD_STACK_DEFINE(g_lvgl_task_stack, LVGL_TASK_STACK_SIZE);

    crt_task_t *cptr_task = new crt_task_t("lvgl_task", 
                                            lvgl_task_entry, 
                                            g_lvgl_task_stack, 
                                            sizeof(g_lvgl_task_stack),
                                            THREAD_PROTY_DEFAULT,
                                            40);
    if (!cptr_task)
        return -ER_FAILD;

    struct mailbox &sgtc_mb = cptr_task->self_mailbox();
    mailbox_init(&sgtc_mb, cptr_task->self_id(), "lvgl-task-mailbox");

    return ER_NORMAL;
}

/*!< end of file */
