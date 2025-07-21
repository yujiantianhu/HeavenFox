/*
 * Generic Configuration
 *
 * File Name:   core.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.07.21
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __BOOT_CORE_H
#define __BOOT_CORE_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <configs/configs.h>
#include <common/generic.h>

/*!< The defines */
#define CORE_AMP                                (0)
#define CORE_SMP                                (1)

#if (CONFIG_CORE == CORE_AMP)
#define CONFIG_USE_AMP                          (1)
#else
#define CONFIG_USE_AMP                          (0)
#endif

#if (CONFIG_CORE == CORE_SMP)
#define CONFIG_USE_SMP                          (1)
#else
#define CONFIG_USE_SMP                          (0)
#endif

#if ((CONFIG_CORE > CORE_SMP) || (!CONFIG_USE_AMP && !CONFIG_USE_SMP))
    #error "Multi core running mode error!"
#endif

/*!< The functions */
extern kchar_t *get_version(void);
extern kchar_t *get_arch(void);
extern kchar_t *get_arch_type(void);
extern kchar_t *get_arch_class(void);
extern kchar_t *get_cpu_verdor(void);
extern kchar_t *get_cpu_name(void);
extern kchar_t *get_board_manufacturer(void);
extern kchar_t *get_board_name(void);
extern kchar_t *get_cpu_mode(void);

#ifdef __cplusplus
    }
#endif

#endif /* __BOOT_CORE_H */
