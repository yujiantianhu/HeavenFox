/*
 * Arch General Initial
 *
 * File Name:   setup.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.07.18
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <configs/configs.h>
#include <arch/setup.h>
#include <common/generic.h>
#include <common/time.h>
#include <boot/board_init.h>

/*!< The defines */

/*!< API functions */
/*!
 * @brief  	setup arch
 * @param  	none
 * @retval 	none
 * @note   	initial for kernel
 */
void __init setup_arch(void)
{
	arch_delay_config(true);

	print_info("\r\nStart kernel ...... \r\n");
}

/*!
 * @brief  	setup system tick
 * @param  	none
 * @retval 	none
 * @note   	initial for kernel
 */
void __init setup_systick(void)
{
	arch_systick_init();

    print_info("System Tick start to run, frequency is: %u(Hz)\r\n", SYSTICK_FREQ);
    print_info("System counter (jiffies) enable, TICK_HZ is %u(Hz)\r\n", TICK_HZ);
}

/*!
 * @brief  	setup high accuracy timer
 * @param  	none
 * @retval 	none
 * @note   	initial for kernel
 */
void __init setup_hrtick(void)
{
	arch_hrtimer_init();
	print_info("System hrtimer start to run, frequency is: %u(Hz)\r\n", HRTIMER_FREQ);
}

/* end of file */
