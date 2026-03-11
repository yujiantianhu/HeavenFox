/*
 * General Function
 *
 * File Name:   core.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2026.03.03
 *
 * Copyright (c) 2026   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <boot/core.h>

/*!< The defines */

/*!< The globals */
kuaddr_t g_UND_MODE_STACK_BASE[CONFIG_CORE_NUM];
kuaddr_t g_ABT_MODE_STACK_BASE[CONFIG_CORE_NUM];
kuaddr_t g_FIQ_MODE_STACK_BASE[CONFIG_CORE_NUM];
kuaddr_t g_IRQ_MODE_STACK_BASE[CONFIG_CORE_NUM];
kuaddr_t g_SYS_MODE_STACK_BASE[CONFIG_CORE_NUM];
kuaddr_t g_SVC_MODE_STACK_BASE[CONFIG_CORE_NUM];

struct irq_percpu sgtc_irq_percpus[CONFIG_CORE_NUM];

/*!< API functions */
/*!
 * @brief   wake up another cpu (smp)
 * @param   none
 * @retval  none
 * @note    none
 */
__weak void smp_slave_init(void)
{

}

/* end of file */
