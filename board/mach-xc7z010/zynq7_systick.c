/*
 * ZYNQ7 Board Terminal Initial
 *
 * File Name:   zynq7_console.c
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

/*!< The globals */
static XScuTimer sgtc_ps7_xscu_timer_data;

#ifdef CONFIG_OF
static const struct fwk_of_device_id sgtc_zynq7_systick_ids[] =
{
	{ .compatible = "arm,cortex-a9-twd-timer" },
	{},
};
#endif

/*!< The functions */
irq_return_t zynq7_systick_isr(kint32_t irq, void *ptrDev);

/*!< API function */
/*!
 * @brief   initial and start serial
 * @param   none
 * @retval  none
 * @note    none
 */
void zynq7_systick_init(void)
{
    struct fwk_device_node *sptr_node;
    XScuTimer *sptr_timer;
    XScuTimer_Config *sptr_cfg, sgtc_cfg;
    kuaddr_t irq;
    kint32_t retval;

    sptr_timer = &sgtc_ps7_xscu_timer_data;

#ifdef CONFIG_OF
    sptr_cfg = &sgtc_cfg;

    sptr_node = fwk_of_find_matching_node_and_match(mr_nullptr, sgtc_zynq7_systick_ids, mr_nullptr);
    if (!isValid(sptr_node))
        return;

    sptr_cfg->DeviceId = XPAR_PS7_SCUTIMER_0_DEVICE_ID;
    sptr_cfg->BaseAddr = (kuaddr_t)fwk_of_iomap(sptr_node, 0);
    irq = fwk_of_irq_get(sptr_node, 0);

#else
    sptr_cfg = XScuTimer_LookupConfig(XPAR_PS7_SCUTIMER_0_DEVICE_ID);
    if (!isValid(sptr_cfg))
        return;

    irq = XPAR_SCUTIMER_INTR - 16;

#endif

    retval = XScuTimer_CfgInitialize(sptr_timer, sptr_cfg, sptr_cfg->BaseAddr);
    if (retval)
        return;

    XScuTimer_DisableInterrupt(sptr_timer);

    /*!< enable interrupt */
    retval = fwk_request_irq(irq, zynq7_systick_isr, 0, "zynq7-systick", sptr_timer);
    if (!retval)
        XScuTimer_EnableInterrupt(sptr_timer);

    /*!< 10ms */
    XScuTimer_LoadTimer(sptr_timer, ZYNQ7_SCUTIMER0_FREQ_HZ / TICK_HZ);
    XScuTimer_EnableAutoReload(sptr_timer);
    XScuTimer_Start(sptr_timer);

    print_info("System Tick start to run, frequency is: %d\r\n", TICK_HZ);
}

/*!
 * @brief   zynq7 systick irq handler
 * @param   none
 * @retval  none
 * @note    increase jiffies
 */
irq_return_t zynq7_systick_isr(kint32_t irq, void *ptrDev)
{
    XScuTimer *sptr_timer = (XScuTimer *)ptrDev;

    if (XScuTimer_GetInterruptStatus(sptr_timer))
    {
        /*!< reset jiffies when counter over */
		get_time_counter();
		do_timer_event();

        XScuTimer_ClearInterruptStatus(sptr_timer);
    }

    return ER_NORMAL;
}

/* end of file*/
