/*
 * Hardware Abstraction Layer Clock Interface
 *
 * File Name:   fwk_clk_gate.c
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

/*!< The defines */
typedef struct fwk_of_clk_provider 
{
	struct list_head sgrt_link;

	srt_fwk_device_node_t *sprt_node;
	srt_fwk_clk_t *(*get)(srt_fwk_of_phandle_args_t *sprt_args, void *data);
	void *data;

} srt_fwk_of_clk_provider_t;

/*!< The globals */
static DECLARE_LIST_HEAD(sgrt_fwk_clk_providers);

/*!< API function */
srt_fwk_clk_t *fwk_of_clk_src_onecell_get(srt_fwk_of_phandle_args_t *sprt_args, void *data)
{
    srt_fwk_clk_one_cell_t *sprt_cell;
    kuint32_t index;

    if (!sprt_args || !data)
        return mrt_nullptr;

    sprt_cell = (srt_fwk_clk_one_cell_t *)data;
    index = sprt_args->args[0];

    return sprt_cell->sprt_clks ? &sprt_cell->sprt_clks[index] : mrt_nullptr;
}

ksint32_t fwk_clk_add_provider(srt_fwk_device_node_t *sprt_node, 
                    srt_fwk_clk_t *(*get)(srt_fwk_of_phandle_args_t *, void *), void *data)
{
    srt_fwk_of_clk_provider_t *sprt_provider;

    if (!sprt_node || !get)
        return -NR_IS_NODEV;

    sprt_provider = kzalloc(sizeof(*sprt_provider), GFP_KERNEL);
    if (!isValid(sprt_provider))
        return -NR_IS_NOMEM;

    sprt_provider->sprt_node = sprt_node;
    sprt_provider->get = get;
    sprt_provider->data = data;

    list_head_add_tail(&sgrt_fwk_clk_providers, &sprt_provider->sgrt_link);

    return NR_IS_NORMAL;
}

void fwk_clk_del_provider(srt_fwk_device_node_t *sprt_node)
{
    srt_fwk_of_clk_provider_t *sprt_provider;

    if (!sprt_node)
        return;

    foreach_list_next_entry(sprt_provider, &sgrt_fwk_clk_providers, sgrt_link)
    {
        if (sprt_provider->sprt_node == sprt_node)
        {
            list_head_del(&sprt_provider->sgrt_link);
            kfree(sprt_provider);
        }
    }
}

srt_fwk_clk_t *fwk_clk_provider_look_up(srt_fwk_of_phandle_args_t *sprt_args)
{
    srt_fwk_of_clk_provider_t *sprt_provider;

    if (!sprt_args)
        return mrt_nullptr;

    foreach_list_next_entry(sprt_provider, &sgrt_fwk_clk_providers, sgrt_link)
    {
        if (sprt_provider->sprt_node == sprt_args->sprt_node)
            return sprt_provider->get(sprt_args, sprt_provider->data);
    }

    return mrt_nullptr;
}

/*!< end of file */