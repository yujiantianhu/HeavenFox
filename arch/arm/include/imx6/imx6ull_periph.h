/*
 * IMX6ULL CPU part of Peripheral Configure
 *
 * File Name:   imx6ull_periph.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.10.18
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __IMX6UL_PERIPH_H
#define __IMX6UL_PERIPH_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <common/io_stream.h>

/*!< The defines */
/*!< ------------------------------------------------------------------------- */
/*!< Gpio - Register Layout Typedef */
typedef struct hal_imx_gpio
{
    kuint32_t DR;                                                   /*!< GPIO data register, offset: 0x0 */
    kuint32_t GDIR;                                                 /*!< GPIO direction register, offset: 0x4 */
    kuint32_t PSR;                                                  /*!< GPIO pad status register, offset: 0x8 */
    kuint32_t ICR1;                                                 /*!< GPIO interrupt configuration register1, offset: 0xC */
    kuint32_t ICR2;                                                 /*!< GPIO interrupt configuration register2, offset: 0x10 */
    kuint32_t IMR;                                                  /*!< GPIO interrupt mask register, offset: 0x14 */
    kuint32_t ISR;                                                  /*!< GPIO interrupt status register, offset: 0x18 */
    kuint32_t EDGE_SEL;                                             /*!< GPIO edge select register, offset: 0x1C */

} srt_hal_imx_gpio_t;

/*!< Peripheral GPIO base address */
#define IMX6UL_GPIO1_ADDR_BASE                                      (0x209C000u)
#define IMX6UL_GPIO2_ADDR_BASE                                      (0x20A0000u)
#define IMX6UL_GPIO3_ADDR_BASE                                      (0x20A4000u)
#define IMX6UL_GPIO4_ADDR_BASE                                      (0x20A8000u)
#define IMX6UL_GPIO5_ADDR_BASE                                      (0x20AC000u)
#define IMX6UL_GPIO_PROPERTY_ENTRY(x)                               (srt_hal_imx_gpio_t *)IMX6UL_GPIO##x##_ADDR_BASE
#define IMX6UL_GPIO_PIN_OFFSET_BIT(x)                               mr_bit(x)

/*!< ICR1/ICR2 Register */
#define HAL_IMX_ICR_LOW_LEVEL						                (0x00)
#define HAL_IMX_ICR_HIGH_LEVEL						                (0x01)
#define HAL_IMX_ICR_RISING_EDGE						                (0x02)
#define HAL_IMX_ICR_FALLING_EDGE					                (0x03)
#define HAL_IMX_ICR_MASK							                (0x03)

/*!< ------------------------------------------------------------------------- */
/*!< Timer - Register Layout Typedef */
typedef struct hal_imx_gptimer
{
    kuint32_t CR;                                                   /*!< GPT Control Register, offset: 0x0 */
    kuint32_t PR;                                                   /*!< GPT Prescaler Register, offset: 0x4 */
    kuint32_t SR;                                                   /*!< GPT Status Register, offset: 0x8 */
    kuint32_t IR;                                                   /*!< GPT Interrupt Register, offset: 0xC */
    kuint32_t OCR[3];                                               /*!< GPT Output Compare Register 1..GPT Output Compare Register 3, array offset: 0x10, array step: 0x4 */
    kuint32_t ICR[2];                                               /*!< GPT Input Capture Register 1..GPT Input Capture Register 2, array offset: 0x1C, array step: 0x4 */
    kuint32_t CNT;                                                  /*!< GPT Counter Register, offset: 0x24 */
    
} srt_hal_imx_gptimer_t;

/*!< Peripheral GPT base address */
#define IMX6UL_GPT1_ADDR_BASE                                       (0x2098000u)
#define IMX6UL_GPT2_ADDR_BASE                                       (0x20E8000u)
#define IMX6UL_GPT_PROPERTY_ENTRY(x)                                (srt_hal_imx_gptimer_t *)IMX6UL_GPT##x##_ADDR_BASE

/*!< ------------------------------------------------------------------------- */
/*!< WDOG - Register Layout Typedef */
typedef struct 
{
    kuint16_t WCR;                                                  /*!< Watchdog Control Register, offset: 0x0 */
    kuint16_t WSR;                                                  /*!< Watchdog Service Register, offset: 0x2 */
    kuint16_t WRSR;                                                 /*!< Watchdog Reset Status Register, offset: 0x4 */
    kuint16_t WICR;                                                 /*!< Watchdog Interrupt Control Register, offset: 0x6 */
    kuint16_t WMCR;                                                 /*!< Watchdog Miscellaneous Control Register, offset: 0x8 */
} srt_imx_wdog_t;

/*!< Peripheral WDOG base address */
#define IMX6UL_WDOG1_ADDR_BASE                                      (0x20BC000u)
#define IMX6UL_WDOG2_ADDR_BASE                                      (0x20C0000u)
#define IMX6UL_WDOG3_ADDR_BASE                                      (0x21E4000u)
#define IMX6UL_WDOG_PROPERTY_ENTRY(x)                               (srt_imx_wdog_t *)IMX6UL_WDOG##x##_ADDR_BASE

/*!< ------------------------------------------------------------------------- */
/*!< Uart - Register Layout Typedef */
typedef struct 
{
    kuint32_t URXD;                                                 /*!< UART Receiver Register, offset: 0x0 */
    kuint8_t  RESERVED_0[60];               
    kuint32_t UTXD;                                                 /*!< UART Transmitter Register, offset: 0x40 */
    kuint8_t  RESERVED_1[60];               
    kuint32_t UCR1;                                                 /*!< UART Control Register 1, offset: 0x80 */
    kuint32_t UCR2;                                                 /*!< UART Control Register 2, offset: 0x84 */
    kuint32_t UCR3;                                                 /*!< UART Control Register 3, offset: 0x88 */
    kuint32_t UCR4;                                                 /*!< UART Control Register 4, offset: 0x8C */
    kuint32_t UFCR;                                                 /*!< UART FIFO Control Register, offset: 0x90 */
    kuint32_t USR1;                                                 /*!< UART Status Register 1, offset: 0x94 */
    kuint32_t USR2;                                                 /*!< UART Status Register 2, offset: 0x98 */
    kuint32_t UESC;                                                 /*!< UART Escape Character Register, offset: 0x9C */
    kuint32_t UTIM;                                                 /*!< UART Escape Timer Register, offset: 0xA0 */
    kuint32_t UBIR;                                                 /*!< UART BRM Incremental Register, offset: 0xA4 */
    kuint32_t UBMR;                                                 /*!< UART BRM Modulator Register, offset: 0xA8 */
    kuint32_t UBRC;                                                 /*!< UART Baud Rate Count Register, offset: 0xAC */
    kuint32_t ONEMS;                                                /*!< UART One Millisecond Register, offset: 0xB0 */
    kuint32_t UTS;                                                  /*!< UART Test Register, offset: 0xB4 */
    kuint32_t UMCR;                                                 /*!< UART RS-485 Mode Control Register, offset: 0xB8 */
} srt_imx_uart_t;

