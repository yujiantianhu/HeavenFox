/*
 * Framebuffer Driver Top Interface
 *
 * File Name:   fwk_fbmem.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.01
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_uaccess.h>
#include <platform/fwk_cdev.h>
#include <platform/fwk_chrdev.h>
#include <platform/fwk_inode.h>
#include <platform/video/fwk_fbmem.h>
#include <kernel/mutex.h>

/*!< The defines */
/*!< Maximum number of fb devices (total number of secondary devices) */
#define FWK_FB_DEVICE_MAX								(32)
/*!< The primary device number of the fb device */
#define FWK_FB_DEVICE_MAJOR								(NR_FBDEV_MAJOR)

/*!< The globals */
static struct fwk_fb_info *sgtc_fwk_registered_fb[FWK_FB_DEVICE_MAX];
static struct mutex_lock sgtc_fwk_fbmem_mutex = MUTEX_LOCK_INIT();

/*!< The functions */

/*!< API function */
/*!
 * @brief   Apply for a fb_info struct
 * @param   size: the size(bytes) of the private data
 * @retval  none
 * @note    none
 */
struct fwk_fb_info *fwk_framebuffer_alloc(kusize_t size, struct fwk_device *sptr_dev)
{
#define RET_PADDING_LONG(x)								(RET_BYTES_PER_LONG  - ((x) % RET_BYTES_PER_LONG))
#define RET_FD_INFO_PADDING								RET_PADDING_LONG(sizeof(struct fwk_fb_info))

    struct fwk_fb_info *sptr_fb_info;
    kuint32_t fb_info_size;

    fb_info_size = sizeof(struct fwk_fb_info);

    if (size)
    {
        /*!< Consider byte alignment */
        fb_info_size += RET_FD_INFO_PADDING;
    }

    sptr_fb_info = (struct fwk_fb_info *)kzalloc(fb_info_size + size, GFP_KERNEL);
    if (!isValid(sptr_fb_info))
        return mr_nullptr;

    if (size)
    {
        /*!< Save private variables */
        sptr_fb_info->ptr_par = (void *)sptr_fb_info + fb_info_size;
        memset(sptr_fb_info->ptr_par, 0, size);
    }

    /*!< Save the parent node of the device */
    sptr_fb_info->sptr_dev = sptr_dev;

    return sptr_fb_info;

#undef RET_FD_INFO_PADDING
#undef RET_PADDING_LONG
}

/*!
 * @brief   Release a fb_info struct
 * @param   none
 * @retval  none
 * @note    none
 */
void fwk_framebuffer_release(struct fwk_fb_info *sptr_fb_info)
{
    if (isValid(sptr_fb_info))
        kfree(sptr_fb_info);
}

/*!
 * @brief   fwk_register_framebuffer
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_register_framebuffer(struct fwk_fb_info *sptr_fb_info)
{
    struct fwk_fb_info **sptr_exsited;
    struct fwk_device *sptr_idev;
    kuint32_t i;

    if (!isValid(sptr_fb_info))
        return -ER_INVALID;

    sptr_exsited = &sgtc_fwk_registered_fb[0];
    mutex_lock(&sgtc_fwk_fbmem_mutex);

    /*!< Search, find an empty location */
    for (i = 0; i < FWK_FB_DEVICE_MAX; i++)
    {
        if (!sptr_exsited[i])
            break;
    }

    /*!< The retrieval failed, and the number of fb devices has reached the upper limit */
    if (i == FWK_FB_DEVICE_MAX)
    {
        mutex_unlock(&sgtc_fwk_fbmem_mutex);
        return -ER_MORE;
    }

    /*!< Save the secondary device number */
    sptr_fb_info->node = i;

    /*!< Create a character device node */
    sptr_idev = fwk_device_create(NR_TYPE_CHRDEV, MKE_DEV_NUM(FWK_FB_DEVICE_MAJOR, i), "fb%d", i);
    if (!isValid(sptr_idev))
    {
        mutex_unlock(&sgtc_fwk_fbmem_mutex);
        return -ER_INVALID;
    }

    sptr_fb_info->sptr_idev = sptr_idev;
    /*!< Register to the global array */
    sptr_exsited[i] = sptr_fb_info;

    mutex_unlock(&sgtc_fwk_fbmem_mutex);
    return ER_NORMAL;
}

