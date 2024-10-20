/*
 * ZYNQ7 Board Terminal Initial
 *
 * File Name:   zynq7_console.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.06.22
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include "zynq7_common.h"

/*!< The globals */
static XUartPs sgrt_ps7_xuart_ps_data;

/*!< API function */
/*!
 * @brief   initial and start serial
 * @param   none
 * @retval  none
 * @note    none
 */
void zynq7_console_init(void)
{
    XUartPs *sprt_uart;
    XUartPs_Config *sprt_cfg;

    sprt_uart = &sgrt_ps7_xuart_ps_data;

    sprt_cfg = XUartPs_LookupConfig(XPAR_PS7_UART_0_DEVICE_ID);
    if (!isValid(sprt_cfg))
        return;

    XUartPs_CfgInitialize(sprt_uart, sprt_cfg, sprt_cfg->BaseAddress);
    XUartPs_SetBaudRate(sprt_uart, 115200);
}

/*!
 * @brief   io_putc
 * @param   none
 * @retval  none
 * @note    printk typedef
 */
void io_putc(const kubyte_t ch)
{
    XUartPs *sprt_uart;

    sprt_uart = &sgrt_ps7_xuart_ps_data;
    XUartPs_Send(sprt_uart, (kuint8_t *)&ch, 1);
}

/*!
 * @brief   io_putstr
 * @param   none
 * @retval  none
 * @note    printk typedef
 */
void io_putstr(const kubyte_t *msgs, kusize_t size)
{
    XUartPs *sprt_uart;

    sprt_uart = &sgrt_ps7_xuart_ps_data;
    XUartPs_Send(sprt_uart, (kuint8_t *)msgs, size);
}

/* end of file*/