/*
 * Generic Configuration
 *
 * File Name:   core.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.07.21
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __BOOT_CORE_H
#define __BOOT_CORE_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <configs/configs.h>
#include <common/generic.h>

/*!< The defines */
#define CORE_AMP                                (0)
#define CORE_SMP                                (1)

#if (CONFIG_CORE == CORE_AMP)
#define CONFIG_AMP                              (1)
#else
#define CONFIG_AMP                              (0)
#endif

#if (CONFIG_CORE == CORE_SMP)
#define CONFIG_SMP                              (1)
#else
#define CONFIG_SMP                              (0)
#endif

#if ((CONFIG_CORE > CORE_SMP) || (!CONFIG_AMP && !CONFIG_SMP))
    #error "Multi core running mode error!"
#endif

#if (!defined(CONFIG_CORE_NUM) || (!CONFIG_CORE_NUM))
    #error "Multi core configurations error!"
#endif

/*!
 * cpu affinity
 */
#define CPU_AFFINITY_MASK                       ((1 << CONFIG_CORE_NUM) - 1)
#define __CPU_CHECK_AFFINITY(affinity)          ((affinity) & CPU_AFFINITY_MASK)
#define CPU_AFFINITY_SINGEL(cpuid)              (1 << (cpuid))
#define CPU_AFFINITY_DEFAULT                    __CPU_CHECK_AFFINITY((kuint32_t)(~0))

/*!< IRQ hook */
struct irq_percpu
{
    kint32_t index;

    void (*irq_init)(void *args);
    void *args;
};

/*!< The globals */
extern kuaddr_t g_UND_MODE_STACK_BASE[];
extern kuaddr_t g_ABT_MODE_STACK_BASE[];
extern kuaddr_t g_FIQ_MODE_STACK_BASE[];
extern kuaddr_t g_IRQ_MODE_STACK_BASE[];
extern kuaddr_t g_SYS_MODE_STACK_BASE[];
extern kuaddr_t g_SVC_MODE_STACK_BASE[];

extern struct irq_percpu sgtc_irq_percpus[];

/*!< The functions */
extern kchar_t *get_version(void);
extern kchar_t *get_arch(void);
extern kchar_t *get_arch_type(void);
extern kchar_t *get_arch_class(void);
extern kchar_t *get_cpu_verdor(void);
extern kchar_t *get_cpu_name(void);
extern kchar_t *get_board_manufacturer(void);
extern kchar_t *get_board_name(void);
extern kchar_t *get_cpu_mode(void);

extern void smp_slave_init(void);

/*!< API function */
/*!
 * @brief   set stack of undefined irq
 * @param   cpuid
 * @param   stack_base
 * @retval  none
 * @note    none
 */
static inline void set_und_mode_stack(kuint32_t cpuid, kuaddr_t stack_base)
{
    g_UND_MODE_STACK_BASE[cpuid] = mr_ralign(stack_base, ARCH_PER_SIZE);
}

/*!
 * @brief   set stack of abort (prefetch & data abort)
 * @param   cpuid
 * @param   stack_base
 * @retval  none
 * @note    none
 */
static inline void set_abt_mode_stack(kuint32_t cpuid, kuaddr_t stack_base)
{
    g_ABT_MODE_STACK_BASE[cpuid] = mr_ralign(stack_base, ARCH_PER_SIZE);
}

/*!
 * @brief   set stack of fast irq
 * @param   cpuid
 * @param   stack_base
 * @retval  none
 * @note    none
 */
static inline void set_fiq_mode_stack(kuint32_t cpuid, kuaddr_t stack_base)
{
    g_FIQ_MODE_STACK_BASE[cpuid] = mr_ralign(stack_base, ARCH_PER_SIZE);
}

/*!
 * @brief   set stack of irq
 * @param   cpuid
 * @param   stack_base
 * @retval  none
 * @note    none
 */
static inline void set_irq_mode_stack(kuint32_t cpuid, kuaddr_t stack_base)
{
    g_IRQ_MODE_STACK_BASE[cpuid] = mr_ralign(stack_base, ARCH_PER_SIZE);
}

/*!
 * @brief   set stack of sys
 * @param   cpuid
 * @param   stack_base
 * @retval  none
 * @note    none
 */
static inline void set_sys_mode_stack(kuint32_t cpuid, kuaddr_t stack_base)
{
    g_SYS_MODE_STACK_BASE[cpuid] = mr_ralign(stack_base, ARCH_PER_SIZE);
}

/*!
 * @brief   set stack of svc
 * @param   cpuid
 * @param   stack_base
 * @retval  none
 * @note    none
 */
static inline void set_svc_mode_stack(kuint32_t cpuid, kuaddr_t stack_base)
{
    g_SVC_MODE_STACK_BASE[cpuid] = mr_ralign(stack_base, ARCH_PER_SIZE);
}


#ifdef __cplusplus
    }
#endif

#endif /* __BOOT_CORE_H */
