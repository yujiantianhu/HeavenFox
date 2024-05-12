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

/*!< API function */
srt_fwk_clk_t *fwk_clk_config(srt_fwk_clk_t *sprt_clk, srt_fwk_clk_hw_t *sprt_hw, const kstring_t *dev_id, const kstring_t *con_id)
{
    if (!sprt_hw || !sprt_clk)
        return mrt_nullptr;
    
    sprt_clk->dev_id = dev_id;
    sprt_clk->con_id = con_id;
    sprt_clk->max_rate = ~(kutype_t)0;
    sprt_clk->sprt_core = sprt_hw->sprt_core;
    sprt_clk->flags &= ~NR_FWK_CLK_IS_DYNAMIC;

    list_head_add_tail(&sprt_hw->sprt_core->sgrt_clks, &sprt_clk->sgrt_link);

    return sprt_clk;
}

srt_fwk_clk_t *fwk_create_clk(srt_fwk_clk_hw_t *sprt_hw, const kstring_t *dev_id, const kstring_t *con_id)
{
    srt_fwk_clk_t *sprt_clk;

    if (!sprt_hw)
        return mrt_nullptr;

    sprt_clk = kzalloc(sizeof(*sprt_clk), GFP_KERNEL);
    if (!isValid(sprt_clk))
        return mrt_nullptr;

    if (fwk_clk_config(sprt_clk, sprt_hw, dev_id, con_id))
        sprt_clk->flags |= NR_FWK_CLK_IS_DYNAMIC;

    return sprt_clk;
}

ksint32_t fwk_init_clk(srt_fwk_device_t *sprt_dev, srt_fwk_clk_t *sprt_clk)
{
    return NR_IS_NORMAL;
}

void fwk_free_clk(srt_fwk_clk_t *sprt_clk)
{
    if (!sprt_clk)
        return;

    list_head_del(&sprt_clk->sgrt_link);

    if (sprt_clk->flags & NR_FWK_CLK_IS_DYNAMIC)
        kfree(sprt_clk);
}

srt_fwk_clk_t *fwk_clk_register(srt_fwk_device_t *sprt_dev, srt_fwk_clk_hw_t *sprt_hw)
{
    srt_fwk_clk_core_t *sprt_core;
    const srt_fwk_clk_init_data_t *sprt_init;
    kuint32_t i;

    sprt_init = sprt_hw->sprt_init;
    
    sprt_core = kzalloc(sizeof(*sprt_core), GFP_KERNEL);
    if (!isValid(sprt_core))
        return mrt_nullptr;

    sprt_core->sprt_ops = sprt_init->sprt_ops;
    sprt_core->name = sprt_init->name;
    sprt_core->sprt_hw = sprt_hw;
    sprt_hw->sprt_core = sprt_core;
    init_list_head(&sprt_core->sgrt_clks);

    sprt_core->parent_names = kzalloc(sprt_init->num_parents * sizeof(kstring_t *), GFP_KERNEL);
    if (!isValid(sprt_core->parent_names))
        goto fail1;

    for (i = 0; i < sprt_init->num_parents; i++)
    {
        sprt_core->parent_names[i] = sprt_init->parent_names[i];

        ERR_OUT(!sprt_core->parent_names[i], 
                    "parent names is empty!\n");
        if (!sprt_core->parent_names[i])
            goto fail2;
    }

    if (!sprt_hw->sprt_clk)
    {
        sprt_hw->sprt_clk = fwk_create_clk(sprt_hw, mrt_nullptr, mrt_nullptr);
        if (!isValid(sprt_hw->sprt_clk))
            goto fail2;
    }
    else
        fwk_clk_config(sprt_hw->sprt_clk, sprt_hw, mrt_nullptr, mrt_nullptr);

    if (fwk_init_clk(sprt_dev, sprt_hw->sprt_clk))
        goto fail3;

    return sprt_hw->sprt_clk;

fail3:
    fwk_free_clk(sprt_hw->sprt_clk);
    sprt_hw->sprt_clk = mrt_nullptr;
fail2:
    kfree(sprt_core->parent_names);
fail1:
    kfree(sprt_core);

    return mrt_nullptr;
}

