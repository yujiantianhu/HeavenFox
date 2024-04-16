/*
 * Platform Bus Interface of Hardware Abstraction Layer
 *
 * File Name:   fwk_platform.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.25
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_BUSTYPE_H_
#define __FWK_BUSTYPE_H_

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/of/fwk_of.h>
#include <platform/irq/fwk_irq_types.h>

/*!< The defines */
typedef ksint32_t			haldev_return_t;

struct fwk_device_oprts
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

struct fwk_SysPrivate
{
	struct fwk_bus_type *sprt_bus_type;

	/*!< Device list */
	struct list_head sgrt_list_devices;
	/*!< Driver list */
	struct list_head sgrt_list_drivers;
};

struct fwk_device
{
	struct fwk_device *sprt_parent;

	struct fwk_bus_type *sprt_bus_type;
	struct list_head sgrt_list;

	struct fwk_device_oprts *sprt_oprts;
	struct fwk_driver *sprt_driver;

	struct fwk_device_node *sprt_node;
	void (*release)	(struct fwk_device *sprt_dev);

	void *privData;
};

struct fwk_driver
{
	kstring_t *name;
	ksint32_t id;

	ksint32_t matches;

	struct fwk_of_device_id *sprt_of_match_table;

	struct fwk_bus_type *sprt_bus_type;
	struct list_head sgrt_list;

	ksint32_t (*probe)	(struct fwk_device *sprt_dev);
	ksint32_t (*remove)	(struct fwk_device *sprt_dev);

	struct fwk_device_oprts *sprt_oprts;
};

struct fwk_bus_type
{
	kstring_t *name;

	ksint32_t (*match)	(struct fwk_device *sprt_dev, struct fwk_driver *sprt_driver);
	ksint32_t (*probe)	(struct fwk_device *sprt_dev);
	ksint32_t (*remove)	(struct fwk_device *sprt_dev);

	struct fwk_SysPrivate *sprt_SysPriv;
};

#define FWK_GET_BUS_DEVICE(bus)								(&bus->sprt_SysPriv->sgrt_list_devices)
#define FWK_GET_BUS_DRIVER(bus)								(&bus->sprt_SysPriv->sgrt_list_drivers)

#define FWK_INIT_BUS_DEVICE_LIST(parent, list, bus)	\
{	\
	parent 	= FWK_GET_BUS_DEVICE(bus);	\
	list	= parent;	\
}
#define FWK_INIT_BUS_DRIVER_LIST(parent, list, bus)	\
{	\
	parent 	= FWK_GET_BUS_DRIVER(bus);	\
	list	= parent;	\
}
#define FWK_NEXT_DEVICE(parent, list)						mrt_list_head_parent(parent, list, struct fwk_device, sgrt_list)
#define FWK_NEXT_DRIVER(parent, list)						mrt_list_head_parent(parent, list, struct fwk_driver, sgrt_list)

/*!< The globals */
TARGET_EXT struct fwk_bus_type sgrt_fwk_platform_bus_type;
TARGET_EXT struct fwk_bus_type sgrt_fwk_spi_bus_type;
TARGET_EXT struct fwk_bus_type sgrt_fwk_i2c_bus_type;

/*!< The functions */
TARGET_EXT ksint32_t fwk_device_driver_probe(struct fwk_device *sprt_dev);
TARGET_EXT ksint32_t fwk_device_driver_remove(struct fwk_device *sprt_dev);
TARGET_EXT ksint32_t fwk_device_driver_match(struct fwk_device *sprt_dev, struct fwk_bus_type *sprt_bus_type, void *ptr_data);

#endif /*!< __FWK_BUSTYPE_H_ */
