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

	struct fwk_device_node *sprt_node;
	struct fwk_clk *(*get)(struct fwk_of_phandle_args *sprt_args, void *data);
	void *data;

} srt_fwk_of_clk_provider_t;

/*!< The globals */
static DECLARE_LIST_HEAD(sgrt_fwk_clk_providers);

/*!< API function */
struct fwk_clk *fwk_of_clk_src_onecell_get(struct fwk_of_phandle_args *sprt_args, void *data)
{
    struct fwk_clk_one_cell *sprt_cell;
    kuint32_t index;

    if (!sprt_args || !data)
        return mrt_nullptr;

    sprt_cell = (struct fwk_clk_one_cell *)data;
    index = sprt_args->args[0];

    return sprt_cell->sprt_clks ? &sprt_cell->sprt_clks[index] : mrt_nullptr;
}

kint32_t fwk_clk_add_provider(struct fwk_device_node *sprt_node, 
                    struct fwk_clk *(*get)(struct fwk_of_phandle_args *, void *), void *data)
{
    struct fwk_of_clk_provider *sprt_provider;

    if (!sprt_node || !get)
        return -ER_NODEV;

    sprt_provider = kzalloc(sizeof(*sprt_provider), GFP_KERNEL);
    if (!isValid(sprt_provider))
        return -ER_NOMEM;

    sprt_provider->sprt_node = sprt_node;
    sprt_provider->get = get;
    sprt_provider->data = data;

    list_head_add_tail(&sgrt_fwk_clk_providers, &sprt_provider->sgrt_link);

    return ER_NORMAL;
}

void fwk_clk_del_provider(struct fwk_device_node *sprt_node)
{
    struct fwk_of_clk_provider *sprt_provider;

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

struct fwk_clk *fwk_clk_provider_look_up(struct fwk_of_phandle_args *sprt_args)
{
    struct fwk_of_clk_provider *sprt_provider;

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
