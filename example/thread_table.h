/*
 * User Thread Instance Tables
 *
 * File Name:   thread_table.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.02
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __THREAD_TABLE_H_
#define __THREAD_TABLE_H_

/*!< The globals */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <kernel/instance.h>

/*!< The defines */


/*!< The globals */


/*!< The functions */
TARGET_EXT ksint32_t light_app_init(void);
TARGET_EXT ksint32_t display_app_init(void);

#endif /* __THREAD_TABLE_H_ */
