/*
 * IMX6ULL CPU part of SDMA Configure
 *
 * File Name:   imx6ull_sdma.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.05.05
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __IMX6UL_SDMA_H
#define __IMX6UL_SDMA_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <imx6/imx6ull_periph.h>

/*!< The defines */
/*!< Peripheral SDMA Register */
/*!< MC0PTR Register */
#define IMX6UL_SDMA_MC0PTR_MC0PTR_U32(x)                            ((kuint32_t)(x))

/*!< INTR Register */
#define IMX6UL_SDMA_INTR_HI_U32(x)                                  ((kuint32_t)(x))

/*!< STOP_STAT Register */
#define IMX6UL_SDMA_STOP_STAT_HE_U32(x)                             ((kuint32_t)(x))

/*!< HSTART Register */
#define IMX6UL_SDMA_HSTART_HSTART_HE_U32(x)                         ((kuint32_t)(x))

/*!< EVTOVR Register */
#define IMX6UL_SDMA_EVTOVR_EO_U32(x)                                ((kuint32_t)(x))

/*!< DSPOVR Register */
#define IMX6UL_SDMA_DSPOVR_DO_U32(x)                                ((kuint32_t)(x))

/*!< HOSTOVR Register */
#define IMX6UL_SDMA_HOSTOVR_HO_U32(x)                               ((kuint32_t)(x))

/*!< EVTPEND Register */
#define IMX6UL_SDMA_EVTPEND_EP_U32(x)                               ((kuint32_t)(x))

/*!< EVTERR Register */
#define IMX6UL_SDMA_EVTERR_CHNERR_U32(x)                            ((kuint32_t)(x))

/*!< INTRMASK Register */
#define IMX6UL_SDMA_INTRMASK_HIMASK_U32(x)                          ((kuint32_t)(x))

/*!< EVTERRDBG Register */
#define IMX6UL_SDMA_EVTERRDBG_CHNERR_U32(x)                         ((kuint32_t)(x))

/*!< RESET Register */
#define IMX6UL_SDMA_RESET_RESET_MASK                                (0x1U)
#define IMX6UL_SDMA_RESET_RESET_OFFSET                              (0U)
#define IMX6UL_SDMA_RESET_RESET_BIT(x)                              mr_bit_mask(x, IMX6UL_SDMA_RESET_RESET_MASK, IMX6UL_SDMA_RESET_RESET_OFFSET)

#define IMX6UL_SDMA_RESET_RESCHED_MASK                              (0x1U)
#define IMX6UL_SDMA_RESET_RESCHED_OFFSET                            (0U)
#define IMX6UL_SDMA_RESET_RESCHED_BIT(x)                            mr_bit_mask(x, IMX6UL_SDMA_RESET_RESCHED_MASK, IMX6UL_SDMA_RESET_RESCHED_OFFSET)

/*!< PSW-Schedule Register */
#define IMX6UL_SDMA_PSW_CCR_MASK                                    (0xFU)
#define IMX6UL_SDMA_PSW_CCR_OFFSET                                  (0U)
#define IMX6UL_SDMA_PSW_CCR(x)                                      mr_bit_mask(x, IMX6UL_SDMA_PSW_CCR_MASK, IMX6UL_SDMA_PSW_CCR_OFFSET)
#define IMX6UL_SDMA_PSW_CCP_MASK                                    (0xF0U)
#define IMX6UL_SDMA_PSW_CCP_OFFSET                                  (4U)
#define IMX6UL_SDMA_PSW_CCP(x)                                      mr_bit_mask(x, IMX6UL_SDMA_PSW_CCP_MASK, IMX6UL_SDMA_PSW_CCP_OFFSET)
#define IMX6UL_SDMA_PSW_NCR_MASK                                    (0x1F00U)
#define IMX6UL_SDMA_PSW_NCR_OFFSET                                  (8U)
#define IMX6UL_SDMA_PSW_NCR(x)                                      mr_bit_mask(x, IMX6UL_SDMA_PSW_NCR_MASK, IMX6UL_SDMA_PSW_NCR_OFFSET)
#define IMX6UL_SDMA_PSW_NCP_MASK                                    (0xE000U)
#define IMX6UL_SDMA_PSW_NCP_OFFSET                                  (13U)
#define IMX6UL_SDMA_PSW_NCP(x)                                      mr_bit_mask(x, IMX6UL_SDMA_PSW_NCP_MASK, IMX6UL_SDMA_PSW_NCP_OFFSET)

