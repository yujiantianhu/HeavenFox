/*
 * IMX6ULL CPU part of USDHC Configure
 *
 * File Name:   imx6ull_usdhc.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.05.05
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __IMX6UL_USDHC_H
#define __IMX6UL_USDHC_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <imx6/imx6ull_periph.h>

/*!< The defines */
/*!< USDHC Interrupt Status/Signal Bit */
/*!< for Registers: INT_SIGNAL_EN, INT_STATUS_EN, INT_STATUS */
enum __ERT_IMX_USDHC_INTERRUPT_BIT
{
    NR_ImxUsdhc_IntCmdComplete_Bit = mr_bit(0U),                   /*!< Command Complete Status Enable */
    NR_ImxUsdhc_IntDataComplete_Bit = mr_bit(1U),                  /*!< Transfer Complete Status Enable */
    NR_ImxUsdhc_IntBlockGapEvent_Bit = mr_bit(2U),                 /*!< Block Gap Event Status Enable */
    NR_ImxUsdhc_IntDmaInterrupt_Bit = mr_bit(3U),                  /*!< DMA Interrupt Status Enable */
    NR_ImxUsdhc_IntBufferWriteReady_Bit = mr_bit(4U),              /*!< Buffer Write Ready Status Enable */
    NR_ImxUsdhc_IntBufferReadReady_Bit = mr_bit(5U),               /*!< Buffer Read Ready Status Enable */
    NR_ImxUsdhc_IntCardInsertion_Bit = mr_bit(6U),                 /*!< Card Insertion Status Enable */
    NR_ImxUsdhc_IntCardRemoval_Bit = mr_bit(7U),                   /*!< Card Removal Status Enable */
    NR_ImxUsdhc_IntCardInterrupt_Bit = mr_bit(8U),                 /*!< Card Interrupt Status Enable */
    NR_ImxUsdhc_IntReTuningEvent_Bit = mr_bit(12U),                /*!< Re-Tuning Event Status Enable */
    NR_ImxUsdhc_IntTuningPass_Bit = mr_bit(14U),                   /*!< Tuning Pass Status Enable */
    NR_ImxUsdhc_IntCmdTimeOutErr_Bit = mr_bit(16U),                /*!< Command Timeout Error Status Enable */
    NR_ImxUsdhc_IntCmdCrcErr_Bit = mr_bit(17U),                    /*!< Command CRC Error Status Enable */
    NR_ImxUsdhc_IntCmdEndBitErr_Bit = mr_bit(18U),                 /*!< Command End Bit Error Status Enable */
    NR_ImxUsdhc_IntCmdIndexErr_Bit = mr_bit(19U),                  /*!< Command Index Error Status Enable */
    NR_ImxUsdhc_IntDataTimeOutErr_Bit = mr_bit(20U),               /*!< Data Timeout Error Status Enable */
    NR_ImxUsdhc_IntDataCrcErr_Bit = mr_bit(21U),                   /*!< Data CRC Error Status Enable */
    NR_ImxUsdhc_IntDataEndBitErr_Bit = mr_bit(22U),                /*!< Data End Bit Error Status Enable */
    NR_ImxUsdhc_IntACmd12Err_Bit = mr_bit(24U),                    /*!< Auto CMD12 Error Status Enable */
    NR_ImxUsdhc_IntTuningErr_Bit = mr_bit(26U),                    /*!< Tuning Error Status Enable */
    NR_ImxUsdhc_IntDmaErr_Bit = mr_bit(28U),                       /*!< DMA Error Status Enable */

    NR_ImxUsdhc_IntCmdErr_Bit = (NR_ImxUsdhc_IntCmdIndexErr_Bit | NR_ImxUsdhc_IntCmdEndBitErr_Bit |
                                NR_ImxUsdhc_IntCmdCrcErr_Bit | NR_ImxUsdhc_IntCmdTimeOutErr_Bit),
    NR_ImxUsdhc_IntDataErr_Bit = (NR_ImxUsdhc_IntDataTimeOutErr_Bit | NR_ImxUsdhc_IntDataCrcErr_Bit |
                                NR_ImxUsdhc_IntDataEndBitErr_Bit | NR_ImxUsdhc_IntACmd12Err_Bit),
    NR_ImxUsdhc_IntAllErr_Bit = (NR_ImxUsdhc_IntCmdErr_Bit | NR_ImxUsdhc_IntDataErr_Bit | NR_ImxUsdhc_IntDmaErr_Bit),
    NR_ImxUsdhc_IntAllCmd_Bit = (NR_ImxUsdhc_IntCmdErr_Bit | NR_ImxUsdhc_IntCmdComplete_Bit),
    NR_ImxUsdhc_IntAllData_Bit = (NR_ImxUsdhc_IntDataErr_Bit | NR_ImxUsdhc_IntDataComplete_Bit),
    NR_ImxUsdhc_IntAllTuning_Bit = (NR_ImxUsdhc_IntReTuningEvent_Bit | NR_ImxUsdhc_IntTuningPass_Bit |
                                NR_ImxUsdhc_IntTuningErr_Bit),
    NR_ImxUsdhc_IntCardDetect_Bit = (NR_ImxUsdhc_IntCardInsertion_Bit | NR_ImxUsdhc_IntCardRemoval_Bit),

