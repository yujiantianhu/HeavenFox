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
#include <kernel/rw_lock.h>

/*!< The globals */
static DECLARE_LIST_HEAD(sgtc_fwk_irq_domain);
static struct rw_lock sgtc_irq_domain_lock = RW_LOCK_INIT();

/*!< API function */
/*!
 * @brief   get irq domain
 * @param   sptr_node
 * @retval  sptr_domain
 * @note    none
 */
struct fwk_irq_domain *fwk_of_irq_host(struct fwk_device_node *sptr_node)
{
	struct fwk_irq_domain *sptr_domain, *found = mr_nullptr;

	rd_lock(&sgtc_irq_domain_lock);

	foreach_list_next_entry(sptr_domain, &sgtc_fwk_irq_domain, sgtc_link)
	{
		if (sptr_node == sptr_domain->sptr_node)
		{
			found = sptr_domain;
			break;
		}
	}

	rd_unlock(&sgtc_irq_domain_lock);
	return found;
}

/*!
 * @brief   Get the irq value
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_irq_parse_one(struct fwk_device_node *sptr_node, kuint32_t index, struct fwk_of_phandle_args *sptr_irq)
{
	struct fwk_device_node *sptr_parent;
	kuint32_t i, cells;
	kuint32_t lenth, *ptr_value;
	kint32_t retval;

	if ((!isValid(sptr_node)) || (!sptr_irq))
		return -ER_NULLPTR;

	ptr_value = (kuint32_t *)fwk_of_get_property(sptr_node, "interrupts", &lenth);
	lenth /= sizeof(kuint32_t);

	/* how many value per group */
	cells = fwk_of_n_irq_cells(sptr_node);
	if ((!cells) || (cells >= FWK_OF_MAX_PHANDLE_ARGS))
		return -ER_FAULT;

	if ((!ptr_value) || (((index + 1) * cells) > lenth))
		return -ER_FAULT;

	for (i = 0; i < cells; i++)
	{
		retval = fwk_of_property_read_u32_index(sptr_node, "interrupts", (index * cells) + i, &sptr_irq->args[i]);
		if (retval < 0)
			return -ER_FAULT;
	}

	/* for intc, parent == null; but it should not to be translated */
	sptr_parent = fwk_of_irq_parent(sptr_node);
	if (!isValid(sptr_parent))
		return -ER_FAULT;

	sptr_irq->args_count = i;
	sptr_irq->sptr_node = sptr_parent;

	return ER_NORMAL;
}

/*!
 * @brief   find irq number
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_irq_find_mapping(struct fwk_irq_domain *sptr_domain, kuint32_t type, kint32_t hwirq)
{
	return fwk_irq_domain_find_map(sptr_domain, hwirq, type);
}

/*!
 * @brief   Get the virtual-irq number (soft irq number)
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_irq_create_of_mapping(struct fwk_of_phandle_args *sptr_irq)
{
	struct fwk_irq_domain *sptr_domain;
	kint32_t virq;
	kuint32_t hwirq, type;
	kint32_t retval;

	if ((!sptr_irq) || !isValid(sptr_irq->sptr_node))
		return -ER_NOMEM;
	
	sptr_domain = fwk_of_irq_host(sptr_irq->sptr_node);
	if (!isValid(sptr_domain) || (!sptr_domain->sptr_ops->xlate))
		return -ER_NOTFOUND;

	/* get hwirq/type form sptr_irq->args[] */
	retval = sptr_domain->sptr_ops->xlate(sptr_domain, sptr_irq->sptr_node, 
										sptr_irq->args, sptr_irq->args_count, &hwirq, &type);
	if (retval < 0)
		return retval;

	virq = fwk_irq_find_mapping(sptr_domain, type, hwirq);
	if (virq >= 0)
		return virq;

	/* map one virtual irq number */
	virq = fwk_irq_domain_alloc_irqs(sptr_domain, -1, hwirq, 1);
	if (virq < 0)
		return virq;

	if (sptr_domain->sptr_ops->alloc)
		sptr_domain->sptr_ops->alloc(sptr_domain, virq, 1, mr_nullptr);
	
	fwk_irq_desc_set_type(virq, type);

	return virq;
}

