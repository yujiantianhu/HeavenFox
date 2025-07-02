/*
 * IMX6ULL Board USDHC Initial
 *
 * File Name:   imx6_sdmmc.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.10.02
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/time.h>
#include <board/board.h>
#include <platform/base/fwk_mempool.h>
#include <platform/mmc/fwk_sdcard.h>

#include "imx6_common.h"
#include <imx6/imx6ull_usdhc.h>

/*!< The defines */
/*!< Mux */
#define IMX_SDMMC_MUX_WP                                IMX6UL_MUX_GPIO1_IO02_USDHC1_WP         /*!< write protection */
#define IMX_SDMMC_MUX_CD                                IMX6UL_MUX_UART1_RTS_B_GPIO1_IO19       /*!< card detect */
#define IMX_SDMMC_MUX_CMD                               IMX6UL_MUX_SD1_CMD_USDHC1_CMD           /*!< command */
#define IMX_SDMMC_MUX_CLK                               IMX6UL_MUX_SD1_CLK_USDHC1_CLK           /*!< clock */
#define IMX_SDMMC_MUX_DATA0                             IMX6UL_MUX_SD1_DATA0_USDHC1_DATA0       /*!< data line 0 */
#define IMX_SDMMC_MUX_DATA1                             IMX6UL_MUX_SD1_DATA1_USDHC1_DATA1       /*!< data line 1 */
#define IMX_SDMMC_MUX_DATA2                             IMX6UL_MUX_SD1_DATA2_USDHC1_DATA2       /*!< data line 2 */
#define IMX_SDMMC_MUX_DATA3                             IMX6UL_MUX_SD1_DATA3_USDHC1_DATA3       /*!< data line 3 */

/*!< CCM */
#define IMX_SDMMC_CLK_CG_REG							CG1
#define IMX_SDMMC_CLK_SELECT							IMX6UL_CCM_CCGR_CLOCK_ENTRY(6)
#define IMX_SDMMC_CD_CLK_CG_REG							CG13
#define IMX_SDMMC_CD_CLK_SELECT							IMX6UL_CCM_CCGR_CLOCK_ENTRY(1)
#define IMX_SDMMC_CLK_PORT_ENTRY()                      IMX6UL_CCM_PROPERTY_ENTRY()             /*!< register base address */

/*!< Pin */
#define IMX_SDMMC_CD_PORT_ENTRY()                       IMX6UL_GPIO_PROPERTY_ENTRY(1)
#define IMX_SDMMC_CD_PIN_BIT                            IMX6UL_GPIO_PIN_OFFSET_BIT(19)
#define IMX_SDMMC_IF_PORT_ENTRY()                       IMX6UL_USDHC_PROPERTY_ENTRY(1)          /*!< register base address */

/*!< The functions */
static kbool_t imx6ull_sdmmc_is_card_insert(struct fwk_sdcard_host *sptr_host);
static void imx6ull_sdmmc_set_bus_width(struct fwk_sdcard_host *sptr_host, kuint32_t option);
static void imx6ull_sdmmc_set_clk_freq(struct fwk_sdcard_host *sptr_host, kuint32_t option);
static kbool_t imx6ull_sdmmc_initial_active(struct fwk_sdcard_host *sptr_host, kuint32_t timeout);
static kint32_t imx6ull_sdmmc_switch_voltage(struct fwk_sdcard_host *sptr_host, kuint32_t voltage);
static kint32_t imx6ull_sdmmc_send_command(struct fwk_sdcard_cmd *sptr_cmds);
static void imx6ull_sdmmc_recv_response(struct fwk_sdcard_cmd *sptr_cmds);
static kint32_t imx6ull_sdmmc_transfer_data(struct fwk_sdcard_data *sptr_data);
static void imx6ull_sdmmc_reset_transfer(struct fwk_sdcard_host *sptr_host);

/*!< private function */
static void imx6ull_sdmmc_reset(srt_imx_usdhc_t *sptr_usdhc, kuint32_t optBit, kuint32_t timeout);
static void imx6ull_sdmmc_data_configure(srt_imx_usdhc_t *sptr_usdhc, struct fwk_sdcard_data *sptr_data, void *ptrData);
static kint32_t imx6ull_sdmmc_write_data(srt_imx_usdhc_t *sptr_usdhc, struct fwk_sdcard_data *sptr_data);
static kint32_t imx6ull_sdmmc_read_data(srt_imx_usdhc_t *sptr_usdhc, struct fwk_sdcard_data *sptr_data);

/*!< API function */
/*!
 * @brief   imx6ull_sdmmc_clk_initial
 * @param   none
 * @retval  none
 * @note    initial USDHC clk for SD Card
 */
static void imx6ull_sdmmc_clk_initial(void)
{
    srt_hal_imx_ccm_t *sptr_clk;

    sptr_clk = IMX_SDMMC_CLK_PORT_ENTRY();

    /*!<
     * The clock of USDHC is from two PFDs(Phase Fractional Dividers): PFD0(352MHz), PFD2(396MHz), which will output to USDHC after dividing 2.
     * The register CSCMR1 is used to choose PFD0 or PFD2; Usually, PFD2 will be first.
     * The register CSCDR1 is used to configure dividing level, generally 2
     * 
     * clock = PFD2 / 2 = 192MHz
     */
    mr_clrbitl(mr_bit(16), &sptr_clk->CSCMR1);
    mr_clrbitl(mr_bit(11) | mr_bit(12) | mr_bit(13), &sptr_clk->CSCDR1);
    mr_setbitl(mr_bit(11), &sptr_clk->CSCDR1);

    /*!< enable clock */
    mr_imx_ccm_clk_enable(IMX_SDMMC_CLK_CG_REG, IMX_SDMMC_CLK_SELECT);

    /*!< enable cd pin clock */
    mr_imx_ccm_clk_enable(IMX_SDMMC_CD_CLK_CG_REG, IMX_SDMMC_CD_CLK_SELECT);
}

/*!
 * @brief   imx6ull_sdmmc_pin_initial
 * @param   none
 * @retval  none
 * @note    initial USDHC Pin for SD Card
 */
