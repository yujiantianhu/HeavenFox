/*
 * Platform File System: Inode
 *
 * File Name:   fwk_inode.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.29
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_inode.h>

/*!< API function */
/*!
 * @brief   fwk_chrdev_open
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t fwk_chrdev_open(struct fwk_inode *sprt_inode, struct fwk_file *sprt_file)
{
	struct fwk_cdev *sprt_cdev;
	kuint32_t devNum;

	devNum = sprt_inode->r_dev;
/*
	if (devNum < DEVICE_MAJOR_BASE)
		goto fail;
*/

	sprt_cdev = (struct fwk_cdev *)fwk_kobj_lookUp(sprt_fwk_chrdev_map, devNum);
	if (!isValid(sprt_cdev))
		goto fail;

	sprt_inode->sprt_cdev = sprt_cdev;

	/*!< Replace the device operate function */
	sprt_file->sprt_foprts = sprt_cdev->sprt_oprts;
	if (sprt_file->sprt_foprts->open)
		return sprt_file->sprt_foprts->open(sprt_inode, sprt_file);

	return NR_isWell;

fail:
	return -NR_isUnvalid;
}

/*!
 * @brief   fwk_chrdev_close
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t fwk_chrdev_close(struct fwk_inode *sprt_inode, struct fwk_file *sprt_file)
{
	sprt_inode->sprt_cdev = mrt_nullptr;

	return NR_isWell;
}

static struct fwk_file_oprts sgrt_fwk_inode_def_chrfoprts =
{
	.open	= fwk_chrdev_open,
	.close	= fwk_chrdev_close,
};

/*!
 * @brief   fwk_blkdev_open
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t fwk_blkdev_open(struct fwk_inode *sprt_inode, struct fwk_file *sprt_file)
{
	return NR_isWell;
}

/*!
 * @brief   fwk_blkdev_close
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t fwk_blkdev_close(struct fwk_inode *sprt_inode, struct fwk_file *sprt_file)
{
	return NR_isWell;
}

static struct fwk_file_oprts sgrt_fwk_inode_def_blkfoprts =
{
	.open	= fwk_blkdev_open,
	.close	= fwk_blkdev_close,
};

/*!
 * @brief   fwk_netdev_open
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t fwk_netdev_open(struct fwk_inode *sprt_inode, struct fwk_file *sprt_file)
{
	return NR_isWell;
}

/*!
 * @brief   fwk_netdev_close
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t fwk_netdev_close(struct fwk_inode *sprt_inode, struct fwk_file *sprt_file)
{
	return NR_isWell;
}

static struct fwk_file_oprts sgrt_fwk_inode_def_netfoprts =
{
	.open	= fwk_netdev_open,
	.close	= fwk_netdev_close,
};

static struct fwk_inode sgrt_fwk_inode_stdio[DEVICE_MAJOR_BASE] =
{
	{ .fileName = "stdin",	.r_dev = 0, .sprt_next = &sgrt_fwk_inode_stdio[1],	.sprt_foprts = &sgrt_fwk_inode_def_chrfoprts },
	{ .fileName	= "stdout",	.r_dev = 1,	.sprt_next = &sgrt_fwk_inode_stdio[2],	.sprt_foprts = &sgrt_fwk_inode_def_chrfoprts },
	{ .fileName	= "error", 	.r_dev = 2, .sprt_next = mrt_nullptr,				.sprt_foprts = &sgrt_fwk_inode_def_chrfoprts }
};

static struct fwk_inode sgrt_fwk_inode =
{
	.fileName		= "inode",
	.r_dev			= 0,

	.sprt_next		= &sgrt_fwk_inode_stdio[0],
	.sprt_foprts	= mrt_nullptr,
};

/*!
 * @brief   fwk_mk_inode
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t fwk_mk_inode(kstring_t *name, kuint32_t type, kuint32_t devNum)
{
	struct fwk_inode *sprt_head;
	struct fwk_inode *sprt_tail;
	struct fwk_inode *sprt_inode;

	if (!name)
		return -NR_isAnyErr;

	sprt_head = &sgrt_fwk_inode;

	/*!< Retrieval: Make sure the device name is unique */
	foreach_list_odd_next(sprt_head, sprt_inode)
	{
		/*!< Heavy name */
		if (!strcmp(name, sprt_inode->fileName))
			return -NR_isAnyErr;

		/*!< Get the last inode */
		sprt_tail = sprt_inode;
	}

	sprt_inode = (struct fwk_inode *)kzalloc(sizeof(struct fwk_inode), GFP_KERNEL);
	if (!isValid(sprt_inode))
		return -NR_isMemErr;

	sprt_inode->r_dev = devNum;
	sprt_inode->sprt_next = mrt_nullptr;
	do_string_n_copy(sprt_inode->fileName, name, sizeof(sprt_inode->fileName));

	switch(type)
	{
		case NR_TYPE_CHRDEV:
			sprt_inode->sprt_foprts	= &sgrt_fwk_inode_def_chrfoprts;
			break;

		case NR_TYPE_BLKDEV:
			sprt_inode->sprt_foprts	= &sgrt_fwk_inode_def_blkfoprts;
			break;

		case NR_TYPE_NETDEV:
			sprt_inode->sprt_foprts	= &sgrt_fwk_inode_def_netfoprts;
			break;

		default:
			break;
	};

	/*!< Updated to the last inode */
	mrt_list_add_tail(sprt_tail, sprt_inode);

	return NR_isWell;
}