    NR_ImxUsdhc_IntAllInterrupt_Bit = (NR_ImxUsdhc_IntBlockGapEvent_Bit | NR_ImxUsdhc_IntCardInterrupt_Bit |
                                NR_ImxUsdhc_IntAllErr_Bit | NR_ImxUsdhc_IntAllCmd_Bit | NR_ImxUsdhc_IntAllData_Bit | 
                                NR_ImxUsdhc_IntAllTuning_Bit),

/*!< write 1 to clear */
#define mr_imx_clear_interrupt_flags(flagBit, usdhc)                mr_writel(flagBit, usdhc->INT_STATUS)
};

/*!< for Register: SYS_CTRL */
enum __ERT_IMX_USDHC_SYS_CTRL_BIT
{
/*!< 
 * DVS: bit[7:4]. Divisor
 * This register is used to select the frequency of the SDCLK pin. This register field selects the divide value
 * of the first divisor stage.
 * Before changing clock divisor value(SDCLKFS or DVS), Host Driver should make sure the SDSTB bit is
 * high.
 * dividers: 1(0h) ~ 16(fh)
 */
#define IMX_USDHC_SYS_CTRL_DVS_OFFSET                               (4U)
#define IMX_USDHC_SYS_CTRL_DVS_MASK                                 (0xf0U) 
#define IMX_USDHC_SYS_CTRL_DVS_BIT(x)                               mr_bit_mask((x), IMX_USDHC_SYS_CTRL_DVS_MASK, IMX_USDHC_SYS_CTRL_DVS_OFFSET)
#define IMX_USDHC_SYS_CTRL_DVS_U32(x)                               (((x) >= 1) ? IMX_USDHC_SYS_CTRL_DVS_BIT((x) - 1) : IMX_USDHC_SYS_CTRL_DVS_BIT(1U))

/*!< 
 * SDCLKFS: bit[15:8]. SDCLK Frequency Select
 * This register is used to select the frequency of the SDCLK pin.This register field selects the divide value of
 * the second divisor stage.
 * In Single Data Rate mode(DDR_EN bit of MIX_CTRL is '0'): it can divided by 1(00h), 2(01h), 4(10h), ..., 128(40h), 256(80h)
 * While in Dual Data Rate mode(DDR_EN bit of MIX_CTRL is '1'): it can divided by 2(00h), 4(01h), 8(10h), ..., 256(40h), 512(80h)
 */
#define IMX_USDHC_SYS_CTRL_SDCLKFS_OFFSET                           (8U)
#define IMX_USDHC_SYS_CTRL_SDCLKFS_MASK                             (0xff00U) 
#define IMX_USDHC_SYS_CTRL_SDCLKFS_BIT(x)                           mr_bit_mask((x), IMX_USDHC_SYS_CTRL_SDCLKFS_MASK, IMX_USDHC_SYS_CTRL_SDCLKFS_OFFSET)
#define IMX_USDHC_SYS_CTRL_SDCLKFS_U32(ddr_en, x)                   ((ddr_en) ? IMX_USDHC_SYS_CTRL_SDCLKFS_BIT((x) >> 2U) : IMX_USDHC_SYS_CTRL_SDCLKFS_BIT((x) >> 1U))

/*!< DTOCV: bit[19:16]. Data Timeout Counter Value */
#define IMX_USDHC_SYS_CTRL_DTOCV_OFFSET                             (16U)
#define IMX_USDHC_SYS_CTRL_DTOCV_MASK                               (0xf0000U) 
#define IMX_USDHC_SYS_CTRL_DTOCV_BIT(x)                             mr_bit_mask((x), IMX_USDHC_SYS_CTRL_DTOCV_MASK, IMX_USDHC_SYS_CTRL_DTOCV_OFFSET)
#define IMX_USDHC_SYS_CTRL_DTOCV_U32(x)                             IMX_USDHC_SYS_CTRL_DTOCV_BIT(x)

    NR_ImxUsdhc_SysCtrl_SoftResetAll = mr_bit(24U),                /*!< RSTA: Software Reset For ALL. 1: Reset; 0: No Reset */   
    NR_ImxUsdhc_SysCtrl_SoftResetCmdLine = mr_bit(25U),            /*!< RSTC: Software Reset For CMD Line. 1: Reset; 0: No Reset */
    NR_ImxUsdhc_SysCtrl_SoftResetDataLine = mr_bit(26U),           /*!< RSTD: Software Reset For DATA Line. 1: Reset; 0: No Reset */

