/*
 * Platform File System Controller
 *
 * File Name:   fwk_fcntl.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.28
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/base/fwk_fs.h>
#include <platform/base/fwk_kobj.h>
#include <platform/base/fwk_cdev.h>
#include <platform/base/fwk_inode.h>
#include <platform/base/fwk_fcntl.h>
#include <platform/net/fwk_netif.h>
#include <platform/net/fwk_socket.h>
#include <kernel/spinlock.h>

/*!< The globals */
static struct fwk_file_table sgtc_fwk_file_table =
{
    .max_fdarr	= 0,
    .max_fds	= FILE_DESC_NUM_MAX,
    .max_fdset	= -1,
    .ref_fdarr	= 0,

    .fds		= mr_nullptr,
    .fd_array	= { mr_nullptr },
};

static struct fwk_file sgtc_fwk_file_stdio[DEVICE_MAJOR_BASE] =
{
    {
        .sptr_inode		= mr_nullptr,
        .sptr_foprts	= mr_nullptr,
    },
    {
        .sptr_inode		= mr_nullptr,
        .sptr_foprts	= mr_nullptr,
    },
    {
        .sptr_inode		= mr_nullptr,
        .sptr_foprts	= mr_nullptr,
    },
};

/*!< API function */
/*!
 * @brief   fwk_file_system_init
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t __plat_init fwk_file_system_init(void)
{
    struct fwk_file_table *sptr_table;
    kusize_t  num_farray;
    kuint32_t fileCnt;

    sptr_table = &sgtc_fwk_file_table;
    mutex_init(&sptr_table->sgtc_mutex);

    num_farray = ARRAY_SIZE(sptr_table->fd_array);

    if (!sptr_table->max_fds)
        sptr_table->max_fds	= num_farray;

    sptr_table->max_fdarr = CMP_MIN2(sptr_table->max_fds, num_farray);
    num_farray = ARRAY_SIZE(sgtc_fwk_file_stdio);

    if (sptr_table->max_fdarr < num_farray)
        return -ER_MORE;

    /*!< Occupy the top three */
    for (fileCnt = 0; fileCnt < num_farray; fileCnt++)
        sptr_table->fd_array[fileCnt] = &sgtc_fwk_file_stdio[fileCnt];

    sptr_table->max_fds		= sptr_table->max_fdarr;
    sptr_table->ref_fdarr	= fileCnt;
    sptr_table->max_fdset 	= CMP_GT2(num_farray, 0, num_farray - 1, sptr_table->max_fdset);

    return ER_NORMAL;
}

/*!
 * @brief   fwk_get_expand_fdtable
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_get_expand_fdtable(struct fwk_file_table *sptr_table, kuint32_t flags)
{
    kint32_t index;

    /*!< Created for the first time */
    if (!isValid(sptr_table->fds))
        sptr_table->fds	= (struct fwk_file **)kcalloc(sizeof(struct fwk_file), FILE_DESC_EXP_NUM, GFP_KERNEL);

    /*!< It still doesn't exist after it was created */
    if (!isValid(sptr_table->fds))
        goto fail;

    /*!< Fetch an unused index from fds */
    for (index = 0; index < FILE_DESC_EXP_NUM; index++)
    {
        if (!sptr_table->fds[index])
            goto fdget;
    }

    /*!< Not found */
    goto fail;

fdget:
    index += sptr_table->max_fdarr;
    sptr_table->max_fdset = mr_ret_max2(sptr_table->max_fdset, index);
    sptr_table->max_fds	= sptr_table->max_fdarr	+ FILE_DESC_EXP_NUM;

    return index;

fail:
    return -ER_ERROR;
}

/*!
 * @brief   fwk_put_expand_fdtable
 * @param   none
 * @retval  none
 * @note    none
 */
