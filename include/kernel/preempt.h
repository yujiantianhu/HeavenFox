/*
 * Kernel Generic Interface Defines
 *
 * File Name:   preempt.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2026.03.13
 *
 * Copyright (c) 2026   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __PREEMPT_H
#define __PREEMPT_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/generic.h>
#include <common/list_types.h>
#include <configs/configs.h>
#include <kernel/sched.h>

/*!< The defines */
#define __IRQ_COUNT(_sptr_thr)                  ((_sptr_thr)->irq_count)
#define __PREEMPT_COUNT(_sptr_thr)              ((_sptr_thr)->sgtc_preempt)

#define IRQ_COUNT()                             __IRQ_COUNT(mr_current)
#define PREEMPT_COUNT()                         __PREEMPT_COUNT(mr_current)

/*!< The globals */
extern kbool_t g_kernel_preempt_enable;

/*!< The defines */
#define mr_preempt_cnt_dec()                    atomic_dec(&PREEMPT_COUNT())
#define mr_preempt_cnt_inc()                    atomic_inc(&PREEMPT_COUNT())
#define mr_preempt_cnt()                        atomic_get_val(&PREEMPT_COUNT())
#define mr_preempt_is_locked()                  (!!mr_preempt_cnt())

#ifdef CONFIG_PREEMPT_NESTING
#define mr_preempt_enable()                     mr_barrier()
#define mr_preempt_disable()                    mr_barrier()
#define mr_preempt_is_locked()                  atomic_get_val(&PREEMPT_COUNT())

#else
#define mr_preempt_enable()	\
    do {	\
        mr_barrier();	\
        if (g_kernel_preempt_enable && mr_preempt_is_locked())  \
            mr_preempt_cnt_dec();   \
    } while (0)

#define mr_preempt_disable()	\
    do {	\
        g_kernel_preempt_enable ? mr_preempt_cnt_inc() : (void)0;	\
        mr_barrier();	\
    } while (0)

#endif

/*!< bit[9:0]: softirq nest count */
#define SOFTIRQ_BITS                            (10)
/*!< bit[19:10]: irq nest count */
#define HARDIRQ_BITS                            (10)
/*!< bit[31:20]: interrupt mode (bit[27:20]: exception mode) */
#define INTERRUPT_BITS                          (12)

/*!< 0 */
#define SOFTIRQ_OFFSET                          (0)
/*!< 8 */
#define HARDIRQ_OFFSET                          (SOFTIRQ_OFFSET + SOFTIRQ_BITS)
/*!< 16 */
#define INTERRUPT_OFFSET                        (HARDIRQ_OFFSET + HARDIRQ_BITS)

/*!< 0x000003ff */
#define SOFTIRQ_MASK                            mr_mk_mask(SOFTIRQ_BITS, SOFTIRQ_OFFSET)
/*!< 0x000ffc00 */
#define HARDIRQ_MASK                            mr_mk_mask(HARDIRQ_BITS, HARDIRQ_OFFSET)
/*!< 0xfff00000 */
#define INTERRUPT_MASK                          mr_mk_mask(INTERRUPT_BITS, INTERRUPT_OFFSET)

#define __SOFTIRQ_COUNT(count)                  mr_get_mask(count, SOFTIRQ_MASK, SOFTIRQ_OFFSET)
#define __IRQ_NEST_COUNT(count)                 mr_get_mask(count, HARDIRQ_MASK, HARDIRQ_OFFSET)
#define __INTERRUPT_MODE(count)                 mr_get_mask(count, INTERRUPT_MASK, INTERRUPT_OFFSET)
#define SOFTIRQ_COUNT()                         __SOFTIRQ_COUNT(IRQ_COUNT())
#define IRQ_NEST_COUNT()                        __IRQ_NEST_COUNT(IRQ_COUNT())
#define INTERRUPT_MODE()                        __INTERRUPT_MODE(IRQ_COUNT())

/*!< for INTERRUPT_BITS: */
#define IRQ_INTERRUPT_BITS                      (4)     /*!< for irq, fiq, swi */
#define IRQ_EXCEPTION_BITS                      (8)     /*!< for und, abt, unuse */
#define IRQ_INTERRUPT_OFFSET                    (0)
#define IRQ_EXCEPTION_OFFSET                    (IRQ_INTERRUPT_OFFSET + IRQ_INTERRUPT_BITS)
#define IRQ_INTERRUPT_MASK                      mr_mk_mask(IRQ_INTERRUPT_BITS, IRQ_INTERRUPT_OFFSET)
#define IRQ_EXCEPTION_MASK                      mr_mk_mask(IRQ_EXCEPTION_BITS, IRQ_EXCEPTION_OFFSET)
#define IRQ_INTERRUPT_MODE()                    mr_get_mask(INTERRUPT_MODE(), IRQ_INTERRUPT_MASK, IRQ_INTERRUPT_OFFSET)
#define IRQ_EXCEPTION_MODE()                    mr_get_mask(INTERRUPT_MODE(), IRQ_EXCEPTION_MASK, IRQ_EXCEPTION_OFFSET)