/*!
 * @brief   fwk_rm_inode
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t fwk_rm_inode(kstring_t *name)
{
	struct fwk_inode *sprt_head;
	struct fwk_inode *sprt_prev;
	struct fwk_inode *sprt_inode;

	if (!name)
		return -NR_isAnyErr;

	sprt_head = &sgrt_fwk_inode;
	sprt_prev = mrt_nullptr;

	/*!< Search: By device name matching */
	foreach_list_odd_next(sprt_head, sprt_inode)
	{
		/*!< The name matches successfully (the device name cannot be the same) */
		if (!strcmp(name, sprt_inode->fileName))
			break;

		sprt_prev = sprt_inode;
	}

	/*!< Not found */
	if (!isValid(sprt_inode))
		return -NR_isArrayOver;

	/*!< Deletes the current node */
	mrt_list_del_any(sprt_prev, sprt_inode);

	/*!< Free up resources */
	kfree(sprt_inode);

	return NR_isWell;
}

/*!
 * @brief   fwk_inode_find
 * @param   none
 * @retval  none
 * @note    none
 */
struct fwk_inode *fwk_inode_find(kstring_t *name)
{
	struct fwk_inode *sprt_head;
	struct fwk_inode *sprt_inode;

	if (!name)
		return mrt_nullptr;

	sprt_head = &sgrt_fwk_inode;

	/*!< Search: By device name matching */
	foreach_list_odd_next(sprt_head, sprt_inode)
	{
		/*!< The name matches successfully */
		if (!strcmp(name, sprt_inode->fileName))
			break;
	}

	return !isValid(sprt_inode) ? mrt_nullptr : sprt_inode;
}

/*!
 * @brief   fwk_device_create
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t fwk_device_create(kuint32_t type, kuint32_t devNum, kstring_t *name, ...)
{
	kstring_t node[INODE_ADD_PREX_LEN];
	va_list ptr_list;

	if (!name)
		return -NR_isAnyErr;

	sprintk(node, "/dev/");

	va_start(ptr_list, name);
	do_fmt_convert(node + 5, mrt_nullptr, name, ptr_list, INODE_ADD_PREX_LEN - 5);
	va_end(ptr_list);

	return fwk_mk_inode(node, type, devNum);
}

/*!
 * @brief   fwk_device_destroy
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t fwk_device_destroy(kstring_t *name, ...)
{
	kstring_t node[INODE_ADD_PREX_LEN];
	va_list ptr_list;

	if (!name)
		return -NR_isAnyErr;

	sprintk(node, "/dev/");

	va_start(ptr_list, name);
	do_fmt_convert(node + 5, mrt_nullptr, name, ptr_list, INODE_ADD_PREX_LEN - 5);
	va_end(ptr_list);

	return fwk_rm_inode(node);
}