/*!< CONFIG Register */
#define IMX6UL_SDMA_CONFIG_CSM_MASK                                 (0x3U)
#define IMX6UL_SDMA_CONFIG_CSM_OFFSET                               (0U)
#define IMX6UL_SDMA_CONFIG_CSM(x)                                   mr_bit_mask(x, IMX6UL_SDMA_CONFIG_CSM_MASK, IMX6UL_SDMA_CONFIG_CSM_OFFSET)
#define IMX6UL_SDMA_CONFIG_ACR_MASK                                 (0x10U)
#define IMX6UL_SDMA_CONFIG_ACR_OFFSET                               (4U)
#define IMX6UL_SDMA_CONFIG_ACR(x)                                   mr_bit_mask(x, IMX6UL_SDMA_CONFIG_ACR_MASK, IMX6UL_SDMA_CONFIG_ACR_OFFSET)
#define IMX6UL_SDMA_CONFIG_RTDOBS_MASK                              (0x800U)
#define IMX6UL_SDMA_CONFIG_RTDOBS_OFFSET                            (11U)
#define IMX6UL_SDMA_CONFIG_RTDOBS(x)                                mr_bit_mask(x, IMX6UL_SDMA_CONFIG_RTDOBS_MASK, IMX6UL_SDMA_CONFIG_RTDOBS_OFFSET)
#define IMX6UL_SDMA_CONFIG_DSPDMA_MASK                              (0x1000U)
#define IMX6UL_SDMA_CONFIG_DSPDMA_OFFSET                            (12U)
#define IMX6UL_SDMA_CONFIG_DSPDMA(x)                                mr_bit_mask(x, IMX6UL_SDMA_CONFIG_DSPDMA_MASK, IMX6UL_SDMA_CONFIG_DSPDMA_OFFSET)

/*!< SDMA_LOCK Register */
#define IMX6UL_SDMA_LOCK_LOCK_MASK                                  (0x1U)
#define IMX6UL_SDMA_LOCK_LOCK_OFFSET                                (0U)
#define IMX6UL_SDMA_LOCK_LOCK(x)                                    mr_bit_mask(x, IMX6UL_SDMA_LOCK_LOCK_MASK, IMX6UL_SDMA_LOCK_LOCK_OFFSET)
#define IMX6UL_SDMA_LOCK_SRESET_LOCK_CLR_MASK                       (0x2U)
#define IMX6UL_SDMA_LOCK_SRESET_LOCK_CLR_OFFSET                     (1U)
#define IMX6UL_SDMA_LOCK_SRESET_LOCK_CLR(x)                         mr_bit_mask(x, IMX6UL_SDMA_LOCK_SRESET_LOCK_CLR_MASK, IMX6UL_SDMA_LOCK_SRESET_LOCK_CLR_OFFSET)

/*!< ONCE_ENB Register */
#define IMX6UL_SDMA_ONCE_ENB_ENB_MASK                               (0x1U)
#define IMX6UL_SDMA_ONCE_ENB_ENB_OFFSET                             (0U)
#define IMX6UL_SDMA_ONCE_ENB_ENB(x)                                 mr_bit_mask(x, IMX6UL_SDMA_ONCE_ENB_ENB_MASK, IMX6UL_SDMA_ONCE_ENB_ENB_OFFSET)

/*!< ONCE_DATA Register */
#define IMX6UL_SDMA_ONCE_DATA_U32(x)                                mr_bit_mask(x, IMX6UL_SDMA_ONCE_DATA_DATA_MASK, IMX6UL_SDMA_ONCE_DATA_DATA_OFFSET)

/*!< ONCE_INSTR Register */
#define IMX6UL_SDMA_ONCE_INSTR_INSTR_MASK                           (0xFFFFU)
#define IMX6UL_SDMA_ONCE_INSTR_INSTR_OFFSET                         (0U)
#define IMX6UL_SDMA_ONCE_INSTR_INSTR(x)                             mr_bit_mask(x, IMX6UL_SDMA_ONCE_INSTR_INSTR_MASK, IMX6UL_SDMA_ONCE_INSTR_INSTR_OFFSET)

