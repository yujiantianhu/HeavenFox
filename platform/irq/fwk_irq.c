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
typedef kint32_t (*func_fwk_irq_init_cb_t) (struct fwk_device_node *, struct fwk_device_node *);

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
void fwk_of_irq_init(const struct fwk_of_device_id *sptr_matches)
{
    struct fwk_irq_intcs_desc
    {
        struct list_head sgtc_link;
        struct fwk_device_node *sptr_np;
        struct fwk_device_node *sptr_parent;    
    };

    struct fwk_device_node *sptr_np;
    struct fwk_of_property *sptr_prop;
    struct fwk_irq_intcs_desc *sptr_desc, *sptr_desc_temp;
    struct fwk_of_device_id *sptr_match;
    struct fwk_device_node *parent = mr_nullptr;
    kbool_t of_status;
    kint32_t retval;

    DECLARE_LIST_HEAD(sgtc_intc_desc_list);
    DECLARE_LIST_HEAD(sgtc_intc_parent_list);

    /*!< for each node */
    foreach_fwk_of_dt_node(sptr_np, mr_nullptr)
    {
        sptr_prop = fwk_of_find_property(sptr_np, "interrupt-controller", mr_nullptr);
        of_status = fwk_of_device_is_avaliable(sptr_np);

        if ((!isValid(sptr_prop)) || (!of_status))
            continue;

        /*!< if is not matched with "sptr_matches" */
        if (!fwk_of_node_try_matches(sptr_np, sptr_matches, mr_nullptr))
            continue;

        /*!< matched sucessfully */
        sptr_desc = (struct fwk_irq_intcs_desc *)kzalloc(sizeof(struct fwk_irq_intcs_desc), GFP_KERNEL);
        if (!isValid(sptr_desc))
            goto fail;

        sptr_desc->sptr_np = sptr_np;

        /*!< for intc, it's irq parent is null; for gpc, it's parent is intc */
        sptr_desc->sptr_parent = fwk_of_irq_parent(sptr_np);
        if (sptr_np == sptr_desc->sptr_parent)
            sptr_desc->sptr_parent = mr_nullptr;

        list_head_add_tail(&sgtc_intc_desc_list, &sptr_desc->sgtc_link);
    }

    while (!mr_list_head_empty(&sgtc_intc_desc_list))
    {
        foreach_list_next_entry_safe(sptr_desc, sptr_desc_temp, &sgtc_intc_desc_list, sgtc_link)
        {
            sptr_match = fwk_of_match_node(sptr_matches, sptr_desc->sptr_np);

            /* if equal, skip it; this way, the list will not be empty for the time being */
            if (parent != sptr_desc->sptr_parent)
                continue;

            list_head_del(&sptr_desc->sgtc_link);

            if ((!sptr_match) || (!sptr_match->data))
            {
                kfree(sptr_desc);
                continue;
            }

            /*!< do intc initial */
            retval = ((func_fwk_irq_init_cb_t)sptr_match->data)(sptr_desc->sptr_np, sptr_desc->sptr_parent);
            if (retval < 0)
            {
                print_err("Initial IRQ Controller: %s failed!\r\n", sptr_match->compatible);
                kfree(sptr_desc);
                continue;
            }

            list_head_add_tail(&sgtc_intc_parent_list, &sptr_desc->sgtc_link);
        }

        sptr_desc = mr_list_first_valid_entry(&sgtc_intc_parent_list, typeof(*sptr_desc), sgtc_link);
        if (!isValid(sptr_desc))
        {
            print_err("List is already empty, can not get any desc\r\n");
            break;
        }

        parent = sptr_desc->sptr_np;

        list_head_del(&sptr_desc->sgtc_link);
        kfree(sptr_desc);
    }

    foreach_list_next_entry_safe(sptr_desc, sptr_desc_temp, &sgtc_intc_parent_list, sgtc_link)
    {
        list_head_del(&sptr_desc->sgtc_link);
        kfree(sptr_desc);
    }

fail:
    foreach_list_next_entry_safe(sptr_desc, sptr_desc_temp, &sgtc_intc_desc_list, sgtc_link)
    {
        list_head_del(&sptr_desc->sgtc_link);
        kfree(sptr_desc);
    }
}

/* end of file */