static void imx6ull_sdmmc_pin_initial(void)
{
    srt_hal_imx_pin_t sgtc_uSDHC;
    urt_imx_io_ctl_pad_t ugtr_ioPad;

    /*!< Card detect pin, input direction, 47k pull up, enable HYS */
    mr_reset_urt_data(&ugtr_ioPad);
    mr_write_urt_bits(&ugtr_ioPad, HYS, IMX6UL_IO_CTL_PAD_HYS_ENABLE);
    mr_write_urt_bits(&ugtr_ioPad, PUS, IMX6UL_IO_CTL_PAD_PUS_47K_UP);
    mr_write_urt_bits(&ugtr_ioPad, PUE, IMX6UL_IO_CTL_PAD_PUE_PULL);
    mr_write_urt_bits(&ugtr_ioPad, PKE, IMX6UL_IO_CTL_PAD_PKE_DISABLE);
    mr_write_urt_bits(&ugtr_ioPad, DSE, IMX6UL_IO_CTL_PAD_DSE_RDIV(4));
    mr_write_urt_bits(&ugtr_ioPad, SRE, IMX6UL_IO_CTL_PAD_FAST_RATE);
    mr_write_urt_bits(&ugtr_ioPad, SPEED, IMX6UL_IO_CTL_PAD_SPEED_200MHZ);
    hal_imx_pin_attribute_init(&sgtc_uSDHC, IMX6UL_PIN_ADDR_BASE, 
                        IMX_SDMMC_MUX_CD, mr_trans_urt_data(&ugtr_ioPad), IMX6UL_PIN_MUX_FUNC_DISABLE);
    hal_imx_pin_mux_configure(&sgtc_uSDHC);
    hal_imx_pin_pad_configure(&sgtc_uSDHC);

    /*!< Clock pin */
    mr_write_urt_bits(&ugtr_ioPad, HYS, IMX6UL_IO_CTL_PAD_HYS_DISABLE);
    mr_write_urt_bits(&ugtr_ioPad, PUS, IMX6UL_IO_CTL_PAD_PUS_22K_UP);
    mr_write_urt_bits(&ugtr_ioPad, PUE, IMX6UL_IO_CTL_PAD_PUE_PULL);
    mr_write_urt_bits(&ugtr_ioPad, PKE, IMX6UL_IO_CTL_PAD_PKE_DISABLE);
    mr_write_urt_bits(&ugtr_ioPad, DSE, IMX6UL_IO_CTL_PAD_DSE_RDIV(4));
    mr_write_urt_bits(&ugtr_ioPad, SRE, IMX6UL_IO_CTL_PAD_FAST_RATE);
    mr_write_urt_bits(&ugtr_ioPad, SPEED, IMX6UL_IO_CTL_PAD_SPEED_100MHZ);
    hal_imx_pin_attribute_init(&sgtc_uSDHC, IMX6UL_PIN_ADDR_BASE, 
                        IMX_SDMMC_MUX_CLK, mr_trans_urt_data(&ugtr_ioPad), IMX6UL_PIN_MUX_FUNC_DISABLE);
    hal_imx_pin_mux_configure(&sgtc_uSDHC);
    hal_imx_pin_pad_configure(&sgtc_uSDHC);

    /*!< Common Settings */
    mr_reset_urt_data(&ugtr_ioPad);
    mr_write_urt_bits(&ugtr_ioPad, HYS, IMX6UL_IO_CTL_PAD_HYS_DISABLE);
    mr_write_urt_bits(&ugtr_ioPad, PUS, IMX6UL_IO_CTL_PAD_PUS_22K_UP);
    mr_write_urt_bits(&ugtr_ioPad, PUE, IMX6UL_IO_CTL_PAD_PUE_PULL);
    mr_write_urt_bits(&ugtr_ioPad, PKE, IMX6UL_IO_CTL_PAD_PKE_DISABLE);
    mr_write_urt_bits(&ugtr_ioPad, DSE, IMX6UL_IO_CTL_PAD_DSE_RDIV(4));
    mr_write_urt_bits(&ugtr_ioPad, SRE, IMX6UL_IO_CTL_PAD_FAST_RATE);

    /*!< Command pin */
    mr_write_urt_bits(&ugtr_ioPad, SPEED, IMX6UL_IO_CTL_PAD_SPEED_100MHZ);
    hal_imx_pin_attribute_init(&sgtc_uSDHC, IMX6UL_PIN_ADDR_BASE, 
                        IMX_SDMMC_MUX_CMD, mr_trans_urt_data(&ugtr_ioPad), IMX6UL_PIN_MUX_FUNC_DISABLE);
    hal_imx_pin_mux_configure(&sgtc_uSDHC);
    hal_imx_pin_pad_configure(&sgtc_uSDHC);

    /*!< Data pin 0 ~ 3 */
    mr_write_urt_bits(&ugtr_ioPad, SPEED, IMX6UL_IO_CTL_PAD_SPEED_100MHZ);

    /*!< Data0 pin */
    hal_imx_pin_attribute_init(&sgtc_uSDHC, IMX6UL_PIN_ADDR_BASE, 
                        IMX_SDMMC_MUX_DATA0, mr_trans_urt_data(&ugtr_ioPad), IMX6UL_PIN_MUX_FUNC_DISABLE);
    hal_imx_pin_mux_configure(&sgtc_uSDHC);
    hal_imx_pin_pad_configure(&sgtc_uSDHC);

    /*!< Data1 pin */
    hal_imx_pin_attribute_init(&sgtc_uSDHC, IMX6UL_PIN_ADDR_BASE, 
                        IMX_SDMMC_MUX_DATA1, mr_trans_urt_data(&ugtr_ioPad), IMX6UL_PIN_MUX_FUNC_DISABLE);
    hal_imx_pin_mux_configure(&sgtc_uSDHC);
    hal_imx_pin_pad_configure(&sgtc_uSDHC);

    /*!< Data2 pin */
    hal_imx_pin_attribute_init(&sgtc_uSDHC, IMX6UL_PIN_ADDR_BASE, 
                        IMX_SDMMC_MUX_DATA2, mr_trans_urt_data(&ugtr_ioPad), IMX6UL_PIN_MUX_FUNC_DISABLE);
    hal_imx_pin_mux_configure(&sgtc_uSDHC);
    hal_imx_pin_pad_configure(&sgtc_uSDHC);

    /*!< Data3 pin */
    hal_imx_pin_attribute_init(&sgtc_uSDHC, IMX6UL_PIN_ADDR_BASE, 
                        IMX_SDMMC_MUX_DATA3, mr_trans_urt_data(&ugtr_ioPad), IMX6UL_PIN_MUX_FUNC_DISABLE);
    hal_imx_pin_mux_configure(&sgtc_uSDHC);
    hal_imx_pin_pad_configure(&sgtc_uSDHC);
}

/*!
 * @brief   imx6ull_sdmmc_host_initial
 * @param   none
 * @retval  none
 * @note    initial USDHC func for SD Card
 */