/*!< ONCE_STAT Register */
#define IMX6UL_SDMA_ONCE_STAT_ECDR_MASK                             (0x7U)
#define IMX6UL_SDMA_ONCE_STAT_ECDR_OFFSET                           (0U)
#define IMX6UL_SDMA_ONCE_STAT_ECDR(x)                               mr_bit_mask(x, IMX6UL_SDMA_ONCE_STAT_ECDR_MASK, IMX6UL_SDMA_ONCE_STAT_ECDR_OFFSET)
#define IMX6UL_SDMA_ONCE_STAT_MST_MASK                              (0x80U)
#define IMX6UL_SDMA_ONCE_STAT_MST_OFFSET                            (7U)
#define IMX6UL_SDMA_ONCE_STAT_MST(x)                                mr_bit_mask(x, IMX6UL_SDMA_ONCE_STAT_MST_MASK, IMX6UL_SDMA_ONCE_STAT_MST_OFFSET)
#define IMX6UL_SDMA_ONCE_STAT_SWB_MASK                              (0x100U)
#define IMX6UL_SDMA_ONCE_STAT_SWB_OFFSET                            (8U)
#define IMX6UL_SDMA_ONCE_STAT_SWB(x)                                mr_bit_mask(x, IMX6UL_SDMA_ONCE_STAT_SWB_MASK, IMX6UL_SDMA_ONCE_STAT_SWB_OFFSET)
#define IMX6UL_SDMA_ONCE_STAT_ODR_MASK                              (0x200U)
#define IMX6UL_SDMA_ONCE_STAT_ODR_OFFSET                            (9U)
#define IMX6UL_SDMA_ONCE_STAT_ODR(x)                                mr_bit_mask(x, IMX6UL_SDMA_ONCE_STAT_ODR_MASK, IMX6UL_SDMA_ONCE_STAT_ODR_OFFSET)
#define IMX6UL_SDMA_ONCE_STAT_EDR_MASK                              (0x400U)
#define IMX6UL_SDMA_ONCE_STAT_EDR_OFFSET                            (10U)
#define IMX6UL_SDMA_ONCE_STAT_EDR(x)                                mr_bit_mask(x, IMX6UL_SDMA_ONCE_STAT_EDR_MASK, IMX6UL_SDMA_ONCE_STAT_EDR_OFFSET)
#define IMX6UL_SDMA_ONCE_STAT_RCV_MASK                              (0x800U)
#define IMX6UL_SDMA_ONCE_STAT_RCV_OFFSET                            (11U)
#define IMX6UL_SDMA_ONCE_STAT_RCV(x)                                mr_bit_mask(x, IMX6UL_SDMA_ONCE_STAT_RCV_MASK, IMX6UL_SDMA_ONCE_STAT_RCV_OFFSET)
#define IMX6UL_SDMA_ONCE_STAT_PST_MASK                              (0xF000U)
#define IMX6UL_SDMA_ONCE_STAT_PST_OFFSET                            (12U)
#define IMX6UL_SDMA_ONCE_STAT_PST(x)                                mr_bit_mask(x, IMX6UL_SDMA_ONCE_STAT_PST_MASK, IMX6UL_SDMA_ONCE_STAT_PST_OFFSET)

/*!< ONCE_CMD Register */
#define IMX6UL_SDMA_ONCE_CMD_CMD_MASK                               (0xFU)
#define IMX6UL_SDMA_ONCE_CMD_CMD_OFFSET                             (0U)
#define IMX6UL_SDMA_ONCE_CMD_CMD(x)                                 mr_bit_mask(x, IMX6UL_SDMA_ONCE_CMD_CMD_MASK, IMX6UL_SDMA_ONCE_CMD_CMD_OFFSET)

/*!< ILLINSTADDR Register */
#define IMX6UL_SDMA_ILLINSTADDR_MASK                                (0x3FFFU)
#define IMX6UL_SDMA_ILLINSTADDR_OFFSET                              (0U)
#define IMX6UL_SDMA_ILLINSTADDR(x)                                  mr_bit_mask(x, IMX6UL_SDMA_ILLINSTADDR_ILLINSTADDR_MASK, IMX6UL_SDMA_ILLINSTADDR_ILLINSTADDR_OFFSET)

/*!< CHN0ADDR Register */
#define IMX6UL_SDMA_CHN0ADDR_CHN0ADDR_MASK                          (0x3FFFU)
#define IMX6UL_SDMA_CHN0ADDR_CHN0ADDR_OFFSET                        (0U)
#define IMX6UL_SDMA_CHN0ADDR_CHN0ADDR(x)                            mr_bit_mask(x, IMX6UL_SDMA_CHN0ADDR_CHN0ADDR_MASK, IMX6UL_SDMA_CHN0ADDR_CHN0ADDR_OFFSET)
#define IMX6UL_SDMA_CHN0ADDR_SMSZ_MASK                              (0x4000U)
#define IMX6UL_SDMA_CHN0ADDR_SMSZ_OFFSET                            (14U)
#define IMX6UL_SDMA_CHN0ADDR_SMSZ(x)                                mr_bit_mask(x, IMX6UL_SDMA_CHN0ADDR_SMSZ_MASK, IMX6UL_SDMA_CHN0ADDR_SMSZ_OFFSET)

/*!< EVT_MIRROR Register */
#define IMX6UL_SDMA_EVT_MIRROR_EVENTS_U32(x)                        ((kuint32_t)(x))

/*!< EVT_MIRROR2 Register */
#define IMX6UL_SDMA_EVT_MIRROR2_EVENTS_MASK                         (0xFFFFU)
#define IMX6UL_SDMA_EVT_MIRROR2_EVENTS_OFFSET                       (0U)
#define IMX6UL_SDMA_EVT_MIRROR2_EVENTS(x)                           mr_bit_mask(x, IMX6UL_SDMA_EVT_MIRROR2_EVENTS_MASK, IMX6UL_SDMA_EVT_MIRROR2_EVENTS_OFFSET)

