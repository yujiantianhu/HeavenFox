/*
 * Hardware Abstraction Layer Gpio Interface
 *
 * File Name:   fwk_gpiodesc.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.03
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/of/fwk_of.h>
#include <platform/gpio/fwk_gpiodesc.h>

/*!< API function */
/*!
 * @brief   gpiodesc ---> gpio number
 * @param   sprt_chip, sprt_desc
 * @retval  gpio number
 * @note    none
 */
kuint32_t fwk_gpiodesc_to_hwgpio(struct fwk_gpio_chip *sprt_chip, struct fwk_gpio_desc *sprt_desc)
{
    return (kuint32_t)(sprt_desc - &sprt_chip->sprt_desc[0]);
}

/*!
 * @brief   gpio number ---> gpio desc
 * @param   sprt_chip, offset
 * @retval  gpio desc
 * @note    none
 */
struct fwk_gpio_desc *fwk_gpiochip_get_desc(struct fwk_gpio_chip *sprt_chip, kuint32_t offset)
{
    return (offset < sprt_chip->ngpios) ? &sprt_chip->sprt_desc[offset] : mrt_nullptr;
}

/*!
 * @brief   get gpio_desc and flag by property name
 * @param   list_name: the full name of property, such as "light-gpios"
 * @retval  none
 * @note    none
 */
struct fwk_gpio_desc *fwk_of_get_named_gpiodesc_flags(struct fwk_device_node *sprt_node, 
                                            const kchar_t *list_name, kint32_t index, kuint32_t *flags)
{
    struct fwk_gpio_desc *sprt_desc;
    struct fwk_gpio_node_prop sgrt_data;
    struct fwk_of_phandle_args sgrt_spec;

    if (fwk_of_parse_phandle_with_args(sprt_node, list_name, "#gpio-cells", 0, index, &sgrt_spec))
        return mrt_nullptr;

    sprt_desc = fwk_gpiochip_and_desc_find(&sgrt_data, &sgrt_spec);
    if (!isValid(sprt_desc))
        return sprt_desc;

    if (flags)
        *flags = sgrt_data.flags;

    if (sgrt_data.flags & GPIO_ACTIVE_LOW)
        fwk_gpio_desc_set_flags(sprt_desc, NR_FWK_GPIODESC_ACTIVE_LOW);
    if (sgrt_data.flags & GPIO_OPEN_DRAIN)
        fwk_gpio_desc_set_flags(sprt_desc, NR_FWK_GPIODESC_OPEN_DRAIN);
    if (sgrt_data.flags & GPIO_OPEN_SOURCE)
        fwk_gpio_desc_set_flags(sprt_desc, NR_FWK_GPIODESC_OPEN_SOURCE);

    return sprt_desc;
}

/*!
 * @brief   get gpio and flag by property name
 * @param   list_name: the full name of property, such as "light-gpios"
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_get_named_gpio_flags(struct fwk_device_node *sprt_node, 
                                            const kchar_t *list_name, kint32_t index, kuint32_t *flags)
{
    struct fwk_gpio_desc *sprt_desc;

    sprt_desc = fwk_of_get_named_gpiodesc_flags(sprt_node, list_name, index, flags);
    if (!isValid(sprt_desc))
        return -ER_NOTFOUND;

    return fwk_gpiodesc_to_hwgpio(sprt_desc->sprt_chip, sprt_desc);
}

/*!
 * @brief   request gpio
 * @param   sprt_desc, label
 * @retval  errno
 * @note    none
 */
kint32_t fwk_gpio_request(struct fwk_gpio_desc *sprt_desc, const kchar_t *label)
{
    struct fwk_gpio_chip *sprt_chip;
    kuint32_t gpio;
    kint32_t retval;

    sprt_chip = sprt_desc->sprt_chip;
    gpio = fwk_gpiodesc_to_hwgpio(sprt_chip, sprt_desc);

    if (sprt_chip->request)
    {
        retval = sprt_chip->request(sprt_chip, gpio);
        if (retval < 0)
            return retval;
    }

    sprt_desc->label = label;

    retval = fwk_gpio_get_direction(sprt_desc);
    if (retval < 0)
        return retval;

    return ER_NORMAL;
}

/*!
 * @brief   release gpio
 * @param   sprt_desc
 * @retval  none
 * @note    none
 */
