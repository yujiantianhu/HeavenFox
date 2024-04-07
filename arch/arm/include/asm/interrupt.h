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

/*!< The includes*/
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>

/*!< The functions */
TARGET_EXT void exec_fiq_handler(void);
TARGET_EXT void exec_irq_handler(void);
TARGET_EXT void exec_software_irq_handler(void);

#endif /* __INTERRUPT_H */