    /*!< 
     * INITA: Initialization Active.
     * When this bit is set, 80 SD-Clocks are sent to the card. After the 80 clocks are sent, this bit is self cleared.
     * This bit is very useful during the card power-up period when 74 SD-Clocks are needed and the clock auto
     * gating feature is enabled. Writing 1 to this bit when this bit is already 1 has no effect. Writing 0 to this bit at
     * any time has no effect. When either of the CIHB and CDIHB bits in the Present State Register are set,
     * writing 1 to this bit is ignored (i.e. when command line or data lines are active, write to this bit is not
     * allowed). On the otherhand, when this bit is set, i.e., during intialization active period, it is allowed to issue
     * command, and the command bit stream will appear on the CMD pad after all 80 clock cycles are done. So
     * when this command ends, the driver can make sure the 80 clock cycles are sent out. This is very useful
     * when the driver needs send 80 cycles to the card and does not want to wait till this bit is self cleared
     */
    NR_ImxUsdhc_SysCtrl_InitialActive = mr_bit(27U),

    /*!< 
     * RSTT: Reset Tuning. 
     * When set this bit to 1, it will reset tuning circuit. After tuning circuits are reset, bit value is 0 
     */
    NR_ImxUsdhc_SysCtrl_ResetTuning = mr_bit(28U),

    NR_ImxUsdhc_SysCtrl_ResetMask = ((NR_ImxUsdhc_SysCtrl_SoftResetAll | NR_ImxUsdhc_SysCtrl_SoftResetCmdLine |
                                      NR_ImxUsdhc_SysCtrl_SoftResetDataLine | NR_ImxUsdhc_SysCtrl_ResetTuning) & 0x17000000U),
};

/*
 * for Register: MIX_CTRL
 * This register is used to DMA and data transfer
 */
enum __ERT_IMX_USDHC_MIX_CTRL_BIT
{
    /*!<
     * DMAEN: DMA Enable
     * This bit enables DMA functionality. If this bit is set to 1, a DMA operation shall begin when the Host Driver
     * sets the DPSEL bit of this register. Whether the Simple DMA or the Advanced DMA is active depends on
     * the DMA Select field of the Protocol Control register
     */
    NR_ImxUsdhc_MixCtrl_DmaEnable = mr_bit(0U),

    /*!< 
     * BCEN: Block Count Enable
     * This bit is used to enable the Block Count register, which is only relevant for multiple block transfers.
     * When this bit is 0, the internal counter for block is disabled, which is useful in executing an infinite transfer
     */
    NR_ImxUsdhc_MixCtrl_BlockCountEnable = mr_bit(1U),

    NR_ImxUsdhc_MixCtrl_AutoCmd12Enable = mr_bit(2U),              /*!< Auto CMD12 Enable */
    NR_ImxUsdhc_MixCtrl_DualDataRateMode = mr_bit(3U),             /*!< DDR_EN: Dual Data Rate mode selection */

    /*!< 
     * DTDSEL: Data Transfer Direction Select
     *  1, Read (Card to Host)
     *  0, Write (Host to Card)
     */
    NR_ImxUsdhc_MixCtrl_DataTransferDirection = mr_bit(4U),

    /*!<
     * MSBSEL: Multi/Single Block Select
     * This bit enables multiple block DATA line data transfers. For any other commands, this bit can be set to 0.
     * If this bit is 0, it is not necessary to set the Block Count register (CMD_XFR_TYP)
     *  1, Multiple Blocks
     *  0, Single Block
     */
    NR_ImxUsdhc_MixCtrl_MultiSingleBlockSelect = mr_bit(5U),

    /*!<
     * AC23EN
     * When this bit is set to 1, the Host Controller issues a CMD23 automatically before issuing a command
     * specified in the Command Register
     */
    NR_ImxUsdhc_MixCtrl_AutoCmd23Enable = mr_bit(7U),

    /*!<
     * EXE_TUNE: Execute Tuning: (Only used for SD3.0, SDR104 mode)
     * When STD_TUNING_EN is 0, this bit is set to 1 to indicate the Host Driver is starting tuning procedure.
     * Tuning procedure is aborted by writing 0.
     *  1, Execute Tuning
     *  0, Not Tuned or Tuning Completed
     */
    NR_ImxUsdhc_MixCtrl_ExecuteTuning = mr_bit(22U),

    /*!<
     * SMP_CLK_SEL: When STD_TUNING_EN is 0, this bit is used to select Tuned clock or Fixed clock to sample data / cmd
     * (Only used for SD3.0, SDR104 mode)
     *  1, Tuned clock is used to sample data/cmd
     *  0, Fixed clock is used to sample data/cmd
     */
    NR_ImxUsdhc_MixCtrl_SampleClkSelect = mr_bit(23U),

    /*!<
     * AUTO_TUNE_EN: Auto Tuning Enable (Only used for SD3.0, SDR104 mode)
     *  1, Enable auto tuning
     *  0, Disable auto tuning
     */
    NR_ImxUsdhc_MixCtrl_AutoTuning = mr_bit(24U),

