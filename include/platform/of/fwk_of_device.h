/*
 * Hardware Abstraction Layer Device-Tree / Platform Device Convertion
 *
 * File Name:   fwk_of_device.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.14
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_OF_DEVICE_H_
#define __FWK_OF_DEVICE_H_

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_prop.h>
#include <platform/fwk_platform.h>
#include <platform/fwk_platdev.h>

/*!< The functions */
extern kint32_t fwk_of_register_platdevice(struct fwk_device_node *sptr_node, struct fwk_platdev *sptr_platdev);
extern void fwk_of_unregister_platdevice(struct fwk_device_node *sptr_node);
extern kint32_t fwk_of_platform_populate_init(void);

extern kint32_t fwk_of_address_to_resource(struct fwk_device_node *sptr_node, kuint32_t index, struct fwk_resources *sptr_res);
extern kint32_t fwk_of_irq_to_resource_table(struct fwk_device_node *sptr_node, struct fwk_resources *sptr_res, kuint32_t nr_irqs);
extern struct fwk_resources *fwk_platform_get_resources(struct fwk_platdev *sptr_pdev, kuint32_t index, kuint32_t type);
extern kuaddr_t fwk_platform_get_address(struct fwk_platdev *sptr_pdev, kuint32_t index);
extern kusize_t fwk_platform_get_address_size(struct fwk_platdev *sptr_pdev, kuint32_t index);
extern kint32_t fwk_platform_get_irq(struct fwk_platdev *sptr_pdev, kuint32_t index);
extern void *fwk_of_iomap(struct fwk_device_node *sptr_node, kuint32_t index);

#ifdef __cplusplus
    }
#endif

#endif /*!< __FWK_OF_DEVICE_H_ */