/*!< XTRIG_CONF1 Register */
#define IMX6UL_SDMA_XTRIG_CONF1_NUM0_MASK                           (0x3FU)
#define IMX6UL_SDMA_XTRIG_CONF1_NUM0_OFFSET                         (0U)
#define IMX6UL_SDMA_XTRIG_CONF1_NUM0(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF1_NUM0_MASK, IMX6UL_SDMA_XTRIG_CONF1_NUM0_OFFSET)
#define IMX6UL_SDMA_XTRIG_CONF1_CNF0_MASK                           (0x40U)
#define IMX6UL_SDMA_XTRIG_CONF1_CNF0_OFFSET                         (6U)
#define IMX6UL_SDMA_XTRIG_CONF1_CNF0(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF1_CNF0_MASK, IMX6UL_SDMA_XTRIG_CONF1_CNF0_OFFSET)
#define IMX6UL_SDMA_XTRIG_CONF1_NUM1_MASK                           (0x3F00U)
#define IMX6UL_SDMA_XTRIG_CONF1_NUM1_OFFSET                         (8U)
#define IMX6UL_SDMA_XTRIG_CONF1_NUM1(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF1_NUM1_MASK, IMX6UL_SDMA_XTRIG_CONF1_NUM1_OFFSET)
#define IMX6UL_SDMA_XTRIG_CONF1_CNF1_MASK                           (0x4000U)
#define IMX6UL_SDMA_XTRIG_CONF1_CNF1_OFFSET                         (14U)
#define IMX6UL_SDMA_XTRIG_CONF1_CNF1(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF1_CNF1_MASK, IMX6UL_SDMA_XTRIG_CONF1_CNF1_OFFSET)
#define IMX6UL_SDMA_XTRIG_CONF1_NUM2_MASK                           (0x3F0000U)
#define IMX6UL_SDMA_XTRIG_CONF1_NUM2_OFFSET                         (16U)
#define IMX6UL_SDMA_XTRIG_CONF1_NUM2(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF1_NUM2_MASK, IMX6UL_SDMA_XTRIG_CONF1_NUM2_OFFSET)
#define IMX6UL_SDMA_XTRIG_CONF1_CNF2_MASK                           (0x400000U)
#define IMX6UL_SDMA_XTRIG_CONF1_CNF2_OFFSET                         (22U)
#define IMX6UL_SDMA_XTRIG_CONF1_CNF2(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF1_CNF2_MASK, IMX6UL_SDMA_XTRIG_CONF1_CNF2_OFFSET)
#define IMX6UL_SDMA_XTRIG_CONF1_NUM3_MASK                           (0x3F000000U)
#define IMX6UL_SDMA_XTRIG_CONF1_NUM3_OFFSET                         (24U)
#define IMX6UL_SDMA_XTRIG_CONF1_NUM3(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF1_NUM3_MASK, IMX6UL_SDMA_XTRIG_CONF1_NUM3_OFFSET)
#define IMX6UL_SDMA_XTRIG_CONF1_CNF3_MASK                           (0x40000000U)
#define IMX6UL_SDMA_XTRIG_CONF1_CNF3_OFFSET                         (30U)
#define IMX6UL_SDMA_XTRIG_CONF1_CNF3(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF1_CNF3_MASK, IMX6UL_SDMA_XTRIG_CONF1_CNF3_OFFSET)

/*!< XTRIG_CONF2 Register */
#define IMX6UL_SDMA_XTRIG_CONF2_NUM4_MASK                           (0x3FU)
#define IMX6UL_SDMA_XTRIG_CONF2_NUM4_OFFSET                         (0U)
#define IMX6UL_SDMA_XTRIG_CONF2_NUM4(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF2_NUM4_MASK, IMX6UL_SDMA_XTRIG_CONF2_NUM4_OFFSET)
#define IMX6UL_SDMA_XTRIG_CONF2_CNF4_MASK                           (0x40U)
#define IMX6UL_SDMA_XTRIG_CONF2_CNF4_OFFSET                         (6U)
#define IMX6UL_SDMA_XTRIG_CONF2_CNF4(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF2_CNF4_MASK, IMX6UL_SDMA_XTRIG_CONF2_CNF4_OFFSET)
#define IMX6UL_SDMA_XTRIG_CONF2_NUM5_MASK                           (0x3F00U)
#define IMX6UL_SDMA_XTRIG_CONF2_NUM5_OFFSET                         (8U)
#define IMX6UL_SDMA_XTRIG_CONF2_NUM5(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF2_NUM5_MASK, IMX6UL_SDMA_XTRIG_CONF2_NUM5_OFFSET)
#define IMX6UL_SDMA_XTRIG_CONF2_CNF5_MASK                           (0x4000U)
#define IMX6UL_SDMA_XTRIG_CONF2_CNF5_OFFSET                         (14U)
#define IMX6UL_SDMA_XTRIG_CONF2_CNF5(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF2_CNF5_MASK, IMX6UL_SDMA_XTRIG_CONF2_CNF5_OFFSET)
#define IMX6UL_SDMA_XTRIG_CONF2_NUM6_MASK                           (0x3F0000U)
#define IMX6UL_SDMA_XTRIG_CONF2_NUM6_OFFSET                         (16U)
#define IMX6UL_SDMA_XTRIG_CONF2_NUM6(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF2_NUM6_MASK, IMX6UL_SDMA_XTRIG_CONF2_NUM6_OFFSET)
#define IMX6UL_SDMA_XTRIG_CONF2_CNF6_MASK                           (0x400000U)
#define IMX6UL_SDMA_XTRIG_CONF2_CNF6_OFFSET                         (22U)
#define IMX6UL_SDMA_XTRIG_CONF2_CNF6(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF2_CNF6_MASK, IMX6UL_SDMA_XTRIG_CONF2_CNF6_OFFSET)
#define IMX6UL_SDMA_XTRIG_CONF2_NUM7_MASK                           (0x3F000000U)
#define IMX6UL_SDMA_XTRIG_CONF2_NUM7_OFFSET                         (24U)
#define IMX6UL_SDMA_XTRIG_CONF2_NUM7(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF2_NUM7_MASK, IMX6UL_SDMA_XTRIG_CONF2_NUM7_OFFSET)
#define IMX6UL_SDMA_XTRIG_CONF2_CNF7_MASK                           (0x40000000U)
#define IMX6UL_SDMA_XTRIG_CONF2_CNF7_OFFSET                         (30U)
#define IMX6UL_SDMA_XTRIG_CONF2_CNF7(x)                             mr_bit_mask(x, IMX6UL_SDMA_XTRIG_CONF2_CNF7_MASK, IMX6UL_SDMA_XTRIG_CONF2_CNF7_OFFSET)

