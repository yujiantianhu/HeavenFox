/*
 * Interrupt Interface Type Defines
 *
 * File Name:   fwk_irq_types.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.11.14
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_IRQ_TYPES_H
#define __FWK_IRQ_TYPES_H

/*!< The includes */
#include <common/generic.h>
#include <common/list_types.h>

#include "fwk_irq_domain.h"

#ifdef __cplusplus
TARGET_EXT "C" {
#endif

/*!< The defines */
#define	SWI_EVENT_SCHEDULED						0x00000001
#define	SWI_EVENT_SYSCALL						0x00000002

#define	_SWI_EVENT_SCHEDULED					" swi 0x00000001 "
#define	_SWI_EVENT_SYSCALL						" swi 0x00000002 "

#define mrt_rasie_schedule_event()	\
	do {	\
		__asm__ __volatile__ (	\
			_SWI_EVENT_SCHEDULED : : );	\
	} while (0)

#define FWK_IRQ_DESC_NAME_LENTH					(16)

typedef	ksint32_t irq_return_t;
typedef ksint32_t (*irq_handler_t)(void *ptrDev);

#define IRQ_TYPE_NONE							0x00000000
#define IRQ_TYPE_EDGE_RISING					0x00000001
#define IRQ_TYPE_EDGE_FALLING					0x00000002
#define IRQ_TYPE_EDGE_BOTH						(IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING)
#define IRQ_TYPE_LEVEL_HIGH						0x00000004
#define IRQ_TYPE_LEVEL_LOW						0x00000008
#define IRQ_TYPE_LEVEL_MASK						(IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH)
#define IRQ_TYPE_SENSE_MASK						(IRQ_TYPE_EDGE_BOTH | IRQ_TYPE_LEVEL_MASK)

typedef struct fwk_irq_data 
{
	unsigned int irq;
	unsigned long hwirq;
	unsigned int node;
	struct fwk_irq_chip *sprt_chip;
	struct fwk_irq_domain *sprt_domain;

	struct fwk_irq_data *sprt_parent;
	void *handler_data;
	void *chip_data;

} srt_fwk_irq_data_t;

typedef struct fwk_irq_chip 
{
	void (*irq_enable)(srt_fwk_irq_data_t *sprt_data);
	void (*irq_disable)(srt_fwk_irq_data_t *sprt_data);

} srt_fwk_irq_chip_t;

typedef struct fwk_irq_action
{
	kstring_t name[FWK_IRQ_DESC_NAME_LENTH];
	irq_handler_t handler;
	kuint32_t flags;
	void *ptrArgs;

	struct list_head sgrt_link;

} srt_fwk_irq_action_t;

typedef struct fwk_irq_desc
{
	ksint32_t irq;

	kuint32_t flags;
	ksint8_t irq_name[FWK_IRQ_DESC_NAME_LENTH];
	struct list_head sgrt_action;;

	struct list_head sgrt_link;
	
	struct fwk_irq_data sgrt_data;
	void *descs_priv;

} srt_fwk_irq_desc_t;

/*!< The functions */
TARGET_EXT srt_fwk_irq_desc_t *fwk_irq_to_desc(kuint32_t virq);
TARGET_EXT srt_fwk_irq_data_t *fwk_irq_get_data(kuint32_t virq);
TARGET_EXT srt_fwk_irq_data_t *fwk_irq_domain_get_data(srt_fwk_irq_domain_t *sprt_domain, kuint32_t hwirq);
TARGET_EXT void fwk_irq_set_type(kuint32_t virq, kuint32_t type);
TARGET_EXT ksint32_t fwk_irq_domain_find_map(srt_fwk_irq_domain_t *sprt_domain, kuint32_t hwirq, kuint32_t type);
TARGET_EXT ksint32_t fwk_irq_domain_alloc_irqs(srt_fwk_irq_domain_t *sprt_domain, ksint32_t irq_base, kuint32_t hwirq, kuint32_t nr_irqs);
TARGET_EXT void fwk_irq_domain_free_irqs(srt_fwk_irq_domain_t *sprt_domain, kbool_t option);
TARGET_EXT ksint32_t fwk_of_irq_get(struct fwk_device_node *sprt_node, kuint32_t index);

TARGET_EXT void *fwk_find_irq_action(ksint32_t irq, const kstring_t *name, void *ptrDev);
TARGET_EXT ksint32_t fwk_request_irq(ksint32_t irq, irq_handler_t handler, kuint32_t flags, const kstring_t *name, void *ptrDev);
TARGET_EXT void fwk_free_irq(ksint32_t irq, void *ptrDev);
TARGET_EXT void fwk_destroy_irq_desc(ksint32_t irq);
TARGET_EXT void fwk_do_irq_handler(ksint32_t softIrq);
TARGET_EXT void fwk_handle_softirq(ksint32_t softIrq, kuint32_t event);


#ifdef __cplusplus
	}
#endif

#endif /* __FWK_IRQ_TYPES_H */
