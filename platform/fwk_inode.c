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
static kint32_t fwk_chrdev_open(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    struct fwk_cdev *sptr_cdev;
    kuint32_t devNum;

    devNum = sptr_inode->r_dev;
/*
    if (devNum < DEVICE_MAJOR_BASE)
        goto fail;
*/

    sptr_cdev = (struct fwk_cdev *)fwk_kobjmap_lookup(sptr_fwk_chrdev_map, devNum);
    if (!isValid(sptr_cdev))
        goto fail;

    sptr_inode->sptr_cdev = sptr_cdev;

    /*!< Replace the device operate function */
    sptr_file->sptr_foprts = sptr_cdev->sptr_oprts;
    if (sptr_file->sptr_foprts->open)
        return sptr_file->sptr_foprts->open(sptr_inode, sptr_file);

    return ER_NORMAL;

fail:
    return -ER_INVALID;
}

/*!
 * @brief   fwk_chrdev_close
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_chrdev_close(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    sptr_inode->sptr_cdev = mr_nullptr;

    return ER_NORMAL;
}

static struct fwk_file_oprts sgtc_fwk_inode_def_chrfoprts =
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
static kint32_t fwk_blkdev_open(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    struct fwk_block_device *sptr_blkdev;
    kuint32_t devNum;

    devNum = sptr_inode->r_dev;

    sptr_blkdev = (struct fwk_block_device *)fwk_kobjmap_lookup(sptr_fwk_blkdev_map, devNum);
    if (!isValid(sptr_blkdev))
        goto fail;

    sptr_inode->sptr_blkdev = sptr_blkdev;
    sptr_blkdev->sptr_inode = sptr_inode;

    return ER_NORMAL;

fail:
    return -ER_INVALID;
}

/*!
 * @brief   fwk_blkdev_close
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_blkdev_close(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    struct fwk_block_device *sptr_blkdev;

    sptr_blkdev = sptr_inode->sptr_blkdev;
    sptr_blkdev->sptr_inode = mr_nullptr;
    sptr_inode->sptr_blkdev = mr_nullptr;

    return ER_NORMAL;
}

static struct fwk_file_oprts sgtc_fwk_inode_def_blkfoprts =
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
static kint32_t fwk_netdev_open(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    return ER_NORMAL;
}

/*!
 * @brief   fwk_netdev_close
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_netdev_close(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    return ER_NORMAL;
}

static struct fwk_file_oprts sgtc_fwk_inode_def_netfoprts =
{
    .open	= fwk_netdev_open,
    .close	= fwk_netdev_close,
};

/*!
 * @brief   set the operation functions to inode
 * @param   sptr_node, type, devNum
 * @retval  errno
 * @note    none
 */
kint32_t fwk_inode_set_ops(struct fwk_inode *sptr_inode, kuint32_t type, kint32_t devNum)
{
    if (!sptr_inode)
        return -ER_NOMEM;

    sptr_inode->r_dev = devNum;

    switch(type)
    {
        case NR_TYPE_CHRDEV:
            sptr_inode->sptr_foprts	= &sgtc_fwk_inode_def_chrfoprts;
            break;

        case NR_TYPE_BLKDEV:
            sptr_inode->sptr_foprts	= &sgtc_fwk_inode_def_blkfoprts;
            break;

        case NR_TYPE_NETDEV:
            sptr_inode->sptr_foprts	= &sgtc_fwk_inode_def_netfoprts;
            break;

        default:
            break;
    };

    return ER_NORMAL;
}

/*!
 * @brief   fwk_mk_inode
 * @param   none
 * @retval  none
 * @note    none
 */
struct fwk_inode *fwk_mk_inode(struct fwk_kobject *sptr_kobj, kuint32_t type, kint32_t devNum)
{
    struct fwk_inode *sptr_inode;

    if (!sptr_kobj)
        return ERR_PTR(-ER_ERROR);

    sptr_inode = (struct fwk_inode *)kzalloc(sizeof(struct fwk_inode), GFP_KERNEL);
    if (!isValid(sptr_inode))
        return ERR_PTR(-ER_NOMEM);

    sptr_inode->name = sptr_kobj->name;
    sptr_inode->type = sptr_kobj->is_dir ? INODE_TYPE_DIR : INODE_TYPE_FILE;
    sptr_inode->sptr_kobj = sptr_kobj;

    fwk_inode_set_ops(sptr_inode, type, devNum);

    return sptr_inode;
}

/*!
 * @brief   fwk_rm_inode
 * @param   none
 * @retval  none
 * @note    none
 */
void fwk_rm_inode(struct fwk_inode *sptr_inode)
{
    if (!sptr_inode)
        return;

    /*!< Free up resources */
    kfree(sptr_inode);
}

/*!
 * @brief   fwk_inode_find
 * @param   none
 * @retval  none
 * @note    none
 */
struct fwk_inode *fwk_inode_find(kchar_t *name)
{
    struct fwk_kobject *sptr_kobj;

    if (!name || !(*name))
        return mr_nullptr;

    sptr_kobj = fwk_find_kobject_by_path(mr_nullptr, name);
    if (!isValid(sptr_kobj))
        return ERR_PTR(-ER_NOTFOUND);

    return sptr_kobj->sptr_inode;
}

/*!
 * @brief   fwk_inode_find
 * @param   none
 * @retval  none
 * @note    none
 */
struct fwk_inode *fwk_inode_find_disk(const kchar_t *name)
{
    struct fwk_kobject *sptr_kobj = mr_nullptr;
    kchar_t *disk_name;
    kuint32_t lenth, mark = 0;

    if (!name || (*name != '/'))
        return mr_nullptr;

    lenth = kstrlen(name);
    if (*(name + lenth - 1) == '/')
        return mr_nullptr;

    disk_name = (kchar_t *)kmalloc(lenth + 1, GFP_KERNEL);
    if (!isValid(disk_name))
        return mr_nullptr;

    kstrcpy(disk_name, name);

    while (lenth > 1)
    {
        lenth--;
        if (*(disk_name + lenth - 1) != '/')
        {
            mark = 0;
            continue;
        }

        /*!< if '/' appears 2 times continually, it is unvalid */
        if (mark)
            break;
        
        mark = 1;
        *(disk_name + lenth) = '\0';

        sptr_kobj = fwk_find_kobject_by_path(sptr_kobj, disk_name);
        if (!isValid(sptr_kobj))
            continue;

        if (sptr_kobj->is_disk)
            goto succ;
    }

    sptr_kobj = mr_nullptr;

succ:
    kfree(disk_name);
    return (isValid(sptr_kobj) ? sptr_kobj->sptr_inode : mr_nullptr);
}

/*!< end of file */
