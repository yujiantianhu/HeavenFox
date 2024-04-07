/*
 * Platform File System: Inode
 *
 * File Name:   hal_inode.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.29
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_INODE_H_
#define __HAL_INODE_H_

/*!< The includes */
#include <platform/hal_basic.h>
#include <platform/hal_fs.h>
#include <platform/hal_cdev.h>

/*!< The defines */
#define INODE_PREX_LEN									5
#define INODE_ADD_PREX_LEN								(DEVICE_NAME_LEN + INODE_PREX_LEN)

#define RET_INODE_MAJOR(inode)							GET_DEV_MAJOR(inode->r_dev)
#define RET_INODE_MINOR(inode)							GET_DEV_MINOR(inode->r_dev)

struct hal_inode
{
	kstring_t fileName[INODE_ADD_PREX_LEN];
	kuint32_t r_dev;

	struct hal_inode *sprt_next;
	struct hal_file_oprts *sprt_foprts;

	union
	{
		struct hal_cdev *sprt_cdev;
	};
};

/*!< The functions */
TARGET_EXT ksint32_t hal_mk_inode(kstring_t *name, kuint32_t type, kuint32_t devNum);
TARGET_EXT ksint32_t hal_rm_inode(kstring_t *name);
TARGET_EXT struct hal_inode *hal_inode_find(kstring_t *name);
TARGET_EXT ksint32_t hal_device_create(kuint32_t type, kuint32_t devNum, kstring_t *name, ...);
TARGET_EXT ksint32_t hal_device_destroy(kstring_t *name, ...);

#endif /*!< __HAL_INODE_H_ */
