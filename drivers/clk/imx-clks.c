/*
 * Clock Driver
 *
 * File Name:   imx-clks.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.03.26
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/hal_basic.h>
#include <platform/hal_inode.h>
#include <platform/hal_fs.h>
#include <platform/of/hal_of.h>
#include <platform/of/hal_of_device.h>
#include <platform/hal_platdrv.h>
#include <platform/hal_uaccess.h>

#include <asm/imx6/imx6ull_clocks.h>

/*!< The defines */
struct imx_clks_data
{
    srt_imx_ccm_t *sprt_ccm;
    srt_imx_ccm_pll_t *sprt_pll;
    struct hal_device_node *sprt_clks;
    struct hal_device_node *sprt_anatop;
};

struct clk_desc
{
    void *sprt_data;

    void (*enable) (struct clk_desc *sprt_clk);
    void (*disable) (struct clk_desc *sprt_clk);
    void (*set_rate) (struct clk_desc *sprt_clk);
};

/*!< --------------------------------------------------------------------- */
/*!< device id for device-tree */
static struct hal_of_device_id sgrt_imx_antop_driver_ids[] =
{
	{ .compatible = "fsl,imx6ul-anatop", },
	{},
};

static struct hal_of_device_id sgrt_imx_ccm_driver_ids[] =
{
	{ .compatible = "fsl,imx6ul-ccm", },
	{},
};

static void imx_clks_video_init(struct imx_clks_data *sprt_data)
{
	srt_imx_ccm_t *sprt_ccm;
	srt_imx_ccm_pll_t *sprt_pll;

    sprt_ccm = sprt_data->sprt_ccm;
    sprt_pll = sprt_data->sprt_pll;

    /* PLL5 (PLL_VIDEO), output clock is (OSC24MHz / (PLL_VIDEO[20:19] + (PLL_VIDEO_NUM / PLL_VIDEO_DENOM)) / MISC2[31:30]) */

    /*!<
     * PLL_VIDEO_NUM: Numerator of Video PLL Fractional Loop Divider Register
     *  bit[31:30]: set to zero
     *  bit[29:0]:  30 bit numerator of fractional loop divider(Signed number), absolute value should be less than denominator
     */
    mrt_writel(0, &sprt_pll->PLL_VIDEO_NUM);

    /*!<
     * PLL_VIDEO_DENOM: Denominator of Video PLL Fractional Loop Divider Register
     *  bit[31:30]: set to zero
     *  bit[29:0]:  30 bit Denominator of fractional loop divider.
     */
    mrt_writel(0, &sprt_pll->PLL_VIDEO_DENOM);

	/*!<
     * MISC2: Miscellaneous Register 2
     * bit[31:30]: Post-divider for video. 
     *      The output clock of the video PLL should be gated prior to changing this divider to prevent glitches. 
     *      This divider is feed by PLL_VIDEOn[POST_DIV_SELECT] to achieve division ratios of /1, /2, /4, /8, and /16.
     *      value: 00(1), 01(2), 10(4), 11(8)
     * 
     * here set to 11, divied by 8
	 */
    mrt_clrbitl(mrt_bit_nr(0x3, 30), &sprt_pll->MISC2);
    mrt_setbitl(mrt_bit(31) | mrt_bit(30), &sprt_pll->MISC2);

	/*!<
     * PLL_VIDEO (PLL5): Analog Video PLL control Register
     * bit31:       1: pll is currently locked; 0: pll is not currently locked
     * bit[20:19]:  These bits implement a divider after the PLL, but before the enable and bypass mux. value: 00(4), 01(2), 10(1)
     * bit[13]:     Enalbe PLL output
     * bit[6:0]:    pll loop divider, value: 27 ~ 54
     * 
     * here set bit19 (divied by 2) and bit 13, 
	 */
    mrt_writel(mrt_bit(19) | mrt_bit(13), &sprt_pll->PLL_VIDEO);

    /*!< pll loop divider */
    mrt_clrbitl(0x7f, &sprt_pll->PLL_VIDEO);
    mrt_setbitl(31, &sprt_pll->PLL_VIDEO);

    /*!< check if locked */
    while (mrt_isBitResetl(mrt_bit(31), &sprt_pll->PLL_VIDEO));

	/*!<
     * CSCDR2: CCM Serial Clock Divider Register 2
     * bit[17:15]: Selector for lcdif1 root clock pre-multiplexer
     *      000 derive clock from PLL2
     *      001 derive clock from PLL3 PFD3
     *      010 derive clock from PLL5
     *      011 derive clock from PLL2 PFD0
     *      100 derive clock from PLL2 PFD1
     *      101 derive clock from PLL3 PFD1
     *      110-111 Reserved
     * bit[14:12]: Pre-divider for lcdif1 clock. (NOTE: Divider should be updated when output clock is gated )
     *      000 ~ 111 divide by 1 ~ 8
     * bit[11:9]: Selector for LCDIF1 root clock multiplexer
     *      000 derive clock from divided pre-muxed LCDIF1 clock
     *      001 derive clock from ipp_di0_clk
     *      010 derive clock from ipp_di1_clk
     *      011 derive clock from ldb_di0_clk
     *      100 derive clock from ldb_di1_clk
     * 
     * select PLL5
     */
    mrt_clrbitl(mrt_bit(17) | mrt_bit(16) | mrt_bit(15), &sprt_ccm->CSCDR2);
    mrt_setbitl(mrt_bit(16), &sprt_ccm->CSCDR2);

    /*!<
     * set ratio = PLL_VIDEO[6:0] / CSCDR2[14:12] / CBCMR[25:23] ===> pixclk = OSC24MHz * ratio
     * 
     * if LCD pixclk = 9.2MHz, ===> make it to 9.3MHz = (OSC24MHZ * (31 + 0) / 8) / (5 * 2)
     * set:
     *      PLL_VIDEO[6:0] = 31, CSCDR2[14:12] = 5, CBCMR[25:23] = 2, and MISC2[31:30] = 8
     */

	/*!< Pre-divider for lcdif1 clock */
    mrt_clrbitl(mrt_bit_nr(0x7, 12), &sprt_ccm->CSCDR2);
    mrt_setbitl(mrt_bit_nr(5 - 1, 12), &sprt_ccm->CSCDR2);

	/*!<
     * CBCMR: CCM Bus Clock Multiplexer Register
     * bit[25:23], Post-divider for LCDIF1 clock. value: 1 ~ 8
     */
    mrt_clrbitl(mrt_bit_nr(0x7, 23), &sprt_ccm->CBCMR);
    mrt_setbitl(mrt_bit_nr(2 - 1, 23), &sprt_ccm->CBCMR);

    /*!< select pre-muxed LCDIF1 clock */
    mrt_clrbitl(mrt_bit(11) | mrt_bit(10) | mrt_bit(9), &sprt_ccm->CSCDR2);

    /*!< enbale LCD clock */
    mrt_setbitl(IMX6UL_CCM_CCGR_BIT(14), &sprt_ccm->CCGR2);

    /*!< enable LCD PixClock */
    mrt_setbitl(IMX6UL_CCM_CCGR_BIT(5), &sprt_ccm->CCGR3);
}