/*!< -------------------------------------------------------------------------------- */
#define __IS_SOFTIRQ_LOCKED(count)              ((count) & SOFTIRQ_MASK)
#define __IN_IRQ_NESTD(count)                   ((count) & HARDIRQ_MASK)
#define __IN_INTERRUPT(count)                   ((count) & INTERRUPT_MASK)
#define __IN_IRQ_INTERRUPT(count)               (__INTERRUPT_MODE(count) & IRQ_INTERRUPT_MASK)
#define __IN_IRQ_EXCEPTION(count)               (__INTERRUPT_MODE(count) & IRQ_EXCEPTION_MASK)

#define __SET_INTERRUPT_FLAG(count, mask)       do { (count) |=  ((mask) << INTERRUPT_OFFSET); mr_smp_mb(); } while (0)
#define __CLR_INTERRUPT_FLAG(count, mask)       do { (count) &= ~((mask) << INTERRUPT_OFFSET); mr_smp_mb(); } while (0)
#define __SET_EXCEPTION_FLAG(mask)              __SET_INTERRUPT_FLAG(mask)
#define __CLR_EXCEPTION_FLAG(mask)              __CLR_INTERRUPT_FLAG(mask)

#define IS_SOFTIRQ_LOCKED()                     __IS_SOFTIRQ_LOCKED(IRQ_COUNT())
#define IN_IRQ_NESTD()                          __IN_IRQ_NESTD(IRQ_COUNT())
#define IN_INTERRUPT()                          __IN_INTERRUPT(IRQ_COUNT())
#define IN_IRQ_INTERRUPT()                      __IN_IRQ_INTERRUPT(IRQ_COUNT())
#define IN_IRQ_EXCEPTION()                      __IN_IRQ_EXCEPTION(IRQ_COUNT())

#define SET_INTERRUPT_FLAG(mask)                __SET_INTERRUPT_FLAG(IRQ_COUNT(), mask)
#define CLR_INTERRUPT_FLAG(mask)                __CLR_INTERRUPT_FLAG(IRQ_COUNT(), mask)
#define SET_EXCEPTION_FLAG(mask)                SET_INTERRUPT_FLAG(mask)
#define CLR_EXCEPTION_FLAG(mask)                CLR_INTERRUPT_FLAG(mask)

#define INC_HARDIRQ_NEST_COUNT()                do { IRQ_COUNT() += (1 << HARDIRQ_OFFSET); mr_smp_mb(); } while (0)
#define DEC_HARDIRQ_NEST_COUNT()    \
    do {    \
        if (IN_IRQ_NESTD()) {   \
            IRQ_COUNT() -= (1 << HARDIRQ_OFFSET);   \
            mr_smp_mb();    \
        }   \
    } while (0)

#define mr_local_bh_is_locked()                 IS_SOFTIRQ_LOCKED()
#define mr_local_bh_disable()                   do { IRQ_COUNT() += (1 << SOFTIRQ_OFFSET); mr_smp_mb(); } while (0)
#define mr_local_bh_enable()    \
    do {    \
        if (mr_likely(mr_local_bh_is_locked())) {   \
            IRQ_COUNT() -= (1 << SOFTIRQ_OFFSET);   \
            mr_smp_mb();    \
        }   \
    } while (0)

/*!< The functions */
/*!
 * @brief  check if irq bottom half is locked
 * @param  none
 * @retval 1: locked; 0: not locked
 * @note   get (irq_count & SOFTIRQ_MASK) ? true : false
 */
__force_inline 
static inline kbool_t local_bh_is_locked(void)
{
    return !!mr_local_bh_is_locked();
}

/*!
 * @brief  disable irq bottom half
 * @param  none
 * @retval none
 * @note   inc irq_count
 */
__force_inline 
static inline void local_bh_disable(void)
{
    mr_local_bh_disable();
}

/*!
 * @brief  enable irq bottom half
 * @param  none
 * @retval none
 * @note   dec irq_count
 */
__force_inline 
static inline void local_bh_enable(void)
{
    mr_local_bh_enable();
}

#ifdef __cplusplus
    }
#endif

#endif /* __PREEMPT_H */