/*!
 * @brief   Get the irq value
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_irq_of_parse_and_map(struct fwk_device_node *sptr_node, kuint32_t index)
{
	struct fwk_of_phandle_args sgtc_old;
	kint32_t retval;

	retval = fwk_of_irq_parse_one(sptr_node, index, &sgtc_old);
	if (retval < 0)
		return -ER_FAILD;

	return fwk_irq_create_of_mapping(&sgtc_old);
}

/*!
 * @brief   Add new IRQ domain
 * @param   none
 * @retval  none
 * @note    none
 */
struct fwk_irq_domain *fwk_irq_domain_add_linear(struct fwk_device_node *sptr_node, kuint32_t size,
					 							const struct fwk_irq_domain_ops *ops, void *host_data)
{
	struct fwk_irq_domain *sptr_domain;

	/*!< extra size, with hard interrupt number as index and soft interrupt number as value */
	sptr_domain = (struct fwk_irq_domain *)kzalloc(sizeof(struct fwk_irq_domain) + sizeof(kint32_t) * size, GFP_KERNEL);
	if (!isValid(sptr_domain))
		return mr_nullptr;

	sptr_domain->sptr_ops = ops;
	sptr_domain->sptr_node = sptr_node;
	sptr_domain->host_data = host_data;
	sptr_domain->hwirq_max = size;

	for (kuint32_t i = 0; i < size; i++)
		sptr_domain->revmap[i] = -1;

	wr_lock(&sgtc_irq_domain_lock);
	list_head_add_tail(&sgtc_fwk_irq_domain, &sptr_domain->sgtc_link);
	wr_unlock(&sgtc_irq_domain_lock);

	return sptr_domain;
}

/*!
 * @brief   Add new IRQ domain
 * @param   none
 * @retval  none
 * @note    none
 */
struct fwk_irq_domain *fwk_irq_domain_add_hierarchy(struct fwk_irq_domain *sptr_parent, struct fwk_device_node *sptr_node, 
												kuint32_t size, const struct fwk_irq_domain_ops *ops, void *host_data)
{
	struct fwk_irq_domain *sptr_domain;

	sptr_domain = fwk_irq_domain_add_linear(sptr_node, size, ops, host_data);
	if (isValid(sptr_domain))
	{
		sptr_domain->sptr_parent = sptr_parent;
		sptr_domain->name = sptr_node->name;
	}

	return sptr_domain;
}

/*!
 * @brief   delete sptr_domain
 * @param   sptr_domain
 * @retval  none
 * @note    none
 */
void fwk_irq_domain_del_hierarchy(struct fwk_irq_domain *sptr_domain)
{
	if (!sptr_domain)
		return;

	fwk_irq_domain_free_irqs(sptr_domain);

	wr_lock(&sgtc_irq_domain_lock);
	list_head_del(&sptr_domain->sgtc_link);
	wr_unlock(&sgtc_irq_domain_lock);
}

/*!
 * @brief   find sptr_domain by name
 * @param   name, hwirq
 * @retval  sptr_domain
 * @note    none
 */
struct fwk_irq_domain *fwk_irq_get_domain_by_name(kchar_t *name, kint32_t hwirq)
{
	struct fwk_irq_domain *sptr_domain;
	kint32_t retval;

	rd_lock(&sgtc_irq_domain_lock);

	foreach_list_next_entry(sptr_domain, &sgtc_fwk_irq_domain, sgtc_link)
	{
		retval = sptr_domain->name && (!strcmp(sptr_domain->name, name));
		if (retval && (hwirq < sptr_domain->hwirq_max))
		{
			rd_unlock(&sgtc_irq_domain_lock);
			return sptr_domain;
		}
	}

	rd_unlock(&sgtc_irq_domain_lock);
	return mr_nullptr;
}

/*!
 * @brief   get virtual irq by hwirq
 * @param   sptr_domain, hwirq
 * @retval  virtual irq number
 * @note    none
 */
kint32_t fwk_irq_get_by_domain(struct fwk_irq_domain *sptr_domain, kint32_t hwirq)
{
	return sptr_domain ? sptr_domain->revmap[hwirq] : -1;
}

/*!
 * @brief   get sptr_domain by name
 * @param   name, hwirq
 * @retval  sptr_domain
 * @note    none
 */
kint32_t fwk_irq_get_by_domain_name(kchar_t *name, kint32_t hwirq)
{
	struct fwk_irq_domain *sptr_domain;

	sptr_domain = fwk_irq_get_domain_by_name(name, hwirq);
	return fwk_irq_get_by_domain(sptr_domain, hwirq);
}

/* end of file */
