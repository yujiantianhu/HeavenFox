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

struct ps7_xtime_data
{
    XScuTimer sgtc_timer;
    XTtcPs sgtc_ttc;

    struct fwk_device_node *sptr_timnd;
    struct fwk_device_node *sptr_ttcnd;
    kint32_t timer_irq;
    kint32_t ttc_irq;
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

static const struct fwk_of_device_id sgtc_zynq7_ttc_ids[] =
{
	{ .compatible = "cdns,ttc" },
	{},
};
#endif

/*!< The functions */
irq_return_t zynq7_systick_isr(kint32_t irq, void *ptrDev);

/*!< API function */
#if 0
/*!
 * @brief   load compare value to hardware
 * @param   expires (unit: tick)
 * @retval  none
 * @note    called by systick interrupt
 */
void reload_htick_cnt(kutime_t expires)
{
    XTtcPs *sptr_ttc = &sgtc_ps7_xtime_data.sgtc_ttc;

    XTtcPs_Stop(sptr_ttc);
    
    /*!< Set expect tick */
    XTtcPs_SetMatchValue(sptr_ttc, sptr_ttc->Config.DeviceId, expires);
    
    XTtcPs_Start(sptr_ttc);
}

/*!
 * @brief   reset htick counter
 * @param   none
 * @retval  none
 * @note    none
 */
void reset_htick_cnt(void)
{
    XTtcPs *sptr_ttc = &sgtc_ps7_xtime_data.sgtc_ttc;

    XTtcPs_Stop(sptr_ttc);
    XTtcPs_ResetCounterValue(sptr_ttc);
    XTtcPs_Start(sptr_ttc);
}

/*!
 * @brief   initial and start htick
 * @param   none
 * @retval  none
 * @note    none
 */
static void zynq7_htick_init(void)
{
    struct ps7_xtime_data *sptr_data;
    struct fwk_device_node *sptr_node;
    XTtcPs *sptr_ttc;
    XTtcPs_Config *sptr_cfg, sgtc_cfg;
    kuaddr_t irq;
    kuint32_t prescaler;
    kint32_t retval;

    sptr_data = &sgtc_ps7_xtime_data;
    sptr_ttc = &sptr_data->sgtc_ttc;

#ifdef CONFIG_OF
    sptr_cfg = &sgtc_cfg;

    sptr_node = fwk_of_find_matching_node_and_match(mr_nullptr, sgtc_zynq7_ttc_ids, mr_nullptr);
    if (!isValid(sptr_node))
        return;

    sptr_cfg->DeviceId = XPAR_PS7_TTC_0_DEVICE_ID;
    sptr_cfg->BaseAddress = (kuaddr_t)fwk_of_iomap(sptr_node, 0);
    sptr_cfg->InputClockHz = XPAR_PS7_TTC_0_TTC_CLK_FREQ_HZ;
    irq = fwk_of_irq_get(sptr_node, 0);

    sptr_data->sptr_ttcnd = sptr_node;

#else
    sptr_cfg = XTtcPs_LookupConfig(XPAR_PS7_TTC_0_DEVICE_ID);
    if (!isValid(sptr_cfg))
        return;

    irq = XPAR_XTTCPS_0_INTR - 16;

#endif

    sptr_data->ttc_irq = irq;

    retval = XTtcPs_CfgInitialize(sptr_ttc, sptr_cfg, sptr_cfg->BaseAddress);
    if (retval)
        return;

    /*!< Disable IRQ and Timer */
    XTtcPs_DisableInterrupts(sptr_ttc, XTTCPS_IXR_MATCH_0_MASK);
    XTtcPs_Stop(sptr_ttc);

    g_is_systick_up = true;
    ptr_systick_counter = (volatile kutime_t *)(sptr_ttc->Config.BaseAddress + XTTCPS_COUNT_VALUE_OFFSET);
    /*!< Period: 1us (1MHz) */
    g_systick_freq = 1000000;

    /*!< Set clocksource (use cpu clock) */
    XTtcPs_SetOptions(sptr_ttc, XTTCPS_OPTION_WAVE_DISABLE | XTTCPS_OPTION_MATCH_MODE);

    /*!< PrescalerValue = (InputClockHz / 1MHz) - 1 = 110.111115 */
    prescaler = sptr_cfg->InputClockHz / SYSTICK_FREQ - 1;
    XTtcPs_SetPrescaler(sptr_ttc, (kuint8_t)((prescaler < 255) ? prescaler : 255));

    /*!< enable interrupt */
    retval = fwk_request_irq(irq, zynq7_systick_isr, 0, "zynq7-htime_tick", &sgtc_ps7_xtime_data);
    if (!retval)
        XTtcPs_EnableInterrupts(sptr_ttc, XTTCPS_IXR_MATCH_0_MASK);

    /*!< Reset */
    XTtcPs_ClearInterruptStatus(sptr_ttc, XTTCPS_IXR_MATCH_0_MASK);

    /*!< Load expect tick and start TTC */
    reload_htick_cnt(1000);
    XTtcPs_ResetCounterValue(sptr_ttc);
}
#endif

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
    sptr_timer = &sptr_data->sgtc_timer;

#ifdef CONFIG_OF
    sptr_cfg = &sgtc_cfg;