/*!< Peripheral UART base address */
#define IMX6UL_UART1_ADDR_BASE                                      (0x2020000u)
#define IMX6UL_UART2_ADDR_BASE                                      (0x21E8000u)
#define IMX6UL_UART3_ADDR_BASE                                      (0x21EC000u)
#define IMX6UL_UART4_ADDR_BASE                                      (0x21F0000u)
#define IMX6UL_UART5_ADDR_BASE                                      (0x21F4000u)
#define IMX6UL_UART6_ADDR_BASE                                      (0x21FC000u)
#define IMX6UL_UART7_ADDR_BASE                                      (0x2018000u)
#define IMX6UL_UART8_ADDR_BASE                                      (0x2288000u)
#define IMX6UL_UART_PROPERTY_ENTRY(x)                               (srt_imx_uart_t *)IMX6UL_UART##x##_ADDR_BASE

/*!< ------------------------------------------------------------------------- */
/*!< I2C - Register Layout Typedef */
typedef struct hal_imx_i2c
{
    kuint16_t IADR;                                                 /*!< I2C Address Register, offset: 0x0 */
    kuint8_t  RESERVED_0[2];
    kuint16_t IFDR;                                                 /*!< I2C Frequency Divider Register, offset: 0x4 */
    kuint8_t  RESERVED_1[2];
    kuint16_t I2CR;                                                 /*!< I2C Control Register, offset: 0x8 */
    kuint8_t  RESERVED_2[2];
    kuint16_t I2SR;                                                 /*!< I2C Status Register, offset: 0xC */
    kuint8_t  RESERVED_3[2];
    kuint16_t I2DR;                                                 /*!< I2C Data I/O Register, offset: 0x10 */
} srt_hal_imx_i2c_t;

/*!< Peripheral I2C base address */
#define IMX6UL_I2C1_ADDR_BASE                                       (0x21A0000u)
#define IMX6UL_I2C2_ADDR_BASE                                       (0x21A4000u)
#define IMX6UL_I2C3_ADDR_BASE                                       (0x21A8000u)
#define IMX6UL_I2C4_ADDR_BASE                                       (0x21F8000u)
#define IMX6UL_I2C_PROPERTY_ENTRY(x)                                (srt_hal_imx_i2c_t *)IMX6UL_I2C##x##_ADDR_BASE

/*!< ------------------------------------------------------------------------- */
/*!< ECSPI - Register Layout Typedef */
typedef struct 
{
    kuint32_t RXDATA;                                               /*!< Receive Data Register, offset: 0x0 */
    kuint32_t TXDATA;                                               /*!< Transmit Data Register, offset: 0x4 */
    kuint32_t CONREG;                                               /*!< Control Register, offset: 0x8 */
    kuint32_t CONFIGREG;                                            /*!< Config Register, offset: 0xC */
    kuint32_t INTREG;                                               /*!< Interrupt Control Register, offset: 0x10 */
    kuint32_t DMAREG;                                               /*!< DMA Control Register, offset: 0x14 */
    kuint32_t STATREG;                                              /*!< Status Register, offset: 0x18 */
    kuint32_t PERIODREG;                                            /*!< Sample Period Control Register, offset: 0x1C */
    kuint32_t TESTREG;                                              /*!< Test Control Register, offset: 0x20 */
    kuint8_t  RESERVED_0[28];
    kuint32_t MSGDATA;                                              /*!< Message Data Register, offset: 0x40 */
} srt_imx_ecspi_t;

/*!< Peripheral ECSPI base address */
#define IMX6UL_ECSPI1_ADDR_BASE                                     (0x2008000u)
#define IMX6UL_ECSPI2_ADDR_BASE                                     (0x200C000u)
#define IMX6UL_ECSPI3_ADDR_BASE                                     (0x2010000u)
#define IMX6UL_ECSPI4_ADDR_BASE                                     (0x2014000u)
#define IMX6UL_ECSPI_PROPERTY_ENTRY(x)                              (srt_imx_ecspi_t *)IMX6UL_ECSPI##x##_ADDR_BASE

