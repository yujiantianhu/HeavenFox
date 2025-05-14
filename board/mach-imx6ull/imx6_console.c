/*
 * IMX6ULL Board UART Initial
 *
 * File Name:   imx6_console.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/io_stream.h>
#include "imx6_common.h"

/*!< The defines */
/*!< Mux */
#define IMX_CONSOLE_MUX_SELECT_TX                           IMX6UL_MUX_UART1_TX_DATA_UART1_DCE_TX
#define IMX_CONSOLE_MUX_SELECT_RX                           IMX6UL_MUX_UART1_RX_DATA_UART1_DCE_RX

/*!< CCM */
#define IMX_CONSOLE_CLK_CG_REG								CG12
#define IMX_CONSOLE_CLK_SELECT								IMX6UL_CCM_CCGR_CLOCK_ENTRY(5)
#define IMX_CONSOLE_CLK_PORT_ENTRY()						IMX6UL_CCM_PROPERTY_ENTRY()

/*!< Pin */
#define IMX_CONSOLE_PORT_ENTRY()                        	IMX6UL_UART_PROPERTY_ENTRY(1)

/*!< the functions */

/*!< API function */
/*!
 * @brief   imx_console_putc
 * @param   none
 * @retval  none
 * @note    send character
 */
static void imx_console_putc(const kubyte_t ch)
{
    srt_imx_uart_t *sptr_Uart;

    sptr_Uart = IMX_CONSOLE_PORT_ENTRY();

    /*!< Send Data */
    mr_writel(ch & 0Xff, &sptr_Uart->UTXD);

    /*!< 
     * Wait for sending finished
     * bit3: 0: Transmit is incomplete; 1: Transmit is complete
     */
    while (!mr_isBitSetl(mr_bit(3), &sptr_Uart->USR2));
}

/*!
 * @brief   imx_console_putstr
 * @param   none
 * @retval  none
 * @note    printk typedef
 */
static void imx_console_putstr(const kubyte_t *msgs, kusize_t size)
{
    for (kusize_t i = 0; i < size; i++)
        imx_console_putc(*(msgs + i));
}

/*!
 * @brief   imx_console_getc
 * @param   none
 * @retval  none
 * @note    get character
 */
static kubyte_t imx_console_getc(kubyte_t *ch)
{
    srt_imx_uart_t *sptr_Uart;
    kubyte_t val;

    sptr_Uart = IMX_CONSOLE_PORT_ENTRY();

    /*!<
     * Wait for receiving ready
     * bit0: 0: No receive data ready; 1: Receive data ready
     */
    while (!mr_isBitSetl(mr_bit(0), &sptr_Uart->USR2));

    /*!< Receive Data */
    val = mr_readl(&sptr_Uart->URXD);
    if (ch)
        *ch = val;

    return val;
}

/*!
 * @brief   imx_console_getstr
 * @param   string
 * @retval  none
 * @note    string read
 */
static kssize_t imx_console_getstr(kubyte_t *msgs, kusize_t size)
{
    return 0;
}

/*!<
 * UCR1
 *	bit[31:16]: reserved
 *	
 *	The following bits are all enabled by writing 1, and disabled by writing 0.
 *
 *	bit15: ADEN. Enables/Disables the automatic baud rate detect complete (ADET) bit to generate an interrupt (interrupt_uart = 0)
 *	bit14: ABBR. Enables/Disables automatic baud rate detection
 *	bit13: TRDYEN. Enables/Disables the transmitter Ready Interrupt (TRDY) when the transmitter has one or more slots available in the TxFIFO
 *	bit9: RRDYEN. Enables/Disables the RRDY interrupt when the RxFIFO contains data
 *	bit8: RXDMAEN. Enables/Disables the receive DMA request dma_req_rx when the receiver has data in the RxFIFO
 *	bit7: IREN. Enables/Disables the Infrared(IR) interface
 *	bit6: TXMPTYEN. Enables/Disables the transmitter FIFO empty (TXFE) interrupt
 *	bit4: SNDBRK. Forces the transmitter to send a BREAK character
 *	bit3: TXDMAEN. Enables/Disables the transmit DMA request dma_req_tx when the transmitter has one or more slots available in the TxFIFO
 * 	bit0: UARTEN. Enables/Disables the UART
 */

