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
 * @param   sptr_chip, sptr_desc
 * @retval  gpio number
 * @note    none
 */
kuint32_t fwk_gpiodesc_to_hwgpio(struct fwk_gpio_chip *sptr_chip, struct fwk_gpio_desc *sptr_desc)
{
    return (kuint32_t)(sptr_desc - &sptr_chip->sptr_desc[0]);
}

/*!
 * @brief   gpio number ---> gpio desc
 * @param   sptr_chip, offset
 * @retval  gpio desc
 * @note    none
 */
struct fwk_gpio_desc *fwk_gpiochip_get_desc(struct fwk_gpio_chip *sptr_chip, kuint32_t offset)
{
    return (offset < sptr_chip->ngpios) ? &sptr_chip->sptr_desc[offset] : mr_nullptr;
}

/*!
 * @brief   get gpio_desc and flag by property name
 * @param   list_name: the full name of property, such as "light-gpios"
 * @retval  none
 * @note    none
 */
struct fwk_gpio_desc *fwk_of_get_named_gpiodesc_flags(struct fwk_device_node *sptr_node, 
                                            const kchar_t *list_name, kint32_t index, kuint32_t *flags)
{
    struct fwk_gpio_desc *sptr_desc;
    struct fwk_gpio_node_prop sgtc_data;
    struct fwk_of_phandle_args sgtc_spec;

    if (fwk_of_parse_phandle_with_args(sptr_node, list_name, "#gpio-cells", 0, index, &sgtc_spec))
        return mr_nullptr;

    sptr_desc = fwk_gpiochip_and_desc_find(&sgtc_data, &sgtc_spec);
    if (!isValid(sptr_desc))
        return sptr_desc;

    if (flags)
        *flags = sgtc_data.flags;

    if (sgtc_data.flags & GPIO_ACTIVE_LOW)
        fwk_gpio_desc_set_flags(sptr_desc, NR_FWK_GPIODESC_ACTIVE_LOW);
    if (sgtc_data.flags & GPIO_OPEN_DRAIN)
        fwk_gpio_desc_set_flags(sptr_desc, NR_FWK_GPIODESC_OPEN_DRAIN);
    if (sgtc_data.flags & GPIO_OPEN_SOURCE)
        fwk_gpio_desc_set_flags(sptr_desc, NR_FWK_GPIODESC_OPEN_SOURCE);

    return sptr_desc;
}

/*!
 * @brief   get gpio and flag by property name
 * @param   list_name: the full name of property, such as "light-gpios"
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_get_named_gpio_flags(struct fwk_device_node *sptr_node, 
                                            const kchar_t *list_name, kint32_t index, kuint32_t *flags)
{
    struct fwk_gpio_desc *sptr_desc;

    sptr_desc = fwk_of_get_named_gpiodesc_flags(sptr_node, list_name, index, flags);
    if (!isValid(sptr_desc))
        return -ER_NOTFOUND;

    return fwk_gpiodesc_to_hwgpio(sptr_desc->sptr_chip, sptr_desc);
}

/*!
 * @brief   request gpio
 * @param   sptr_desc, label
 * @retval  errno
 * @note    none
 */
kint32_t fwk_gpio_request(struct fwk_gpio_desc *sptr_desc, const kchar_t *label)
{
    struct fwk_gpio_chip *sptr_chip;
    kuint32_t gpio;
    kint32_t retval;

    sptr_chip = sptr_desc->sptr_chip;
    gpio = fwk_gpiodesc_to_hwgpio(sptr_chip, sptr_desc);

    if (sptr_chip->request)
    {
        retval = sptr_chip->request(sptr_chip, gpio);
        if (retval < 0)
            return retval;
    }

    sptr_desc->label = label;

    retval = fwk_gpio_get_direction(sptr_desc);
    if (retval < 0)
        return retval;

    return ER_NORMAL;
}

/*!
 * @brief   release gpio
 * @param   sptr_desc
 * @retval  none
 * @note    none
 */
