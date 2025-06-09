/*
 * Interrupt Interface Defines
 *
 * File Name:   fwk_irq_desc.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.03.23
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/of/fwk_of.h>
#include <platform/irq/fwk_irq_domain.h>
#include <platform/irq/fwk_irq_types.h>
#include <platform/irq/fwk_irq_chip.h>
#include <kernel/spinlock.h>

/*!< The defines */
#define FWK_IRQ_DESC_RADIXTREE				(1)
#define FWK_IRQ_DESC_MAX					(1024)

/*!< The globals */
static DECLARE_RADIX_TREE(sgtc_fwk_irq_radix_tree, default_malloc, kfree);
static kuint32_t g_fwk_allocated_irqs[mr_num_align(FWK_IRQ_DESC_MAX, RET_BITS_PER_INT) / RET_BITS_PER_INT] = { 0 };
static struct spin_lock sgtc_fwk_irqs_lock = SPIN_LOCK_INIT();

/*!< API functions */
/*!
 * @brief   allocate irq_desc
 * @param   gfp
 * @retval  none
 * @note    none
 */
static struct fwk_irq_desc *fwk_allocate_irq_desc(nrt_gfp_t gfp)
{
	struct fwk_irq_desc *sptr_desc;

	sptr_desc = (struct fwk_irq_desc *)kzalloc(sizeof(*sptr_desc), gfp);
	if (!isValid(sptr_desc))
		return mr_nullptr;

	init_list_head(&sptr_desc->sgtc_action);
	spin_lock_init(&sptr_desc->sgtc_lock);
		
	return sptr_desc;
}

/*!
 * @brief   find irq number that is not used from bitmap
 * @param   bitmap, irq_base, total_bits, nr_irqs
 * @retval  irq number base
 * @note    none
 */
static kint32_t fwk_irq_bitmap_find_areas(kuint32_t *bitmap, kint32_t irq_base, kuint32_t total_bits, kuint32_t nr_irqs)
{
	kuint32_t index = 0, temp = 0;

	index = (irq_base >= 0) ? irq_base : 0;

	do 
	{
		if ((index + nr_irqs) >= total_bits)
			return -ER_MORE;

		/*!< get the first non-zero bit  */
		index = bitmap_find_first_zero_bit(bitmap, index, total_bits);
		if (index < 0)
			return -ER_NOTFOUND;

		/*!< base on index, get nr_irqs consecutive non-zero bits; if success, return 0; otherwise, return index that causing failed */
		temp = bitmap_find_nr_zero_bit(bitmap, index, total_bits, nr_irqs);
		if (temp)
		{
			index = temp + 1;
			continue;
		}

	} while (temp);

	return index;
}

/*!
 * @brief   allocate and initialize irq_data
 * @param   sptr_domain, virq, hwirq, nr_irqs
 * @retval  virtual irq number or errno
 * @note    none
 */
static kint32_t irq_domain_alloc_irq_data(struct fwk_irq_domain *sptr_domain, kuint32_t virq, kuint32_t hwirq, kuint32_t nr_irqs)
{
	struct fwk_irq_data *sptr_data;

	for (kuint32_t i = 0; i < nr_irqs; i++)
	{
		sptr_data = fwk_irq_get_data(virq + i);

		if (!isValid(sptr_data))
			return -ER_NOTFOUND;

		sptr_data->sptr_domain = sptr_domain;
		sptr_data->irq = virq + i;
		sptr_data->hwirq = hwirq + i;
		sptr_data->sptr_chip = &sgtc_fwk_irq_dummy_chip;
		sptr_data->mask = 0;
		sptr_domain->revmap[hwirq + i] = virq + i;
	}

	return virq;
}

/*!
 * @brief   Get the irq value
 * @param   none
 * @retval  none
 * @note    none
 */
static kint32_t fwk_irq_domain_alloc_descs(kint32_t irq_base, kuint32_t nr_irqs)
{
	struct fwk_irq_desc *sptr_desc;
	kuint32_t virq;

	spin_lock_irqsave(&sgtc_fwk_irqs_lock);

	virq = fwk_irq_bitmap_find_areas(g_fwk_allocated_irqs, irq_base, FWK_IRQ_DESC_MAX, nr_irqs);
	if (virq < 0)
	{
		spin_unlock_irqrestore(&sgtc_fwk_irqs_lock);
		return virq;
	}

	/*!< create desc for every irq */
	for (kuint32_t i = 0; i < nr_irqs; i++)
	{
		sptr_desc = fwk_allocate_irq_desc(GFP_KERNEL);
		if (!isValid(sptr_desc))
		{
			spin_unlock_irqrestore(&sgtc_fwk_irqs_lock);
			return -ER_NOMEM;
		}

		sptr_desc->irq = virq + i;
		radix_tree_add(&sgtc_fwk_irq_radix_tree, virq + i, &sptr_desc->sgtc_radix);
	}

	bitmap_set_nr_bit_valid(g_fwk_allocated_irqs, virq, FWK_IRQ_DESC_MAX, nr_irqs);
	spin_unlock_irqrestore(&sgtc_fwk_irqs_lock);

	return virq;
}

/*!< ----------------------------------------------------------- */
/*!
 * @brief   irq ---> irq_desc
 * @param   irq
 * @retval  irq_desc
 * @note    none
 */
struct fwk_irq_desc *fwk_irq_to_desc(kuint32_t virq)
{
	struct fwk_irq_desc *sptr_desc;

	if (virq < 0)
		return mr_nullptr;
	
