/*
 * IMX6ULL Board GPIO Initial
 *
 * File Name:   imx6_led.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <configs/configs.h>
#include <common/time.h>
#include <platform/of/fwk_of.h>
#include <platform/irq/fwk_irq_types.h>
#include "imx6_common.h"

/*!< The defines */
/*!< CCM */
#define IMX_SYSTICK_CLK_CG_REG								CG10
#define IMX_SYSTICK_CLK_SELECT								IMX6UL_CCM_CCGR_CLOCK_ENTRY(1)

/*!< port */
#define IMX_SYSTICK_PORT_ENTRY()                        	IMX6UL_GPT_PROPERTY_ENTRY(1)

/* The globals */
static const struct fwk_of_device_id sgtc_imx_systick_ids[] =
{
	{ .compatible = "fsl,imx6ul-gpt" },
	{},
};

/*!< The functions */
irq_return_t imx6_systick_isr(kint32_t irq, void *ptrDev);

/*!< API function */
/*!
 * @brief   imx6ull_systick_init
 * @param   none
 * @retval  none
 * @note    initial Timer
 */
void imx6ull_systick_init(void)
{
	struct fwk_device_node *sptr_node;
	srt_hal_imx_gptimer_t *sptr_tick;
	kint32_t irq;
	kint32_t retval;

	sptr_node = fwk_of_find_matching_node_and_match(mr_nullptr, sgtc_imx_systick_ids, mr_nullptr);
	if (!isValid(sptr_node))
		return;

	irq = fwk_of_irq_get(sptr_node, 0);
	if (irq < 0)
		return;

	sptr_tick = IMX_SYSTICK_PORT_ENTRY();

	/*!< enable gpt1 clock */
  	mr_imx_ccm_clk_enable(IMX_SYSTICK_CLK_CG_REG, IMX_SYSTICK_CLK_SELECT);

	/*!< 
	 * EN: bit0, GPT Enable
	 * Disable GPT by setting EN=0 in GPT_CR register
	 */
	mr_clrbitl(mr_bit(0U), &sptr_tick->CR);

	/*!< 
	 * SWR: bit15, Software reset of the GPT module. It is a self-clearing bit
	 * Assert the SWR bit in GPT_CR register 
	 */
	mr_setbitl(mr_bit(15U), &sptr_tick->CR);
	while (mr_isBitSetl(mr_bit(15U), &sptr_tick->CR));

	/*!< 
	 * FRR: bit9, Free-Run or Restart mode
	 * The FFR bit determines the behavior of the GPT when a compare event in channel 1 occurs.
	 *	• In Restart mode (bit9 is 0), after a compare event, the counter resets to 0x00000000 and resumes counting
	 *	  (after the occurrence of a compare event).
	 *	• In Free-Run mode (bit9 is 1), after a compare event, the counter continues counting until 0xFFFFFFFF and
	 *	  then rolls over to 0
	 */
	mr_clrbitl(mr_bit(9U), &sptr_tick->CR);

	/*!< 
	 * ROVIE: bit5, Rollover Interrupt Enable
	 * when gpt counter up to 0xffffffff, ROV Interrupt is generated, and clear counter to 0x00000000
	 * Disable GPT interrupt register (GPT_IR)
	 * 
	 * OF1IE ~ OF3IE: bit0 ~ bit3
	 */
	mr_resetl(&sptr_tick->IR);

	retval = fwk_request_irq(irq, imx6_systick_isr, 0, "imx6-systick", sptr_tick);
	if (!retval)
		mr_setbitl(mr_bit(0U), &sptr_tick->IR);

	/*!<
	 * CLKSRC: bit[8:6], Clock Source select.
	 * The CLKSRC bits select which clock will go to the prescaler (and subsequently be used to run the GPT
	 * counter).
	 * • The CLKSRC bit field value should only be changed after disabling the GPT by clearing the EN bit in
	 * this register (GPT_CR).
	 * • A software reset does not affect the CLKSRC bit.
	 * 		000 No clock
	 * 		001 Peripheral Clock (ipg_clk)
	 * 		010 High Frequency Reference Clock (ipg_clk_highfreq)
	 * 		011 External Clock
	 * 		100 Low Frequency Reference Clock (ipg_clk_32k)
	 * 		101 Crystal oscillator as Reference Clock (ipg_clk_24M)
	 * 		others Reserved
	 * 
	 * ipg_clk = 66MHz
	 */
	mr_clrbitl(mr_bit(6U) | mr_bit(7U) | mr_bit(8U), &sptr_tick->CR);
	mr_setbitl(mr_bit(6U), &sptr_tick->CR);

	/*!<
	 * PRESCALER: bit[11:0], Prescaler bits
	 * The clock selected by the CLKSRC field is divided by [PRESCALER + 1], and then used to run the
	 * counter
	 * 
	 * Peripheral clock(ipg_clk) = 66MHz, if divider = 66, GPT1 Frequency = 1MHz;
	 * So Timer Period = 1us
	 */
	mr_clrbitl(0xfffU, &sptr_tick->PR);
	mr_setbitl(66 - 1, &sptr_tick->PR);

	/*!< compare value: 1000us = 1ms; 1000000us = 1000ms = 1s */
	mr_writel(1000000 / TICK_HZ, &sptr_tick->OCR[0]);

	/*!< 
	 * ROV: bit5, Rollover Flag
	 * The ROV bit indicates that the counter has reached its maximum possible value and rolled over to 0 (from
	 * which the counter continues counting). The ROV bit is only set if the counter has reached 0xFFFFFFFF in
	 * both Restart and Free-Run modes
	 * 
	 * Clear GPT status register (GPT_SR) (i.e., w1c) 
	 */
	mr_resetl(&sptr_tick->SR);

	/*!<
	 * ENMOD: bit1, GPT Enable mode
	 * When the GPT is disabled (EN=0), then both the Main Counter and Prescaler Counter freeze their current
	 * count values. The ENMOD bit determines the value of the GPT counter when Counter is enabled again (if
	 * the EN bit is set)
	 * 
	 * • If the ENMOD bit is 1, then the Main Counter and Prescaler Counter values are reset to 0 after GPT
	 * 	 is enabled (EN=1).
	 * • If the ENMOD bit is 0, then the Main Counter and Prescaler Counter restart counting from their
	 *	 frozen values after GPT is enabled (EN=1).
	 *
	 * Set ENMOD=1 in GPT_CR register, to bring GPT counter to 0x00000000
	 */
	mr_setbitl(mr_bit(1U), &sptr_tick->CR);

	/*!< EN: bit0, GPT Enable */
	mr_setbitl(mr_bit(0U), &sptr_tick->CR);

	ptr_systick_counter = (kutime_t *)&sptr_tick->CNT;
}

/*!
 * @brief   imx6ull_systick irq handler
 * @param   none
 * @retval  none
 * @note    increase jiffies
 */
irq_return_t imx6_systick_isr(kint32_t irq, void *ptrDev)
{
	srt_hal_imx_gptimer_t *sptr_tick = (srt_hal_imx_gptimer_t *)ptrDev;

	if (mr_isBitSetl(mr_bit(0), &sptr_tick->SR))
	{
		/*!< reset jiffies when counter over */
		get_time_counter();
		do_timer_event();

		/*!< set 1 to clear compare status */
		mr_setbitl(mr_bit(0), &sptr_tick->SR);
	}

	return ER_NORMAL;
}

/* end of file*/
