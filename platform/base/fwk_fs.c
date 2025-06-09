/*
 * Platform File System Controller
 *
 * File Name:   fwk_fs.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.29
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_fs.h>

/*!< API function */
/*!
 * @brief   fwk_do_filp_open
 * @param   none
 * @retval  none
 * @note    none
 */
struct fwk_file *fwk_do_filp_open(kchar_t *name, kuint32_t mode)
{
    struct fwk_file  *sptr_file;
    struct fwk_inode *sptr_inode;
    kint32_t retval;

    sptr_inode = fwk_inode_find(name);
    if (!isValid(sptr_inode))
        goto fail1;
    
    sptr_file = (struct fwk_file *)kzalloc(sizeof(struct fwk_file), GFP_KERNEL);
    if (!isValid(sptr_file))
        goto fail1;

    sptr_file->mode = mode;
    sptr_file->sptr_inode = sptr_inode;
    sptr_file->sptr_foprts = sptr_inode->sptr_foprts;
    if (sptr_file->sptr_foprts->open)
    {
        retval = sptr_file->sptr_foprts->open(sptr_inode, sptr_file);
        if (retval)
            goto fail2;
    }

    return sptr_file;

fail2:
    kfree(sptr_file);

fail1:
    return mr_nullptr;
}

/*!
 * @brief   fwk_do_filp_close
 * @param   none
 * @retval  none
 * @note    none
 */
void fwk_do_filp_close(struct fwk_file *sptr_file)
{
    struct fwk_inode *sptr_inode;

    if (!isValid(sptr_file))
        return;

    sptr_inode = sptr_file->sptr_inode;

    /*!< Close device */
    if (sptr_file->sptr_foprts->close)
        sptr_file->sptr_foprts->close(sptr_inode, sptr_file);

    /*!< 
     * Legacy bug: 
     * If the file is not closed after opening, and the inode node is suddenly deleted, 
     * what should we do with the file? How to release the occupied fd? 
     */
    if (sptr_inode->sptr_foprts->close)
        sptr_inode->sptr_foprts->close(sptr_inode, sptr_file);

    kfree(sptr_file);
}

/*!< end of file */
