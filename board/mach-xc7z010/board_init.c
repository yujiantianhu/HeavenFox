/*
 * Board Initialization
 *
 * File Name:   board_init.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <boot/board_init.h>
#include "zynq7_common.h"

/*!< API function */
/*!
 * @brief   board_init_serial
 * @param   none
 * @retval  none
 * @note    initial serial
 */
kint32_t board_init_serial(void)
{
	zynq7_console_init();

    return RET_BOOT_PASS;
}

/*!
 * @brief   board_init_sdmmc
 * @param   none
 * @retval  none
 * @note    initial sdio
 */
kint32_t board_init_sdmmc(void)
{
	zynq7_sdmmc_init();
//	zynq7_hdmi_init();

    return RET_BOOT_PASS;
}

/*!
 * @brief   board_init_light
 * @param   none
 * @retval  none
 * @note    initial led
 */
kint32_t board_init_light(void)
{
	zynq7_led_init();

    return RET_BOOT_PASS;
}

/* end of file */