    sptr_node = fwk_of_find_matching_node_and_match(mr_nullptr, sgtc_zynq7_systick_ids, mr_nullptr);
    if (!isValid(sptr_node))
        return;

    sptr_cfg->DeviceId = 0;
    sptr_cfg->BaseAddr = (kuaddr_t)fwk_of_iomap(sptr_node, 0);
    irq = fwk_of_irq_get(sptr_node, 0);

    sptr_data->sptr_timnd = sptr_node;

#else
    sptr_cfg = XScuTimer_LookupConfig(XPAR_PS7_SCUTIMER_0_DEVICE_ID);
    if (!isValid(sptr_cfg))
        return;

    irq = XPAR_SCUTIMER_INTR - 16;

#endif

    sptr_data->timer_irq = irq;

    retval = XScuTimer_CfgInitialize(sptr_timer, sptr_cfg, sptr_cfg->BaseAddr);
    if (retval)
        return;

    XScuTimer_DisableInterrupt(sptr_timer);

    /*!< enable interrupt */
    retval = fwk_request_irq(irq, zynq7_systick_isr, 0, "zynq7-systick", sptr_data);
    if (!retval)
        XScuTimer_EnableInterrupt(sptr_timer);

    /*!< 10ms */
    XScuTimer_LoadTimer(sptr_timer, ZYNQ7_SCUTIMER0_FREQ_HZ / TICK_HZ);

#if 1
    XScuTimer_EnableAutoReload(sptr_timer);
#else
    XScuTimer_DisableAutoReload(sptr_timer);
#endif

//  zynq7_htick_init();

    g_is_systick_up = true;
    ptr_systick_counter = (volatile kutime_t *)(sptr_timer->Config.BaseAddr + XSCUTIMER_COUNTER_OFFSET);
    /*!< Period: 0.3us */
    g_systick_freq = ZYNQ7_SCUTIMER0_FREQ_HZ;

    XScuTimer_Start(sptr_timer);

    print_info("System Tick start to run, frequency is: %d\r\n", TICK_HZ);
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
    XScuTimer *sptr_timer = &sptr_data->sgtc_timer;
//  XTtcPs *sptr_ttc = &sptr_data->sgtc_ttc;
    kuint32_t status;

    /*!< Timer Interrupt */
    status = XScuTimer_GetInterruptStatus(sptr_timer);
    if (status)
    {
        /*!< sync tick */
//      reset_htick_cnt();

        /*!< reset jiffies when counter over */
		get_time_counter();
		do_timer_event();

        XScuTimer_ClearInterruptStatus(sptr_timer);
    }

    /*!< Ttc Interrupt */
//  status = XTtcPs_GetInterruptStatus(sptr_ttc);
//  if (status)
//  {
//      if (status & XTTCPS_IXR_MATCH_0_MASK)
//          do_htick_event();
//
//      XTtcPs_ClearInterruptStatus(sptr_ttc, status);
//  }

    return ER_NORMAL;
}

/* end of file*/
