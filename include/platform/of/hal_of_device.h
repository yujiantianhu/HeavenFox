/*
 * Hardware Abstraction Layer Device-Tree / Platform Device Convertion
 *
 * File Name:   hal_of_device.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.14
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_OF_DEVICE_H_
#define __HAL_OF_DEVICE_H_

/*!< The includes */
#include <platform/of/hal_of.h>
#include <platform/of/hal_of_prop.h>
#include <platform/hal_platform.h>
#include <platform/hal_platdev.h>

/*!< The functions */
TARGET_EXT ksint32_t hal_of_register_platdevice(struct hal_device_node *sprt_node, struct hal_platdev *sprt_platdev);
TARGET_EXT void hal_of_unregister_platdevice(struct hal_device_node *sprt_node);

TARGET_EXT ksint32_t hal_of_address_to_resource(struct hal_device_node *sprt_node, kuint32_t index, struct hal_resources *sprt_res);
TARGET_EXT ksint32_t hal_of_irq_to_resource_table(struct hal_device_node *sprt_node, struct hal_resources *sprt_res, kuint32_t nr_irqs);
TARGET_EXT struct hal_resources *hal_platform_get_resources(struct hal_platdev *sprt_pdev, kuint32_t index, kuint32_t type);
TARGET_EXT kuaddr_t hal_platform_get_address(struct hal_platdev *sprt_pdev, kuint32_t index);
TARGET_EXT ksint32_t hal_platform_get_irq(struct hal_platdev *sprt_pdev, kuint32_t index);
TARGET_EXT kuaddr_t hal_of_iomap(struct hal_device_node *sprt_node, kuint32_t index);

#endif /*!< __HAL_OF_DEVICE_H_ */
