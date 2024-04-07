/*
 * Interrupt Interface Defines
 *
 * File Name:   hal_irq.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.01.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_IRQ_H
#define __HAL_IRQ_H

/*!< The includes */
#include <platform/hal_basic.h>

#ifdef __cplusplus
	TARGET_EXT "C" {
#endif

/*!< The defines */

/*!< The functions */
TARGET_EXT void initIRQ(void);
TARGET_EXT void hal_of_irq_init(const struct hal_of_device_id *sprt_matches);

#ifdef __cplusplus
	}
#endif

#endif /* __HAL_IRQ_H */
