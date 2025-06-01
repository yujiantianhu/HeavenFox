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
#include <platform/fwk_basic.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/fwk_platdrv.h>
#include <platform/clk/fwk_clk.h>
#include <platform/clk/fwk_clk_provider.h>
#include <platform/clk/fwk_clk_gate.h>

#include <imx6/imx6ull_clocks.h>

/*!< The defines */
typedef struct fwk_clks srt_fwk_clks_t;

struct imx_clks_data
{
    srt_hal_imx_ccm_t *sptr_ccm;
    srt_hal_imx_ccm_pll_t *sptr_pll;
    struct fwk_device_node *sptr_clks;
    struct fwk_device_node *sptr_anatop;
};

typedef struct imx_clk_gate_fix 
{
    struct fwk_clk_one_cell sgtc_cell;
	const struct fwk_clk_ops *sptr_ops;
	void *reg;

} srt_imx_clk_gate_fix_t;

/*!< The globals */
static struct fwk_clk sgtc_imx_clks_data[IMX6UL_CLK_END];
static struct fwk_clk_one_cell sgtc_imx_clk_one_cell_data;

/*!< device id for device-tree */
static struct fwk_of_device_id sgtc_imx_antop_driver_ids[] =
{
	{ .compatible = "fsl,imx6ul-anatop", },
	{},
};

static struct fwk_of_device_id sgtc_imx_ccm_driver_ids[] =
{
	{ .compatible = "fsl,imx6ul-ccm", },
	{},
};

/*!< API function */
/*!
 * @brief   fill sptr_data
 * @param   sptr_data
 * @retval  none
 * @note    none
 */
static void imx_clk_init_data(srt_imx_clk_gate_fix_t *sptr_data, const struct fwk_clk_ops *sptr_ops, void *reg)
{
    sptr_data->sgtc_cell.sptr_clks = &sgtc_imx_clks_data[0];
    sptr_data->sgtc_cell.clks_size = ARRAY_SIZE(sgtc_imx_clks_data);

	/*!< it is not necessary to update every time */
	if (sptr_ops)
		sptr_data->sptr_ops = sptr_ops;
	
    if (reg)
        sptr_data->reg = reg;
}

/*!
 * @brief   register clk gate
 * @param   sptr_clk, sptr_data
 * @retval  the fwk_clk allocated
 * @note    none
 */
static struct fwk_clk *imx_clk_gate(struct fwk_clk *sptr_clk, const kchar_t *name, 
                            kuint8_t shift, const kchar_t *parent, srt_imx_clk_gate_fix_t *sptr_data)
{
    return fwk_clk_gate_register(sptr_clk, 
                                 sptr_data->sptr_ops,
                                 name,
                                 parent,
                                 sptr_data->reg,
                                 shift);
}

/*!
 * @brief   register clk gate
 * @param   number, sptr_data
 * @retval  errno
 * @note    none
 */
static kint32_t imx_clk_init_gate(kuint32_t number, const kchar_t *name, 
                            kuint8_t shift, const kchar_t *parent, srt_imx_clk_gate_fix_t *sptr_data)
{
    struct fwk_clk *sptr_clk;

    if (number > sptr_data->sgtc_cell.clks_size)
        return -ER_FAULT;

    sptr_clk = &sptr_data->sgtc_cell.sptr_clks[number];
    sptr_clk = imx_clk_gate(sptr_clk, name, shift, parent, sptr_data);

    return isValid(sptr_clk) ? 0 : (-ER_NOMEM);
}

/*!< 
 * PLL5 (PLL_VIDEO), output clock (pixel clock) is :
 *      OSC24MHz * (PLL_VIDEO[6:0] + (PLL_VIDEO_DENOM / PLL_VIDEO_NUM)) / post_divider
 *
 * Examples:
 * --------------------------------------------------------------------------------------------------
 *                  |   PLL_VIDEO[6:0]  |   PLL_VIDEO_DENOM |   PLL_VIDEO_NUM   |   post_divider
 * --------------------------------------------------------------------------------------------------
 *      93MHz       |       31          |           0       |       0           |       8
 * --------------------------------------------------------------------------------------------------
 *      1485MHz     |       61          |           7       |       8          |        1
 * --------------------------------------------------------------------------------------------------
 */
