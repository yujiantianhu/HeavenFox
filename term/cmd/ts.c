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
#include <platform/base/fwk_basic.h>
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
    printk("tid  stack_size(B) priority slice(ms) state   name\r\n");
    printk("----------------------------------------------------------\r\n");
    printk("state: (1)running, (2)ready, (3)suspend, (4)sleep\r\n");
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
    struct spin_lock *sptr_lock = scheduler_lock();
    tid_t tid = -1;
    kint32_t arg = 0;
    kutype_t flags;

    switch (argc)
    {
        case 1:
            term_cmd_ts_title();
            spin_lock_irqsave(sptr_lock, &flags);

            /*!< 1. running */
            sptr_thread = mr_current;
            sptr_attr = sptr_thread->sptr_attr;
            term_cmd_ts_format(sptr_thread->tid, sptr_attr->stacksize, sptr_attr->sgtc_param.cur_priority,
                        thread_get_sched_msecs(sptr_attr), sptr_thread->state, sptr_thread->name);
            
            /*!< 2. ready */
            sptr_thread = mr_nullptr;
            while ((sptr_thread = next_ready_thread(sptr_thread)))
            {
                sptr_attr = sptr_thread->sptr_attr;
                term_cmd_ts_format(sptr_thread->tid, sptr_attr->stacksize, sptr_attr->sgtc_param.cur_priority,
                        thread_get_sched_msecs(sptr_attr), sptr_thread->state, sptr_thread->name);
            }

            /*!< 3. suspend */
            sptr_thread = mr_nullptr;
            while ((sptr_thread = next_suspend_thread(sptr_thread)))
            {
                sptr_attr = sptr_thread->sptr_attr;
                term_cmd_ts_format(sptr_thread->tid, sptr_attr->stacksize, sptr_attr->sgtc_param.cur_priority,
                        thread_get_sched_msecs(sptr_attr), sptr_thread->state, sptr_thread->name);
            }

            /*!< 4. sleep */
            sptr_thread = mr_nullptr;
            while ((sptr_thread = next_sleep_thread(sptr_thread)))
            {
                sptr_attr = sptr_thread->sptr_attr;
                term_cmd_ts_format(sptr_thread->tid, sptr_attr->stacksize, sptr_attr->sgtc_param.cur_priority,
                        thread_get_sched_msecs(sptr_attr), sptr_thread->state, sptr_thread->name);
            }

            spin_unlock_irqrestore(sptr_lock, flags);
            break;

        case 2:
            if (!kstrcmp(argv[1], "--help"))
                sptr_cmd->help();
            else if (!ascii_to_dec(argv[1], &tid))
            {
                sptr_thread = mr_tid_handle(tid);
                if (!sptr_thread)
                {
                    printk("tid format is invalid or thread is not exsist\r\n");
                    goto fail;
                }

                sptr_attr = sptr_thread->sptr_attr;
                printk("thread name: %s\r\n"
                       "tid: %d\r\n"
                       "thread state ((1)running, (2)ready, (3)suspend, (4)sleep): %d\r\n"
                       "original priority: %d\r\n"
                       "real-time priority: %d\r\n"
                       "time slice: %u(ms)\r\n"
                       "time slice left: %u(ms)\r\n"
                       "stack start: %#x\r\n"
                       "stack size: %u(bytes)\r\n",
                       sptr_thread->name, sptr_thread->tid, sptr_thread->state, 
                       thread_get_ori_priority(sptr_attr), thread_get_priority(sptr_attr),
                       thread_get_sched_msecs(sptr_attr), sptr_thread->expires, sptr_attr->stack_addr, sptr_attr->stacksize);
            }
            else
                goto fail;

            break;

        case 4:
            if (ascii_to_dec(argv[1], &tid) || ascii_to_dec(argv[3], &arg))
                goto fail;

            sptr_thread = mr_tid_handle(tid);
            if (!sptr_thread)
            {
                printk("tid format is invalid or thread is not exsist\r\n");
                goto fail;
            }

            sptr_attr = sptr_thread->sptr_attr;
            if (!kstrcmp(argv[2], "-p"))
            {
                if (arg != __THREAD_CHECK_PRIO(arg))
                {
                    printk("priority is not invalid\r\n");
                    goto fail;
                }

                if (arg != thread_get_ori_priority(sptr_attr))
                {
                    kuint32_t cur_prio;

                    spin_lock_irqsave(&sptr_thread->sgtc_lock, &flags);
                    cur_prio = thread_get_rt_priority(sptr_attr);
                    thread_set_priority(sptr_attr, arg);

                    /*!< Priority changed */
                    if (cur_prio != thread_get_rt_priority(sptr_attr))
                    {
                        __SET_THREAD_TARGET_STATE(sptr_thread, __GET_THREAD_STATE(sptr_thread));
                        spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);

                        /*!< Protect scheduler with scheduler-lock */
                        spin_lock_irqsave(sptr_lock, &flags);
                        schedule_thread_switch(sptr_thread);
                        spin_unlock_irqrestore(sptr_lock, flags);
                    }
                    else
                    {
                        /*!< do nothing */
                        spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);
                    }
                }
            }
            else if (!kstrcmp(argv[2], "-t"))
            {
                spin_lock_irqsave(&sptr_thread->sgtc_lock, &flags);
                thread_set_time_slice(sptr_thread->sptr_attr, arg);
                spin_unlock_irqrestore(&sptr_thread->sgtc_lock, flags);
            }
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
    printk("usage: \r\n"
           "    ts                      | list all threads\r\n"
           "    ts [tid]                | list thread with the tid\r\n"
           "    ts [tid] -p [argument]  | set thread's priority\r\n"
           "    ts [tid] -t [argument]  | set thread's time slice\r\n");
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
