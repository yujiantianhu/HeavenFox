/*
 * Framebuffer Driver Top Interface
 *
 * File Name:   hal_fbmem.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.01
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/hal_uaccess.h>
#include <platform/hal_cdev.h>
#include <platform/hal_chrdev.h>
#include <platform/hal_inode.h>
#include <platform/video/hal_fbmem.h>

/*!< -------------------------------------------------------------------------- */
/*!< The defines */
/*!< Maximum number of fb devices (total number of secondary devices) */
#define HAL_FB_DEVICE_MAX							(32)
/*!< The primary device number of the fb device */
#define HAL_FB_DEVICE_MAJOR							(29)

/*!< -------------------------------------------------------------------------- */
/*!< The globals */
static struct hal_fb_info *sgrt_hal_registered_fb[HAL_FB_DEVICE_MAX] = {mrt_nullptr};

/*!< -------------------------------------------------------------------------- */
/*!< The functions */

/*!< -------------------------------------------------------------------------- */
/*!< API function */
/*!
 * @brief   Apply for a fb_info struct
 * @param   size: the size(bytes) of the private data
 * @retval  none
 * @note    none
 */
struct hal_fb_info *hal_framebuffer_alloc(kusize_t size, struct hal_device *sprt_dev)
{
#define RET_PADDING_LONG(x)								(RET_BYTES_PER_LONG  - ((x) % RET_BYTES_PER_LONG))
#define RET_FD_INFO_PADDING								RET_PADDING_LONG(sizeof(struct hal_fb_info))

	struct hal_fb_info *sprt_fb_info;
	kuint32_t fb_info_size;

	fb_info_size = sizeof(struct hal_fb_info);

	if (size)
	{
		/*!< Consider byte alignment */
		fb_info_size += RET_FD_INFO_PADDING;
	}

	sprt_fb_info = (struct hal_fb_info *)kzalloc(fb_info_size + size, GFP_KERNEL);
	if (!mrt_isValid(sprt_fb_info))
	{
		return mrt_nullptr;
	}

	if (size)
	{
		/*!< Save private variables */
		sprt_fb_info->ptr_par = sprt_fb_info + fb_info_size;
	}

	/*!< Save the parent node of the device */
	sprt_fb_info->sprt_device = sprt_dev;

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
void hal_framebuffer_release(struct hal_fb_info *sprt_fb_info)
{
	if (mrt_isValid(sprt_fb_info))
	{
		kfree(sprt_fb_info);
	}
}

/*!
 * @brief   hal_register_framebuffer
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t hal_register_framebuffer(struct hal_fb_info *sprt_fb_info)
{
	struct hal_fb_info **sprt_exsited;
	ksint32_t retval;
	kuint32_t i;

	if (!mrt_isValid(sprt_fb_info))
	{
		return mrt_retval(Ert_isUnvalid);
	}

	sprt_exsited = &sgrt_hal_registered_fb[0];

	/*!< Search, find an empty location */
	for (i = 0; i < HAL_FB_DEVICE_MAX; i++)
	{
		if (!mrt_isValid(sprt_exsited[i]))
		{
			break;
		}
	}

	/*!< The retrieval failed, and the number of fb devices has reached the upper limit */
	if (i == HAL_FB_DEVICE_MAX)
	{
		return mrt_retval(Ert_isArrayOver);
	}

	/*!< Save the secondary device number */
	sprt_fb_info->node = i;

	/*!< Create a character device node */
	retval = hal_device_create(Ert_HAL_TYPE_CHRDEV, MKE_DEV_NUM(HAL_FB_DEVICE_MAJOR, i), "fb%d", i);
	if (mrt_isErr(retval))
	{
		return mrt_retval(Ert_isUnvalid);
	}

