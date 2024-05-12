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
 * @brief   map sprt_map(type = conf) to sprt_setting
 * @param   sprt_pctldev: pinctrl_dev
 * @param   name: pin name (srt_fwk_pinctrl_desc_t::name)
 * @retval  error code
 * @note    none
 */
ksint32_t fwk_pinconf_get_by_name(srt_fwk_pinctrl_dev_t *sprt_pctldev, const kstring_t *name)
{
    srt_fwk_pinctrl_desc_t *sprt_desc;
    const srt_fwk_pinctrl_ops_t *sprt_pctlops;
    const kstring_t *pin_name;
    kuint32_t i, pin;

    sprt_desc = sprt_pctldev->sprt_desc;
    sprt_pctlops = sprt_desc->sprt_pctlops;

    for (i = 0; i < sprt_desc->npins; i++)
    {
        pin = sprt_desc->sprt_pins[i].number;
        pin_name = sprt_pctlops->get_pin_desc(sprt_pctldev, pin);

        if (!strcmp(name, pin_name))
            return pin;
    }

    return -NR_IS_MORE;
}

/*!
 * @brief   map sprt_map(type = conf) to sprt_setting
 * @param   sprt_map: current map (source data)
 * @param   sprt_setting: destination data
 * @retval  error code
 * @note    none
 */
ksint32_t fwk_pinconf_map_to_setting(srt_fwk_pinctrl_map_t const *sprt_map, srt_fwk_pinctrl_setting_t *sprt_setting)
{
    srt_fwk_pinctrl_dev_t *sprt_pctldev;
    ksint32_t pin;

    if (sprt_setting->type != NR_FWK_PINCTRL_PIN_CONF)
        return -NR_IS_FAULT;

    sprt_pctldev = sprt_setting->sprt_pctldev;
    pin = fwk_pinconf_get_by_name(sprt_pctldev, sprt_map->ugrt_data.sgrt_configs.group_or_pin);
    if (pin < 0)
        return pin;

    sprt_setting->ugrt_data.sgrt_configs.group_or_pin = pin;
    sprt_setting->ugrt_data.sgrt_configs.configs = sprt_map->ugrt_data.sgrt_configs.configs;
    sprt_setting->ugrt_data.sgrt_configs.num_configs = sprt_map->ugrt_data.sgrt_configs.num_configs;

    return NR_IS_NORMAL;
}

/*!< end of file */