/*!<
 * UCR2
 *	bit[31:16]: reserved
 *	bit15: ESCI. Enable(1)/Disable(0) the escape sequence interrupt
 *	bit14: IRTS. 0: Transmit only when the RTS pin is asserted; 1: Ignore the RTS pin
 *	bit13: CTSC. 0: The CTS_B pin is controlled by the CTS bit; 1: The CTS_B pin is controlled by the receiver
 *	bit12: CTS. Controls the CTS_B pin when the CTSC bit is negated. 0: The CTS_B pin is high (inactive); 1: The CTS_B pin is low (active)
 *	bit11: ESCEN. Enables/Disables the escape sequence detection logic.
 *	bit[10:9]: RTEC. Request to Send Edge Control
 *	bit8: PREN. Enables/Disables the parity generator in the transmitter and parity checker in the receiver
 *	bit7: PROE. Parity Odd/Even. 0: Even parity; 1: Odd parity
 *	bit6: STPB. 0: The transmitter sends 1 stop bit. The receiver expects 1 or more stop bits
 *				1: The transmitter sends 2 stop bits. The receiver expects 2 or more stop bits
 *	bit5: WS. 	0: 7-bit transmit and receive character length (not including START, STOP or PARITY bits)
 *				1: 8-bit transmit and receive character length (not including START, STOP or PARITY bits)
 *	bit4: RTSEN. Request to Send Interrupt Enable/Disable
 *	bit3: ATEN. Aging Timer Enable/Disable
 *	bit2: TXEN. Enables/Disables the transmitter
 *	bit1: RXEN. Enables/Disables the receiver
 *	bit0: SRST. 0: Reset the transmit and receive state machines, all FIFOs and register USR1, USR2, UBIR, UBMR, UBRC , URXD, UTXD and UTS[6-3]
 *				1: No reset
 */

/*!<
 * UCR3
 *	bit[31:16]: reserved
 *	bit[15:14]: DTR/DSR Interrupt Edge Control
 *	bit13: Data Terminal Ready Interrupt Enable
 *	bit12: Parity Error Interrupt Enable
 *	bit11: Frame Error Interrupt Enable
 *	bit10: Data Set Ready
 *	bit9: Data Carrier Detect
 *	bit8: Ring Indicator
 *	bit7: 0: Autobaud detection new features selected; 1: Keep old autobaud detection mechanism
 * 	bit6: Receive Status Interrupt Enable
 * 	bit5: Asynchronous IR WAKE Interrupt Enable
 * 	bit4: Asynchronous WAKE Interrupt Enable
 * 	bit3: Data Terminal Ready Delta Enable
 * 	bit2: RXD Muxed Input Selected
 * 			<In I.MX6ULL, UARTs are used in MUXED mode, so that this bit should always be set>
 * 	bit1: Invert TXD output in RS-232/RS-485 mode, set TXD active level in IrDA mode
 * 	bit0: Autobaud Counter Interrupt Enable
 */

/*!<
 * UFCR
 *	bit[31:16]: reserved
 *	bit[15:10]: Transmitter Trigger Level. TxFIFO has "0x000010 ~ 0x10000" or few characters
 *	bit[9:7]:	Reference Frequency Divider. Controls the divide ratio for the reference clock. The input clock is
 *				module_clock. The output from the divider is ref_clk which is used by BRM to create the 16x baud rate
 *				oversampling clock (brm_clk).
 *				000, Divide input clock by 6
 *				001, Divide input clock by 5
 *				...
 *				101, Divide input clock by 1
 *				110, Divide input clock by 7
 *	bit6:		0: DCE mode selected; 1: DTE mode selected
 *	bit[5:0]:	Receiver Trigger Level. RxFIFO has "0x000001 ~ 0x10000" characters		
 */

/*!
 * @brief   imx6ull_console_clk_init
 * @param   none
 * @retval  none
 * @note    initial UART
 */
static void imx6ull_console_clk_init(void)
{
    srt_hal_imx_ccm_t *sptr_clkRegs;

    sptr_clkRegs = IMX_CONSOLE_CLK_PORT_ENTRY();

    /*!< initial clock */
    /*!< 
     * UART CLK should be 80MHz, it is from osc or pll3_sw_clk
     * pll3_sw_clk needs to configure "CCSR->pll3_sw_clk_sel", if select pll3, freq is 480MHz
     * pll3_sw_clk ---> divide by 6, if "CSCDR1->UART_CLK_PODF" = 0 ---> UART_Freq = 480MHz / 6 / 1 = 80MHz
     */
    mr_clrbitl(mr_bit(6), &sptr_clkRegs->CSCDR1);
    mr_clrbitl(0x3f, &sptr_clkRegs->CSCDR1);

    /*!< enable uart1 clock */
    mr_imx_ccm_clk_enable(IMX_CONSOLE_CLK_CG_REG, IMX_CONSOLE_CLK_SELECT);
}

/*!< io stream reality */
static struct io_stream_dev sgtc_imx6_io_stream =
{
    .name = CONFIG_CONSOLE_DEVICE,

    ._putc = imx_console_putc,
    ._putstr = imx_console_putstr,
    ._getc = imx_console_getc,
    ._getstr = imx_console_getstr,
};

