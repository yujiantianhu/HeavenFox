/*
 * Terminal Core API: Command ttc (terminal thread comunication)
 *
 * File Name:   ttc.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.12.23
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <term/term.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/mailbox.h>

/*!< The defines */


/*!< The globals */

 
/*!< The functions */


/*!< API functions */
/*!
 * @brief   cmd 'ttc': excute function
 * @param   sptr_cmd, argc, argv
 * @retval  errno
 * @note    none
 */
static kint32_t term_cmd_ttc_mail(struct term_cmd *sptr_cmd, kint32_t argc, kchar_t **argv)
{
    struct thread *sptr_thread;
    struct mailbox *sptr_tar;
    struct mail sgtc_mail;
    struct mail_msg sgtc_msg[2] = {};
    tid_t tid;

    switch (argc)
    {
        case 2:
            if (!kstrcmp(argv[1], "--help"))
                sptr_cmd->help();
            else
                goto fail;

            break;

        case 3:
        case 4:
            if (ascii_to_dec(argv[1], &tid))
            {
                printk("tid is not correct!\r\n");
                return -ER_FAULT;
            }

            sptr_thread = get_thread_handle(tid);
            if (!isValid(sptr_thread))
            {
                printk("can not find thread, check the tid please\r\n");
                return -ER_INVALID;
            }

            /*!< if destination thread is sleepy, mail should not be sent */
            if (!is_thread_valid(tid))
                break;

            mail_init(mr_current->sptr_mb, &sgtc_mail);
            
            sgtc_msg[0].buffer = (kuint8_t *)argv[2];
            sgtc_msg[0].size = kstrlen(argv[2]) + 1;
            sgtc_msg[0].type = NR_MAIL_TYPE_SERIAL;

            if (argc == 4)
            {
                sgtc_msg[1].buffer = (kuint8_t *)argv[3];
                sgtc_msg[1].size = kstrlen(argv[3]) + 1;
                sgtc_msg[1].type = NR_MAIL_TYPE_SERIAL;
            }

            sgtc_mail.sptr_msg = &sgtc_msg[0];
            sgtc_mail.num_msgs = (argc == 3) ? 1 : 2;

            sptr_tar = sptr_thread->sptr_mb;
            mail_send(sptr_tar->name, &sgtc_mail);

            break;

        default:
            goto fail;
    }

    return ER_NORMAL;

fail:
    printk("argument error, try entering \'%s --help\' to get usage\r\n", argv[0]);
    return -ER_FAULT;
}

/*!
 * @brief   cmd 'ttc': help function
 * @param   none
 * @retval  none
 * @note    none
 */
static void term_cmd_ttc_help(void)
{
    printk("usage: ttc [tid] [op1] [op2]\r\n");
}

/*!
 * @brief   cmd 'ttc' init and add
 * @param   none
 * @retval  none
 * @note    none
 */
void term_cmd_add_ttc(void)
{
    struct term_cmd *sptr_cmd;

    sptr_cmd = term_cmd_allocate("ttc", GFP_KERNEL);
    if (!isValid(sptr_cmd))
        return;

    sptr_cmd->do_excute = term_cmd_ttc_mail;
    sptr_cmd->help = term_cmd_ttc_help;

    term_cmd_add(sptr_cmd);
}