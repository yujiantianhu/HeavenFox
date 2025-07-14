/*
 * ZYNQ7 Common Definition
 *
 * File Name:   zynq7_common.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.06.19
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/time.h>
#include "zynq7_common.h"

/*!< The defines */
#if (CONFIG_OPTIMIZE_CLASS != 0)
/*!< 1GHz = (10 ^ 9)Hz ===> 1ms = (10 ^ 6)Hz */
#define SYSTEM_CLK_FREQ_CNT_PER_MS                          (1000000)
#else
#define SYSTEM_CLK_FREQ_CNT_PER_MS                          (10000)
#endif

#define SYSTEM_CLK_FREQ_CNT_PER_S                          	(SYSTEM_CLK_FREQ_CNT_PER_MS * 1000)
#define SYSTEM_CLK_FREQ_CNT_PER_US                          (SYSTEM_CLK_FREQ_CNT_PER_MS / 1000)

/*!< API function */


/* end of file */