static void fwk_put_expand_fdtable(struct fwk_file_table *sptr_table, kint32_t fd)
{
    kint32_t index;

    /*!< The array does not exist */
    if (!isValid(sptr_table->fds) || (fd < sptr_table->max_fdarr))
        return;

    index = fd - sptr_table->max_fdarr;
    sptr_table->fds[index] = mr_nullptr;

    sptr_table->max_fdset = CMP_GTS(sptr_table->max_fdset, fd, fd - 1);

    /*!< The extended array has all been emptied. Free up space and save memory */
    if (!index)
    {
        kfree(sptr_table->fds);

        sptr_table->fds 	= mr_nullptr;
        sptr_table->max_fds = sptr_table->max_fdarr;
    }
}

/*!
 * @brief   fwk_get_unused_fd_flags
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_get_unused_fd_flags(kuint32_t flags)
{
    struct fwk_file_table *sptr_table;
    kint32_t index;

    sptr_table = &sgtc_fwk_file_table;

    /*!< Check if fd_array are fully assigned */
    index = fwk_get_fd_available(sptr_table);
    if (index < 0)
        goto expand;

    mutex_lock(&sptr_table->sgtc_mutex);

    /*!< Priority is given to backward allocation */
    index = CMP_LTS(sptr_table->max_fdset + 1, sptr_table->max_fdarr, -1);
    if (index >= 0)
    {
        sptr_table->max_fdset++;
        goto fdget;
    }

    /*!< Fetch an unused index from the file_array */
    for (index = 0; index < sptr_table->max_fdset; index++)
    {
        if (!sptr_table->fd_array[index])
            goto fdget;
    }

    mutex_unlock(&sptr_table->sgtc_mutex);

    /*!< Fail: fwk_get_fd_available check failed || RET_MIN_SUPER fail to get || for loop calls the overrun */
    return -ER_MORE;

fdget:
    fwk_fdtable_get(sptr_table);
    mutex_unlock(&sptr_table->sgtc_mutex);

    return index;

expand:
    /*!< fd has run out */
    index = fwk_get_expand_fdtable(sptr_table, flags);
    mutex_unlock(&sptr_table->sgtc_mutex);

    return index;
}

/*!
 * @brief   fwk_put_used_fd_flags
 * @param   none
 * @retval  none
 * @note    none
 */
static void fwk_put_used_fd_flags(kint32_t fd)
{
    struct fwk_file_table *sptr_table;
    kint32_t index;

    if (FILE_DESC_OVER_BASE(fd))
        return;

    sptr_table = &sgtc_fwk_file_table;
    index = fd;

    if (index >= sptr_table->max_fdarr)
        goto expand;

    mutex_lock(&sptr_table->sgtc_mutex);

    sptr_table->fd_array[index]	= mr_nullptr;
    sptr_table->max_fdset = CMP_GTS(sptr_table->max_fdset, fd, fd - 1);
    fwk_fdtable_put(sptr_table);

    mutex_unlock(&sptr_table->sgtc_mutex);

    return;

expand:
    mutex_lock(&sptr_table->sgtc_mutex);
    /*!< fd has run out */
    fwk_put_expand_fdtable(sptr_table, fd);
    mutex_unlock(&sptr_table->sgtc_mutex);
}

/*!
 * @brief   fwk_fd_install
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_fd_install(kint32_t fd, struct fwk_file *sptr_file)
{
    struct fwk_file_table *sptr_table;
    struct fwk_file **sptr_fdt;
    kint32_t index;

    if (FILE_DESC_OVER_BASE(fd) || !isValid(sptr_file))
        return -ER_INVALID;

    sptr_table	= &sgtc_fwk_file_table;
    sptr_fdt	= sptr_table->fd_array;
    index		= fd;

    if (fd >= sptr_table->max_fdarr)
    {
        sptr_fdt = sptr_table->fds;
        index = fd - sptr_table->max_fdarr;
    }

    if (sptr_fdt[index])
        return -ER_INVALID;

    mutex_lock(&sptr_table->sgtc_mutex);
    sptr_fdt[index] = sptr_file;
    mutex_unlock(&sptr_table->sgtc_mutex);

    return ER_NORMAL;
}

/*!
 * @brief   fwk_fd_to_file
 * @param   none
 * @retval  none
 * @note    none
 */
