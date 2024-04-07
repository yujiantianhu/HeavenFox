/*
 * Hardware Abstraction Layer Gpio Interface
 *
 * File Name:   hal_gpiodesc.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.03
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_GPIODESC_H_
#define __HAL_GPIODESC_H_

/*!< The includes */
#include <platform/hal_basic.h>

/*!< The defines */
struct hal_gpio_desc
{
    ksint32_t gpio;
};

/*!< The functions */
TARGET_EXT void hal_gpio_set_value(struct hal_gpio_desc *sprt_desc, kuint32_t value);
TARGET_EXT kuint32_t hal_gpio_get_value(struct hal_gpio_desc *sprt_desc);
TARGET_EXT void hal_gpio_set_direction(struct hal_gpio_desc *sprt_desc, kuint32_t value);

#endif /*!< __HAL_GPIODESC_H_ */
