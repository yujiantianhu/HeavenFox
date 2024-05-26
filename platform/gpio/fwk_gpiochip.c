/*
 * Hardware Abstraction Layer Gpio Interface
 *
 * File Name:   fwk_gpiochip.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.29
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/gpio/fwk_gpiochip.h>
#include <platform/gpio/fwk_gpiodesc.h>

/*!< The globals */
static DECLARE_LIST_HEAD(sgrt_fwk_gpiochip_list);

/*!< API function */
static kint32_t fwk_gpiochip_request(struct fwk_gpio_chip *sprt_chip, kuint32_t offset)
{
    struct fwk_gpio_desc *sprt_desc;

    if (!sprt_chip || (offset >= sprt_chip->ngpios))
        return -NR_IS_NODEV;

    sprt_desc = fwk_gpiochip_get_desc(sprt_chip, offset);
    if (!isValid(sprt_desc))
        return -NR_IS_NODEV;

    if (fwk_gpio_desc_check_flags(sprt_desc, NR_FWK_GPIODESC_REQUESTED))
        return -NR_IS_USED;

    fwk_gpio_desc_set_flags(sprt_desc, NR_FWK_GPIODESC_REQUESTED);

    return offset;
}

static void fwk_gpiochip_free(struct fwk_gpio_chip *sprt_chip, kuint32_t offset)
{
    struct fwk_gpio_desc *sprt_desc;

    if (!sprt_chip || (offset >= sprt_chip->ngpios))
        return;

    sprt_desc = fwk_gpiochip_get_desc(sprt_chip, offset);
    if (!isValid(sprt_desc))
        return;
    
    sprt_desc->flags = 0;
}

static kint32_t fwk_gpiochip_of_xlate(struct fwk_gpio_chip *sprt_chip,
							const struct fwk_of_phandle_args *sprt_spec, kuint32_t *flags)
{
    kuint32_t base = 0;

    if (!sprt_chip || !sprt_spec || !sprt_spec->sprt_node)
        return -NR_IS_NODEV;

    if ((sprt_chip->sprt_dev->sprt_node != sprt_spec->sprt_node) ||
        (sprt_chip->of_gpio_n_cells != sprt_spec->args_count))
        return -NR_IS_CHECKERR;

    if (sprt_spec->args_count == 3)
        base = 1;

    if (flags && (sprt_spec->args_count > 1))
        *flags = sprt_spec->args[base + 1];

    return sprt_spec->args[base];
}

kint32_t fwk_gpiochip_init(struct fwk_gpio_chip *sprt_chip, struct fwk_device *sprt_dev, kuint32_t base, kuint32_t size)
{
    if (!sprt_chip || !sprt_dev)
        return -NR_IS_NODEV;

    sprt_chip->sprt_dev = sprt_dev;
    sprt_chip->name = mrt_dev_get_name(sprt_dev);

    sprt_chip->base = base;
    sprt_chip->ngpios = size << 3;

    if (!sprt_chip->request)
        sprt_chip->request = fwk_gpiochip_request;
    
    if (!sprt_chip->free)
        sprt_chip->free = fwk_gpiochip_free;

    if (!sprt_chip->of_xlate)
        sprt_chip->of_xlate = fwk_gpiochip_of_xlate;

    init_list_head(&sprt_chip->sgrt_pin_ranges);

    return NR_IS_NORMAL;
}

kint32_t fwk_gpiochip_add(struct fwk_gpio_chip *sprt_chip)
{
    struct fwk_gpio_desc *sprt_desc;
    kuint32_t i;

    if (!sprt_chip)
        return -NR_IS_NODEV;

    sprt_desc = kzalloc(sprt_chip->ngpios * sizeof(*sprt_desc), GFP_KERNEL);
    if (!isValid(sprt_desc))
        return -NR_IS_NOMEM;

    for (i = 0; i < sprt_chip->ngpios; i++)
    {
        sprt_desc[i].sprt_chip = sprt_chip;
        fwk_gpio_desc_set_flags(&sprt_desc[i], NR_FWK_GPIODESC_IS_OUT);
    }

    sprt_chip->sprt_desc = sprt_desc;
    list_head_add_tail(&sgrt_fwk_gpiochip_list, &sprt_chip->sgrt_link);

    return NR_IS_NORMAL;
}

void fwk_gpiochip_del(struct fwk_gpio_chip *sprt_chip)
{
    if (!sprt_chip)
        return;

    if (sprt_chip->sprt_desc)
        kfree(sprt_chip->sprt_desc);
    
    sprt_chip->sprt_desc = mrt_nullptr;
    list_head_del(&sprt_chip->sgrt_link);
}

struct fwk_gpio_desc *fwk_gpiochip_and_desc_find(struct fwk_gpio_node_prop *sprt_data, struct fwk_of_phandle_args *sprt_spec)
{
    struct fwk_gpio_chip *sprt_chip;
    kint32_t gpio;
    kuint32_t flags;

    if (!sprt_spec || !sprt_data)
        return mrt_nullptr;

    foreach_list_next_entry(sprt_chip, &sgrt_fwk_gpiochip_list, sgrt_link)
    {
        gpio = sprt_chip->of_xlate(sprt_chip, sprt_spec, &flags);
        if (gpio < 0)
            continue;

        sprt_data->sprt_par = sprt_spec->sprt_node;
        sprt_data->gpio = gpio;
        sprt_data->flags = flags;
        
        /*!< found */
        return fwk_gpiochip_get_desc(sprt_chip, gpio);
    }

    return mrt_nullptr;
}

/*!< end of file */