/*!< ------------------------------------------------------------------------- */
/*!< ENET - Register Layout Typedef */
typedef struct 
{
    kuint8_t  RESERVED_0[4];
    kuint32_t EIR;                                                  /*!< Interrupt Event Register, offset: 0x4 */
    kuint32_t EIMR;                                                 /*!< Interrupt Mask Register, offset: 0x8 */
    kuint8_t  RESERVED_1[4];
    kuint32_t RDAR;                                                 /*!< Receive Descriptor Active Register, offset: 0x10 */
    kuint32_t TDAR;                                                 /*!< Transmit Descriptor Active Register, offset: 0x14 */
    kuint8_t  RESERVED_2[12];
    kuint32_t ECR;                                                  /*!< Ethernet Control Register, offset: 0x24 */
    kuint8_t  RESERVED_3[24];
    kuint32_t MMFR;                                                 /*!< MII Management Frame Register, offset: 0x40 */
    kuint32_t MSCR;                                                 /*!< MII Speed Control Register, offset: 0x44 */
    kuint8_t  RESERVED_4[28];
    kuint32_t MIBC;                                                 /*!< MIB Control Register, offset: 0x64 */
    kuint8_t  RESERVED_5[28];
    kuint32_t RCR;                                                  /*!< Receive Control Register, offset: 0x84 */
    kuint8_t  RESERVED_6[60];
    kuint32_t TCR;                                                  /*!< Transmit Control Register, offset: 0xC4 */
    kuint8_t  RESERVED_7[28];
    kuint32_t PALR;                                                 /*!< Physical Address Lower Register, offset: 0xE4 */
    kuint32_t PAUR;                                                 /*!< Physical Address Upper Register, offset: 0xE8 */
    kuint32_t OPD;                                                  /*!< Opcode/Pause Duration Register, offset: 0xEC */
    kuint32_t TXIC;                                                 /*!< Transmit Interrupt Coalescing Register, offset: 0xF0 */
    kuint8_t  RESERVED_8[12];
    kuint32_t RXIC;                                                 /*!< Receive Interrupt Coalescing Register, offset: 0x100 */
    kuint8_t  RESERVED_9[20];
    kuint32_t IAUR;                                                 /*!< Descriptor Individual Upper Address Register, offset: 0x118 */
    kuint32_t IALR;                                                 /*!< Descriptor Individual Lower Address Register, offset: 0x11C */
    kuint32_t GAUR;                                                 /*!< Descriptor Group Upper Address Register, offset: 0x120 */
    kuint32_t GALR;                                                 /*!< Descriptor Group Lower Address Register, offset: 0x124 */
    kuint8_t  RESERVED_10[28];
    kuint32_t TFWR;                                                 /*!< Transmit FIFO Watermark Register, offset: 0x144 */
    kuint8_t  RESERVED_11[56];
    kuint32_t RDSR;                                                 /*!< Receive Descriptor Ring Start Register, offset: 0x180 */
    kuint32_t TDSR;                                                 /*!< Transmit Buffer Descriptor Ring Start Register, offset: 0x184 */
    kuint32_t MRBR;                                                 /*!< Maximum Receive Buffer Size Register, offset: 0x188 */
    kuint8_t  RESERVED_12[4];
    kuint32_t RSFL;                                                 /*!< Receive FIFO Section Full Threshold, offset: 0x190 */
    kuint32_t RSEM;                                                 /*!< Receive FIFO Section Empty Threshold, offset: 0x194 */
    kuint32_t RAEM;                                                 /*!< Receive FIFO Almost Empty Threshold, offset: 0x198 */
    kuint32_t RAFL;                                                 /*!< Receive FIFO Almost Full Threshold, offset: 0x19C */
    kuint32_t TSEM;                                                 /*!< Transmit FIFO Section Empty Threshold, offset: 0x1A0 */
    kuint32_t TAEM;                                                 /*!< Transmit FIFO Almost Empty Threshold, offset: 0x1A4 */
    kuint32_t TAFL;                                                 /*!< Transmit FIFO Almost Full Threshold, offset: 0x1A8 */
    kuint32_t TIPG;                                                 /*!< Transmit Inter-Packet Gap, offset: 0x1AC */
    kuint32_t FTRL;                                                 /*!< Frame Truncation Length, offset: 0x1B0 */
    kuint8_t  RESERVED_13[12];
    kuint32_t TACC;                                                 /*!< Transmit Accelerator Function Configuration, offset: 0x1C0 */
    kuint32_t RACC;                                                 /*!< Receive Accelerator Function Configuration, offset: 0x1C4 */
    kuint8_t  RESERVED_14[56];
    kuint32_t RMON_T_DROP;                                          /*!< Reserved Statistic Register, offset: 0x200 */
    kuint32_t RMON_T_PACKETS;                                       /*!< Tx Packet Count Statistic Register, offset: 0x204 */
    kuint32_t RMON_T_BC_PKT;                                        /*!< Tx Broadcast Packets Statistic Register, offset: 0x208 */
    kuint32_t RMON_T_MC_PKT;                                        /*!< Tx Multicast Packets Statistic Register, offset: 0x20C */
    kuint32_t RMON_T_CRC_ALIGN;                                     /*!< Tx Packets with CRC/Align Error Statistic Register, offset: 0x210 */
    kuint32_t RMON_T_UNDERSIZE;                                     /*!< Tx Packets Less Than Bytes and Good CRC Statistic Register, offset: 0x214 */
    kuint32_t RMON_T_OVERSIZE;                                      /*!< Tx Packets GT MAX_FL bytes and Good CRC Statistic Register, offset: 0x218 */
    kuint32_t RMON_T_FRAG;                                          /*!< Tx Packets Less Than 64 Bytes and Bad CRC Statistic Register, offset: 0x21C */
    kuint32_t RMON_T_JAB;                                           /*!< Tx Packets Greater Than MAX_FL bytes and Bad CRC Statistic Register, offset: 0x220 */
    kuint32_t RMON_T_COL;                                           /*!< Tx Collision Count Statistic Register, offset: 0x224 */
    kuint32_t RMON_T_P64;                                           /*!< Tx 64-Byte Packets Statistic Register, offset: 0x228 */
    kuint32_t RMON_T_P65TO127;                                      /*!< Tx 65- to 127-byte Packets Statistic Register, offset: 0x22C */
    kuint32_t RMON_T_P128TO255;                                     /*!< Tx 128- to 255-byte Packets Statistic Register, offset: 0x230 */
    kuint32_t RMON_T_P256TO511;                                     /*!< Tx 256- to 511-byte Packets Statistic Register, offset: 0x234 */
    kuint32_t RMON_T_P512TO1023;                                    /*!< Tx 512- to 1023-byte Packets Statistic Register, offset: 0x238 */
    kuint32_t RMON_T_P1024TO2047;                                   /*!< Tx 1024- to 2047-byte Packets Statistic Register, offset: 0x23C */
    kuint32_t RMON_T_P_GTE2048;                                     /*!< Tx Packets Greater Than 2048 Bytes Statistic Register, offset: 0x240 */
    kuint32_t RMON_T_OCTETS;                                        /*!< Tx Octets Statistic Register, offset: 0x244 */
    kuint32_t IEEE_T_DROP;                                          /*!< Reserved Statistic Register, offset: 0x248 */
    kuint32_t IEEE_T_FRAME_OK;                                      /*!< Frames Transmitted OK Statistic Register, offset: 0x24C */
    kuint32_t IEEE_T_1COL;                                          /*!< Frames Transmitted with Single Collision Statistic Register, offset: 0x250 */
    kuint32_t IEEE_T_MCOL;                                          /*!< Frames Transmitted with Multiple Collisions Statistic Register, offset: 0x254 */
    kuint32_t IEEE_T_DEF;                                           /*!< Frames Transmitted after Deferral Delay Statistic Register, offset: 0x258 */
    kuint32_t IEEE_T_LCOL;                                          /*!< Frames Transmitted with Late Collision Statistic Register, offset: 0x25C */
    kuint32_t IEEE_T_EXCOL;                                         /*!< Frames Transmitted with Excessive Collisions Statistic Register, offset: 0x260 */
    kuint32_t IEEE_T_MACERR;                                        /*!< Frames Transmitted with Tx FIFO Underrun Statistic Register, offset: 0x264 */
    kuint32_t IEEE_T_CSERR;                                         /*!< Frames Transmitted with Carrier Sense Error Statistic Register, offset: 0x268 */
    kuint32_t IEEE_T_SQE;                                           /*!< Reserved Statistic Register, offset: 0x26C */
    kuint32_t IEEE_T_FDXFC;                                         /*!< Flow Control Pause Frames Transmitted Statistic Register, offset: 0x270 */
    kuint32_t IEEE_T_OCTETS_OK;                                     /*!< Octet Count for Frames Transmitted w/o Error Statistic Register, offset: 0x274 */
    kuint8_t  RESERVED_15[12];
    kuint32_t RMON_R_PACKETS;                                       /*!< Rx Packet Count Statistic Register, offset: 0x284 */
    kuint32_t RMON_R_BC_PKT;                                        /*!< Rx Broadcast Packets Statistic Register, offset: 0x288 */
    kuint32_t RMON_R_MC_PKT;                                        /*!< Rx Multicast Packets Statistic Register, offset: 0x28C */
    kuint32_t RMON_R_CRC_ALIGN;                                     /*!< Rx Packets with CRC/Align Error Statistic Register, offset: 0x290 */
    kuint32_t RMON_R_UNDERSIZE;                                     /*!< Rx Packets with Less Than 64 Bytes and Good CRC Statistic Register, offset: 0x294 */
    kuint32_t RMON_R_OVERSIZE;                                      /*!< Rx Packets Greater Than MAX_FL and Good CRC Statistic Register, offset: 0x298 */
    kuint32_t RMON_R_FRAG;                                          /*!< Rx Packets Less Than 64 Bytes and Bad CRC Statistic Register, offset: 0x29C */
    kuint32_t RMON_R_JAB;                                           /*!< Rx Packets Greater Than MAX_FL Bytes and Bad CRC Statistic Register, offset: 0x2A0 */
    kuint32_t RMON_R_RESVD_0;                                       /*!< Reserved Statistic Register, offset: 0x2A4 */
    kuint32_t RMON_R_P64;                                           /*!< Rx 64-Byte Packets Statistic Register, offset: 0x2A8 */
    kuint32_t RMON_R_P65TO127;                                      /*!< Rx 65- to 127-Byte Packets Statistic Register, offset: 0x2AC */
    kuint32_t RMON_R_P128TO255;                                     /*!< Rx 128- to 255-Byte Packets Statistic Register, offset: 0x2B0 */
    kuint32_t RMON_R_P256TO511;                                     /*!< Rx 256- to 511-Byte Packets Statistic Register, offset: 0x2B4 */
    kuint32_t RMON_R_P512TO1023;                                    /*!< Rx 512- to 1023-Byte Packets Statistic Register, offset: 0x2B8 */
    kuint32_t RMON_R_P1024TO2047;                                   /*!< Rx 1024- to 2047-Byte Packets Statistic Register, offset: 0x2BC */
    kuint32_t RMON_R_P_GTE2048;                                     /*!< Rx Packets Greater than 2048 Bytes Statistic Register, offset: 0x2C0 */
    kuint32_t RMON_R_OCTETS;                                        /*!< Rx Octets Statistic Register, offset: 0x2C4 */
    kuint32_t IEEE_R_DROP;                                          /*!< Frames not Counted Correctly Statistic Register, offset: 0x2C8 */
    kuint32_t IEEE_R_FRAME_OK;                                      /*!< Frames Received OK Statistic Register, offset: 0x2CC */
    kuint32_t IEEE_R_CRC;                                           /*!< Frames Received with CRC Error Statistic Register, offset: 0x2D0 */
    kuint32_t IEEE_R_ALIGN;                                         /*!< Frames Received with Alignment Error Statistic Register, offset: 0x2D4 */
    kuint32_t IEEE_R_MACERR;                                        /*!< Receive FIFO Overflow Count Statistic Register, offset: 0x2D8 */
    kuint32_t IEEE_R_FDXFC;                                         /*!< Flow Control Pause Frames Received Statistic Register, offset: 0x2DC */
    kuint32_t IEEE_R_OCTETS_OK;                                     /*!< Octet Count for Frames Received without Error Statistic Register, offset: 0x2E0 */
    kuint8_t  RESERVED_16[284];
    kuint32_t ATCR;                                                 /*!< Adjustable Timer Control Register, offset: 0x400 */
    kuint32_t ATVR;                                                 /*!< Timer Value Register, offset: 0x404 */
    kuint32_t ATOFF;                                                /*!< Timer Offset Register, offset: 0x408 */
    kuint32_t ATPER;                                                /*!< Timer Period Register, offset: 0x40C */
    kuint32_t ATCOR;                                                /*!< Timer Correction Register, offset: 0x410 */
    kuint32_t ATINC;                                                /*!< Time-Stamping Clock Period Register, offset: 0x414 */
    kuint32_t ATSTMP;                                               /*!< Timestamp of Last Transmitted Frame, offset: 0x418 */
    kuint8_t  RESERVED_17[488];
    kuint32_t TGSR;                                                 /*!< Timer Global Status Register, offset: 0x604 */

    struct 
    {                                                               /*!< offset: 0x608, array step: 0x8 */
        kuint32_t TCSR;                                             /*!< Timer Control Status Register, array offset: 0x608, array step: 0x8 */
        kuint32_t TCCR;                                             /*!< Timer Compare Capture Register, array offset: 0x60C, array step: 0x8 */
    } CHANNEL[4];

} srt_imx_enet_t;