/*!< SDMA_CHNPRI Register */
#define IMX6UL_SDMA_CHNPRI_CHNPRIn_MASK                             (0x7U)
#define IMX6UL_SDMA_CHNPRI_CHNPRIn_OFFSET                           (0U)
#define IMX6UL_SDMA_CHNPRI_CHNPRIn(x)                               mr_bit_mask(x, IMX6UL_SDMA_SDMA_CHNPRI_CHNPRIn_MASK, IMX6UL_SDMA_SDMA_CHNPRI_CHNPRIn_OFFSET)
/* The count of IMX6UL_SDMA_SDMA_CHNPRI */
#define IMX6UL_SDMA_CHNPRI_COUNT                                    (32U)

/*!< CHNENBL Register */
#define IMX6UL_SDMA_CHNENBL_ENBLn(x)                                ((kuint32_t)(x))
/*!< The count of IMX6UL_SDMA_CHNENBL */
#define IMX6UL_SDMA_CHNENBL_COUNT                                   (48U)

/*!< ------------------------------------------------------------------------------ */
/*!< SDMA module channel number. */
#define IMX6UL_SDMA_MODULE_CHANNEL                                  (32)
/*!< SDMA module event number. */
#define IMX6UL_SDMA_EVENT_NUM                                       (48)

/*!< SDMA ROM memory to memory script start address. */
#define IMX6UL_SDMA_M2M_ADDR                                        (642)
/*!< SDMA ROM peripheral to memory script start address. */
#define IMX6UL_SDMA_P2M_ADDR                                        (683)
/*!< SDMA ROM memory to peripheral script start address. */
#define IMX6UL_SDMA_M2P_ADDR                                        (747)
/*!< SDMA ROM uart to memory script start address. */
#define IMX6UL_SDMA_UART2M_ADDR                                     (817)
/*!< SDMA ROM peripheral on SPBA to memory script start address. */
#define IMX6UL_SDMA_SHP2M_ADDR                                      (891)
/*!< SDMA ROM memory to peripheral on SPBA script start address. */
#define IMX6UL_SDMA_M2SHP_ADDR                                      (960)
/*!< SDMA ROM UART on SPBA to memory script start address. */
#define IMX6UL_SDMA_UARTSH2M_ADDR                                   (1032)
/*!< SDMA ROM SPDIF to memory script start address. */
#define IMX6UL_SDMA_SPDIF2M_ADDR                                    (1100)
/*!< SDMA ROM memory to SPDIF script start address. */
#define IMX6UL_SDMA_M2SPDIF_ADDR                                    (1134)

struct imx_sdma_script_addr 
{
	kuint32_t ap_2_ap_addr;
	kuint32_t ap_2_bp_addr;
	kuint32_t ap_2_ap_fixed_addr;
	kuint32_t bp_2_ap_addr;
	kuint32_t loopback_on_dsp_side_addr;
	kuint32_t mcu_interrupt_only_addr;
	kuint32_t firi_2_per_addr;
	kuint32_t firi_2_mcu_addr;
	kuint32_t per_2_firi_addr;
	kuint32_t mcu_2_firi_addr;
	kuint32_t uart_2_per_addr;
	kuint32_t uart_2_mcu_addr;
	kuint32_t per_2_app_addr;
	kuint32_t mcu_2_app_addr;
	kuint32_t per_2_per_addr;
	kuint32_t uartsh_2_per_addr;
	kuint32_t uartsh_2_mcu_addr;
	kuint32_t per_2_shp_addr;
	kuint32_t mcu_2_shp_addr;
	kuint32_t ata_2_mcu_addr;
	kuint32_t mcu_2_ata_addr;
	kuint32_t app_2_per_addr;
	kuint32_t app_2_mcu_addr;
	kuint32_t shp_2_per_addr;
	kuint32_t shp_2_mcu_addr;
	kuint32_t mshc_2_mcu_addr;
	kuint32_t mcu_2_mshc_addr;
	kuint32_t spdif_2_mcu_addr;
	kuint32_t mcu_2_spdif_addr;
	kuint32_t asrc_2_mcu_addr;
	kuint32_t ext_mem_2_ipu_addr;
	kuint32_t descrambler_addr;
	kuint32_t dptc_dvfs_addr;
	kuint32_t utra_addr;
	kuint32_t ram_code_start_addr;
	/*!< End of v1 array */