static void imx6ull_sdmmc_host_initial(void)
{
    srt_hal_imx_gpio_t  *sptr_cd;
    srt_imx_usdhc_t *sptr_usdhc;
    kuint32_t iSysCtrlReg, iWtmkLvlReg, iProtCtrlReg, iIntStatusReg;

    sptr_cd    = IMX_SDMMC_CD_PORT_ENTRY();
    sptr_usdhc = IMX_SDMMC_IF_PORT_ENTRY();

    /*!< set to input direction */
    mr_clrbitl(IMX_SDMMC_CD_PIN_BIT, &sptr_cd->GDIR);

    /*!< ------------------------------------------------------------------------------- */
    /*!< Reset all */
    mr_setbitl(NR_ImxUsdhc_SysCtrl_SoftResetAll, &sptr_usdhc->SYS_CTRL);
    while (mr_isBitSetl(NR_ImxUsdhc_SysCtrl_SoftResetAll, &sptr_usdhc->SYS_CTRL));

    /*!< Reset Tuning */
    mr_setbitl(NR_ImxUsdhc_SysCtrl_ResetTuning, &sptr_usdhc->SYS_CTRL);
    while (mr_isBitSetl(NR_ImxUsdhc_SysCtrl_ResetTuning, &sptr_usdhc->SYS_CTRL));

    /*!< Read Registers */
    iSysCtrlReg = mr_readl(&sptr_usdhc->SYS_CTRL);

    /*!< Data Timeout Counter Value */
    mr_clrbitl(IMX_USDHC_SYS_CTRL_DTOCV_MASK, &iSysCtrlReg);
    mr_setbitl(IMX_USDHC_SYS_CTRL_DTOCV_U32(0xfU), &iSysCtrlReg);

    /*!< Update Registers */
    mr_writel(iSysCtrlReg,  &sptr_usdhc->SYS_CTRL);

    /*!< ------------------------------------------------------------------------------- */
    /*!< Read Registers */
    iWtmkLvlReg = mr_readl(&sptr_usdhc->WTMK_LVL);

    /*!<
     * Read Watermark Level
     * The number of words used as the watermark level (FIFO threshold) in a DMA read operation. Also the
     * number of words as a sequence of read bursts in back-to-back mode. The maximum legal value for the
     * read water mark level is 128
     */
    mr_clrbitl(IMX_USDHC_WTMK_LVL_RD_WML_MASK, &iWtmkLvlReg);
    mr_setbitl(IMX_USDHC_WTMK_LVL_RD_WML_U32(0x80U), &iWtmkLvlReg);

    /*!<
     * Read Burst Length
     * The number of words the uSDHC reads in a single burst. The read burst length must be less than or equal
     * to the read watermark level, and all bursts within a watermark level transfer will be in back-to-back mode.
     * On reset, this field will be 8. Writing 0 to this field will result in '01000' (i.e. it is not able to clear this field)
     */
    mr_clrbitl(IMX_USDHC_WTMK_LVL_RD_BRST_LEN_MASK, &iWtmkLvlReg);
    mr_setbitl(IMX_USDHC_WTMK_LVL_RD_BRST_LEN_U32(0x08U), &iWtmkLvlReg);

    /*!<
     * Write Watermark Level
     * The number of words used as the watermark level (FIFO threshold) in a DMA write operation. Also the
     * number of words as a sequence of write bursts in back-to-back mode. The maximum legal value for the
     * write watermark level is 128
     */
    mr_clrbitl(IMX_USDHC_WTMK_LVL_WR_WML_MASK, &iWtmkLvlReg);
    mr_setbitl(IMX_USDHC_WTMK_LVL_WR_WML_U32(0x80U), &iWtmkLvlReg);

    /*!<
     * Write Burst Length
     * The number of words the uSDHC writes in a single burst. The write burst length must be less than or
     * equal to the write watermark level, and all bursts within a watermark level transfer will be in back-to-back
     * mode. On reset, this field will be 8. Writing 0 to this field will result in '01000' (i.e. it is not able to clear this
     * field).
     */
    mr_clrbitl(IMX_USDHC_WTMK_LVL_WR_BRST_LEN_MASK, &iWtmkLvlReg);
    mr_setbitl(IMX_USDHC_WTMK_LVL_WR_BRST_LEN_U32(0x08U), &iWtmkLvlReg);

    /*!< Update Registers */
    mr_writel(iWtmkLvlReg,  &sptr_usdhc->WTMK_LVL);
    
    /*!< ------------------------------------------------------------------------------- */
    /*!< Read Registers */
    iProtCtrlReg = mr_readl(&sptr_usdhc->PROT_CTRL);

    /*!< Endian Mode */
    mr_clrbitl(IMX_USDHC_PROT_CTRL_EMODE_MASK, &iProtCtrlReg);
    mr_setbitl(NR_ImxUsdhc_ProtCtrl_LittleEndian, &iProtCtrlReg);

    /*!< select which DMA (SDMA or ADMA) will be enabled */
    mr_clrbitl(IMX_USDHC_PROT_CTRL_DMASEL_MASK, &iProtCtrlReg);
    mr_setbitl(NR_ImxUsdhc_ProtCtrl_SDmaSelect, &iProtCtrlReg);

    /*!< Update Registers */
    mr_writel(iProtCtrlReg, &sptr_usdhc->PROT_CTRL);

    /*!< ------------------------------------------------------------------------------- */
    /*!<
     * External DMA Request Enable
     *  0: In any scenario, uSDHC does not send out external DMA request.
     *  1: When internal DMA is not active, the external DMA request will be sent out.
     */
    mr_clrbitl(NR_ImxUsdhc_VendSpec_ExtDmaRequestEnable, &sptr_usdhc->VEND_SPEC);

    /*!< ------------------------------------------------------------------------------- */
    /*!< DMA Enable */
    mr_clrbitl(NR_ImxUsdhc_MixCtrl_DmaEnable, &sptr_usdhc->MIX_CTRL);

    /*!< ------------------------------------------------------------------------------- */
    /*!< Read Registers */
    iIntStatusReg = mr_readl(&sptr_usdhc->INT_STATUS_EN);

    /*!< Disable interrupt signal, but enable interrupt status  */
    mr_resetl(&iIntStatusReg);

    /*!< Select Interrupt Status Bit to Enable */
    /*!< Command Status Bit: Timeout Error, CRC Error, End Bit Error, Index Error */
    mr_setbitl(NR_ImxUsdhc_IntCmdTimeOutErr_Bit | NR_ImxUsdhc_IntCmdCrcErr_Bit | 
               NR_ImxUsdhc_IntCmdEndBitErr_Bit | NR_ImxUsdhc_IntCmdIndexErr_Bit, &iIntStatusReg);
    /*!< Card Insertion & Removal Status Bit */
    mr_setbitl(NR_ImxUsdhc_IntCardInsertion_Bit | NR_ImxUsdhc_IntCardRemoval_Bit, &iIntStatusReg);
    /*!< Status Bit: Cmd Transfer Complete, Data Transfer Complete, DMA Interrupt, Buffer Write Ready, Buffer Read Ready */
    mr_setbitl(NR_ImxUsdhc_IntCmdComplete_Bit | NR_ImxUsdhc_IntDataComplete_Bit | 
               NR_ImxUsdhc_IntDmaInterrupt_Bit | NR_ImxUsdhc_IntBufferWriteReady_Bit | 
               NR_ImxUsdhc_IntBufferReadReady_Bit, &iIntStatusReg);
    /*!< Data Status Bit: Data Timeout Error, Data CRC Error, Data End Bit Error, Auto CMD12 Error */
    mr_setbitl(NR_ImxUsdhc_IntDataTimeOutErr_Bit | NR_ImxUsdhc_IntDataCrcErr_Bit |
               NR_ImxUsdhc_IntDataEndBitErr_Bit | NR_ImxUsdhc_IntACmd12Err_Bit, &iIntStatusReg);
    /*!< SDR104 Tuning Status Bit: Re-Tuning Event, Tuning Pass, Tuning Error */
    mr_setbitl(NR_ImxUsdhc_IntReTuningEvent_Bit | NR_ImxUsdhc_IntTuningPass_Bit |
               NR_ImxUsdhc_IntTuningErr_Bit, &iIntStatusReg);

    /*!< Update Registers */
    mr_writel(iIntStatusReg, &sptr_usdhc->INT_STATUS_EN);
    mr_resetl(&sptr_usdhc->INT_SIGNAL_EN);
}

/*!
 * @brief   imx6ull_sdmmc_init
 * @param   none
 * @retval  none
 * @note    initial USDHC for SD Card
 */
void imx6ull_sdmmc_init(void)
{
    imx6ull_sdmmc_clk_initial();
    imx6ull_sdmmc_pin_initial();
    imx6ull_sdmmc_host_initial();
}

/*!< ------------------------------------------------------------------------- */
/*!
 * @brief   imx6ull_sdmmc_reset_transfer
 * @param   none
 * @retval  none
 * @note    reset command/data line
 */
static void imx6ull_sdmmc_reset(srt_imx_usdhc_t *sptr_usdhc, kuint32_t optBit, kuint32_t timeout)
{
    mr_setbitl(optBit & NR_ImxUsdhc_SysCtrl_ResetMask, &sptr_usdhc->SYS_CTRL);

    mr_run_code_retry(timeout,

        if (mr_isBitResetl(optBit & NR_ImxUsdhc_SysCtrl_ResetMask, &sptr_usdhc->SYS_CTRL))
            break;
    )

}

