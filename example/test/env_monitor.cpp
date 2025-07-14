/*
 * User Thread Instance (environment monitor task) Interface
 *
 * File Name:   env_monitor.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.06.06
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
#include <platform/i2c/fwk_eeprom.h>
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
#define ENV_TASK_STACK_SIZE                         THREAD_STACK_PAGE(1)    /*!< 1/2 page (2kbytes) */

/*!< The globals */

/*!< API functions */
/*!
 * @brief  environment monitor task
 * @param  none
 * @retval none
 * @note   monitor environment
 */
static void *env_monitor_entry(void *args)
{
    crt_task_t *cptr_this = (crt_task_t *)args;
    kint32_t fd;
    struct mailbox &sgtc_mb = cptr_this->self_mailbox();
    struct mail *sptr_mail;
    kuint32_t info[3] = {};
    struct fwk_eeprom sgtc_eep;
    kssize_t retval;

    do 
    {
        fd = virt_open("/dev/ap3216c", O_RDWR);
        if (fd < 0)
            msleep(200);

    } while (fd < 0);

    sgtc_eep.addr = 0x2d;
    sgtc_eep.offset = 0;
    sgtc_eep.buf = (kuint8_t *)&info[0];
    sgtc_eep.size = sizeof(info);

    for (;;)
    {
        retval = virt_read(fd, info, sizeof(info));
        if (retval < 0)
            goto END;

        /*!< Choose if show info */
        sptr_mail = mail_recv(&sgtc_mb, 0);
        if (!isValid(sptr_mail))
            goto END;

        if (sptr_mail->sptr_msg->type == NR_MAIL_TYPE_SERIAL)
        {
            kchar_t *buffer = (kchar_t *)sptr_mail->sptr_msg[0].buffer;

            if (!string::strncmp(buffer, "info", 4))
            {
                cout << cptr_this->self_name()
                    << ": ir: "    << info[0] 
                    << ", als: "   << info[1] 
                    << ", ps: "    << info[2] 
                    << endl;
            }
            else if (!string::strncmp(buffer, "sync", 4))
            {
                kint32_t eep_fd;

                eep_fd = virt_open("/dev/at24c02", O_RDWR);
                if (eep_fd < 0)
                    goto fini;

                cout << "Write enviromental data to EEPROM, waitting ..." << endl;

                retval = virt_ioctl(eep_fd, FWK_EEPROM_WRITE, &sgtc_eep);
                if (retval < 0)
                {
                    virt_close(eep_fd);
                    cout << "Write enviromental data to EEPROM failed!" << endl;
                    goto fini;
                }

                cout << "Enviromental data has been synchronized to EEPROM" << endl;
                memset(info, 0, sizeof(info));

                cout << "Read EEPROM ===> " << endl;
                retval = virt_ioctl(eep_fd, FWK_EEPROM_READ, &sgtc_eep);
                if (retval < 0)
                {
                    virt_close(eep_fd);
                    cout << "Read enviromental data from EEPROM failed!" << endl;
                    goto fini;
                }

                virt_close(eep_fd);

                cout << "   ir : " << info[0] << endl;
                cout << "   als: " << info[1] << endl;
                cout << "   ps : " << info[2] << endl;
            }
        }

fini:
        mail_recv_finish(sptr_mail);
        
END:
        msleep(207);
    }

    virt_close(fd);
    return args;
}

/*!
 * @brief	create environment monitor task
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t env_monitor_init(void)
{
    static THREAD_STACK_DEFINE(g_env_monitor_stack, ENV_TASK_STACK_SIZE);

    crt_task_t *cptr_task = new crt_task_t("env_monitor_task", 
                                            env_monitor_entry, 
                                            g_env_monitor_stack, 
                                            sizeof(g_env_monitor_stack));
    if (!cptr_task)
        return -ER_FAILD;

    struct mailbox &sgtc_mb = cptr_task->self_mailbox();
    mailbox_init(&sgtc_mb, cptr_task->self_id(), "env_monitor-task-mailbox");

    return ER_NORMAL;
}

/*!< end of file */
