/*
 * Interrupt Interface Defines
 *
 * File Name:   fwk_irq_domain.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.01.07
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/irq/fwk_irq_domain.h>
#include <platform/irq/fwk_irq_types.h>
#include <platform/of/fwk_of.h>

/*!< The globals */
static DECLARE_LIST_HEAD(sgrt_fwk_irq_domain);

/*!< API function */
srt_fwk_irq_domain_t *fwk_of_irq_host(struct fwk_device_node *sprt_node)
{
	srt_fwk_irq_domain_t *sprt_domain, *found = mrt_nullptr;

	foreach_list_next_entry(sprt_domain, &sgrt_fwk_irq_domain, sgrt_link)
	{
		if (sprt_node == sprt_domain->sprt_node)
		{
			found = sprt_domain;
			break;
		}
	}

	return found;
}

/*!
 * @brief   Get the irq value
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t fwk_of_irq_parse_one(struct fwk_device_node *sprt_node, kuint32_t index, struct fwk_of_phandle_args *sprt_irq)
{
	struct fwk_device_node *sprt_parent;
	kuint32_t i, cells;
	kuint32_t lenth, *ptr_value;
	ksint32_t retval;

	if ((!mrt_isValid(sprt_node)) || (!mrt_isValid(sprt_irq)))
	{
		return -NR_isNullPtr;
	}

	ptr_value = (kuint32_t *)fwk_of_get_property(sprt_node, "interrupts", &lenth);
	lenth /= sizeof(kuint32_t);

	/* how many value per group */
	cells = fwk_of_n_irq_cells(sprt_node);
	if (!mrt_isValid(cells) || (cells >= FWK_OF_MAX_PHANDLE_ARGS))
	{
		return -NR_isArgFault;
	}

	if (!mrt_isValid(ptr_value) || (((index + 1) * cells) > lenth))
	{
		return -NR_isArgFault;
	}

	for (i = 0; i < cells; i++)
	{
		retval = fwk_of_property_read_u32_index(sprt_node, "interrupts", (index * cells) + i, &sprt_irq->args[i]);
		if (mrt_isErr(retval))
		{
			return -NR_isArgFault;
		}
	}

	/* for intc, parent == null; but it should not to be translated */
	sprt_parent = fwk_of_irq_parent(sprt_node);
	if (!mrt_isValid(sprt_parent))
	{
		return -NR_isArgFault;
	}

	sprt_irq->args_count = i;
	sprt_irq->sprt_node = sprt_parent;

	return NR_isWell;
}

/*!
 * @brief   find irq number
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t fwk_irq_find_mapping(srt_fwk_irq_domain_t *sprt_domain, kuint32_t type, ksint32_t hwirq)
{
	return fwk_irq_domain_find_map(sprt_domain, hwirq, type);
}

/*!
 * @brief   Get the virtual-irq number (soft irq number)
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t fwk_irq_create_of_mapping(struct fwk_of_phandle_args *sprt_irq)
{
	srt_fwk_irq_domain_t *sprt_domain;
	ksint32_t virq;
	kuint32_t hwirq, type;
	ksint32_t retval;

	if (!mrt_isValid(sprt_irq) || !mrt_isValid(sprt_irq->sprt_node))
	{
		return -NR_isMemErr;
	}

	sprt_domain = fwk_of_irq_host(sprt_irq->sprt_node);
	if (!mrt_isValid(sprt_domain) || !mrt_isValid(sprt_domain->sprt_ops->xlate))
	{
		return -NR_isNotFound;
	}

	/* get hwirq/type form sprt_irq->args[] */
	retval = sprt_domain->sprt_ops->xlate(sprt_domain, sprt_irq->sprt_node, sprt_irq->args, sprt_irq->args_count, &hwirq, &type);
	if (mrt_isErr(retval))
	{
		return retval;
	}

	virq = fwk_irq_find_mapping(sprt_domain, type, hwirq);
	if (virq >= 0)
	{
		return virq;
	}

	/* map one virtual irq number */
	virq = fwk_irq_domain_alloc_irqs(sprt_domain, -1, hwirq, 1);
	if (virq < 0)
	{
		return virq;
	}

	fwk_irq_set_type(virq, type);

	return virq;
}

/*!
 * @brief   Get the irq value
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t fwk_irq_of_parse_and_map(struct fwk_device_node *sprt_node, kuint32_t index)
{
	struct fwk_of_phandle_args sgrt_old;
	ksint32_t retval;

	retval = fwk_of_irq_parse_one(sprt_node, index, &sgrt_old);
	if (mrt_isErr(retval))
	{
		return -NR_isNotSuccess;
	}

	return fwk_irq_create_of_mapping(&sgrt_old);
}

/*!
 * @brief   Add new IRQ domain
 * @param   none
 * @retval  none
 * @note    none
 */
srt_fwk_irq_domain_t *fwk_irq_domain_add_linear(struct fwk_device_node *sprt_node, kuint32_t size,
					 							const srt_fwk_irq_domain_ops_t *ops, void *host_data)
{
	srt_fwk_irq_domain_t *sprt_domain;

	/* 多出来的size, 以硬中断号为下标, 软中断号为值 */
	sprt_domain = (srt_fwk_irq_domain_t *)kzalloc(sizeof(srt_fwk_irq_domain_t) + sizeof(ksint32_t) * size, GFP_KERNEL);
	if (!mrt_isValid(sprt_domain))
	{
		return mrt_nullptr;
	}

	sprt_domain->sprt_ops = ops;
	sprt_domain->sprt_node = sprt_node;
	sprt_domain->host_data = host_data;
	sprt_domain->hwirq_max = size;

	for (kuint32_t i = 0; i < size; i++)
	{
		sprt_domain->revmap[i] = -1;
	}

	list_head_add_tail(&sgrt_fwk_irq_domain, &sprt_domain->sgrt_link);

	return sprt_domain;
}

/*!
 * @brief   Add new IRQ domain
 * @param   none
 * @retval  none
 * @note    none
 */
srt_fwk_irq_domain_t *fwk_irq_domain_add_hierarchy(srt_fwk_irq_domain_t *sprt_parent, struct fwk_device_node *sprt_node, 
												kuint32_t size, const srt_fwk_irq_domain_ops_t *ops, void *host_data)
{
	srt_fwk_irq_domain_t *sprt_domain;

	sprt_domain = fwk_irq_domain_add_linear(sprt_node, size, ops, host_data);
	if (mrt_isValid(sprt_domain))
	{
		sprt_domain->sprt_parent = sprt_parent;
		sprt_domain->name = sprt_node->name;
	}

	return sprt_domain;
}

srt_fwk_irq_domain_t *fwk_irq_get_domain_by_name(kstring_t *name, ksint32_t hwirq)
{
	srt_fwk_irq_domain_t *sprt_domain;
	ksint32_t retval;

	foreach_list_next_entry(sprt_domain, &sgrt_fwk_irq_domain, sgrt_link)
	{
		retval = mrt_isValid(sprt_domain->name) && (!strcmp(sprt_domain->name, name));
		if (retval && (hwirq < sprt_domain->hwirq_max))
		{
			return sprt_domain;
		}
	}

	return mrt_nullptr;
}

ksint32_t fwk_irq_get_by_domain(kstring_t *name, ksint32_t hwirq)
{
	srt_fwk_irq_domain_t *sprt_domain;

	sprt_domain = fwk_irq_get_domain_by_name(name, hwirq);
	return sprt_domain ? sprt_domain->revmap[hwirq] : -1;
}

/* end of file */
