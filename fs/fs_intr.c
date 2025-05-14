/*
 * General FatFs Interface
 *
 * File Name:   fs_fatfs.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.11.03
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#include <platform/fwk_mempool.h>
#include <platform/fwk_inode.h>
#include <platform/fwk_fs.h>
#include <platform/block/fwk_gendisk.h>
#include <fs/fs_intr.h>

/*!< The defines */

/*!< The globals */

/*!< API function */
/*!
 * @brief   open file
 * @param   name, mode
 * @retval  file pointer
 * @note    open file (block device)
 */
struct fs_stream *file_open(const kchar_t *name, kuint32_t mode)
{
    struct fwk_inode *sptr_inode;
    struct fs_stream *sptr_fs;
    struct fwk_file sgtc_file;
    struct fwk_block_device *sptr_blkdev;
    kint32_t retval;

    sptr_inode = fwk_inode_find_disk(name);
    if (!isValid(sptr_inode))
        return ERR_PTR(-ER_NOTFOUND);

    sptr_fs = (struct fs_stream *)kzalloc(sizeof(*sptr_fs), GFP_KERNEL);
    if (!isValid(sptr_fs))
        return ERR_PTR(-ER_NOMEM);

    sptr_fs->full_name = (kchar_t *)name;
    sptr_fs->mode = mode;
    sptr_fs->sptr_dnode = sptr_inode;

    sgtc_file.sptr_foprts = sptr_inode->sptr_foprts;
    if (sgtc_file.sptr_foprts->open)
    {
        sgtc_file.mode = mode;
        retval = sgtc_file.sptr_foprts->open(sptr_inode, &sgtc_file);
        if (retval)
            goto fail1;
    }

    sptr_blkdev = sptr_inode->sptr_blkdev;
    sptr_fs->sptr_bops = sptr_blkdev->sptr_gdisk->sptr_bops;

    if (sptr_fs->sptr_bops->open)
    {
        retval = sptr_fs->sptr_bops->open(sptr_blkdev, sptr_fs);
        if (retval)
            goto fail2;
    }

    return sptr_fs;

fail2:
    if (sgtc_file.sptr_foprts->close)
        sgtc_file.sptr_foprts->close(sptr_inode, &sgtc_file);

fail1:
    kfree(sptr_fs);
    return ERR_PTR(-ER_FAILD);
}

/*!
 * @brief   close file
 * @param   sptr_fs
 * @retval  none
 * @note    close file (block device)
 */
void file_close(struct fs_stream *sptr_fs)
{
    struct fwk_inode *sptr_dnode;
    struct fwk_block_device *sptr_blkdev;
    struct fwk_file sgtc_file;
    kint32_t retval;

    sptr_dnode = sptr_fs->sptr_dnode;
    sptr_blkdev = sptr_dnode->sptr_blkdev;

    if (sptr_fs->sptr_bops->close)
    {
        retval = sptr_fs->sptr_bops->close(sptr_blkdev, sptr_fs);
        if (retval)
            return;
    }

    sgtc_file.sptr_foprts = sptr_dnode->sptr_foprts;

    if (sgtc_file.sptr_foprts->close)
        sgtc_file.sptr_foprts->close(sptr_dnode, &sgtc_file);

    kfree(sptr_fs);
}

/*!
 * @brief   write file
 * @param   sptr_fs, buf, size
 * @retval  size of data written
 * @note    write buf to sptr_fs
 */
kssize_t file_write(struct fs_stream *sptr_fs, const void *buf, kusize_t size)
{
    if (!sptr_fs->sptr_bops->write)
        return -ER_ERROR;

    return sptr_fs->sptr_bops->write(sptr_fs, buf, size, 0);
}

/*!
 * @brief   read file
 * @param   sptr_fs, buf, size
 * @retval  size of data read
 * @note    read buf from sptr_fs
 */
kssize_t file_read(struct fs_stream *sptr_fs, void *buf, kusize_t size)
{
    if (!sptr_fs->sptr_bops->read)
        return -ER_ERROR;

    return sptr_fs->sptr_bops->read(sptr_fs, buf, size, 0);
}

/*!
 * @brief   get file size
 * @param   sptr_fs
 * @retval  size of file data
 * @note    none
 */
kssize_t file_size(struct fs_stream *sptr_fs)
{
    if (!sptr_fs->sptr_bops->fsize)
        return -ER_ERROR;

    return sptr_fs->sptr_bops->fsize(sptr_fs);
}

/*!
 * @brief   locate to offset
 * @param   sptr_fs, offset
 * @retval  errno
 * @note    set offset
 */
kint32_t file_lseek(struct fs_stream *sptr_fs, kuint32_t offset)
{
    if (!sptr_fs->sptr_bops->lseek)
        return -ER_ERROR;

    return sptr_fs->sptr_bops->lseek(sptr_fs, offset);
}

/*!
 * @brief   get offset
 * @param   sptr_fs
 * @retval  errno
 * @note    get offset
 */
kssize_t file_tell(struct fs_stream *sptr_fs)
{
    if (!sptr_fs->sptr_bops->fpos)
        return -ER_ERROR;

    return sptr_fs->sptr_bops->fpos(sptr_fs);
}

/* end of file */