static struct fwk_file *fwk_fd_to_file(kint32_t fd)
{
    struct fwk_file_table *sptr_table;
    struct fwk_file  **sptr_fdt;
    kint32_t index;

    if (FILE_DESC_OVER_BASE(fd) ||
        (fd >= FILE_DESC_TOT_NUM))
        return mr_nullptr;

    sptr_table	= &sgtc_fwk_file_table;
    sptr_fdt	= sptr_table->fd_array;
    index		= fd;

    if (fd >= sptr_table->max_fdarr)
    {
        sptr_fdt = sptr_table->fds;
        index = fd - sptr_table->max_fdarr;
    }

    return sptr_fdt[index];
}

/*!
 * @brief   fwk_do_open
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_do_open(const kchar_t *dev, kuint32_t mode)
{
    struct fwk_file  *sptr_file;
    kint32_t fd;
    kint32_t retval;

    fd = fwk_get_unused_fd_flags(mode);
    if (fd < 0)
        goto fail1;

    sptr_file = fwk_do_filp_open((kchar_t *)dev, mode);
    if (!isValid(sptr_file))
        goto fail2;

    retval = fwk_fd_install(fd, sptr_file);
    if (retval)
        goto fail3;

    return fd;

fail3:
    fwk_do_filp_close(sptr_file);
fail2:
    fwk_put_used_fd_flags(fd);
fail1:
    return -ER_FAULT;
}

/*!
 * @brief   fwk_do_close
 * @param   none
 * @retval  none
 * @note    none
 */
static void fwk_do_close(kint32_t fd)
{
    struct fwk_file *sptr_file;

    sptr_file = fwk_fd_to_file(fd);
    if (!isValid(sptr_file))
        return;

    fwk_do_filp_close(sptr_file);
    fwk_put_used_fd_flags(fd);
}

/*!
 * @brief   fwk_do_write
 * @param   none
 * @retval  none
 * @note    none
 */
static kssize_t fwk_do_write(kint32_t fd, const void *buf, kusize_t size)
{
    struct fwk_file *sptr_file;
    kint32_t retval;

    if (fd < 0)
        return -ER_ERROR;

    sptr_file = fwk_fd_to_file(fd);
    if (!isValid(sptr_file))
        return -ER_ERROR;

    if ((sptr_file->mode & O_WRONLY) != O_WRONLY)
        return -ER_FORBID;

    if (sptr_file->sptr_foprts->write)
    {
        retval = sptr_file->sptr_foprts->write(sptr_file, (const kbuffer_t *)buf, size);
        if (!retval)
            return size;
    }

    return -ER_ERROR;
}

/*!
 * @brief   fwk_do_read
 * @param   none
 * @retval  none
 * @note    none
 */
static kssize_t fwk_do_read(kint32_t fd, void *buf, kusize_t size)
{
    struct fwk_file *sptr_file;
    kint32_t retval;

    if (fd < 0)
        return -ER_ERROR;

    sptr_file = fwk_fd_to_file(fd);
    if (!isValid(sptr_file))
        return -ER_ERROR;

    if ((sptr_file->mode & O_RDONLY) != O_RDONLY)
        return -ER_FORBID;

    if (sptr_file->sptr_foprts->read)
    {
        retval = sptr_file->sptr_foprts->read(sptr_file, (kbuffer_t *)buf, size);
        if (!retval)
            return size;
        else if (retval > 0)
            return retval;
    }

    return -ER_ERROR;
}

