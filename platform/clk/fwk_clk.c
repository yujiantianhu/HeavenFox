/*
 * Hardware Abstraction Layer Clock Interface
 *
 * File Name:   fwk_clk.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.03
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/of/fwk_of.h>
#include <platform/clk/fwk_clk.h>
#include <platform/clk/fwk_clk_provider.h>
#include <kernel/mutex.h>

/*!< API function */
/*!
 * @brief   initial and record sptr_clk
 * @param   sptr_clk, sptr_hw
 * @retval  sptr_clk
 * @note    none
 */
struct fwk_clk *fwk_clk_config(struct fwk_clk *sptr_clk, struct fwk_clk_hw *sptr_hw, const kchar_t *dev_id, const kchar_t *con_id)
{
    if (!sptr_hw || !sptr_clk)
        return mr_nullptr;
    
    sptr_clk->dev_id = dev_id;
    sptr_clk->con_id = con_id;
    sptr_clk->max_rate = ~(kutype_t)0;
    sptr_clk->sptr_core = sptr_hw->sptr_core;
    sptr_clk->flags &= ~NR_FWK_CLK_IS_DYNAMIC;

    mutex_lock(&sptr_hw->sptr_core->sgtc_mutex);
    list_head_add_tail(&sptr_hw->sptr_core->sgtc_clks, &sptr_clk->sgtc_link);
    mutex_unlock(&sptr_hw->sptr_core->sgtc_mutex);

    return sptr_clk;
}

/*!
 * @brief   create and initialize sptr_clk
 * @param   sptr_hw, dev_id, con_id
 * @retval  sptr_clk
 * @note    none
 */
struct fwk_clk *fwk_create_clk(struct fwk_clk_hw *sptr_hw, const kchar_t *dev_id, const kchar_t *con_id)
{
    struct fwk_clk *sptr_clk;

    if (!sptr_hw)
        return mr_nullptr;

    sptr_clk = kzalloc(sizeof(*sptr_clk), GFP_KERNEL);
    if (!isValid(sptr_clk))
        return mr_nullptr;

    if (fwk_clk_config(sptr_clk, sptr_hw, dev_id, con_id))
        sptr_clk->flags |= NR_FWK_CLK_IS_DYNAMIC;

    return sptr_clk;
}

/*!
 * @brief   initialize sptr_clk
 * @param   sptr_dev, sptr_clk
 * @retval  errno
 * @note    none
 */
kint32_t fwk_init_clk(struct fwk_device *sptr_dev, struct fwk_clk *sptr_clk)
{
    return ER_NORMAL;
}

/*!
 * @brief   destroy sptr_clk
 * @param   sptr_clk
 * @retval  none
 * @note    none
 */
void fwk_free_clk(struct fwk_clk *sptr_clk)
{
    if (!sptr_clk)
        return;

    list_head_del(&sptr_clk->sgtc_link);

    if (sptr_clk->flags & NR_FWK_CLK_IS_DYNAMIC)
        kfree(sptr_clk);
}

/*!
 * @brief   register sptr_clk
 * @param   sptr_dev, sptr_hw
 * @retval  sptr_clk
 * @note    none
 */
struct fwk_clk *fwk_clk_register(struct fwk_device *sptr_dev, struct fwk_clk_hw *sptr_hw)
{
    struct fwk_clk_core *sptr_core;
    const struct fwk_clk_init_data *sptr_init;
    kuint32_t i;

    sptr_init = sptr_hw->sptr_init;
    
    sptr_core = kzalloc(sizeof(*sptr_core), GFP_KERNEL);
    if (!isValid(sptr_core))
        return mr_nullptr;

    sptr_core->sptr_ops = sptr_init->sptr_ops;
    sptr_core->name = sptr_init->name;
    sptr_core->sptr_hw = sptr_hw;
    sptr_hw->sptr_core = sptr_core;
    init_list_head(&sptr_core->sgtc_clks);
    mutex_init(&sptr_core->sgtc_mutex);

    sptr_core->parent_names = kzalloc(sptr_init->num_parents * sizeof(kchar_t *), GFP_KERNEL);
    if (!isValid(sptr_core->parent_names))
        goto fail1;

    for (i = 0; i < sptr_init->num_parents; i++)
    {
        sptr_core->parent_names[i] = sptr_init->parent_names[i];

        ERR_OUT(!sptr_core->parent_names[i], "parent names is empty!\r\n");
        if (!sptr_core->parent_names[i])
            goto fail2;
    }

    if (!sptr_hw->sptr_clk)
    {
        sptr_hw->sptr_clk = fwk_create_clk(sptr_hw, mr_nullptr, mr_nullptr);
        if (!isValid(sptr_hw->sptr_clk))
            goto fail2;
    }
    else
        fwk_clk_config(sptr_hw->sptr_clk, sptr_hw, mr_nullptr, mr_nullptr);

    if (fwk_init_clk(sptr_dev, sptr_hw->sptr_clk))
        goto fail3;

    return sptr_hw->sptr_clk;

fail3:
    fwk_free_clk(sptr_hw->sptr_clk);
    sptr_hw->sptr_clk = mr_nullptr;
fail2:
    kfree(sptr_core->parent_names);
fail1:
    kfree(sptr_core);

    return mr_nullptr;
}

/*!
 * @brief   unregister sptr_clk
 * @param   sptr_clk
 * @retval  none
 * @note    none
 */
