/*
 * IMX6ULL CPU part of UART Configure
 *
 * File Name:   imx6ull_uart.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.05.05
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __IMX6UL_UART_H
#define __IMX6UL_UART_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <imx6/imx6ull_periph.h>

/*!< The defines */
/*!< URXD Regitser */
#define IMX_UART_URXD_RX_DATA_OFFSET                        (0U)

/*!
 * RXDATA:  Received Data. Holds the received character. In 7-bit mode, the most significant bit (MSB) is forced to 0.
 *          In 8-bit mode, all bits are active.
 * PRERR:   In RS-485 mode, it holds the ninth data bit (bit [8]) of received 9-bit RS-485 data;
 *          In RS232/IrDA mode, it is the Parity Error flag
 *          0 = No parity error was detected for data in the RX_DATA field
 *          1 = A parity error was detected for data in the RX_DATA field
 * BRK:     BREAK Detect.
 *          0 The current character is not a BREAK character
 *          1 The current character is a BREAK character
 * FRMERR:  Frame Error
 *          0 The current character has no framing error
 *          1 The current character has a framing error
 * OVRRUN:  Receiver Overrun
 *          0 No RxFIFO overrun was detected
 *          1 A RxFIFO overrun was detected
 * ERR:     Error Detect. Indicates whether the character present in the RX_DATA field has an error (OVRRUN,
 *          FRMERR, BRK or PRERR) status
 *          0 No error status was detected
 *          1 An error status was detected
 * CHARRDY: Character Ready
 *          0 Character in RX_DATA field and associated flags are invalid.
 *          1 Character in RX_DATA field and associated flags valid and ready for reading.
 */
enum __ERT_IMX_UART_URXD_MASK
{
    NR_IMX_UART_URXD_RX_DATA = mr_bit_nr(0xFFU, IMX_UART_URXD_RX_DATA_OFFSET),
    NR_IMX_UART_URXD_PRERR = mr_bit(10U),
    NR_IMX_UART_URXD_BRK = mr_bit(11U),
    NR_IMX_UART_URXD_FRMERR = mr_bit(12U),
    NR_IMX_UART_URXD_OVRRUN = mr_bit(13U),
    NR_IMX_UART_URXD_ERR = mr_bit(14U),
    NR_IMX_UART_URXD_CHARRDY = mr_bit(15U),
};

/*!< UTXD Regitser */
#define IMX_UART_URXD_TX_DATA_OFFSET                        (0U)

enum __ERT_IMX_UART_UTXD_MASK
{
    /*!< Transmit Data */
    NR_IMX_UART_URXD_TX_DATA = mr_bit_nr(0xFFU, IMX_UART_URXD_TX_DATA_OFFSET),
};

/*!< UCR1 Regitser */
#define IMX_UART_UCR1_ICD_OFFSET                            (10U)

/*!
 * UARTEN:  UART Enable (1: Enable; 0: Disable)
 * DOZE:    Determines the UART enable condition in the DOZE state
 *          0 The UART is enabled when in DOZE state
 *          1 The UART is disabled when in DOZE state
 * ATDMAEN: Aging DMA Timer Enable (1: Enable; 0: Disable)
 * TXDMAEN: Transmitter Ready DMA Enable (1: Enable; 0: Disable)
 * SNDBRK:  Send BREAK
 *          0 Do not send a BREAK character
 *          1 Send a BREAK character (continuous 0s)
 * RTSDEN:  RTS Delta Interrupt Enable (1: Enable; 0: Disable)
 * TXMPTYEN:Transmitter Empty Interrupt Enable (1: Enable; 0: Disable)
 * IREN:    Infrared Interface Enable (1: Enable; 0: Disable)
 * RXDMAEN: Receive Ready DMA Enable (1: Enable; 0: Disable)
 * RRDYEN:  Receiver Ready Interrupt Enable (1: Enable; 0: Disable)
 * ICD:     Idle Condition Detect
 * IDEN:    Idle Condition Detected Interrupt Enable (1: Enable; 0: Disable)
 * TRDYEN:  Transmitter Ready Interrupt Enable (1: Enable; 0: Disable)
 * ADBR:    Automatic Detection of Baud Rate (1: Enable; 0: Disable)
 * ADEN:    Automatic Baud Rate Detection Interrupt Enable (1: Enable; 0: Disable)
 */
