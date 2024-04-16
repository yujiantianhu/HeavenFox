/*
 * Platform File System: Inode
 *
 * File Name:   fwk_inode.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.29
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_INODE_H_
#define __FWK_INODE_H_

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_fs.h>
#include <platform/fwk_cdev.h>

/*!< The defines */
#define INODE_PREX_LEN									5
#define INODE_ADD_PREX_LEN								(DEVICE_NAME_LEN + INODE_PREX_LEN)

#define RET_INODE_MAJOR(inode)							GET_DEV_MAJOR(inode->r_dev)
#define RET_INODE_MINOR(inode)							GET_DEV_MINOR(inode->r_dev)

struct fwk_inode
{
	kstring_t fileName[INODE_ADD_PREX_LEN];
	kuint32_t r_dev;

	struct fwk_inode *sprt_next;
	struct fwk_file_oprts *sprt_foprts;

	union
	{
		struct fwk_cdev *sprt_cdev;
	};
};

/*!< The functions */
TARGET_EXT ksint32_t fwk_mk_inode(kstring_t *name, kuint32_t type, kuint32_t devNum);
TARGET_EXT ksint32_t fwk_rm_inode(kstring_t *name);
TARGET_EXT struct fwk_inode *fwk_inode_find(kstring_t *name);
TARGET_EXT ksint32_t fwk_device_create(kuint32_t type, kuint32_t devNum, kstring_t *name, ...);
TARGET_EXT ksint32_t fwk_device_destroy(kstring_t *name, ...);

#endif /*!< __FWK_INODE_H_ */