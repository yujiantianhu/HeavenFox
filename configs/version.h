/*
 * Generic Configuration
 *
 * File Name:   version.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.07.22
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __VERSION_H
#define __VERSION_H

/*!< The defines */
#define CONFIG_MAJOR                            "3"
#define CONFIG_MINOR                            "2"
#define CONFIG_PATCH                            "1"

#ifdef CONFIG_BUILD_TIME
#define CONFIG_VERSION                          CONFIG_MAJOR "." CONFIG_MINOR "."  CONFIG_PATCH"." CONFIG_BUILD_TIME
#else
#define CONFIG_VERSION                          CONFIG_MAJOR "." CONFIG_MINOR "." CONFIG_PATCH ".00000000.000000"
#endif

#endif /* __VERSION_H */