    /*!<
     * FBCLK_SEL: Feedback Clock Source Selection (Only used for SD3.0, SDR104 mode)
     *  1, Feedback clock comes from the ipp_card_clk_out
     *  0, Feedback clock comes from the loopback CLK
     */
    NR_ImxUsdhc_MixCtrl_FeedbackClkSelect = mr_bit(25U),
};

/*!< for Register: PROT_CTRL */
enum __ERT_IMX_USDHC_PROT_CTRL_BIT
{
    NR_ImxUsdhc_ProtCtrl_LedControl = mr_bit(0U),                  /*!< LCTL. 1: LED on; 0: LED off */

/*!<
 * DTW: Data Transfer Width
 * This bit selects the data width of the SD bus for a data transfer. The Host Driver shall set it to match the
 * data width of the card. Possible Data transfer Width is 1-bit, 4-bits or 8-bits.
 */
#define IMX_USDHC_PROT_CTRL_DTW_OFFSET                              (1U)
#define IMX_USDHC_PROT_CTRL_DTW_MASK                                (0x6U)
#define IMX_USDHC_PROT_CTRL_DTW_BIT(x)                              mr_bit_mask((x), IMX_USDHC_PROT_CTRL_DTW_MASK, IMX_USDHC_PROT_CTRL_DTW_OFFSET)

    NR_ImxUsdhc_ProtCtrl_DataWidth1Bits = IMX_USDHC_PROT_CTRL_DTW_BIT(0U),
    NR_ImxUsdhc_ProtCtrl_DataWidth4Bits = IMX_USDHC_PROT_CTRL_DTW_BIT(1U),
    NR_ImxUsdhc_ProtCtrl_DataWidth8Bits = IMX_USDHC_PROT_CTRL_DTW_BIT(2U),

    NR_ImxUsdhc_ProtCtrl_Data3AsCardDetect = mr_bit(3U),           /*!< D3CD. 1: DATA3 as Card Detection Pin; 0 DATA3 does not monitor Card Insertion */

/*!< EMODE: Endian Mode */
#define IMX_USDHC_PROT_CTRL_EMODE_OFFSET                            (4U)
#define IMX_USDHC_PROT_CTRL_EMODE_MASK                              (0x30U)
#define IMX_USDHC_PROT_CTRL_EMODE_BIT(x)                            mr_bit_mask((x), IMX_USDHC_PROT_CTRL_EMODE_MASK, IMX_USDHC_PROT_CTRL_EMODE_OFFSET)

    NR_ImxUsdhc_ProtCtrl_BigEndian = IMX_USDHC_PROT_CTRL_EMODE_BIT(0U),
    NR_ImxUsdhc_ProtCtrl_HalfWordEndian = IMX_USDHC_PROT_CTRL_EMODE_BIT(1U),
    NR_ImxUsdhc_ProtCtrl_LittleEndian = IMX_USDHC_PROT_CTRL_EMODE_BIT(2U),

    /*!<
     * CDTL: Card Detect Test Level
     * This is bit is enabled while the Card Detection Signal Selection is set to 1 and it indicates card insertion.
     *  1, Card Detect Test Level is 1, card inserted
     *  0, Card Detect Test Level is 0, no card inserted
     */
    NR_ImxUsdhc_ProtCtrl_CardDetectTestLevel = mr_bit(6U),

    /*!<
     * CDSS: Card Detect Signal Selection
     * This bit selects the source for the card detection.
     *  1, Card Detection Test Level is selected (for test purpose).
     *  0, Card Detection Level is selected (for normal purpose).
     */
    NR_ImxUsdhc_ProtCtrl_CardDetectLevelSelect = mr_bit(7U),

/*!< DMA Select */
#define IMX_USDHC_PROT_CTRL_DMASEL_OFFSET                           (8U)
#define IMX_USDHC_PROT_CTRL_DMASEL_MASK                             (0x300U)
#define IMX_USDHC_PROT_CTRL_DMASEL_BIT(x)                           mr_bit_mask((x), IMX_USDHC_PROT_CTRL_DMASEL_MASK, IMX_USDHC_PROT_CTRL_DMASEL_OFFSET)

    NR_ImxUsdhc_ProtCtrl_SDmaSelect = IMX_USDHC_PROT_CTRL_DMASEL_BIT(0U),
    NR_ImxUsdhc_ProtCtrl_ADma1Select = IMX_USDHC_PROT_CTRL_DMASEL_BIT(1U),
    NR_ImxUsdhc_ProtCtrl_ADma2Select = IMX_USDHC_PROT_CTRL_DMASEL_BIT(2U),

    NR_ImxUsdhc_ProtCtrl_StopBlockkGapRequest = mr_bit(16U),       /*!< SABGREQ. Stop At Block Gap Request. */
    NR_ImxUsdhc_ProtCtrl_ContinueRequest = mr_bit(17U),            /*!< CREQ. Continue Request */
    NR_ImxUsdhc_ProtCtrl_ReadWaitControl = mr_bit(18U),            /*!< RWCTL. Read Wait Control */
    NR_ImxUsdhc_ProtCtrl_InterruptBlockGap = mr_bit(19U),          /*!< IABG. Interrupt At Block Gap */