enum __ERT_IMX_VIDEO_PIXEL_CLK
{
    NR_IMX_VIDEO_PCLK_93MHZ = 0,
    NR_IMX_VIDEO_PCLK_1485MHZ,

    NR_IMX_VIDEO_PCLK_NUM,
};

struct imx_clks_video_pclk
{
    kuint32_t pixel_clock;                                  /*!< unit: Hz */

    kuint32_t pll_video;                                    /*!< PLL_VIDEO[6:0] */
    kuint32_t video_denom;                                  /*!< PLL_VIDEO_DENOM */
    kuint32_t video_num;                                    /*!< PLL_VIDEO_NUM */
    kuint32_t post_divider;                                 /*!< post_divider */
};

struct imx_clks_video_pclk sgtc_imx_video_post_divider[NR_IMX_VIDEO_PCLK_NUM] =
{
    [NR_IMX_VIDEO_PCLK_93MHZ]   = { 9200000,    31, 0, 0, 8 },
    [NR_IMX_VIDEO_PCLK_1485MHZ] = { 148500000,  61, 7, 8, 1 },
};

/*!
 * @brief   get suitable divider
 * @param   index: if index is 0 ~ ARRAY_SIZE(sgtc_imx_video_post_divider), return imediately
 * @param   pixel_clock: if index < 0, serach sgtc_imx_video_post_divider and return
 * @retval  &sptr_pclk[idx_satisfy]
 * @note    none
 */
struct imx_clks_video_pclk *imx_clks_get_video_divider(kint32_t index, kuint32_t pixel_clock)
{
    struct imx_clks_video_pclk *sptr_pclk = &sgtc_imx_video_post_divider[0];
    kusize_t num_field = NR_IMX_VIDEO_PCLK_NUM;
    kuint32_t field, last_temp, cur_temp; 
    kuint16_t idx, idx_satisfy;

    if ((index >= 0) && (index < num_field))
        return &sptr_pclk[index];

    last_temp = (kuint16_t)(~0);
    idx_satisfy = last_temp;

    for (idx = 0; idx < num_field; idx++)
    {
        field = sptr_pclk[idx].pixel_clock;
        if (field == pixel_clock)
        {
            idx_satisfy = idx;
            break;
        }

        cur_temp = mr_usub(field, pixel_clock);
        if (cur_temp < last_temp)
        {
            last_temp = cur_temp;
            idx_satisfy = idx;
        }
    }

    if (idx_satisfy < num_field)
        return &sptr_pclk[idx_satisfy];

    return mr_nullptr;
}

/*!
 * @brief   configure pll clock
 * @param   sptr_data
 * @retval  none
 * @note    none
 */
