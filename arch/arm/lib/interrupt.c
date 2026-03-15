/*
 * ARM V7 Interrupt API Function
 *
 * File Name:   interrupt.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <configs/configs.h>
#include <common/generic.h>
#include <common/io_stream.h>
#include <common/time.h>
#include <arch/interrupt.h>
#include <platform/irq/fwk_irq_types.h>
#include <kernel/kernel.h>
#include <kernel/context.h>
#include <kernel/preempt.h>
#include <kernel/sched.h>

/*!< The globals */
// extern kbool_t g_sched_flag;
// extern kuint32_t g_asm_sched_flag;

// static kuint32_t g_irq_nested_count[CONFIG_CORE_NUM] = { 0 };

/*!< API function */
/*!
 * @brief   exec_fiq_handler
 * @param   none
 * @retval  none
 * @note    FIQ exception
 */
void exec_fiq_handler(void)
{
    SET_INTERRUPT_FLAG(FIQ_BIT);

    /*!< Entry */

    CLR_INTERRUPT_FLAG(FIQ_BIT);
}

/*!
 * @brief   exec_irq_handler
 * @param   none
 * @retval  none
 * @note    IRQ exception
 */
void exec_irq_handler(void)
{
    kint32_t hardirq, softIrq;
    kuint32_t cpuid = get_cpu_id();
    struct percpu_sched_data *sptr_sched = &sgtc_sched_data[cpuid];

    SET_INTERRUPT_FLAG(IRQ_BIT);
    INC_HARDIRQ_NEST_COUNT();

    /*!< read IAR, enable IRQ */
    hardirq = hw_irq_acknowledge();

    /*!< find system soft IRQn, and excute IRQ handler */
    softIrq = fwk_gic_to_actual_irq(hardirq);
    fwk_do_irq_handler(softIrq);

    /*!< write IAR, disable IRQ */
    hw_irq_deactivate(hardirq);

    /*!< nesting depth */
    if (fwk_softirq_avaliable() && (IRQ_NEST_COUNT() < 9))
    {
        /*!< check and excute softirq (irq will be open) */
        mr_local_irq_exit();
        fwk_handle_softirq();
        mr_local_irq_enter();
    }

    /*!< preemptetion allowd, update schedule flag */
    if (!mr_preempt_is_locked())
    {
    	kuint32_t thread_info = PERCPU_CURRENT();

//      g_asm_sched_flag = sptr_sched->sched_flag;
    	thread_info |= ((kuint32_t)(!!sptr_sched->sched_flag) << THREAD_SCHED_OFFSET);
    	SET_PERCPU_CURRENT(thread_info);

        sptr_sched->sched_flag = false;
    }

    DEC_HARDIRQ_NEST_COUNT();
    if (!IN_IRQ_NESTD())
        CLR_INTERRUPT_FLAG(IRQ_BIT);
}

/*!
 * @brief   exec_software_irq_handler
 * @param   none
 * @retval  none
 * @note    SWI exception
 */
void exec_software_irq_handler(void)
{
//  kint32_t hardirq;
    kuint32_t event = 0;

    __asm__ __volatile__ (
        " mov %0, r12 " 
        : "=&r"(event) 
    );

    SET_INTERRUPT_FLAG(SWI_BIT);

    /*!< read IAR, enable IRQ */
//  hardirq = hw_irq_acknowledge();
//  fwk_handle_softirq();

    /*!< write IAR, disable IRQ */
//  hw_irq_deactivate(hardirq);

    CLR_INTERRUPT_FLAG(SWI_BIT);
}

/* end of file*/