void fwk_clk_unregister(struct fwk_clk *sptr_clk)
{
    struct fwk_clk_hw *sptr_hw;
    struct fwk_clk *sptr_child, *sptr_temp;

    sptr_hw = fwk_clk_to_hw(sptr_clk);
    mutex_lock(&sptr_hw->sptr_core->sgtc_mutex);

    foreach_list_next_entry_safe(sptr_child, sptr_temp, &sptr_hw->sptr_core->sgtc_clks, sgtc_link)
        fwk_free_clk(sptr_child);

    mutex_unlock(&sptr_hw->sptr_core->sgtc_mutex);

    if (sptr_hw->sptr_core->parent_names)
        kfree(sptr_hw->sptr_core->parent_names);

    kfree(sptr_hw->sptr_core);
    sptr_hw->sptr_core = mr_nullptr;
}

/*!
 * @brief   parse clk from device_node
 * @param   sptr_dev, name
 * @retval  sptr_clk
 * @note    none
 */
struct fwk_clk *fwk_clk_get(struct fwk_device *sptr_dev, const kchar_t *name)
{
    struct fwk_clk *sptr_clk;
    struct fwk_device_node *sptr_node;
    struct fwk_of_phandle_args sgtc_args;
    kint32_t index = 0;

    sptr_node = sptr_dev->sptr_node;

    if (name)
    {
        index = fwk_of_property_match_string(sptr_node, "clock-names", name);
        if (index < 0)
            return mr_nullptr;
    }

    if (fwk_of_parse_phandle_with_args(sptr_node, "clocks", "#clock-cells", 0, index, &sgtc_args))
        return mr_nullptr;

    sptr_clk = fwk_clk_provider_look_up(&sgtc_args);
    if (!isValid(sptr_clk))
        return mr_nullptr;

    return fwk_create_clk(fwk_clk_to_hw(sptr_clk), mr_dev_get_name(sptr_dev), name);
}

/*!
 * @brief   release clk
 * @param   sptr_clk
 * @retval  none
 * @note    none
 */
void fwk_clk_put(struct fwk_clk *sptr_clk)
{
    fwk_free_clk(sptr_clk);
}

/*!
 * @brief   enable clk
 * @param   sptr_clk
 * @retval  none
 * @note    none
 */
void fwk_clk_enable(struct fwk_clk *sptr_clk)
{
    if (!sptr_clk || !sptr_clk->sptr_core)
        return;

    /*!< Enable Clock Only Once */
    if (!(sptr_clk->sptr_core->enable_count++)) 
    {
        const struct fwk_clk_ops *sptr_ops;

        sptr_ops = sptr_clk->sptr_core->sptr_ops;

        if (sptr_ops && sptr_ops->enable)
            sptr_ops->enable(fwk_clk_to_hw(sptr_clk));
    }
}

/*!
 * @brief   disable clk
 * @param   sptr_clk
 * @retval  none
 * @note    none
 */
void fwk_clk_disable(struct fwk_clk *sptr_clk)
{
    if (!sptr_clk || !sptr_clk->sptr_core)
        return;

    /*!< Clock was closed, do not excute again */
    if (!sptr_clk->sptr_core->enable_count)
        return;

    if (!(--sptr_clk->sptr_core->enable_count)) 
    {
        const struct fwk_clk_ops *sptr_ops;

        sptr_ops = sptr_clk->sptr_core->sptr_ops;

        if (sptr_ops && sptr_ops->disable)
            sptr_ops->disable(fwk_clk_to_hw(sptr_clk));
    }
}

/*!
 * @brief   prepare clk
 * @param   sptr_clk
 * @retval  none
 * @note    none
 */
void fwk_clk_prepare(struct fwk_clk *sptr_clk)
{
    if (!sptr_clk || !sptr_clk->sptr_core)
        return;

    /*!< Prepare Clock Only Once */
    if (!(sptr_clk->sptr_core->prepare_count++)) 
    {
        const struct fwk_clk_ops *sptr_ops;

        sptr_ops = sptr_clk->sptr_core->sptr_ops;

        if (sptr_ops && sptr_ops->prepare)
            sptr_ops->prepare(fwk_clk_to_hw(sptr_clk));
    }
}

/*!
 * @brief   unprepare clk
 * @param   sptr_clk
 * @retval  none
 * @note    none
 */
void fwk_clk_unprepare(struct fwk_clk *sptr_clk)
{
    if (!sptr_clk || !sptr_clk->sptr_core)
        return;

    /*!< Clock was released, do not excute again */
    if (!sptr_clk->sptr_core->prepare_count)
        return;

    if (!(--sptr_clk->sptr_core->prepare_count)) 
    {
        const struct fwk_clk_ops *sptr_ops;

        sptr_ops = sptr_clk->sptr_core->sptr_ops;

        if (sptr_ops && sptr_ops->unprepare)
            sptr_ops->unprepare(fwk_clk_to_hw(sptr_clk));
    }
}

/*!
 * @brief   check if clk is enabled
 * @param   sptr_clk
 * @retval  enable/disable
 * @note    none
 */
kbool_t fwk_clk_is_enabled(struct fwk_clk *sptr_clk)
{
    const struct fwk_clk_ops *sptr_ops;

    sptr_ops = sptr_clk->sptr_core->sptr_ops;

    if (sptr_ops && sptr_ops->is_enabled)
        return !sptr_ops->is_enabled(fwk_clk_to_hw(sptr_clk));

    return false;
}

/*!
 * @brief   enable clk
 * @param   sptr_clk
 * @retval  none
 * @note    none
 */
void fwk_clk_prepare_enable(struct fwk_clk *sptr_clk)
{
    fwk_clk_prepare(sptr_clk);
    fwk_clk_enable(sptr_clk);
}

/*!
 * @brief   disable clk
 * @param   sptr_clk
 * @retval  none
 * @note    none
 */
void fwk_clk_disable_unprepare(struct fwk_clk *sptr_clk)
{
    fwk_clk_disable(sptr_clk);
    fwk_clk_unprepare(sptr_clk);
}

/*!< end of file */
