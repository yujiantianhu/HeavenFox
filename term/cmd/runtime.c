/*
 * Terminal Core API: Command runtime
 *
 * File Name:   runtime.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.06.11
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <term/term.h>

/*!< The defines */


/*!< The globals */


/*!< The functions */

/*!< API functions */
/*!
 * @brief   cmd 'runtime': excute function
 * @param   sptr_cmd, argc, argv
 * @retval  errno
 * @note    none
 */
static kint32_t term_cmd_show_runtime(struct term_cmd *sptr_cmd, kint32_t argc, kchar_t **argv)
{
    struct time_clock *sptr_clock = &sgtc_systime_clock;

    switch (argc)
    {
        case 1:
            printk("System runtime after kernel starting:\r\n");
            printk("    Systick: %lu\r\n", JIFFIES_COUNT());
            printk("    Time:    %u(year)-%u(month)-%u(day) %u:%u:%u:%u\r\n",
                sptr_clock->year, sptr_clock->month, sptr_clock->day,
                sptr_clock->hour, sptr_clock->minute, sptr_clock->second, sptr_clock->milsecond);

            break;

        case 2:
            if (!kstrcmp(argv[1], "--help"))
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
 * @brief   cmd 'runtime': help function
 * @param   none
 * @retval  none
 * @note    none
 */
static void term_cmd_runtime_help(void)
{
    printk("usage: runtime\r\n");
}

/*!
 * @brief   cmd 'runtime' init and add
 * @param   none
 * @retval  none
 * @note    none
 */
void term_cmd_add_runtime(void)
{
    struct term_cmd *sptr_cmd;

    sptr_cmd = term_cmd_allocate("runtime", GFP_KERNEL);
    if (!isValid(sptr_cmd))
        return;

    sptr_cmd->do_excute = term_cmd_show_runtime;
    sptr_cmd->help = term_cmd_runtime_help;

    term_cmd_add(sptr_cmd);
}
