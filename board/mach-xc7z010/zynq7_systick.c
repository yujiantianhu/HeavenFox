/*
 * ZYNQ7 Board Terminal Initial
 *
 * File Name:   zynq7_systick.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.06.22
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/time.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/irq/fwk_irq_types.h>
#include "zynq7_common.h"

/*!< The defines */
#define ZYNQ7_SCUTIMER0_FREQ_HZ                 (XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2)

struct ps7_xtime_data
{
    XScuTimer sgtc_tick;

    struct fwk_device_node *sptr_ticknd;
    kint32_t tick_irq;
};

/*!< The globals */
static struct ps7_xtime_data sgtc_ps7_xtime_data;

#ifdef CONFIG_OF
static const struct fwk_of_device_id sgtc_zynq7_systick_ids[] =
{
    { .compatible = "arm,cortex-a9-twd-timer" },
//  { .compatible = "arm,cortex-a9-global-timer" },
    {},
};
#endif

/*!< The functions */
irq_return_t zynq7_systick_isr(kint32_t irq, void *args);

/*!< API function */
/*!
 * @brief   initial and start systick
 * @param   none
 * @retval  none
 * @note    none
 */
void zynq7_systick_init(void)
{
    struct ps7_xtime_data *sptr_data;
    struct fwk_device_node *sptr_node;
    XScuTimer *sptr_timer;
    XScuTimer_Config *sptr_cfg, sgtc_cfg;
    kuaddr_t irq;
    kint32_t retval;

    sptr_data = &sgtc_ps7_xtime_data;
    sptr_timer = &sptr_data->sgtc_tick;

#ifdef CONFIG_OF
    sptr_cfg = &sgtc_cfg;

    sptr_node = fwk_of_find_matching_node_and_match(mr_nullptr, sgtc_zynq7_systick_ids, mr_nullptr);
    if (!isValid(sptr_node))
        return;

    sptr_cfg->DeviceId = 0;
    sptr_cfg->BaseAddr = (kuaddr_t)fwk_of_iomap(sptr_node, 0);
    irq = fwk_of_irq_get(sptr_node, 0);

    sptr_data->sptr_ticknd = sptr_node;

#else
    sptr_cfg = XScuTimer_LookupConfig(XPAR_PS7_SCUTIMER_0_DEVICE_ID);
    if (!isValid(sptr_cfg))
        return;

    irq = XPAR_SCUTIMER_INTR - 16;

#endif

    sptr_data->tick_irq = irq;

    retval = XScuTimer_CfgInitialize(sptr_timer, sptr_cfg, sptr_cfg->BaseAddr);
    if (retval)
        return;

    XScuTimer_DisableInterrupt(sptr_timer);

    /*!< enable interrupt */
    retval = fwk_request_irq(irq, zynq7_systick_isr, 0, "zynq7-systick", sptr_data);
    if (!retval)
        XScuTimer_EnableInterrupt(sptr_timer);

    /*!< Period: 3ns */
    SYSTICK_INIT(ZYNQ7_SCUTIMER0_FREQ_HZ,
                 IS_TICKCNT_DEC,
                 sptr_timer->Config.BaseAddr + XSCUTIMER_COUNTER_OFFSET);

    /*!< TICK_HZ = 200, period = 5ms */
    XScuTimer_LoadTimer(sptr_timer, SYSTICK_FREQ / TICK_HZ);

#if 1
    XScuTimer_EnableAutoReload(sptr_timer);
#else
    XScuTimer_DisableAutoReload(sptr_timer);
#endif

    XScuTimer_Start(sptr_timer);
    
    print_info("System Tick start to run, frequency is: %u(Hz)\r\n", SYSTICK_FREQ);
    print_info("System counter (jiffies) enable, TICK_HZ is %u(Hz)\r\n", TICK_HZ);
}

/*!
 * @brief   zynq7 systick irq handler
 * @param   none
 * @retval  none
 * @note    increase jiffies
 */
irq_return_t zynq7_systick_isr(kint32_t irq, void *args)
{
    struct ps7_xtime_data *sptr_data = (struct ps7_xtime_data *)args;
    XScuTimer *sptr_timer = &sptr_data->sgtc_tick;
    kuint32_t status;

    /*!< Timer Interrupt */
    status = XScuTimer_GetInterruptStatus(sptr_timer);
    if (status)
    {
        /*!< reset jiffies when counter over */
        get_time_counter();
        do_timer_event();

        XScuTimer_ClearInterruptStatus(sptr_timer);
    }

    return ER_NORMAL;
}

/* end of file*/
