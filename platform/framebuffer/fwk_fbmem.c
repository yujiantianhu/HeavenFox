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

/*!< The defines */
/*!< Maximum number of fb devices (total number of secondary devices) */
#define FWK_FB_DEVICE_MAX							(32)
/*!< The primary device number of the fb device */
#define FWK_FB_DEVICE_MAJOR							(NR_FBDEV_MAJOR)

/*!< The globals */
static struct fwk_fb_info *sgrt_fwk_registered_fb[FWK_FB_DEVICE_MAX] = {mrt_nullptr};

/*!< The functions */

/*!< API function */
/*!
 * @brief   Apply for a fb_info struct
 * @param   size: the size(bytes) of the private data
 * @retval  none
 * @note    none
 */
struct fwk_fb_info *fwk_framebuffer_alloc(kusize_t size, struct fwk_device *sprt_dev)
{
#define RET_PADDING_LONG(x)								(RET_BYTES_PER_LONG  - ((x) % RET_BYTES_PER_LONG))
#define RET_FD_INFO_PADDING								RET_PADDING_LONG(sizeof(struct fwk_fb_info))

	struct fwk_fb_info *sprt_fb_info;
	kuint32_t fb_info_size;

	fb_info_size = sizeof(struct fwk_fb_info);

	if (size)
	{
		/*!< Consider byte alignment */
		fb_info_size += RET_FD_INFO_PADDING;
	}

	sprt_fb_info = (struct fwk_fb_info *)kzalloc(fb_info_size + size, GFP_KERNEL);
	if (!isValid(sprt_fb_info))
		return mrt_nullptr;

	if (size)
	{
		/*!< Save private variables */
		sprt_fb_info->ptr_par = sprt_fb_info + fb_info_size;
		memset(sprt_fb_info->ptr_par, 0, size);
	}

	/*!< Save the parent node of the device */
	sprt_fb_info->sprt_dev = sprt_dev;

	return sprt_fb_info;

#undef RET_FD_INFO_PADDING
#undef RET_PADDING_LONG
}

/*!
 * @brief   Release a fb_info struct
 * @param   none
 * @retval  none
 * @note    none
 */
void fwk_framebuffer_release(struct fwk_fb_info *sprt_fb_info)
{
	if (isValid(sprt_fb_info))
		kfree(sprt_fb_info);
}

/*!
 * @brief   fwk_register_framebuffer
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_register_framebuffer(struct fwk_fb_info *sprt_fb_info)
{
	struct fwk_fb_info **sprt_exsited;
	kint32_t retval;
	kuint32_t i;

	if (!isValid(sprt_fb_info))
		return -NR_IS_UNVALID;

	sprt_exsited = &sgrt_fwk_registered_fb[0];

	/*!< Search, find an empty location */
	for (i = 0; i < FWK_FB_DEVICE_MAX; i++)
	{
		if (!sprt_exsited[i])
			break;
	}

	/*!< The retrieval failed, and the number of fb devices has reached the upper limit */
	if (i == FWK_FB_DEVICE_MAX)
		return -NR_IS_MORE;

	/*!< Save the secondary device number */
	sprt_fb_info->node = i;

	/*!< Create a character device node */
	retval = fwk_device_create(NR_TYPE_CHRDEV, MKE_DEV_NUM(FWK_FB_DEVICE_MAJOR, i), "fb%d", i);
	if (retval < 0)
		return -NR_IS_UNVALID;

	/*!< Register to the global array */
	sprt_exsited[i] = sprt_fb_info;

	return NR_IS_NORMAL;
}

/*!
 * @brief   fwk_unregister_framebuffer
 * @param   none
 * @retval  none
 * @note    none
 */
void fwk_unregister_framebuffer(struct fwk_fb_info *sprt_fb_info)
{
	struct fwk_fb_info **sprt_exsited;
	kuint32_t i;

	i = sprt_fb_info->node;
	sprt_exsited = &sgrt_fwk_registered_fb[0];

	fwk_device_destroy("fb%d", i);
	sprt_exsited[i] = mrt_nullptr;
}

/*!
 * @brief   fwk_get_fb_info
 * @param   none
 * @retval  none
 * @note    none
 */
struct fwk_fb_info *fwk_get_fb_info(kuint32_t idx)
{
	return sgrt_fwk_registered_fb[idx];
}

/*!
 * @brief   fwk_file_fb_info
 * @param   none
 * @retval  none
 * @note    none
 */
struct fwk_fb_info *fwk_file_fb_info(struct fwk_file *sprt_file)
{
	struct fwk_inode *sprt_inode;
	kuint32_t fbidx;

	sprt_inode  = RET_INODE_FROM_FILE(sprt_file);
	fbidx		= RET_INODE_MINOR(sprt_inode);

	return sgrt_fwk_registered_fb[fbidx];
}

/*!< ------------------------------------------------------------------------- */
/*!< Frambuffer driver Interface */
/*!< The globals */
static struct fwk_cdev *sprt_fwk_fb_cdev;

/*!< ------------------------------------------------------------------------- */
/*!
 * @brief   fwk_fb_open
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_fb_open(struct fwk_inode *sprt_inode, struct fwk_file *sprt_file)
{
	struct fwk_fb_info *sprt_info;
	kuint32_t fbidx;
	kint32_t retval;

	fbidx = RET_INODE_MINOR(sprt_inode);
	sprt_info = fwk_get_fb_info(fbidx);
	if (!isValid(sprt_info))
		return -NR_IS_FAULT;

	sprt_file->private_data	= sprt_info;

	if (sprt_info->sprt_fbops->fb_open)
	{
		retval = sprt_info->sprt_fbops->fb_open(sprt_info, 1);
		if (retval < 0)
		{
			/*!< Open fb device failed */
			return -NR_IS_ERROR;
		}
	}

	return NR_IS_NORMAL;
}