enum __ERT_IMX_UART_UCR1_MASK
{
    NR_IMX_UART_UCR1_UARTEN = mr_bit(0U),
    NR_IMX_UART_UCR1_DOZE = mr_bit(1U),
    NR_IMX_UART_UCR1_ATDMAEN = mr_bit(2U),
    NR_IMX_UART_UCR1_TXDMAEN = mr_bit(3U),
    NR_IMX_UART_UCR1_SNDBRK = mr_bit(4U),
    NR_IMX_UART_UCR1_RTSDEN = mr_bit(5U),
    NR_IMX_UART_UCR1_TXMPTYEN = mr_bit(6U),
    NR_IMX_UART_UCR1_IREN = mr_bit(7U),
    NR_IMX_UART_UCR1_RXDMAEN = mr_bit(8U),
    NR_IMX_UART_UCR1_RRDYEN = mr_bit(9U),
    NR_IMX_UART_UCR1_ICD = mr_bit_nr(0x3U, IMX_UART_UCR1_ICD_OFFSET),
    NR_IMX_UART_UCR1_IDEN = mr_bit(12U),
    NR_IMX_UART_UCR1_TRDYEN = mr_bit(13U),
    NR_IMX_UART_UCR1_ADBR = mr_bit(14U),
    NR_IMX_UART_UCR1_ADEN = mr_bit(15U),
};

#define IMX_UART_UCR1_ICD_4FRAME                            (((0U) << IMX_UART_UCR1_ICD_OFFSET) & NR_IMX_UART_UCR1_ICD)
#define IMX_UART_UCR1_ICD_8FRAME                            (((1U) << IMX_UART_UCR1_ICD_OFFSET) & NR_IMX_UART_UCR1_ICD)
#define IMX_UART_UCR1_ICD_16FRAME                           (((2U) << IMX_UART_UCR1_ICD_OFFSET) & NR_IMX_UART_UCR1_ICD)
#define IMX_UART_UCR1_ICD_32FRAME                           (((3U) << IMX_UART_UCR1_ICD_OFFSET) & NR_IMX_UART_UCR1_ICD)

/*!< UCR2 Regitser */
#define IMX_UART_UCR2_RTEC_OFFSET                           (9U)

/*!
 * SRST:    Software Reset
 *          0 Reset the transmit and receive state machines, all FIFOs and register USR1, USR2, UBIR, UBMR,
 *            UBRC , URXD, UTXD and UTS[6-3].
 *          1 No reset
 * RXEN:    Receiver Enable (1: Enable; 0: Disable)
 * TXEN:    Transmitter Enable (1: Enable; 0: Disable)
 *          (The transmitter FIFO cannot be written when this bit is cleared)
 * ATEN:    Aging Timer Enable (1: Enable; 0: Disable)
 * RTSEN:   Request to Send Interrupt Enable (1: Enable; 0: Disable)
 * WS:      Word Size. Controls the character length
 *          0 7-bit transmit and receive character length (not including START, STOP or PARITY bits)
 *          1 8-bit transmit and receive character length (not including START, STOP or PARITY bits)
 * STPB:    Stop. Controls the number of stop bits after a character
 *          0 The transmitter sends 1 stop bit. The receiver expects 1 or more stop bits.
 *          1 The transmitter sends 2 stop bits. The receiver expects 2 or more stop bits.
 * PROE:    Parity Odd/Even
 *          0 Even parity
 *          1 Odd parity
 * PREN:    Parity Enable (1: Enable; 0: Disable)
 * RTEC:    Request to Send Edge Control
 * ESCEN:   Escape Enable (1: Enable; 0: Disable)
 * CTS:     Clear to Send. Controls the CTS_B pin when the CTSC bit is negated
 *          0 The CTS_B pin is high (inactive)
 *          1 The CTS_B pin is low (active)
 * CTSC:    CTS Pin Control. Controls the operation of the CTS_B module output
 *          0 The CTS_B pin is controlled by the CTS bit
 *          1 The CTS_B pin is controlled by the receiver
 * IRTS:    Ignore RTS Pin
 *          0 Transmit only when the RTS pin is asserted
 *          1 Ignore the RTS pin
 * ESCI:    Escape Sequence Interrupt Enable
 */
