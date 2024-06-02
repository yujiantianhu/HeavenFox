/*
 * Generic Configuration
 *
 * File Name:   mach_configs.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.12.16
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __MACH_CONFIGS_H
#define __MACH_CONFIGS_H

/*!< The defines */
/*!< Configure */
#define CONFIG_STACK_WITH_LDS                   (1)                 /*!< configure stack by .lds */
#define CONFIG_PRINT_LEVEL                      "7"

#define CONFIG_OF                               (1)
#define CONFIG_HZ                               (100UL)             /*!< timer interrupt per 10ms*/

#define CONFIG_SCHDULE                          (1)
#define CONFIG_PREEMPT                          (1)
#define CONFIG_ROLL_POLL                        (0)

/*!< The includes */
/*!< Board Configuration */
#if defined(CONFIG_MACH_AM4378_MYIR)
    #include "mach/am4378_myir_config.h"
#elif defined(CONFIG_MACH_IMX6ULL_TOPPET)
    #include "mach/imx6ull_toppet_config.h"
#elif defined(CONFIG_MACH_XC7Z010_MICROPHASE)
    #include "mach/zynq010_microphase_config.h"
#endif

#endif /* __MACH_CONFIGS_H */
