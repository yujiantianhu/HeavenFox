/*
 * User Thread Instance Tables
 *
 * File Name:   demo_task.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.02
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __DEMO_TASK_H_
#define __DEMO_TASK_H_

#ifdef __cplusplus

/*!< The globals */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <common/list_types.h>
#include <common/libcxplus.h>
#include <kernel/instance.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/mutex.h>
#include <kernel/mailbox.h>

/*!< The defines */

/*!< The globals */

/*!< The functions */
#ifdef CONFIG_NETWORK_TASK
extern kint32_t network_task_init(void);
#else
static inline kint32_t network_task_init(void) { return 0; }
#endif

#ifdef CONFIG_LVGL_TASK
extern kint32_t lvgl_task_init(void);
#else
static inline kint32_t lvgl_task_init(void) { return 0; }
#endif

#endif

#endif /* __DEMO_TASK_H_ */