/*!< Peripheral ENET base address */
#define IMX6UL_ENET1_ADDR_BASE                                      (0x2188000u)
#define IMX6UL_ENET2_ADDR_BASE                                      (0x20B4000u)
#define IMX6UL_ENET_PROPERTY_ENTRY(x)                               (srt_imx_enet_t *)IMX6UL_ENET##x##_ADDR_BASE

/*!< ------------------------------------------------------------------------- */
/*!< USB - Register Layout Typedef */
typedef struct 
{
    kuint32_t ID;                                                   /*!< Identification register, offset: 0x0 */
    kuint32_t HWGENERAL;                                            /*!< Hardware General, offset: 0x4 */
    kuint32_t HWHOST;                                               /*!< Host Hardware Parameters, offset: 0x8 */
    kuint32_t HWDEVICE;                                             /*!< Device Hardware Parameters, offset: 0xC */
    kuint32_t HWTXBUF;                                              /*!< TX Buffer Hardware Parameters, offset: 0x10 */
    kuint32_t HWRXBUF;                                              /*!< RX Buffer Hardware Parameters, offset: 0x14 */
    kuint8_t  RESERVED_0[104];
    kuint32_t GPTIMER0LD;                                           /*!< General Purpose Timer #0 Load, offset: 0x80 */
    kuint32_t GPTIMER0CTRL;                                         /*!< General Purpose Timer #0 Controller, offset: 0x84 */
    kuint32_t GPTIMER1LD;                                           /*!< General Purpose Timer #1 Load, offset: 0x88 */
    kuint32_t GPTIMER1CTRL;                                         /*!< General Purpose Timer #1 Controller, offset: 0x8C */
    kuint32_t SBUSCFG;                                              /*!< System Bus Config, offset: 0x90 */
    kuint8_t  RESERVED_1[108];
    kuint8_t  CAPLENGTH;                                            /*!< Capability Registers Length, offset: 0x100 */
    kuint8_t  RESERVED_2[1];
    kuint16_t HCIVERSION;                                           /*!< Host Controller Interface Version, offset: 0x102 */
    kuint32_t HCSPARAMS;                                            /*!< Host Controller Structural Parameters, offset: 0x104 */
    kuint32_t HCCPARAMS;                                            /*!< Host Controller Capability Parameters, offset: 0x108 */
    kuint8_t  RESERVED_3[20];
    kuint16_t DCIVERSION;                                           /*!< Device Controller Interface Version, offset: 0x120 */
    kuint8_t  RESERVED_4[2];
    kuint32_t DCCPARAMS;                                            /*!< Device Controller Capability Parameters, offset: 0x124 */
    kuint8_t  RESERVED_5[24];
    kuint32_t USBCMD;                                               /*!< USB Command Register, offset: 0x140 */
    kuint32_t USBSTS;                                               /*!< USB Status Register, offset: 0x144 */
    kuint32_t USBINTR;                                              /*!< Interrupt Enable Register, offset: 0x148 */
    kuint32_t FRINDEX;                                              /*!< USB Frame Index, offset: 0x14C */
    kuint8_t  RESERVED_6[4];
  
    union 
    {                                                               /*!< offset: 0x154 */
        kuint32_t DEVICEADDR;                                       /*!< Device Address, offset: 0x154 */
        kuint32_t PERIODICLISTBASE;                                 /*!< Frame List Base Address, offset: 0x154 */
    };

    union 
    {                                                               /*!< offset: 0x158 */
        kuint32_t ASYNCLISTADDR;                                    /*!< Next Asynch. Address, offset: 0x158 */
        kuint32_t ENDPTLISTADDR;                                    /*!< Endpoint List Address, offset: 0x158 */
    };

    kuint8_t  RESERVED_7[4];
    kuint32_t BURSTSIZE;                                            /*!< Programmable Burst Size, offset: 0x160 */
    kuint32_t TXFILLTUNING;                                         /*!< TX FIFO Fill Tuning, offset: 0x164 */
    kuint8_t  RESERVED_8[16];
    kuint32_t ENDPTNAK;                                             /*!< Endpoint NAK, offset: 0x178 */
    kuint32_t ENDPTNAKEN;                                           /*!< Endpoint NAK Enable, offset: 0x17C */
    kuint32_t CONFIGFLAG;                                           /*!< Configure Flag Register, offset: 0x180 */
    kuint32_t PORTSC1;                                              /*!< Port Status & Control, offset: 0x184 */
    kuint8_t  RESERVED_9[28];
    kuint32_t OTGSC;                                                /*!< On-The-Go Status & control, offset: 0x1A4 */
    kuint32_t USBMODE;                                              /*!< USB Device Mode, offset: 0x1A8 */
    kuint32_t ENDPTSETUPSTAT;                                       /*!< Endpoint Setup Status, offset: 0x1AC */
    kuint32_t ENDPTPRIME;                                           /*!< Endpoint Prime, offset: 0x1B0 */
    kuint32_t ENDPTFLUSH;                                           /*!< Endpoint Flush, offset: 0x1B4 */
    kuint32_t ENDPTSTAT;                                            /*!< Endpoint Status, offset: 0x1B8 */
    kuint32_t ENDPTCOMPLETE;                                        /*!< Endpoint Complete, offset: 0x1BC */
    kuint32_t ENDPTCTRL0;                                           /*!< Endpoint Control0, offset: 0x1C0 */
    kuint32_t ENDPTCTRL[7];                                         /*!< Endpoint Control 1..Endpoint Control 7, array offset: 0x1C4, array step: 0x4 */
} srt_imx_usbotg_t;

