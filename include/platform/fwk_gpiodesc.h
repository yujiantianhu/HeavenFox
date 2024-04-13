/*
 * Hardware Abstraction Layer Gpio Interface
 *
 * File Name:   fwk_gpiodesc.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.03
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_GPIODESC_H_
#define __FWK_GPIODESC_H_

/*!< The includes */
#include <platform/fwk_basic.h>

/*!< The defines */
struct fwk_gpio_desc
{
    ksint32_t gpio;
};

/*!< The functions */
TARGET_EXT void fwk_gpio_set_value(struct fwk_gpio_desc *sprt_desc, kuint32_t value);
TARGET_EXT kuint32_t fwk_gpio_get_value(struct fwk_gpio_desc *sprt_desc);
TARGET_EXT void fwk_gpio_set_direction(struct fwk_gpio_desc *sprt_desc, kuint32_t value);

#endif /*!< __FWK_GPIODESC_H_ */
