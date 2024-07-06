/*
 * Clock Driver
 *
 * File Name:   zynq7-clks.c
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

/*!< The defines */

/*!< The globals */
/*!< device id for device-tree */
static struct fwk_of_device_id sgrt_zynq7_clks_driver_ids[] =
{
	{ .compatible = "xlnx,ps7-clkc", },
	{},
};

static kchar_t *g_zynq7_clks_gate_name[] =
{
    "armpll",       "ddrpll",       "iopll",    
    "cpu_6or4x",    "cpu_3or2x",    "cpu_2x",       "cpu_1x",   
    "ddr2x",        "ddr3x",        "dci",          "lqspi",    
    "smc",          "pcap",         "gem0",         "gem1",     
    "fclk0",        "fclk1",        "fclk2",        "fclk3", 
    "can0",         "can1",         "sdio0",        "sdio1", 
    "uart0",        "uart1",        "spi0",         "spi1",
    "dma",          "usb0_aper",    "usb1_aper", 
    "gem0_aper",    "gem1_aper",    "sdio0_aper",   "sdio1_aper",
    "spi0_aper",    "spi1_aper",    "can0_aper",    "can1_aper",
    "i2c0_aper",    "i2c1_aper",    "uart0_aper",   "uart1_aper",
    "gpio_aper",    "lqspi_aper",   "smc_aper",     "swdt",
    "dbg_trc",      "dbg_apb"
};

enum __ERT_ZYNQ7_CLKS_NUMBER
{
    NR_ZYNQ7_CLK_ARMPLL = 0U,
    NR_ZYNQ7_CLK_DDRPLL,
    NR_ZYNQ7_CLK_IOPLL,

    NR_ZYNQ7_CLK_CPU_6OR4X,     NR_ZYNQ7_CLK_CPU_3OR2X,     NR_ZYNQ7_CLK_CPU_2X,        NR_ZYNQ7_CLK_CPU_1X,
    NR_ZYNQ7_CLK_DDR2X,         NR_ZYNQ7_CLK_DDR3X,         NR_ZYNQ7_CLK_DCI,           NR_ZYNQ7_CLK_LQSPI,
    NR_ZYNQ7_CLK_SMC,           NR_ZYNQ7_CLK_PCAP,          NR_ZYNQ7_CLK_GEM0,          NR_ZYNQ7_CLK_GEM1,
    NR_ZYNQ7_CLK_FCLK0,         NR_ZYNQ7_CLK_FCLK1,         NR_ZYNQ7_CLK_FCLK2,         NR_ZYNQ7_CLK_FCLK3,
    NR_ZYNQ7_CLK_CAN0,          NR_ZYNQ7_CLK_CAN1,          NR_ZYNQ7_CLK_SDIO0,         NR_ZYNQ7_CLK_SDIO1,
    NR_ZYNQ7_CLK_UART0,         NR_ZYNQ7_CLK_UART1,         NR_ZYNQ7_CLK_SPI0,          NR_ZYNQ7_CLK_SPI1,      
    NR_ZYNQ7_CLK_DMA,           NR_ZYNQ7_CLK_USB0_APER,     NR_ZYNQ7_CLK_USB1_APER, 
    NR_ZYNQ7_CLK_GEM0_APER,     NR_ZYNQ7_CLK_GEM1_APER,     NR_ZYNQ7_CLK_SDIO0_APER,    NR_ZYNQ7_CLK_SDIO1_APER,
    NR_ZYNQ7_CLK_SPI0_APER,     NR_ZYNQ7_CLK_SPI1_APER,     NR_ZYNQ7_CLK_CAN0_APER,     NR_ZYNQ7_CLK_CAN1_APER,
    NR_ZYNQ7_CLK_I2C0_APER,     NR_ZYNQ7_CLK_I2C1_APER,     NR_ZYNQ7_CLK_UART0_APER,    NR_ZYNQ7_CLK_UART1_APER,
    NR_ZYNQ7_CLK_GPIO_APER,     NR_ZYNQ7_CLK_LQSPI_APER,    NR_ZYNQ7_CLK_SMC_APER,      NR_ZYNQ7_CLK_SWDT,
    NR_ZYNQ7_CLK_DBG_TRC,       NR_ZYNQ7_CLK_DBG_APB
};

/*!< API function */
/*!
 * @brief   enable clk
 * @param   sprt_hw
 * @retval  errono
 * @note    none
 */
static kint32_t zynq7_clks_gate_enable(struct fwk_clk_hw *sprt_hw)
{
    struct fwk_clk_gate *sprt_gate;

    sprt_gate = mrt_container_of(sprt_hw, struct fwk_clk_gate, sgrt_hw);

    return ER_NORMAL;
}

/*!
 * @brief   disable clk
 * @param   sprt_hw
 * @retval  none
 * @note    none
 */
static void	zynq7_clks_gate_disable(struct fwk_clk_hw *sprt_hw)
{
    struct fwk_clk_gate *sprt_gate;

    sprt_gate = mrt_container_of(sprt_hw, struct fwk_clk_gate, sgrt_hw);

}

/*!
 * @brief   get clk status
 * @param   sprt_hw
 * @retval  errono
 * @note    none
 */
static kint32_t zynq7_clks_gate_is_enabled(struct fwk_clk_hw *sprt_hw)
{
    struct fwk_clk_gate *sprt_gate;
    kuint32_t value;

    sprt_gate = mrt_container_of(sprt_hw, struct fwk_clk_gate, sgrt_hw);


    return !!value;
}

static const struct fwk_clk_ops sgrt_zynq7_clks_gate_oprts =
{
    .enable = zynq7_clks_gate_enable,
    .disable = zynq7_clks_gate_disable,
    .is_enabled = zynq7_clks_gate_is_enabled,
};

/*!
 * @brief   create clk global data from device_tree
 * @param   sprt_data
 * @retval  errono
 * @note    none
 */
static kint32_t zynq7_clks_driver_of_init(struct zynq7_clks_data *sprt_data)
{

}

/*!
 * @brief   init driver
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init zynq7_clks_driver_init(void)
{

    return ER_NORMAL;
}

/*!
 * @brief   exit driver
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit zynq7_clks_driver_exit(void)
{

}

IMPORT_LATE_INIT(zynq7_clks_driver_init);
IMPORT_LATE_EXIT(zynq7_clks_driver_exit);

/*!< end of file */
