/*
 * Block Device Interface
 *
 * File Name:   fwk_blkdevice.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.11.03
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_inode.h>
#include <platform/fwk_fs.h>
#include <platform/block/fwk_gendisk.h>
#include <platform/fwk_platdrv.h>

/*!< The defines */

/*!< The globals */

/*!< API function */
/*!
 * @brief   gendisk initial
 * @param   sptr_gdisk, sptr_oprts
 * @retval  errno
 * @note    none
 */
kint32_t fwk_gendisk_init(struct fwk_gendisk *sptr_gdisk, const struct fwk_block_device_oprts *sptr_oprts)
{
    if (!isValid(sptr_gdisk))
        return -ER_NOMEM;

    memset(sptr_gdisk, 0, sizeof(*sptr_gdisk));
    sptr_gdisk->sptr_bops = (struct fwk_block_device_oprts *)sptr_oprts;

    return ER_NORMAL;
}

/*!
 * @brief   gendisk alloc
 * @param   sptr_oprts
 * @retval  errno
 * @note    none
 */
struct fwk_gendisk *fwk_alloc_gendisk(kint32_t minors, const struct fwk_block_device_oprts *sptr_oprts)
{
    struct fwk_gendisk *sptr_gdisk;

    sptr_gdisk = (struct fwk_gendisk *)kzalloc(sizeof(*sptr_gdisk), GFP_KERNEL);
    if (!isValid(sptr_gdisk))
        return ERR_PTR(-ER_FAILD);

    if (fwk_gendisk_init(sptr_gdisk, sptr_oprts))
    {
        kfree(sptr_gdisk);
        return ERR_PTR(-ER_FAILD);
    }

    sptr_gdisk->minors = minors;
    return sptr_gdisk;
}

/*!
 * @brief   gendisk add
 * @param   sptr_gdisk
 * @retval  errno
 * @note    none
 */
kint32_t fwk_add_gendisk(struct fwk_gendisk *sptr_gdisk)
{
    struct fwk_block_device *sptr_blkdev;
    struct fwk_device *sptr_dev;
    kuint32_t devNum;
    kint32_t retval;

	if (!isValid(sptr_gdisk))
		return -ER_NOMEM;

    sptr_blkdev = (struct fwk_block_device *)kzalloc(sizeof(*sptr_blkdev), GFP_KERNEL);
    if (!isValid(sptr_blkdev))
        return -ER_FAILD;

    sptr_blkdev->sptr_gdisk = sptr_gdisk;
    sptr_blkdev->major = sptr_gdisk->major;
    sptr_gdisk->sptr_blkdev = sptr_blkdev;

	devNum = MKE_DEV_NUM(sptr_gdisk->major, sptr_gdisk->first_minor);
	retval = fwk_kobj_map(sptr_fwk_blkdev_map, devNum, sptr_gdisk->minors, sptr_blkdev);
    if (retval)
        goto fail1;

    if (sptr_gdisk->mount)
    {
        retval = sptr_gdisk->mount(sptr_gdisk);
        if (retval)
            goto fail2;

        return ER_NORMAL;
    }

    sptr_dev = fwk_device_create(NR_TYPE_BLKDEV, devNum, sptr_gdisk->disk_name);
    if (!isValid(sptr_dev))
        goto fail2;

    sptr_gdisk->sptr_devfs = sptr_dev;
    return ER_NORMAL;

fail2:
    fwk_kobj_unmap(sptr_fwk_blkdev_map, devNum, sptr_gdisk->minors);
fail1:
    kfree(sptr_blkdev);
    sptr_gdisk->sptr_blkdev = mr_nullptr;

    return -ER_FAILD;
}

/*!
 * @brief   gendisk delete
 * @param   sptr_gdisk
 * @retval  errno
 * @note    none
 */
kint32_t fwk_del_gendisk(struct fwk_gendisk *sptr_gdisk)
{
    kuint32_t devNum;

	if (!isValid(sptr_gdisk))
		return -ER_NOMEM;

    devNum = MKE_DEV_NUM(sptr_gdisk->major, sptr_gdisk->first_minor);
	fwk_kobj_unmap(sptr_fwk_blkdev_map, devNum, sptr_gdisk->minors);

    if (sptr_gdisk->unmount)
        sptr_gdisk->unmount(sptr_gdisk);
    else
    {
        fwk_device_destroy(sptr_gdisk->sptr_devfs);
        sptr_gdisk->sptr_devfs = mr_nullptr;
    }

    kfree(sptr_gdisk->sptr_blkdev);
    sptr_gdisk->sptr_blkdev = mr_nullptr;

	return ER_NORMAL;
}

/* end of file */
