/*
 * Platform File System: Inode
 *
 * File Name:   hal_inode.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.29
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/hal_basic.h>
#include <platform/hal_inode.h>

/*!< API function */
/*!
 * @brief   hal_chrdev_open
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_chrdev_open(struct hal_inode *sprt_inode, struct hal_file *sprt_file)
{
	struct hal_cdev *sprt_cdev;
	kuint32_t devNum;

	devNum = sprt_inode->r_dev;
/*
	if (devNum < DEVICE_MAJOR_BASE)
	{
		goto fail;
	}
*/

	sprt_cdev = (struct hal_cdev *)hal_kobj_lookUp(sprt_hal_chrdev_map, devNum);
	if (!mrt_isValid(sprt_cdev))
	{
		goto fail;
	}

	sprt_inode->sprt_cdev = sprt_cdev;

	/*!< Replace the device operate function */
	sprt_file->sprt_foprts = sprt_cdev->sprt_oprts;
	if (mrt_isValid(sprt_file->sprt_foprts->open))
	{
		return sprt_file->sprt_foprts->open(sprt_inode, sprt_file);
	}

	return mrt_retval(Ert_isWell);

fail:
	return mrt_retval(Ert_isUnvalid);
}

/*!
 * @brief   hal_chrdev_close
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_chrdev_close(struct hal_inode *sprt_inode, struct hal_file *sprt_file)
{
	sprt_inode->sprt_cdev = mrt_nullptr;

	return mrt_retval(Ert_isWell);
}

static struct hal_file_oprts sgrt_hal_inode_def_chrfoprts =
{
	.open	= hal_chrdev_open,
	.close	= hal_chrdev_close,
};

/*!
 * @brief   hal_blkdev_open
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_blkdev_open(struct hal_inode *sprt_inode, struct hal_file *sprt_file)
{
	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   hal_blkdev_close
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_blkdev_close(struct hal_inode *sprt_inode, struct hal_file *sprt_file)
{
	return mrt_retval(Ert_isWell);
}

static struct hal_file_oprts sgrt_hal_inode_def_blkfoprts =
{
	.open	= hal_blkdev_open,
	.close	= hal_blkdev_close,
};

/*!
 * @brief   hal_netdev_open
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_netdev_open(struct hal_inode *sprt_inode, struct hal_file *sprt_file)
{
	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   hal_netdev_close
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_netdev_close(struct hal_inode *sprt_inode, struct hal_file *sprt_file)
{
	return mrt_retval(Ert_isWell);
}

static struct hal_file_oprts sgrt_hal_inode_def_netfoprts =
{
	.open	= hal_netdev_open,
	.close	= hal_netdev_close,
};

static struct hal_inode sgrt_hal_inode_stdio[DEVICE_MAJOR_BASE] =
{
	{ .fileName = "stdin",	.r_dev = 0, .sprt_next = &sgrt_hal_inode_stdio[1],	.sprt_foprts = &sgrt_hal_inode_def_chrfoprts },
	{ .fileName	= "stdout",	.r_dev = 1,	.sprt_next = &sgrt_hal_inode_stdio[2],	.sprt_foprts = &sgrt_hal_inode_def_chrfoprts },
	{ .fileName	= "error", 	.r_dev = 2, .sprt_next = mrt_nullptr,				.sprt_foprts = &sgrt_hal_inode_def_chrfoprts }
};

static struct hal_inode sgrt_hal_inode =
{
	.fileName		= "inode",
	.r_dev			= 0,

	.sprt_next		= &sgrt_hal_inode_stdio[0],
	.sprt_foprts	= mrt_nullptr,
};

/*!
 * @brief   hal_mk_inode
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t hal_mk_inode(kstring_t *name, kuint32_t type, kuint32_t devNum)
{
	struct hal_inode *sprt_head;
	struct hal_inode *sprt_tail;
	struct hal_inode *sprt_inode;

	if (!mrt_isValid(name))
	{
		return mrt_retval(Ert_isAnyErr);
	}

	sprt_head = &sgrt_hal_inode;

	/*!< Retrieval: Make sure the device name is unique */
	foreach_list_odd_next(sprt_head, sprt_inode)
	{
		/*!< Heavy name */
		if (!strcmp(name, sprt_inode->fileName))
		{
			return mrt_retval(Ert_isAnyErr);
		}

		/*!< Get the last inode */
		sprt_tail = sprt_inode;
	}

	sprt_inode = (struct hal_inode *)kzalloc(sizeof(struct hal_inode), GFP_KERNEL);
	if (!mrt_isValid(sprt_inode))
	{
		return mrt_retval(Ert_isMemErr);
	}

	sprt_inode->r_dev = devNum;
	sprt_inode->sprt_next = mrt_nullptr;
	do_string_n_copy(sprt_inode->fileName, name, sizeof(sprt_inode->fileName));

	switch(type)
	{
		case Ert_HAL_TYPE_CHRDEV:
			sprt_inode->sprt_foprts	= &sgrt_hal_inode_def_chrfoprts;
			break;

		case Ert_HAL_TYPE_BLKDEV:
			sprt_inode->sprt_foprts	= &sgrt_hal_inode_def_blkfoprts;
			break;

		case Ert_HAL_TYPE_NETDEV:
			sprt_inode->sprt_foprts	= &sgrt_hal_inode_def_netfoprts;
			break;

		default:
			break;
	};

	/*!< Updated to the last inode */
	mrt_list_add_tail(sprt_tail, sprt_inode);

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   hal_rm_inode
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t hal_rm_inode(kstring_t *name)
{
	struct hal_inode *sprt_head;
	struct hal_inode *sprt_prev;
	struct hal_inode *sprt_inode;

	if (!mrt_isValid(name))
	{
		return mrt_retval(Ert_isAnyErr);
	}

	sprt_head = &sgrt_hal_inode;
	sprt_prev = mrt_nullptr;

	/*!< Search: By device name matching */
	foreach_list_odd_next(sprt_head, sprt_inode)
	{
		/*!< The name matches successfully (the device name cannot be the same) */
		if (!strcmp(name, sprt_inode->fileName))
		{
			break;
		}

		sprt_prev = sprt_inode;
	}

	/*!< Not found */
	if (!mrt_isValid(sprt_inode))
	{
		return mrt_retval(Ert_isArrayOver);
	}

	/*!< Deletes the current node */
	mrt_list_del_any(sprt_prev, sprt_inode);

	/*!< Free up resources */
	kfree(sprt_inode);

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   hal_inode_find
 * @param   none
 * @retval  none
 * @note    none
 */
struct hal_inode *hal_inode_find(kstring_t *name)
{
	struct hal_inode *sprt_head;
	struct hal_inode *sprt_inode;

	if (!mrt_isValid(name))
	{
		return mrt_nullptr;
	}

	sprt_head = &sgrt_hal_inode;

	/*!< Search: By device name matching */
	foreach_list_odd_next(sprt_head, sprt_inode)
	{
		/*!< The name matches successfully */
		if (!strcmp(name, sprt_inode->fileName))
		{
			break;
		}
	}

	return !mrt_isValid(sprt_inode) ? mrt_nullptr : sprt_inode;
}

/*!
 * @brief   hal_device_create
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t hal_device_create(kuint32_t type, kuint32_t devNum, kstring_t *name, ...)
{
	kstring_t node[INODE_ADD_PREX_LEN];
	va_list ptr_list;

	if (!mrt_isValid(name))
	{
		return mrt_retval(Ert_isAnyErr);
	}

	sprintk(node, "/dev/");

	va_start(ptr_list, name);
	do_fmt_convert(node + 5, mrt_nullptr, name, ptr_list);
	va_end(ptr_list);

	return hal_mk_inode(node, type, devNum);
}

/*!
 * @brief   hal_device_destroy
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t hal_device_destroy(kstring_t *name, ...)
{
	kstring_t node[INODE_ADD_PREX_LEN];
	va_list ptr_list;

	if (!mrt_isValid(name))
	{
		return mrt_retval(Ert_isAnyErr);
	}

	sprintk(node, "/dev/");

	va_start(ptr_list, name);
	do_fmt_convert(node + 5, mrt_nullptr, name, ptr_list);
	va_end(ptr_list);

	return hal_rm_inode(node);
}
