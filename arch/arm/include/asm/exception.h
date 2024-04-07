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

/*!< The includes */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>

/*!< The functions */
TARGET_EXT void exec_undefined_handler(void);
TARGET_EXT void exec_prefetch_abort_handler(void);
TARGET_EXT void exec_data_abort_handler(void);
TARGET_EXT void exec_unused_handler(void);

#endif /* __EXCEPTION_H */
