/*
 * Generic Configuration
 *
 * File Name:   configs.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.12.16
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __CONFIGS_H
#define __CONFIGS_H

/*!< The includes */
#include "mach_configs.h"

#if defined(CONFIG_ARCH_NUMBER)

/*!< armv7 */
#if (__IS_ARCH_ARMV7(CONFIG_ARCH_NUMBER))
    #include <asm/armv7/arch_common.h>
#endif
#endif


#endif /* __CONFIGS_H */