/*!
 * @brief   imx6ull_sdmmc_reset_transfer
 * @param   none
 * @retval  none
 * @note    reset command/data line
 */
static void imx6ull_sdmmc_reset_transfer(struct fwk_sdcard_host *sptr_host)
{
    srt_imx_usdhc_t *sptr_usdhc = (srt_imx_usdhc_t *)sptr_host->iHostIfBase;

    /*!<
     * CIHB: bit0, Command Inhibit (CMD). 0: idle; 1 : busy
     * CDIHB: bit1, Command Inhibit (DATA). 0: idle; 1 : busy
     */
    if (mr_isBitSetl(NR_ImxUsdhc_PresState_CmdInhibitCmdLine, &sptr_usdhc->PRES_STATE))
        imx6ull_sdmmc_reset(sptr_usdhc, NR_ImxUsdhc_SysCtrl_SoftResetCmdLine, 100U);

    if (mr_isBitSetl(NR_ImxUsdhc_PresState_CmdInhibitDataLine, &sptr_usdhc->PRES_STATE))
        imx6ull_sdmmc_reset(sptr_usdhc, NR_ImxUsdhc_SysCtrl_SoftResetDataLine, 100U); 
}

/*!
 * @brief   imx6ull_sdmmc_is_card_insert
 * @param   none
 * @retval  none
 * @note    detect if card insert
 */
static kbool_t imx6ull_sdmmc_is_card_insert(struct fwk_sdcard_host *sptr_host)
{
    srt_hal_imx_gpio_t *sptr_cd = (srt_hal_imx_gpio_t *)sptr_host->iHostCDBase;

    /*!< Waitting for Card Inserting */
    return mr_isBitResetl(IMX_SDMMC_CD_PIN_BIT, &sptr_cd->DR);
}

/*!
 * @brief   imx6ull_sdmmc_set_bus_width
 * @param   none
 * @retval  none
 * @note    configure card bus width
 */
static void imx6ull_sdmmc_set_bus_width(struct fwk_sdcard_host *sptr_host, kuint32_t option)
{
    srt_imx_usdhc_t *sptr_usdhc = (srt_imx_usdhc_t *)sptr_host->iHostIfBase;

    if (option > NR_SdCard_BusWidth_8Bit)
        option = NR_SdCard_BusWidth_4Bit;

    /*!< reset command line and data line */
    imx6ull_sdmmc_reset_transfer(sptr_host);

    /*!< clear all interrupt flags */
    mr_imx_clear_interrupt_flags(NR_ImxUsdhc_IntAllInterrupt_Bit, &sptr_usdhc);
    while (!mr_isBitResetl(NR_ImxUsdhc_IntAllInterrupt_Bit, &sptr_usdhc->INT_STATUS));

    /*!< set DATA bus width */
    mr_clrbitl(IMX_USDHC_PROT_CTRL_DTW_MASK, &sptr_usdhc->PROT_CTRL);
    mr_setbitl(IMX_USDHC_PROT_CTRL_DTW_BIT(option), &sptr_usdhc->PROT_CTRL);
}

/*!
 * @brief   imx6ull_sdmmc_set_clk_freq
 * @param   none
 * @retval  none
 * @note    configure sdcard clock frequency
 */
static void imx6ull_sdmmc_set_clk_freq(struct fwk_sdcard_host *sptr_host, kuint32_t option)
{
    srt_imx_usdhc_t *sptr_usdhc = (srt_imx_usdhc_t *)sptr_host->iHostIfBase;
    kuint32_t bitFreq, preScaler = 256U, divisor = 16U;
    kuint32_t freqTimes2, sys_ctl;

    if (option > NR_SdCard_ClkFreq_50MHz)
        option = NR_SdCard_ClkFreq_25MHz;

    /*!< 
     * Dual Data Rate mode selection
     * check if freqTimes2 is 1:
     *  1: Dual Data Rate mode. Max dividers is 512
     *  0: Single Data Rate mode. Max dividers is 256
     */
    freqTimes2 = mr_isBitSetl(NR_ImxUsdhc_MixCtrl_DualDataRateMode, &sptr_usdhc->MIX_CTRL);

    /*!<
     * uSDHC Frequency is 192MHz
     * if sdclk = 400KHz, divide = 192MHz / 400KHz = 480 = 32 * 15
     * if sdclk = 25MHz,  divide = 192MHz / 24MHz  = 8   = 8  * 1
     * if sdclk = 50MHz,  divide = 192MHz / 48MHz  = 4   = 4  * 1
     */
    switch (option)
    {
        case NR_SdCard_ClkFreq_400KHz:
            preScaler = IMX_USDHC_SYS_CTRL_SDCLKFS_U32(freqTimes2, 32U);
            divisor   = IMX_USDHC_SYS_CTRL_DVS_U32(15U);
            break;

        case NR_SdCard_ClkFreq_25MHz:
            preScaler = IMX_USDHC_SYS_CTRL_SDCLKFS_U32(freqTimes2, 8U);
            divisor   = IMX_USDHC_SYS_CTRL_DVS_U32(1U);
            break;

        case NR_SdCard_ClkFreq_50MHz:
            preScaler = IMX_USDHC_SYS_CTRL_SDCLKFS_U32(freqTimes2, 4U);
            divisor   = IMX_USDHC_SYS_CTRL_DVS_U32(1U);
            break;
    }

    bitFreq = preScaler + divisor;
    sys_ctl = mr_readl(&sptr_usdhc->SYS_CTRL);

    /*!< Update Divisor and SDCLK */
    mr_clrbitl(IMX_USDHC_SYS_CTRL_DVS_MASK | IMX_USDHC_SYS_CTRL_SDCLKFS_MASK, &sys_ctl);
    mr_setbitl(bitFreq, &sys_ctl);
    mr_writel(sys_ctl, &sptr_usdhc->SYS_CTRL);

    /*!< make sure that SDSTB is true: SD clock is stable */
    while (!mr_isBitSetl(NR_ImxUsdhc_PresState_SDClockStable, &sptr_usdhc->PRES_STATE));
}

/*!
 * @brief   imx6ull_sdmmc_initial_active
 * @param   none
 * @retval  none
 * @note    initial active card; it will make card enter idle status
 */
static kbool_t imx6ull_sdmmc_initial_active(struct fwk_sdcard_host *sptr_host, kuint32_t timeout)
{
    srt_imx_usdhc_t *sptr_usdhc = (srt_imx_usdhc_t *)sptr_host->iHostIfBase;

    /*!<
     * When this bit is set, 80 SD-Clocks are sent to the card. After the 80 clocks are sent, this bit is self cleared.
     * This bit is very useful during the card power-up period when 74 SD-Clocks are needed and the clock auto
     * gating feature is enabled. Writing 1 to this bit when this bit is already 1 has no effect. Writing 0 to this bit at
     * any time has no effect. When either of the CIHB and CDIHB bits in the Present State Register are set,
     * writing 1 to this bit is ignored (i.e. when command line or data lines are active, write to this bit is not
     * allowed). On the otherhand, when this bit is set, i.e., during intialization active period, it is allowed to issue
     * command, and the command bit stream will appear on the CMD pad after all 80 clock cycles are done. So
     * when this command ends, the driver can make sure the 80 clock cycles are sent out. This is very useful
     * when the driver needs send 80 cycles to the card and does not want to wait till this bit is self cleared.
     */
    mr_setbitl(NR_ImxUsdhc_SysCtrl_InitialActive, &sptr_usdhc->SYS_CTRL);

    /*!< wait for 74 clk at less */
    while (mr_isBitSetl(NR_ImxUsdhc_SysCtrl_InitialActive, &sptr_usdhc->SYS_CTRL))
    {
        if (!(timeout--))
        {
            break;
        }
    }

    return mr_to_kbool(timeout);
}

