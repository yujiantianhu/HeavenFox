/*
 * Hardware Abstraction Layer I2C Interface
 *
 * File Name:   hal_i2c_dev.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.16
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_I2C_DEV_H_
#define __HAL_I2C_DEV_H_

/*!< The includes */
#include <platform/hal_basic.h>
#include <platform/hal_platform.h>
#include <platform/of/hal_of.h>
#include <platform/i2c/hal_i2c_core.h>

/*!< The defines */
#define HAL_I2C_NAME_SIZE									(32)

struct hal_i2c_device_id
{
	kstring_t *name;
	ksint32_t driver_data;
};

struct hal_i2c_board_info
{
	ksint8_t type[HAL_I2C_NAME_SIZE];	     				/*!< device name */
	kuint16_t flags;				 						/*!< property */
	kuint16_t addr;											/*!< device slave address */
	void *ptr_platform_data;								/*!< device private data */

	struct hal_device_node *sprt_node;
	ksint32_t irq;											/*!< Interrupt number */
};

struct hal_i2c_client
{
	kuint16_t flags;										/*!< div., see below */
	kuint16_t addr;											/*!< chip address - NOTE: 7bit */
					            							/*!< addresses are stored in the _LOWER_ 7 bits */
	ksint8_t name[HAL_I2C_NAME_SIZE];
	struct hal_i2c_adapter *sprt_adapter;				/*!< the adapter we sit on */
	struct hal_i2c_driver *sprt_driver;					/*!< and our access routines */
	struct hal_device sgrt_dev;		    				/*!< the device structure */
	ksint32_t irq;			        						/*!< irq issued by device	*/
	struct list_head sgrt_detected;
};

struct hal_i2c_driver
{
	ksint32_t (*attach_adapter) (struct hal_i2c_adapter *);
	ksint32_t (*detach_adapter) (struct hal_i2c_adapter *);

	/*!< Standard driver model interfaces */
	ksint32_t (*probe) (struct hal_i2c_client *, const struct hal_i2c_device_id *);
	ksint32_t (*remove) (struct hal_i2c_client *);

	/*!< driver model interfaces that don't relate to enumeration  */
	void (*shutdown) (struct hal_i2c_client *);
	ksint32_t (*suspend) (struct hal_i2c_client *, ksint32_t mesg);
	ksint32_t (*resume) (struct hal_i2c_client *);
	ksint32_t (*command) (struct hal_i2c_client *client, kuint32_t cmd, void *ptr_arg);

	struct hal_driver sgrt_driver;
	const struct hal_i2c_device_id *sprt_id_table;

	/*!< Device detection callback for automatic device creation */
	ksint32_t (*detect) (struct hal_i2c_client *, struct hal_i2c_board_info *);
	const kuint32_t *ptr_address_list;
	struct list_head sgrt_clients;
};

#endif /*!< __HAL_I2C_DEV_H_ */