/*!
 * @brief   init driver
 * @param   none
 * @retval  errno
 * @note    none
 */
ksint32_t __hal_init imx_clks_driver_init(void)
{
	srt_imx_ccm_t *sprt_ccm;
	srt_imx_ccm_pll_t *sprt_pll;
	struct hal_device_node *sprt_clks, *sprt_anatop;
    struct imx_clks_data sgrt_data;

	sprt_anatop = hal_of_find_matching_node_and_match(mrt_nullptr, sgrt_imx_antop_driver_ids, mrt_nullptr);
    sprt_clks = hal_of_find_matching_node_and_match(mrt_nullptr, sgrt_imx_ccm_driver_ids, mrt_nullptr);
	if (!mrt_isValid(sprt_anatop) || !mrt_isValid(sprt_clks))
	{
		return Ert_isNone;
	}

	sprt_pll = (srt_imx_ccm_pll_t *)hal_of_iomap(sprt_anatop, 0);
    sprt_ccm = (srt_imx_ccm_t *)hal_of_iomap(sprt_clks, 0);
    if (!mrt_isValid(sprt_pll) || !mrt_isValid(sprt_ccm))
    {
        return -Ert_isUnvalid;
    }

    sgrt_data.sprt_ccm = sprt_ccm;
    sgrt_data.sprt_pll = sprt_pll;
    sgrt_data.sprt_clks = sprt_clks;
    sgrt_data.sprt_anatop = sprt_anatop;

/*!< ----------------------------------------------------------------------------------
 * Part of the clock has been initialized in boot, and the rest is supplemented here 
 * ----------------------------------------------------------------------------------*/
    /*!< 1. gpio */
    /*!< gpio5 */
    mrt_setbitl(IMX6UL_CCM_CCGR_BIT(15), &sprt_ccm->CCGR1);

    /*!< 2. lcdif */
    imx_clks_video_init(&sgrt_data);

    return Ert_isWell;
}

/*!
 * @brief   exit driver
 * @param   none
 * @retval  none
 * @note    none
 */
void __hal_exit imx_clks_driver_exit(void)
{

}

IMPORT_LATE_INIT(imx_clks_driver_init);
IMPORT_LATE_EXIT(imx_clks_driver_exit);

/*!< end of file */