/*!
 * @brief   fwk_fb_close
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_fb_close(struct fwk_inode *sprt_inode, struct fwk_file *sprt_file)
{
	struct fwk_fb_info *sprt_info;

	sprt_info = (struct fwk_fb_info *)sprt_file->private_data;

	if (sprt_info->sprt_fbops->fb_release)
		sprt_info->sprt_fbops->fb_release(sprt_info, 1);

	return NR_IS_NORMAL;
}

/*!
 * @brief   fwk_fb_write
 * @param   none
 * @retval  none
 * @note    none
 */
static kssize_t fwk_fb_write(struct fwk_file *sprt_file, const kbuffer_t *ptr_buf, kssize_t size)
{
	return NR_IS_NORMAL;
}

/*!
 * @brief   fwk_fb_read
 * @param   none
 * @retval  none
 * @note    none
 */
static kssize_t fwk_fb_read(struct fwk_file *sprt_file, kbuffer_t *ptr_buf, kssize_t size)
{
	return NR_IS_NORMAL;
}

/*!
 * @brief   fwk_fb_ioctl
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_fb_ioctl(struct fwk_file *sprt_file, kuint32_t cmd, kuaddr_t args)
{
	struct fwk_fb_info *sprt_info;
	struct fwk_fb_fix_screen_info sgrt_fix;
	struct fwk_fb_var_screen_info sgrt_var;
	kuint8_t *ptr_user;
	kint32_t retval;

	sprt_info = (struct fwk_fb_info *)sprt_file->private_data;
	ptr_user = (kuint8_t *)args;

	switch (cmd)
	{
		case NR_FB_IOGetVScreenInfo:
			sgrt_var = sprt_info->sgrt_var;
			retval = fwk_copy_to_user(ptr_user, &sgrt_var, sizeof(sgrt_var)) ? NR_IS_NORMAL : -NR_IS_ERROR;
			break;

		case NR_FB_IOPutVScreenInfo:
			if (!fwk_copy_to_user(&sgrt_var, ptr_user, sizeof(sgrt_var)))
				return -NR_IS_ERROR;

			/*!< Set the variable parameters */

			break;

		case NR_FB_IOGetFScreenInfo:
			sgrt_fix = sprt_info->sgrt_fix;
			retval = fwk_copy_to_user(ptr_user, &sgrt_fix, sizeof(sgrt_fix)) ? NR_IS_NORMAL : -NR_IS_ERROR;
			break;

		default:
			retval = -NR_IS_UNVALID;
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
static kint32_t fwk_fb_mmap(struct fwk_file *sprt_file, struct fwk_vm_area *vm_area)
{
	struct fwk_fb_info *sprt_info;
	kint32_t retval;

	sprt_info = (struct fwk_fb_info *)sprt_file->private_data;
	if (!isValid(sprt_info))
		return -NR_IS_FAULT;

	if (sprt_info->sprt_fbops->fb_mmap)
	{
		retval = sprt_info->sprt_fbops->fb_mmap(sprt_info, vm_area);
		if (retval < 0)
		{
			/*!< Open fb device failed */
			return -NR_IS_ERROR;
		}
	}

	vm_area->virt_addr = sprt_info->sgrt_fix.smem_start;
	vm_area->size = sprt_info->sgrt_fix.smem_len;

	return NR_IS_NORMAL;
}

static struct fwk_file_oprts sgrt_fwk_fb_foprts =
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
kint32_t __dync_init fwk_fbmem_init(void)
{
	struct fwk_cdev *sprt_cdev;
	kuint32_t devNum;
	kint32_t retval;

	devNum = MKE_DEV_NUM(FWK_FB_DEVICE_MAJOR, 0);
	retval = fwk_register_chrdev(devNum, FWK_FB_DEVICE_MAX, "fb");
	if (retval < 0)
		goto fail1;

	sprt_cdev = fwk_cdev_alloc(&sgrt_fwk_fb_foprts);
	if (!isValid(sprt_cdev))
		goto fail2;

	retval = fwk_cdev_add(sprt_cdev, devNum, FWK_FB_DEVICE_MAX);
	if (retval < 0)
		goto fail3;

	/*!< Save to global variable */
	sprt_fwk_fb_cdev = sprt_cdev;

	return NR_IS_NORMAL;

fail3:
	fwk_cdev_del(sprt_cdev);
fail2:
	fwk_unregister_chrdev(devNum, FWK_FB_DEVICE_MAX);
fail1:
	return -NR_IS_NOMEM;
}

/*!
 * @brief   fwk_fbmem_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __dync_exit fwk_fbmem_exit(void)
{
	struct fwk_cdev *sprt_cdev;
	kuint32_t devNum;

	sprt_cdev = sprt_fwk_fb_cdev;
	devNum = sprt_cdev->devNum;

	fwk_cdev_del(sprt_cdev);
	fwk_unregister_chrdev(devNum, FWK_FB_DEVICE_MAX);
}

IMPORT_PATTERN_INIT(fwk_fbmem_init);
IMPORT_PATTERN_EXIT(fwk_fbmem_exit);