enum __ERT_IMX_UART_UCR2_MASK
{
    NR_IMX_UART_UCR2_SRST = mr_bit(0U),
    NR_IMX_UART_UCR2_RXEN = mr_bit(1U),
    NR_IMX_UART_UCR2_TXEN = mr_bit(2U),
    NR_IMX_UART_UCR2_ATEN = mr_bit(3U),
    NR_IMX_UART_UCR2_RTSEN = mr_bit(4U),
    NR_IMX_UART_UCR2_WS = mr_bit(5U),
    NR_IMX_UART_UCR2_STPB = mr_bit(6U),
    NR_IMX_UART_UCR2_PROE = mr_bit(7U),
    NR_IMX_UART_UCR2_PREN = mr_bit(8U),
    NR_IMX_UART_UCR2_RTEC = mr_bit_nr(0x3U, IMX_UART_UCR2_RTEC_OFFSET),
    NR_IMX_UART_UCR2_ESCEN = mr_bit(11U),
    NR_IMX_UART_UCR2_CTS = mr_bit(12U),
    NR_IMX_UART_UCR2_CTSC = mr_bit(13U),
    NR_IMX_UART_UCR2_IRTS = mr_bit(14U),
    NR_IMX_UART_UCR2_ESCI = mr_bit(15U),
};

/*!
 * 00 Trigger interrupt on a rising edge
 * 01 Trigger interrupt on a falling edge
 * 1X Trigger interrupt on any edge
 */
#define IMX_UART_UCR2_RTEC_RISING                           (((0U) << IMX_UART_UCR2_RTEC_OFFSET) & NR_IMX_UART_UCR2_RTEC)
#define IMX_UART_UCR2_RTEC_FALLING                          (((1U) << IMX_UART_UCR2_RTEC_OFFSET) & NR_IMX_UART_UCR2_RTEC)
#define IMX_UART_UCR2_RTEC_ANY                              (((2U) << IMX_UART_UCR2_RTEC_OFFSET) & NR_IMX_UART_UCR2_RTEC)

/*!< UCR3 Regitser */
#define IMX_UART_UCR3_DPEC_OFFSET                           (14U)

/*!
 * ACIEN:   Autobaud Counter Interrupt Enable
 * INVT:    Invert TXD output in RS-232/RS-485 mode, set TXD active level in IrDA mode
 * RXDMUXSEL:RXD Muxed Input Selected. Selects proper input pins for serial and Infrared input signal
 * DTRDEN:  Data Terminal Ready Delta Enable
 * AWAKEN:  Asynchronous WAKE Interrupt Enable
 * AIRINTEN:Asynchronous IR WAKE Interrupt Enable
 * RXDSEN:  Receive Status Interrupt Enable
 * ADNIMP:  Autobaud Detection Not Improved-. Disables new features of autobaud detection
 * RI:      Ring Indicator(DCE mode or DTE mode)
 * DCD:     Data Carrier Detect
 * DSR:     Data Set Ready
 * FRAERREN:Frame Error Interrupt Enable
 * PARERREN:Parity Error Interrupt Enable
 * DTREN:   Data Terminal Ready Interrupt Enable
 * DPEC:    DTR/DSR Interrupt Edge Control
 */
enum __ERT_IMX_UART_UCR3_MASK
{
    NR_IMX_UART_UCR3_ACIEN = mr_bit(0U),
    NR_IMX_UART_UCR3_INVT = mr_bit(1U),
    NR_IMX_UART_UCR3_RXDMUXSEL = mr_bit(2U),
    NR_IMX_UART_UCR3_DTRDEN = mr_bit(3U),
    NR_IMX_UART_UCR3_AWAKEN = mr_bit(4U),
    NR_IMX_UART_UCR3_AIRINTEN = mr_bit(5U),
    NR_IMX_UART_UCR3_RXDSEN = mr_bit(6U),
    NR_IMX_UART_UCR3_ADNIMP = mr_bit(7U),
    NR_IMX_UART_UCR3_RI = mr_bit(8U),
    NR_IMX_UART_UCR3_DCD = mr_bit(9U),
    NR_IMX_UART_UCR3_DSR = mr_bit(10U),
    NR_IMX_UART_UCR3_FRAERREN = mr_bit(11U),
    NR_IMX_UART_UCR3_PARERREN = mr_bit(12U),
    NR_IMX_UART_UCR3_DTREN = mr_bit(13U),
    NR_IMX_UART_UCR3_DPEC = mr_bit_nr(0x3U, IMX_UART_UCR3_DPEC_OFFSET),
};