void fwk_gpio_free(struct fwk_gpio_desc *sprt_desc)
{
    struct fwk_gpio_chip *sprt_chip;
    kuint32_t gpio;

    sprt_chip = sprt_desc->sprt_chip;
    gpio = fwk_gpiodesc_to_hwgpio(sprt_chip, sprt_desc);

    if (sprt_chip->free)
        sprt_chip->free(sprt_chip, gpio);

    sprt_desc->label = mrt_nullptr;
    fwk_gpio_desc_set_flags(sprt_desc, NR_FWK_GPIODESC_IS_OUT);
}

/*!
 * @brief   check if gpio is requested
 * @param   sprt_desc
 * @retval  status
 * @note    none
 */
kbool_t fwk_gpio_is_requested(struct fwk_gpio_desc *sprt_desc)
{
    return fwk_gpio_desc_check_flags(sprt_desc, NR_FWK_GPIODESC_REQUESTED);
}

/*!
 * @brief   get gpio_desc by property name
 * @param   con_id: the major name of property, such as "light", without "-gpios"
 * @retval  none
 * @note    none
 */
struct fwk_gpio_desc *fwk_gpio_desc_get(struct fwk_device *sprt_dev, const kchar_t *con_id, kuint32_t flags)
{
    struct fwk_gpio_desc *sprt_desc;
    kchar_t list_name[16] = {0};
    kuint32_t of_flags;

    if (!sprt_dev || !con_id)
        return mrt_nullptr;

    sprintk(list_name, "%s-gpios", con_id);

    sprt_desc = fwk_of_get_named_gpiodesc_flags(sprt_dev->sprt_node, list_name, 0, &of_flags);
    if (!isValid(sprt_desc))
        return mrt_nullptr;

    if (fwk_gpio_request(sprt_desc, con_id))
        return mrt_nullptr;

    if (!(flags & NR_FWK_GPIO_BIT_SET))
        return sprt_desc;

    if (flags & NR_FWK_GPIO_BIT_OUT)
        fwk_gpio_set_direction_output(sprt_desc, flags & NR_FWK_GPIO_BIT_VAL);
    else
        fwk_gpio_set_direction_input(sprt_desc);

    return sprt_desc;
}

/*!
 * @brief   release gpio
 * @param   sprt_desc
 * @retval  none
 * @note    none
 */
void fwk_gpio_desc_put(struct fwk_gpio_desc *sprt_desc)
{
    fwk_gpio_free(sprt_desc);

    fwk_gpio_set_direction_output(sprt_desc, 0);
    fwk_gpio_desc_set_flags(sprt_desc, NR_FWK_GPIODESC_ACTIVE_LOW);
}

/*!
 * @brief   gpio_desc ---> irq
 * @param   sprt_desc
 * @retval  errno
 * @note    none
 */
kint32_t fwk_gpio_desc_to_irq(struct fwk_gpio_desc *sprt_desc)
{
    struct fwk_gpio_chip *sprt_chip;
    kuint32_t offset, irq;

    sprt_chip = sprt_desc->sprt_chip;
    offset = fwk_gpiodesc_to_hwgpio(sprt_chip, sprt_desc);

    if (sprt_chip->to_irq)
    {
        irq = sprt_chip->to_irq(sprt_chip, offset);
        if (irq >= 0)
        {
            fwk_gpio_desc_set_flags(sprt_desc, NR_FWK_GPIODESC_USED_AS_IRQ);
            return irq;
        }
    }

    return -ER_IOERR;
}

/*!
 * @brief   fwk_gpio_set_value
 * @param   none
 * @retval  none
 * @note    none
 */
void fwk_gpio_set_value(struct fwk_gpio_desc *sprt_desc, kuint32_t value)
{
    struct fwk_gpio_chip *sprt_chip;
    kuint32_t offset;

    sprt_chip = sprt_desc->sprt_chip;
    offset = fwk_gpiodesc_to_hwgpio(sprt_chip, sprt_desc);

    if (!fwk_gpio_desc_check_flags(sprt_desc, NR_FWK_GPIODESC_IS_OUT))
        return;

    /*!< if active level is 0, it means that level 0 is logic 1 */
    if (fwk_gpio_desc_check_flags(sprt_desc, NR_FWK_GPIODESC_ACTIVE_LOW))
        value = !value;

    if (sprt_chip->set)
        sprt_chip->set(sprt_chip, offset, value);
}