	/*!< Register to the global array */
	sprt_exsited[i] = sprt_fb_info;

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   hal_unregister_framebuffer
 * @param   none
 * @retval  none
 * @note    none
 */
void hal_unregister_framebuffer(struct hal_fb_info *sprt_fb_info)
{
	struct hal_fb_info **sprt_exsited;
	kuint32_t i;

	i = sprt_fb_info->node;
	sprt_exsited = &sgrt_hal_registered_fb[0];

	hal_device_destroy("fb%d", i);
	sprt_exsited[i] = mrt_nullptr;
}

/*!
 * @brief   hal_get_fb_info
 * @param   none
 * @retval  none
 * @note    none
 */
struct hal_fb_info *hal_get_fb_info(kuint32_t idx)
{
	return sgrt_hal_registered_fb[idx];
}

/*!
 * @brief   hal_file_fb_info
 * @param   none
 * @retval  none
 * @note    none
 */
struct hal_fb_info *hal_file_fb_info(struct hal_file *sprt_file)
{
	struct hal_inode *sprt_inode;
	kuint32_t fbidx;

	sprt_inode  = RET_INODE_FROM_FILE(sprt_file);
	fbidx		= RET_INODE_MINOR(sprt_inode);

	return sgrt_hal_registered_fb[fbidx];
}

/*!< ------------------------------------------------------------------------- */
/*!< Frambuffer driver Interface */
/*!< The globals */
static struct hal_cdev *sprt_hal_fb_cdev;

/*!< ------------------------------------------------------------------------- */
/*!
 * @brief   hal_fb_open
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_fb_open(struct hal_inode *sprt_inode, struct hal_file *sprt_file)
{
	struct hal_fb_info *sprt_info;
	kuint32_t fbidx;
	ksint32_t retval;

	fbidx = RET_INODE_MINOR(sprt_inode);
	sprt_info = hal_get_fb_info(fbidx);
	if (!mrt_isValid(sprt_info))
	{
		return mrt_retval(Ert_isArgFault);
	}

	sprt_file->private_data	= sprt_info;

	if (mrt_isValid(sprt_info->sprt_fbops->fb_open))
	{
		retval = sprt_info->sprt_fbops->fb_open(sprt_info, 1);
		if (mrt_isErr(retval))
		{
			/*!< Open fb device failed */
			return mrt_retval(Ert_isAnyErr);
		}
	}

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   hal_fb_close
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_fb_close(struct hal_inode *sprt_inode, struct hal_file *sprt_file)
{
	struct hal_fb_info *sprt_info;

	sprt_info = (struct hal_fb_info *)sprt_file->private_data;
	if (mrt_isValid(sprt_info->sprt_fbops->fb_release))
	{
		sprt_info->sprt_fbops->fb_release(sprt_info, 1);
	}

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   hal_fb_write
 * @param   none
 * @retval  none
 * @note    none
 */
static kssize_t hal_fb_write(struct hal_file *sprt_file, const ksbuffer_t *ptr_buf, kssize_t size)
{
	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   hal_fb_read
 * @param   none
 * @retval  none
 * @note    none
 */
static kssize_t hal_fb_read(struct hal_file *sprt_file, ksbuffer_t *ptr_buf, kssize_t size)
{
	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   hal_fb_ioctl
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_fb_ioctl(struct hal_file *sprt_file, kuint32_t cmd, kuaddr_t args)
{
	struct hal_fb_info *sprt_info;
	struct hal_fb_fix_screen_info sgrt_fix;
	struct hal_fb_var_screen_info sgrt_var;
	kuint8_t *ptr_user;
	ksint32_t retval;

	sprt_info = (struct hal_fb_info *)sprt_file->private_data;
	ptr_user = (kuint8_t *)args;

	switch (cmd)
	{
		case Ert_HAL_FB_IOGetVScreenInfo:
			sgrt_var = sprt_info->sgrt_var;
			retval = hal_copy_to_user(ptr_user, &sgrt_var, sizeof(sgrt_var)) ? mrt_retval(Ert_isWell) : mrt_retval(Ert_isAnyErr);
			break;

		case Ert_HAL_FB_IOPutVScreenInfo:
			if (!hal_copy_to_user(&sgrt_var, ptr_user, sizeof(sgrt_var)))
			{
				return mrt_retval(Ert_isAnyErr);
			}

			/*!< Set the variable parameters */

			break;

		case Ert_HAL_FB_IOGetFScreenInfo:
			sgrt_fix = sprt_info->sgrt_fix;
			retval = hal_copy_to_user(ptr_user, &sgrt_fix, sizeof(sgrt_fix)) ? mrt_retval(Ert_isWell) : mrt_retval(Ert_isAnyErr);
			break;

		default:
			retval = mrt_retval(Ert_isUnvalid);
			break;
	}

	return retval;
}

/*!
 * @brief   hal_fb_mmap
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_fb_mmap(struct hal_file *sprt_file, struct hal_vm_area *vm_area)
{
	struct hal_fb_info *sprt_info;
	ksint32_t retval;

	sprt_info = (struct hal_fb_info *)sprt_file->private_data;
	if (!mrt_isValid(sprt_info))
	{
		return mrt_retval(Ert_isArgFault);
	}

	if (mrt_isValid(sprt_info->sprt_fbops->fb_mmap))
	{
		retval = sprt_info->sprt_fbops->fb_mmap(sprt_info, vm_area);
		if (mrt_isErr(retval))
		{
			/*!< Open fb device failed */
			return mrt_retval(Ert_isAnyErr);
		}
	}

	vm_area->virt_addr = sprt_info->sgrt_fix.smem_start;
	vm_area->size = sprt_info->sgrt_fix.smem_len;

	return mrt_retval(Ert_isWell);
}