/*!
 * @brief   imx6ull_sdmmc_switch_voltage
 * @param   none
 * @retval  none
 * @note    switch voltage by uSDHC
 */
static kint32_t imx6ull_sdmmc_switch_voltage(struct fwk_sdcard_host *sptr_host, kuint32_t voltage)
{
    srt_imx_usdhc_t *sptr_usdhc = (srt_imx_usdhc_t *)sptr_host->iHostIfBase;

    kbool_t blRetval;

    /* check data line and cmd line status */
    /*!< all status should be "0" before switching */
    blRetval = mr_isBitResetl(NR_ImxUsdhc_PresState_Data0LineLevel | NR_ImxUsdhc_PresState_Data1LineLevel |
                             NR_ImxUsdhc_PresState_Data2LineLevel | NR_ImxUsdhc_PresState_Data3LineLevel, 
                             &sptr_usdhc->PRES_STATE);
    if (!blRetval)
        return -ER_NREADY;

    /*!< switch to "voltage" */
    if (NR_SdCard_toVoltage1_8V == voltage)
        mr_setbitl(NR_ImxUsdhc_VendSpec_VoltageSelect, &sptr_usdhc->VEND_SPEC);
    else
        mr_clrbitl(NR_ImxUsdhc_VendSpec_VoltageSelect, &sptr_usdhc->VEND_SPEC);

    delay_ms(100U);

    /*!< enable force clock on */
    mr_setbitl(NR_ImxUsdhc_VendSpec_ActiveClk, &sptr_usdhc->VEND_SPEC);
    delay_ms(10U);

    /*!< disable force clock on */
    mr_clrbitl(NR_ImxUsdhc_VendSpec_ActiveClk, &sptr_usdhc->VEND_SPEC);

    /* check data line and cmd line status */
    /*!< at least one of the status should be "1" after switching */
    blRetval = mr_isBitResetl(NR_ImxUsdhc_PresState_Data0LineLevel | NR_ImxUsdhc_PresState_Data1LineLevel |
                             NR_ImxUsdhc_PresState_Data2LineLevel | NR_ImxUsdhc_PresState_Data3LineLevel, 
                             &sptr_usdhc->PRES_STATE);
    if (blRetval)
        return -ER_FAILD;

    return ER_NORMAL;
}

/*!
 * @brief   imx6ull_sdmmc_send_command
 * @param   none
 * @retval  none
 * @note    send command by uSDHC
 */
static kint32_t imx6ull_sdmmc_send_command(struct fwk_sdcard_cmd *sptr_cmds)
{
    srt_imx_usdhc_t *sptr_usdhc;
    struct fwk_sdcard_host *sptr_host;
    kuint32_t iCmdXfrTypReg;
    kuint8_t index;
    kuint32_t argument;
    kint32_t iRetval;

    sptr_host = (struct fwk_sdcard_host *)sptr_cmds->ptrHost;
    if (!isValid(sptr_host))
        return -ER_NULLPTR;

    sptr_usdhc = (srt_imx_usdhc_t *)sptr_host->iHostIfBase;

    index = sptr_cmds->index & 0x3fU;
    argument = mr_be32_to_cpu(sptr_cmds->args);

    /*!< Wait until command/data bus out of busy status */
    while (mr_isBitSetl(NR_ImxUsdhc_PresState_CmdInhibitCmdLine, &sptr_usdhc->PRES_STATE))
    {
        /*!< do nothing */
    }

    /*!< check re-tuning request */
    if (mr_isBitSetl(NR_ImxUsdhc_IntReTuningEvent_Bit, &sptr_usdhc->INT_STATUS))
    {
        mr_imx_clear_interrupt_flags(NR_ImxUsdhc_IntReTuningEvent_Bit, &sptr_usdhc);
        return -ER_NREADY;
    }

    /*!< Read Registers */
    iCmdXfrTypReg = mr_readl(&sptr_usdhc->CMD_XFR_TYP);

    /*!< configure data registers before command configure */
    imx6ull_sdmmc_data_configure(sptr_usdhc, (struct fwk_sdcard_data *)sptr_cmds->ptrData, &iCmdXfrTypReg);

    /*!< set Command index */
    mr_clrbitl(IMX_USDHC_CMD_XFR_TYP_CMDINX_MASK, &iCmdXfrTypReg);
    mr_setbitl(IMX_USDHC_CMD_XFR_TYP_CMDINX_U32(index), &iCmdXfrTypReg);

    /*!< Normal Other commands */
    mr_clrbitl(IMX_USDHC_CMD_XFR_TYP_CMDTYP_MASK, &iCmdXfrTypReg);
    /*!< Disable [Command Index Check] and [Command CRC Check] temporarily */
    mr_clrbitl(NR_ImxUsdhc_CmdXfrTyp_CmdIndexCheckEnable | NR_ImxUsdhc_CmdXfrTyp_CmdCrcCheckEnable | 
               IMX_USDHC_CMD_XFR_TYP_RSPTYP_MASK, &iCmdXfrTypReg);

    /*!< set/reset bits according to response type */
    switch (sptr_cmds->respType)
    {
        case NR_SdCard_Response_0:
            break;

        case NR_SdCard_Response_1:
        case NR_SdCard_Response_5:
        case NR_SdCard_Response_6:
        case NR_SdCard_Response_7:
            mr_setbitl(NR_ImxUsdhc_CmdXfrTyp_CmdIndexCheckEnable | NR_ImxUsdhc_CmdXfrTyp_CmdCrcCheckEnable, &iCmdXfrTypReg);
            mr_setbitl(NR_ImxUsdhc_CmdXfrTyp_Response48, &iCmdXfrTypReg);
            break;

        case NR_SdCard_Response_2:
            mr_setbitl(NR_ImxUsdhc_CmdXfrTyp_CmdCrcCheckEnable, &iCmdXfrTypReg);
            mr_setbitl(NR_ImxUsdhc_CmdXfrTyp_Response136, &iCmdXfrTypReg);
            break;

        case NR_SdCard_Response_3:
        case NR_SdCard_Response_4:
            mr_setbitl(NR_ImxUsdhc_CmdXfrTyp_Response48, &iCmdXfrTypReg);
            break;

        case NR_SdCard_Response_1b:
        case NR_SdCard_Response_5b:
            mr_setbitl(NR_ImxUsdhc_CmdXfrTyp_CmdIndexCheckEnable | NR_ImxUsdhc_CmdXfrTyp_CmdCrcCheckEnable, &iCmdXfrTypReg);
            mr_setbitl(NR_ImxUsdhc_CmdXfrTyp_ResponseBusy48, &iCmdXfrTypReg);
            break;

        default:
            mr_assert(IT_FALSE);
            break;
    }

    /*!< CMD_ARG::CMDARG: bit[31:0], Command Argument */
    mr_writel(argument, &sptr_usdhc->CMD_ARG);
    mr_writel(iCmdXfrTypReg, &sptr_usdhc->CMD_XFR_TYP);

    /*!< ------------------------------------------------------------------------------- */
    /*!< Wait command done */
    while (mr_isBitResetl(NR_ImxUsdhc_IntCmdErr_Bit | NR_ImxUsdhc_IntCmdComplete_Bit, &sptr_usdhc->INT_STATUS))
    {
        /*!< do nothing */
    }

    iRetval = (mr_isBitResetl(NR_ImxUsdhc_IntCmdErr_Bit | NR_ImxUsdhc_IntTuningErr_Bit, &sptr_usdhc->INT_STATUS) ? 
                                                                ER_NORMAL : (-ER_SCMD_FAILD));

    /*!< Clear Interrupt Status: write 1 to clear */
    mr_imx_clear_interrupt_flags(NR_ImxUsdhc_IntCmdErr_Bit | NR_ImxUsdhc_IntTuningErr_Bit | 
                                                    NR_ImxUsdhc_IntCmdComplete_Bit, &sptr_usdhc);
    while (mr_isBitSetl(NR_ImxUsdhc_IntCmdComplete_Bit, &sptr_usdhc->INT_STATUS))
    {
        /*!< do nothing */
    }                                                    

    /*!< reset */
    if ((-ER_SCMD_FAILD) == iRetval)
        imx6ull_sdmmc_reset_transfer(sptr_host);

    return iRetval;
}

