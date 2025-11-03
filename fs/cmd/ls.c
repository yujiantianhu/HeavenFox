/*
 * Terminal Core API: File list
 *
 * File Name:   ls.c
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
#include <platform/base/fwk_fcntl.h>
#include <fs/fs_cmd.h>
#include <fs/fs_intr.h>
#include <term/term.h>

/*!< The defines */


/*!< The globals */


/*!< The functions */

/*!< API functions */
/*!
 * @brief   cmd 'ls': excute function
 * @param   sptr_cmd, argc, argv
 * @retval  errno
 * @note    none
 */
static kint32_t term_cmd_list_file(struct term_cmd *sptr_cmd, kint32_t argc, kchar_t **argv)
{
    kchar_t *list_path = mr_nullptr;

    switch (argc)
    {
        case 1:
            list_path = mr_fs_get_path_ptr();
            break;

        case 2:
            list_path = argv[1];
            break;

        default: 
            goto fail;
    }

    /*!< Try to open disk and read list */
    struct fs_list *sptr_dir = dir_open(list_path, "*", O_RDONLY);
    if (isValid(sptr_dir))
    {
        struct fs_item *sptr_item;

        foreach_dir_item(sptr_item, sptr_dir)
            printk("%-8s%-8s | %s\r\n", "gendisk", (sptr_item->type == NR_FS_ITEM_DIR) ? "dir" : "file", sptr_item->name);

        dir_close(sptr_dir);
    }
    /*!< Not a disk, perhaps virtual */
    else
    {
        struct fwk_kset *sptr_kset;
        struct fwk_kobject *sptr_kobj;

        sptr_kset = fwk_find_kset_by_path(mr_nullptr, (const kchar_t *)list_path);
        if (!isValid(sptr_kset))
        {
            printk("path \'%s\' is not exists!\r\n", list_path);
            return -ER_CHECKERR;
        }

        foreach_next_kobject(sptr_kset, sptr_kobj)
            printk("%-8s%-8s | %s\r\n", "virtual", sptr_kobj->is_dir ? "dir" : "file", sptr_kobj->name);
    }

    return ER_NORMAL;

fail:
    printk("argument error, try entering \'%s --help\' to get usage\r\n", argv[0]);
    return -ER_FAULT;
}

/*!
 * @brief   cmd 'ls': help function
 * @param   none
 * @retval  none
 * @note    none
 */
static void term_cmd_ls_help(void)
{
    printk("usage: ls, or ls [path]\r\n");
}

/*!
 * @brief   cmd 'ls' init and add
 * @param   none
 * @retval  none
 * @note    none
 */
void term_cmd_add_ls(void)
{
    struct term_cmd *sptr_cmd;

    sptr_cmd = term_cmd_allocate("ls", GFP_KERNEL);
    if (!isValid(sptr_cmd))
        return;

    sptr_cmd->do_excute = term_cmd_list_file;
    sptr_cmd->help = term_cmd_ls_help;

    term_cmd_add(sptr_cmd);
}