/*!
 * @brief   fwk_do_ioctl
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_do_ioctl(kint32_t fd, kuint32_t request, kuaddr_t args)
{
    struct fwk_file *sptr_file;
    kint32_t retval;

    if (fd < 0)
        return -ER_ERROR;

    sptr_file = fwk_fd_to_file(fd);
    if (!isValid(sptr_file))
        return -ER_ERROR;

    if (sptr_file->sptr_foprts->unlocked_ioctl)
    {
        retval = sptr_file->sptr_foprts->unlocked_ioctl(sptr_file, request, args);
        if (!retval)
            return ER_NORMAL;
    }

    return -ER_ERROR;
}

/*!
 * @brief   fwk_do_mmap
 * @param   none
 * @retval  none
 * @note    none
 */
static void *fwk_do_mmap(void *addr, kusize_t length, kint32_t prot, kint32_t flags, kint32_t fd, kuint32_t offset)
{
    struct fwk_vm_area sgtc_vm;
    struct fwk_file *sptr_file;
    kint32_t retval;

    if (fd < 0)
        return mr_nullptr;

    sptr_file = fwk_fd_to_file(fd);
    if (!isValid(sptr_file))
        return mr_nullptr;

    sgtc_vm.offset = offset;
    if (sptr_file->sptr_foprts->mmap)
    {
        retval = sptr_file->sptr_foprts->mmap(sptr_file, &sgtc_vm);
        if (retval)
            return mr_nullptr;
    }

    if (length > sgtc_vm.size)
        return mr_nullptr;
    
    if (!isValid(addr))
        return (void *)sgtc_vm.virt_addr;

    return mr_nullptr;
}

/*!< ------------------------------------------------------------ */
/*!
 * @brief   virt_open
 * @param   none
 * @retval  none
 * @note    The interface is provided for use by the application layer
 */
kint32_t virt_open(const kchar_t *dev, kuint32_t mode)
{
    return fwk_do_open(dev, mode);
}

/*!
 * @brief   virt_close
 * @param   none
 * @retval  none
 * @note    The interface is provided for use by the application layer
 */
void virt_close(kint32_t fd)
{
    if (fd >= NETWORK_SOCKETS_BASE)
        return network_close(fd);

    return fwk_do_close(fd);
}

/*!
 * @brief   virt_write
 * @param   none
 * @retval  none
 * @note    The interface is provided for use by the application layer
 */
kssize_t virt_write(kint32_t fd, const void *buf, kusize_t size)
{
    if (fd >= NETWORK_SOCKETS_BASE)
        return ER_NORMAL;

    return fwk_do_write(fd, buf, size);
}

/*!
 * @brief   virt_read
 * @param   none
 * @retval  none
 * @note    The interface is provided for use by the application layer
 */
kssize_t virt_read(kint32_t fd, void *buf, kusize_t size)
{
    if (fd >= NETWORK_SOCKETS_BASE)
        return ER_NORMAL;

    return fwk_do_read(fd, buf, size);
}

/*!
 * @brief   virt_ioctl
 * @param   none
 * @retval  none
 * @note    The interface is provided for use by the application layer
 */
kint32_t virt_ioctl(kint32_t fd, kuint32_t request, ...)
{
    va_list ptr_list;
    kuaddr_t args;
    kint32_t retval = ER_NORMAL;

    va_start(ptr_list, request);
    args = (kuaddr_t)va_arg(ptr_list, void *);

    if (fd >= NETWORK_SOCKETS_BASE)
        retval = fwk_netif_ioctl(request, args);
    else
        retval = fwk_do_ioctl(fd, request, args);

    va_end(ptr_list);
    return retval;
}

/*!
 * @brief   virt_mmap
 * @param   none
 * @retval  none
 * @note    The interface is provided for use by the application layer
 */
void *virt_mmap(void *addr, kusize_t length, kint32_t prot, kint32_t flags, kint32_t fd, kuint32_t offset)
{
    return fwk_do_mmap(addr, length, prot, flags, fd, offset);
}

/*!
 * @brief   virt_munmap
 * @param   none
 * @retval  none
 * @note    The interface is provided for use by the application layer
 */
kint32_t virt_munmap(void *addr, kusize_t length)
{
    return 0;
}

/*!< end of file */