/*!
 * @brief   imx6ull_sdmmc_recv_response
 * @param   none
 * @retval  none
 * @note    receive response by uSDHC
 */
static void imx6ull_sdmmc_recv_response(struct fwk_sdcard_cmd *sptr_cmds)
{
    srt_imx_usdhc_t *sptr_usdhc;
    struct fwk_sdcard_host *sptr_host;
    kuint8_t rsp_cnt = 3U;

    sptr_host = (struct fwk_sdcard_host *)sptr_cmds->ptrHost;
    if (!isValid(sptr_host))
        return;

    sptr_usdhc = (srt_imx_usdhc_t *)sptr_host->iHostIfBase;  

    if (NR_SdCard_Response_0 == sptr_cmds->respType)
        return;

    /*!< for CID/CSD: CMD_RSP0->bit[31:0] is CID/CSD bit[39:8] */
    *(sptr_cmds->resp) = mr_readl(&sptr_usdhc->CMD_RSP0);

    /*!< R2 args = CID/CSD (bit[127:8]) */
    if (NR_SdCard_Response_2 == sptr_cmds->respType)
    {
        /*!< CMD_RSP3: bit[23:0] is CID/CSD bit[127:104]; CMD_RSP3 bit[31:24] is reserved */
        *(sptr_cmds->resp + 3U) = mr_readl(&sptr_usdhc->CMD_RSP3);

        /*!< CMD_RSP2: bit[31:0] is CID/CSD bit[103:72] */
        *(sptr_cmds->resp + 2U) = mr_readl(&sptr_usdhc->CMD_RSP2);

        /*!< CMD_RSP1: bit[31:0] is CID/CSD bit[71:40] */
        *(sptr_cmds->resp + 1U) = mr_readl(&sptr_usdhc->CMD_RSP1);

        /*!< make resp similar to CID/CSD(bit[127:8]) */
        do
        {
            sptr_cmds->resp[rsp_cnt] = mr_bit_mask(sptr_cmds->resp[rsp_cnt], 0xffffff00U, 8U);
            mr_setbitl(sptr_cmds->resp[rsp_cnt - 1] >> 24U, &sptr_cmds->resp[rsp_cnt]);
            
        } while (--rsp_cnt);

        sptr_cmds->resp[0] = mr_bit_mask(sptr_cmds->resp[0], 0xffffff00U, 8U);
    }

}

/*!
 * @brief   imx6ull_sdmmc_transfer_data
 * @param   none
 * @retval  none
 * @note    send data by uSDHC
 */
static kint32_t imx6ull_sdmmc_transfer_data(struct fwk_sdcard_data *sptr_data)
{
    srt_imx_usdhc_t *sptr_usdhc;
    struct fwk_sdcard_host *sptr_host;
    kint32_t iRetval;

    if (!sptr_data)
        return -ER_NULLPTR;

    sptr_host = (struct fwk_sdcard_host *)sptr_data->ptrHost;
    if (!isValid(sptr_host))
        return -ER_NULLPTR;

    sptr_usdhc = (srt_imx_usdhc_t *)sptr_host->iHostIfBase;

    /*!< ------------------------------------------------------------ */
    /*!< write or read */
    iRetval = (sptr_data->txBuffer) ? imx6ull_sdmmc_write_data(sptr_usdhc, sptr_data) : imx6ull_sdmmc_read_data(sptr_usdhc, sptr_data);
    switch (iRetval)
    {
        case -ER_BUSY:
            imx6ull_sdmmc_reset(sptr_usdhc, NR_ImxUsdhc_SysCtrl_ResetTuning, 100U);
            break;

        case -ER_SDATA_FAILD:
        case -ER_RDATA_FAILD:
            imx6ull_sdmmc_reset_transfer(sptr_host);
            break;

        default:
            break;
    }

    mr_imx_clear_interrupt_flags(NR_ImxUsdhc_IntDataComplete_Bit | NR_ImxUsdhc_IntDataErr_Bit, &sptr_usdhc);
    while (!mr_isBitResetl(NR_ImxUsdhc_IntDataComplete_Bit, &sptr_usdhc->INT_STATUS));

    return iRetval;
}

/*!
 * @brief   imx6ull_sdmmc_write_data
 * @param   none
 * @retval  none
 * @note    write data to USDHC
 */