/*!
 * @brief   fwk_gpio_get_value
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_gpio_get_value(struct fwk_gpio_desc *sprt_desc)
{
    struct fwk_gpio_chip *sprt_chip;
    kuint32_t offset, value;

    sprt_chip = sprt_desc->sprt_chip;
    offset = fwk_gpiodesc_to_hwgpio(sprt_chip, sprt_desc);

    if (sprt_chip->get)
    {
        value = sprt_chip->get(sprt_chip, offset);
        if (value < 0)
            return -ER_IOERR;

        /*!< if active level is 0, it means that level 0 is logic 1 */
        if (fwk_gpio_desc_check_flags(sprt_desc, NR_FWK_GPIODESC_ACTIVE_LOW))
            value = !value;

        return value;
    }    

    return -ER_IOERR;
}

/*!
 * @brief   fwk_gpio_set_direction
 * @param   value: active low/high
 * @retval  none
 * @note    none
 */
void fwk_gpio_set_direction(struct fwk_gpio_desc *sprt_desc, kuint32_t dir, kint32_t value)
{
    struct fwk_gpio_chip *sprt_chip;
    kuint32_t offset;

    sprt_chip = sprt_desc->sprt_chip;
    offset = fwk_gpiodesc_to_hwgpio(sprt_chip, sprt_desc);
    
    if ((dir & NR_FWK_GPIO_BIT_OUT) && sprt_chip->direction_output)
    {       
        if (!sprt_chip->direction_output(sprt_chip, offset, value))
            fwk_gpio_desc_set_flags(sprt_desc, NR_FWK_GPIODESC_IS_OUT);

        return;
    }

    if (sprt_chip->direction_input)
    {
        if (!sprt_chip->direction_input(sprt_chip, offset))
            fwk_gpio_desc_clr_flags(sprt_desc, NR_FWK_GPIODESC_IS_OUT);
    }
}

/*!
 * @brief   configure gpio direction to input
 * @param   sprt_desc
 * @retval  none
 * @note    none
 */
void fwk_gpio_set_direction_input(struct fwk_gpio_desc *sprt_desc)
{
    fwk_gpio_set_direction(sprt_desc, FWK_GPIO_DIR_IN, 0);
}

/*!
 * @brief   configure gpio direction to output
 * @param   sprt_desc
 * @retval  none
 * @note    none
 */
void fwk_gpio_set_direction_output(struct fwk_gpio_desc *sprt_desc, kint32_t value)
{
    if (value && fwk_gpio_desc_check_flags(sprt_desc, NR_FWK_GPIODESC_OPEN_DRAIN))
        goto set_input;

    if (!value && fwk_gpio_desc_check_flags(sprt_desc, NR_FWK_GPIODESC_OPEN_SOURCE))
        goto set_input;

    /*!< if active level is 0, it means that level 0 is logic 1 */
    if (fwk_gpio_desc_check_flags(sprt_desc, NR_FWK_GPIODESC_ACTIVE_LOW))
        value = !value;

    fwk_gpio_set_direction(sprt_desc, FWK_GPIO_DIR_OUT, value);

    return;

set_input:
    fwk_gpio_set_direction_input(sprt_desc);
}

/*!
 * @brief   fwk_gpio_get_value
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_gpio_get_direction(struct fwk_gpio_desc *sprt_desc)
{
    struct fwk_gpio_chip *sprt_chip;
    kuint32_t offset, dir;

    sprt_chip = sprt_desc->sprt_chip;
    offset = fwk_gpiodesc_to_hwgpio(sprt_chip, sprt_desc);

    if (sprt_chip->get_direction)
    {
        dir = sprt_chip->get_direction(sprt_chip, offset);
        if (dir & NR_FWK_GPIO_BIT_OUT)
            fwk_gpio_desc_set_flags(sprt_desc, NR_FWK_GPIODESC_IS_OUT);
        else
            fwk_gpio_desc_clr_flags(sprt_desc, NR_FWK_GPIODESC_IS_OUT);

        return (dir & NR_FWK_GPIO_BIT_OUT);
    }

    return -ER_IOERR;
}

/*!
 * @brief   check if gpio direction is input
 * @param   sprt_desc
 * @retval  direction is input ?
 * @note    none
 */
kbool_t fwk_gpio_dir_is_input(struct fwk_gpio_desc *sprt_desc)
{
    kint32_t retval;

    retval = fwk_gpio_get_direction(sprt_desc);
    if (retval < 0)
        return false;

    return !(retval & NR_FWK_GPIO_BIT_OUT);
}

/*!< end of file */
