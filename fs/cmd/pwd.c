/*
 * Terminal Core API: Show Current Path
 *
 * File Name:   pwd.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.11.02
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/base/fwk_kobj.h>
#include <fs/fs_cmd.h>
#include <term/term.h>

/*!< The defines */


/*!< The globals */


/*!< The functions */

/*!< API functions */
/*!
 * @brief   cmd 'pwd': excute function
 * @param   sptr_cmd, argc, argv
 * @retval  errno
 * @note    none
 */
static kint32_t term_cmd_current_path(struct term_cmd *sptr_cmd, kint32_t argc, kchar_t **argv)
{
    switch (argc)
    {
        case 1:
            if (mr_fs_get_path_ptr())
                printk("%s\r\n", mr_fs_get_path_ptr());
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
 * @brief   cmd 'pwd': help function
 * @param   none
 * @retval  none
 * @note    none
 */
static void term_cmd_pwd_help(void)
{
    printk("usage: pwd\r\n");
}

/*!
 * @brief   cmd 'pwd' init and add
 * @param   none
 * @retval  none
 * @note    none
 */
void term_cmd_add_pwd(void)
{
    struct term_cmd *sptr_cmd;

    sptr_cmd = term_cmd_allocate("pwd", GFP_KERNEL);
    if (!isValid(sptr_cmd))
        return;

    sptr_cmd->do_excute = term_cmd_current_path;
    sptr_cmd->help = term_cmd_pwd_help;

    term_cmd_add(sptr_cmd);
}
