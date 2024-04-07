/*
 * Platform File System Controller
 *
 * File Name:   hal_fs.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.29
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/hal_fs.h>

/*!< API function */
/*!
 * @brief   hal_do_filp_open
 * @param   none
 * @retval  none
 * @note    none
 */
struct hal_file *hal_do_filp_open(kstring_t *name, kuint32_t mode)
{
	struct hal_file  *sprt_file;
	struct hal_inode *sprt_inode;
	ksint32_t retval;

	sprt_inode = hal_inode_find(name);
	if (!mrt_isValid(sprt_inode))
	{
		goto fail1;
	}

	sprt_file = (struct hal_file *)kzalloc(sizeof(struct hal_file), GFP_KERNEL);
	if (!mrt_isValid(sprt_file))
	{
		goto fail1;
	}

	sprt_file->sprt_inode = sprt_inode;
	sprt_file->sprt_foprts = sprt_inode->sprt_foprts;
	if (mrt_isValid(sprt_file->sprt_foprts->open))
	{
		retval = sprt_file->sprt_foprts->open(sprt_inode, sprt_file);
		if (mrt_isErr(retval))
		{
			goto fail2;
		}
	}

	return sprt_file;

fail2:
	kfree(sprt_file);

fail1:
	return mrt_nullptr;
}

/*!
 * @brief   hal_do_filp_close
 * @param   none
 * @retval  none
 * @note    none
 */
void hal_do_filp_close(struct hal_file *sprt_file)
{
	struct hal_inode *sprt_inode;

	if (!mrt_isValid(sprt_file))
	{
		return;
	}

	sprt_inode = sprt_file->sprt_inode;

	/*!< Close device */
	if (mrt_isValid(sprt_file->sprt_foprts->close))
	{
		sprt_file->sprt_foprts->close(sprt_inode, sprt_file);
	}

	/*!< 
	 * Legacy bug: 
	 * If the file is not closed after opening, and the inode node is suddenly deleted, 
	 * what should we do with the file? How to release the occupied fd? 
	 */
	if (mrt_isValid(sprt_inode->sprt_foprts->close))
	{
		sprt_inode->sprt_foprts->close(sprt_inode, sprt_file);
	}

	kfree(sprt_file);
}
