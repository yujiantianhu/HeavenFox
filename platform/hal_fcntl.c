/*
 * Platform File System Controller
 *
 * File Name:   hal_fcntl.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.28
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/hal_basic.h>
#include <platform/hal_fs.h>
#include <platform/hal_kobj.h>
#include <platform/hal_cdev.h>
#include <platform/hal_inode.h>
#include <platform/hal_fcntl.h>
#include <kernel/spinlock.h>

/*!< The globals */
static struct hal_file_table sgrt_hal_file_table =
{
	.max_fdarr	= 0,
	.max_fds	= FILE_DESC_NUM_MAX,
	.max_fdset	= -1,
	.ref_fdarr	= 0,

	.fds		= mrt_nullptr,
	.fd_array	= { mrt_nullptr },
};

static struct hal_file sgrt_hal_file_stdio[DEVICE_MAJOR_BASE] =
{
	{
		.sprt_inode		= mrt_nullptr,
		.sprt_foprts	= mrt_nullptr,
	},
	{
		.sprt_inode		= mrt_nullptr,
		.sprt_foprts	= mrt_nullptr,
	},
	{
		.sprt_inode		= mrt_nullptr,
		.sprt_foprts	= mrt_nullptr,
	},
};

/*!< API function */
/*!
 * @brief   hal_file_system_init
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t __plat_init hal_file_system_init(void)
{
	struct hal_file_table *sprt_table;
	kusize_t  num_farray;
	kuint32_t fileCnt;

	sprt_table = &sgrt_hal_file_table;
	num_farray = ARRAY_SIZE(sprt_table->fd_array);

	if (!sprt_table->max_fds)
	{
		sprt_table->max_fds	= num_farray;
	}

	sprt_table->max_fdarr = mrt_ret_min2(sprt_table->max_fds, num_farray);
	num_farray = ARRAY_SIZE(sgrt_hal_file_stdio);

	if (sprt_table->max_fdarr < num_farray)
	{
		return mrt_retval(Ert_isArrayOver);
	}

	/*!< Occupy the top three */
	for (fileCnt = 0; fileCnt < num_farray; fileCnt++)
	{
		sprt_table->fd_array[fileCnt] = &sgrt_hal_file_stdio[fileCnt];
	}

	sprt_table->max_fds		= sprt_table->max_fdarr;
	sprt_table->ref_fdarr	= fileCnt;
	sprt_table->max_fdset 	= mrt_cmp_gt(num_farray, 0, num_farray - 1, sprt_table->max_fdset);

	return mrt_retval(Ert_isWell);
}
IMPORT_PLATFORM_INIT(hal_file_system_init);

/*!
 * @brief   hal_get_expand_fdtable
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_get_expand_fdtable(struct hal_file_table *sprt_table, kuint32_t flags)
{
	ksint32_t index;

	/*!< Created for the first time */
	if (!mrt_isValid(sprt_table->fds))
	{
		sprt_table->fds	= (struct hal_file **)kzalloc(sizeof(struct hal_file) * FILE_DESC_EXP_NUM, GFP_KERNEL);
	}

	/*!< It still doesn't exist after it was created */
	if (!mrt_isValid(sprt_table->fds))
	{
		goto fail;
	}

	/*!< Fetch an unused index from fds */
	for (index = 0; index < FILE_DESC_EXP_NUM; index++)
	{
		if (!mrt_isValid(sprt_table->fds[index]))
		{
			goto fdget;
		}
	}

	/*!< Not found */
	goto fail;

fdget:
	index += sprt_table->max_fdarr;
	sprt_table->max_fdset = mrt_ret_max2(sprt_table->max_fdset, index);
	sprt_table->max_fds	= sprt_table->max_fdarr	+ FILE_DESC_EXP_NUM;

	return index;

fail:
	return -1;
}

/*!
 * @brief   hal_put_expand_fdtable
 * @param   none
 * @retval  none
 * @note    none
 */
static void hal_put_expand_fdtable(struct hal_file_table *sprt_table, ksint32_t fd)
{
	ksint32_t index;

	/*!< The array does not exist */
	if (!mrt_isValid(sprt_table->fds) || (fd < sprt_table->max_fdarr))
	{
		return;
	}

	index = fd - sprt_table->max_fdarr;
	sprt_table->fds[index] = mrt_nullptr;

	sprt_table->max_fdset = mrt_ret_max_super(sprt_table->max_fdset, fd, fd - 1);

	/*!< The extended array has all been emptied. Free up space and save memory */
	if (!index)
	{
		kfree(sprt_table->fds);

		sprt_table->fds 	= mrt_nullptr;
		sprt_table->max_fds = sprt_table->max_fdarr;
	}
}

