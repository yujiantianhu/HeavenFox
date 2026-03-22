/*
 * ARM V7 Interrupt API Function
 *
 * File Name:   interrupt.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __INTERRUPT_H
#define __INTERRUPT_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes*/
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include "exception.h"

/*!< The functions */
extern void exec_fiq_handler(kutype_t _sp, kutype_t _lr);
extern void exec_irq_handler(kutype_t _sp, kutype_t _lr);
extern void exec_software_irq_handler(kutype_t _sp, kutype_t _lr);

#ifdef __cplusplus
    }
#endif

#endif /* __INTERRUPT_H */
