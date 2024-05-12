/*
 * Hardware Abstraction Layer Gpio Interface
 *
 * File Name:   fwk_gpiochip.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.27
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_GPIOCHIP_H_
#define __FWK_GPIOCHIP_H_

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_platform.h>
#include <platform/of/fwk_of.h>

/*!< The defines */
typedef struct fwk_gpio_desc srt_fwk_gpio_desc_t;
typedef struct fwk_gpio_chip srt_fwk_gpio_chip_t;
typedef struct fwk_gpio_node_prop srt_fwk_gpio_node_prop_t;

typedef struct fwk_gpio_chip
{
    const kstring_t *name;
    struct fwk_device *sprt_dev;
    ksint32_t base;
    kuint32_t ngpios;
    struct fwk_gpio_desc *sprt_desc;
    struct list_head sgrt_link;
	struct list_head sgrt_pin_ranges;

	ksint32_t   (*request) (srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset);
	void        (*free) (srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset);
	ksint32_t   (*get_direction) (srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset);
	ksint32_t   (*direction_input) (srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset);
	ksint32_t   (*direction_output) (srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset, ksint32_t value);
	ksint32_t   (*get) (srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset);
	void        (*set) (srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset, ksint32_t value);

	ksint32_t   (*to_irq) (srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset);

	kuint32_t of_gpio_n_cells;
	ksint32_t (*of_xlate)(srt_fwk_gpio_chip_t *sprt_chip,
							const srt_fwk_of_phandle_args_t *sprt_spec, kuint32_t *flags);

} srt_fwk_gpio_chip_t;

/*!< The functions */
TARGET_EXT ksint32_t fwk_gpiochip_init(srt_fwk_gpio_chip_t *sprt_chip, srt_fwk_device_t *sprt_dev, kuint32_t base, kuint32_t size);
TARGET_EXT ksint32_t fwk_gpiochip_add(srt_fwk_gpio_chip_t *sprt_chip);
TARGET_EXT void fwk_gpiochip_del(srt_fwk_gpio_chip_t *sprt_chip);
TARGET_EXT srt_fwk_gpio_desc_t *fwk_gpiochip_and_desc_find(srt_fwk_gpio_node_prop_t *sprt_data, srt_fwk_of_phandle_args_t *sprt_spec);

#endif /*!< __FWK_GPIOCHIP_H_ */