/*!
 * @brief   fwk_unregister_framebuffer
 * @param   none
 * @retval  none
 * @note    none
 */
void fwk_unregister_framebuffer(struct fwk_fb_info *sptr_fb_info)
{
    struct fwk_fb_info **sptr_exsited;
    kuint32_t i;

    i = sptr_fb_info->node;
    sptr_exsited = &sgtc_fwk_registered_fb[0];

    fwk_device_destroy(sptr_exsited[i]->sptr_idev);

    mutex_lock(&sgtc_fwk_fbmem_mutex);
    sptr_exsited[i] = mr_nullptr;
    mutex_unlock(&sgtc_fwk_fbmem_mutex);
}

/*!
 * @brief   fwk_get_fb_info
 * @param   none
 * @retval  none
 * @note    none
 */
struct fwk_fb_info *fwk_get_fb_info(kuint32_t idx)
{
    return sgtc_fwk_registered_fb[idx];
}

/*!
 * @brief   fwk_file_fb_info
 * @param   none
 * @retval  none
 * @note    none
 */
struct fwk_fb_info *fwk_file_fb_info(struct fwk_file *sptr_file)
{
    struct fwk_inode *sptr_inode;
    kuint32_t fbidx;

    sptr_inode  = RET_INODE_FROM_FILE(sptr_file);
    fbidx		= RET_INODE_MINOR(sptr_inode);

    return sgtc_fwk_registered_fb[fbidx];
}

/*!< ------------------------------------------------------------------------- */
/*!< Frambuffer driver Interface */
/*!< The globals */
static struct fwk_cdev *sptr_fwk_fb_cdev;

/*!< ------------------------------------------------------------------------- */
/*!
 * @brief   fwk_fb_open
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_fb_open(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    struct fwk_fb_info *sptr_info;
    kuint32_t fbidx;
    kint32_t retval;

    fbidx = RET_INODE_MINOR(sptr_inode);
    sptr_info = fwk_get_fb_info(fbidx);
    if (!isValid(sptr_info))
        return -ER_FAULT;

    sptr_file->private_data	= sptr_info;

    if (sptr_info->sptr_fbops->fb_open)
    {
        retval = sptr_info->sptr_fbops->fb_open(sptr_info, 1);
        if (retval < 0)
        {
            /*!< Open fb device failed */
            return -ER_ERROR;
        }
    }

    return ER_NORMAL;
}

/*!
 * @brief   fwk_fb_close
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_fb_close(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    struct fwk_fb_info *sptr_info;

    sptr_info = (struct fwk_fb_info *)sptr_file->private_data;

    if (sptr_info->sptr_fbops->fb_release)
        sptr_info->sptr_fbops->fb_release(sptr_info, 1);

    return ER_NORMAL;
}

/*!
 * @brief   fwk_fb_write
 * @param   none
 * @retval  none
 * @note    none
 */
static kssize_t fwk_fb_write(struct fwk_file *sptr_file, const kbuffer_t *ptr_buf, kssize_t size)
{
    return ER_NORMAL;
}

/*!
 * @brief   fwk_fb_read
 * @param   none
 * @retval  none
 * @note    none
 */
static kssize_t fwk_fb_read(struct fwk_file *sptr_file, kbuffer_t *ptr_buf, kssize_t size)
{
    return ER_NORMAL;
}