/*!< Peripheral USBOTG base address */
#define IMX6UL_USBOTG1_ADDR_BASE                                    (0x2184000u)
#define IMX6UL_USBOTG2_ADDR_BASE                                    (0x2184200u)
#define IMX6UL_USBOTG_PROPERTY_ENTRY(x)                             (srt_imx_usbotg_t *)IMX6UL_USBOTG##x##_ADDR_BASE

typedef enum __ERT_IMX_USBOTG_INTR_BIT
{
    NR_ImxUsbOtgIntr_UsbIntBit = mr_bit(0U),
    NR_ImxUsbOtgIntr_UsbErrIntBit = mr_bit(1U),
    NR_ImxUsbOtgIntr_PortChangeDetectIntBit = mr_bit(2U),
    NR_ImxUsbOtgIntr_FrameRolloverIntBit = mr_bit(3U),
    NR_ImxUsbOtgIntr_SystemErrIntBit = mr_bit(4U),
    NR_ImxUsbOtgIntr_AsyncAdvanceIntBit = mr_bit(5U),
    NR_ImxUsbOtgIntr_UsbResetIntBit = mr_bit(6U),
    NR_ImxUsbOtgIntr_SofRecvIntBit = mr_bit(7U),
    NR_ImxUsbOtgIntr_SleepIntBit = mr_bit(8U),
    NR_ImxUsbOtgIntr_UlpiIntBit = mr_bit(10U),
    NR_ImxUsbOtgIntr_NakIntBit = mr_bit(16U),
    NR_ImxUsbOtgIntr_UsbHostAsyncIntBit = mr_bit(18U),
    NR_ImxUsbOtgIntr_UsbHostPeriodicIntBit = mr_bit(19U),
    NR_ImxUsbOtgIntr_GpTimer0IntBit = mr_bit(24U),
    NR_ImxUsbOtgIntr_GpTimer1IntBit = mr_bit(25U),

} nrt_imx_usb_intr_t;

/*!< ------------------------------------------------------------------------- */
/*!< USBNC - Register Layout Typedef */
typedef struct 
{
    kuint32_t USB_OTGn_CTRL;                                        /*!< USB OTGn Control Register, offset: 0x0 */
    kuint8_t  RESERVED_0[20];
    kuint32_t USB_OTGn_PHY_CTRL_0;                                  /*!< OTGn UTMI PHY Control 0 Register, offset: 0x18 */
} srt_imx_usbnc_t;

/*!< Peripheral USBNC base address */
#define IMX6UL_USBNC1_ADDR_BASE                                     (0x2184800u)
#define IMX6UL_USBNC2_ADDR_BASE                                     (0x2184804u)
#define IMX6UL_USBNC_PROPERTY_ENTRY(x)                              (srt_imx_usbnc_t *)IMX6UL_USBNC##x##_ADDR_BASE

/*!< ------------------------------------------------------------------------- */
/*!< USBPHY - Register Layout Typedef */
typedef struct 
{
    kuint32_t PWD;                                                  /*!< USB PHY Power-Down Register, offset: 0x0 */
    kuint32_t PWD_SET;                                              /*!< USB PHY Power-Down Register, offset: 0x4 */
    kuint32_t PWD_CLR;                                              /*!< USB PHY Power-Down Register, offset: 0x8 */
    kuint32_t PWD_TOG;                                              /*!< USB PHY Power-Down Register, offset: 0xC */
    kuint32_t TX;                                                   /*!< USB PHY Transmitter Control Register, offset: 0x10 */
    kuint32_t TX_SET;                                               /*!< USB PHY Transmitter Control Register, offset: 0x14 */
    kuint32_t TX_CLR;                                               /*!< USB PHY Transmitter Control Register, offset: 0x18 */
    kuint32_t TX_TOG;                                               /*!< USB PHY Transmitter Control Register, offset: 0x1C */
    kuint32_t RX;                                                   /*!< USB PHY Receiver Control Register, offset: 0x20 */
    kuint32_t RX_SET;                                               /*!< USB PHY Receiver Control Register, offset: 0x24 */
    kuint32_t RX_CLR;                                               /*!< USB PHY Receiver Control Register, offset: 0x28 */
    kuint32_t RX_TOG;                                               /*!< USB PHY Receiver Control Register, offset: 0x2C */
    kuint32_t CTRL;                                                 /*!< USB PHY General Control Register, offset: 0x30 */
    kuint32_t CTRL_SET;                                             /*!< USB PHY General Control Register, offset: 0x34 */
    kuint32_t CTRL_CLR;                                             /*!< USB PHY General Control Register, offset: 0x38 */
    kuint32_t CTRL_TOG;                                             /*!< USB PHY General Control Register, offset: 0x3C */
    kuint32_t STATUS;                                               /*!< USB PHY Status Register, offset: 0x40 */
    kuint8_t  RESERVED_0[12];
    kuint32_t DEBUGr;                                               /*!< USB PHY Debug Register, offset: 0x50 */
    kuint32_t DEBUG_SET;                                            /*!< USB PHY Debug Register, offset: 0x54 */
    kuint32_t DEBUG_CLR;                                            /*!< USB PHY Debug Register, offset: 0x58 */
    kuint32_t DEBUG_TOG;                                            /*!< USB PHY Debug Register, offset: 0x5C */
    kuint32_t DEBUG0_STATUS;                                        /*!< UTMI Debug Status Register 0, offset: 0x60 */
    kuint8_t  RESERVED_1[12];
    kuint32_t DEBUG1;                                               /*!< UTMI Debug Status Register 1, offset: 0x70 */
    kuint32_t DEBUG1_SET;                                           /*!< UTMI Debug Status Register 1, offset: 0x74 */
    kuint32_t DEBUG1_CLR;                                           /*!< UTMI Debug Status Register 1, offset: 0x78 */
    kuint32_t DEBUG1_TOG;                                           /*!< UTMI Debug Status Register 1, offset: 0x7C */
    kuint32_t VERSION;                                              /*!< UTMI RTL Version, offset: 0x80 */
} srt_imx_usbphy_t;

