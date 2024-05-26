/*
 * General SD Card FatFs Interface
 *
 * File Name:   fwk_sdfatfs.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.11.11
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef _FWK_SDFATFS_H_
#define _FWK_SDFATFS_H_

/*!< The includes */
#include <common/generic.h>
#include <common/api_string.h>
#include <rootfs/fatfs/ff.h>
#include <rootfs/fatfs/diskio.h>
#include <rootfs/rfs_disk.h>

#ifdef __cplusplus
	TARGET_EXT "C" {
#endif

/*!< called by fatfs */
TARGET_EXT DRESULT fwk_sdfatfs_write(kuint8_t physicalDrive, const kuint8_t *buffer, kuint32_t sector, kuint8_t count);
TARGET_EXT DRESULT fwk_sdfatfs_read(kuint8_t physicalDrive, kuint8_t *buffer, kuint32_t sector, kuint8_t count);
TARGET_EXT DRESULT fwk_sdfatfs_ioctl(kuint8_t physicalDrive, kuint8_t command, void *buffer);
TARGET_EXT DSTATUS fwk_sdfatfs_status(kuint8_t physicalDrive);
TARGET_EXT DSTATUS fwk_sdfatfs_initial(kuint8_t physicalDrive);
TARGET_EXT DSTATUS fwk_sdfatfs_release(kuint8_t physicalDrive);

/*!< called by main */
TARGET_EXT void rfs_fatfs_file_initial(struct rfs_disk_file *sprt_file);
TARGET_EXT void *rfs_fatfs_file_allocate(void);
TARGET_EXT void rfs_fatfs_file_free(struct rfs_disk_file *sprt_file);
TARGET_EXT kint32_t rfs_fatfs_disk_create(struct rfs_disk *sprt_disk, kuint16_t drvNumber);
TARGET_EXT void rfs_fatfs_disk_destroy(struct rfs_disk *sprt_disk);

#ifdef __cplusplus
	}
#endif

#endif /* _FWK_SDFATFS_H_ */
