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

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include "mach_configs.h"

#ifdef CONFIG_DEBUG
#define CONFIG_PRINT_LEVEL                      "7"
#else
#define CONFIG_PRINT_LEVEL                      "6"
#endif

/*!< all scheduler will be quited, the kernel will degrade to standlone */
#define CONFIG_SCHDULE                          (1)
/*!< 
 * 0: the pending must be requested by self-thread or others (scheduling to another hightest priority thread);
 * 1: the pending will be requested by timer-out ISR (check priority and time-slice, e.g, thread will be preempted suddenly)
 */
#define CONFIG_PREEMPT                          (1)

/*!< scheduled by time slice (rely on timeout interrupt) */
#define CONFIG_SCHED_SLICE                      (1)

/*!< if CONFIG_SCHED_SLICE is diabled, thread can be chosen ===> 0: polling by priority; 1: polling in order */
#define CONFIG_ROLL_POLL                        (1)

#if CONFIG_SCHED_SLICE
/*!< disable priority,  */
#undef  CONFIG_ROLL_POLL
#define CONFIG_ROLL_POLL                        (0)
#endif

#define CONFIG_HRTIMER_SOFTIRQ                  (0)

/*!< softirq or threading for network rx/tx */
#define CONFIG_NET_RX_SOFTIRQ                   (0)
#define CONFIG_NET_TX_SOFTIRQ                   (0)

#define CONFIG_DEFAULT_LOGIN                    "root"
#define CONFIG_DEFAULT_HOST                     "heavenfox"

#define CONFIG_POWER_LOGO                       "/media/FAT32_2/boot/logo/logo.bmp"
#define CONFIG_WALL_PAPER                       "/media/FAT32_2/boot/windows/windows.bmp"

/*!< armv7 */
#if (defined(CONFIG_ARCH_ARMV7) && (CONFIG_ARCH_ARMV7))
    #include <arch/armv7/arch_common.h>
#endif

#ifdef __cplusplus
    }
#endif

#endif /* __CONFIGS_H */
