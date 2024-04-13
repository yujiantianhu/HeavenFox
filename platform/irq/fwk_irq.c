/*
 * Interrupt Interface Defines
 *
 * File Name:   irq_domain.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.01.07
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <configs/configs.h>
#include <platform/fwk_basic.h>
#include <platform/irq/fwk_irq_domain.h>
#include <platform/irq/fwk_irq.h>
#include <platform/irq/fwk_irq_types.h>
#include <platform/of/fwk_of.h>

/*!< The defines */
typedef ksint32_t (*func_fwk_irq_init_cb_t) (struct fwk_device_node *, struct fwk_device_node *);

/*!< The globals */

/*!< The functions */

/*!< API function */
/*!
 * @brief   initial IRQ
 * @param   none
 * @retval  none
 * @note    none
 */
void __weak initIRQ(void)
{

}

/*!
 * @brief   initial IRQ by DTS
 * @param   none
 * @retval  none
 * @note    none
 */
void fwk_of_irq_init(const struct fwk_of_device_id *sprt_matches)
{
    struct fwk_irq_intcs_desc
    {
        struct list_head sgrt_link;
        struct fwk_device_node *sprt_np;
        struct fwk_device_node *sprt_parent;    
    };

    struct fwk_device_node *sprt_np;
    struct fwk_of_property *sprt_prop;
    struct fwk_irq_intcs_desc *sprt_desc, *sprt_desc_temp;
    struct fwk_of_device_id *sprt_match;
    struct fwk_device_node *parent = mrt_nullptr;
    kbool_t of_status;
    ksint32_t retval;

    DECLARE_LIST_HEAD(sgrt_intc_desc_list);
    DECLARE_LIST_HEAD(sgrt_intc_parent_list);

    /*!< for each node */
    FOREACH_OF_DT_NODE(sprt_np, mrt_nullptr)
    {
        sprt_prop = fwk_of_find_property(sprt_np, "interrupt-controller", mrt_nullptr);
        of_status = fwk_of_device_is_avaliable(sprt_np);

        if ((!mrt_isValid(sprt_prop)) || (!mrt_isValid(of_status)))
        {
            continue;
        }

        /*!< if is not matched with "sprt_matches" */
        if (!fwk_of_node_try_matches(sprt_np, sprt_matches, mrt_nullptr))
        {
            continue;
        }

        /*!< matched sucessfully */
        sprt_desc = (struct fwk_irq_intcs_desc *)kzalloc(sizeof(struct fwk_irq_intcs_desc), GFP_KERNEL);
        if (!mrt_isValid(sprt_desc))
        {
            goto fail;
        }

        sprt_desc->sprt_np = sprt_np;

        /*!< 对于intc, 它的irq parent为null; 对于gpc, 它的parent为intc */
        sprt_desc->sprt_parent = fwk_of_irq_parent(sprt_np);
        if (sprt_np == sprt_desc->sprt_parent)
        {
            sprt_desc->sprt_parent = mrt_nullptr;
        }

        list_head_add_tail(&sgrt_intc_desc_list, &sprt_desc->sgrt_link);
    }

    while (!mrt_list_head_empty(&sgrt_intc_desc_list))
    {
        foreach_list_next_entry_safe(sprt_desc, sprt_desc_temp, &sgrt_intc_desc_list, sgrt_link)
        {
            sprt_match = fwk_of_match_node(sprt_matches, sprt_desc->sprt_np);

            /* if equal, skip it; this way, the list will not be empty for the time being */
            if (parent != sprt_desc->sprt_parent)
            {
                continue;
            }

            list_head_del(&sprt_desc->sgrt_link);

            if ((!mrt_isValid(sprt_match)) || (!mrt_isValid(sprt_match->data)))
            {
                kfree(sprt_desc);
                continue;
            }

            /*!< do intc initial */
            retval = ((func_fwk_irq_init_cb_t)sprt_match->data)(sprt_desc->sprt_np, sprt_desc->sprt_parent);
            if (mrt_isErr(retval))
            {
                print_err("Initial IRQ Controller: %s failed!\n", sprt_match->compatible);
                kfree(sprt_desc);
                continue;
            }

            list_head_add_tail(&sgrt_intc_parent_list, &sprt_desc->sgrt_link);
        }

        sprt_desc = mrt_list_first_valid_entry(&sgrt_intc_parent_list, typeof(*sprt_desc), sgrt_link);
        if (!mrt_isValid(sprt_desc))
        {
            print_err("List is already empty, can not get any desc\n");
            break;
        }

        parent = sprt_desc->sprt_np;

        list_head_del(&sprt_desc->sgrt_link);
        kfree(sprt_desc);
    }

    foreach_list_next_entry_safe(sprt_desc, sprt_desc_temp, &sgrt_intc_parent_list, sgrt_link)
    {
        list_head_del(&sprt_desc->sgrt_link);
        kfree(sprt_desc);
    }

fail:
    foreach_list_next_entry_safe(sprt_desc, sprt_desc_temp, &sgrt_intc_desc_list, sgrt_link)
    {
        list_head_del(&sprt_desc->sgrt_link);
        kfree(sprt_desc);
    }
}

/* end of file */
