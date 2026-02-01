/*
 * File System Command
 *
 * File Name:   fs_cmd.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.11.02
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

#include <fs/fs_intr.h>
#include <fs/fs_cmd.h>

/*!< The defines */

/*!< The globals */
kchar_t *g_fs_current_path;

/*!< The functions */

/*!< API function */
/*!
 * @brief   fs_cmd_init
 * @param   none
 * @retval  none
 * @note    file system command init
 */
kint32_t __plat_init fs_cmd_init(void)
{
    kchar_t *cur_path;

    cur_path = kmalloc(kstrlen("/") + 1, GFP_KERNEL);
    if (!isValid(cur_path))
    {
        print_err("Build file system current path failed, but standing it\r\n");
        return ER_NORMAL;
    }

    /*!< Default path: root */
    *cur_path = '/';
    *(cur_path + 1) = '\0';
    mr_fs_set_path_ptr(cur_path);

    /*!< Add command */
    term_cmd_add_ls();
    term_cmd_add_pwd();
    
    return ER_NORMAL;
}

/*!
 * @brief   fs_cmd_exit
 * @param   none
 * @retval  none
 * @note    file system command exit
 */
void __plat_exit fs_cmd_exit(void)
{
    if (mr_fs_get_path_ptr())
    {
        kfree(g_fs_current_path);
        mr_fs_set_path_ptr(mr_nullptr);
    }
}

IMPORT_ROOTFS_INIT(fs_cmd_init);
IMPORT_ROOTFS_EXIT(fs_cmd_exit);
