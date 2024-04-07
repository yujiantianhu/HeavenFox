/*
 * Interrupt Interface Defines
 *
 * File Name:   hal_irq_desc.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.03.23
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/of/hal_of.h>
#include <platform/irq/hal_irq_domain.h>
#include <platform/irq/hal_irq_types.h>

/*!< The defines */
#define HAL_IRQ_DESC_MAX					(1024)

struct hal_irq_descs
{
	ksint32_t irq;
	srt_hal_irq_desc_t *sprt_desc;
	struct list_head sgrt_link;
};

/*!< The globals */
static kuint32_t g_hal_allocated_irqs[mrt_num_align(HAL_IRQ_DESC_MAX, RET_BITS_PER_INT) / RET_BITS_PER_INT];
static DECLARE_LIST_HEAD(sgrt_hal_irq_descs_list);

/*!< API function */
static srt_hal_irq_desc_t *hal_allocate_irq_desc(ert_kmem_pool_t gfp)
{
	srt_hal_irq_desc_t *sprt_desc;
	kusize_t size;

	size = mrt_num_align4(sizeof(srt_hal_irq_desc_t));

	sprt_desc = (srt_hal_irq_desc_t *)kzalloc(size + sizeof(struct hal_irq_descs), gfp);
	if (!mrt_isValid(sprt_desc))
	{
		return mrt_nullptr;
	}

	sprt_desc->descs_priv = ((kuint8_t *)sprt_desc) + size;
	return sprt_desc;
}

static void hal_insert_irq_descs(srt_hal_irq_desc_t *sprt_desc, kuint32_t virq)
{
	struct hal_irq_descs *sprt_table;

	if (!mrt_isValid(sprt_desc))
	{
		return;
	}

	sprt_table = (struct hal_irq_descs *)sprt_desc->descs_priv;
	sprt_table->irq = virq;
	sprt_table->sprt_desc = sprt_desc;
	init_list_head(&sprt_table->sgrt_link);

	sprt_desc->irq = virq;
	init_list_head(&sprt_desc->sgrt_action);

	list_head_add_tail(&sgrt_hal_irq_descs_list, &sprt_table->sgrt_link);
}

static void hal_remove_irq_descs(srt_hal_irq_desc_t *sprt_desc)
{
	struct hal_irq_descs *sprt_table;

	if (!mrt_isValid(sprt_desc))
	{
		return;
	}

	sprt_table = (struct hal_irq_descs *)sprt_desc->descs_priv;
	list_head_del(&sprt_table->sgrt_link);
}

