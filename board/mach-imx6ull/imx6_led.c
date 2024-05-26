/*
 * IMX6ULL Board GPIO Initial
 *
 * File Name:   imx6_led.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include "imx6_common.h"

/*!< The defines */
/*!< Mux */
#define IMX_LIGHT_MUX_SELECT                           		IMX6UL_MUX_GPIO1_IO03_GPIO1_IO03

/*!< CCM */
#define IMX_LIGHT_CLK_CG_REG								CG13
#define IMX_LIGHT_CLK_SELECT								IMX6UL_CCM_CCGR_CLOCK_ENTRY(1)

/*!< Pin */
#define IMX_LIGHT_PORT_ENTRY()                        		IMX6UL_GPIO_PROPERTY_ENTRY(1)
#define IMX_LIGHT_PIN_BIT									IMX6UL_GPIO_PIN_OFFSET_BIT(3)

/*!< API function */
/*!
 * @brief   imx6ull_led_init
 * @param   none
 * @retval  none
 * @note    initial GPIO for LED
 */
void imx6ull_led_init(void)
{
	srt_imx_pin_t  sgrt_Port;
	urt_imx_io_ctl_pad_t ugrt_ioPad;
	srt_hal_imx_gpio_t *sprt_Light;

	sprt_Light = IMX_LIGHT_PORT_ENTRY();

	/*!< enable gpio1 clock */
    mrt_imx_ccm_clk_enable(IMX_LIGHT_CLK_CG_REG, IMX_LIGHT_CLK_SELECT);

	/*!< set Gpio Peripheral */
	mrt_write_urt_data(&ugrt_ioPad, 
						IMX6UL_IO_CTL_PAD_PKE_BIT(IMX6UL_IO_CTL_PAD_PKE_ENABLE)  |
						IMX6UL_IO_CTL_PAD_PUS_BIT(IMX6UL_IO_CTL_PAD_PUS_100K_UP) |
						IMX6UL_IO_CTL_PAD_SPEED_BIT(IMX6UL_IO_CTL_PAD_SPEED_100MHZ) |
						IMX6UL_IO_CTL_PAD_DSE_BIT(IMX6UL_IO_CTL_PAD_DSE_RDIV(6)));
	hal_imx_pin_attribute_init(&sgrt_Port, IMX6UL_PIN_ADDR_BASE, IMX_LIGHT_MUX_SELECT, mrt_trans_urt_data(&ugrt_ioPad));

	hal_imx_pin_mux_configure(&sgrt_Port);
	hal_imx_pin_pad_configure(&sgrt_Port);

	/*!< set Gpio Pin Direction Output */
	mrt_setbitl(IMX_LIGHT_PIN_BIT, &sprt_Light->GDIR);

	/*!< set Gpio Default Level */
	mrt_clrbitl(IMX_LIGHT_PIN_BIT, &sprt_Light->DR);
}

/* end of file*/
