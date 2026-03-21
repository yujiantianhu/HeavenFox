/*
 * Terminal Core API: Command cpu info
 *
 * File Name:   cpuinfo.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2026.03.15
 *
 * Copyright (c) 2026   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <kernel/sched.h>
#include <kernel/preempt.h>
#include <kernel/mutex.h>
#include <term/term.h>

/*!< The defines */


/*!< The globals */


/*!< The functions */

/*!< API functions */
/*!
 * @brief   cmd 'cpuinfo': excute function
 * @param   sptr_cmd, argc, argv
 * @retval  errno
 * @note    none
 */
static kint32_t term_cmd_show_cpuinfo(struct term_cmd *sptr_cmd, kint32_t argc, kchar_t **argv)
{
    struct scheduler_core *sptr_core;
    kuint32_t ready_num, suspend_num, sleep_num, zombie_num;

    switch (argc)
    {
        case 1:
            foreach_percpu(kuint32_t, cpuid)
            {
                if (!get_cpu_current_thread(cpuid))
                    continue;

                sptr_core = get_scheduler_core(cpuid);

                ready_num = sptr_core->ready_num;
                suspend_num = sptr_core->suspend_num;
                sleep_num = sptr_core->sleep_num;
                zombie_num = sptr_core->zombie_num;

                printk("processor           : %u\r\n", cpuid);
                printk("architecture        : %s %s\r\n", get_arch_type(), get_arch_class());
                printk("cpu name            : %s %s\r\n", get_cpu_verdor(), get_cpu_name());
                printk("OS mode             : %s\r\n", get_cpu_mode());

                printk("ready thread num    : %u\r\n", ready_num);
                printk("suspend thread num  : %u\r\n", suspend_num);
                printk("sleep thread num    : %u\r\n", sleep_num);
                printk("zombie thread num   : %u\r\n", zombie_num);

                printk("\r\n");
            }

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
 * @brief   cmd 'cpuinfo': help function
 * @param   none
 * @retval  none
 * @note    none
 */
static void term_cmd_cpuinfo_help(void)
{
    printk("usage: cpuinfo\r\n");
}

/*!
 * @brief   cmd 'cpuinfo' init and add
 * @param   none
 * @retval  none
 * @note    none
 */
void term_cmd_add_cpuinfo(void)
{
    struct term_cmd *sptr_cmd;

    sptr_cmd = term_cmd_allocate("cpuinfo", GFP_KERNEL);
    if (!isValid(sptr_cmd))
        return;

    sptr_cmd->do_excute = term_cmd_show_cpuinfo;
    sptr_cmd->help = term_cmd_cpuinfo_help;

    term_cmd_add(sptr_cmd);
}