    /*!<
     * RD_DONE_NO_8CLK: Read done no 8 clock
     * According to the SD/MMC spec, for read data transaction, 8 clocks are needed after the end bit of the
     * last data block. So, by default(RD_DONE_NO_8CLK=0), 8 clocks will be active after the end bit of the
     * last read data transaction.
     * However, this 8 clocks should not be active if user wants to use stop at block gap(include the auto stop
     * at block gap in boot mode) feature for read and the RWCTL bit(bit18) is not enabled. In this case,
     * software should set RD_DONE_NO_8CLK to avoid this 8 clocks. Otherwise, the device may send extra
     * data to uSDHC while uSDHC ignores these data.
     * In a summary, this bit should be set only if the use case needs to use stop at block gap feature while the
     * device can't support the read wait feature
     */
    NR_ImxUsdhc_ProtCtrl_ReadDoneNo8Clock = mr_bit(20U),

    NR_ImxUsdhc_ProtCtrl_WakeEventOnCardInt = mr_bit(24U),         /*!< WECINT. Wakeup Event Enable On Card Interrupt */
    NR_ImxUsdhc_ProtCtrl_WakeEventOnCardInsert = mr_bit(25U),      /*!< WECINS. Wakeup Event Enable On SD Card Insertion */
    NR_ImxUsdhc_ProtCtrl_WakeEventOnCardRemoval = mr_bit(26U),     /*!< WECRM. Wakeup Event Enable On SD Card Removal */
};

/*!< for Register: PRES_STATE */
enum __ERT_IMX_USDHC_PRES_STATE_BIT
{
    NR_ImxUsdhc_PresState_CmdInhibitCmdLine = mr_bit(0U),          /*!< CIHB. 1: Cannot issue command; 0: Can issue command using only CMD line */
    NR_ImxUsdhc_PresState_CmdInhibitDataLine = mr_bit(1U),         /*!< CDIHB. 1: Cannot issue command which uses the DATA line; 0: Can issue command which uses the DATA line */
    NR_ImxUsdhc_PresState_DataLineActive = mr_bit(2U),             /*!< DLA. Data Line Active */
    NR_ImxUsdhc_PresState_SDClockStable = mr_bit(3U),              /*!< SDSTB. SD Clock Stable. */
    NR_ImxUsdhc_PresState_WriteTransferActive = mr_bit(8U),        /*!< WTA. Write Transfer Active */

    NR_ImxUsdhc_PresState_ReadTransferActive = mr_bit(9U),         /*!< RTA. Read Transfer Active */
    NR_ImxUsdhc_PresState_BufferWriteEnable = mr_bit(10U),         /*!< BWEN. Buffer Write Enable */
    NR_ImxUsdhc_PresState_BufferReadEnable = mr_bit(11U),          /*!< BREN. Buffer Read Enable */
    NR_ImxUsdhc_PresState_CardInserted = mr_bit(16U),              /*!< CINST. Card Inserted */
    NR_ImxUsdhc_PresState_CommandLineLevel = mr_bit(23U),          /*!< CLSL. CMD Line Signal Level */

    NR_ImxUsdhc_PresState_Data0LineLevel = mr_bit(24U),            /*!< DLSL. Data 0 line signal level */
    NR_ImxUsdhc_PresState_Data1LineLevel = mr_bit(25U),            /*!< DLSL. Data 1 line signal level */
    NR_ImxUsdhc_PresState_Data2LineLevel = mr_bit(26U),            /*!< DLSL. Data 2 line signal level */
    NR_ImxUsdhc_PresState_Data3LineLevel = mr_bit(27U),            /*!< DLSL. Data 3 line signal level */
    NR_ImxUsdhc_PresState_Data4LineLevel = mr_bit(28U),            /*!< DLSL. Data 4 line signal level */
    NR_ImxUsdhc_PresState_Data5LineLevel = mr_bit(29U),            /*!< DLSL. Data 5 line signal level */
    NR_ImxUsdhc_PresState_Data6LineLevel = mr_bit(30U),            /*!< DLSL. Data 6 line signal level */
    NR_ImxUsdhc_PresState_Data7LineLevel = mr_bit(31U),            /*!< DLSL. Data 7 line signal level */
};

/*!< for Register: CMD_XFR_TYP */
enum __ERT_IMX_USDHC_CMD_XFR_TYP_BIT
{
/*!< RSPTYP: Response Type Select */
#define IMX_USDHC_CMD_XFR_TYP_RSPTYP_OFFSET                         (16U)
#define IMX_USDHC_CMD_XFR_TYP_RSPTYP_MASK                           (0x30000U)
#define IMX_USDHC_CMD_XFR_TYP_RSPTYP_BIT(x)                         mr_bit_mask((x), IMX_USDHC_CMD_XFR_TYP_RSPTYP_MASK, IMX_USDHC_CMD_XFR_TYP_RSPTYP_OFFSET)

