/*
 * IMX6ULL Global Timer Initial
 *
 * File Name:   hrtimer.c
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
#include <common/generic.h>
#include <common/io_stream.h>
#include <arch/setup.h>
#include <imx6/imx6ull_clocks.h>
#include <imx6/imx6ull_pins.h>
#include <imx6/imx6ull_periph.h>
#include <imx6/imx6ull_irqvector.h>
#include <platform/of/fwk_of.h>
#include <platform/irq/fwk_irq_types.h>

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
 * @brief   Check if hrtime counter out
 * @param   none
 * @retval  none
 * @note    The newest over count
 */
kuint32_t khrtime_check_overcnt(void)
{
    srt_hal_imx_gptimer_t *sptr_tick;
    struct ktime_manage *sptr_tmn = HRTIMER_PTR;

    sptr_tick = IMX_HRTIMER_PORT_ENTRY();
    
    if (mr_bit(5U) & mr_readl(&sptr_tick->SR))
    {
        sptr_tmn->over_cnt++;
        mr_writel(mr_bit(5U), &sptr_tick->SR);
    }

    return sptr_tmn->over_cnt;
}

/*!
 * @brief   load compare value to hardware
 * @param   expires (unit: tick)
 * @retval  none
 * @note    called by systick interrupt
 */
void khrtime_reload_cnt(khrtime_t expires)
{
#define KHRTIME_ADJUST_TICK(cur, limit) \
    (kuint32_t)(((cur) > (limit)) ? ((cur) - (limit)) : (cur))

    srt_hal_imx_gptimer_t *sptr_tick;
    khrtime_t max, one_us, cur_tick;
    kuint32_t over_cnt, over_cnt2, exp_tick = 0;
    kuint32_t flags;

    sptr_tick = IMX_HRTIMER_PORT_ENTRY();
    max = HRTIMER_MAX;
    one_us = USEC_TO_HRTICK(1);

    mr_local_irq_save(flags);
    over_cnt2 = khrtime_check_overcnt();

    do {
        over_cnt = over_cnt2;
        cur_tick = (khrtime_t)mr_readl(&sptr_tick->CNT);
        over_cnt2 = khrtime_check_overcnt();

        /*!< Check if CNT is over max during reading, ensure cur_tick is avaliable */
    } while (over_cnt != over_cnt2);

    /*!< 
     * If khrtime_reload_cnt is called, it indiacates that OCR[0] will be changed, 
     * and expires must be the smallest time!
     * Ignore invalid and out-of-time compare event
     */
    mr_writel(mr_bit(0U), &sptr_tick->SR);

    /*!< Expect time is not reached */
    if (expires > (cur_tick + (over_cnt * (khrtime_t)max)))
    {
        expires -= over_cnt * max;

        /*!< cur_tick must lower than exp_tick, but there values maybe similar */
        cur_tick += one_us;
        expires = CMP_MAX2(cur_tick, expires);
        exp_tick = KHRTIME_ADJUST_TICK(expires, max);
    }
    else
    {
        /*!< +1us to ensure that compare can be triggered next time */
        cur_tick += one_us;
        exp_tick = KHRTIME_ADJUST_TICK(cur_tick, max);
    }

    mr_writel(exp_tick, &sptr_tick->OCR[0]);
    mr_local_irq_restore(flags);

#undef  KHRTIME_ADJUST_TICK
}

/*!
 * @brief   Read high time total tick
 * @param   none
 * @retval  tick
 * @note    get the time register's current value
 */
khrtime_t khrtime_ticks(void)
{
    volatile kutime_t *time_cnt = HRTIMER_PTR->count;
    kutime_t tick, over_count, over_count2;
    kuint32_t max = HRTIMER_MAX;
    kuint32_t flags;

    if (mr_unlikely(!time_cnt))
        return 0;

    mr_local_irq_save(flags);
    over_count2 = khrtime_check_overcnt();

    do {
        over_count = over_count2;
        tick = *time_cnt;
        over_count2 = khrtime_check_overcnt();

        /*!< Check if counter is over max, and return over count */
    } while (over_count != over_count2);

    mr_local_irq_restore(flags);
    return ((khrtime_t)tick + (over_count * (khrtime_t)max));
}

/*!
 * @brief   start hrtimer
 * @param   none
 * @retval  none
 * @note    open compare interrupt
 */
void khrtime_event_enable(void)
{
    srt_hal_imx_gptimer_t *sptr_tick;
    kuint32_t flags;

    sptr_tick = IMX_HRTIMER_PORT_ENTRY();

    mr_local_irq_save(flags);
    mr_writel((kuint32_t)(~0U), &sptr_tick->OCR[0]);
    mr_writel(mr_bit(0U), &sptr_tick->SR);
    mr_setbitl(mr_bit(0U), &sptr_tick->IR);
    mr_local_irq_restore(flags);
}

/*!
 * @brief   stop hrtimer
 * @param   none
 * @retval  none
 * @note    close compare interrupt
 */
void khrtime_event_disable(void)
{
    srt_hal_imx_gptimer_t *sptr_tick;
    kuint32_t flags;

    sptr_tick = IMX_HRTIMER_PORT_ENTRY();

    mr_local_irq_save(flags);
    mr_clrbitl(mr_bit(0U), &sptr_tick->IR);
    mr_local_irq_restore(flags);
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
        /*!< 
         * Just clear compare bit 
         * Clear before excuting "do_hrtime_event"
         */
        mr_writel(mr_bit(0U), &sptr_tick->SR);

        /*!< Compare interrupt */
        do_hrtime_event();
    }

    return ER_NORMAL;
}

/*!< -------------------------------------------------------------------------- */
/*!
 * @brief   initial high real-time timer
 * @param   none
 * @retval  none
 * @note    initial Timer
 */
void __init arch_hrtimer_init(void)
{
    imx6ull_hrtimer_init();
}

/* end of file*/
