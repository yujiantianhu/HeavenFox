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

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/generic.h>
#include <common/list_types.h>
#include <platform/of/fwk_of.h>

/*!< The defines */
typedef struct fwk_irq_domain
{
    kint32_t hwirq;
    struct fwk_device_node *sptr_node;
    kuint32_t flags;
    const kchar_t *name;

    struct list_head sgtc_link;
    const struct fwk_irq_domain_ops *sptr_ops;
    void *host_data;

    struct fwk_irq_domain *sptr_parent;

    kuint32_t hwirq_max;
    kint32_t revmap[];

} srt_fwk_irq_domain_t;

typedef struct fwk_irq_domain_ops 
{
    kint32_t (*xlate)(struct fwk_irq_domain *sptr_domain, struct fwk_device_node *sptr_intc,
                const kuint32_t *intspec, kuint32_t intsize, kuint32_t *out_hwirq, kuint32_t *out_type);
    kint32_t (*alloc)(struct fwk_irq_domain *sptr_domain, kuint32_t virq, kuint32_t nr_irqs, void *arg);
    void (*free)(struct fwk_irq_domain *sptr_domain, kuint32_t virq, kuint32_t nr_irqs);
                
} srt_fwk_irq_domain_ops_t;

/*!< The functions */
extern struct fwk_irq_domain *fwk_of_irq_host(struct fwk_device_node *sptr_node);
extern kint32_t fwk_of_irq_parse_one(struct fwk_device_node *sptr_node, kuint32_t index, struct fwk_of_phandle_args *sptr_irq);
extern kint32_t fwk_irq_find_mapping(struct fwk_irq_domain *sptr_domain, kuint32_t type, kint32_t hwirq);
extern kint32_t fwk_irq_create_of_mapping(struct fwk_of_phandle_args *sptr_irq);
extern kint32_t fwk_irq_of_parse_and_map(struct fwk_device_node *sptr_node, kuint32_t index);
extern struct fwk_irq_domain *fwk_irq_domain_add_linear(struct fwk_device_node *sptr_node, kuint32_t size,
                     const struct fwk_irq_domain_ops *sptr_ops, void *host_data);
extern struct fwk_irq_domain *fwk_irq_domain_add_hierarchy(struct fwk_irq_domain *sptr_parent, struct fwk_device_node *sptr_node, 
                                                kuint32_t size, const struct fwk_irq_domain_ops *sptr_ops, void *host_data);
extern void fwk_irq_domain_del_hierarchy(struct fwk_irq_domain *sptr_domain);
extern struct fwk_irq_domain *fwk_irq_get_domain_by_name(kchar_t *name, kint32_t hwirq);
extern kint32_t fwk_irq_get_by_domain(struct fwk_irq_domain *sptr_domain, kint32_t hwirq);
extern kint32_t fwk_irq_get_by_domain_name(kchar_t *name, kint32_t hwirq);

#ifdef __cplusplus
    }
#endif

#endif /* __FWK_IRQ_DOMAIN_H */