    NR_ImxUsdhc_CmdXfrTyp_NoResponse = IMX_USDHC_CMD_XFR_TYP_RSPTYP_BIT(0U),
    NR_ImxUsdhc_CmdXfrTyp_Response136 = IMX_USDHC_CMD_XFR_TYP_RSPTYP_BIT(1U),
    NR_ImxUsdhc_CmdXfrTyp_Response48 = IMX_USDHC_CMD_XFR_TYP_RSPTYP_BIT(2U),
    NR_ImxUsdhc_CmdXfrTyp_ResponseBusy48 = IMX_USDHC_CMD_XFR_TYP_RSPTYP_BIT(3U),

    NR_ImxUsdhc_CmdXfrTyp_CmdCrcCheckEnable = mr_bit(19U),
    NR_ImxUsdhc_CmdXfrTyp_CmdIndexCheckEnable = mr_bit(20U),
    NR_ImxUsdhc_CmdXfrTyp_DataPresentSelect = mr_bit(21U),

#define IMX_USDHC_CMD_XFR_TYP_CMDTYP_OFFSET                         (22U)
#define IMX_USDHC_CMD_XFR_TYP_CMDTYP_MASK                           (0xc00000U)
#define IMX_USDHC_CMD_XFR_TYP_CMDTYP_BIT(x)                         mr_bit_mask((x), IMX_USDHC_CMD_XFR_TYP_CMDTYP_MASK, IMX_USDHC_CMD_XFR_TYP_CMDTYP_OFFSET)

    /*!<
     * CMDTYP: bit[23:22], Command Type
     *  11, Abort CMD12, CMD52 for writing I/O Abort in CCCR
     *  10, Resume CMD52 for writing Function Select in CCCR
     *  01, Suspend CMD52 for writing Bus Suspend in CCCR
     *  00, Normal Other commands
     */
    NR_ImxUsdhc_CmdXfrTyp_SuspendCommand = IMX_USDHC_CMD_XFR_TYP_CMDTYP_BIT(1U),
    NR_ImxUsdhc_CmdXfrTyp_ResumeCommand = IMX_USDHC_CMD_XFR_TYP_CMDTYP_BIT(2U),
    NR_ImxUsdhc_CmdXfrTyp_AbortCommand = IMX_USDHC_CMD_XFR_TYP_CMDTYP_BIT(3U),

#define IMX_USDHC_CMD_XFR_TYP_CMDINX_OFFSET                         (24U)
#define IMX_USDHC_CMD_XFR_TYP_CMDINX_MASK                           (0x3f000000U)
#define IMX_USDHC_CMD_XFR_TYP_CMDINX_BIT(x)                         mr_bit_mask((x), IMX_USDHC_CMD_XFR_TYP_CMDINX_MASK, IMX_USDHC_CMD_XFR_TYP_CMDINX_OFFSET)
#define IMX_USDHC_CMD_XFR_TYP_CMDINX_U32(x)                         IMX_USDHC_CMD_XFR_TYP_CMDINX_BIT(x)
};

/*!< for Register: HOST_CTRL_CAP */
enum __ERT_IMX_USDHC_HOST_CTRL_CAP_BIT
{
    NR_ImxUsdhc_HostCtrlCap_Sdr50Support = mr_bit(0U),             /*!< This bit indicates support of SDR50 mode */
    NR_ImxUsdhc_HostCtrlCap_Sdr104Support = mr_bit(1U),            /*!< This bit indicates support of SDR104 mode */
    NR_ImxUsdhc_HostCtrlCap_Ddr50Support = mr_bit(2U),             /*!< This bit indicates support of DDR50 mode */

#define IMX_USDHC_HOST_CTRL_CAP_TIMCNT_RETUNING_OFFSET              (8U)
#define IMX_USDHC_HOST_CTRL_CAP_TIMCNT_RETUNING_MASK                (0xf00U)
#define IMX_USDHC_HOST_CTRL_CAP_TIMCNT_RETUNING_BIT(x)              mr_bit_mask((x), IMX_USDHC_HOST_CTRL_CAP_TIMCNT_RETUNING_MASK, IMX_USDHC_HOST_CTRL_CAP_TIMCNT_RETUNING_OFFSET)

    NR_ImxUsdhc_HostCtrlCap_UseTuningSdr50 = mr_bit(13U),          /*!< This bit is set to 1. Host controller requires tuning to operate SDR50 */

#define IMX_USDHC_HOST_CTRL_CAP_RETUNING_MODE_OFFSET                (14U)
#define IMX_USDHC_HOST_CTRL_CAP_RETUNING_MODE_MASK                  (0xc000U)
#define IMX_USDHC_HOST_CTRL_CAP_RETUNING_MODE_BIT(x)                mr_bit_mask((x), IMX_USDHC_HOST_CTRL_CAP_RETUNING_MODE_MASK, IMX_USDHC_HOST_CTRL_CAP_RETUNING_MODE_OFFSET)

