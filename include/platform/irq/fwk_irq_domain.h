/*
 * Interrupt Interface Type Defines
 *
 * File Name:   fwk_irq_domain.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.01.07
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_IRQ_DOMAIN_H
#define __FWK_IRQ_DOMAIN_H

/*!< The includes */
#include <common/generic.h>
#include <common/list_types.h>
#include <platform/of/fwk_of.h>

#ifdef __cplusplus
	TARGET_EXT "C" {
#endif

/*!< The defines */
typedef struct fwk_irq_domain
{
	ksint32_t hwirq;
	kuint32_t group;
	struct fwk_device_node *sprt_node;
	kuint32_t flags;
	const kstring_t *name;

	struct list_head sgrt_link;
	const struct fwk_irq_domain_ops *sprt_ops;
	void *host_data;

	struct fwk_irq_domain *sprt_parent;

	kuint32_t hwirq_max;
	ksint32_t revmap[];

} srt_fwk_irq_domain_t;

typedef struct fwk_irq_domain_ops 
{
	ksint32_t (*match)(struct fwk_irq_domain *sprt_domain, struct fwk_device_node *sprt_intc);
	ksint32_t (*map)(struct fwk_irq_domain *sprt_domain, kuint32_t virq, kuint32_t hw);
	void (*unmap)(struct fwk_irq_domain *sprt_domain, kuint32_t virq);

	ksint32_t (*xlate)(struct fwk_irq_domain *sprt_domain, struct fwk_device_node *sprt_intc,
				const kuint32_t *intspec, kuint32_t intsize, kuint32_t *out_hwirq, kuint32_t *out_type);
	ksint32_t (*alloc)(struct fwk_irq_domain *d, kuint32_t virq, kuint32_t nr_irqs, void *arg);
	void (*free)(struct fwk_irq_domain *d, kuint32_t virq, kuint32_t nr_irqs);
				
} srt_fwk_irq_domain_ops_t;

/*!< The functions */
TARGET_EXT srt_fwk_irq_domain_t *fwk_of_irq_host(struct fwk_device_node *sprt_node);
TARGET_EXT ksint32_t fwk_of_irq_parse_one(struct fwk_device_node *sprt_node, kuint32_t index, struct fwk_of_phandle_args *sprt_irq);
TARGET_EXT ksint32_t fwk_irq_find_mapping(srt_fwk_irq_domain_t *sprt_domain, kuint32_t type, ksint32_t hwirq);
TARGET_EXT ksint32_t fwk_irq_create_of_mapping(struct fwk_of_phandle_args *sprt_irq);
TARGET_EXT ksint32_t fwk_irq_of_parse_and_map(struct fwk_device_node *sprt_node, kuint32_t index);
TARGET_EXT srt_fwk_irq_domain_t *fwk_irq_domain_add_linear(struct fwk_device_node *of_node, kuint32_t size,
					 const struct fwk_irq_domain_ops *ops, void *host_data);
TARGET_EXT srt_fwk_irq_domain_t *fwk_irq_domain_add_hierarchy(srt_fwk_irq_domain_t *sprt_parent, struct fwk_device_node *sprt_node, 
												kuint32_t size, const srt_fwk_irq_domain_ops_t *ops, void *host_data);
TARGET_EXT srt_fwk_irq_domain_t *fwk_irq_get_domain_by_name(kstring_t *name, ksint32_t hwirq);
TARGET_EXT ksint32_t fwk_irq_get_by_domain(kstring_t *name, ksint32_t hwirq);

#ifdef __cplusplus
	}
#endif

#endif /* __FWK_IRQ_DOMAIN_H */