static ksint32_t hal_irq_bitmap_find_areas(kuint32_t *bitmap, ksint32_t irq_base, kuint32_t total_bits, kuint32_t nr_irqs)
{
	kuint32_t index = 0, temp = 0;

	index = (irq_base >= 0) ? irq_base : 0;

	do 
	{
		if ((index + nr_irqs) >= total_bits)
		{
			return -Ert_isArrayOver;
		}

		/* 获取第一个非1的bit */
		index = bitmap_find_first_zero_bit(bitmap, index, total_bits);
		if (index < 0)
		{
			return -Ert_isNotFound;
		}

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

static ksint32_t irq_domain_alloc_irq_data(srt_hal_irq_domain_t *sprt_domain, kuint32_t virq, kuint32_t hwirq, kuint32_t nr_irqs)
{
	srt_hal_irq_data_t *sprt_data;

	for (kuint32_t i = 0; i < nr_irqs; i++)
	{
		sprt_data = hal_irq_get_data(virq + i);

		if (!mrt_isValid(sprt_data))
		{
			return -Ert_isNotFound;
		}

		sprt_data->sprt_domain = sprt_domain;
		sprt_data->irq = virq + i;
		sprt_data->hwirq = hwirq;
		sprt_data->sprt_chip = mrt_nullptr;
		sprt_domain->revmap[hwirq] = virq + i;

	}

	return virq;
}

/*!
 * @brief   Get the irq value
 * @param   none
 * @retval  none
 * @note    none
 */
static ksint32_t hal_irq_domain_alloc_descs(ksint32_t irq_base, kuint32_t nr_irqs)
{
	srt_hal_irq_desc_t *sprt_desc;
	kuint32_t virq;

	virq = hal_irq_bitmap_find_areas(g_hal_allocated_irqs, irq_base, HAL_IRQ_DESC_MAX, nr_irqs);
	if (virq < 0)
	{
		return virq;
	}

	/* create desc for every irq */
	for (kuint32_t i = 0; i < nr_irqs; i++)
	{
		sprt_desc = hal_allocate_irq_desc(GFP_KERNEL);
		if (!mrt_isValid(sprt_desc))
		{
			return -Ert_isMemErr;
		}

		hal_insert_irq_descs(sprt_desc, virq + i);
		bitmap_set_nr_bit_valid(g_hal_allocated_irqs, virq + i, HAL_IRQ_DESC_MAX, nr_irqs);
	}

	return virq;
}

/*!< ----------------------------------------------------------- */
srt_hal_irq_desc_t *hal_irq_to_desc(kuint32_t virq)
{
	struct hal_irq_descs *sprt_table;

	foreach_list_next_entry(sprt_table, &sgrt_hal_irq_descs_list, sgrt_link)
	{
		if (sprt_table->irq == virq)
		{
			return sprt_table->sprt_desc;
		}
	}

	return mrt_nullptr;
}

srt_hal_irq_data_t *hal_irq_get_data(kuint32_t virq)
{
	srt_hal_irq_desc_t *sprt_desc = hal_irq_to_desc(virq);
	return (sprt_desc) ? &sprt_desc->sgrt_data : mrt_nullptr;
}

srt_hal_irq_data_t *hal_irq_domain_get_data(srt_hal_irq_domain_t *sprt_domain, kuint32_t hwirq)
{
	ksint32_t virq;
	srt_hal_irq_data_t *sprt_data;

	if (!mrt_isValid(sprt_domain))
	{
		goto fail;
	}

	virq = sprt_domain->revmap[hwirq];
	if (virq < 0)
	{
		goto fail;
	}

	sprt_data = hal_irq_get_data(virq);
	if (mrt_isValid(sprt_data))
	{
		return ((sprt_data->hwirq == hwirq) && (sprt_data->sprt_domain == sprt_domain)) ? sprt_data : mrt_nullptr;
	}

fail:
	return mrt_nullptr;
}

ksint32_t hal_irq_domain_find_map(srt_hal_irq_domain_t *sprt_domain, kuint32_t hwirq, kuint32_t type)
{
	srt_hal_irq_desc_t *sprt_desc;
	srt_hal_irq_data_t *sprt_data;

	sprt_data = hal_irq_domain_get_data(sprt_domain, hwirq);
	if (!mrt_isValid(sprt_data))
	{
		return -Ert_isUnvalid;
	}

	sprt_desc = mrt_container_of(sprt_data, typeof(*sprt_desc), sgrt_data);

	type &= IRQ_TYPE_SENSE_MASK;
	if (sprt_desc->flags & type)
	{
		sprt_desc->flags |= type;
	}

	return sprt_data->irq;
}

void hal_irq_set_type(kuint32_t virq, kuint32_t type)
{
	srt_hal_irq_desc_t *sprt_desc = hal_irq_to_desc(virq);

	if (mrt_isValid(sprt_desc))
	{
		sprt_desc->flags |= type;
	}
}

ksint32_t hal_irq_domain_alloc_irqs(srt_hal_irq_domain_t *sprt_domain, ksint32_t irq_base, kuint32_t hwirq, kuint32_t nr_irqs)
{
	ksint32_t virq;

	virq = hal_irq_domain_alloc_descs(irq_base, nr_irqs);
	if (virq < 0)
	{
		return virq;
	}

	return irq_domain_alloc_irq_data(sprt_domain, virq, hwirq, nr_irqs);
}

ksint32_t hal_of_irq_get(struct hal_device_node *sprt_node, kuint32_t index)
{
	return hal_irq_of_parse_and_map(sprt_node, index);
}

/* end of file */
