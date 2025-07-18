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

#include "autoconf.h"

#if (defined(CONFIG_ARCH_ARMV7) && (CONFIG_ARCH_ARMV7))
#include <arch/armv7/asm_config.h>
#endif
#include <board/board_config.h>

/*!< The defines */
#ifndef CONFIG_HZ
#define CONFIG_HZ                               (100UL)             /*!< timer interrupt per 10ms*/
#endif

#define CONFIG_USE_AMP                          (0)

/*!< save more infomation */
#define CONFIG_CONTEXT_MORE                     (0)

#endif /* __MACH_CONFIGS_H */
