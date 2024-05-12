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

#include "fwk_gpiochip.h"

/*!< The defines */
enum __ERT_FWK_GPIO_DT_FLAG 
{
	GPIO_ACTIVE_HIGH = (0 << 0),
	GPIO_ACTIVE_LOW = (1 << 0),
	GPIO_OPEN_DRAIN = (1 << 1),
	GPIO_OPEN_SOURCE = (1 << 2),
};

enum __ERT_FWK_GPIO_TEMP
{
    NR_FWK_GPIO_BIT_SET = mrt_bit(0),
    NR_FWK_GPIO_BIT_OUT = mrt_bit(1),
    NR_FWK_GPIO_BIT_VAL = mrt_bit(2),

    NR_FWK_GPIO_NONE = 0,
    NR_FWK_GPIO_DIR_IN = NR_FWK_GPIO_BIT_SET,
    NR_FWK_GPIO_DIR_OUT = NR_FWK_GPIO_BIT_SET | NR_FWK_GPIO_BIT_OUT,
    NR_FWK_GPIO_ACTIVE_LOW = NR_FWK_GPIO_BIT_SET | NR_FWK_GPIO_BIT_OUT,
    NR_FWK_GPIO_ACTIVE_HIGH = NR_FWK_GPIO_BIT_SET | NR_FWK_GPIO_BIT_OUT | NR_FWK_GPIO_BIT_VAL,

#define FWK_GPIO_NONE                           NR_FWK_GPIO_NONE
#define FWK_GPIO_DIR_IN                         NR_FWK_GPIO_DIR_IN
#define FWK_GPIO_DIR_OUT                        NR_FWK_GPIO_DIR_OUT
#define FWK_GPIO_ACTIVE_LOW                     NR_FWK_GPIO_ACTIVE_LOW
#define FWK_GPIO_ACTIVE_HIGH                    NR_FWK_GPIO_ACTIVE_HIGH

#define FWK_GPIO_DIR_MASK                       (FWK_GPIO_DIR_IN | FWK_GPIO_DIR_OUT)
#define FWK_GPIO_ACTIVE_MASK                    (FWK_GPIO_ACTIVE_LOW | FWK_GPIO_ACTIVE_HIGH)
#define FWK_GPIO_DIR_ACTIVE_MASK                (FWK_GPIO_DIR_MASK | FWK_GPIO_ACTIVE_MASK)
};

typedef enum __ERT_FWK_GPIODESC_FLAGS
{
    NR_FWK_GPIODESC_REQUESTED = 0,              /*!< request */
    NR_FWK_GPIODESC_IS_OUT,                     /*!< direction is out */
    NR_FWK_GPIODESC_TRIG_FALL,                  /*!< trigger on falling edge */
    NR_FWK_GPIODESC_TRIG_RISE,                  /*!< trigger on rising edge */
    NR_FWK_GPIODESC_ACTIVE_LOW,                 /*!< value has active low */
    NR_FWK_GPIODESC_OPEN_DRAIN,                 /*!< open drain type */
    NR_FWK_GPIODESC_OPEN_SOURCE,                /*!< open source type */
    NR_FWK_GPIODESC_USED_AS_IRQ,                /*!< onnected to an IRQ */
    NR_FWK_GPIODESC_IS_HOGGED                   /*!< is hogged */

/*!< max is 16 */
#define FWK_GPIODESC_FLAG_SHIFT	                16
#define FWK_GPIODESC_FLAGS_MASK		            ((1 << FWK_GPIODESC_FLAG_SHIFT) - 1)
#define FWK_GPIODESC_TRIGGER_MASK	            (mrt_bit(NR_FWK_GPIODESC_TRIG_FALL) | mrt_bit(NR_FWK_GPIODESC_TRIG_RISE))

} ert_fwk_gpiodesc_flag_t;

typedef struct fwk_gpio_desc
{
    struct fwk_gpio_chip *sprt_chip;

    /*!< refer to "__ERT_FWK_GPIODESC_FLAGS" */
    kuint32_t flags;

    /*!< name of gpio property, such as: xxx-gpios, xxx is label*/
    const kstring_t *label;

} srt_fwk_gpio_desc_t;

typedef struct fwk_gpio_node_prop
{
    struct fwk_device_node *sprt_par;
    ksint32_t gpio;
    kuint32_t flags;

} srt_fwk_gpio_node_prop_t;

/*!< The functions */
TARGET_EXT kuint32_t fwk_gpiodesc_to_hwgpio(srt_fwk_gpio_chip_t *sprt_chip, srt_fwk_gpio_desc_t *sprt_desc);
TARGET_EXT srt_fwk_gpio_desc_t *fwk_gpiochip_get_desc(srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset);

TARGET_EXT srt_fwk_gpio_desc_t *fwk_of_get_named_gpiodesc_flags(srt_fwk_device_node_t *sprt_node, 
                                            const kstring_t *list_name, ksint32_t index, kuint32_t *flags);

TARGET_EXT ksint32_t fwk_gpio_request(srt_fwk_gpio_desc_t *sprt_desc, const kstring_t *label);
TARGET_EXT void fwk_gpio_free(srt_fwk_gpio_desc_t *sprt_desc);
TARGET_EXT kbool_t fwk_gpio_is_requested(srt_fwk_gpio_desc_t *sprt_desc);
TARGET_EXT srt_fwk_gpio_desc_t *fwk_gpio_desc_get(srt_fwk_device_t *sprt_dev, const kstring_t *con_id, kuint32_t flags);
TARGET_EXT void fwk_gpio_desc_put(srt_fwk_gpio_desc_t *sprt_desc);
TARGET_EXT ksint32_t fwk_gpio_desc_to_irq(srt_fwk_gpio_desc_t *sprt_desc);
TARGET_EXT void fwk_gpio_set_value(srt_fwk_gpio_desc_t *sprt_desc, kuint32_t value);
TARGET_EXT ksint32_t fwk_gpio_get_value(srt_fwk_gpio_desc_t *sprt_desc);
TARGET_EXT void fwk_gpio_set_direction(srt_fwk_gpio_desc_t *sprt_desc, kuint32_t value);
TARGET_EXT void fwk_gpio_set_direction_input(srt_fwk_gpio_desc_t *sprt_desc);
TARGET_EXT void fwk_gpio_set_direction_output(srt_fwk_gpio_desc_t *sprt_desc, kuint32_t value);
TARGET_EXT ksint32_t fwk_gpio_get_direction(srt_fwk_gpio_desc_t *sprt_desc);
TARGET_EXT kbool_t fwk_gpio_dir_is_input(srt_fwk_gpio_desc_t *sprt_desc);

/*!< API functions */
static inline void fwk_gpio_desc_set_flags(srt_fwk_gpio_desc_t *sprt_desc, ert_fwk_gpiodesc_flag_t flag)
{
    sprt_desc->flags |= mrt_bit(flag);
}

static inline kbool_t fwk_gpio_desc_check_flags(srt_fwk_gpio_desc_t *sprt_desc, ert_fwk_gpiodesc_flag_t flag)
{
    return !!(sprt_desc->flags & mrt_bit(flag));
}

static inline void fwk_gpio_desc_clr_flags(srt_fwk_gpio_desc_t *sprt_desc, ert_fwk_gpiodesc_flag_t flag)
{
    sprt_desc->flags &= ~mrt_bit(flag);
}

#endif /*!< __FWK_GPIODESC_H_ */