    NR_ImxUsdhc_HostCtrlCap_RetuningMode1 = IMX_USDHC_HOST_CTRL_CAP_RETUNING_MODE_BIT(0U),
    NR_ImxUsdhc_HostCtrlCap_RetuningMode2 = IMX_USDHC_HOST_CTRL_CAP_RETUNING_MODE_BIT(1U),
    NR_ImxUsdhc_HostCtrlCap_RetuningMode3 = IMX_USDHC_HOST_CTRL_CAP_RETUNING_MODE_BIT(3U),

#define IMX_USDHC_HOST_CTRL_CAP_MBL_OFFSET                          (16U)
#define IMX_USDHC_HOST_CTRL_CAP_MBL_MASK                            (0x70000U)
#define IMX_USDHC_HOST_CTRL_CAP_MBL_BIT(x)                          mr_bit_mask((x), IMX_USDHC_HOST_CTRL_CAP_MBL_MASK, IMX_USDHC_HOST_CTRL_CAP_MBL_OFFSET)

    NR_ImxUsdhc_HostCtrlCap_MaxBlockLenth512Byte = IMX_USDHC_HOST_CTRL_CAP_MBL_BIT(0U),
    NR_ImxUsdhc_HostCtrlCap_MaxBlockLenth1024Byte = IMX_USDHC_HOST_CTRL_CAP_MBL_BIT(1U),
    NR_ImxUsdhc_HostCtrlCap_MaxBlockLenth2048Byte = IMX_USDHC_HOST_CTRL_CAP_MBL_BIT(2U),
    NR_ImxUsdhc_HostCtrlCap_MaxBlockLenth4096Byte = IMX_USDHC_HOST_CTRL_CAP_MBL_BIT(3U),

    NR_ImxUsdhc_HostCtrlCap_AdmaSupport = mr_bit(20U),             /*!< This bit indicates whether the uSDHC supports the ADMA feature */
    NR_ImxUsdhc_HostCtrlCap_HighSpeedSupport = mr_bit(21U),        /*!< High Speed Supported (Host System maybe supply a SD Clock frequency from 25 MHz to 50 MHz) */
    NR_ImxUsdhc_HostCtrlCap_DmaSupport = mr_bit(22U),              /*!< DMA Support */
    NR_ImxUsdhc_HostCtrlCap_SuspendResumeSupport = mr_bit(23U),    /*!< Suspend / Resume Support */
    NR_ImxUsdhc_HostCtrlCap_Voltage33VSupport = mr_bit(24U),       /*!< Voltage Support 3.3V */
    NR_ImxUsdhc_HostCtrlCap_Voltage30VSupport = mr_bit(25U),       /*!< Voltage Support 3.0V */
    NR_ImxUsdhc_HostCtrlCap_Voltage18VSupport = mr_bit(26U),       /*!< Voltage Support 1.8V */
};

/*!< for Register: WTMK_LVL */
#define IMX_USDHC_WTMK_LVL_RD_WML_OFFSET                            (0U)
#define IMX_USDHC_WTMK_LVL_RD_WML_MASK                              (0xffU)
#define IMX_USDHC_WTMK_LVL_RD_WML_BIT(x)                            mr_bit_mask((x), IMX_USDHC_WTMK_LVL_RD_WML_MASK, IMX_USDHC_WTMK_LVL_RD_WML_OFFSET)
#define IMX_USDHC_WTMK_LVL_RD_WML_U32(x)                            IMX_USDHC_WTMK_LVL_RD_WML_BIT(x)

#define IMX_USDHC_WTMK_LVL_RD_BRST_LEN_OFFSET                       (8U)
#define IMX_USDHC_WTMK_LVL_RD_BRST_LEN_MASK                         (0x1f00U)
#define IMX_USDHC_WTMK_LVL_RD_BRST_LEN_BIT(x)                       mr_bit_mask((x), IMX_USDHC_WTMK_LVL_RD_BRST_LEN_MASK, IMX_USDHC_WTMK_LVL_RD_BRST_LEN_OFFSET)
#define IMX_USDHC_WTMK_LVL_RD_BRST_LEN_U32(x)                       IMX_USDHC_WTMK_LVL_RD_BRST_LEN_BIT(x)

#define IMX_USDHC_WTMK_LVL_WR_WML_OFFSET                            (16U)
#define IMX_USDHC_WTMK_LVL_WR_WML_MASK                              (0xff0000U)
#define IMX_USDHC_WTMK_LVL_WR_WML_BIT(x)                            mr_bit_mask((x), IMX_USDHC_WTMK_LVL_WR_WML_MASK, IMX_USDHC_WTMK_LVL_WR_WML_OFFSET)
#define IMX_USDHC_WTMK_LVL_WR_WML_U32(x)                            IMX_USDHC_WTMK_LVL_WR_WML_BIT(x)