/*!< Peripheral USBPHY base address */
#define IMX6UL_USBPHY1_ADDR_BASE                                    (0x20C9000u)
#define IMX6UL_USBPHY2_ADDR_BASE                                    (0x20CA000u)
#define IMX6UL_USBPHY_PROPERTY_ENTRY(x)                             (srt_imx_usbphy_t *)IMX6UL_USBPHY##x##_ADDR_BASE

/*!< ------------------------------------------------------------------------- */
/*!< LCDIF - Register Layout Typedef */
typedef struct 
{
    kuint32_t CTRL;                                                 /*!< eLCDIF General Control Register, offset: 0x0 */
    kuint32_t CTRL_SET;                                             /*!< eLCDIF General Control Register, offset: 0x4 */
    kuint32_t CTRL_CLR;                                             /*!< eLCDIF General Control Register, offset: 0x8 */
    kuint32_t CTRL_TOG;                                             /*!< eLCDIF General Control Register, offset: 0xC */
    kuint32_t CTRL1;                                                /*!< eLCDIF General Control1 Register, offset: 0x10 */
    kuint32_t CTRL1_SET;                                            /*!< eLCDIF General Control1 Register, offset: 0x14 */
    kuint32_t CTRL1_CLR;                                            /*!< eLCDIF General Control1 Register, offset: 0x18 */
    kuint32_t CTRL1_TOG;                                            /*!< eLCDIF General Control1 Register, offset: 0x1C */
    kuint32_t CTRL2;                                                /*!< eLCDIF General Control2 Register, offset: 0x20 */
    kuint32_t CTRL2_SET;                                            /*!< eLCDIF General Control2 Register, offset: 0x24 */
    kuint32_t CTRL2_CLR;                                            /*!< eLCDIF General Control2 Register, offset: 0x28 */
    kuint32_t CTRL2_TOG;                                            /*!< eLCDIF General Control2 Register, offset: 0x2C */
    kuint32_t TRANSFER_COUNT;                                       /*!< eLCDIF Horizontal and Vertical Valid Data Count Register, offset: 0x30 */
    kuint8_t  RESERVED_0[12];
    kuint32_t CUR_BUF;                                              /*!< LCD Interface Current Buffer Address Register, offset: 0x40 */
    kuint8_t  RESERVED_1[12];
    kuint32_t NEXT_BUF;                                             /*!< LCD Interface Next Buffer Address Register, offset: 0x50 */
    kuint8_t  RESERVED_2[12];
    kuint32_t TIMING;                                               /*!< LCD Interface Timing Register, offset: 0x60 */
    kuint8_t  RESERVED_3[12];
    kuint32_t VDCTRL0;                                              /*!< eLCDIF VSYNC Mode and Dotclk Mode Control Register0, offset: 0x70 */
    kuint32_t VDCTRL0_SET;                                          /*!< eLCDIF VSYNC Mode and Dotclk Mode Control Register0, offset: 0x74 */
    kuint32_t VDCTRL0_CLR;                                          /*!< eLCDIF VSYNC Mode and Dotclk Mode Control Register0, offset: 0x78 */
    kuint32_t VDCTRL0_TOG;                                          /*!< eLCDIF VSYNC Mode and Dotclk Mode Control Register0, offset: 0x7C */
    kuint32_t VDCTRL1;                                              /*!< eLCDIF VSYNC Mode and Dotclk Mode Control Register1, offset: 0x80 */
    kuint8_t  RESERVED_4[12];
    kuint32_t VDCTRL2;                                              /*!< LCDIF VSYNC Mode and Dotclk Mode Control Register2, offset: 0x90 */
    kuint8_t  RESERVED_5[12];
    kuint32_t VDCTRL3;                                              /*!< eLCDIF VSYNC Mode and Dotclk Mode Control Register3, offset: 0xA0 */
    kuint8_t  RESERVED_6[12];
    kuint32_t VDCTRL4;                                              /*!< eLCDIF VSYNC Mode and Dotclk Mode Control Register4, offset: 0xB0 */
    kuint8_t  RESERVED_7[12];
    kuint32_t DVICTRL0;                                             /*!< Digital Video Interface Control0 Register, offset: 0xC0 */
    kuint8_t  RESERVED_8[12];
    kuint32_t DVICTRL1;                                             /*!< Digital Video Interface Control1 Register, offset: 0xD0 */
    kuint8_t  RESERVED_9[12];
    kuint32_t DVICTRL2;                                             /*!< Digital Video Interface Control2 Register, offset: 0xE0 */
    kuint8_t  RESERVED_10[12];
    kuint32_t DVICTRL3;                                             /*!< Digital Video Interface Control3 Register, offset: 0xF0 */
    kuint8_t  RESERVED_11[12];
    kuint32_t DVICTRL4;                                             /*!< Digital Video Interface Control4 Register, offset: 0x100 */
    kuint8_t  RESERVED_12[12];
    kuint32_t CSC_COEFF0;                                           /*!< RGB to YCbCr 4:2:2 CSC Coefficient0 Register, offset: 0x110 */
    kuint8_t  RESERVED_13[12];
    kuint32_t CSC_COEFF1;                                           /*!< RGB to YCbCr 4:2:2 CSC Coefficient1 Register, offset: 0x120 */
    kuint8_t  RESERVED_14[12];
    kuint32_t CSC_COEFF2;                                           /*!< RGB to YCbCr 4:2:2 CSC Coefficent2 Register, offset: 0x130 */
    kuint8_t  RESERVED_15[12];
    kuint32_t CSC_COEFF3;                                           /*!< RGB to YCbCr 4:2:2 CSC Coefficient3 Register, offset: 0x140 */
    kuint8_t  RESERVED_16[12];
    kuint32_t CSC_COEFF4;                                           /*!< RGB to YCbCr 4:2:2 CSC Coefficient4 Register, offset: 0x150 */
    kuint8_t  RESERVED_17[12];
    kuint32_t CSC_OFFSET;                                           /*!< RGB to YCbCr 4:2:2 CSC Offset Register, offset: 0x160 */
    kuint8_t  RESERVED_18[12];
    kuint32_t CSC_LIMIT;                                            /*!< RGB to YCbCr 4:2:2 CSC Limit Register, offset: 0x170 */
    kuint8_t  RESERVED_19[12];
    kuint32_t DATA;                                                 /*!< LCD Interface Data Register, offset: 0x180 */
    kuint8_t  RESERVED_20[12];
    kuint32_t BM_ERROR_STAT;                                        /*!< Bus Master Error Status Register, offset: 0x190 */
    kuint8_t  RESERVED_21[12];
    kuint32_t CRC_STAT;                                             /*!< CRC Status Register, offset: 0x1A0 */
    kuint8_t  RESERVED_22[12];
    kuint32_t STAT;                                                 /*!< LCD Interface Status Register, offset: 0x1B0 */
    kuint8_t  RESERVED_23[76];
    kuint32_t THRES;                                                /*!< eLCDIF Threshold Register, offset: 0x200 */
    kuint8_t  RESERVED_24[12];
    kuint32_t AS_CTRL;                                              /*!< eLCDIF AS Buffer Control Register, offset: 0x210 */
    kuint8_t  RESERVED_25[12];
    kuint32_t AS_BUF;                                               /*!< Alpha Surface Buffer Pointer, offset: 0x220 */
    kuint8_t  RESERVED_26[12];
    kuint32_t AS_NEXT_BUF;                                          /*!< offset: 0x230 */
    kuint8_t  RESERVED_27[12];
    kuint32_t AS_CLRKEYLOW;                                         /*!< eLCDIF Overlay Color Key Low, offset: 0x240 */
    kuint8_t  RESERVED_28[12];
    kuint32_t AS_CLRKEYHIGH;                                        /*!< eLCDIF Overlay Color Key High, offset: 0x250 */
    kuint8_t  RESERVED_29[12];
    kuint32_t SYNC_DELAY;                                           /*!< LCD working insync mode with CSI for VSYNC delay, offset: 0x260 */
} srt_imx_lcdif_t;