static void imx_clks_video_pixelclk(struct imx_clks_data *sptr_data, struct imx_clks_video_pclk *sptr_pclk)
{
    srt_hal_imx_ccm_pll_t *sptr_pll;
    kuint32_t misc_reg = 0, video_reg = 0;

    sptr_pll = sptr_data->sptr_pll;

    /*!<
     * PLL_VIDEO_NUM: Numerator of Video PLL Fractional Loop Divider Register
     *  bit[31:30]: set to zero
     *  bit[29:0]:  30 bit numerator of fractional loop divider(Signed number), absolute value should be less than denominator
     */
    mr_writel(sptr_pclk->video_num, &sptr_pll->PLL_VIDEO_NUM);

    /*!<
     * PLL_VIDEO_DENOM: Denominator of Video PLL Fractional Loop Divider Register
     *  bit[31:30]: set to zero
     *  bit[29:0]:  30 bit Denominator of fractional loop divider.
     */
    mr_writel(sptr_pclk->video_denom, &sptr_pll->PLL_VIDEO_DENOM);

	/*!<
     * MISC2: Miscellaneous Register 2
     * bit[31:30]: Post-divider for video. 
     *      The output clock of the video PLL should be gated prior to changing this divider to prevent glitches. 
     *      This divider is feed by PLL_VIDEOn[POST_DIV_SELECT] to achieve division ratios of /1, /2, /4, /8, and /16.
     *      value: 00(1), 01(2), 10(4), 11(8)
	 */
    mr_clrbitl(mr_bit_nr(0x3, 30), &sptr_pll->MISC2);

	/*!<
     * PLL_VIDEO (PLL5): Analog Video PLL control Register
     * bit31:       1: pll is currently locked; 0: pll is not currently locked
     * bit[20:19]:  These bits implement a divider after the PLL, but before the enable and bypass mux. value: 00(4), 01(2), 10(1)
     * bit[13]:     Enalbe PLL output
     * bit[6:0]:    pll loop divider, value: 27 ~ 54
	 */
    mr_writel(0, &sptr_pll->PLL_VIDEO);

    /*!
     * @instruction:
     * ------------------------------------------------------------------------
     *      post_divider     |      PLL_VIDEO[20:19]       |    MISC2[31:30]  
     * ------------------------------------------------------------------------
     *           1           |            2                |        0         
     * ------------------------------------------------------------------------
     *           2           |            1                |        0         
     * ------------------------------------------------------------------------
     *           4           |            2                |        3         
     * ------------------------------------------------------------------------
     *           8           |            1                |        3         
     * ------------------------------------------------------------------------
     *           16          |            0                |        3         
     * ------------------------------------------------------------------------
     */
    switch (sptr_pclk->post_divider)
    {
        case 2:
            video_reg = 0x01;
            misc_reg = 0x00;
            break;

        case 4:
            video_reg = 0x02;
            misc_reg = 0x03;
            break;

        case 8:
            video_reg = 0x01;
            misc_reg = 0x03;
            break;

        case 16:
            video_reg = 0x00;
            misc_reg = 0x03;
            break;

        default:
            video_reg = 0x02;
            misc_reg = 0x00;
            break;
    }

    /*!< Enalbe PLL output, and set divider */
    mr_setbitl(mr_bit_nr(misc_reg, 30), &sptr_pll->MISC2);
    mr_setbitl(mr_bit_nr(video_reg, 19) | mr_bit(13), &sptr_pll->PLL_VIDEO);

    /*!< pll loop divider */
    mr_clrbitl(0x7f, &sptr_pll->PLL_VIDEO);
    mr_setbitl(sptr_pclk->pll_video, &sptr_pll->PLL_VIDEO);

    /*!< check if locked */
    while (mr_isBitResetl(mr_bit(31), &sptr_pll->PLL_VIDEO));
}

/*!
 * @brief   initial video clock
 * @param   sptr_data
 * @retval  none
 * @note    none
 */
static void imx_clks_video_init(struct imx_clks_data *sptr_data)
{
	srt_hal_imx_ccm_t *sptr_ccm;
    struct imx_clks_video_pclk *sptr_pclk;

    sptr_ccm = sptr_data->sptr_ccm;

    /*!< Set to 1485MHz (148.5MHz) */
    sptr_pclk = imx_clks_get_video_divider(NR_IMX_VIDEO_PCLK_1485MHZ, 148500000);
    imx_clks_video_pixelclk(sptr_data, sptr_pclk);

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
    mr_clrbitl(mr_bit(17) | mr_bit(16) | mr_bit(15), &sptr_ccm->CSCDR2);
    mr_setbitl(mr_bit(16), &sptr_ccm->CSCDR2);

    /*!<
     * set ratio = PLL_VIDEO[6:0] / CSCDR2[14:12] / CBCMR[25:23] ===> pixclk = OSC24MHz * ratio
     * 
     * if LCD pixclk = 9.2MHz, ===> make it to 9.3MHz = (OSC24MHZ * (31 + 0) / 8) / (5 * 2)
     * set:
     *      PLL_VIDEO[6:0] = 31, CSCDR2[14:12] = 5, CBCMR[25:23] = 2, and MISC2[31:30] = 8
     */

	/*!< Pre-divider for lcdif1 clock */
    mr_clrbitl(mr_bit_nr(0x7, 12), &sptr_ccm->CSCDR2);
    mr_setbitl(mr_bit_nr(5 - 1, 12), &sptr_ccm->CSCDR2);

	/*!<
     * CBCMR: CCM Bus Clock Multiplexer Register
     * bit[25:23], Post-divider for LCDIF1 clock. value: 1 ~ 8
     */
    mr_clrbitl(mr_bit_nr(0x7, 23), &sptr_ccm->CBCMR);
    mr_setbitl(mr_bit_nr(2 - 1, 23), &sptr_ccm->CBCMR);

    /*!< select pre-muxed LCDIF1 clock */
    mr_clrbitl(mr_bit(11) | mr_bit(10) | mr_bit(9), &sptr_ccm->CSCDR2);

    /*!< enbale LCD clock */
//  mr_setbitl(IMX6UL_CCM_CCGR_BIT(14), &sptr_ccm->CCGR2);

    /*!< enable LCD PixClock */
//  mr_setbitl(IMX6UL_CCM_CCGR_BIT(5), &sptr_ccm->CCGR3);
}

