/*
 * User Thread Instance (net task) Interface
 *
 * File Name:   network_task.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.11.21
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
#include <fs/fs_intr.h>

#include "../../task.h"
#include "../demo_task.h"
#include "app/app.h"

using namespace tsk;

/*!< The defines */
#define NETWORK_TASK_STACK_SIZE                        THREAD_STACK_PAGE(1)    /*!< 1 page (4kbytes) */

/*!< The globals */
static crt_lwip_data_t sgtc_network_task_data;

/*!< API functions */
/*!
 * @brief  net task
 * @param  none
 * @retval none
 * @note   do display
 */
static void *network_task_entry(void *args)
{
    crt_lwip_data_t &cgtc_data = sgtc_network_task_data;

    cgtc_data.args = args;
    cgtc_data.echo_cnt = 0;

    cgtc_data.startup();

    for (;;)
    {
        cgtc_data.excute();
        sleep(1);
    }

    return args;
}

/*!
 * @brief	create net app task
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t network_task_init(void)
{
    static THREAD_STACK_DEFINE(g_network_task_stack, NETWORK_TASK_STACK_SIZE);

    crt_task_t *cptr_task = new crt_task_t("network_task", 
                                            network_task_entry, 
                                            g_network_task_stack, 
                                            sizeof(g_network_task_stack),
                                            THREAD_PROTY_DEFAULT,
                                            100);
    if (!cptr_task)
        return -ER_FAILD;

    struct mailbox &sgtc_mb = cptr_task->self_mailbox();
    mailbox_init(&sgtc_mb, cptr_task->self_id(), "network-task-mailbox");

    return ER_NORMAL;
}

/*!< end of file */
