/*
 * Platform File System Controller
 *
 * File Name:   fwk_fcntl.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.28
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_FCNTL_H_
#define __FWK_FCNTL_H_

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_fs.h>

/*!< The defines */
#define FILE_DESC_NUM_MAX										(32)	/*!< The maximum number of default file descriptors that can be assigned */
#define FILE_DESC_EXP_NUM										(128)	/*!< The maximum number of extension file descriptors that can be assigned */

#define FILE_DESC_OVER_BASE(fd)									(fd < DEVICE_MAJOR_BASE)

struct fwk_file_table
{
	ksint32_t max_fdarr;												/*!< size of fd_array */
	ksint32_t max_fds; 													/*!< The maximum number of current file objects = max_fdarr + array number of fds */
	ksint32_t max_fdset;												/*!< The maximum number of current file descriptors */
	ksint32_t ref_fdarr; 												/*!< The number of descriptors that have been assigned on fd_array */

	struct fwk_file **fds;
	struct fwk_file *fd_array[FILE_DESC_NUM_MAX];
};

/*!< The functions */
TARGET_EXT ksint32_t virt_open(const kstring_t *dev, kuint32_t mode);
TARGET_EXT void virt_close(ksint32_t fd);
TARGET_EXT kssize_t virt_write(ksint32_t fd, const void *buf, kusize_t size);
TARGET_EXT kssize_t virt_read(ksint32_t fd, void *buf, kusize_t size);
TARGET_EXT kssize_t virt_ioctl(ksint32_t fd, kuint32_t request, ...);
TARGET_EXT void *virt_mmap(void *addr, kusize_t length, ksint32_t prot, ksint32_t flags, ksint32_t fd, kuint32_t offset);
TARGET_EXT ksint32_t virt_munmap(void *addr, kusize_t length);

/*!< API function */
/*!
 * @brief   Get avaliable index
 * @param   none
 * @retval  none
 * @note    none
 */
static inline ksint32_t fwk_get_fd_available(struct fwk_file_table *sprt_table)
{
	return (sprt_table->ref_fdarr < sprt_table->max_fdarr) ? (sprt_table->ref_fdarr) : (-1);
}

/*!
 * @brief   fwk_fdtable_get
 * @param   none
 * @retval  none
 * @note    none
 */
static inline void fwk_fdtable_get(struct fwk_file_table *sprt_table)
{
	sprt_table->ref_fdarr = (sprt_table->ref_fdarr < sprt_table->max_fdarr) ? (sprt_table->ref_fdarr + 1) : sprt_table->max_fdarr;
}

/*!
 * @brief   fwk_fdtable_put
 * @param   none
 * @retval  none
 * @note    none
 */
static inline void fwk_fdtable_put(struct fwk_file_table *sprt_table)
{
	sprt_table->ref_fdarr = (sprt_table->ref_fdarr > DEVICE_MAJOR_BASE) ? (sprt_table->ref_fdarr - 1) : DEVICE_MAJOR_BASE;
}


#endif /*!< __FWK_FCNTL_H_ */