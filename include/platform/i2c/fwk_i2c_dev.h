/*
 * Hardware Abstraction Layer I2C Interface
 *
 * File Name:   fwk_i2c_dev.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.16
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_I2C_DEV_H_
#define __FWK_I2C_DEV_H_

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_platform.h>
#include <platform/of/fwk_of.h>
#include <platform/i2c/fwk_i2c_core.h>

/*!< The defines */
#define FWK_I2C_NAME_SIZE									(32)

struct fwk_i2c_device_id
{
	kstring_t *name;
	ksint32_t driver_data;
};

struct fwk_i2c_board_info
{
	ksint8_t type[FWK_I2C_NAME_SIZE];	     				/*!< device name */
	kuint16_t flags;				 						/*!< property */
	kuint16_t addr;											/*!< device slave address */
	void *ptr_platform_data;								/*!< device private data */

	struct fwk_device_node *sprt_node;
	ksint32_t irq;											/*!< Interrupt number */
};

struct fwk_i2c_client
{
	kuint16_t flags;										/*!< div., see below */
	kuint16_t addr;											/*!< chip address - NOTE: 7bit */
					            							/*!< addresses are stored in the _LOWER_ 7 bits */
	ksint8_t name[FWK_I2C_NAME_SIZE];
	struct fwk_i2c_adapter *sprt_adapter;					/*!< the adapter we sit on */
	struct fwk_i2c_driver *sprt_driver;						/*!< and our access routines */
	struct fwk_device sgrt_dev;		    					/*!< the device structure */
	ksint32_t irq;			        						/*!< irq issued by device	*/
	struct list_head sgrt_detected;
};

struct fwk_i2c_driver
{
	ksint32_t (*attach_adapter) (struct fwk_i2c_adapter *);
	ksint32_t (*detach_adapter) (struct fwk_i2c_adapter *);

	/*!< Standard driver model interfaces */
	ksint32_t (*probe) (struct fwk_i2c_client *, const struct fwk_i2c_device_id *);
	ksint32_t (*remove) (struct fwk_i2c_client *);

	/*!< driver model interfaces that don't relate to enumeration  */
	void (*shutdown) (struct fwk_i2c_client *);
	ksint32_t (*suspend) (struct fwk_i2c_client *, ksint32_t mesg);
	ksint32_t (*resume) (struct fwk_i2c_client *);
	ksint32_t (*command) (struct fwk_i2c_client *client, kuint32_t cmd, void *ptr_arg);

	struct fwk_driver sgrt_driver;
	const struct fwk_i2c_device_id *sprt_id_table;

	/*!< Device detection callback for automatic device creation */
	ksint32_t (*detect) (struct fwk_i2c_client *, struct fwk_i2c_board_info *);
	const kuint32_t *ptr_address_list;
	struct list_head sgrt_clients;
};

#endif /*!< __FWK_I2C_DEV_H_ */