/*!
 * @brief   hal_get_unused_fd_flags
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_get_unused_fd_flags(kuint32_t flags)
{
	struct hal_file_table *sprt_table;
	ksint32_t index;

	sprt_table = &sgrt_hal_file_table;

	/*!< Check if fd_array are fully assigned */
	index = hal_get_fd_available(sprt_table);
	if (index < 0)
	{
		goto expand;
	}

	/*!< Priority is given to backward allocation */
	index = mrt_ret_min_super(sprt_table->max_fdset + 1, sprt_table->max_fdarr, -1);
	if (index >= 0)
	{
		sprt_table->max_fdset++;
		goto fdget;
	}

	/*!< Fetch an unused index from the file_array */
	for (index = 0; index < sprt_table->max_fdset; index++)
	{
		if (!mrt_isValid(sprt_table->fd_array[index]))
		{
			goto fdget;
		}
	}

	/*!< Fail: hal_get_fd_available check failed || RET_MIN_SUPER fail to get || for loop calls the overrun */
	return mrt_retval(Ert_isArrayOver);

fdget:
	hal_fdtable_get(sprt_table);
	return index;

expand:
	/*!< fd has run out */
	return hal_get_expand_fdtable(sprt_table, flags);
}

/*!
 * @brief   hal_put_used_fd_flags
 * @param   none
 * @retval  none
 * @note    none
 */
static void hal_put_used_fd_flags(ksint32_t fd)
{
	struct hal_file_table *sprt_table;
	ksint32_t index;

	if (FILE_DESC_OVER_BASE(fd))
	{
		return;
	}

	sprt_table = &sgrt_hal_file_table;
	index = fd;

	if (index >= sprt_table->max_fdarr)
	{
		goto expand;
	}

	sprt_table->fd_array[index]	= mrt_nullptr;
	sprt_table->max_fdset = mrt_ret_max_super(sprt_table->max_fdset, fd, fd - 1);
	hal_fdtable_put(sprt_table);

	return;

expand:
	/*!< fd has run out */
	hal_put_expand_fdtable(sprt_table, fd);
}

/*!
 * @brief   hal_fd_install
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_fd_install(ksint32_t fd, struct hal_file *sprt_file)
{
	struct hal_file_table *sprt_table;
	struct hal_file  **sprt_fdt;
	ksint32_t index;

	if (FILE_DESC_OVER_BASE(fd) || !mrt_isValid(sprt_file))
	{
		return mrt_retval(Ert_isUnvalid);
	}

	sprt_table	= &sgrt_hal_file_table;
	sprt_fdt	= sprt_table->fd_array;
	index		= fd;

	if (fd >= sprt_table->max_fdarr)
	{
		sprt_fdt = sprt_table->fds;
		index = fd - sprt_table->max_fdarr;
	}

	if (mrt_isValid(sprt_fdt[index]))
	{
		return mrt_retval(Ert_isUnvalid);
	}

	sprt_fdt[index] = sprt_file;

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   hal_fd_to_file
 * @param   none
 * @retval  none
 * @note    none
 */
static struct hal_file *hal_fd_to_file(ksint32_t fd)
{
	struct hal_file_table *sprt_table;
	struct hal_file  **sprt_fdt;
	ksint32_t index;

	if (FILE_DESC_OVER_BASE(fd))
	{
		return mrt_nullptr;
	}

	sprt_table	= &sgrt_hal_file_table;
	sprt_fdt	= sprt_table->fd_array;
	index		= fd;

	if (fd >= sprt_table->max_fdarr)
	{
		sprt_fdt = sprt_table->fds;
		index = fd - sprt_table->max_fdarr;
	}

	return sprt_fdt[index];
}

/*!
 * @brief   hal_do_open
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_do_open(const kstring_t *dev, kuint32_t mode)
{
	struct hal_file  *sprt_file;
	ksint32_t fd;
	ksint32_t retval;

	fd = hal_get_unused_fd_flags(mode);
	if (fd < 0)
	{
		goto fail1;
	}

	sprt_file = hal_do_filp_open((kstring_t *)dev, mode);
	if (!mrt_isValid(sprt_file))
	{
		goto fail2;
	}

	retval = hal_fd_install(fd, sprt_file);
	if (mrt_isErr(retval))
	{
		goto fail3;
	}

	return fd;

fail3:
	hal_do_filp_close(sprt_file);
fail2:
	hal_put_used_fd_flags(fd);
fail1:
	return mrt_retval(Ert_isArgFault);
}

/*!
 * @brief   hal_do_close
 * @param   none
 * @retval  none
 * @note    none
 */
static void hal_do_close(ksint32_t fd)
{
	struct hal_file *sprt_file;

	sprt_file = hal_fd_to_file(fd);
	if (!mrt_isValid(sprt_file))
	{
		return;
	}

	hal_do_filp_close(sprt_file);
	hal_put_used_fd_flags(fd);
}

/*!
 * @brief   hal_do_write
 * @param   none
 * @retval  none
 * @note    none
 */
static kssize_t hal_do_write(ksint32_t fd, const void *buf, kusize_t size)
{
	struct hal_file *sprt_file;
	ksint32_t retval;

	if (fd < 0)
	{
		return mrt_retval(Ert_isAnyErr);
	}

	sprt_file = hal_fd_to_file(fd);
	if (!mrt_isValid(sprt_file))
	{
		return mrt_retval(Ert_isAnyErr);
	}

	if (mrt_isValid(sprt_file->sprt_foprts->write))
	{
		retval = sprt_file->sprt_foprts->write(sprt_file, (const ksbuffer_t *)buf, size);
		if (!mrt_isErr(retval))
		{
			return size;
		}
	}

	return mrt_retval(Ert_isAnyErr);
}