/*!< Peripheral LCDIF base address */
#define IMX6UL_LCDIF_ADDR_BASE                                      (0x21C8000u)
#define IMX6UL_LCDIF_PROPERTY_ENTRY()                               (srt_imx_lcdif_t *)IMX6UL_LCDIF_ADDR_BASE

/*!< ------------------------------------------------------------------------- */
/*!< USDHC - Register Layout Typedef */
typedef struct 
{
    kuint32_t DS_ADDR;                                              /*!< DMA System Address, offset: 0x0 */
    kuint32_t BLK_ATT;                                              /*!< Block Attributes, offset: 0x4 */
    kuint32_t CMD_ARG;                                              /*!< Command Argument, offset: 0x8 */
    kuint32_t CMD_XFR_TYP;                                          /*!< Command Transfer Type, offset: 0xC */
    kuint32_t CMD_RSP0;                                             /*!< Command Response0, offset: 0x10 */
    kuint32_t CMD_RSP1;                                             /*!< Command Response1, offset: 0x14 */
    kuint32_t CMD_RSP2;                                             /*!< Command Response2, offset: 0x18 */
    kuint32_t CMD_RSP3;                                             /*!< Command Response3, offset: 0x1C */
    kuint32_t DATA_BUFF_ACC_PORT;                                   /*!< Data Buffer Access Port, offset: 0x20 */
    kuint32_t PRES_STATE;                                           /*!< Present State, offset: 0x24 */
    kuint32_t PROT_CTRL;                                            /*!< Protocol Control, offset: 0x28 */
    kuint32_t SYS_CTRL;                                             /*!< System Control, offset: 0x2C */
    kuint32_t INT_STATUS;                                           /*!< Interrupt Status, offset: 0x30 */
    kuint32_t INT_STATUS_EN;                                        /*!< Interrupt Status Enable, offset: 0x34 */
    kuint32_t INT_SIGNAL_EN;                                        /*!< Interrupt Signal Enable, offset: 0x38 */
    kuint32_t AUTOCMD12_ERR_STATUS;                                 /*!< Auto CMD12 Error Status, offset: 0x3C */
    kuint32_t HOST_CTRL_CAP;                                        /*!< Host Controller Capabilities, offset: 0x40 */
    kuint32_t WTMK_LVL;                                             /*!< Watermark Level, offset: 0x44 */
    kuint32_t MIX_CTRL;                                             /*!< Mixer Control, offset: 0x48 */
    kuint8_t  RESERVED_0[4];
    kuint32_t FORCE_EVENT;                                          /*!< Force Event, offset: 0x50 */
    kuint32_t ADMA_ERR_STATUS;                                      /*!< ADMA Error Status Register, offset: 0x54 */
    kuint32_t ADMA_SYS_ADDR;                                        /*!< ADMA System Address, offset: 0x58 */
    kuint8_t  RESERVED_1[4];
    kuint32_t DLL_CTRL;                                             /*!< DLL (Delay Line) Control, offset: 0x60 */
    kuint32_t DLL_STATUS;                                           /*!< DLL Status, offset: 0x64 */
    kuint32_t CLK_TUNE_CTRL_STATUS;                                 /*!< CLK Tuning Control and Status, offset: 0x68 */
    kuint8_t  RESERVED_2[84];
    kuint32_t VEND_SPEC;                                            /*!< Vendor Specific Register, offset: 0xC0 */
    kuint32_t MMC_BOOT;                                             /*!< MMC Boot Register, offset: 0xC4 */
    kuint32_t VEND_SPEC2;                                           /*!< Vendor Specific 2 Register, offset: 0xC8 */
    kuint32_t TUNING_CTRL;                                          /*!< Tuning Control Register, offset: 0xCC */
} srt_imx_usdhc_t;

/*!< Peripheral USDHC base address */
#define IMX6UL_USDHC1_ADDR_BASE                                     (0x2190000u)
#define IMX6UL_USDHC2_ADDR_BASE                                     (0x2194000u)
#define IMX6UL_USDHC_PROPERTY_ENTRY(x)                              (srt_imx_usdhc_t *)IMX6UL_USDHC##x##_ADDR_BASE