#define IMX_USDHC_WTMK_LVL_WR_BRST_LEN_OFFSET                       (24U)
#define IMX_USDHC_WTMK_LVL_WR_BRST_LEN_MASK                         (0x1f000000U)
#define IMX_USDHC_WTMK_LVL_WR_BRST_LEN_BIT(x)                       mr_bit_mask((x), IMX_USDHC_WTMK_LVL_WR_BRST_LEN_MASK, IMX_USDHC_WTMK_LVL_WR_BRST_LEN_OFFSET)
#define IMX_USDHC_WTMK_LVL_WR_BRST_LEN_U32(x)                       IMX_USDHC_WTMK_LVL_WR_BRST_LEN_BIT(x)

/*!< for Register: BLK_ATT */
/*!<
 * BLKCNT: bit[31:16], Blocks Count For Current Transfer
 *          0xffff: 65535 blocks
 *          0x0002: 2 blocks
 *          0x0001: 1 blocks
 *          0x0000: stop count
 */
#define IMX_USDHC_BLK_ATT_BLKCNT_OFFSET                             (16U)
#define IMX_USDHC_BLK_ATT_BLKCNT_MASK                               (0xffff0000U)
#define IMX_USDHC_BLK_ATT_BLKCNT_BIT(x)                             mr_bit_mask((x), IMX_USDHC_BLK_ATT_BLKCNT_MASK, IMX_USDHC_BLK_ATT_BLKCNT_OFFSET)
#define IMX_USDHC_BLK_ATT_BLKCNT_U32(x)                             IMX_USDHC_BLK_ATT_BLKCNT_BIT(x)

/*!<
 * BLKSIZE: bit[12:0], Transfer Block Size
 *          0x1000(4096 Bytes), 0x800(2048 Bytes), 0x200(512 Bytes), 0x1ff(511 Bytes), 0x004(4 Bytes)
 *          and 0x003, 0x002, 0x001, 0x000(no data transfer)
 */
#define IMX_USDHC_BLK_ATT_BLKSIZE_OFFSET                            (0U)
#define IMX_USDHC_BLK_ATT_BLKSIZE_MASK                              (0x00000fffU)
#define IMX_USDHC_BLK_ATT_BLKSIZE_BIT(x)                            mr_bit_mask((x), IMX_USDHC_BLK_ATT_BLKSIZE_MASK, IMX_USDHC_BLK_ATT_BLKSIZE_OFFSET)
#define IMX_USDHC_BLK_ATT_BLKSIZE_U32(x)                            IMX_USDHC_BLK_ATT_BLKSIZE_BIT(x)

/*!< for Register: VEND_SPEC */
enum  __ERT_IMX_USDHC_VEND_SPEC_BIT
{
    NR_ImxUsdhc_VendSpec_ExtDmaRequestEnable = mr_bit(0U),         /*!< External DMA Request Enable */
    NR_ImxUsdhc_VendSpec_VoltageSelect = mr_bit(1U),               /*!< Voltage Selection */
    NR_ImxUsdhc_VendSpec_ConflictCheckEnable = mr_bit(2U),         /*!< Conflict check enable. */
    NR_ImxUsdhc_VendSpec_CheckBusyAfterAC12WR = mr_bit(3U),        /*!< Check busy enable after auto CMD12 for write data packet */
    NR_ImxUsdhc_VendSpec_Data3DetectPolarity = mr_bit(4U),         /*!< Polarity of DATA3 pin when it is used as card detection. Only for debug */
    NR_ImxUsdhc_VendSpec_CD_B_Polarity = mr_bit(5U),               /*!< Polarity of the CD_B pin. Only for debug */
    NR_ImxUsdhc_VendSpec_WriteProtectPolarity = mr_bit(6U),        /*!< Polarity of the WP pin. Only for debug */
    NR_ImxUsdhc_VendSpec_ActiveClkInAbortSend = mr_bit(7U),        /*!< Force CLK output active when sending Abort command. Only for debug */
    NR_ImxUsdhc_VendSpec_ActiveClk = mr_bit(8U),                   /*!< Force CLK output active */
    NR_ImxUsdhc_VendSpec_IpgClkSoftEnable = mr_bit(11U),           /*!< IPG_CLK Software Enable */
    NR_ImxUsdhc_VendSpec_AhbClkSoftEnable = mr_bit(12U),           /*!< AHB Clock Software Enable */
    NR_ImxUsdhc_VendSpec_IpgPerClkSoftEnable = mr_bit(13U),        /*!< IPG_PERCLK Software Enable */
    NR_ImxUsdhc_VendSpec_CardClkSoftEnable = mr_bit(14U),          /*!< Card Clock Software Enable */
    NR_ImxUsdhc_VendSpec_CrcCheckDisable = mr_bit(15U),            /*!< CRC Check Disable */

    NR_ImxUsdhc_VendSpec_CmdByteAccess = mr_bit(31U),              /*!< Byte access */
};


#ifdef __cplusplus
    }
#endif

#endif /* __IMX6UL_USDHC_H */