static kint32_t imx6ull_sdmmc_write_data(srt_imx_usdhc_t *sptr_usdhc, struct fwk_sdcard_data *sptr_data)
{
    struct fwk_sdcard_host *sptr_host;
    kuint32_t iWaterMarkLimit;
    kuint32_t *ptrTxBuffer;
    kuint32_t iDataWords, iTransWords;
    kuint32_t iRetry = 4096U;
    kbool_t blRetval;

    if ((!sptr_data) || (!sptr_usdhc))
        return -ER_NULLPTR;

    sptr_host = (struct fwk_sdcard_host *)sptr_data->ptrHost;
    if (!isValid(sptr_host))
        return -ER_NULLPTR;

    if (!mr_isBitResetl(NR_ImxUsdhc_MixCtrl_DataTransferDirection, &sptr_usdhc->MIX_CTRL))
        return -ER_NSUPPORT;

    /*!< 4 bytes align. blocksize is per block size (unit: byte) */
    iDataWords = mr_num_align4(sptr_data->blockSize) >> 2;
    iDataWords *= sptr_data->blockCount;
    ptrTxBuffer = (kuint32_t *)sptr_data->txBuffer;

    /*!< get watermark */
    iWaterMarkLimit = mr_getbitl(IMX_USDHC_WTMK_LVL_WR_WML_MASK, &sptr_usdhc->WTMK_LVL) >> IMX_USDHC_WTMK_LVL_WR_WML_OFFSET;

    /*!< the maximum of data_size that transmited must less than iWaterMarkLimit */
    while (iDataWords)
    {
        iTransWords = mr_ret_min2(iDataWords, iWaterMarkLimit);

        /*!< wait for buffer ready */
        do
        {
            blRetval = mr_isBitResetl(NR_ImxUsdhc_IntBufferWriteReady_Bit | NR_ImxUsdhc_IntDataErr_Bit | 
                                     NR_ImxUsdhc_IntTuningErr_Bit, &sptr_usdhc->INT_STATUS);

        } while (blRetval); // && (--iRetry));

        if (blRetval || (!iRetry))
            return -ER_TIMEOUT;

        if (mr_isBitSetl(NR_ImxUsdhc_IntTuningErr_Bit, &sptr_usdhc->INT_STATUS))
        {
            /*!< write 1 to clear */
            mr_imx_clear_interrupt_flags(NR_ImxUsdhc_IntTuningErr_Bit, &sptr_usdhc);
            return -ER_BUSY;
        }

        /*!< blRetval = (no error) ? true : false */
        blRetval = mr_isBitResetl(NR_ImxUsdhc_IntDataErr_Bit, &sptr_usdhc->INT_STATUS);
        mr_imx_clear_interrupt_flags(NR_ImxUsdhc_IntDataErr_Bit, &sptr_usdhc);

        if (blRetval)
        {
            iDataWords -= iTransWords;

            if (mr_is_aligned((kuaddr_t)ptrTxBuffer, 4U))
            {
                /*!< 32 bits (4 bytes) will be written to Register */
                while (iTransWords--)
                    mr_writel(*(ptrTxBuffer++), &sptr_usdhc->DATA_BUFF_ACC_PORT);
            }
            else
            {
                kuint32_t data;

                while (iTransWords--)
                {
                    /*!< If ptrRxBuffer is not 4-bytes-alignment, it may cause data_abort; use the super interface to get data */
                    u32_set2u8(&data, ptrTxBuffer++);

                    /*!< 32 bits (4 bytes) will be written to Register */
                    mr_writel(data, &sptr_usdhc->DATA_BUFF_ACC_PORT);
                }
            }

            /*!< watermark is full, clear write-ready bit, waiting for the next transmission */
            mr_imx_clear_interrupt_flags(NR_ImxUsdhc_IntBufferWriteReady_Bit, &sptr_usdhc);
        }
    }

    /*!< ------------------------------------------------------------------- */
    /*!< waitting for writting data complete, or error occur */
    while (mr_isBitResetl(NR_ImxUsdhc_IntDataComplete_Bit | NR_ImxUsdhc_IntDataErr_Bit, &sptr_usdhc->INT_STATUS))
    {
        /*!< do nothing */
    }

    if (mr_isBitSetl(NR_ImxUsdhc_IntDataErr_Bit, &sptr_usdhc->INT_STATUS))
        return -ER_SDATA_FAILD;

    return ER_NORMAL;
}

/*!
 * @brief   imx6ull_sdmmc_read_data
 * @param   none
 * @retval  none
 * @note    read data from USDHC
 */
static kint32_t imx6ull_sdmmc_read_data(srt_imx_usdhc_t *sptr_usdhc, struct fwk_sdcard_data *sptr_data)
{
    struct fwk_sdcard_host *sptr_host;
    kuint32_t iWaterMarkLimit;
    kuint32_t *ptrRxBuffer;
    kuint32_t iDataWords, iTransWords;
    kuint32_t iRetry = 4096U;
    kint32_t iRetval = ER_NORMAL;
    kbool_t blRetval;

    if ((!sptr_data) || (!sptr_usdhc))
        return -ER_NULLPTR;

    sptr_host = (struct fwk_sdcard_host *)sptr_data->ptrHost;
    if (!isValid(sptr_host))
        return -ER_NULLPTR;

    if (!mr_isBitSetl(NR_ImxUsdhc_MixCtrl_DataTransferDirection, &sptr_usdhc->MIX_CTRL))
        return -ER_NSUPPORT;

    /*!< 4 bytes align. blocksize is per block size (unit: byte) */
    iDataWords = mr_ralign(sptr_data->blockSize, 4U) >> 2;
    iDataWords *= sptr_data->blockCount;
    ptrRxBuffer = (kuint32_t *)sptr_data->rxBuffer;

    /*!< get watermark */
    iWaterMarkLimit = mr_getbitl(IMX_USDHC_WTMK_LVL_RD_WML_MASK, &sptr_usdhc->WTMK_LVL) >> IMX_USDHC_WTMK_LVL_RD_WML_OFFSET;

    /*!< the maximum of data_size that transmited must less than iWaterMarkLimit */
    while (iDataWords)
    {
        iTransWords = mr_ret_min2(iDataWords, iWaterMarkLimit);

        /*!< wait for buffer ready */
        do
        {
            blRetval = mr_isBitResetl(NR_ImxUsdhc_IntBufferReadReady_Bit | NR_ImxUsdhc_IntDataErr_Bit | 
                                     NR_ImxUsdhc_IntTuningErr_Bit, &sptr_usdhc->INT_STATUS);

        } while (blRetval); // && (--iRetry));

        if (blRetval || (!iRetry))
            return -ER_TIMEOUT;

        if (mr_isBitSetl(NR_ImxUsdhc_IntTuningErr_Bit, &sptr_usdhc->INT_STATUS))
        {
            mr_imx_clear_interrupt_flags(NR_ImxUsdhc_IntTuningErr_Bit, &sptr_usdhc);
            return -ER_BUSY;
        }

        /*!< blRetval = (no error) ? true : false */
        blRetval = mr_isBitResetl(NR_ImxUsdhc_IntDataErr_Bit, &sptr_usdhc->INT_STATUS);
        mr_imx_clear_interrupt_flags(NR_ImxUsdhc_IntDataErr_Bit, &sptr_usdhc);

        if (blRetval)
        {
            iDataWords -= iTransWords;

            if (mr_is_aligned((kuaddr_t)ptrRxBuffer, 4U))
            {
                /*!< 32 bits (4 bytes) will be read from Register */
                while (iTransWords--)
                    *(ptrRxBuffer++) = mr_readl(&sptr_usdhc->DATA_BUFF_ACC_PORT);
            }
            /*!< If ptrRxBuffer is not 4-bytes-alignment, it may cause data_abort */
            else
            {
                kuint32_t data;

                while (iTransWords--)
                {
                    /*!< 32 bits (4 bytes) will be read from Register */
                    data = mr_readl(&sptr_usdhc->DATA_BUFF_ACC_PORT);
                    u32_set2u8(ptrRxBuffer++, &data);
                }
            }

            /*!< watermark is full, clear read-ready bit, waiting for the next transmission */
            mr_imx_clear_interrupt_flags(NR_ImxUsdhc_IntBufferReadReady_Bit, &sptr_usdhc);
            iRetval = ER_NORMAL;
        }
        else
            iRetval = -ER_RDATA_FAILD;
    }

    return iRetval;
}

/*!
 * @brief   imx6ull_sdmmc_data_configure
 * @param   none
 * @retval  none
 * @note    configure registers about data transfer
 */
