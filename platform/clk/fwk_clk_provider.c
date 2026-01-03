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
#include <kernel/rw_lock.h>

/*!< The defines */
typedef struct fwk_of_clk_provider
{
	struct list_head sgtc_link;

	struct fwk_device_node *sptr_node;
	struct fwk_clk *(*get)(struct fwk_of_phandle_args *sptr_args, void *data);
	void *data;

} srt_fwk_of_clk_provider_t;

/*!< The globals */
static DECLARE_LIST_HEAD(sgtc_fwk_clk_providers);
static struct rw_lock sgtc_clk_providers_lock;

/*!< API function */
/*!
 * @brief   get sptr_clk from data
 * @param   sptr_args, data
 * @retval  sptr_clk
 * @note    none
 */
struct fwk_clk *fwk_of_clk_src_onecell_get(struct fwk_of_phandle_args *sptr_args, void *data)
{
    struct fwk_clk_one_cell *sptr_cell;
    kuint32_t index;

    if (!sptr_args || !data)
        return mr_nullptr;

    sptr_cell = (struct fwk_clk_one_cell *)data;
    index = sptr_args->args[0];

    return sptr_cell->sptr_clks ? &sptr_cell->sptr_clks[index] : mr_nullptr;
}

/*!
 * @brief   add a new clk provider (data)
 * @param   sptr_node, data
 * @retval  errno
 * @note    none
 */
kint32_t fwk_clk_add_provider(struct fwk_device_node *sptr_node, 
                    struct fwk_clk *(*get)(struct fwk_of_phandle_args *, void *), void *data)
{
    struct fwk_of_clk_provider *sptr_provider;

    if (!sptr_node || !get)
        return -ER_NODEV;

    sptr_provider = kzalloc(sizeof(*sptr_provider), GFP_KERNEL);
    if (!isValid(sptr_provider))
        return -ER_NOMEM;

    sptr_provider->sptr_node = sptr_node;
    sptr_provider->get = get;
    sptr_provider->data = data;

    wr_lock(&sgtc_clk_providers_lock);
    list_head_add_tail(&sgtc_fwk_clk_providers, &sptr_provider->sgtc_link);
    wr_unlock(&sgtc_clk_providers_lock);

    return ER_NORMAL;
}

/*!
 * @brief   remove and destroy clk a provider
 * @param   sptr_node
 * @retval  none
 * @note    none
 */
void fwk_clk_del_provider(struct fwk_device_node *sptr_node)
{
    struct fwk_of_clk_provider *sptr_provider;

    if (!sptr_node)
        return;

    wr_lock(&sgtc_clk_providers_lock);

    foreach_list_next_entry(sptr_provider, &sgtc_fwk_clk_providers, sgtc_link)
    {
        if (sptr_provider->sptr_node == sptr_node)
        {
            list_head_del(&sptr_provider->sgtc_link);
            kfree(sptr_provider);
        }
    }

    wr_unlock(&sgtc_clk_providers_lock);
}

/*!
 * @brief   look up a provider
 * @param   sptr_args
 * @retval  sptr_clk
 * @note    none
 */
struct fwk_clk *fwk_clk_provider_look_up(struct fwk_of_phandle_args *sptr_args)
{
    struct fwk_of_clk_provider *sptr_provider;

    if (!sptr_args)
        return mr_nullptr;

    rd_lock(&sgtc_clk_providers_lock);

    foreach_list_next_entry(sptr_provider, &sgtc_fwk_clk_providers, sgtc_link)
    {
        if (sptr_provider->sptr_node == sptr_args->sptr_node)
        {
            rd_unlock(&sgtc_clk_providers_lock);
            return sptr_provider->get(sptr_args, sptr_provider->data);
        }
    }

    rd_unlock(&sgtc_clk_providers_lock);
    return mr_nullptr;
}

/*!< --------------------------------------------------------------------------- */
/*!
 * @brief   clk_provider init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __plat_init fwk_clk_provider_global_init(void)
{
    rw_lock_init(&sgtc_clk_providers_lock);
    return ER_NORMAL;
}

/*!
 * @brief   clk_provider exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __plat_exit fwk_clk_provider_global_exit(void)
{

}

IMPORT_LATE_INIT(fwk_clk_provider_global_init);
IMPORT_LATE_EXIT(fwk_clk_provider_global_exit);

/*!< end of file */
