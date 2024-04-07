/*
 * Hardware Abstraction Layer I2C Interface
 *
 * File Name:   hal_i2c_core.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.16
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_I2C_CORE_H_
#define __HAL_I2C_CORE_H_

/*!< The includes */
#include <platform/hal_basic.h>
#include <platform/hal_platform.h>
#include <platform/i2c/hal_i2c_algo.h>

/*!< The defines */
struct hal_i2c_adapter
{
	kuint32_t id;
	const struct hal_i2c_algo *sprt_algo; 				/*!< the algorithm to access the bus */
	void *ptr_algo_data;

	ksint32_t timeout;		         	 					/*!< in jiffies */
	ksint32_t retries;
	struct hal_device sgrt_dev;		         			/*!< the adapter device */

	ksint32_t nr;
	ksint8_t name[48];
};


#endif /*!< _I2C_CORE_H_ */
