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
#include <kernel/mutex.h>

/*!< The defines */
struct fwk_block_major_name
{
    kchar_t name[DEVICE_NAME_LEN];
    kint32_t major;
};

/*!< The globals */
struct fwk_block_major_name *sgtc_block_major_name[DEVICE_MAX_NUM];
static struct mutex_lock sgtc_blkdev_mutex = MUTEX_LOCK_INIT();

/*!< API function */
/*!
 * @brief   register block device
 * @param   major, name
 * @retval  errno
 * @note    none
 */
static struct fwk_block_major_name *__fwk_register_blkdev(kuint32_t major, const kchar_t *name)
{
    struct fwk_block_major_name *sptr_blkdev;
    struct fwk_block_major_name **sptr_Dst;
    kuint32_t index;
    kusize_t blkdevMax;
    kint32_t i;

    blkdevMax = ARRAY_SIZE(sgtc_block_major_name);

    if (!major)
    {
        sptr_Dst = &sgtc_block_major_name[blkdevMax - 1];

        /*!< 
         * The composite value of the Major + Minor must be less than 2^32, 
         * that is, the number of primary device numbers and secondary device numbers is limited 
         */
        for (i = (blkdevMax - 1); i >= 0; i--)
        {
            if (!(*(sptr_Dst--)))
                break;
        }

        if (i < 0)
            return ERR_PTR(-ER_EXISTED);

        major = i;
    }

    if (major >= blkdevMax)
        return ERR_PTR(-ER_INVALID);

    index = major % blkdevMax;
    sptr_Dst = &sgtc_block_major_name[index];

    /*!< If sptr_Temp[index] is empty, sptr_blkdev can be deposited directly */
    if (*sptr_Dst)
        return ERR_PTR(-ER_EXISTED);

    sptr_blkdev = (struct fwk_block_major_name *)kmalloc(sizeof(*sptr_blkdev), GFP_KERNEL);
    if (!isValid(sptr_blkdev))
        return ERR_PTR(-ER_FAILD);

    sptr_blkdev->major = major;
    strncpy(sptr_blkdev->name, (const char *)name, sizeof(sptr_blkdev->name));
    *sptr_Dst = sptr_blkdev;

    return sptr_blkdev;
}

/*!
 * @brief   unregister block device
 * @param   major, name
 * @retval  errno
 * @note    none
 */
static struct fwk_block_major_name *__fwk_unregister_blkdev(kuint32_t major, const kchar_t *name)
{
    struct fwk_block_major_name *sptr_Rlt;
    struct fwk_block_major_name **sptr_Dst;
    kuint32_t index;
    kusize_t  blkdevMax;

    blkdevMax = ARRAY_SIZE(sgtc_block_major_name);
    if (major >= blkdevMax)
        return ERR_PTR(-ER_INVALID);

    index = major % blkdevMax;
    sptr_Dst = &sgtc_block_major_name[index];
    if (!(*sptr_Dst))
        return ERR_PTR(-ER_EMPTY);

    sptr_Rlt = *sptr_Dst;
    if (strcmp(sptr_Rlt->name, name))
        return ERR_PTR(-ER_CHECKERR);

    *sptr_Dst = mr_nullptr;

    return sptr_Rlt;
}

/*!
 * @brief   register block device
 * @param   major, name
 * @retval  errno
 * @note    none
 */
kint32_t fwk_register_blkdev(kuint32_t major, const kchar_t *name)
{
    struct fwk_block_major_name *sptr_blkdev;

    mutex_lock(&sgtc_blkdev_mutex);

    sptr_blkdev = __fwk_register_blkdev(major, name);
    if (!isValid(sptr_blkdev))
    {
        mutex_unlock(&sgtc_blkdev_mutex);
        print_err("register block device \"%s\" failed!\r\n", name);

        return -ER_FAILD;
    }

    mutex_unlock(&sgtc_blkdev_mutex);
    return ER_NORMAL;
}

/*!
 * @brief   unregister block device
 * @param   major, name
 * @retval  errno
 * @note    none
 */
void fwk_unregister_blkdev(kuint32_t major, const kchar_t *name)
{
    mutex_lock(&sgtc_blkdev_mutex);
    __fwk_unregister_blkdev(major, name);
    mutex_unlock(&sgtc_blkdev_mutex);
}

/* end of file */
