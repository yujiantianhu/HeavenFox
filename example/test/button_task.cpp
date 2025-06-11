/*
 * User Thread Instance (button task) Interface
 *
 * File Name:   button_task.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.25
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
#define BUTTON_TASK_STACK_SIZE                          THREAD_STACK_HALF(1)    /*!< 1/2 page (1kbytes) */

/*!< The globals */

/*!< API functions */
/*!
 * @brief  button task
 * @param  none
 * @retval none
 * @note   trun on/off led by timer
 */
static void *button_task_entry(void *args)
{
    crt_task_t *cptr_this = (crt_task_t *)args;
    kuint8_t status = 0, last_status = 0;
    kint32_t fd;
    struct mailbox &sgtc_mb = cptr_this->get_mailbox();
    struct mail *sptr_mail = mr_nullptr;
    struct mail_msg sgtc_msg[1] = {};
    kchar_t msgs[8];
    kssize_t retval;

    do {
        fd = virt_open("/dev/input/event0", O_RDONLY);
        if (mr_unlikely(fd < 0))
            msleep(200);

    } while (fd < 0);

    for (;;)
    {
        retval = virt_read(fd, &status, 1);
        if ((retval < 0) || (status == last_status))
            goto END;
        
        if (sptr_mail)
            mail_destroy(&sgtc_mb, sptr_mail);

        sptr_mail = mail_create(&sgtc_mb);
        if (!isValid(sptr_mail))
        {
            sptr_mail = mr_nullptr;
            goto END;
        }

        if (status)
        {
            string::strcpy(msgs, "on");
            sgtc_msg[0].size = 2;
            msgs[2] = '\0';
        }
        else
        {
            string::strcpy(msgs, "off");
            sgtc_msg[0].size = 3;
            msgs[3] = '\0';
        }

        sgtc_msg[0].buffer = (kuint8_t *)msgs;
        sgtc_msg[0].type = NR_MAIL_TYPE_KEY;

        sptr_mail->sptr_msg = &sgtc_msg[0];
        sptr_mail->num_msgs = 1;

        mail_send("light-task-mailbox", sptr_mail);
        last_status = status;

END:
        msleep(200);
    }

    virt_close(fd);
    return args;
}

/*!
 * @brief	create button app task
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t button_task_init(void)
{
    static THREAD_STACK_DEFINE(g_button_task_stack, BUTTON_TASK_STACK_SIZE);

    crt_task_t *cptr_task = new crt_task_t("button_task", 
                                            button_task_entry, 
                                            g_button_task_stack, 
                                            sizeof(g_button_task_stack),
                                            __THREAD_HIGHER_DEFAULT(1));
    if (!cptr_task)
        return -ER_FAILD;

    struct mailbox &sgtc_mb = cptr_task->get_mailbox();
    mailbox_init(&sgtc_mb, cptr_task->get_self(), "button-task-mailbox");

    return ER_NORMAL;
}

/*!< end of file */
