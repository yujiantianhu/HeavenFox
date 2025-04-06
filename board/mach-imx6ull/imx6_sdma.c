/*
 * IMX6ULL Board SDMA Initial
 *
 * File Name:   imx6_sdma.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.04.06
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include "imx6_common.h"

/*!< The defines */
enum __ERT_IMX_SDMA_SWITCH_MODE
{
    NR_IMX_SDMA_MODE_STATIC = 0U,                   /*!< SDMA context switch mode static */
    NR_IMX_SDMA_MODE_DYNC_LOW_PWR,                  /*!< SDMA context switch mode dynamic with low power */
    NR_IMX_SDMA_MODE_DYNC_NO_LOOP,                  /*!< SDMA context switch mode dynamic with no loop */
    NR_IMX_SDMA_MODE_DYNC,                          /*!< SDMA context switch mode dynamic */
};

/*!< API function */
/*!
 * @brief   start a channel by software
 * @param   sprt_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    set HSTART
 */
void imx_sdma_start_channel(srt_imx_sdma_t *sprt_sdma, kuint32_t channel)
{
    mrt_writel(mrt_bit(channel), &sprt_sdma->HSTART);
}

/*!
 * @brief   start a channel by hardware event
 * @param   sprt_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    set EVTPEND
 */
void imx_sdma_start_channel_event(srt_imx_sdma_t *sprt_sdma, kuint32_t channel)
{
    mrt_writel(mrt_bit(channel), &sprt_sdma->EVTPEND);
}

/*!
 * @brief   stop a channel
 * @param   sprt_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    set STOP_STAT
 */
void imx_sdma_start_channel_event(srt_imx_sdma_t *sprt_sdma, kuint32_t channel)
{
    mrt_writel(mrt_bit(channel), &sprt_sdma->STOP_STAT);
}

/*!
 * @brief   get interrupt status of all channels
 * @param   sprt_sdma: base address of sdma
 * @retval  *INTR
 * @note    get *INTR
 */
kuint32_t imx_sdma_get_int_status(srt_imx_sdma_t *sprt_sdma)
{
    return mrt_readl(&sprt_sdma->INTR);
}

/*!
 * @brief   set interrupt status ---> clear one status
 * @param   sprt_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_channel_int_status(srt_imx_sdma_t *sprt_sdma, kuint32_t channel)
{
    mrt_writel(mrt_bit(channel), &sprt_sdma->INTR);
}

/*!
 * @brief   set interrupt status ---> clear multiple status
 * @param   sprt_sdma: base address of sdma
 * @param   mask: e.g. 0x3 for channel 1 and 2
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_multiple_int_status(srt_imx_sdma_t *sprt_sdma, kuint32_t mask)
{
    mrt_writel(mask, &sprt_sdma->INTR);
}

/*!
 * @brief   set interrupt status ---> clear all status
 * @param   sprt_sdma: base address of sdma
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_all_int_status(srt_imx_sdma_t *sprt_sdma)
{
    kuint32_t mask = mrt_readl(&sprt_sdma->INTR);
    mrt_writel(mask, &sprt_sdma->INTR);
}

/*!
 * @brief   get stop status of all channels
 * @param   sprt_sdma: base address of sdma
 * @retval  *STOP_STAT
 * @note    get *STOP_STAT
 */
kuint32_t imx_sdma_get_stop_status(srt_imx_sdma_t *sprt_sdma)
{
    return mrt_readl(&sprt_sdma->STOP_STAT);
}

/*!
 * @brief   set stop status ---> clear one status
 * @param   sprt_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_channel_stop_status(srt_imx_sdma_t *sprt_sdma, kuint32_t channel)
{
    mrt_writel(mrt_bit(channel), &sprt_sdma->STOP_STAT);
}

/*!
 * @brief   set stop status ---> clear multiple status
 * @param   sprt_sdma: base address of sdma
 * @param   mask: e.g. 0x3 for channel 1 and 2
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_multiple_stop_status(srt_imx_sdma_t *sprt_sdma, kuint32_t mask)
{
    mrt_writel(mask, &sprt_sdma->STOP_STAT);
}

/*!
 * @brief   set stop status ---> clear all status
 * @param   sprt_sdma: base address of sdma
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_all_stop_status(srt_imx_sdma_t *sprt_sdma)
{
    kuint32_t mask = mrt_readl(&sprt_sdma->STOP_STAT);
    mrt_writel(mask, &sprt_sdma->STOP_STAT);
}

/*!
 * @brief   get pending status of all channels
 * @param   sprt_sdma: base address of sdma
 * @retval  *EVTPEND
 * @note    get *EVTPEND
 */
kuint32_t imx_sdma_get_pend_status(srt_imx_sdma_t *sprt_sdma)
{
    return mrt_readl(&sprt_sdma->EVTPEND);
}

/*!
 * @brief   set pending status ---> clear one status
 * @param   sprt_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_channel_pend_status(srt_imx_sdma_t *sprt_sdma, kuint32_t channel)
{
    mrt_writel(mrt_bit(channel), &sprt_sdma->EVTPEND);
}

/*!
 * @brief   set pending status ---> clear multiple status
 * @param   sprt_sdma: base address of sdma
 * @param   mask: e.g. 0x3 for channel 1 and 2
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_multiple_pend_status(srt_imx_sdma_t *sprt_sdma, kuint32_t mask)
{
    mrt_writel(mask, &sprt_sdma->EVTPEND);
}

/*!
 * @brief   set pending status ---> clear all status
 * @param   sprt_sdma: base address of sdma
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_all_pend_status(srt_imx_sdma_t *sprt_sdma)
{
    kuint32_t mask = mrt_readl(&sprt_sdma->EVTPEND);
    mrt_writel(mask, &sprt_sdma->EVTPEND);
}

/*!
 * @brief   get error status of all channels
 * @param   sprt_sdma: base address of sdma
 * @retval  *EVTERR
 * @note    get *EVTERR
 */
kuint32_t imx_sdma_get_error_status(srt_imx_sdma_t *sprt_sdma)
{
    return mrt_readl(&sprt_sdma->EVTERR);
}

/*!< ---------------------------------------------------------------------------- */
/*!
 * @brief   set context switch mode
 * @param   sprt_sdma: base address of sdma
 * @param   mode: __ERT_IMX_SDMA_SWITCH_MODE
 * @retval  none
 * @note    none
 */
void imx_sdma_set_context_switch_mode(srt_imx_sdma_t *sprt_sdma, kuint32_t mode)
{
    kuint32_t val;

    val = mrt_readl(&sprt_sdma->CONFIG) & (~IMX6UL_SDMA_CONFIG_CSM_MASK);
    mrt_writel(val | mode, &sprt_sdma->CONFIG);
}

/* end of file*/
