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
 * @brief   board_init_console
 * @param   none
 * @retval  none
 * @note    initial console
 */
ksint32_t board_init_console(void)
{
    imx6ull_console_init();

    /*!< Output Board Information */
    print_info("Console Initial Finished ...\n");
    print_info("Welcome to Use Real-Kernel!\n");

    return RET_BOOT_PASS;
}

/*!
 * @brief   board_init_systick
 * @param   none
 * @retval  none
 * @note    initial systick
 */
ksint32_t board_init_systick(void)
{
    imx6ull_systick_init();

    return RET_BOOT_PASS;
}

/*!
 * @brief   board_init_sdmmc
 * @param   none
 * @retval  none
 * @note    initial sdio
 */
ksint32_t board_init_sdmmc(void)
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
ksint32_t board_init_light(void)
{
    imx6ull_led_init();

    return RET_BOOT_PASS;
}

/* end of file */
