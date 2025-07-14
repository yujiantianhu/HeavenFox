/*
 * ARM V7 Exception API Function
 *
 * File Name:   exception.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __EXCEPTION_H
#define __EXCEPTION_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <common/atomic_types.h>

/*!< The globals */
extern kuint32_t g_interrupt_flags;

/*!< The defines */
#define IRQ_BIT                                     (0x02)
#define FIQ_BIT                                     (0x04)
#define SWI_BIT                                     (0x08)
#define UND_ABORT_BIT                               (0x10)
#define PREFETCH_ABORT_BIT                          (0x20)
#define DATA_ABORT_BIT                              (0x40)
#define UNUSED_BIT                                  (0x80)

#define IS_IN_INTERRUPT()                           atomic_is_set(0xff, &g_interrupt_flags)
#define IS_IN_EXCEPTION()                           atomic_is_set(0xf0, &g_interrupt_flags)
#define SET_INTERRUPT_FLAG(mask)                    atomic_set_mask(mask, &g_interrupt_flags)
#define CLR_INTERRUPT_FLAG(mask)                    atomic_clear_mask(mask, &g_interrupt_flags)
#define SET_EXCEPTION_FLAG(mask)                    SET_INTERRUPT_FLAG(mask)
#define CLR_EXCEPTION_FLAG(mask)                    CLR_INTERRUPT_FLAG(mask)

/*!< The functions */
extern void exec_undefined_handler(void);
extern void exec_prefetch_abort_handler(void);
extern void exec_data_abort_handler(void);
extern void exec_unused_handler(void);

#ifdef __cplusplus
    }
#endif

#endif /* __EXCEPTION_H */
