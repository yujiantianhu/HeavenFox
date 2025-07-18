/*
 * Arch Common Header
 *
 * File Name:   setup.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.07.18
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __SETUP_H
#define __SETUP_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/generic.h>

/*!< The functions */
extern void arch_delay_config(kbool_t in_kernel);
extern void arch_systick_init(void);
extern void arch_hrtimer_init(void);

extern void setup_arch(void);
extern void setup_systick(void);
extern void setup_hrtick(void);

#ifdef __cplusplus
    }
#endif

#endif
