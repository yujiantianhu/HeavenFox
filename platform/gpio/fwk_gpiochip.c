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
#include <kernel/rw_lock.h>

/*!< The globals */
static DECLARE_LIST_HEAD(sgtc_fwk_gpiochip_list);
static struct rw_lock sgtc_gpiochip_lock;

/*!< API function */
/*!
 * @brief   request gpiochip
 * @param   sptr_chip, offset
 * @retval  none
 * @note    none
 */
static kint32_t fwk_gpiochip_request(struct fwk_gpio_chip *sptr_chip, kuint32_t offset)
{
    struct fwk_gpio_desc *sptr_desc;

    if (!sptr_chip || (offset >= sptr_chip->ngpios))
        return -ER_NODEV;

    sptr_desc = fwk_gpiochip_get_desc(sptr_chip, offset);
    if (!isValid(sptr_desc))
        return -ER_NODEV;

    if (fwk_gpio_desc_check_flags(sptr_desc, NR_FWK_GPIODESC_REQUESTED))
        return -ER_USED;

    fwk_gpio_desc_set_flags(sptr_desc, NR_FWK_GPIODESC_REQUESTED);

    return offset;
}

/*!
 * @brief   release gpiochip
 * @param   sptr_chip, offset
 * @retval  none
 * @note    none
 */
static void fwk_gpiochip_free(struct fwk_gpio_chip *sptr_chip, kuint32_t offset)
{
    struct fwk_gpio_desc *sptr_desc;

    if (!sptr_chip || (offset >= sptr_chip->ngpios))
        return;

    sptr_desc = fwk_gpiochip_get_desc(sptr_chip, offset);
    if (!isValid(sptr_desc))
        return;
    
    sptr_desc->flags = 0;
}

/*!
 * @brief   parse gpiochip
 * @param   sptr_chip, sptr_spec
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_gpiochip_of_xlate(struct fwk_gpio_chip *sptr_chip,
							const struct fwk_of_phandle_args *sptr_spec, kuint32_t *flags)
{
    kuint32_t base = 0;

    if (!sptr_chip || !sptr_spec || !sptr_spec->sptr_node)
        return -ER_NODEV;

    if ((sptr_chip->sptr_dev->sptr_node != sptr_spec->sptr_node) ||
        (sptr_chip->of_gpio_n_cells != sptr_spec->args_count))
        return -ER_CHECKERR;

    if (sptr_spec->args_count == 3)
        base = 1;

    if (flags && (sptr_spec->args_count > 1))
        *flags = sptr_spec->args[base + 1];

    return sptr_spec->args[base];
}

/*!
 * @brief   initialize gpiochip
 * @param   sptr_chip, sptr_dev
 * @retval  errno
 * @note    none
 */
kint32_t fwk_gpiochip_init(struct fwk_gpio_chip *sptr_chip, struct fwk_device *sptr_dev, kuint32_t base, kuint32_t size)
{
    if (!sptr_chip || !sptr_dev)
        return -ER_NODEV;

    sptr_chip->sptr_dev = sptr_dev;
    sptr_chip->name = mr_dev_get_name(sptr_dev);

    sptr_chip->base = base;
    sptr_chip->ngpios = size << 3;

    if (!sptr_chip->request)
        sptr_chip->request = fwk_gpiochip_request;
    
    if (!sptr_chip->free)
        sptr_chip->free = fwk_gpiochip_free;

    if (!sptr_chip->of_xlate)
        sptr_chip->of_xlate = fwk_gpiochip_of_xlate;

    init_list_head(&sptr_chip->sgtc_pin_ranges);

    return ER_NORMAL;
}

/*!
 * @brief   add a new gpiochip to global list
 * @param   sptr_chip
 * @retval  errno
 * @note    none
 */
kint32_t fwk_gpiochip_add(struct fwk_gpio_chip *sptr_chip)
{
    struct fwk_gpio_desc *sptr_desc;
    kuint32_t i;

    if (!sptr_chip)
        return -ER_NODEV;

    sptr_desc = kzalloc(sptr_chip->ngpios * sizeof(*sptr_desc), GFP_KERNEL);
    if (!isValid(sptr_desc))
        return -ER_NOMEM;

    for (i = 0; i < sptr_chip->ngpios; i++)
    {
        sptr_desc[i].sptr_chip = sptr_chip;
        fwk_gpio_desc_set_flags(&sptr_desc[i], NR_FWK_GPIODESC_IS_OUT);
    }

    sptr_chip->sptr_desc = sptr_desc;

    wr_lock(&sgtc_gpiochip_lock);
    list_head_add_tail(&sgtc_fwk_gpiochip_list, &sptr_chip->sgtc_link);
    wr_unlock(&sgtc_gpiochip_lock);

    return ER_NORMAL;
}

/*!
 * @brief   delete gpiochip from global list
 * @param   sptr_chip
 * @retval  errno
 * @note    none
 */
void fwk_gpiochip_del(struct fwk_gpio_chip *sptr_chip)
{
    if (!sptr_chip)
        return;

    if (sptr_chip->sptr_desc)
        kfree(sptr_chip->sptr_desc);
    
    sptr_chip->sptr_desc = mr_nullptr;
    list_head_del(&sptr_chip->sgtc_link);
}

/*!
 * @brief   find gpiochip and return gpiodesc
 * @param   sptr_data, sptr_spec
 * @retval  gpio desc
 * @note    none
 */
struct fwk_gpio_desc *fwk_gpiochip_and_desc_find(struct fwk_gpio_node_prop *sptr_data, struct fwk_of_phandle_args *sptr_spec)
{
    struct fwk_gpio_chip *sptr_chip;
    kint32_t gpio;
    kuint32_t flags;

    if (!sptr_spec || !sptr_data)
        return mr_nullptr;

    rd_lock(&sgtc_gpiochip_lock);

    foreach_list_next_entry(sptr_chip, &sgtc_fwk_gpiochip_list, sgtc_link)
    {
        gpio = sptr_chip->of_xlate(sptr_chip, sptr_spec, &flags);
        if (gpio < 0)
            continue;

        rd_unlock(&sgtc_gpiochip_lock);

        sptr_data->sptr_par = sptr_spec->sptr_node;
        sptr_data->gpio = gpio;
        sptr_data->flags = flags;
        
        /*!< found */
        return fwk_gpiochip_get_desc(sptr_chip, gpio);
    }

    rd_unlock(&sgtc_gpiochip_lock);
    return mr_nullptr;
}

/*!< --------------------------------------------------------------------------- */
/*!
 * @brief   gpiochip init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __plat_init fwk_gpiochip_global_init(void)
{
    rw_lock_init(&sgtc_gpiochip_lock);
    return ER_NORMAL;
}

/*!
 * @brief   gpiochip exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __plat_exit fwk_gpiochip_global_exit(void)
{

}

IMPORT_LATE_INIT(fwk_gpiochip_global_init);
IMPORT_LATE_EXIT(fwk_gpiochip_global_exit);

/*!< end of file */
