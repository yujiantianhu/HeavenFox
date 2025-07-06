/*
 * imx6ull Common Definition
 *
 * File Name:   imx_common.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.10.09
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/time.h>
#include "imx6_common.h"

/*!< The defines */
#define SYSTEM_CLK_FREQ_CNT_PER_MS                          (0x7ffU)
#define SYSTEM_CLK_FREQ_CNT_PER_S                          	(SYSTEM_CLK_FREQ_CNT_PER_MS * 1000)
#define SYSTEM_CLK_FREQ_CNT_PER_US                          (SYSTEM_CLK_FREQ_CNT_PER_MS / 1000)

/*!< API function */


/* end of file */
