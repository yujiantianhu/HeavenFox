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
 * @param   sptr_cdev, sptr_oprts
 * @retval  errno
 * @note    none
 */
kint32_t fwk_cdev_init(struct fwk_cdev *sptr_cdev, const struct fwk_file_oprts *sptr_oprts)
{
	if (!isValid(sptr_cdev))
		return -ER_NOMEM;

	memset(sptr_cdev, 0, sizeof(struct fwk_cdev));
	sptr_cdev->sptr_oprts = (struct fwk_file_oprts *)sptr_oprts;
	sptr_cdev->sptr_next = mr_nullptr;

	return ER_NORMAL;
}

/*!
 * @brief   cdev alloc
 * @param   sptr_oprts
 * @retval  errno
 * @note    none
 */
struct fwk_cdev *fwk_cdev_alloc(const struct fwk_file_oprts *sptr_oprts)
{
	struct fwk_cdev *sptr_cdev;

	sptr_cdev = (struct fwk_cdev *)kzalloc(sizeof(struct fwk_cdev), GFP_KERNEL);
	if (!isValid(sptr_cdev))
		goto fail2;

	if (fwk_cdev_init(sptr_cdev, sptr_oprts))
		goto fail1;

	return sptr_cdev;

fail1:
	if (isValid(sptr_cdev))
		kfree(sptr_cdev);

fail2:
	return mr_nullptr;
}

/*!
 * @brief   cdev add
 * @param   sptr_cdev, devNum, count
 * @retval  errno
 * @note    none
 */
kint32_t fwk_cdev_add(struct fwk_cdev *sptr_cdev, kuint32_t devNum, kuint32_t count)
{
	if (!isValid(sptr_cdev))
		return -ER_NOMEM;

	sptr_cdev->devNum = devNum;
	sptr_cdev->count = count;

	return fwk_kobj_map(sptr_fwk_chrdev_map, devNum, count, sptr_cdev);
}

/*!
 * @brief   cdev delete
 * @param   sptr_cdev
 * @retval  errno
 * @note    none
 */
kint32_t fwk_cdev_del(struct fwk_cdev *sptr_cdev)
{
	if (!isValid(sptr_cdev))
		return -ER_NOMEM;

	fwk_kobj_unmap(sptr_fwk_chrdev_map, sptr_cdev->devNum, sptr_cdev->count);

	return ER_NORMAL;
}
