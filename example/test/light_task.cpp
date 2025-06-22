/*
 * User Thread Instance (light task) Interface
 *
 * File Name:   light_app.cpp
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
#include <platform/base/fwk_fcntl.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/mutex.h>
#include <kernel/mailbox.h>

#include "../task.h"
#include "test_task.h"

using namespace tsk;
using namespace bsc;

/*!< The defines */
#define LIGHT_TASK_STACK_SIZE                   THREAD_STACK_HALF(1)    /*!< 1/2 page (2kbytes) */

/*!< The globals */

/*!< API functions */
/*!
 * @brief  light task
 * @param  none
 * @retval none
 * @note   trun on/off led by timer
 */
static void *light_task_entry(void *args)
{
    crt_task_t *cptr_this = (crt_task_t *)args;
    kbool_t status = 0;
    kint32_t fd;
    struct mailbox &sgtc_mb = cptr_this->self_mailbox();
    struct mail *sptr_mail;
    tid_t tid = cptr_this->self_id();
    
    cout << __FUNCTION__ << " is running, which tid is: " << tid << endl;

    for (;;)
    {        
        fd = virt_open("/dev/ledgpio", O_RDWR);
        if (fd < 0)
            goto END1;
        
        sptr_mail = mail_recv(&sgtc_mb, 0);
        if (!isValid(sptr_mail))
            goto END2;

        if ((sptr_mail->sptr_msg->type == NR_MAIL_TYPE_SERIAL) ||
            (sptr_mail->sptr_msg->type == NR_MAIL_TYPE_KEY))
        {
            kchar_t *buffer = (kchar_t *)sptr_mail->sptr_msg[0].buffer;

            if (!string::strncmp(buffer, "on", 2))
                status = 1;
            else if (!string::strncmp(buffer, "off", 3))
                status = 0;
        }

        virt_write(fd, &status, 1);
        mail_recv_finish(sptr_mail);

END2:
        virt_close(fd);
END1:
        msleep(203);
    }

    return args;
}

/*!
 * @brief	create light app task
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t light_task_init(void)
{
    static THREAD_STACK_DEFINE(g_light_task_stack, LIGHT_TASK_STACK_SIZE);

    crt_task_t *cptr_task = new crt_task_t("light_task", 
                                            light_task_entry, 
                                            g_light_task_stack, 
                                            sizeof(g_light_task_stack));
    if (!cptr_task)
        return -ER_FAILD;

    struct mailbox &sgtc_mb = cptr_task->self_mailbox();
    mailbox_init(&sgtc_mb, cptr_task->self_id(), "light-task-mailbox");

    return ER_NORMAL;
}

/*!< end of file */