/*!
 * @brief   fwk_fb_ioctl
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_fb_ioctl(struct fwk_file *sptr_file, kuint32_t cmd, kuaddr_t args)
{
    struct fwk_fb_info *sptr_info;
    struct fwk_fb_fix_screen_info sgtc_fix;
    struct fwk_fb_var_screen_info sgtc_var;
    kuint8_t *ptr_user;
    kint32_t retval = 0;

    sptr_info = (struct fwk_fb_info *)sptr_file->private_data;
    ptr_user = (kuint8_t *)args;

    switch (cmd)
    {
        case NR_FB_IOGET_VARINFO:
            sgtc_var = sptr_info->sgtc_var;
            retval = fwk_copy_to_user(ptr_user, &sgtc_var, sizeof(sgtc_var));
            if (!retval)
                return -ER_FAILD;
            
            break;

        case NR_FB_IOSET_VARINFO:
            retval = fwk_copy_from_user(&sgtc_var, ptr_user, sizeof(sgtc_var));
            if (!retval)
                return -ER_FAILD;

            /*!< Set the variable parameters */
            if (sptr_info->sptr_fbops->fb_ioctl)
            {
                retval = sptr_info->sptr_fbops->fb_ioctl(sptr_info, 
                                            NR_FB_IOSET_VARINFO, (kuaddr_t)(&sgtc_var));
                if (retval)
                    return retval;

                memcpy(&sptr_info->sgtc_var, &sgtc_var, sizeof(sgtc_var));
            }

            break;

        case NR_FB_IOGET_FIXINFO:
            sgtc_fix = sptr_info->sgtc_fix;
            retval = fwk_copy_to_user(ptr_user, &sgtc_fix, sizeof(sgtc_fix));
            if (!retval)
                return -ER_FAILD;
            
            break;

        default:
            retval = -ER_INVALID;
            break;
    }

    return retval;
}

/*!
 * @brief   fwk_fb_mmap
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_fb_mmap(struct fwk_file *sptr_file, struct fwk_vm_area *vm_area)
{
    struct fwk_fb_info *sptr_info;
    kuint32_t offset;
    kint32_t retval;

    sptr_info = (struct fwk_fb_info *)sptr_file->private_data;
    if (!isValid(sptr_info))
        return -ER_NODEV;

    if (sptr_info->sptr_fbops->fb_mmap)
    {
        retval = sptr_info->sptr_fbops->fb_mmap(sptr_info, vm_area);
        if (retval < 0)
        {
            /*!< Open fb device failed */
            return -ER_ERROR;
        }
    }

    /*!< default 8-bytes alignment */
    offset = mr_align(vm_area->offset, 8);
    if (offset >= sptr_info->screen_size)
        return -ER_MORE;
    
    vm_area->size = sptr_info->sgtc_fix.smem_len;
    vm_area->virt_addr = sptr_info->sgtc_fix.smem_start + offset;

    return ER_NORMAL;
}

static struct fwk_file_oprts sgtc_fwk_fb_foprts =
{
    .open	= fwk_fb_open,
    .close	= fwk_fb_close,
    .write	= fwk_fb_write,
    .read	= fwk_fb_read,
    .unlocked_ioctl	= fwk_fb_ioctl,
    .mmap	= fwk_fb_mmap,
};

/*!
 * @brief   fwk_fbmem_init
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t __plat_init fwk_fbmem_init(void)
{
    struct fwk_cdev *sptr_cdev;
    kuint32_t devNum;
    kint32_t retval;

    devNum = MKE_DEV_NUM(FWK_FB_DEVICE_MAJOR, 0);
    retval = fwk_register_chrdev(devNum, FWK_FB_DEVICE_MAX, "fb");
    if (retval < 0)
        goto fail1;

    sptr_cdev = fwk_cdev_alloc(&sgtc_fwk_fb_foprts);
    if (!isValid(sptr_cdev))
        goto fail2;

    retval = fwk_cdev_add(sptr_cdev, devNum, FWK_FB_DEVICE_MAX);
    if (retval < 0)
        goto fail3;

    /*!< Save to global variable */
    sptr_fwk_fb_cdev = sptr_cdev;

    return ER_NORMAL;

fail3:
    fwk_cdev_del(sptr_cdev);
fail2:
    fwk_unregister_chrdev(devNum, FWK_FB_DEVICE_MAX);
fail1:
    return -ER_NOMEM;
}

/*!
 * @brief   fwk_fbmem_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __plat_exit fwk_fbmem_exit(void)
{
    struct fwk_cdev *sptr_cdev;
    kuint32_t devNum;

    sptr_cdev = sptr_fwk_fb_cdev;
    devNum = sptr_cdev->devNum;

    fwk_cdev_del(sptr_cdev);
    fwk_unregister_chrdev(devNum, FWK_FB_DEVICE_MAX);
}

/*!< end of file */
