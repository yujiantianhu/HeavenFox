/*
 * Platform Basic Defines
 *
 * File Name:   hal_basic.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.22
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_BASIC_H_
#define __HAL_BASIC_H_

/*!< The includes */
#include <common/generic.h>
#include <common/list_types.h>
#include <common/api_string.h>
#include <common/io_stream.h>
#include <common/time.h>
#include <board/board_common.h>
#include <boot/boot_text.h>
#include <boot/implicit_call.h>
#include <platform/kmem_pool.h>

/*!< The defines */
enum __ERT_HAL_PIN_DIRECTION
{
	Ert_HAL_IOPortDirInput = 0,
	Ert_HAL_IOPortDirOutput,
};

/*!< Port Level Defines */
enum __ERT_HAL_PIN_LEVEL
{
	Ert_HAL_IOPortLowLevel = 0,
	Ert_HAL_IOPorHighLevel,
};

enum __ERT_HAL_PIN_ACTIVE
{
	Ert_HAL_IOPorActiveLow = 0,
	Ert_HAL_IOPorActiveHigh,
};

enum __ERT_HAL_DEVICE_TYPE
{
	/*!< character device */
	Ert_HAL_TYPE_CHRDEV	= 0,
	/*!< block device */
	Ert_HAL_TYPE_BLKDEV,
	/*!< network device */
	Ert_HAL_TYPE_NETDEV,
};

#define USE_VIRTUAL_MEM_ADDR						(0)						/*!< Whether virtual memory is used. 0: not use, virtual memory = physic memory */

#define DEVICE_MAX_NUM								(255)					/*!< Maximum number of device numbers */
#define DEVICE_MAJOR_BASE							(3)						/*!< The primary device number starts (0, 1, 2 are reserved.)） */
#define DEVICE_NAME_LEN								(32)					/*!< The maximum number of bytes allowed for a device name */

/*!< Maximum number of secondary devices: 2 ^ 20, Maximum number of primary devices: min(DEVICE_MAX_NUM, 2 ^ 12) */
#define DEV_NUM_BITMASK(bits)                   	((1u << (bits)) - 1)
#define DEV_MINOR_BITS                          	(20)
#define DEV_MAJOR_BITS                          	(32 - (DEV_MINOR_BITS))
#define MKE_DEV_NUM(major, minor)               	((kuint32_t)(((major) << (DEV_MINOR_BITS)) | (minor)))
#define GET_DEV_MAJOR(devNum)                   	((kuint32_t)((devNum) >> (DEV_MINOR_BITS)))
#define GET_DEV_MINOR(devNum)                   	((kuint32_t)((devNum) & (DEV_NUM_BITMASK(DEV_MINOR_BITS))))

#define HAL_IOC_NONE								(0U)
#define HAL_IOC_WRITE								(1U)
#define HAL_IOC_READ								(2U)

#define HAL_IOC_NRBITS								(8)
#define HAL_IOC_NRSHIFT	            				(0)
#define HAL_IOC_NRMASK								(0xff)

#define HAL_IOC_TYPEBITS							(8)
#define HAL_IOC_TYPESHIFT	            			(HAL_IOC_NRSHIFT + HAL_IOC_NRBITS)
#define HAL_IOC_TYPEMASK							(0xff)

#define HAL_IOC_SIZEBITS							(14)
#define HAL_IOC_SIZESHIFT	            			(HAL_IOC_TYPESHIFT + HAL_IOC_TYPEBITS)
#define HAL_IOC_SIZEMASK							(0x3fff)

#define HAL_IOC_DIRBITS								(2)
#define HAL_IOC_DIRSHIFT	            			(HAL_IOC_SIZESHIFT + HAL_IOC_SIZEBITS)
#define HAL_IOC_DIRMASK								(0x03)

#define HAL_IOC_TYPECHECK(t)           				sizeof(t)

#define HAL_IOC(dir, type, nr, size) \
	(((dir)  << HAL_IOC_DIRSHIFT) | ((type) << HAL_IOC_TYPESHIFT) | \
	 ((nr)   << HAL_IOC_NRSHIFT)  | ((size) << HAL_IOC_SIZESHIFT))

#define HAL_IOWR(type, nr, size)           			HAL_IOC(HAL_IOC_READ | HAL_IOC_WRITE, (type), (nr), (HAL_IOC_TYPECHECK(size)))
#define HAL_IOW(type, nr, size)           			HAL_IOC(HAL_IOC_WRITE, (type), (nr), (HAL_IOC_TYPECHECK(size)))
#define HAL_IOR(type, nr, size)           			HAL_IOC(HAL_IOC_READ, (type), (nr), (HAL_IOC_TYPECHECK(size)))
#define HAL_ION(type, nr, size)           			HAL_IOC(HAL_IOC_NONE, (type), (nr), (HAL_IOC_TYPECHECK(size)))

#define HAL_IOC_DIR(nr)   							(((nr) >> HAL_IOC_DIRSHIFT) & HAL_IOC_DIRMASK)
#define HAL_IOC_SIZE(nr)   							(((nr) >> HAL_IOC_SIZESHIFT) & HAL_IOC_SIZEMASK)
#define HAL_IOC_TYPE(nr)   							(((nr) >> HAL_IOC_TYPESHIFT) & HAL_IOC_TYPEMASK)
#define HAL_IOC_NR(nr)   							(((nr) >> HAL_IOC_NRSHIFT) & HAL_IOC_NRMASK)

#endif /*!< __HAL_BASIC_H_ */