static struct hal_file_oprts sgrt_hal_fb_foprts =
{
	.open	= hal_fb_open,
	.close	= hal_fb_close,
	.write	= hal_fb_write,
	.read	= hal_fb_read,
	.unlocked_ioctl	= hal_fb_ioctl,
	.mmap	= hal_fb_mmap,
};

/*!
 * @brief   hal_fbmem_init
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t __dync_init hal_fbmem_init(void)
{
	struct hal_cdev *sprt_cdev;
	kuint32_t devNum;
	ksint32_t retval;

	devNum = MKE_DEV_NUM(HAL_FB_DEVICE_MAJOR, 0);
	retval = hal_register_chrdev(devNum, HAL_FB_DEVICE_MAX, "fb");
	if (mrt_isErr(retval))
	{
		goto fail1;
	}

	sprt_cdev = hal_cdev_alloc(&sgrt_hal_fb_foprts);
	if (!mrt_isValid(sprt_cdev))
	{
		goto fail2;
	}

	retval = hal_cdev_add(sprt_cdev, devNum, HAL_FB_DEVICE_MAX);
	if (mrt_isErr(retval))
	{
		goto fail3;
	}

	/*!< Save to global variable */
	sprt_hal_fb_cdev = sprt_cdev;

	return mrt_retval(Ert_isWell);

fail3:
	hal_cdev_del(sprt_cdev);
fail2:
	hal_unregister_chrdev(devNum, HAL_FB_DEVICE_MAX);
fail1:
	return mrt_retval(Ert_isMemErr);
}

/*!
 * @brief   hal_fbmem_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __dync_exit hal_fbmem_exit(void)
{
	struct hal_cdev *sprt_cdev;
	kuint32_t devNum;

	sprt_cdev = sprt_hal_fb_cdev;
	devNum = sprt_cdev->devNum;

	hal_cdev_del(sprt_cdev);
	hal_unregister_chrdev(devNum, HAL_FB_DEVICE_MAX);
}

IMPORT_PATTERN_INIT(hal_fbmem_init);
IMPORT_PATTERN_EXIT(hal_fbmem_exit);
