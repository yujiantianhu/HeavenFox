/*
 * Character Device Interface
 *
 * File Name:   fwk_cdev.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.18
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_cdev.h>

/*!< API function */
/*!
 * @brief   cdev initial
 * @param   sprt_cdev, sprt_oprts
 * @retval  errno
 * @note    none
 */
kint32_t fwk_cdev_init(struct fwk_cdev *sprt_cdev, const struct fwk_file_oprts *sprt_oprts)
{
	if (!isValid(sprt_cdev))
		return -NR_IS_NOMEM;

	memset(sprt_cdev, 0, sizeof(struct fwk_cdev));
	sprt_cdev->sprt_oprts = (struct fwk_file_oprts *)sprt_oprts;
	sprt_cdev->sprt_next = mrt_nullptr;

	return NR_IS_NORMAL;
}

/*!
 * @brief   cdev alloc
 * @param   sprt_oprts
 * @retval  errno
 * @note    none
 */
struct fwk_cdev *fwk_cdev_alloc(const struct fwk_file_oprts *sprt_oprts)
{
	struct fwk_cdev *sprt_cdev;

	sprt_cdev = (struct fwk_cdev *)kzalloc(sizeof(struct fwk_cdev), GFP_KERNEL);
	if (!isValid(sprt_cdev))
		goto fail2;

	if (fwk_cdev_init(sprt_cdev, sprt_oprts))
		goto fail1;

	return sprt_cdev;

fail1:
	if (isValid(sprt_cdev))
		kfree(sprt_cdev);

fail2:
	return mrt_nullptr;
}

/*!
 * @brief   cdev add
 * @param   sprt_cdev, devNum, count
 * @retval  errno
 * @note    none
 */
kint32_t fwk_cdev_add(struct fwk_cdev *sprt_cdev, kuint32_t devNum, kuint32_t count)
{
	if (!isValid(sprt_cdev))
		return -NR_IS_NOMEM;

	sprt_cdev->devNum = devNum;
	sprt_cdev->count = count;

	return fwk_kobj_map(sprt_fwk_chrdev_map, devNum, count, sprt_cdev);
}

/*!
 * @brief   cdev delete
 * @param   sprt_cdev
 * @retval  errno
 * @note    none
 */
kint32_t fwk_cdev_del(struct fwk_cdev *sprt_cdev)
{
	if (!isValid(sprt_cdev))
		return -NR_IS_NOMEM;

	fwk_kobj_unmap(sprt_fwk_chrdev_map, sprt_cdev->devNum, sprt_cdev->count);

	return NR_IS_NORMAL;
}
