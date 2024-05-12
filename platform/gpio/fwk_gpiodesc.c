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
kuint32_t fwk_gpiodesc_to_hwgpio(srt_fwk_gpio_chip_t *sprt_chip, srt_fwk_gpio_desc_t *sprt_desc)
{
    return (kuint32_t)(sprt_desc - &sprt_chip->sprt_desc[0]);
}

srt_fwk_gpio_desc_t *fwk_gpiochip_get_desc(srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset)
{
    return (offset < sprt_chip->ngpios) ? &sprt_chip->sprt_desc[offset] : mrt_nullptr;
}

/*!
 * @brief   get gpio_desc and flag by property name
 * @param   list_name: the full name of property, such as "light-gpios"
 * @retval  none
 * @note    none
 */
srt_fwk_gpio_desc_t *fwk_of_get_named_gpiodesc_flags(srt_fwk_device_node_t *sprt_node, 
                                            const kstring_t *list_name, ksint32_t index, kuint32_t *flags)
{
    srt_fwk_gpio_desc_t *sprt_desc;
    srt_fwk_gpio_node_prop_t sgrt_data;
    srt_fwk_of_phandle_args_t sgrt_spec;

    if (fwk_of_parse_phandle_with_args(sprt_node, list_name, "#gpio-cells", 0, index, &sgrt_spec))
        return mrt_nullptr;

    sprt_desc = fwk_gpiochip_and_desc_find(&sgrt_data, &sgrt_spec);
    if (!isValid(sprt_desc))
        return sprt_desc;

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
ksint32_t fwk_of_get_named_gpio_flags(srt_fwk_device_node_t *sprt_node, 
                                            const kstring_t *list_name, ksint32_t index, kuint32_t *flags)
{
    srt_fwk_gpio_desc_t *sprt_desc;

    sprt_desc = fwk_of_get_named_gpiodesc_flags(sprt_node, list_name, index, flags);
    if (!isValid(sprt_desc))
        return -NR_IS_NOTFOUND;

    return fwk_gpiodesc_to_hwgpio(sprt_desc->sprt_chip, sprt_desc);
}

ksint32_t fwk_gpio_request(srt_fwk_gpio_desc_t *sprt_desc, const kstring_t *label)
{
    srt_fwk_gpio_chip_t *sprt_chip;
    kuint32_t gpio;
    ksint32_t retval;

    sprt_chip = sprt_desc->sprt_chip;
    gpio = fwk_gpiodesc_to_hwgpio(sprt_chip, sprt_desc);

    if (sprt_chip->request)
    {
        if (sprt_chip->request(sprt_chip, gpio))
            return -NR_IS_FAILD;
    }

    sprt_desc->label = label;

    retval = fwk_gpio_get_direction(sprt_desc);
    if (retval < 0)
        return retval;

    return NR_IS_NORMAL;
}

void fwk_gpio_free(srt_fwk_gpio_desc_t *sprt_desc)
{
    srt_fwk_gpio_chip_t *sprt_chip;
    kuint32_t gpio;

    sprt_chip = sprt_desc->sprt_chip;
    gpio = fwk_gpiodesc_to_hwgpio(sprt_chip, sprt_desc);

    if (sprt_chip->free)
        sprt_chip->free(sprt_chip, gpio);

    sprt_desc->label = mrt_nullptr;
    fwk_gpio_desc_set_flags(sprt_desc, NR_FWK_GPIODESC_IS_OUT);
}

kbool_t fwk_gpio_is_requested(srt_fwk_gpio_desc_t *sprt_desc)
{
    return fwk_gpio_desc_check_flags(sprt_desc, NR_FWK_GPIODESC_REQUESTED);
}

/*!
 * @brief   get gpio_desc by property name
 * @param   con_id: the major name of property, such as "light", without "-gpios"
 * @retval  none
 * @note    none
 */
srt_fwk_gpio_desc_t *fwk_gpio_desc_get(srt_fwk_device_t *sprt_dev, const kstring_t *con_id, kuint32_t flags)
{
    srt_fwk_gpio_desc_t *sprt_desc;
    kstring_t list_name[16] = {0};
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

    fwk_gpio_set_direction(sprt_desc, flags & FWK_GPIO_DIR_ACTIVE_MASK);

    return sprt_desc;
}

void fwk_gpio_desc_put(srt_fwk_gpio_desc_t *sprt_desc)
{
    fwk_gpio_free(sprt_desc);

    fwk_gpio_set_direction(sprt_desc, FWK_GPIO_ACTIVE_LOW);
    fwk_gpio_desc_set_flags(sprt_desc, NR_FWK_GPIODESC_ACTIVE_LOW);
}

ksint32_t fwk_gpio_desc_to_irq(srt_fwk_gpio_desc_t *sprt_desc)
{
    srt_fwk_gpio_chip_t *sprt_chip;
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

    return -NR_IS_IOERR;
}

/*!
 * @brief   fwk_gpio_set_value
 * @param   none
 * @retval  none
 * @note    none
 */
void fwk_gpio_set_value(srt_fwk_gpio_desc_t *sprt_desc, kuint32_t value)
{
    srt_fwk_gpio_chip_t *sprt_chip;
    kuint32_t offset;

    sprt_chip = sprt_desc->sprt_chip;
    offset = fwk_gpiodesc_to_hwgpio(sprt_chip, sprt_desc);

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
ksint32_t fwk_gpio_get_value(srt_fwk_gpio_desc_t *sprt_desc)
{
    srt_fwk_gpio_chip_t *sprt_chip;
    kuint32_t offset, value;

    sprt_chip = sprt_desc->sprt_chip;
    offset = fwk_gpiodesc_to_hwgpio(sprt_chip, sprt_desc);

    if (sprt_chip->get)
    {
        value = sprt_chip->get(sprt_chip, offset);
        if (value < 0)
            return -NR_IS_IOERR;

        /*!< if active level is 0, it means that level 0 is logic 1 */
        if (fwk_gpio_desc_check_flags(sprt_desc, NR_FWK_GPIODESC_ACTIVE_LOW))
            value = !value;

        return value;
    }    

    return -NR_IS_IOERR;
}

/*!
 * @brief   fwk_gpio_set_direction
 * @param   value: active low/high
 * @retval  none
 * @note    none
 */
void fwk_gpio_set_direction(srt_fwk_gpio_desc_t *sprt_desc, kuint32_t value)
{
    srt_fwk_gpio_chip_t *sprt_chip;
    kuint32_t offset, dir, val;

    sprt_chip = sprt_desc->sprt_chip;
    offset = fwk_gpiodesc_to_hwgpio(sprt_chip, sprt_desc);
    dir = value & FWK_GPIO_DIR_MASK;
    
    if ((dir & FWK_GPIO_DIR_OUT) && sprt_chip->direction_output)
    {
        val = value & FWK_GPIO_ACTIVE_MASK;
        
        if (!sprt_chip->direction_output(sprt_chip, offset, val))
        {
            fwk_gpio_desc_set_flags(sprt_desc, NR_FWK_GPIODESC_IS_OUT);

            if (!(val & FWK_GPIO_ACTIVE_HIGH))
                fwk_gpio_desc_set_flags(sprt_desc, NR_FWK_GPIODESC_ACTIVE_LOW);
        }

        return;
    }

    if (sprt_chip->direction_input)
    {
        if (sprt_chip->direction_input(sprt_chip, offset))
            fwk_gpio_desc_clr_flags(sprt_desc, NR_FWK_GPIODESC_IS_OUT);
    }
}

void fwk_gpio_set_direction_input(srt_fwk_gpio_desc_t *sprt_desc)
{
    fwk_gpio_set_direction(sprt_desc, FWK_GPIO_DIR_IN);
}

void fwk_gpio_set_direction_output(srt_fwk_gpio_desc_t *sprt_desc, kuint32_t value)
{
    fwk_gpio_set_direction(sprt_desc, value | FWK_GPIO_DIR_OUT);
}

/*!
 * @brief   fwk_gpio_get_value
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t fwk_gpio_get_direction(srt_fwk_gpio_desc_t *sprt_desc)
{
    srt_fwk_gpio_chip_t *sprt_chip;
    kuint32_t offset, dir;

    sprt_chip = sprt_desc->sprt_chip;
    offset = fwk_gpiodesc_to_hwgpio(sprt_chip, sprt_desc);

    if (sprt_chip->get_direction)
    {
        dir = sprt_chip->get_direction(sprt_chip, offset);
        if (dir & FWK_GPIO_DIR_OUT)
            fwk_gpio_desc_set_flags(sprt_desc, NR_FWK_GPIODESC_IS_OUT);
        else
            fwk_gpio_desc_clr_flags(sprt_desc, NR_FWK_GPIODESC_IS_OUT);

        return (dir & FWK_GPIO_DIR_MASK);
    }

    return -NR_IS_IOERR;
}

kbool_t fwk_gpio_dir_is_input(srt_fwk_gpio_desc_t *sprt_desc)
{
    ksint32_t retval;

    retval = fwk_gpio_get_direction(sprt_desc);
    if (retval < 0)
        return false;

    return !(retval & FWK_GPIO_DIR_OUT);
}

/*!< end of file */
