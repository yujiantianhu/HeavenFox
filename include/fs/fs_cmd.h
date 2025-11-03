/*
 * File System Command
 *
 * File Name:   fs_cmd.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.11.02
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FS_CMD_H
#define __FS_CMD_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/generic.h>

/*!< The defines */

/*!< The globals */
extern kchar_t *g_fs_current_path;

#define mr_fs_set_path_ptr(cur_path_ptr)                do { g_fs_current_path = (cur_path_ptr); } while (0)
#define mr_fs_get_path_ptr()                            (g_fs_current_path)

/*!< The functions */
extern void term_cmd_add_ls(void);
extern void term_cmd_add_pwd(void);

#ifdef __cplusplus
    }
#endif

#endif /* __FS_CMD_H */