static void imx6ull_sdmmc_data_configure(srt_imx_usdhc_t *sptr_usdhc, struct fwk_sdcard_data *sptr_data, void *ptrData)
{
    kuint32_t iCmdXfrTyp;
    kuint32_t iMixCtrlReg;
    kuint32_t iBlockAttr;

    /*!<
     * ACMD23_ARGU2_EN: bit23
     * 1:   Argument2 register enable for ACMD23 sharing with SDMA system address register. Default is
     *      enable
     * 0:   Disable
     * 
     * default 1
     */
    mr_clrbitl(mr_bit(23U), &sptr_usdhc->VEND_SPEC2);

    iCmdXfrTyp = *(kuint32_t *)ptrData;

    /*!< Commands only using CMD line */
    mr_clrbitl(NR_ImxUsdhc_CmdXfrTyp_DataPresentSelect, &iCmdXfrTyp);

    /*!< read register */
    iMixCtrlReg = mr_readl(&sptr_usdhc->MIX_CTRL);

    /*!< Disable DMA */
    mr_clrbitl(NR_ImxUsdhc_MixCtrl_DmaEnable, &iMixCtrlReg);
    /*!< Block Count Disable, Select Single Block, Auto CMD12 Disable, Data Transfer Direction is Write */
    mr_clrbitl(NR_ImxUsdhc_MixCtrl_BlockCountEnable | NR_ImxUsdhc_MixCtrl_AutoCmd12Enable | NR_ImxUsdhc_MixCtrl_AutoCmd23Enable |
               NR_ImxUsdhc_MixCtrl_DataTransferDirection | NR_ImxUsdhc_MixCtrl_MultiSingleBlockSelect, &iMixCtrlReg);

    /*!< check if data request command */
    if (!sptr_data)
    {
        /*!< Update Register */
        *(kuint32_t *)ptrData = iMixCtrlReg;

        return;
    }

    /*!< Wait until command/data bus out of busy status */
    while (mr_isBitSetl(NR_ImxUsdhc_PresState_CmdInhibitDataLine, &sptr_usdhc->PRES_STATE))
    {
        /*!< do nothing */
    }

    if (sptr_data->blockCount > 1U)
    {
        mr_setbitl(NR_ImxUsdhc_MixCtrl_BlockCountEnable | NR_ImxUsdhc_MixCtrl_MultiSingleBlockSelect, &iMixCtrlReg);

        if (mr_isBitSetw(NR_SdCard_CmdFlagsAuto23Enable, &sptr_data->flags))
        {
            mr_setbitl(NR_ImxUsdhc_MixCtrl_AutoCmd23Enable, &iMixCtrlReg);
            mr_setbitl(mr_bit(23U), &sptr_usdhc->VEND_SPEC2);

            /*!<
            * DMA System Address
            * When ACMD23_ARGU2_EN is set to 0, SDMA uses this register as system address and supports only
            * 32-bit addressing mode. Auto CMD23 cannot be used with SDMA. When ACMD23_ARGU2_EN is set to
            * 1, SDMA uses ADMA System Address register (05Fh – 058h) instead of this register to support both 32-bit
            * and 64-bit addressing. This register is used only for Argument2 and SDMA may use Auto CMD23
            */
            mr_writel(sptr_data->blockCount, &sptr_usdhc->DS_ADDR);
        }

        if (mr_isBitSetw(NR_SdCard_CmdFlagsAuto12Enable, &sptr_data->flags))
            mr_setbitl(NR_ImxUsdhc_MixCtrl_AutoCmd12Enable, &iMixCtrlReg);
    }

    if (mr_isBitSetw(NR_SdCard_CmdFlagsReadEnable, &sptr_data->flags))
        mr_setbitl(NR_ImxUsdhc_MixCtrl_DataTransferDirection, &iMixCtrlReg);

    /*!< BLK_ATT (Block Attribute) */
    iBlockAttr = mr_readl(&sptr_usdhc->BLK_ATT);
    mr_clrbitl(IMX_USDHC_BLK_ATT_BLKCNT_MASK | IMX_USDHC_BLK_ATT_BLKSIZE_MASK, &iBlockAttr);

    /*!< block count */
    mr_setbitl(IMX_USDHC_BLK_ATT_BLKCNT_U32(sptr_data->blockCount), &iBlockAttr);
    /*!< block size */
    mr_setbitl(IMX_USDHC_BLK_ATT_BLKSIZE_U32(sptr_data->blockSize), &iBlockAttr);

    /*!< Commands using CMD line and DATA line */
    mr_setbitl(NR_ImxUsdhc_CmdXfrTyp_DataPresentSelect, &iCmdXfrTyp);

    /*!< Update Register */
    mr_writel(iBlockAttr, &sptr_usdhc->BLK_ATT);
    mr_writel(iMixCtrlReg, &sptr_usdhc->MIX_CTRL);

    *(kuint32_t *)ptrData = iCmdXfrTyp;
}

/*!
 * @brief   host_sdmmc_card_initial
 * @param   none
 * @retval  none
 * @note    initial host of SD Card
 */
void *host_sdmmc_card_initial(struct fwk_sdcard *sptr_card)
{
    srt_imx_usdhc_t *sptr_usdhc;
    struct fwk_sdcard_if *sptr_if;
    struct fwk_sdcard_host *sptr_host;
    kuint32_t iDoEmpty;

    if (!isValid(sptr_card))
        return mr_nullptr;

    sptr_if = &sptr_card->sgtc_if;

    sptr_host = (struct fwk_sdcard_host *)kzalloc(sizeof(struct fwk_sdcard_host), GFP_KERNEL);
    if (!isValid(sptr_host))
        return mr_nullptr;

    sptr_host->iHostIfBase = (kuaddr_t)IMX_SDMMC_IF_PORT_ENTRY();
    sptr_host->iHostCDBase = (kuaddr_t)IMX_SDMMC_CD_PORT_ENTRY();
    sptr_host->isSelfDync = true;

    sptr_usdhc = (srt_imx_usdhc_t *)sptr_host->iHostIfBase;

    /*!< get support */
    sptr_host->maxBlockLength = mr_getbit_u32(IMX_USDHC_HOST_CTRL_CAP_MBL_MASK, IMX_USDHC_HOST_CTRL_CAP_MBL_OFFSET, &sptr_usdhc->HOST_CTRL_CAP);
    sptr_host->maxBlockCount  = 0xffffU;

    mr_resetl(&sptr_host->flagBit);
    if (mr_isBitSetl(NR_ImxUsdhc_HostCtrlCap_Voltage18VSupport, &sptr_usdhc->HOST_CTRL_CAP))
        mr_setbitl(NR_SdCard_SupportVoltage1_8V, &sptr_host->flagBit);

    mr_setbitl(NR_SdCard_Support4BitWidth, &sptr_host->flagBit);

    /*!< interface initial */
    sptr_if->is_insert      = imx6ull_sdmmc_is_card_insert;
    sptr_if->setBusWidth    = imx6ull_sdmmc_set_bus_width;
    sptr_if->setClkFreq     = imx6ull_sdmmc_set_clk_freq;
    sptr_if->cardActive     = imx6ull_sdmmc_initial_active;
    sptr_if->sendCommand    = imx6ull_sdmmc_send_command;
    sptr_if->recvResp       = imx6ull_sdmmc_recv_response;
    sptr_if->sendData       = imx6ull_sdmmc_transfer_data;
    sptr_if->recvData       = imx6ull_sdmmc_transfer_data;
    sptr_if->switchVoltage  = imx6ull_sdmmc_switch_voltage;
    sptr_if->addHeadTail    = mr_nullptr;

    /*!< read but not use */
    iDoEmpty = mr_readl(&sptr_usdhc->DATA_BUFF_ACC_PORT);
    iDoEmpty++;

    mr_imx_clear_interrupt_flags(NR_ImxUsdhc_IntBufferWriteReady_Bit, &sptr_usdhc);
    mr_imx_clear_interrupt_flags(NR_ImxUsdhc_IntBufferReadReady_Bit, &sptr_usdhc);

    return sptr_host;
}


/* end of file */