/*!
 * @brief   enable clk
 * @param   sptr_hw
 * @retval  errono
 * @note    none
 */
static kint32_t imx_clks_gate_enable(struct fwk_clk_hw *sptr_hw)
{
    struct fwk_clk_gate *sptr_gate;

    sptr_gate = mr_container_of(sptr_hw, struct fwk_clk_gate, sgtc_hw);
    mr_setbitl(IMX6UL_CCM_CCGR_BIT(sptr_gate->bit_idx), sptr_gate->reg);

    return ER_NORMAL;
}

/*!
 * @brief   disable clk
 * @param   sptr_hw
 * @retval  none
 * @note    none
 */
static void	imx_clks_gate_disable(struct fwk_clk_hw *sptr_hw)
{
    struct fwk_clk_gate *sptr_gate;

    sptr_gate = mr_container_of(sptr_hw, struct fwk_clk_gate, sgtc_hw);
    mr_clrbitl(IMX6UL_CCM_CCGR_BIT(sptr_gate->bit_idx), sptr_gate->reg);
}

/*!
 * @brief   get clk status
 * @param   sptr_hw
 * @retval  errono
 * @note    none
 */
static kint32_t imx_clks_gate_is_enabled(struct fwk_clk_hw *sptr_hw)
{
    struct fwk_clk_gate *sptr_gate;
    kuint32_t value;

    sptr_gate = mr_container_of(sptr_hw, struct fwk_clk_gate, sgtc_hw);
    value = mr_getbitl(IMX6UL_CCM_CCGR_BIT(sptr_gate->bit_idx), sptr_gate->reg);

    return !!value;
}

static const struct fwk_clk_ops sgtc_imx_clks_gate_oprts =
{
    .enable = imx_clks_gate_enable,
    .disable = imx_clks_gate_disable,
    .is_enabled = imx_clks_gate_is_enabled,
};

/*!
 * @brief   create clk global data from device_tree
 * @param   sptr_data
 * @retval  errono
 * @note    none
 */
