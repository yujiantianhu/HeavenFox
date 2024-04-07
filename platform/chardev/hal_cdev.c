/*
 * Character Device Interface
 *
 * File Name:   hal_cdev.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.18
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/hal_cdev.h>

/*!< API function */
/*!
 * @brief   cdev initial
 * @param   sprt_cdev, sprt_oprts
 * @retval  errno
 * @note    none
 */
ksint32_t hal_cdev_init(struct hal_cdev *sprt_cdev, const struct hal_file_oprts *sprt_oprts)
{
	if (!mrt_isValid(sprt_cdev))
	{
		return mrt_retval(Ert_isMemErr);
	}

	memset(sprt_cdev, 0, sizeof(struct hal_cdev));
	sprt_cdev->sprt_oprts = (struct hal_file_oprts *)sprt_oprts;
	sprt_cdev->sprt_next = mrt_nullptr;

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   cdev alloc
 * @param   sprt_oprts
 * @retval  errno
 * @note    none
 */
struct hal_cdev *hal_cdev_alloc(const struct hal_file_oprts *sprt_oprts)
{
	struct hal_cdev *sprt_cdev;

	sprt_cdev = (struct hal_cdev *)kzalloc(sizeof(struct hal_cdev), GFP_KERNEL);
	if (!mrt_isValid(sprt_cdev))
	{
		goto fail2;
	}

	if (mrt_isErr(hal_cdev_init(sprt_cdev, sprt_oprts)))
	{
		goto fail1;
	}

	return sprt_cdev;

fail1:
	if (mrt_isValid(sprt_cdev))
	{
		kfree(sprt_cdev);
	}

fail2:
	return mrt_nullptr;
}

/*!
 * @brief   cdev add
 * @param   sprt_cdev, devNum, count
 * @retval  errno
 * @note    none
 */
ksint32_t hal_cdev_add(struct hal_cdev *sprt_cdev, kuint32_t devNum, kuint32_t count)
{
	if (!mrt_isValid(sprt_cdev))
	{
		return mrt_retval(Ert_isMemErr);
	}

	sprt_cdev->devNum = devNum;
	sprt_cdev->count = count;

	return hal_kobj_map(sprt_hal_chrdev_map, devNum, count, sprt_cdev);
}

/*!
 * @brief   cdev delete
 * @param   sprt_cdev
 * @retval  errno
 * @note    none
 */
ksint32_t hal_cdev_del(struct hal_cdev *sprt_cdev)
{
	if (!mrt_isValid(sprt_cdev))
	{
		return mrt_retval(Ert_isMemErr);
	}

	hal_kobj_unmap(sprt_hal_chrdev_map, sprt_cdev->devNum, sprt_cdev->count);

	return mrt_retval(Ert_isWell);
}
