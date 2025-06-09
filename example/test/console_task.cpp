/*
 * User Thread Instance (light task) Interface
 *
 * File Name:   console_task.c
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
#define CONSOLE_TASK_STACK_SIZE                       THREAD_STACK_HALF(1)    /*!< 1/2 page (1kbytes) */

/*!< The globals */

/*!< API functions */
/*!
 * @brief  send mail to light thread
 * @param  cptr_this, command_line
 * @retval none
 * @note   none
 */
static void command_mail_to_light(crt_task_t *cptr_this, const kchar_t *command_line)
{
    struct mailbox &sgtc_mb = cptr_this->get_mailbox();
    struct mail sgtc_mail;
    struct mail_msg sgtc_msg[1] = {};
    kuint8_t status = 0;

    if (!string::strncmp(command_line, "led1 on", 7))
        status = 1;
    else if (!string::strncmp(command_line, "led1 off", 8))
        status = 0;
    else
        return;

    mail_init(&sgtc_mb, &sgtc_mail);
    
    sgtc_msg[0].buffer = &status;
    sgtc_msg[0].size = 1;
    sgtc_msg[0].type = NR_MAIL_TYPE_SERIAL;

    sgtc_mail.sptr_msg = &sgtc_msg[0];
    sgtc_mail.num_msgs = 1;

    mail_send("light-app-mailbox", &sgtc_mail);
}

/*!
 * @brief  send mail to display thread
 * @param  cptr_this, command_line
 * @retval none
 * @note   none
 */
static void command_mail_to_display(crt_task_t *cptr_this, const kchar_t *command_line)
{
    struct mailbox &sgtc_mb = cptr_this->get_mailbox();
    struct mail sgtc_mail;
    struct mail_msg sgtc_msg[1] = {};
    kuint8_t status = 0;

    if (!string::strncmp(command_line, "page up", 9))
        status = 1;
    else if (!string::strncmp(command_line, "page down", 8))
        status = 2;
    else
        return;

    mail_init(&sgtc_mb, &sgtc_mail);
    
    sgtc_msg[0].buffer = &status;
    sgtc_msg[0].size = 1;
    sgtc_msg[0].type = NR_MAIL_TYPE_SERIAL;

    sgtc_mail.sptr_msg = &sgtc_msg[0];
    sgtc_mail.num_msgs = 1;

    mail_send("display-app-mailbox", &sgtc_mail);
}

/*!
 * @brief  console recieve task
 * @param  none
 * @retval none
 * @note   none
 */
static void *console_task_entry(void *args)
{   
    crt_task_t *cptr_this = (crt_task_t *)args;
    string cgtc_str(1024);

    for (;;)
    {       
        do {
            /*!< read command line */
            cin >> cgtc_str;

        } while (cgtc_str.size() <= 0);

        cout << "recv command line, data is: " << cgtc_str.c_str() << endl;

        command_mail_to_light(cptr_this, cgtc_str.c_str());
        command_mail_to_display(cptr_this, cgtc_str.c_str());
    }

    return args;
}

/*!
 * @brief	create console app task
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t console_task_init(void)
{
    static kuint8_t g_console_task_stack[CONSOLE_TASK_STACK_SIZE];

    crt_task_t *cptr_task = new crt_task_t("console task", 
                                            console_task_entry, 
                                            g_console_task_stack, 
                                            sizeof(g_console_task_stack));
    if (!cptr_task)
        return -ER_FAILD;

    struct mailbox &sgtc_mb = cptr_task->get_mailbox();
    mailbox_init(&sgtc_mb, cptr_task->get_self(), "console-task-mailbox");

    return ER_NORMAL;
}

/*!< end of file */