/*!
 * @brief   imx6ull_console_init
 * @param   none
 * @retval  none
 * @note    initial UART
 */
void imx6ull_console_init(void)
{
    srt_hal_imx_pin_t  sgtc_txPort;
    srt_hal_imx_pin_t  sgtc_rxPort;
    urt_imx_io_ctl_pad_t ugtr_ioPad;
    srt_imx_uart_t *sptr_Uart;
    struct io_stream_dev *sptr_stream;

    sptr_Uart = IMX_CONSOLE_PORT_ENTRY();

    /*!< initial clk */
    imx6ull_console_clk_init();

    /*!< select serial peripheral */
    mr_write_urt_data(&ugtr_ioPad, 
                        IMX6UL_IO_CTL_PAD_PKE_BIT(IMX6UL_IO_CTL_PAD_PKE_ENABLE)  |
                        IMX6UL_IO_CTL_PAD_PUS_BIT(IMX6UL_IO_CTL_PAD_PUS_100K_UP) |
                        IMX6UL_IO_CTL_PAD_SPEED_BIT(IMX6UL_IO_CTL_PAD_SPEED_100MHZ) |
                        IMX6UL_IO_CTL_PAD_DSE_BIT(IMX6UL_IO_CTL_PAD_DSE_RDIV(6)));
    hal_imx_pin_attribute_init(&sgtc_txPort, IMX6UL_PIN_ADDR_BASE, 
                        IMX_CONSOLE_MUX_SELECT_TX, mr_trans_urt_data(&ugtr_ioPad), IMX6UL_PIN_MUX_FUNC_DISABLE);
    hal_imx_pin_attribute_init(&sgtc_rxPort, IMX6UL_PIN_ADDR_BASE, 
                        IMX_CONSOLE_MUX_SELECT_RX, mr_trans_urt_data(&ugtr_ioPad), IMX6UL_PIN_MUX_FUNC_DISABLE);

    hal_imx_pin_mux_configure(&sgtc_txPort);
    hal_imx_pin_mux_configure(&sgtc_rxPort);
    
    hal_imx_pin_pad_configure(&sgtc_txPort);
    hal_imx_pin_pad_configure(&sgtc_rxPort);
    
    /*!< Disable Uart, and clear all settings (including automatic baud rate detection) */
    mr_writel(0, &sptr_Uart->UCR1);

    /*!< Reset the Tx and Rx state machines, all FIFOs and USR1/2, UBIR, UBMR, UBRC, URXD, UTXD, UTS */
    mr_clrbitl(mr_bit(0), &sptr_Uart->UCR2);
    /*!< Wating for reseting finished */
//	while (!mr_isBitResetl(mr_bit(0), &sptr_Uart->UCR2));

    /*!<
     * bit14: Ignore RTS Pin
     * bit8: Disable parity generator and checker (Not Odd/Even)
     * bit6: The transmitter sends 1 stop bit. The receiver expects 1 or more stop bits
     * bit5: 8-bit transmit and receive character length (not including START, STOP or PARITY bits)
     * bit2: TXEN. Enable the transmitter
     * bit1: RXEN. Enable the receiver
     */
    mr_clrbitl(mr_bit(8)  | mr_bit(6), &sptr_Uart->UCR2);
    mr_setbitl(mr_bit(14) | mr_bit(5) | mr_bit(2) | mr_bit(1), &sptr_Uart->UCR2);

    /*!<
     * RXD Muxed Input Selected
     * 	<In I.MX6ULL, UARTs are used in MUXED mode, so that this bit should always be set>
     */
    mr_setbitl(mr_bit(2), &sptr_Uart->UCR3); 

    /*!<
     * Set Baud Rate
     * Baud Rate = Ref_Freq / (16 * (UBMR + 1) / (UBIR + 1)), because uart_clk = 80MHz:
     * if Baud Rate is set to be 115200bps
     * ===> UFCR bit[9:7] = 101 (Divide input clock by 1), So Ref_Freq = uart_clk / 1 = 80MHz
     * ===> UBMR = 3124
     * ===> UBIR = 71
     * Baud Rate = 80000000 / (16 * (3124 + 1) / (71 + 1)) = (80000000 * 72) / (16 * 3125) = 115200
     */
    mr_setbitl(mr_bit(9) | mr_bit(7), &sptr_Uart->UFCR);
    mr_writel(71, &sptr_Uart->UBIR);
    mr_writel(3124, &sptr_Uart->UBMR);

    /*!< Enable Uart */
    mr_setbitl(mr_bit(0), &sptr_Uart->UCR1);

    /*!< Register IO Stream */
    sptr_stream = &sgtc_imx6_io_stream;
    init_list_head(&sptr_stream->sgtc_link);
    sptr_stream->is_opened = true;

    register_io_stream(sptr_stream);
}

/* end of file */