void fwk_gpio_free(struct fwk_gpio_desc *sptr_desc)
{
    struct fwk_gpio_chip *sptr_chip;
    kuint32_t gpio;

    sptr_chip = sptr_desc->sptr_chip;
    gpio = fwk_gpiodesc_to_hwgpio(sptr_chip, sptr_desc);

    if (sptr_chip->free)
        sptr_chip->free(sptr_chip, gpio);

    sptr_desc->label = mr_nullptr;
    fwk_gpio_desc_set_flags(sptr_desc, NR_FWK_GPIODESC_IS_OUT);
}

/*!
 * @brief   check if gpio is requested
 * @param   sptr_desc
 * @retval  status
 * @note    none
 */
kbool_t fwk_gpio_is_requested(struct fwk_gpio_desc *sptr_desc)
{
    return fwk_gpio_desc_check_flags(sptr_desc, NR_FWK_GPIODESC_REQUESTED);
}

/*!
 * @brief   get gpio_desc by property name
 * @param   con_id: the major name of property, such as "light", without "-gpios"
 * @retval  none
 * @note    none
 */
struct fwk_gpio_desc *fwk_gpio_desc_get(struct fwk_device *sptr_dev, const kchar_t *con_id, kuint32_t flags)
{
    struct fwk_gpio_desc *sptr_desc;
    kchar_t list_name[16] = {0};
    kuint32_t of_flags;

    if (!sptr_dev || !con_id)
        return mr_nullptr;

    sprintk(list_name, "%s-gpios", con_id);

    sptr_desc = fwk_of_get_named_gpiodesc_flags(sptr_dev->sptr_node, list_name, 0, &of_flags);
    if (!isValid(sptr_desc))
        return mr_nullptr;

    if (fwk_gpio_request(sptr_desc, con_id))
        return mr_nullptr;

    if (!(flags & NR_FWK_GPIO_BIT_SET))
        return sptr_desc;

    if (flags & NR_FWK_GPIO_BIT_OUT)
        fwk_gpio_set_direction_output(sptr_desc, flags & NR_FWK_GPIO_BIT_VAL);
    else
        fwk_gpio_set_direction_input(sptr_desc);

    return sptr_desc;
}

/*!
 * @brief   release gpio
 * @param   sptr_desc
 * @retval  none
 * @note    none
 */
void fwk_gpio_desc_put(struct fwk_gpio_desc *sptr_desc)
{
    fwk_gpio_free(sptr_desc);

    fwk_gpio_set_direction_output(sptr_desc, 0);
    fwk_gpio_desc_set_flags(sptr_desc, NR_FWK_GPIODESC_ACTIVE_LOW);
}

/*!
 * @brief   gpio_desc ---> irq
 * @param   sptr_desc
 * @retval  errno
 * @note    none
 */
