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
#include <platform/of/fwk_of.h>
#include <platform/irq/fwk_irq_domain.h>
#include <platform/irq/fwk_irq_types.h>

/*!< The defines */
#define FWK_IRQ_DESC_MAX					(1024)

typedef struct fwk_irq_descs
{
	ksint32_t irq;
	srt_fwk_irq_desc_t *sprt_desc;
	struct list_head sgrt_link;

} srt_fwk_irq_descs_t;

/*!< The globals */
static kuint32_t g_fwk_allocated_irqs[mrt_num_align(FWK_IRQ_DESC_MAX, RET_BITS_PER_INT) / RET_BITS_PER_INT];
static DECLARE_LIST_HEAD(sgrt_fwk_irq_descs_list);

static srt_fwk_irq_desc_t *fwk_irq_desc_search(srt_fwk_irq_desc_t *sprt_desc)
{
	const srt_fwk_irq_descs_t *sprt_descs = sprt_desc ? sprt_desc->descs_priv : mrt_nullptr;

	if (!sprt_descs)
	{
		sprt_descs = mrt_list_first_entry(&sgrt_fwk_irq_descs_list, srt_fwk_irq_descs_t, sgrt_link);
		goto END;
	}

	sprt_descs = mrt_list_next_entry(sprt_descs, sgrt_link);
	if (&sprt_descs->sgrt_link == &sgrt_fwk_irq_descs_list)
		return mrt_nullptr;

END:
	return sprt_descs->sprt_desc;
}

#define foreach_fwk_irq_desc_safe(sprt_desc, sprt_temp)		\
	for (sprt_desc = fwk_irq_desc_search(mrt_nullptr),	\
		 sprt_temp = sprt_desc ? fwk_irq_desc_search(sprt_desc) : mrt_nullptr; 	\
		 isValid(sprt_desc);	\
		 sprt_desc = sprt_temp,	\
		 sprt_temp = sprt_desc ? fwk_irq_desc_search(sprt_desc) : mrt_nullptr)

/*!< API function */
static srt_fwk_irq_desc_t *fwk_allocate_irq_desc(ert_fwk_mempool_t gfp)
{
	srt_fwk_irq_desc_t *sprt_desc;
	kusize_t size;

	size = mrt_num_align4(sizeof(srt_fwk_irq_desc_t));

	sprt_desc = (srt_fwk_irq_desc_t *)kzalloc(size + sizeof(struct fwk_irq_descs), gfp);
	if (!isValid(sprt_desc))
		return mrt_nullptr;

	sprt_desc->descs_priv = ((kuint8_t *)sprt_desc) + size;
	return sprt_desc;
}

static void fwk_insert_irq_descs(srt_fwk_irq_desc_t *sprt_desc, kuint32_t virq)
{
	struct fwk_irq_descs *sprt_table;

	if (!isValid(sprt_desc))
		return;

	sprt_table = (struct fwk_irq_descs *)sprt_desc->descs_priv;
	sprt_table->irq = virq;
	sprt_table->sprt_desc = sprt_desc;

	sprt_desc->irq = virq;
	init_list_head(&sprt_desc->sgrt_action);

	list_head_add_tail(&sgrt_fwk_irq_descs_list, &sprt_table->sgrt_link);
}

static void fwk_remove_irq_descs(srt_fwk_irq_desc_t *sprt_desc)
{
	struct fwk_irq_descs *sprt_table;

	if (!isValid(sprt_desc))
		return;

	sprt_table = (struct fwk_irq_descs *)sprt_desc->descs_priv;
	list_head_del(&sprt_table->sgrt_link);
}

static ksint32_t fwk_irq_bitmap_find_areas(kuint32_t *bitmap, ksint32_t irq_base, kuint32_t total_bits, kuint32_t nr_irqs)
{
	kuint32_t index = 0, temp = 0;

	index = (irq_base >= 0) ? irq_base : 0;

	do 
	{
		if ((index + nr_irqs) >= total_bits)
			return -NR_IS_MORE;

		/* 获取第一个非1的bit */
		index = bitmap_find_first_zero_bit(bitmap, index, total_bits);
		if (index < 0)
			return -NR_IS_NOTFOUND;

		/* 以index为起始, 获取连续nr_irqs个非1的bit, 成功则返回0; 失败返回错误的index */
		temp = bitmap_find_nr_zero_bit(bitmap, index, total_bits, nr_irqs);
		if (temp)
		{
			index = temp + 1;
			continue;
		}

	} while (temp);

	return index;
}

static ksint32_t irq_domain_alloc_irq_data(srt_fwk_irq_domain_t *sprt_domain, kuint32_t virq, kuint32_t hwirq, kuint32_t nr_irqs)
{
	srt_fwk_irq_data_t *sprt_data;

	for (kuint32_t i = 0; i < nr_irqs; i++)
	{
		sprt_data = fwk_irq_get_data(virq + i);

		if (!isValid(sprt_data))
			return -NR_IS_NOTFOUND;

		sprt_data->sprt_domain = sprt_domain;
		sprt_data->irq = virq + i;
		sprt_data->hwirq = hwirq + i;
		sprt_data->sprt_chip = mrt_nullptr;
		sprt_domain->revmap[hwirq + i] = virq + i;

	}

	return virq;
}

