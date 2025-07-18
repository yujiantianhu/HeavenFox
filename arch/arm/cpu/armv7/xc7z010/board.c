/*
 * IMX6ULL Board General Initial
 *
 * File Name:   board.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/generic.h>
#include <boot/board_init.h>
#include <arch/armv7/gcc_config.h>
#include <arch/setup.h>
#include <common/time.h>

/*!< Check */
#if (!defined(CONFIG_CPU_FREQ) || (CONFIG_CPU_FREQ != 666666687))
    #error "Cpu Frequency is configured (CONFIG_CPU_FREQ) error!"
#endif

/*!< The defines */

/*!< API functions */
/*!
 * @brief  	xc7z010_clk_initial
 * @param  	none
 * @retval 	none
 * @note   	initial all clock
 */
static void xc7z010_clk_initial(void)
{

}

/*!
 * @brief  	xc7z010_sram_initial
 * @param  	none
 * @retval 	none
 * @note   	void function
 */
static void xc7z010_sram_initial(void)
{

}

/*!
 * @brief  	delay_config
 * @param  	none
 * @retval 	none
 * @note   	void function
 */
void arch_delay_config(kbool_t in_kernel)
{
    /*!< Configure delay cnt */
    if (in_kernel)
        init_delay_freq(111U, 3U, 3U);
    else
        init_delay_freq(111U, 3U, 3U);
}

/*!
 * @brief  	s_init
 * @param  	none
 * @retval 	none
 * @note   	The first C program, initial lowlevel enviroment
 */
void s_init(void)
{
    /*!< delay freq configure */
    arch_delay_config(false);

    /*!< clock init */
    xc7z010_clk_initial();

    /*!< sram init */
    xc7z010_sram_initial();
}

/* end of file */
