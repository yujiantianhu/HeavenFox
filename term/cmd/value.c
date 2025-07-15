/*
 * Terminal Core API: Command value
 *
 * File Name:   value.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.12.23
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <kernel/signal.h>
#include <kernel/sched.h>
#include <term/term.h>

/*!< The defines */


/*!< The globals */


/*!< The functions */

/*!< API functions */
/*!
 * @brief   cmd 'value': excute function
 * @param   sptr_cmd, argc, argv
 * @retval  errno
 * @note    none
 */
static kint32_t term_cmd_value(struct term_cmd *sptr_cmd, kint32_t argc, kchar_t **argv)
{
    struct term_variable *sptr_var;
    kint32_t value;

    switch (argc)
    {
        case 4:
            if (kstrcmp(argv[1], "-w"))
                goto fail;

            if (ascii_to_dec(argv[2], &value))
            {
                kuint32_t max = (((kuint32_t)(~0U)) << 1) >> 1;
                printk("Variable '%s' is invalid, please input [-%u ~ %u]\r\n", argv[2], max, max);
                break;
            }

            sptr_var = term_variable_find_by_name(argv[3]);
            if (!sptr_var || !sptr_var->var)
            {
                printk("No variable named '%s'\r\n", argv[3]);
                break;
            }

            *sptr_var->var = value;
            printk("Write value '%d' to variable '%s' succussfully\r\n", value, argv[3]);

            break;

        case 3:
            if (kstrcmp(argv[1], "-r"))
                goto fail;
            
            sptr_var = term_variable_find_by_name(argv[2]);
            if (!sptr_var || !sptr_var->var)
            {
                printk("No variable named '%s'\r\n", argv[2]);
                break;
            }

            printk("Variable '%s' value = %d\r\n", argv[2], *sptr_var->var);
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
 * @brief   cmd 'value': help function
 * @param   none
 * @retval  none
 * @note    none
 */
static void term_cmd_value_help(void)
{
    printk("usage: value -r [value_name], or value -w [value] [value_name]\r\n");
}

/*!
 * @brief   cmd 'value' init and add
 * @param   none
 * @retval  none
 * @note    none
 */
void term_cmd_add_value(void)
{
    struct term_cmd *sptr_cmd;

    sptr_cmd = term_cmd_allocate("value", GFP_KERNEL);
    if (!isValid(sptr_cmd))
        return;

    sptr_cmd->do_excute = term_cmd_value;
    sptr_cmd->help = term_cmd_value_help;

    term_cmd_add(sptr_cmd);
}
