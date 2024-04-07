/*
 * Platform Bus Interface of Hardware Abstraction Layer
 *
 * File Name:   hal_platform.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.25
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_BUSTYPE_H_
#define __HAL_BUSTYPE_H_

/*!< The includes */
#include <platform/hal_basic.h>
#include <platform/of/hal_of.h>
#include <platform/irq/hal_irq_types.h>

/*!< The defines */
typedef ksint32_t			haldev_return_t;

struct hal_device_oprts
{
	/*!< Unified configuration */
	haldev_return_t (*init) 		(void *data);
	haldev_return_t (*release) 		(void *data);

	/*!< Separately configuration  */
	/*!< Clock configure */
	haldev_return_t (*setClk)		(void *data, ksint32_t clk);
	/*!< High level configure */
	haldev_return_t (*setHigh)		(void *data);
	/*!< Low level configure */
	haldev_return_t (*setLow)		(void *data);
	/*!< Level configure: the output can be selected high or low */
	haldev_return_t (*setLevel)		(void *data, kuint8_t level);
	/*!< Read level */
	haldev_return_t (*getLevel)		(void *data, kuint8_t *level);
	/*!< Direction configure */
	haldev_return_t (*setDir)		(void *data, kuint8_t dir);
	/*!< Interrupt configure: irq <interrupt number>, handler <isr: interrupt service routine>, flags <interrupt trigger mode (edge/level...)> */
	haldev_return_t (*setIntr)		(void *data, ksint32_t irq, irq_handler_t (*handler)(void *data), kuint32_t flags);
};

struct hal_SysPrivate
{
	struct hal_bus_type *sprt_bus_type;

	/*!< Device list */
	struct list_head sgrt_list_devices;
	/*!< Driver list */
	struct list_head sgrt_list_drivers;
};

struct hal_device
{
	struct hal_device *sprt_parent;

	struct hal_bus_type *sprt_bus_type;
	struct list_head sgrt_list;

	struct hal_device_oprts *sprt_oprts;
	struct hal_driver *sprt_driver;

	struct hal_device_node *sprt_node;
	void (*release)	(struct hal_device *sprt_device);

	void *privData;
};

struct hal_driver
{
	kstring_t *name;
	ksint32_t id;

	ksint32_t matches;

	struct hal_of_device_id *sprt_of_match_table;

	struct hal_bus_type *sprt_bus_type;
	struct list_head sgrt_list;

	ksint32_t (*probe)	(struct hal_device *sprt_device);
	ksint32_t (*remove)	(struct hal_device *sprt_device);

	struct hal_device_oprts *sprt_oprts;
};

struct hal_bus_type
{
	kstring_t *name;

	ksint32_t (*match)	(struct hal_device *sprt_device, struct hal_driver *sprt_driver);
	ksint32_t (*probe)	(struct hal_device *sprt_device);
	ksint32_t (*remove)	(struct hal_device *sprt_device);

	struct hal_SysPrivate *sprt_SysPriv;
};

#define HAL_GET_BUS_DEVICE(bus)								(&bus->sprt_SysPriv->sgrt_list_devices)
#define HAL_GET_BUS_DRIVER(bus)								(&bus->sprt_SysPriv->sgrt_list_drivers)

#define HAL_INIT_BUS_DEVICE_LIST(parent, list, bus)	\
{	\
	parent 	= HAL_GET_BUS_DEVICE(bus);	\
	list	= parent;	\
}
#define HAL_INIT_BUS_DRIVER_LIST(parent, list, bus)	\
{	\
	parent 	= HAL_GET_BUS_DRIVER(bus);	\
	list	= parent;	\
}
#define HAL_NEXT_DEVICE(parent, list)						mrt_list_head_parent(parent, list, struct hal_device, sgrt_list)
#define HAL_NEXT_DRIVER(parent, list)						mrt_list_head_parent(parent, list, struct hal_driver, sgrt_list)

/*!< The globals */
TARGET_EXT struct hal_bus_type sgrt_hal_platform_bus_type;
TARGET_EXT struct hal_bus_type sgrt_hal_spi_bus_type;
TARGET_EXT struct hal_bus_type sgrt_hal_i2c_bus_type;

/*!< The functions */
TARGET_EXT ksint32_t hal_device_driver_probe(struct hal_device *sprt_device);
TARGET_EXT ksint32_t hal_device_driver_remove(struct hal_device *sprt_device);
TARGET_EXT ksint32_t hal_device_driver_match(struct hal_device *sprt_device, struct hal_bus_type *sprt_bus_type, void *ptr_data);

#endif /*!< __HAL_BUSTYPE_H_ */