/*!< ------------------------------------------------------------------------- */
/*!< QuadSPI - Register Layout Typedef */
typedef struct 
{
    kuint32_t MCR;                                                  /*!< Module Configuration Register, offset: 0x0 */
    kuint8_t  RESERVED_0[4];
    kuint32_t IPCR;                                                 /*!< IP Configuration Register, offset: 0x8 */
    kuint32_t FLSHCR;                                               /*!< Flash Configuration Register, offset: 0xC */
    kuint32_t BUF0CR;                                               /*!< Buffer0 Configuration Register, offset: 0x10 */
    kuint32_t BUF1CR;                                               /*!< Buffer1 Configuration Register, offset: 0x14 */
    kuint32_t BUF2CR;                                               /*!< Buffer2 Configuration Register, offset: 0x18 */
    kuint32_t BUF3CR;                                               /*!< Buffer3 Configuration Register, offset: 0x1C */
    kuint32_t BFGENCR;                                              /*!< Buffer Generic Configuration Register, offset: 0x20 */
    kuint8_t  RESERVED_1[12];
    kuint32_t BUF0IND;                                              /*!< Buffer0 Top Index Register, offset: 0x30 */
    kuint32_t BUF1IND;                                              /*!< Buffer1 Top Index Register, offset: 0x34 */
    kuint32_t BUF2IND;                                              /*!< Buffer2 Top Index Register, offset: 0x38 */
    kuint8_t  RESERVED_2[196];
    kuint32_t SFAR;                                                 /*!< Serial Flash Address Register, offset: 0x100 */
    kuint8_t  RESERVED_3[4];
    kuint32_t SMPR;                                                 /*!< Sampling Register, offset: 0x108 */
    kuint32_t RBSR;                                                 /*!< RX Buffer Status Register, offset: 0x10C */
    kuint32_t RBCT;                                                 /*!< RX Buffer Control Register, offset: 0x110 */
    kuint8_t  RESERVED_4[60];
    kuint32_t TBSR;                                                 /*!< TX Buffer Status Register, offset: 0x150 */
    kuint32_t TBDR;                                                 /*!< TX Buffer Data Register, offset: 0x154 */
    kuint8_t  RESERVED_5[4];
    kuint32_t SR;                                                   /*!< Status Register, offset: 0x15C */
    kuint32_t FR;                                                   /*!< Flag Register, offset: 0x160 */
    kuint32_t RSER;                                                 /*!< Interrupt and DMA Request Select and Enable Register, offset: 0x164 */
    kuint32_t SPNDST;                                               /*!< Sequence Suspend Status Register, offset: 0x168 */
    kuint32_t SPTRCLR;                                              /*!< Sequence Pointer Clear Register, offset: 0x16C */
    kuint8_t  RESERVED_6[16];
    kuint32_t SFA1AD;                                               /*!< Serial Flash A1 Top Address, offset: 0x180 */
    kuint32_t SFA2AD;                                               /*!< Serial Flash A2 Top Address, offset: 0x184 */
    kuint32_t SFB1AD;                                               /*!< Serial Flash B1Top Address, offset: 0x188 */
    kuint32_t SFB2AD;                                               /*!< Serial Flash B2Top Address, offset: 0x18C */
    kuint8_t  RESERVED_7[112];
    kuint32_t RBDR[32];                                             /*!< RX Buffer Data Register, array offset: 0x200, array step: 0x4 */
    kuint8_t  RESERVED_8[128];
    kuint32_t LUTKEY;                                               /*!< LUT Key Register, offset: 0x300 */
    kuint32_t LCKCR;                                                /*!< LUT Lock Configuration Register, offset: 0x304 */
    kuint8_t  RESERVED_9[8];
    kuint32_t LUT[64];                                              /*!< Look-up Table register, array offset: 0x310, array step: 0x4 */
} srt_imx_qspi_t;

/*!< Peripheral QuardSPI base address */
#define IMX6UL_QUARDSPI_ADDR_BASE                                   (0x21E0000u)
#define IMX6UL_QSPI_PROPERTY_ENTRY(x)                               (srt_imx_qspi_t *)IMX6UL_QUARDSPI##x##_ADDR_BASE

/*!< ------------------------------------------------------------------------- */
/*! IMX6UL_SDMA - Register Layout Typedef */
typedef struct 
{
    kuint32_t MC0PTR;                                               /*!< ARM platform Channel 0 Pointer, offset: 0x0 */
    kuint32_t INTR;                                                 /*!< Channel Interrupts, offset: 0x4 */
    kuint32_t STOP_STAT;                                            /*!< Channel Stop/Channel Status, offset: 0x8 */
    kuint32_t HSTART;                                               /*!< Channel Start, offset: 0xC */
    kuint32_t EVTOVR;                                               /*!< Channel Event Override, offset: 0x10 */
    kuint32_t DSPOVR;                                               /*!< Channel BP Override, offset: 0x14 */
    kuint32_t HOSTOVR;                                              /*!< Channel ARM platform Override, offset: 0x18 */
    kuint32_t EVTPEND;                                              /*!< Channel Event Pending, offset: 0x1C */
    kuint8_t  RESERVED_0[4];
    kuint32_t RESET;                                                /*!< Reset Register, offset: 0x24 */
    kuint32_t EVTERR;                                               /*!< DMA Request Error Register, offset: 0x28 */
    kuint32_t INTRMASK;                                             /*!< Channel ARM platform Interrupt Mask, offset: 0x2C */
    kuint32_t PSW;                                                  /*!< Schedule Status, offset: 0x30 */
    kuint32_t EVTERRDBG;                                            /*!< DMA Request Error Register, offset: 0x34 */
    kuint32_t CONFIG;                                               /*!< Configuration Register, offset: 0x38 */
    kuint32_t SDMA_LOCK;                                            /*!< SDMA LOCK, offset: 0x3C */
    kuint32_t ONCE_ENB;                                             /*!< OnCE Enable, offset: 0x40 */
    kuint32_t ONCE_DATA;                                            /*!< OnCE Data Register, offset: 0x44 */
    kuint32_t ONCE_INSTR;                                           /*!< OnCE Instruction Register, offset: 0x48 */
    kuint32_t ONCE_STAT;                                            /*!< OnCE Status Register, offset: 0x4C */
    kuint32_t ONCE_CMD;                                             /*!< OnCE Command Register, offset: 0x50 */
    kuint8_t  RESERVED_1[4];
    kuint32_t ILLINSTADDR;                                          /*!< Illegal Instruction Trap Address, offset: 0x58 */
    kuint32_t CHN0ADDR;                                             /*!< Channel 0 Boot Address, offset: 0x5C */
    kuint32_t EVT_MIRROR;                                           /*!< DMA Requests, offset: 0x60 */
    kuint32_t EVT_MIRROR2;                                          /*!< DMA Requests 2, offset: 0x64 */
    kuint8_t  RESERVED_2[8];
    kuint32_t XTRIG_CONF1;                                          /*!< Cross-Trigger Events Configuration Register 1, offset: 0x70 */
    kuint32_t XTRIG_CONF2;                                          /*!< Cross-Trigger Events Configuration Register 2, offset: 0x74 */
    kuint8_t  RESERVED_3[136];
    kuint32_t SDMA_CHNPRI[32];                                      /*!< Channel Priority Registers, array offset: 0x100, array step: 0x4 */
    kuint8_t  RESERVED_4[128];
    kuint32_t CHNENBL[48];                                          /*!< Channel Enable RAM, array offset: 0x200, array step: 0x4 */
} srt_imx_sdma_t;

/*!< Peripheral SDMA base address */
#define IMX6UL_SDMA_ADDR_BASE                                       (0x20EC000u)
#define IMX6UL_SDMA_PROPERTY_ENTRY()                                (srt_imx_sdma_t *)IMX6UL_SDMA_ADDR_BASE

#ifdef __cplusplus
    }
#endif

#endif /* __IMX6UL_PINS_H */
