/*
 * Platform Bus Interface of Hardware Abstraction Layer
 *
 * File Name:   fwk_platdev.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.23
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_PLATDEV_H_
#define __FWK_PLATDEV_H_

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_platform.h>

/*!< The defines */
enum __ERT_DEVICE_RESOURCE_TYPE
{
	NR_DEVICE_RESOURCE_IO  = 0x00000100U,
	NR_DEVICE_RESOURCE_MEM = 0x00000200U,
	NR_DEVICE_RESOURCE_REG = 0x00000300U,
	NR_DEVICE_RESOURCE_IRQ = 0x00000400U,
	NR_DEVICE_RESOURCE_DMA = 0x00000800U,
	NR_DEVICE_RESOURCE_BUS = 0x00001000U,
};

/*!< Hardware Information Table 1: Obtain hardware by address */
struct fwk_resources
{
	kchar_t *name;
	kuaddr_t start;
	kuaddr_t end;
	kuint32_t type;
};

/*!< Hardware Information Table 2: Obtain hardware by enum */
struct fwk_enums
{
	kint32_t pinEnum;
	kint32_t irqNum;

	void *data;
};

struct fwk_platdev
{
	kchar_t *name;
	kint32_t id;

	struct fwk_resources *sptr_resources;
	kusize_t num_resources;

	struct fwk_enums *sptr_enums;
	kusize_t num_enums;

	kchar_t *driver_override;
	struct fwk_device sgtc_dev;
};

#define RESOURCE_SIZE(sptr_res)						(sptr_res ? (sptr_res->end - sptr_res->start + 1) : 0)

/*!< The functions */
extern kint32_t fwk_device_add(struct fwk_device *sptr_dev);
extern kint32_t fwk_device_del(struct fwk_device *sptr_dev);
extern struct fwk_platdev *fwk_platdevice_alloc(const kchar_t *name, kint32_t id);
extern kint32_t fwk_platdevice_add(struct fwk_platdev *sptr_platdev);
extern kint32_t fwk_register_platdevice(struct fwk_platdev *sptr_platdev);
extern kint32_t fwk_unregister_platdevice(struct fwk_platdev *sptr_platdev);

/*!< API functions */
/*!
 * @brief   caculate address/IRQ size
 * @param   none
 * @retval  none
 * @note    none
 */
static inline kusize_t fwk_resource_size(struct fwk_resources *sptr_res)
{
	return sptr_res ? (sptr_res->end - sptr_res->start + 1) : 0;
}

/*!
 * @brief   return address
 * @param   none
 * @retval  none
 * @note    none
 */
static inline kuaddr_t fwk_address_map(struct fwk_resources *sptr_res)
{
	return sptr_res ? (kuaddr_t)fwk_io_remap((void *)sptr_res->start, sptr_res->end - sptr_res->start + 1) : 0;
}

#ifdef __cplusplus
    }
#endif

#endif /*!< __FWK_PLATDEV_H_ */
