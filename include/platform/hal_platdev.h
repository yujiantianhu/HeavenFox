/*
 * Platform Bus Interface of Hardware Abstraction Layer
 *
 * File Name:   hal_platdev.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.23
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_PLATDEV_H_
#define __HAL_PLATDEV_H_

/*!< The includes */
#include <platform/hal_basic.h>
#include <platform/hal_platform.h>

/*!< The defines */
enum __ERT_DEVICE_RESOURCE_TYPE
{
	Ert_DEVICE_RESOURCE_IO  = 0x00000100U,
	Ert_DEVICE_RESOURCE_MEM = 0x00000200U,
	Ert_DEVICE_RESOURCE_REG = 0x00000300U,
	Ert_DEVICE_RESOURCE_IRQ = 0x00000400U,
	Ert_DEVICE_RESOURCE_DMA = 0x00000800U,
	Ert_DEVICE_RESOURCE_BUS = 0x00001000U,
};

/*!< Hardware Information Table 1: Obtain hardware by address */
struct hal_resources
{
	kstring_t *name;
	kuaddr_t start;
	kuaddr_t end;
	kuint32_t type;
};

/*!< Hardware Information Table 2: Obtain hardware by enum */
struct hal_enums
{
	ksint32_t pinEnum;
	ksint32_t irqNum;

	void *data;
};

struct hal_platdev
{
	kstring_t *name;
	ksint32_t id;

	struct hal_resources *sprt_resources;
	kusize_t num_resources;

	struct hal_enums *sprt_enums;
	kusize_t num_enums;

	kstring_t *driver_override;
	struct hal_device sgrt_device;
};

#define RESOURCE_SIZE(sprt_res)						(sprt_res ? (sprt_res->end - sprt_res->start + 1) : 0)

/*!< The functions */
TARGET_EXT ksint32_t hal_device_register(struct hal_device *sprt_device);
TARGET_EXT ksint32_t hal_device_unregister(struct hal_device *sprt_device);
TARGET_EXT ksint32_t hal_register_platdevice(struct hal_platdev *sprt_platdev);
TARGET_EXT ksint32_t hal_unregister_platdevice(struct hal_platdev *sprt_platdev);

static inline kusize_t hal_resource_size(struct hal_resources *sprt_res)
{
	return sprt_res ? (sprt_res->end - sprt_res->start + 1) : 0;
}

static inline kuaddr_t hal_address_map(struct hal_resources *sprt_res)
{
	return sprt_res ? sprt_res->start : 0;
}

#endif /*!< __HAL_PLATDEV_H_ */
