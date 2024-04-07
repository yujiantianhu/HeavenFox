/*
 * Platform File System Controller
 *
 * File Name:   hal_fs.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.29
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_FS_H_
#define __HAL_FS_H_

/*!< The includes */
#include <platform/hal_basic.h>
#include <platform/hal_inode.h>
#include <platform/hal_uaccess.h>

/*!< The defines */
#define RET_INODE_FROM_FILE(file)								(file->sprt_inode)
#define RET_PRIVATE_DRV_DATA(file)								(file->private_data)

struct hal_file
{
	kuint32_t mode;

	struct hal_inode *sprt_inode;
	struct hal_file_oprts *sprt_foprts;

	/*!< All open files form a list */
	struct list_head sgrt_list;

	void *private_data;
};

/*!< Device operation API */
struct hal_file_oprts
{
	ksint32_t (*open) (struct hal_inode *, struct hal_file *);
	ksint32_t (*close) (struct hal_inode *, struct hal_file *);
	kssize_t (*write) (struct hal_file *, const ksbuffer_t *, kssize_t);
	kssize_t (*read) (struct hal_file *, ksbuffer_t *, kssize_t);
	ksint32_t (*unlocked_ioctl) (struct hal_file *, kuint32_t, kuaddr_t);
	ksint32_t (*compat_ioctl) (struct hal_file *, kuint32_t, kuaddr_t);
	ksint32_t (*mmap) (struct hal_file *, struct hal_vm_area *);
};

/*!< The functions */
TARGET_EXT struct hal_file *hal_do_filp_open(kstring_t *name, kuint32_t mode);
TARGET_EXT void hal_do_filp_close(struct hal_file *sprt_file);

#endif /*!< __HAL_FS_H_ */