static kint32_t imx_clks_driver_of_init(struct imx_clks_data *sptr_data)
{
    struct fwk_clk *sptr_gclk;
	srt_hal_imx_ccm_t *sptr_ccm;
    struct fwk_device_node *sptr_npccm;
    srt_imx_clk_gate_fix_t sgtc_init;
    kuint32_t ret = 0;

    sptr_ccm = sptr_data->sptr_ccm;
    sptr_npccm = sptr_data->sptr_clks;

    imx_clk_init_data(&sgtc_init, mr_nullptr, mr_nullptr);
    sptr_gclk = sgtc_init.sgtc_cell.sptr_clks;
    if (!sptr_gclk)
        return -ER_NODEV;

    memset(sptr_gclk, 0, sgtc_init.sgtc_cell.clks_size * sizeof(*sptr_gclk));
    memcpy(&sgtc_imx_clk_one_cell_data, &sgtc_init.sgtc_cell, sizeof(sgtc_imx_clk_one_cell_data));
    
    /*!< gate */
    imx_clk_init_data(&sgtc_init, &sgtc_imx_clks_gate_oprts, mr_nullptr);

    /*!< 1. CCGR0 */
    imx_clk_init_data(&sgtc_init, mr_nullptr, &sptr_ccm->CCGR0);

    ret |= imx_clk_init_gate(IMX6UL_CLK_AIPSTZ1, "aips_tz1", NR_IMX_CCGR0_AIPS_TZ1, "ahb", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_AIPSTZ2, "aips_tz2", NR_IMX_CCGR0_AIPS_TZ2, "ahb", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_APBHDMA, "apbh_dma", NR_IMX_CCGR0_APBHDMA, "bch_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_ASRC_IPG, "asrc_ipg", NR_IMX_CCGR0_ASRC_IPG, "ahb", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_ASRC_MEM, "asrc_mem", NR_IMX_CCGR0_ASRC_MEM, "ahb", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_DCP_CLK, "dcp",	NR_IMX_CCGR0_DCP, "ahb", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_ENET, "enet", NR_IMX_CCGR0_ENET, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_ENET_AHB, "enet_ahb", NR_IMX_CCGR0_ENET, "ahb", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_CAN1_IPG, "can1_ipg", NR_IMX_CCGR0_CAN1_IPG, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_CAN1_SERIAL, "can1_serial",	NR_IMX_CCGR0_CAN1_SERIAL, "can_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_CAN2_IPG, "can2_ipg", NR_IMX_CCGR0_CAN2_IPG, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_CAN2_SERIAL, "can2_serial",	NR_IMX_CCGR0_CAN2_SERIAL, "can_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_GPT2_BUS, "gpt_bus", NR_IMX_CCGR0_GPT2_IPG, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_GPT2_SERIAL, "gpt_serial", NR_IMX_CCGR0_GPT2_SERIAL, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_UART2_IPG, "uart2_ipg",	NR_IMX_CCGR0_UART2, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_UART2_SERIAL, "uart2_serial", NR_IMX_CCGR0_UART2, "uart_podf", &sgtc_init);

    /*!< 2. CCGR1 */
    imx_clk_init_data(&sgtc_init, mr_nullptr, &sptr_ccm->CCGR1);

    ret |= imx_clk_init_gate(IMX6UL_CLK_ECSPI1, "ecspi1", NR_IMX_CCGR1_ECSPI1, "ecspi_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_ECSPI2, "ecspi2", NR_IMX_CCGR1_ECSPI2, "ecspi_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_ECSPI3, "ecspi3", NR_IMX_CCGR1_ESCPI3, "ecspi_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_ECSPI4, "ecspi4", NR_IMX_CCGR1_ESCPI4, "ecspi_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_ADC2, "adc2", NR_IMX_CCGR1_ADC2, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_UART3_IPG, "uart3_ipg", NR_IMX_CCGR1_UART3, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_UART3_SERIAL, "uart3_serial", NR_IMX_CCGR1_UART3, "uart_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_EPIT1, "epit1", NR_IMX_CCGR1_EPIT1, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_EPIT2, "epit2", NR_IMX_CCGR1_EPIT2, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_ADC1, "adc1", NR_IMX_CCGR1_ADC1, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_GPT1_BUS, "gpt1_bus", NR_IMX_CCGR1_GPT_BUS, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_GPT1_SERIAL, "gpt1_serial", NR_IMX_CCGR1_GPT_SERIAL, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_UART4_IPG, "uart4_ipg", NR_IMX_CCGR1_UART4, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_UART4_SERIAL, "uart4_serail", NR_IMX_CCGR1_UART4, "uart_podf", &sgtc_init);

    /*!< 3. CCGR2 */
    imx_clk_init_data(&sgtc_init, mr_nullptr, &sptr_ccm->CCGR2);

    ret |= imx_clk_init_gate(IMX6UL_CLK_CSI, "csi", NR_IMX_CCGR2_CSI, "csi_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_I2C1, "i2c1", NR_IMX_CCGR2_I2C1_SERIAL, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_I2C2, "i2c2", NR_IMX_CCGR2_I2C2_SERIAL, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_I2C3, "i2c3", NR_IMX_CCGR2_I2C3_SERIAL, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_OCOTP, "ocotp", NR_IMX_CCGR2_OCOTP_CTRL, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_IOMUXC, "iomuxc", NR_IMX_CCGR2_IOMUXC_IPT, "lcdif_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_LCDIF_APB, "lcdif_apb", NR_IMX_CCGR2_LCD, "axi", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_PXP, "pxp", NR_IMX_CCGR2_PXP, "axi", &sgtc_init);

    /*!< 4. CCGR3 */
    imx_clk_init_data(&sgtc_init, mr_nullptr, &sptr_ccm->CCGR3);

    ret |= imx_clk_init_gate(IMX6UL_CLK_UART5_IPG, "uart5_ipg", NR_IMX_CCGR3_UART5, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_UART5_SERIAL, "uart5_serial", NR_IMX_CCGR3_UART5, "uart_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_EPDC_ACLK, "epdc_aclk", NR_IMX_CCGR3_EPDC, "axi", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_EPDC_PIX, "epdc_pix", NR_IMX_CCGR3_EPDC, "epdc_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_UART6_IPG, "uart6_ipg", NR_IMX_CCGR3_UART6, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_UART6_SERIAL, "uart6_serial", NR_IMX_CCGR3_UART6, "uart_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_LCDIF_PIX, "lcdif_pix", NR_IMX_CCGR3_LCDIF1_PIX, "lcdif_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_QSPI, "qspi1", NR_IMX_CCGR3_QSPI, "qspi1_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_WDOG1, "wdog1", NR_IMX_CCGR3_WDOG1, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_MMDC_P0_FAST, "mmdc_p0_fast", NR_IMX_CCGR3_MMDC_IPG_P0, "mmdc_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_MMDC_P0_IPG, "mmdc_p0_ipg", NR_IMX_CCGR3_MMDC_IPG_P1, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_AXI, "axi", NR_IMX_CCGR3_RAW_GPMI, "axi_podf", &sgtc_init);

    /*!< 5. CCGR4 */
    imx_clk_init_data(&sgtc_init, mr_nullptr, &sptr_ccm->CCGR4);

    ret |= imx_clk_init_gate(IMX6UL_CLK_PER_BCH, "per_bch", NR_IMX_CCGR4_PL301_BCH, "bch_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_PWM1, "pwm1", NR_IMX_CCGR4_PWM1, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_PWM2, "pwm2", NR_IMX_CCGR4_PWM2, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_PWM3, "pwm3", NR_IMX_CCGR4_PWM3, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_PWM4, "pwm4", NR_IMX_CCGR4_PWM4, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_GPMI_BCH_APB, "gpmi_bch_apb", NR_IMX_CCGR4_RAWNAND_BCH_APB, "bch_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_GPMI_BCH, "gpmi_bch", NR_IMX_CCGR4_RAWNAND_GPMI_BCH, "gpmi_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_GPMI_IO, "gpmi_io", NR_IMX_CCGR4_RWNAND_GPMI_IO, "enfc_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_GPMI_APB, "gpmi_apb", NR_IMX_CCGR4_RWNAND_GPMI_APB, "bch_podf", &sgtc_init);

    /*!< CCGR5 */
    imx_clk_init_data(&sgtc_init, mr_nullptr, &sptr_ccm->CCGR5);

    ret |= imx_clk_init_gate(IMX6UL_CLK_ROM, "rom", NR_IMX_CCGR5_ROM, "ahb", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_SDMA, "sdma", NR_IMX_CCGR5_SDMA, "ahb", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_WDOG2, "wdog2", NR_IMX_CCGR5_WDOG2, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_SPBA, "spba", NR_IMX_CCGR5_SPDA, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_SPDIF, "spdif", NR_IMX_CCGR5_SPDIF_AUDIO, "spdif_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_SPDIF_GCLK, "spdif_gclk", NR_IMX_CCGR5_SPDIF_AUDIO, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_SAI3, "sai3", NR_IMX_CCGR5_SAI3, "sai3_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_SAI3_IPG, "sai3_ipg", NR_IMX_CCGR5_SAI3, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_UART1_IPG, "uart1_ipg", NR_IMX_CCGR5_UART1, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_UART1_SERIAL, "uart1_serial", NR_IMX_CCGR5_UART1, "uart_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_UART7_IPG, "uart7_ipg", NR_IMX_CCGR5_UART7, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_UART7_SERIAL, "uart7_serial", NR_IMX_CCGR5_UART7, "uart_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_SAI1, "sai1", NR_IMX_CCGR5_SAI1, "sai1_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_SAI1_IPG, "sai1_ipg", NR_IMX_CCGR5_SAI1, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_SAI2, "sai2", NR_IMX_CCGR5_SAI2, "sai2_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_SAI2_IPG, "sai2_ipg", NR_IMX_CCGR5_SAI2, "ipg", &sgtc_init);

    /*!< CCGR6 */
    imx_clk_init_data(&sgtc_init, mr_nullptr, &sptr_ccm->CCGR6);

    ret |= imx_clk_init_gate(IMX6UL_CLK_USBOH3, "usboh3", NR_IMX_CCGR6_USBOH3, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_USDHC1, "usdhc1", NR_IMX_CCGR6_USDHC1, "usdhc1_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_USDHC2, "usdhc2", NR_IMX_CCGR6_USDHC2, "usdhc2_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_EIM, "eim", NR_IMX_CCGR6_EIM_SLOW, "eim_slow_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_PWM8, "pwm8", NR_IMX_CCGR6_PWM8, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_UART8_IPG, "uart8_ipg", NR_IMX_CCGR6_UART8, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_UART8_SERIAL, "uart8_serial", NR_IMX_CCGR6_UART8, "uart_podf", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_AIPSTZ3, "aips_tz3", NR_IMX_CCGR6_AIPS_TZ3, "ahb", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_WDOG3, "wdog3", NR_IMX_CCGR6_WDOG3, "ipg", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_I2C4, "i2c4", NR_IMX_CCGR6_I2C4_SERIAL, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_PWM5, "pwm5", NR_IMX_CCGR6_PWM5, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_PWM6, "pwm6", NR_IMX_CCGR6_PWM6, "perclk", &sgtc_init);
    ret |= imx_clk_init_gate(IMX6UL_CLK_PWM7, "pwm7", NR_IMX_CCGR6_PWM7, "perclk", &sgtc_init);

    if (ret)
        return -ER_NOMEM;

    /*!< register clock cell */
    return fwk_clk_add_provider(sptr_npccm, fwk_of_clk_src_onecell_get, &sgtc_imx_clk_one_cell_data);
}

/*!
 * @brief   init driver
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init imx_clks_driver_init(void)
{
	srt_hal_imx_ccm_t *sptr_ccm;
	srt_hal_imx_ccm_pll_t *sptr_pll;
	struct fwk_device_node *sptr_clks, *sptr_anatop;
    struct imx_clks_data sgtc_data;

	sptr_anatop = fwk_of_find_matching_node_and_match(mr_nullptr, sgtc_imx_antop_driver_ids, mr_nullptr);
    sptr_clks = fwk_of_find_matching_node_and_match(mr_nullptr, sgtc_imx_ccm_driver_ids, mr_nullptr);
	if (!isValid(sptr_anatop) || !isValid(sptr_clks))
		return -ER_NOTFOUND;

    sptr_pll = (srt_hal_imx_ccm_pll_t *)fwk_of_iomap(sptr_anatop, 0);
	sptr_ccm = (srt_hal_imx_ccm_t *)fwk_of_iomap(sptr_clks, 0);

    if (!isValid(sptr_pll) || !isValid(sptr_ccm))
        return -ER_INVALID;

    sgtc_data.sptr_ccm = sptr_ccm;
    sgtc_data.sptr_pll = sptr_pll;
    sgtc_data.sptr_clks = sptr_clks;
    sgtc_data.sptr_anatop = sptr_anatop;

    imx_clks_driver_of_init(&sgtc_data);

/*!< ----------------------------------------------------------------------------------
 * Part of the clock has been initialized in boot, and the rest is supplemented here 
 * --------------------------------------------------------------------------------- */
    /*!< 1. gpio */
    /*!< gpio1 */
    mr_setbitl(IMX6UL_CCM_CCGR_BIT(13), &sptr_ccm->CCGR1);
    /*!< gpio2 */
    mr_setbitl(IMX6UL_CCM_CCGR_BIT(15), &sptr_ccm->CCGR0);
    /*!< gpio3 */
    mr_setbitl(IMX6UL_CCM_CCGR_BIT(13), &sptr_ccm->CCGR2);
    /*!< gpio4 */
    mr_setbitl(IMX6UL_CCM_CCGR_BIT(6), &sptr_ccm->CCGR3);
    /*!< gpio5 */
    mr_setbitl(IMX6UL_CCM_CCGR_BIT(15), &sptr_ccm->CCGR1);

    /*!< 2. lcdif */
    imx_clks_video_init(&sgtc_data);

    return ER_NORMAL;
}

/*!
 * @brief   exit driver
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit imx_clks_driver_exit(void)
{

}

IMPORT_LATE_INIT(imx_clks_driver_init);
IMPORT_LATE_EXIT(imx_clks_driver_exit);

/*!< end of file */