void fwk_clk_unregister(srt_fwk_clk_t *sprt_clk)
{
    srt_fwk_clk_hw_t *sprt_hw;
    srt_fwk_clk_t *sprt_child, *sprt_temp;

    sprt_hw = fwk_clk_to_hw(sprt_clk);

    foreach_list_next_entry_safe(sprt_child, sprt_temp, &sprt_hw->sprt_core->sgrt_clks, sgrt_link)
        fwk_free_clk(sprt_child);

    if (sprt_hw->sprt_core->parent_names)
        kfree(sprt_hw->sprt_core->parent_names);

    kfree(sprt_hw->sprt_core);
    sprt_hw->sprt_core = mrt_nullptr;
}

srt_fwk_clk_t *fwk_clk_get(srt_fwk_device_t *sprt_dev, const kstring_t *name)
{
    srt_fwk_clk_t *sprt_clk;
    srt_fwk_device_node_t *sprt_node;
    srt_fwk_of_phandle_args_t sgrt_args;
    ksint32_t index = 0;

    sprt_node = sprt_dev->sprt_node;

    if (name)
    {
        index = fwk_of_property_match_string(sprt_node, "clock-names", name);
        if (index < 0)
            return mrt_nullptr;
    }

    if (fwk_of_parse_phandle_with_args(sprt_node, "clocks", "#clock-cells", 0, index, &sgrt_args))
        return mrt_nullptr;

    sprt_clk = fwk_clk_provider_look_up(&sgrt_args);
    if (!isValid(sprt_clk))
        return mrt_nullptr;

    return fwk_create_clk(fwk_clk_to_hw(sprt_clk), mrt_get_dev_name(sprt_dev), name);
}

void fwk_clk_put(srt_fwk_clk_t *sprt_clk)
{
    fwk_free_clk(sprt_clk);
}

void fwk_clk_enable(srt_fwk_clk_t *sprt_clk)
{
    const srt_fwk_clk_ops_t *sprt_ops;

    sprt_ops = sprt_clk->sprt_core->sprt_ops;

    if (sprt_ops && sprt_ops->enable)
        sprt_ops->enable(fwk_clk_to_hw(sprt_clk));
}

void fwk_clk_disable(srt_fwk_clk_t *sprt_clk)
{
    const srt_fwk_clk_ops_t *sprt_ops;

    sprt_ops = sprt_clk->sprt_core->sprt_ops;

    if (sprt_ops && sprt_ops->disable)
        sprt_ops->disable(fwk_clk_to_hw(sprt_clk));
}

void fwk_clk_prepare(srt_fwk_clk_t *sprt_clk)
{
    const srt_fwk_clk_ops_t *sprt_ops;

    sprt_ops = sprt_clk->sprt_core->sprt_ops;

    if (sprt_ops && sprt_ops->prepare)
        sprt_ops->prepare(fwk_clk_to_hw(sprt_clk));
}

void fwk_clk_unprepare(srt_fwk_clk_t *sprt_clk)
{
    const srt_fwk_clk_ops_t *sprt_ops;

    sprt_ops = sprt_clk->sprt_core->sprt_ops;

    if (sprt_ops && sprt_ops->unprepare)
        sprt_ops->unprepare(fwk_clk_to_hw(sprt_clk));
}

kbool_t fwk_clk_is_enabled(srt_fwk_clk_t *sprt_clk)
{
    const srt_fwk_clk_ops_t *sprt_ops;

    sprt_ops = sprt_clk->sprt_core->sprt_ops;

    if (sprt_ops && sprt_ops->is_enabled)
        return !sprt_ops->is_enabled(fwk_clk_to_hw(sprt_clk));

    return false;
}

void fwk_clk_prepare_enable(srt_fwk_clk_t *sprt_clk)
{
    fwk_clk_prepare(sprt_clk);
    fwk_clk_enable(sprt_clk);
}

void fwk_clk_disable_unprepare(srt_fwk_clk_t *sprt_clk)
{
    fwk_clk_disable(sprt_clk);
    fwk_clk_unprepare(sprt_clk);
}

/*!< end of file */
