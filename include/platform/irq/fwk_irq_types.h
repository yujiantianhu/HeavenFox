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

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/generic.h>
#include <common/list_types.h>
#include <kernel/spinlock.h>

#include "fwk_irq_domain.h"
#include "fwk_irq_chip.h"

/*!< The defines */
#define FWK_IRQ_DESC_NAME_LENTH					(32)

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
    kuint32_t irq;
    kutype_t hwirq;

    struct fwk_irq_chip *sptr_chip;
    struct fwk_irq_domain *sptr_domain;

    struct fwk_irq_data *sptr_parent;
    void *handler_data;
    void *chip_data;

    kuint32_t mask;

} srt_fwk_irq_data_t;

enum __ER_IRQ_RETURN_VAL
{
    NR_IRQ_NONE = 0,
    NR_IRQ_HANDLED = mr_bit(0),
    NR_IRQ_WAKE_THREAD = mr_bit(1),
};

typedef enum __ER_IRQ_RETURN_VAL irq_return_t;
typedef irq_return_t (*irq_handler_t)(kint32_t irq, void *args);

typedef struct fwk_irq_action
{
    kchar_t name[FWK_IRQ_DESC_NAME_LENTH];
    irq_handler_t handler;
    kuint32_t flags;
    void *ptrArgs;

    kuint32_t cpu_affinity;
    struct list_head sgtc_link;

} srt_fwk_irq_action_t;

typedef struct fwk_irq_desc
{
    kint32_t irq;

    kuint32_t flags;
    kchar_t irq_name[FWK_IRQ_DESC_NAME_LENTH];
    struct list_head sgtc_action;

//  struct list_head sgtc_link;
    struct radix_link sgtc_radix;

    struct spin_lock sgtc_lock;
    
    struct fwk_irq_data sgtc_data;

} srt_fwk_irq_desc_t;

/*!< SoftIRQ */
enum __ERT_SOFTIRQ_EVENT
{
    NR_SOFTIRQ_TIMER = 0,
    NR_SOFTIRQ_NET_TX,
    NR_SOFTIRQ_NET_RX,
    NR_SOFTIRQ_TASKLET,
    NR_SOFTIRQ_SCHEDULE,

    NR_SOFTIRQ_NUM,
};

struct fwk_softirq_action
{
    void (*action)(kint32_t event);
};

/*!< Tasklet */
struct fwk_tasklet
{
    struct fwk_tasklet *sptr_next;

    kuint32_t state;
    srt_atomic_t count;

    void (*func)(kutype_t args);
    kutype_t data;
};

#define DECLEARE_TASKLET(name, _func, _data)    \
    struct fwk_tasklet name = { mr_nullptr, 0, ATOMIC_INIT(0), _func, _data }

/*!< The functions */
extern struct fwk_irq_desc *fwk_irq_to_desc(kuint32_t virq);
extern struct fwk_irq_desc *fwk_irq_data_to_desc(struct fwk_irq_data *sptr_data);
extern struct fwk_irq_data *fwk_irq_get_data(kuint32_t virq);
extern struct fwk_irq_data *fwk_irq_domain_get_data(struct fwk_irq_domain *sptr_domain, kuint32_t hwirq);
extern void fwk_irq_desc_set_type(kuint32_t virq, kuint32_t type);
extern kint32_t fwk_irq_domain_find_map(struct fwk_irq_domain *sptr_domain, kuint32_t hwirq, kuint32_t type);
extern kint32_t fwk_irq_domain_alloc_irqs(struct fwk_irq_domain *sptr_domain, kint32_t irq_base, kuint32_t hwirq, kuint32_t nr_irqs);
extern void fwk_irq_desc_free(kint32_t irq);
extern void fwk_irq_domain_free_irqs(struct fwk_irq_domain *sptr_domain);
extern kint32_t fwk_of_irq_get(struct fwk_device_node *sptr_node, kuint32_t index);

extern void *fwk_find_irq_action(kint32_t irq, const kchar_t *name, void *args);
extern kint32_t __fwk_request_threaded_irq(kint32_t irq, irq_handler_t handler, irq_handler_t thread_fn, 
                                kuint32_t flags, kuint32_t cpu_affinity, const kchar_t *name, void *args);
extern void __fwk_free_irq(kint32_t irq, void *args);
extern void fwk_destroy_irq_action(kint32_t irq);
extern void fwk_do_irq_handler(kint32_t softIrq);

extern kuint32_t fwk_softirq_avaliable(void);
extern void local_bh_disable(void);
extern void local_bh_enable(void);
extern void fwk_handle_softirq(void);
extern void fwk_open_softirq(kint32_t nr, void (*action)(kint32_t event));
extern void fwk_raise_softirq(kint32_t nr);
extern void fwk_cancel_softirq(kint32_t nr);
extern void fwk_softirq_init(void);

extern void fwk_tasklet_init(struct fwk_tasklet *sptr_tsk, void (*func)(kutype_t args), kutype_t data);
extern void fwk_tasklet_schedule(struct fwk_tasklet *sptr_tsk);
extern void fwk_tasklet_kill(struct fwk_tasklet *sptr_tsk);

/*!< API functions */
/*!
 * @brief  request irq action (upper and bottom handler)
 * @param  none
 * @retval none
 * @note   irq register
 */
static inline kint32_t fwk_request_threaded_irq(kint32_t irq, irq_handler_t handler, irq_handler_t thread_fn, 
                                kuint32_t flags, const kchar_t *name, void *args)
{
    return __fwk_request_threaded_irq(irq, handler, thread_fn, flags, CPU_AFFINITY_DEFAULT, name, args);
}

/*!
 * @brief  request irq action (upper and bottom handler)
 * @param  none
 * @retval none
 * @note   irq register
 */
static inline kint32_t fwk_request_percpu_threaded_irq(kint32_t irq, irq_handler_t handler, irq_handler_t thread_fn, 
                                kuint32_t flags, kuint32_t cpuid, const kchar_t *name, void *args)
{
    return __fwk_request_threaded_irq(irq, handler, thread_fn, flags, CPU_AFFINITY_SINGEL(cpuid), name, args);
}

/*!
 * @brief  fwk_request_irq
 * @param  none
 * @retval none
 * @note   irq register
 */
static inline kint32_t fwk_request_irq(kint32_t irq, irq_handler_t handler, kuint32_t flags, const kchar_t *name, void *args)
{
    return fwk_request_threaded_irq(irq, handler, mr_nullptr, flags, name, args);
}

/*!
 * @brief  fwk_request_irq
 * @param  none
 * @retval none
 * @note   irq register
 */
static inline kint32_t fwk_request_percpu_irq(kint32_t irq, irq_handler_t handler, kuint32_t flags, 
                                                kuint32_t cpuid, const kchar_t *name, void *args)
{
    return fwk_request_percpu_threaded_irq(irq, handler, mr_nullptr, flags, cpuid, name, args);
}

/*!
 * @brief  fwk_free_irq
 * @param  none
 * @retval none
 * @note   irq unregister
 */
static inline void fwk_free_irq(kint32_t irq, void *args)
{
    __fwk_free_irq(irq, args);
}

#ifdef __cplusplus
    }
#endif

#endif /* __FWK_IRQ_TYPES_H */
