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
#include "imx6_common.h"

/*!< API function */
/*!
 * @brief   board_init_serial
 * @param   none
 * @retval  none
 * @note    initial serial
 */
kint32_t board_init_serial(void)
{
    imx6ull_console_init();

    return RET_BOOT_PASS;
}

/*!
 * @brief   board_init_systick
 * @param   none
 * @retval  none
 * @note    initial systick
 */
kint32_t board_init_systick(void)
{
    imx6ull_systick_init();
    imx6ull_hrtimer_init();

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
    imx6ull_sdmmc_init();

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
    imx6ull_led_init();

    return RET_BOOT_PASS;
}

/* end of file */
