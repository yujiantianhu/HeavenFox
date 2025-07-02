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
#define IMX_HRTIMER_CLK_CG_REG								CG10
#define IMX_HRTIMER_CLK_SELECT								IMX6UL_CCM_CCGR_CLOCK_ENTRY(1)

/*!< port */
#define IMX_HRTIMER_PORT_ENTRY()                        	IMX6UL_GPT_PROPERTY_ENTRY(1)

/*!< The globals */
static const struct fwk_of_device_id sgtc_imx_hrtimer_ids[] =
{
    { .compatible = "fsl,imx6ul-gpt" },
    {},
};

/*!< The functions */
irq_return_t imx6_hrtimer_isr(kint32_t irq, void *ptrDev);

/*!< API function */
/*!
 * @brief   load compare value to hardware
 * @param   expires (unit: tick)
 * @retval  none
 * @note    called by systick interrupt
 */
void khrtime_reload_cnt(khrtime_t expires)
{
    srt_hal_imx_gptimer_t *sptr_tick;
    khrtime_t val, max;

    sptr_tick = IMX_HRTIMER_PORT_ENTRY();
    mr_clrbitl(mr_bit(0U), &sptr_tick->IR);

    max = HRTIMER_MAX;
    expires -= (g_hrtime_over_cnt * max);
    val = (expires > max) ? (expires - max) : expires;

    mr_writel(val, &sptr_tick->OCR[0]);
    mr_setbitl(mr_bit(0U), &sptr_tick->IR);
}

/*!
 * @brief   Check if hrtime counter out
 * @param   none
 * @retval  none
 * @note    true or false
 */
kuint32_t khrtime_check_overcnt(void)
{
    srt_hal_imx_gptimer_t *sptr_tick;
    kuint32_t flags;

    sptr_tick = IMX_HRTIMER_PORT_ENTRY();

    mr_local_irq_save(flags);
    
    if (mr_bit(5U) & mr_readl(&sptr_tick->SR))
    {
        g_hrtime_over_cnt++;
        mr_writel(mr_bit(5U), &sptr_tick->SR);
    }

    mr_local_irq_restore(flags);
    return g_hrtime_over_cnt;
}

/*!
 * @brief   imx6ull_hrtimer_init
 * @param   none
 * @retval  none
 * @note    initial Timer
 */
void imx6ull_hrtimer_init(void)
{
    struct fwk_device_node *sptr_node;
    srt_hal_imx_gptimer_t *sptr_tick;
    kint32_t irq;
    kint32_t retval;

    sptr_node = fwk_of_find_matching_node_and_match(mr_nullptr, sgtc_imx_hrtimer_ids, mr_nullptr);
    if (!isValid(sptr_node))
        return;

    irq = fwk_of_irq_get(sptr_node, 0);
    if (irq < 0)
        return;

    sptr_tick = IMX_HRTIMER_PORT_ENTRY();

    /*!< enable gpt1 clock */
    mr_imx_ccm_clk_enable(IMX_HRTIMER_CLK_CG_REG, IMX_HRTIMER_CLK_SELECT);

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
    mr_setbitl(mr_bit(9U), &sptr_tick->CR);

    /*!< 
     * ROVIE: bit5, Rollover Interrupt Enable
     * when gpt counter up to 0xffffffff, ROV Interrupt is generated, and clear counter to 0x00000000
     * Disable GPT interrupt register (GPT_IR)
     * 
     * OF1IE ~ OF3IE: bit0 ~ bit3
     */
    mr_resetl(&sptr_tick->IR);

    /*!< Enable OCR[0] and Rollover IRQ */
    retval = fwk_request_irq(irq, imx6_hrtimer_isr, 0, "imx6-hrtimer", sptr_tick);
    if (!retval)
        mr_setbitl(mr_bit(0U) | mr_bit(5U), &sptr_tick->IR);

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

#if 0
    mr_setbitl(66U - 1, &sptr_tick->PR);

    /*!< Global interface */
    /*!< Timer Period = 1us, CNT will over after 70min */
    HRTIMER_INIT(1000000U, IS_TICKCNT_INC, &sptr_tick->CNT, mr_nullptr);

#else
    mr_setbitl(0U, &sptr_tick->PR);

    /*!< Global interface */
    /*!< Timer Period = 15.15ns, CNT will over after 65s */
    HRTIMER_INIT(66000000U, IS_TICKCNT_INC, &sptr_tick->CNT, mr_nullptr);
#endif

    /*!< compare value */
    mr_writel(HRTIMER_MAX, &sptr_tick->OCR[0]);

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

    print_info("System hrtimer start to run, frequency is: %u(Hz)\r\n", HRTIMER_FREQ);
}

/*!
 * @brief   irq handler
 * @param   none
 * @retval  none
 * @note    none
 */
irq_return_t imx6_hrtimer_isr(kint32_t irq, void *ptrDev)
{
    srt_hal_imx_gptimer_t *sptr_tick = (srt_hal_imx_gptimer_t *)ptrDev;
    kuint32_t status;

    status = mr_readl(&sptr_tick->SR);

    /*!< Timer ISR */
    if (mr_isBitSetl(mr_bit(5U), &status))
    {
        /*!< Over interrupt */
        mark_hrtime_overone();

        /*!< Just clear over bit (clear after marking right away) */
        mr_writel(mr_bit(5U), &sptr_tick->SR);

//      check_hrtimer();
    }   
    if (mr_isBitSetl(mr_bit(0U), &status))
    {
        /*!< Compare interrupt */
        do_hrtime_event();

        /*!< 
         * Just clear compare bit 
         * (if over status bit become 1 during "do_hrtime_event()", keep status for next solution) 
         */
        mr_writel(mr_bit(0U), &sptr_tick->SR);
    }

    return ER_NORMAL;
}

/* end of file*/
