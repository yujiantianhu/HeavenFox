/*
 * Terminal Core API
 *
 * File Name:   term_core.c
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
static kchar_t g_term_cmdline_argv[TERM_MSG_RECV_LEN];

/*!< top command: help */
static DECLARE_LIST_HEAD(sgtc_term_cmd_lists);
 
/*!< The functions */
/*!
 * @brief   find command by name
 * @param   name
 * @retval  term_cmd
 * @note    none
 */
struct term_cmd *term_cmd_find_by_name(kchar_t *name)
{
    struct term_cmd *sptr_cmd;

    if (!name || !(*name))
        goto fail;

    foreach_list_next_entry(sptr_cmd, &sgtc_term_cmd_lists, sgtc_link)
    {
        if (!kstrcmp(sptr_cmd->name, name))
            return sptr_cmd;
    }

fail:
    return mr_nullptr;
}

/*!
 * @brief   create new command
 * @param   name: command' name
 * @retval  term_cmd
 * @note    none
 */
struct term_cmd *term_cmd_allocate(const kchar_t *name, nrt_gfp_t gfp_mask)
{
    struct term_cmd *sptr_cmd;
    kssize_t size;

    sptr_cmd = kmalloc(sizeof(*sptr_cmd), gfp_mask);
    if (!isValid(sptr_cmd))
        return sptr_cmd;

    size = kstrlen(name);
    memcpy(sptr_cmd->name, name, size);
    sptr_cmd->name[size] = '\0';

    sptr_cmd->do_excute = mr_nullptr;
    sptr_cmd->help = mr_nullptr;
    init_list_head(&sptr_cmd->sgtc_link);

    return sptr_cmd;
}

/*!
 * @brief   release command
 * @param   sptr_cmd
 * @retval  none
 * @note    none
 */
void term_cmd_free(struct term_cmd *sptr_cmd)
{
    if (!mr_list_empty(&sptr_cmd->sgtc_link))
        term_cmd_del(sptr_cmd);

    kfree(sptr_cmd);
}

/*!
 * @brief   add command
 * @param   sptr_cmd
 * @retval  errno
 * @note    none
 */
kint32_t term_cmd_add(struct term_cmd *sptr_cmd)
{
    if ((!sptr_cmd->name) ||
        (*sptr_cmd->name == '\0') ||
        (kstrchr(sptr_cmd->name, ' ')))
        return -ER_INVALID;

    if (term_cmd_find_by_name(sptr_cmd->name))
        return -ER_EXISTED;

    list_head_add_tail(&sgtc_term_cmd_lists, &sptr_cmd->sgtc_link);
    return ER_NORMAL;
}

/*!
 * @brief   detach command
 * @param   sptr_cmd
 * @retval  none
 * @note    none
 */
void term_cmd_del(struct term_cmd *sptr_cmd)
{
    list_head_del(&sptr_cmd->sgtc_link);    
}

/*!
 * @brief   call command's handler
 * @param   argc: argument's number
 * @param   argv: arguments
 * @retval  none
 * @note    none
 */
void term_cmdline_excute(kint32_t argc, kchar_t **argv)
{
    struct term_cmd *sptr_cmd;

    if (argc < 1)
        return;

    foreach_list_next_entry(sptr_cmd, &sgtc_term_cmd_lists, sgtc_link)
    {
        if (!kstrcmp(sptr_cmd->name, argv[0]))
            goto succ;
    }

    printk("can not access the command!\r\n");
    return;

succ:
    if (sptr_cmd->do_excute)
        sptr_cmd->do_excute(sptr_cmd, argc, argv);
}

/*!
 * @brief   deal with per command
 * @param   cmdline
 * @retval  none
 * @note    none
 */
void term_cmdline_distribute(const kchar_t *cmdline)
{
    kchar_t *msg;
    kusize_t cmdlen;
    kint32_t argc = 0;
    kchar_t *argv[32];

    cmdlen = kstrlen(cmdline);
    if (!cmdlen)
        return;

    msg = &g_term_cmdline_argv[0];
    memcpy(msg, cmdline, cmdlen + 1);

    do {
        argv[argc++] = msg;

        msg = kstrchr(msg, ' ');
        if (msg)
            *(msg++) = '\0';

    } while (msg && *msg);

    term_cmdline_excute(argc, argv);
}

/*!< ----------------------------------------------------------- */
/*!
 * @brief   cmd 'help': excute function
 * @param   sptr_cmd, argc, argv
 * @retval  errno
 * @note    none
 */
static kint32_t term_cmd_show_help(struct term_cmd *sptr_cmd, kint32_t argc, kchar_t **argv)
{
    struct term_cmd *sptr_per;

    if (argc != 1)
        goto fail;

    printk("command list: >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n");
    foreach_list_next_entry(sptr_per, &sgtc_term_cmd_lists, sgtc_link)
    {
        if (sptr_per->help)
        {
            printk("\'%s\'> ", sptr_per->name);
            sptr_per->help();
        }
    }

    return ER_NORMAL;

fail:
    printk("argument is too many\r\n");
    return -ER_FAULT;
}

/*!
 * @brief   cmd 'help' init and add
 * @param   none
 * @retval  none
 * @note    none
 */
void term_cmd_add_help(void)
{
    struct term_cmd *sptr_cmd;

    sptr_cmd = term_cmd_allocate("help", GFP_KERNEL);
    if (!isValid(sptr_cmd))
        return;

    sptr_cmd->do_excute = term_cmd_show_help;
    sptr_cmd->help = mr_nullptr;

    term_cmd_add(sptr_cmd);
}