/*!
 * 00 interrupt generated on rising edge
 * 01 interrupt generated on falling edge
 * 1X interrupt generated on either edge
 */
#define IMX_UART_UCR3_DPEC_RISING                           (((0U) << IMX_UART_UCR3_DPEC_OFFSET) & NR_IMX_UART_UCR3_DPEC)
#define IMX_UART_UCR3_DPEC_FALLING                          (((1U) << IMX_UART_UCR3_DPEC_OFFSET) & NR_IMX_UART_UCR3_DPEC)
#define IMX_UART_UCR3_DPEC_ANY                              (((2U) << IMX_UART_UCR3_DPEC_OFFSET) & NR_IMX_UART_UCR3_DPEC)

/*!< UCR4 Regitser */
#define IMX_UART_UCR4_CTSTL_OFFSET                          (10U)

/*!
 * DREN:    Receive Data Ready Interrupt Enable
 * OREN:    Receiver Overrun Interrupt Enable
 * BKEN:    BREAK Condition Detected Interrupt Enable
 * TCEN:    TransmitComplete Interrupt Enable
 * LPBYP:   Low Power Bypass
 * IRSC:    IR Special Case
 * IDDMAEN: DMA IDLE Condition Detected Interrupt Enable
 * WKEN:    WAKE Interrupt Enable
 * ENIRI:   Serial Infrared Interrupt Enable
 * INVR:    Invert RXD input in RS-232/RS-485 Mode, determine RXD input logic level being sampled in In IrDA mode
 * CTSTL:   CTS Trigger Level
 */
enum __ERT_IMX_UART_UCR4_MASK
{
    NR_IMX_UART_UCR4_DREN = mr_bit(0U),
    NR_IMX_UART_UCR4_OREN = mr_bit(1U),
    NR_IMX_UART_UCR4_BKEN = mr_bit(2U),
    NR_IMX_UART_UCR4_TCEN = mr_bit(3U),
    NR_IMX_UART_UCR4_LPBYP = mr_bit(4U),
    NR_IMX_UART_UCR4_IRSC = mr_bit(5U),
    NR_IMX_UART_UCR4_IDDMAEN = mr_bit(6U),
    NR_IMX_UART_UCR4_WKEN = mr_bit(7U),
    NR_IMX_UART_UCR4_ENIRI = mr_bit(8U),
    NR_IMX_UART_UCR4_INVR = mr_bit(9U),
    NR_IMX_UART_UCR4_CTSTL = mr_bit_nr(0x3FU, IMX_UART_UCR4_CTSTL_OFFSET),
};

#define IMX_UART_UCR4_CTSTL_RxFIFO(x)                       (((x) << IMX_UART_UCR4_CTSTL_OFFSET) & NR_IMX_UART_UCR4_CTSTL)

/*!< UFCR Regitser */
#define IMX_UART_UFCR_RXTL_OFFSET                           (0U)
#define IMX_UART_UFCR_RFDIV_OFFSET                          (7U)
#define IMX_UART_UFCR_TXTL_OFFSET                           (10U)

/*!
 * RXTL:    Receiver Trigger Level ( Controls the threshold at which a maskable interrupt is generated by the RxFIFO)
 * DCEDTE:  DCE/DTE mode select
 * RFDIV:   Reference Frequency Divider
 * TXTL:    Transmitter Trigger Level (Controls the threshold at which a maskable interrupt is generated by the TxFIFO)
 */
