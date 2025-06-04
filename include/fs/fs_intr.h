/*
 * General FatFs Interface
 *
 * File Name:   fs_intr.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.11.03
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FS_INTR_H
#define __FS_INTR_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/generic.h>
#include <common/api_string.h>
#include <common/io_stream.h>
#include <platform/fwk_kobj.h>
#include <platform/block/fwk_gendisk.h>

/*!< The defines */
struct fs_stream
{
    kchar_t *full_name;
    kuint32_t mode;
    struct fwk_inode *sptr_dnode;
    struct fwk_block_device_oprts *sptr_bops;

    void *private_data;
};

enum __ERT_FS_ITEM_TYPE
{
    NR_FS_ITEM_DIR = 0,
    NR_FS_ITEM_FILE,
};

struct fs_item
{
    kchar_t *name;
    kuint32_t type;
    kusize_t size;
    struct list_head sgtc_link;

    void *private_data;
};

struct fs_list
{
    kchar_t *path;
    kuint32_t mode;

    kuint32_t file_num;
    struct list_head sgtc_files;

    kuint32_t dir_num;
    struct list_head sgtc_dirs;

    struct fwk_inode *sptr_dnode;
    kint32_t (*readdir)(struct fs_list *sptr_list, const kchar_t *pattern);

    void *private_data;
};
#define mr_dir_items_num(sptr_list)                 ((sptr_list)->dir_num + (sptr_list)->file_num)

/*!< The functions */
extern struct fs_list *dir_open(const kchar_t *path, const kchar_t *pattern, kuint32_t mode);
extern void dir_close(struct fs_list *sptr_fs);
extern kint32_t dir_flush(struct fs_list *sptr_fs, const kchar_t *pattern);
extern struct fs_item *dir_read_item(struct fs_list *sptr_fs, struct fs_item *sptr_prev);

extern struct fs_stream *file_open(const kchar_t *name, kuint32_t mode);
extern void file_close(struct fs_stream *sptr_fs);
extern kssize_t file_write(struct fs_stream *sptr_fs, const void *buf, kusize_t size);
extern kssize_t file_read(struct fs_stream *sptr_fs, void *buf, kusize_t size);
extern kssize_t file_size(struct fs_stream *sptr_fs);
extern kint32_t file_lseek(struct fs_stream *sptr_fs, kuint32_t offset);
extern kssize_t file_tell(struct fs_stream *sptr_fs);

/*!< The defines */
#define foreach_dir_item(_item, _fs)    \
    for ((_item) = dir_read_item(_fs, mr_nullptr); (_item); (_item) = dir_read_item(_fs, _item))

#ifdef __cplusplus
    }
#endif

#endif /* __FS_INTR_H */
