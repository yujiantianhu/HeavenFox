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
static XUartPs sgtc_ps7_xuart_ps_data;

/*!< API function */
/*!
 * @brief   io_putc
 * @param   none
 * @retval  none
 * @note    printk typedef
 */
void zynq7_console_putc(const kubyte_t ch)
{
    XUartPs *sptr_uart;

    sptr_uart = &sgtc_ps7_xuart_ps_data;
    XUartPs_Send(sptr_uart, (kuint8_t *)&ch, 1);
}

/*!
 * @brief   io_putstr
 * @param   none
 * @retval  none
 * @note    printk typedef
 */
void zynq7_console_putstr(const kubyte_t *msgs, kusize_t size)
{
    XUartPs *sptr_uart;
    kusize_t len, offset = 0;

    sptr_uart = &sgtc_ps7_xuart_ps_data;

    while (size)
    {
        len = CMP_MIN2(size, 64);
        XUartPs_Send(sptr_uart, (kuint8_t *)msgs + offset, len);
        mr_delay_nop();

        size -= len;
        offset += len;
    };
}

/*!
 * @brief   io_getc
 * @param   ch
 * @retval  none
 * @note    character read
 */
kubyte_t zynq7_console_getc(kubyte_t *ch)
{
    XUartPs *sptr_uart;
    kubyte_t val;

    sptr_uart = &sgtc_ps7_xuart_ps_data;
    if (XUartPs_Recv(sptr_uart, &val, 1) < 0)
        return 0;

    if (ch)
        *ch = val;

    return val;
}

/*!
 * @brief   io_getstr
 * @param   string
 * @retval  none
 * @note    string read
 */
kssize_t zynq7_console_getstr(kubyte_t *msgs, kusize_t size)
{
    XUartPs *sptr_uart;
    kssize_t retval;
    
    if (!msgs || !size)
        return 0;

    sptr_uart = &sgtc_ps7_xuart_ps_data;
    retval = XUartPs_Recv(sptr_uart, msgs, size - 1);
    if (retval > 0)
        *(msgs + retval) = '\0';

    return retval;
}

/*!< io stream reality */
static struct io_stream_dev sgtc_zynq7_io_stream =
{
    .name = CONFIG_CONSOLE_DEVICE,

    ._putc = zynq7_console_putc,
    ._putstr = zynq7_console_putstr,
    ._getc = zynq7_console_getc,
    ._getstr = zynq7_console_getstr,
};

/*!
 * @brief   initial and start serial
 * @param   none
 * @retval  none
 * @note    none
 */
void zynq7_console_init(void)
{
    XUartPs *sptr_uart;
    XUartPs_Config *sptr_cfg;
    struct io_stream_dev *sptr_stream;

    sptr_uart = &sgtc_ps7_xuart_ps_data;

    sptr_cfg = XUartPs_LookupConfig(XPAR_PS7_UART_0_DEVICE_ID);
    if (!isValid(sptr_cfg))
        return;

    XUartPs_CfgInitialize(sptr_uart, sptr_cfg, sptr_cfg->BaseAddress);
    XUartPs_SetBaudRate(sptr_uart, 115200);

    /*!< Register IO Stream */
    sptr_stream = &sgtc_zynq7_io_stream;
    init_list_head(&sptr_stream->sgtc_link);
    sptr_stream->is_opened = true;

    register_io_stream(sptr_stream);
}

/* end of file*/