/*!
 * @brief   Get the irq value
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t fwk_irq_domain_alloc_descs(ksint32_t irq_base, kuint32_t nr_irqs)
{
	srt_fwk_irq_desc_t *sprt_desc;
	kuint32_t virq;

	virq = fwk_irq_bitmap_find_areas(g_fwk_allocated_irqs, irq_base, FWK_IRQ_DESC_MAX, nr_irqs);
	if (virq < 0)
		return virq;

	/* create desc for every irq */
	for (kuint32_t i = 0; i < nr_irqs; i++)
	{
		sprt_desc = fwk_allocate_irq_desc(GFP_KERNEL);
		if (!isValid(sprt_desc))
			return -NR_IS_NOMEM;

		fwk_insert_irq_descs(sprt_desc, virq + i);
	}

	bitmap_set_nr_bit_valid(g_fwk_allocated_irqs, virq, FWK_IRQ_DESC_MAX, nr_irqs);

	return virq;
}

/*!< ----------------------------------------------------------- */
srt_fwk_irq_desc_t *fwk_irq_to_desc(kuint32_t virq)
{
	struct fwk_irq_descs *sprt_table;

	foreach_list_next_entry(sprt_table, &sgrt_fwk_irq_descs_list, sgrt_link)
	{
		if (sprt_table->irq == virq)
			return sprt_table->sprt_desc;
	}

	return mrt_nullptr;
}

srt_fwk_irq_data_t *fwk_irq_get_data(kuint32_t virq)
{
	srt_fwk_irq_desc_t *sprt_desc = fwk_irq_to_desc(virq);
	return (sprt_desc) ? &sprt_desc->sgrt_data : mrt_nullptr;
}

srt_fwk_irq_data_t *fwk_irq_domain_get_data(srt_fwk_irq_domain_t *sprt_domain, kuint32_t hwirq)
{
	ksint32_t virq;
	srt_fwk_irq_data_t *sprt_data;

	if (!isValid(sprt_domain))
		goto fail;

	virq = sprt_domain->revmap[hwirq];
	if (virq < 0)
		goto fail;

	sprt_data = fwk_irq_get_data(virq);
	if (isValid(sprt_data))
		return ((sprt_data->hwirq == hwirq) && (sprt_data->sprt_domain == sprt_domain)) ? sprt_data : mrt_nullptr;

fail:
	return mrt_nullptr;
}

ksint32_t fwk_irq_domain_find_map(srt_fwk_irq_domain_t *sprt_domain, kuint32_t hwirq, kuint32_t type)
{
	srt_fwk_irq_desc_t *sprt_desc;
	srt_fwk_irq_data_t *sprt_data;

	sprt_data = fwk_irq_domain_get_data(sprt_domain, hwirq);
	if (!isValid(sprt_data))
		return -NR_IS_UNVALID;

	if (!type)
		goto END;

	sprt_desc = mrt_container_of(sprt_data, typeof(*sprt_desc), sgrt_data);
	type &= IRQ_TYPE_SENSE_MASK;
	
	if (sprt_desc->flags & type)
		sprt_desc->flags |= type;

END:
	return sprt_data->irq;
}

void fwk_irq_set_type(kuint32_t virq, kuint32_t type)
{
	srt_fwk_irq_desc_t *sprt_desc = fwk_irq_to_desc(virq);

	if (isValid(sprt_desc))
		sprt_desc->flags |= type;
}

ksint32_t fwk_irq_domain_alloc_irqs(srt_fwk_irq_domain_t *sprt_domain, ksint32_t irq_base, kuint32_t hwirq, kuint32_t nr_irqs)
{
	ksint32_t virq;

	virq = fwk_irq_domain_alloc_descs(irq_base, nr_irqs);
	if (virq < 0)
		return virq;
	
	return irq_domain_alloc_irq_data(sprt_domain, virq, hwirq, nr_irqs);
}

void fwk_irq_domain_free_irqs(srt_fwk_irq_domain_t *sprt_domain, kbool_t option)
{
	srt_fwk_irq_desc_t *sprt_desc, *sprt_temp;
	srt_fwk_irq_data_t *sprt_data;

	foreach_fwk_irq_desc_safe(sprt_desc, sprt_temp)
	{
		sprt_data = &sprt_desc->sgrt_data;
		if (sprt_data->sprt_domain == sprt_domain)
		{
			if (!option)
			{
				sprt_data->sprt_domain = mrt_nullptr;
				continue;
			}

			fwk_remove_irq_descs(sprt_desc);
			bitmap_set_nr_bit_zero(g_fwk_allocated_irqs, sprt_data->irq, FWK_IRQ_DESC_MAX, 1);
			fwk_destroy_irq_desc(sprt_data->irq);

			kfree(sprt_desc);

			for (kuint32_t i; i < sprt_domain->hwirq_max; i++)
				sprt_domain->revmap[i] = -1;
		}
	}
}

ksint32_t fwk_of_irq_get(struct fwk_device_node *sprt_node, kuint32_t index)
{
	return fwk_irq_of_parse_and_map(sprt_node, index);
}

/* end of file */
