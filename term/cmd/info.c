/*
 * Terminal Core API: Command info
 *
 * File Name:   info.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.12.22
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
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
 * @brief   cmd 'info': excute function
 * @param   sptr_cmd, argc, argv
 * @retval  errno
 * @note    none
 */
static kint32_t term_cmd_show_info(struct term_cmd *sptr_cmd, kint32_t argc, kchar_t **argv)
{
    switch (argc)
    {
        case 1:
            printk("----------------------------------------------------\r\n");
            printk("        Kernel:     HeavenFox OS                    \r\n");
            printk("        Author:     Yang Yujun                      \r\n");
            printk("        Country:    China                           \r\n");
            printk("        Province:   GuangXi                         \r\n");
            printk("        E-mail:     <yujiantianhu@163.com>          \r\n");
            printk("----------------------------------------------------\r\n");

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
 * @brief   cmd 'info': help function
 * @param   none
 * @retval  none
 * @note    none
 */
static void term_cmd_info_help(void)
{
    printk("usage: info\r\n");
}

/*!
 * @brief   cmd 'info' init and add
 * @param   none
 * @retval  none
 * @note    none
 */
void term_cmd_add_info(void)
{
    struct term_cmd *sptr_cmd;

    sptr_cmd = term_cmd_allocate("info", GFP_KERNEL);
    if (!isValid(sptr_cmd))
        return;

    sptr_cmd->do_excute = term_cmd_show_info;
    sptr_cmd->help = term_cmd_info_help;

    term_cmd_add(sptr_cmd);
}