/*!
 * @brief   hal_do_read
 * @param   none
 * @retval  none
 * @note    none
 */
static kssize_t hal_do_read(ksint32_t fd, void *buf, kusize_t size)
{
	struct hal_file *sprt_file;
	ksint32_t retval;

	if (fd < 0)
	{
		return mrt_retval(Ert_isAnyErr);
	}

	sprt_file = hal_fd_to_file(fd);
	if (!mrt_isValid(sprt_file))
	{
		return mrt_retval(Ert_isAnyErr);
	}

	if (mrt_isValid(sprt_file->sprt_foprts->read))
	{
		retval = sprt_file->sprt_foprts->read(sprt_file, (ksbuffer_t *)buf, size);
		if (!mrt_isErr(retval))
		{
			return size;
		}
	}

	return mrt_retval(Ert_isAnyErr);
}

/*!
 * @brief   hal_do_ioctl
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_do_ioctl(ksint32_t fd, kuint32_t request, kuaddr_t args)
{
	struct hal_file *sprt_file;
	ksint32_t retval;

	if (fd < 0)
	{
		return mrt_retval(Ert_isAnyErr);
	}

	sprt_file = hal_fd_to_file(fd);
	if (!mrt_isValid(sprt_file))
	{
		return mrt_retval(Ert_isAnyErr);
	}

	if (mrt_isValid(sprt_file->sprt_foprts->unlocked_ioctl))
	{
		retval = sprt_file->sprt_foprts->unlocked_ioctl(sprt_file, request, args);
		if (!mrt_isErr(retval))
		{
			return mrt_retval(Ert_isWell);
		}
	}

	return mrt_retval(Ert_isAnyErr);
}

/*!
 * @brief   hal_do_mmap
 * @param   none
 * @retval  none
 * @note    none
 */
static void *hal_do_mmap(void *addr, kusize_t length, ksint32_t prot, ksint32_t flags, ksint32_t fd, kuint32_t offset)
{
	struct hal_vm_area sgrt_vm;
	struct hal_file *sprt_file;
	ksint32_t retval;

	if (fd < 0)
	{
		return mrt_nullptr;
	}

	sprt_file = hal_fd_to_file(fd);
	if (!mrt_isValid(sprt_file))
	{
		return mrt_nullptr;
	}

	if (mrt_isValid(sprt_file->sprt_foprts->mmap))
	{
		retval = sprt_file->sprt_foprts->mmap(sprt_file, &sgrt_vm);
		if (mrt_isErr(retval))
		{
			return mrt_nullptr;
		}
	}

	if ((offset + length) > sgrt_vm.size)
	{
		return mrt_nullptr;
	}

	if (!mrt_isValid(addr))
	{
		return (void *)sgrt_vm.virt_addr;
	}

	return mrt_nullptr;
}

/*!< ------------------------------------------------------------ */
/*!
 * @brief   virt_open
 * @param   none
 * @retval  none
 * @note    The interface is provided for use by the application layer
 */
ksint32_t virt_open(const kstring_t *dev, kuint32_t mode)
{
	return hal_do_open(dev, mode);
}

/*!
 * @brief   virt_close
 * @param   none
 * @retval  none
 * @note    The interface is provided for use by the application layer
 */
void virt_close(ksint32_t fd)
{
	return hal_do_close(fd);
}

/*!
 * @brief   virt_write
 * @param   none
 * @retval  none
 * @note    The interface is provided for use by the application layer
 */
kssize_t virt_write(ksint32_t fd, const void *buf, kusize_t size)
{
	return hal_do_write(fd, buf, size);
}

/*!
 * @brief   virt_read
 * @param   none
 * @retval  none
 * @note    The interface is provided for use by the application layer
 */
kssize_t virt_read(ksint32_t fd, void *buf, kusize_t size)
{
	return hal_do_read(fd, buf, size);
}

/*!
 * @brief   virt_ioctl
 * @param   none
 * @retval  none
 * @note    The interface is provided for use by the application layer
 */
ksint32_t virt_ioctl(ksint32_t fd, kuint32_t request, ...)
{
	va_list ptr_list;
	kuaddr_t args;
	ksint32_t retval;

	va_start(ptr_list, request);

	args = (kuaddr_t)va_arg(ptr_list, void *);
	retval = hal_do_ioctl(fd, request, args);
	va_end(ptr_list);

	return retval;
}

/*!
 * @brief   virt_mmap
 * @param   none
 * @retval  none
 * @note    The interface is provided for use by the application layer
 */
void *virt_mmap(void *addr, kusize_t length, ksint32_t prot, ksint32_t flags, ksint32_t fd, kuint32_t offset)
{
	return hal_do_mmap(addr, length, prot, flags, fd, offset);
}

/*!
 * @brief   virt_munmap
 * @param   none
 * @retval  none
 * @note    The interface is provided for use by the application layer
 */
ksint32_t virt_munmap(void *addr, kusize_t length)
{
	return 0;
}

/*!< end of file */
