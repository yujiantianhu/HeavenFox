/*
 * Terminal Core API: Command ts
 *
 * File Name:   ts.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.12.24
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <kernel/signal.h>
#include <kernel/sched.h>
#include <kernel/spinlock.h>
#include <term/term.h>

/*!< The defines */


/*!< The globals */


/*!< The functions */

/*!< API functions */
/*!
 * @brief   display title
 * @param   none
 * @retval  none
 * @note    none
 */
static void term_cmd_ts_title(void)
{
    printk("----------------------------------------------------------\r\n");
    printk("tid  stack_size(B) priority slice(ms) status  name\r\n");
    printk("----------------------------------------------------------\r\n");
    printk("status: (1)running, (2)ready, (3)suspend, (4)sleep\r\n");
    printk("----------------------------------------------------------\r\n");
}

/*!
 * @brief   show thread info
 * @param   tid, stack_size, ...
 * @retval  none
 * @note    none
 */
static void term_cmd_ts_format(tid_t tid, kuint32_t stack_size, kuint32_t prio, 
                        kutime_t expires, kuint32_t status, kchar_t *name)
{
    printk("%-10d%-10d%-10d%-10d%-6d%s\r\n", tid, stack_size, prio, expires, status, name);
}

/*!
 * @brief   cmd 'ts': excute function
 * @param   sptr_cmd, argc, argv
 * @retval  errno
 * @note    none
 */
static kint32_t term_cmd_task_show(struct term_cmd *sptr_cmd, kint32_t argc, kchar_t **argv)
{
    struct thread *sptr_thread;
    struct thread_attr *sptr_attr;
    struct spin_lock *sptr_lock;

    switch (argc)
    {
        case 1:
            sptr_lock = scheduler_lock();
            spin_lock_irqsave(sptr_lock);

            term_cmd_ts_title();

            /*!< 1. running */
            sptr_thread = mr_current;
            sptr_attr = sptr_thread->sptr_attr;
            term_cmd_ts_format(sptr_thread->tid, sptr_attr->stacksize, sptr_attr->sgtc_param.sched_curpriority,
                        thread_get_sched_msecs(sptr_attr), sptr_thread->status, sptr_thread->name);
            
            /*!< 2. ready */
            sptr_thread = mr_nullptr;
            while ((sptr_thread = next_ready_thread(sptr_thread)))
            {
                sptr_attr = sptr_thread->sptr_attr;
                term_cmd_ts_format(sptr_thread->tid, sptr_attr->stacksize, sptr_attr->sgtc_param.sched_curpriority,
                        thread_get_sched_msecs(sptr_attr), sptr_thread->status, sptr_thread->name);
            }

            /*!< 3. suspend */
            sptr_thread = mr_nullptr;
            while ((sptr_thread = next_suspend_thread(sptr_thread)))
            {
                sptr_attr = sptr_thread->sptr_attr;
                term_cmd_ts_format(sptr_thread->tid, sptr_attr->stacksize, sptr_attr->sgtc_param.sched_curpriority,
                        thread_get_sched_msecs(sptr_attr), sptr_thread->status, sptr_thread->name);
            }

            /*!< 4. sleep */
            sptr_thread = mr_nullptr;
            while ((sptr_thread = next_sleep_thread(sptr_thread)))
            {
                sptr_attr = sptr_thread->sptr_attr;
                term_cmd_ts_format(sptr_thread->tid, sptr_attr->stacksize, sptr_attr->sgtc_param.sched_curpriority,
                        thread_get_sched_msecs(sptr_attr), sptr_thread->status, sptr_thread->name);
            }

            spin_unlock_irqrestore(sptr_lock);
            break;

        case 2:
            if (!strcmp(argv[1], "--help"))
                sptr_cmd->help();
            else
                goto fail;

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
 * @brief   cmd 'ts': help function
 * @param   none
 * @retval  none
 * @note    none
 */
static void term_cmd_ts_help(void)
{
    printk("usage: ts\r\n");
}

/*!
 * @brief   cmd 'ts' init and add
 * @param   none
 * @retval  none
 * @note    none
 */
void term_cmd_add_ts(void)
{
    struct term_cmd *sptr_cmd;

    sptr_cmd = term_cmd_allocate("ts", GFP_KERNEL);
    if (!isValid(sptr_cmd))
        return;

    sptr_cmd->do_excute = term_cmd_task_show;
    sptr_cmd->help = term_cmd_ts_help;

    term_cmd_add(sptr_cmd);
}