	kuint32_t mcu_2_ssish_addr;
	kuint32_t ssish_2_mcu_addr;
	kuint32_t hdmi_dma_addr;
	/*!< End of v2 array */

	kuint32_t zcanfd_2_mcu_addr;
	kuint32_t zqspi_2_mcu_addr;
	kuint32_t mcu_2_ecspi_addr;
	/*!< End of v3 array */

	kuint32_t mcu_2_zqspi_addr;
	/*!< End of v4 array */
};

/*!< This enumerates peripheral types. Used for SDMA.*/
enum __ERT_IMX_DMA_PERIPH_TYPE 
{
    NR_IMX_DMATYPE_SSI = 0,	                                        /*!< MCU domain SSI */
    NR_IMX_DMATYPE_SSI_SP,	                                        /*!< Shared SSI: SPI */
    NR_IMX_DMATYPE_MMC,	                                            /*!< MMC */
    NR_IMX_DMATYPE_SDHC,	                                        /*!< SDHC */
    NR_IMX_DMATYPE_UART,	                                        /*!< MCU domain UART */
    NR_IMX_DMATYPE_UART_SP,	                                        /*!< Shared UART */
    NR_IMX_DMATYPE_FIRI,	                                        /*!< FIRI */
    NR_IMX_DMATYPE_CSPI,	                                        /*!< MCU domain CSPI */
    NR_IMX_DMATYPE_CSPI_SP,	                                        /*!< Shared CSPI */
    NR_IMX_DMATYPE_SIM,	                                            /*!< SIM */
    NR_IMX_DMATYPE_ATA,	                                            /*!< ATA */
    NR_IMX_DMATYPE_CCM,	                                            /*!< CCM */
    NR_IMX_DMATYPE_EXT,	                                            /*!< External peripheral */
    NR_IMX_DMATYPE_MSHC,	                                        /*!< Memory Stick Host Controller */
    NR_IMX_DMATYPE_MSHC_SP,	                                        /*!< Shared Memory Stick Host Controller */
    NR_IMX_DMATYPE_DSP,	                                            /*!< DSP */
    NR_IMX_DMATYPE_MEMORY,	                                        /*!< Memory */
    NR_IMX_DMATYPE_FIFO_MEMORY,                                     /*!< FIFO type Memory */
    NR_IMX_DMATYPE_SPDIF,	                                        /*!< SPDIF */
    NR_IMX_DMATYPE_IPU_MEMORY,	                                    /*!< IPU Memory */
    NR_IMX_DMATYPE_ASRC,	                                        /*!< ASRC */
    NR_IMX_DMATYPE_ESAI,	                                        /*!< ESAI */
    NR_IMX_DMATYPE_SSI_DUAL,	                                    /*!< SSI Dual FIFO */
    NR_IMX_DMATYPE_ASRC_SP,	                                        /*!< Shared ASRC */
    NR_IMX_DMATYPE_SAI,	                                            /*!< SAI */
    NR_IMX_DMATYPE_HDMI,	                                        /*!< HDMI Audio */
};

enum __ERT_IMX_DMA_PRIO 
{
    NR_DMA_PRIO_HIGH = 0,
    NR_DMA_PRIO_MEDIUM = 1,
    NR_DMA_PRIO_LOW = 2
};

/*!< For Device Tree */
struct imx6_sdma_dt_data
{
    kint32_t dma_request;                                           /*!< DMA request line */
    kint32_t dma_request2;                                          /*!< secondary DMA request line */
    kuint32_t peripheral_type;                                      /*!< refer to "__ERT_IMX_DMA_PERIPH_TYPE" */
    kint32_t priority;                                              /*!< refer to "__ERT_IMX_DMA_PRIO" */

    kbool_t src_dualfifo;
    kbool_t dst_dualfifo;
};

/*!< API functions */
/*!
 * @brief   set priority of channel
 * @param   sptr_sdma: base address of sdma
 * @param   channel: number
 * @param   priority: current channel's attribute
 * @retval  none
 * @note    set SDMA_CHNPRI; the default value is 0 for all channels, priority 0 will prevents
 *          channel from starting, so the priority must be set before start a channel
 */
