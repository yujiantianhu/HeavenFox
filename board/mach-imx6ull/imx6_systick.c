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
#define IMX_SYSTICK_CLK_CG_REG								CG6
#define IMX_SYSTICK_CLK_SELECT								IMX6UL_CCM_CCGR_CLOCK_ENTRY(1)

/*!< port */
#define IMX_SYSTICK_PORT_ENTRY()                        	IMX6UL_EPIT_PROPERTY_ENTRY(1)

/*!< The globals */
static const struct fwk_of_device_id sgtc_imx_systick_ids[] =
{
    { .compatible = "fsl,imx6ull-epit" },
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
    srt_hal_imx_epit_t *sptr_tick;
    kint32_t irq;
    kint32_t retval;

    sptr_node = fwk_of_find_matching_node_and_match(mr_nullptr, sgtc_imx_systick_ids, mr_nullptr);
    if (!isValid(sptr_node))
        return;

    irq = fwk_of_irq_get(sptr_node, 0);
    if (irq < 0)
        return;

    sptr_tick = IMX_SYSTICK_PORT_ENTRY();

    /*!< enable epit1 clock */
    mr_imx_ccm_clk_enable(IMX_SYSTICK_CLK_CG_REG, IMX_SYSTICK_CLK_SELECT);

    /*!< Clear all field */
    mr_resetl(&sptr_tick->CR);

    /*!<
     * Control register
     * bit24: Select clock source
     *      00: Clock is off
     *      01: Peripheral clock
     *      10: High-frequency reference clock
     *      11: Low-frequency reference clock
     * bit[15:4]: Counter clock prescaler value
     *      0x000: Divide by 1
     *      0x001: Divide by 2...
     *      0xFFF: Divide by 4096
     * bit3: Counter reload control
     *      0: When the counter reaches zero it rolls over to 0xFFFF_FFFF (free-running mode)
     *      1: When the counter reaches zero it reloads from the modulus register (set-and-forget mode)
     * bit2: Output compare interrupt enable (1: enable; 0: disable)
     * bit1: EPIT enable mode
     *      0: Counter starts counting from the value it had when it was disabled.
     *      1: Counter starts count from load value (RLD=1) or 0xFFFF_FFFF (If RLD=0)
     * bit0: This bit enables the EPIT (1: enable; 0: disable)
     * 
     *  Set to Peripheral clock (Freq = 66MHz), reloads from sptr_tick->LR
     */
    mr_writel(mr_bit(24U) | mr_bit(3U) | mr_bit(1U), &sptr_tick->CR);

    /*!<
     * prescaler value: bit[15:4] = 0x000, Divide by 1
     * ===> EPIT1 frequency will be 66MHz / 1 = 66MHz
     */
    mr_setbitl(mr_bit_mask_nr(0U, 0xfffU, 4U), &sptr_tick->CR);

    /*!< Global interface */
    /*!< 66MHz, period = 15.15ns; and it is a decrease counter */
    SYSTICK_INIT(66000000U, IS_TICKCNT_DEC, &sptr_tick->CNR);

    /*!<
     * Load register and Compare register
     * Counter will be "(66000000U / TICK_HZ) ---> 0", and satisfy the CMPR
     * The EPIT1 frequency is 66MHz, if TICH_HZ is 100, the compare period is 10ms
     */
    mr_writel(SYSTICK_FREQ / TICK_HZ, &sptr_tick->LR);
    mr_writel(0U, &sptr_tick->CMPR);

    /*!< Enable Compare IRQ */
    retval = fwk_request_irq(irq, imx6_systick_isr, 0, "imx6-systick", sptr_tick);
    if (!retval)
        mr_setbitl(mr_bit(2U), &sptr_tick->CR);

    /*!< EN: bit0, EPIT1 Enable */
    mr_setbitl(mr_bit(0U), &sptr_tick->CR);
}

/*!
 * @brief   imx6ull_systick irq handler
 * @param   none
 * @retval  none
 * @note    increase jiffies
 */
irq_return_t imx6_systick_isr(kint32_t irq, void *ptrDev)
{
    srt_hal_imx_epit_t *sptr_tick = (srt_hal_imx_epit_t *)ptrDev;
    kuint32_t status;

    status = mr_readl(&sptr_tick->SR);

    /*!< Timer ISR */
    if (mr_isBitSetl(mr_bit(0U), &status))
    {
        /*!< reset jiffies when counter over */
        get_time_counter();
        do_timer_event();
    }

    /*!< Set 1 to clear compare status */
    mr_writel(status, &sptr_tick->SR);
    return ER_NORMAL;
}

/* end of file*/
