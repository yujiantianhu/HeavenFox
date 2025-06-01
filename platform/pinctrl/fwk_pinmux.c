/*
 * Hardware Abstraction Layer Pinctrl Interface
 *
 * File Name:   fwk_pinmux.c
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

/*!< API function */
/*!
 * @brief   map sptr_map(type = mux) to sptr_setting
 * @param   sptr_pctldev: pinctrl_dev
 * @param   name: function name (it may be the name of "iomuxc" device_node)
 * @retval  error code
 * @note    none
 */
static kint32_t fwk_pinmux_get_function_by_name(struct fwk_pinctrl_dev *sptr_pctldev, const kchar_t *name)
{
    const struct fwk_pinmux_ops *sptr_pmxops;
    const kchar_t *func_name;
    kuint32_t i, num_funcs;

    sptr_pmxops = sptr_pctldev->sptr_desc->sptr_pmxops;
    if (!sptr_pmxops->get_functions_count ||
        !sptr_pmxops->get_function_name)
        return -ER_NODEV;

    num_funcs = sptr_pmxops->get_functions_count(sptr_pctldev);

    for (i = 0; i < num_funcs; i++)
    {
        func_name = sptr_pmxops->get_function_name(sptr_pctldev, i);
        if (!func_name)
            continue;

        if (!kstrcmp(name, func_name))
            return i;
    }

    return -ER_MORE;
}

/*!
 * @brief   map sptr_map(type = mux) to sptr_setting
 * @param   sptr_pctldev: pinctrl_dev
 * @param   func_selector: function index
 * @param   name: group name (pinctrl-0/1/2, ..., phandle ---> device node)
 * @retval  error code
 * @note    none
 */
static kint32_t fwk_pinmux_get_group_by_name(struct fwk_pinctrl_dev *sptr_pctldev, kuint32_t func_selector, const kchar_t *name)
{
    const struct fwk_pinctrl_ops *sptr_pctlops;
    const kchar_t *grp_name;
    kuint32_t i, num_grps;

    sptr_pctlops = sptr_pctldev->sptr_desc->sptr_pctlops;
    if (!sptr_pctlops->get_groups_count ||
        !sptr_pctlops->get_group_name)
        return -ER_NODEV;

    /*!< how many groups of this functions */
    num_grps = sptr_pctlops->get_groups_count(sptr_pctldev, func_selector);

    for (i = 0; i < num_grps; i++)
    {
        grp_name = sptr_pctlops->get_group_name(sptr_pctldev, func_selector, i);
        if (!grp_name)
            continue;

        if (!kstrcmp(name, grp_name))
            return i;
    }

    return -ER_MORE;
}

/*!
 * @brief   map sptr_map(type = mux) to sptr_setting
 * @param   sptr_map: current map (source data)
 * @param   sptr_setting: destination data
 * @retval  error code
 * @note    none
 */
kint32_t fwk_pinmux_map_to_setting(struct fwk_pinctrl_map const *sptr_map, struct fwk_pinctrl_setting *sptr_setting)
{
    struct fwk_pinctrl_dev *sptr_pctldev;
    kint32_t func_selector, group_selector;

    if (sptr_setting->type != NR_FWK_PINCTRL_PIN_MUX)
        return -ER_FAULT;

    sptr_pctldev = sptr_setting->sptr_pctldev;

    /*!< get function index */
    func_selector = fwk_pinmux_get_function_by_name(sptr_pctldev, sptr_map->ugtr_data.sgtc_mux.function);
    if (func_selector < 0)
        return func_selector;

    /*!< get group index */
    group_selector = fwk_pinmux_get_group_by_name(sptr_pctldev, func_selector, sptr_map->ugtr_data.sgtc_mux.group);
    if (group_selector < 0)
        return group_selector;

    sptr_setting->ugtr_data.sgtc_mux.func = func_selector;
    sptr_setting->ugtr_data.sgtc_mux.group = group_selector;

    return ER_NORMAL;
}

/*!< end of file */
