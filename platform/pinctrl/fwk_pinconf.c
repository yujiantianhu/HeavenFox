/*
 * Hardware Abstraction Layer Pinctrl Interface
 *
 * File Name:   fwk_pinconf.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.22
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_pinctrl.h>

/*!< The defines */

/*!< The globals */

/*!< API functions */
/*!
 * @brief   map sptr_map(type = conf) to sptr_setting
 * @param   sptr_pctldev: pinctrl_dev
 * @param   name: pin name (struct fwk_pinctrl_desc::name)
 * @retval  error code
 * @note    none
 */
kint32_t fwk_pinconf_get_by_name(struct fwk_pinctrl_dev *sptr_pctldev, const kchar_t *name)
{
    struct fwk_pinctrl_desc *sptr_desc;
    const struct fwk_pinctrl_ops *sptr_pctlops;
    const kchar_t *pin_name;
    kuint32_t i, pin;

    sptr_desc = sptr_pctldev->sptr_desc;
    sptr_pctlops = sptr_desc->sptr_pctlops;

    for (i = 0; i < sptr_desc->npins; i++)
    {
        pin = sptr_desc->sptr_pins[i].number;
        pin_name = sptr_pctlops->get_pin_desc(sptr_pctldev, pin);

        if (!kstrcmp(name, pin_name))
            return pin;
    }

    return -ER_MORE;
}

/*!
 * @brief   map sptr_map(type = conf) to sptr_setting
 * @param   sptr_map: current map (source data)
 * @param   sptr_setting: destination data
 * @retval  error code
 * @note    none
 */
kint32_t fwk_pinconf_map_to_setting(struct fwk_pinctrl_map const *sptr_map, struct fwk_pinctrl_setting *sptr_setting)
{
    struct fwk_pinctrl_dev *sptr_pctldev;
    kint32_t pin;

    if (sptr_setting->type != NR_FWK_PINCTRL_PIN_CONF)
        return -ER_FAULT;

    sptr_pctldev = sptr_setting->sptr_pctldev;
    pin = fwk_pinconf_get_by_name(sptr_pctldev, sptr_map->ugtr_data.sgtc_configs.group_or_pin);
    if (pin < 0)
        return pin;

    sptr_setting->ugtr_data.sgtc_configs.group_or_pin = pin;
    sptr_setting->ugtr_data.sgtc_configs.configs = sptr_map->ugtr_data.sgtc_configs.configs;
    sptr_setting->ugtr_data.sgtc_configs.num_configs = sptr_map->ugtr_data.sgtc_configs.num_configs;

    return ER_NORMAL;
}

/*!< end of file */