enum __ERT_IMX_UART_UFCR_MASK
{
    NR_IMX_UART_UFCR_RXTL = mr_bit_nr(0x3FU, IMX_UART_UFCR_RXTL_OFFSET),
    NR_IMX_UART_UFCR_DCEDTE = mr_bit(6U),
    NR_IMX_UART_UFCR_RFDIV = mr_bit_nr(0x7U, IMX_UART_UFCR_RFDIV_OFFSET),
    NR_IMX_UART_UFCR_TXTL = mr_bit_nr(0x3FU, IMX_UART_UFCR_TXTL_OFFSET),
};

/*!< 0 and 1 are reserved, Tx/RxFIFO may have x or fewer characters */
#define IMX_UART_UFCR_RXTL_RxFIFO(x)                        (((x) << IMX_UART_UFCR_RXTL_OFFSET) & NR_IMX_UART_UFCR_RXTL)
#define IMX_UART_UFCR_TXTL_TxFIFO(x)                        (((x) << IMX_UART_UFCR_TXTL_OFFSET) & NR_IMX_UART_UFCR_TXTL)

#define IMX_UART_UFCR_RFDIV_6                               (((0U) << IMX_UART_UFCR_RFDIV_OFFSET) & NR_IMX_UART_UFCR_RFDIV)
#define IMX_UART_UFCR_RFDIV_5                               (((1U) << IMX_UART_UFCR_RFDIV_OFFSET) & NR_IMX_UART_UFCR_RFDIV)
#define IMX_UART_UFCR_RFDIV_4                               (((2U) << IMX_UART_UFCR_RFDIV_OFFSET) & NR_IMX_UART_UFCR_RFDIV)
#define IMX_UART_UFCR_RFDIV_3                               (((3U) << IMX_UART_UFCR_RFDIV_OFFSET) & NR_IMX_UART_UFCR_RFDIV)
#define IMX_UART_UFCR_RFDIV_2                               (((4U) << IMX_UART_UFCR_RFDIV_OFFSET) & NR_IMX_UART_UFCR_RFDIV)
#define IMX_UART_UFCR_RFDIV_1                               (((5U) << IMX_UART_UFCR_RFDIV_OFFSET) & NR_IMX_UART_UFCR_RFDIV)
#define IMX_UART_UFCR_RFDIV_7                               (((6U) << IMX_UART_UFCR_RFDIV_OFFSET) & NR_IMX_UART_UFCR_RFDIV)

/*!< USR1 Regitser */
/*!
 * SAD:     RS-485 Slave Address Detected Interrupt Flag
 * AWAKE:   Asynchronous WAKE Interrupt Flag
 * AIRINT:  Asynchronous IR WAKE Interrupt Flag
 * RXDS:    Receiver IDLE Interrupt Flag
 * DTRD:    DTR Delta
 * AGTIM:   Ageing Timer Interrupt Flag
 * RRDY:    Receiver Ready Interrupt / DMA Flag. Indicates that the RxFIFO data level is above the threshold set by the RXTL bits;
 *          RRDY is automatically cleared when data level in the RxFIFO goes below the set threshold level.
 * FRAMERR: Frame Error Interrupt Flag
 * ESCF:    Escape Sequence Interrupt Flag
 * RTSD:    RTS Delta
 * TRDY:    Transmitter Ready Interrupt / DMA Flag. Indicates that the TxFIFO emptied below its target threshold and requires data;
 *          TRDY is automatically cleared when the data level in the TxFIFO exceeds the threshold set by TXTL bits
 * RTSS:    RTS_B Pin Status
 * PARITYERR:Parity Error Interrupt Flag
 */
enum __ERT_IMX_UART_USR1_MASK
{
    NR_IMX_UART_USR1_SAD = mr_bit(3U),
    NR_IMX_UART_USR1_AWAKE = mr_bit(4U),
    NR_IMX_UART_USR1_AIRINT = mr_bit(5U),
    NR_IMX_UART_USR1_RXDS = mr_bit(6U),
    NR_IMX_UART_USR1_DTRD = mr_bit(7U),
    NR_IMX_UART_USR1_AGTIM = mr_bit(8U),
    NR_IMX_UART_USR1_RRDY = mr_bit(9U),
    NR_IMX_UART_USR1_FRAMERR = mr_bit(10U),
    NR_IMX_UART_USR1_ESCF = mr_bit(11U),
    NR_IMX_UART_USR1_RTSD = mr_bit(12U),
    NR_IMX_UART_USR1_TRDY = mr_bit(13U),
    NR_IMX_UART_USR1_RTSS = mr_bit(14U),
    NR_IMX_UART_USR1_PARITYERR = mr_bit(15U),
};

