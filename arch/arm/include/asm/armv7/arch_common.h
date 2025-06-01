/*
 * ARMv7 Configuration
 *
 * File Name:   arch_common.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.12.16
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __ARCH_COMMON_H
#define __ARCH_COMMON_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include "asm_config.h"
#include "gcc_config.h"
#include "gic_basic.h"

/*!< The defines */
#if (defined(CONFIG_OF))
    #define CPU_INTC_COMPATIBLE                             "arm,cortex-a7-gic"
#endif

#define SOFTIRQ_CALL(x)                                     __asm__ __volatile__ ("svc %0" : : "i"(x))

#define mr_get_irq_pri(irqNumber)                           hw_irq_get_priority(irqNumber)
#define mr_set_irq_pri(irqNumber, pri)                      hw_irq_set_priority(irqNumber, pri)

#define mr_local_irq_enable()                               mr_enable_cpu_irq()
#define mr_local_irq_disable()                              mr_disable_cpu_irq()

#define mr_local_irq_save(flags)   \
    do {    \
        flags = __get_cpsr();   \
        mr_disable_cpu_irq();   \
    } while (0)

#define mr_local_irq_restore(flags)   \
    do {    \
        if (!(flags & CPSR_BIT_I)) \
            mr_enable_cpu_irq();    \
    } while (0)

/*!< API function */
/*!
 * @brief   disable irq
 * @param   none
 * @retval  none
 * @note    GIC Interrupt
 */
__force_inline static inline void local_irq_enable(void)
{
    mr_enable_cpu_irq();
}

/*!
 * @brief   enable irq
 * @param   none
 * @retval  none
 * @note    GIC Interrupt
 */
__force_inline static inline void local_irq_disable(void)
{
    mr_disable_cpu_irq();
}

/*!
 * @brief   disable irq and save
 * @param   none
 * @retval  none
 * @note    GIC Interrupt
 */
__force_inline static inline void local_irq_save(kutype_t *flags)
{
    *flags = __get_cpsr();
    mr_disable_cpu_irq();
}

/*!
 * @brief   restore
 * @param   none
 * @retval  none
 * @note    GIC Interrupt
 */
__force_inline static inline void local_irq_restore(kutype_t *flags)
{
    if (!(*flags & CPSR_BIT_I))
        mr_enable_cpu_irq();
}

/*!
 * @brief   get irq priority
 * @param   none
 * @retval  none
 * @note    GIC Interrupt
 */
static inline kuint32_t get_irq_priority(kint32_t irqNumber)
{
    return mr_get_irq_pri(irqNumber);
}

/*!
 * @brief   set irq priority
 * @param   none
 * @retval  none
 * @note    GIC Interrupt
 */
static inline void set_irq_priority(kint32_t irqNumber, kuint32_t priority)
{
    mr_set_irq_pri(irqNumber, priority);
}

#ifdef __cplusplus
    }
#endif

#endif /* __ARCH_COMMON_H */