static inline void imx_sdma_set_channel_priority(srt_imx_sdma_t *sptr_sdma, kuint32_t channel, kuint8_t priority)
{
    mr_writel(priority, &sptr_sdma->SDMA_CHNPRI[channel]);
}

/*!
 * @brief   get priority of channel
 * @param   sptr_sdma: base address of sdma
 * @param   channel: number, 0 ~ 32
 * @retval  priority
 * @note    none
 */
static inline kuint32_t imx_sdma_get_channel_priority(srt_imx_sdma_t *sptr_sdma, kuint32_t channel)
{
    return mr_readl(&sptr_sdma->SDMA_CHNPRI[channel]);
}

/*!
 * @brief   set sdma request source mapping channel
 * @param   sptr_sdma: base address of sdma
 * @param   source: request source number
 * @param   mask: channel mask, e.g. 0x3 is channel 0 and 1
 * @retval  none
 * @note    set CHNENBL; sdma supports an event trigger multi-channel. 
 *          A channel can also be triggered by several source events
 */
static inline void imx_sdma_set_source_channel(srt_imx_sdma_t *sptr_sdma, kuint32_t source, kuint32_t mask)
{
    mr_setbitl(mask, &sptr_sdma->CHNENBL[source]);
}

/*!
 * @brief   reset sdma request source mapping channel
 * @param   sptr_sdma: base address of sdma
 * @param   source: request source number
 * @param   mask: channel mask, e.g. 0x3 is channel 0 and 1
 * @retval  none
 * @note    reset CHNENBL; sdma supports an event trigger multi-channel. 
 *          A channel can also be triggered by several source events
 */
static inline void imx_sdma_clr_source_channel(srt_imx_sdma_t *sptr_sdma, kuint32_t source, kuint32_t mask)
{
    mr_clrbitl(mask, &sptr_sdma->CHNENBL[source]);
}

/*!
 * @brief   get sdma request source mapping channel
 * @param   sptr_sdma: base address of sdma
 * @param   source: request source number
 * @retval  source
 * @note    sdma supports an event trigger multi-channel. 
 *          A channel can also be triggered by several source events
 */
static inline kuint32_t imx_sdma_get_source_channel(srt_imx_sdma_t *sptr_sdma, kuint32_t source)
{
    return mr_readl(&sptr_sdma->CHNENBL[source]);
}

/*!
 * @brief   start a channel by software
 * @param   sptr_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    set HSTART
 */
static inline void imx_sdma_start_channel_software(srt_imx_sdma_t *sptr_sdma, kuint32_t channel)
{
    mr_writel(mr_bit(channel), &sptr_sdma->HSTART);
}

/*!
 * @brief   start a channel by hardware event
 * @param   sptr_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    set EVTPEND
 */
static inline void imx_sdma_start_channel_event(srt_imx_sdma_t *sptr_sdma, kuint32_t channel)
{
    mr_writel(mr_bit(channel), &sptr_sdma->EVTPEND);
}

/*!
 * @brief   stop a channel
 * @param   sptr_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    set STOP_STAT
 */
static inline void imx_sdma_stop_channel(srt_imx_sdma_t *sptr_sdma, kuint32_t channel)
{
    mr_writel(mr_bit(channel), &sptr_sdma->STOP_STAT);
}

/*!
 * @brief   get interrupt status of all channels
 * @param   sptr_sdma: base address of sdma
 * @retval  *INTR
 * @note    get *INTR
 */
static inline kuint32_t imx_sdma_get_int_status(srt_imx_sdma_t *sptr_sdma)
{
    return mr_readl(&sptr_sdma->INTR);
}

/*!
 * @brief   set interrupt status ---> clear one status
 * @param   sptr_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    none
 */
static inline void imx_sdma_clear_channel_int_status(srt_imx_sdma_t *sptr_sdma, kuint32_t channel)
{
    mr_writel(mr_bit(channel), &sptr_sdma->INTR);
}

/*!
 * @brief   set interrupt status ---> clear multiple status
 * @param   sptr_sdma: base address of sdma
 * @param   mask: e.g. 0x3 for channel 0 and 1
 * @retval  none
 * @note    none
 */
static inline void imx_sdma_clear_multiple_int_status(srt_imx_sdma_t *sptr_sdma, kuint32_t mask)
{
    mr_writel(mask, &sptr_sdma->INTR);
}

/*!
 * @brief   set interrupt status ---> clear all status
 * @param   sptr_sdma: base address of sdma
 * @retval  none
 * @note    none
 */
static inline void imx_sdma_clear_all_int_status(srt_imx_sdma_t *sptr_sdma)
{
    kuint32_t mask = mr_readl(&sptr_sdma->INTR);
    mr_writel(mask, &sptr_sdma->INTR);
}

/*!
 * @brief   get stop status of all channels
 * @param   sptr_sdma: base address of sdma
 * @retval  *STOP_STAT
 * @note    get *STOP_STAT
 */