kint32_t fwk_gpio_desc_to_irq(struct fwk_gpio_desc *sptr_desc)
{
    struct fwk_gpio_chip *sptr_chip;
    kuint32_t offset, irq;

    sptr_chip = sptr_desc->sptr_chip;
    offset = fwk_gpiodesc_to_hwgpio(sptr_chip, sptr_desc);

    if (sptr_chip->to_irq)
    {
        irq = sptr_chip->to_irq(sptr_chip, offset);
        if (irq >= 0)
        {
            fwk_gpio_desc_set_flags(sptr_desc, NR_FWK_GPIODESC_USED_AS_IRQ);
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
void fwk_gpio_set_value(struct fwk_gpio_desc *sptr_desc, kuint32_t value)
{
    struct fwk_gpio_chip *sptr_chip;
    kuint32_t offset;

    sptr_chip = sptr_desc->sptr_chip;
    offset = fwk_gpiodesc_to_hwgpio(sptr_chip, sptr_desc);

    if (!fwk_gpio_desc_check_flags(sptr_desc, NR_FWK_GPIODESC_IS_OUT))
        return;

    /*!< if active level is 0, it means that level 0 is logic 1 */
    if (fwk_gpio_desc_check_flags(sptr_desc, NR_FWK_GPIODESC_ACTIVE_LOW))
        value = !value;

    if (sptr_chip->set)
        sptr_chip->set(sptr_chip, offset, value);
}

/*!
 * @brief   fwk_gpio_get_value
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_gpio_get_value(struct fwk_gpio_desc *sptr_desc)
{
    struct fwk_gpio_chip *sptr_chip;
    kuint32_t offset, value;

    sptr_chip = sptr_desc->sptr_chip;
    offset = fwk_gpiodesc_to_hwgpio(sptr_chip, sptr_desc);

    if (sptr_chip->get)
    {
        value = sptr_chip->get(sptr_chip, offset);
        if (value < 0)
            return -ER_IOERR;

        /*!< if active level is 0, it means that level 0 is logic 1 */
        if (fwk_gpio_desc_check_flags(sptr_desc, NR_FWK_GPIODESC_ACTIVE_LOW))
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
void fwk_gpio_set_direction(struct fwk_gpio_desc *sptr_desc, kuint32_t dir, kint32_t value)
{
    struct fwk_gpio_chip *sptr_chip;
    kuint32_t offset;

    sptr_chip = sptr_desc->sptr_chip;
    offset = fwk_gpiodesc_to_hwgpio(sptr_chip, sptr_desc);
    
    if ((dir & NR_FWK_GPIO_BIT_OUT) && sptr_chip->direction_output)
    {       
        if (!sptr_chip->direction_output(sptr_chip, offset, value))
            fwk_gpio_desc_set_flags(sptr_desc, NR_FWK_GPIODESC_IS_OUT);

        return;
    }

    if (sptr_chip->direction_input)
    {
        if (!sptr_chip->direction_input(sptr_chip, offset))
            fwk_gpio_desc_clr_flags(sptr_desc, NR_FWK_GPIODESC_IS_OUT);
    }
}

/*!
 * @brief   configure gpio direction to input
 * @param   sptr_desc
 * @retval  none
 * @note    none
 */
void fwk_gpio_set_direction_input(struct fwk_gpio_desc *sptr_desc)
{
    fwk_gpio_set_direction(sptr_desc, FWK_GPIO_DIR_IN, 0);
}

/*!
 * @brief   configure gpio direction to output
 * @param   sptr_desc
 * @retval  none
 * @note    none
 */
void fwk_gpio_set_direction_output(struct fwk_gpio_desc *sptr_desc, kint32_t value)
{
    if (value && fwk_gpio_desc_check_flags(sptr_desc, NR_FWK_GPIODESC_OPEN_DRAIN))
        goto set_input;

    if (!value && fwk_gpio_desc_check_flags(sptr_desc, NR_FWK_GPIODESC_OPEN_SOURCE))
        goto set_input;

    /*!< if active level is 0, it means that level 0 is logic 1 */
    if (fwk_gpio_desc_check_flags(sptr_desc, NR_FWK_GPIODESC_ACTIVE_LOW))
        value = !value;

    fwk_gpio_set_direction(sptr_desc, FWK_GPIO_DIR_OUT, value);

    return;

set_input:
    fwk_gpio_set_direction_input(sptr_desc);
}

/*!
 * @brief   fwk_gpio_get_value
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_gpio_get_direction(struct fwk_gpio_desc *sptr_desc)
{
    struct fwk_gpio_chip *sptr_chip;
    kuint32_t offset, dir;

    sptr_chip = sptr_desc->sptr_chip;
    offset = fwk_gpiodesc_to_hwgpio(sptr_chip, sptr_desc);

    if (sptr_chip->get_direction)
    {
        dir = sptr_chip->get_direction(sptr_chip, offset);
        if (dir & NR_FWK_GPIO_BIT_OUT)
            fwk_gpio_desc_set_flags(sptr_desc, NR_FWK_GPIODESC_IS_OUT);
        else
            fwk_gpio_desc_clr_flags(sptr_desc, NR_FWK_GPIODESC_IS_OUT);

        return (dir & NR_FWK_GPIO_BIT_OUT);
    }

    return -ER_IOERR;
}

/*!
 * @brief   check if gpio direction is input
 * @param   sptr_desc
 * @retval  direction is input ?
 * @note    none
 */
kbool_t fwk_gpio_dir_is_input(struct fwk_gpio_desc *sptr_desc)
{
    kint32_t retval;

    retval = fwk_gpio_get_direction(sptr_desc);
    if (retval < 0)
        return false;

    return !(retval & NR_FWK_GPIO_BIT_OUT);
}

/*!< end of file */
