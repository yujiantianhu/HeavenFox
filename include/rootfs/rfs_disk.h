/*
 * General SD Card FatFs Interface
 *
 * File Name:   rfs_disk.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.11.11
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __RFS_DISK_H_
#define __RFS_DISK_H_

/*!< The includes */
#include <common/generic.h>
#include <common/api_string.h>
#include <common/io_stream.h>

#ifdef __cplusplus
	TARGET_EXT "C" {
#endif

/*!< The defines */
enum __ENUM_RFS_DISK_FILE_MODE
{
	/*!< if file has exsisted, "open" function will break directelly */
	NR_RFS_DISK_OpenExsiting = mrt_bit(1U),

	/*!< if file is not exsisted, create new */
	NR_RFS_DISK_OpenCreate = mrt_bit(2U),

	/*!< promit writting to file */
	NR_RFS_DISK_OpenWrite = mrt_bit(3U),

	/*!< promit reading from file */
	NR_RFS_DISK_OpenRead = mrt_bit(4U),

	/*!< append content to file, like: lseek(f_size(sprt_file)) */
	NR_RFS_DISK_AppendEnd = mrt_bit(5U),

	/*!< clear content of file after openning it */
	NR_RFS_DISK_OpenClear = mrt_bit(6U),
};

/*!< for the whole disk */
typedef struct rfs_disk
{
	void (*set_diskPath)(struct rfs_disk *sprt_disk, kuint16_t drvNumber);

	kint32_t (*mount)(struct rfs_disk *sprt_disk);
	kint32_t (*unmount)(struct rfs_disk *sprt_disk);
	kint32_t (*mkfs)(struct rfs_disk *sprt_disk);
	kint32_t (*mkdir)(const kchar_t *ptrDirName);

	void 	   *sprt_fs;
	kuint8_t   	diskPath[10U];
	kbool_t		mnt_status;

} srt_rfs_disk_t;

/*!< for a file */
typedef struct rfs_disk_file
{
	kint32_t (*init)(struct rfs_disk_file *sprt_this, const kchar_t *ptrName, kuint8_t flags, kuaddr_t addr_base, kuint32_t size);
	void (*exit)(struct rfs_disk_file *sprt_this);

	void (*set_fileName)(struct rfs_disk_file *sprt_this, const kchar_t *ptrFileName);
	kbool_t (*is_mounted)(struct rfs_disk_file *sprt_this);
	kbool_t (*is_opened)(struct rfs_disk_file *sprt_this);

	kint32_t (*open)(struct rfs_disk_file *sprt_this, kuint8_t flags);
	kint32_t (*lseek)(struct rfs_disk_file *sprt_this, kuint32_t offset);
	kusize_t  (*write)(struct rfs_disk_file *sprt_this, const void *ptrBuffer, kuint32_t size, kuint32_t offset);
	kusize_t  (*read)(struct rfs_disk_file *sprt_this, void *ptrBuffer, kuint32_t size, kuint32_t offset);
	kusize_t  (*fsize)(struct rfs_disk_file *sprt_this);
	kint32_t (*close)(struct rfs_disk_file *sprt_this);

	/*!< common resource */
	struct rfs_disk *sprt_disk;

	/*!< private variables */
	kusize_t	fileSize;
	kuint8_t   *recvBuffer;

	void	   *sprt_file;
	kuint8_t   *ptrFilePath;
	kuint8_t   *ptrFileName;
	kuint8_t	mode;

	/*!< bool: if file opened */
	kbool_t		opened;
	/*!< bool: if recvBuffer is from heap(kmalloc) */
	kbool_t		isBufferDync;
	/*!< bool: if the instance of "srt_rfs_disk_file" is form heap(kmalloc) */
	kbool_t		isSelfDync;

} srt_rfs_disk_file_t;


#ifdef __cplusplus
	}
#endif

#endif /* __RFS_DISK_H_ */
