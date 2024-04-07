/*
 * Hardware Abstraction Layer I2C Interface
 *
 * File Name:   hal_i2c_algo.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.16
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_I2C_ALGO_H_
#define __HAL_I2C_ALGO_H_

/*!< The includes */
#include <platform/hal_basic.h>
#include <platform/i2c/hal_i2c_core.h>

/*!< The defines */
struct hal_i2c_msg
{
	kuint16_t addr;	                     				/*!< slave address */
	kuint16_t flags;

#define HAL_I2C_M_TEN						0x0010	    /*!< this is a ten bit chip address */
#define HAL_I2C_M_RD						0x0001	    /*!< read data, from slave to master */
#define HAL_I2C_M_NOSTART					0x4000	 	/*!< if I2C_FUNC_PROTOCOL_MANGLING */
#define HAL_I2C_M_REV_DIR_ADDR				0x2000	 	/*!< if I2C_FUNC_PROTOCOL_MANGLING */
#define HAL_I2C_M_IGNORE_NAK				0x1000   	/*!< if I2C_FUNC_PROTOCOL_MANGLING */
#define HAL_I2C_M_NO_RD_ACK					0x0800	 	/*!< if I2C_FUNC_PROTOCOL_MANGLING */
#define HAL_I2C_M_RECV_LEN					0x0400	 	/*!< length will be first received byte */

	kuint16_t len;		                 				/*!< msg length */
	kuint8_t *ptr_buf;		                 			/*!< pointer to msg data */
};

struct hal_i2c_algo
{
	ksint32_t (*master_xfer)(struct hal_i2c_adapter *sprt_adap, struct hal_i2c_msg *sprt_msgs, ksint32_t num);
//	ksint32_t (*smbus_xfer) (struct hal_i2c_adapter *adap, kuint16_t addr, kuint16_t flags, kuint8_t read_write, kuint8_t command, kssize_t size, union i2c_smbus_data *data);

	/*!< To determine what the adapter supports */
	kuint32_t (*functionality) (struct hal_i2c_adapter *);
};


#endif /*!< __HAL_I2C_ALGO_H_ */