/*!< UART Status Register 2 */
/*!
 * RDR:     Receive Data Ready-Indicates that at least 1 character is received and written to the RxFIFO
 * ORE:     Overrun Error
 * BRCD:    BREAK Condition Detected
 * TXDC:    Transmitter Complete. Indicates that the transmit buffer (TxFIFO) and Shift Register is empty; therefore
 *          the transmission is complete. TXDC is cleared automatically when data is written to the TxFIFO.
 *          0 Transmit is incomplete (TxFIFO is not empty)
 *          1 Transmit is complete   (TxFIFO is empty)
 * RTSF:    RTS Edge Triggered Interrupt Flag
 * DCDIN:   Data Carrier Detect Input
 * DCDDELT: Data Carrier Detect Delta
 * WAKE:    Wake. Indicates the start bit is detected
 * IRINT:   Serial Infrared Interrupt Flag
 * RIIN:    Ring Indicator Input
 * RIDELT:  Ring Indicator Delta
 * ACST:    Autobaud Counter Stopped
 * IDLE:    Idle Condition
 * DTRF:    DTR edge triggered interrupt flag
 * TXFE:    Transmit Buffer FIFO Empty
 * ADET:    Automatic Baud Rate Detect Complete
 */
enum __ERT_IMX_UART_USR2_MASK
{
    NR_IMX_UART_USR2_RDR = mr_bit(0U),
    NR_IMX_UART_USR2_ORE = mr_bit(1U),
    NR_IMX_UART_USR2_BRCD = mr_bit(2U),
    NR_IMX_UART_USR2_TXDC = mr_bit(3U),
    NR_IMX_UART_USR2_RTSF = mr_bit(4U),
    NR_IMX_UART_USR2_DCDIN = mr_bit(5U),
    NR_IMX_UART_USR2_DCDDELT = mr_bit(6U),
    NR_IMX_UART_USR2_WAKE = mr_bit(7U),
    NR_IMX_UART_USR2_IRINT = mr_bit(8U),
    NR_IMX_UART_USR2_RIIN = mr_bit(9U),
    NR_IMX_UART_USR2_RIDELT = mr_bit(10U),
    NR_IMX_UART_USR2_ACST = mr_bit(11U),
    NR_IMX_UART_USR2_IDLE = mr_bit(12U),
    NR_IMX_UART_USR2_DTRF = mr_bit(13U),
    NR_IMX_UART_USR2_TXFE = mr_bit(14U),
    NR_IMX_UART_USR2_ADET = mr_bit(15U),
};

/*!< UART Escape Character Register */
#define IMX_UART_UESC_ESC_CHAR_OFFSET                       (0U)
#define IMX_UART_UESC_ESC_CHAR_MASK                         mr_bit_nr(0xFFU, IMX_UART_UESC_ESC_CHAR_OFFSET)
#define IMX_UART_UESC_ESC_CHAR(x)                           (((x) << IMX_UART_UESC_ESC_CHAR_OFFSET) & IMX_UART_UESC_ESC_CHAR_MASK)

/*!< UART Escape Timer Register */
#define IMX_UART_UTIM_TIM_OFFSET                            (0U)
#define IMX_UART_UTIM_TIM_MASK                              mr_bit_nr(0xFFFU, IMX_UART_UTIM_TIM_OFFSET)
#define IMX_UART_UTIM_TIM(x)                                (((x) << IMX_UART_UTIM_TIM_OFFSET) & IMX_UART_UTIM_TIM_MASK)

/*!< UART BRM Incremental Register */
#define IMX_UART_UBIR_INC_OFFSET                            (0U)
#define IMX_UART_UBIR_INC_MASK                              mr_bit_nr(0xFFFFU, IMX_UART_UBIR_INC_OFFSET)
#define IMX_UART_UBIR_INC(x)                                (((x) << IMX_UART_UBIR_INC_OFFSET) & IMX_UART_UBIR_INC_MASK)