	spin_lock_irqsave(&sgtc_fwk_irqs_lock);
	sptr_desc = radix_tree_next_entry(&sgtc_fwk_irq_radix_tree, struct fwk_irq_desc, sgtc_radix, virq);
	spin_unlock_irqrestore(&sgtc_fwk_irqs_lock);

	return sptr_desc;
}

/*!
 * @brief   irq_data ---> irq_desc
 * @param   irq_data
 * @retval  irq_desc
 * @note    none
 */
struct fwk_irq_desc *fwk_irq_data_to_desc(struct fwk_irq_data *sptr_data)
{
	return sptr_data ? mr_container_of(sptr_data, struct fwk_irq_desc, sgtc_data) : mr_nullptr;
}

/*!
 * @brief   irq ---> irq_data
 * @param   irq
 * @retval  irq_data
 * @note    none
 */
struct fwk_irq_data *fwk_irq_get_data(kuint32_t virq)
{
	struct fwk_irq_desc *sptr_desc = fwk_irq_to_desc(virq);
	return (sptr_desc) ? &sptr_desc->sgtc_data : mr_nullptr;
}

/*!
 * @brief   get irq_data by hard irq
 * @param   sptr_domain, hwirq
 * @retval  irq_data
 * @note    none
 */
struct fwk_irq_data *fwk_irq_domain_get_data(struct fwk_irq_domain *sptr_domain, kuint32_t hwirq)
{
	kint32_t virq;
	struct fwk_irq_data *sptr_data;

	if (!isValid(sptr_domain))
		goto fail;

	virq = sptr_domain->revmap[hwirq];
	if (virq < 0)
		goto fail;

	sptr_data = fwk_irq_get_data(virq);
	if (isValid(sptr_data))
		return ((sptr_data->hwirq == hwirq) && (sptr_data->sptr_domain == sptr_domain)) ? sptr_data : mr_nullptr;

fail:
	return mr_nullptr;
}

/*!
 * @brief   get irq_data by hard irq
 * @param   sptr_domain, hwirq
 * @retval  irq_data
 * @note    none
 */
kint32_t fwk_irq_domain_find_map(struct fwk_irq_domain *sptr_domain, kuint32_t hwirq, kuint32_t type)
{
	struct fwk_irq_desc *sptr_desc;
	struct fwk_irq_data *sptr_data;

	sptr_data = fwk_irq_domain_get_data(sptr_domain, hwirq);
	if (!isValid(sptr_data))
		return -ER_INVALID;

	if (!type)
		goto END;

	sptr_desc = fwk_irq_data_to_desc(sptr_data);
	type &= IRQ_TYPE_SENSE_MASK;
	
	if (!(sptr_desc->flags & type))
		sptr_desc->flags |= type;

END:
	return sptr_data->irq;
}

/*!
 * @brief   set irq type
 * @param   virq, type
 * @retval  none
 * @note    none
 */
void fwk_irq_desc_set_type(kuint32_t virq, kuint32_t type)
{
	struct fwk_irq_desc *sptr_desc = fwk_irq_to_desc(virq);

	if (!isValid(sptr_desc))
		return;

	sptr_desc->flags |= type;
}

/*!
 * @brief   allocate multiple irq numbers
 * @param   sptr_domain, irq_base, hwirq, nr_irqs
 * @retval  irq base allocated
 * @note    none
 */
kint32_t fwk_irq_domain_alloc_irqs(struct fwk_irq_domain *sptr_domain, kint32_t irq_base, kuint32_t hwirq, kuint32_t nr_irqs)
{
	kint32_t virq;

	virq = fwk_irq_domain_alloc_descs(irq_base, nr_irqs);
	if (virq < 0)
		return virq;
	
	return irq_domain_alloc_irq_data(sptr_domain, virq, hwirq, nr_irqs);
}

/*!
 * @brief   release irq
 * @param   irq
 * @retval  none
 * @note    none
 */
void fwk_irq_desc_free(kint32_t irq)
{
	struct fwk_irq_desc *sptr_desc;
	struct fwk_irq_data *sptr_data;

	sptr_desc = fwk_irq_to_desc(irq);
	sptr_data = &sptr_desc->sgtc_data;

	spin_lock_irqsave(&sgtc_fwk_irqs_lock);

	bitmap_set_nr_bit_zero(g_fwk_allocated_irqs, sptr_data->irq, FWK_IRQ_DESC_MAX, 1);
	fwk_destroy_irq_action(sptr_data->irq);
	radix_tree_del(&sgtc_fwk_irq_radix_tree, sptr_data->irq);

	spin_unlock_irqrestore(&sgtc_fwk_irqs_lock);
	kfree(sptr_desc);
}

/*!
 * @brief   release multiple irq numbers
 * @param   sptr_domain
 * @retval  none
 * @note    none
 */
void fwk_irq_domain_free_irqs(struct fwk_irq_domain *sptr_domain)
{
	kuint32_t idx;

	for (idx = 0; idx < sptr_domain->hwirq_max; idx++)
	{
		if (sptr_domain->revmap[idx] < 0)
			continue;

		fwk_irq_desc_free(sptr_domain->revmap[idx]);
		sptr_domain->revmap[idx] = -1;
	}
}

/*!
 * @brief   parse and map irq
 * @param   sptr_node, index
 * @retval  irq number (hwirq ---> virtual irq)
 * @note    none
 */
kint32_t fwk_of_irq_get(struct fwk_device_node *sptr_node, kuint32_t index)
{
	return fwk_irq_of_parse_and_map(sptr_node, index);
}


/* end of file */