static inline kuint32_t imx_sdma_get_stop_status(srt_imx_sdma_t *sptr_sdma)
{
    return mr_readl(&sptr_sdma->STOP_STAT);
}

/*!
 * @brief   get stop status of one channel
 * @param   sptr_sdma: base address of sdma
 * @retval  *STOP_STAT
 * @note    get *STOP_STAT
 */
static inline kbool_t imx_sdma_get_channel_stop_status(srt_imx_sdma_t *sptr_sdma, kuint32_t channel)
{
    return !!mr_getbitl(mr_bit(channel), &sptr_sdma->STOP_STAT);
}

/*!
 * @brief   set stop status ---> clear one status
 * @param   sptr_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    none
 */
static inline void imx_sdma_clear_channel_stop_status(srt_imx_sdma_t *sptr_sdma, kuint32_t channel)
{
    mr_writel(mr_bit(channel), &sptr_sdma->STOP_STAT);
}

/*!
 * @brief   set stop status ---> clear multiple status
 * @param   sptr_sdma: base address of sdma
 * @param   mask: e.g. 0x3 for channel 0 and 1
 * @retval  none
 * @note    none
 */
static inline void imx_sdma_clear_multiple_stop_status(srt_imx_sdma_t *sptr_sdma, kuint32_t mask)
{
    mr_writel(mask, &sptr_sdma->STOP_STAT);
}

/*!
 * @brief   set stop status ---> clear all status
 * @param   sptr_sdma: base address of sdma
 * @retval  none
 * @note    none
 */
static inline void imx_sdma_clear_all_stop_status(srt_imx_sdma_t *sptr_sdma)
{
    kuint32_t mask = mr_readl(&sptr_sdma->STOP_STAT);
    mr_writel(mask, &sptr_sdma->STOP_STAT);
}

/*!
 * @brief   get pending status of all channels
 * @param   sptr_sdma: base address of sdma
 * @retval  *EVTPEND
 * @note    get *EVTPEND
 */
static inline kuint32_t imx_sdma_get_pend_status(srt_imx_sdma_t *sptr_sdma)
{
    return mr_readl(&sptr_sdma->EVTPEND);
}

/*!
 * @brief   set pending status ---> clear one status
 * @param   sptr_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    none
 */
static inline void imx_sdma_clear_channel_pend_status(srt_imx_sdma_t *sptr_sdma, kuint32_t channel)
{
    mr_writel(mr_bit(channel), &sptr_sdma->EVTPEND);
}

/*!
 * @brief   set pending status ---> clear multiple status
 * @param   sptr_sdma: base address of sdma
 * @param   mask: e.g. 0x3 for channel 0 and 1
 * @retval  none
 * @note    none
 */
static inline void imx_sdma_clear_multiple_pend_status(srt_imx_sdma_t *sptr_sdma, kuint32_t mask)
{
    mr_writel(mask, &sptr_sdma->EVTPEND);
}

/*!
 * @brief   set pending status ---> clear all status
 * @param   sptr_sdma: base address of sdma
 * @retval  none
 * @note    none
 */
static inline void imx_sdma_clear_all_pend_status(srt_imx_sdma_t *sptr_sdma)
{
    kuint32_t mask = mr_readl(&sptr_sdma->EVTPEND);
    mr_writel(mask, &sptr_sdma->EVTPEND);
}

/*!
 * @brief   get error status of all channels
 * @param   sptr_sdma: base address of sdma
 * @retval  *EVTERR
 * @note    get *EVTERR
 */
static inline kuint32_t imx_sdma_get_error_status(srt_imx_sdma_t *sptr_sdma)
{
    return mr_readl(&sptr_sdma->EVTERR);
}

/*!
 * @brief   set context switch mode
 * @param   sptr_sdma: base address of sdma
 * @param   mode: __ERT_IMX_SDMA_SWITCH_MODE
 * @retval  none
 * @note    none
 */
static inline void imx_sdma_set_context_switch_mode(srt_imx_sdma_t *sptr_sdma, kuint32_t mode)
{
    kuint32_t val;

    val = mr_readl(&sptr_sdma->CONFIG) & (~IMX6UL_SDMA_CONFIG_CSM_MASK);
    mr_writel(val | mode, &sptr_sdma->CONFIG);
}

/*!
 * @brief   set context switch mode
 * @param   sptr_sdma: base address of sdma
 * @param   source: request source
 * @retval  *EVT_MIRROR & mr_bit(source)
 * @note    none
 */
static inline kbool_t imx_sdma_get_request_src_status(srt_imx_sdma_t *sptr_sdma, kuint32_t source)
{
    return !!((source < 32U) ? mr_getbitl(mr_bit(source), &sptr_sdma->EVT_MIRROR) : 
                               mr_getbitl(mr_bit(source - 32U), &sptr_sdma->EVT_MIRROR2));
}

#ifdef __cplusplus
    }
#endif

#endif /* __IMX6UL_SDMA_H */