/*!< UART BRM Modulator Register */
#define IMX_UART_UBMR_INC_OFFSET                            (0U)
#define IMX_UART_UBMR_INC_MASK                              mr_bit_nr(0xFFFFU, IMX_UART_UBMR_INC_OFFSET)
#define IMX_UART_UBMR_INC(x)                                (((x) << IMX_UART_UBMR_INC_OFFSET) & IMX_UART_UBMR_INC_MASK)

/*!< UART Baud Rate Count Register */
#define IMX_UART_UBRC_BCNT_OFFSET                           (0U)
#define IMX_UART_UBRC_BCNT_MASK                             mr_bit_nr(0xFFFFU, IMX_UART_UBRC_BCNT_OFFSET)
#define IMX_UART_UBRC_BCNT(x)                               (((x) << IMX_UART_UBRC_BCNT_OFFSET) & IMX_UART_UBRC_BCNT_MASK)

/*!< UART Test Register */
enum __ERT_IMX_UART_UTS_MASK
{
    NR_IMX_UART_UTS_SOFTRST = mr_bit(0U),
    NR_IMX_UART_UTS_RXFULL = mr_bit(3U),
    NR_IMX_UART_UTS_TXFULL = mr_bit(4U),
    NR_IMX_UART_UTS_RXEMPTY = mr_bit(5U),
    NR_IMX_UART_UTS_TXEMPTY = mr_bit(6U),
    NR_IMX_UART_UTS_RXDBG = mr_bit(9U),
    NR_IMX_UART_UTS_LOOPIR = mr_bit(10U),
    NR_IMX_UART_UTS_DBGEN = mr_bit(11U),
    NR_IMX_UART_UTS_LOOP = mr_bit(12U),
    NR_IMX_UART_UTS_FRCPERR = mr_bit(13U),
};

/*!< UART RS-485 Mode Control Register */
#define IMX_UART_UMCR_SLADDR_OFFSET                         (8U)

enum __ERT_IMX_UART_UMCR_MASK
{
    NR_IMX_UART_UMCR_MDEN = mr_bit(0U),
    NR_IMX_UART_UMCR_SLAM = mr_bit(1U),
    NR_IMX_UART_UMCR_TXB = mr_bit(2U),
    NR_IMX_UART_UMCR_SADEN = mr_bit(3U),
    NR_IMX_UART_UMCR_SLADDR = mr_bit_nr(0xFFU, 8U),
};

#define IMX_UART_UMCR_BCNT(x)                               (((x) << IMX_UART_UMCR_SLADDR_OFFSET) & NR_IMX_UART_UMCR_SLADDR)

/*!< The defines */
#define mr_imx_uart_send_byte(sptr_uart, ch)                mr_writel((ch) & 0xff, (&(sptr_uart)->UTXD))
#define mr_imx_uart_recv_byte(sptr_uart)                    mr_readl(&((sptr_uart)->URXD))
#define mr_imx_uart_tx_full(sptr_uart)                      mr_isBitSetl(NR_IMX_UART_UTS_TXFULL, &((sptr_uart)->UTS))
#define mr_imx_uart_rx_full(sptr_uart)                      mr_isBitSetl(NR_IMX_UART_UTS_RXFULL, &((sptr_uart)->UTS))
#define mr_imx_uart_tx_empty(sptr_uart)                     mr_isBitSetl(NR_IMX_UART_UTS_TXEMPTY, &((sptr_uart)->UTS))
#define mr_imx_uart_rx_empty(sptr_uart)                     mr_isBitSetl(NR_IMX_UART_UTS_RXEMPTY, &((sptr_uart)->UTS))
#define mr_imx_uart_tx_completed(sptr_uart)                 mr_isBitSetl(NR_IMX_UART_USR2_TXDC, &((sptr_uart)->USR2))
#define mr_imx_uart_rx_ready(sptr_uart)                     mr_isBitSetl(NR_IMX_UART_USR2_RDR, &((sptr_uart)->USR2))

#ifdef __cplusplus
    }
#endif

#endif /* __IMX6UL_UART_H */
