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
struct fwk_pinctrl_dev_info;
struct fwk_dev_pm_ops;

typedef struct fwk_SysPrivate
{
	struct fwk_bus_type *sprt_bus;

	/*!< Device list */
	struct list_head sgrt_list_devices;
	/*!< Driver list */
	struct list_head sgrt_list_drivers;
	
} srt_fwk_SysPrivate_t;

typedef struct fwk_device
{
	kstring_t *init_name;

	struct fwk_device *sprt_parent;

	struct fwk_bus_type *sprt_bus;
	struct fwk_device_type *sprt_type;
	struct list_head sgrt_list;

	struct fwk_driver *sprt_driver;

	struct fwk_device_node *sprt_node;
	void (*release)	(struct fwk_device *sprt_dev);

	struct fwk_pinctrl_dev_info *sprt_pctlinfo;
	void *privData;

} srt_fwk_device_t;

typedef struct fwk_driver
{
	kstring_t *name;
	ksint32_t id;

	ksint32_t matches;

	const struct fwk_of_device_id *sprt_of_match_table;

	struct fwk_bus_type *sprt_bus;
	struct list_head sgrt_list;

	ksint32_t (*probe)	(struct fwk_device *sprt_dev);
	ksint32_t (*remove)	(struct fwk_device *sprt_dev);

	struct fwk_device_oprts *sprt_oprts;

} srt_fwk_driver_t;

typedef struct fwk_bus_type
{
	kstring_t *name;

	ksint32_t (*match)	(struct fwk_device *sprt_dev, struct fwk_driver *sprt_driver);
	ksint32_t (*probe)	(struct fwk_device *sprt_dev);
	ksint32_t (*remove)	(struct fwk_device *sprt_dev);

	struct fwk_SysPrivate *sprt_SysPriv;

} srt_fwk_bus_type_t;

typedef struct fwk_device_type 
{
	const kstring_t *name;
	void (*release)(struct fwk_device *sprt_dev);

	const struct fwk_dev_pm_ops *sprt_pm;

} srt_fwk_device_type_t;

#define mrt_get_dev_name(dev)								((dev)->init_name)
#define mrt_set_dev_name(dev, fmt, ...)						sprintk((dev)->init_name, fmt, ##__VA_ARGS__)

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

/*!< The functions */
TARGET_EXT ksint32_t fwk_device_driver_probe(struct fwk_device *sprt_dev);
TARGET_EXT ksint32_t fwk_device_driver_remove(struct fwk_device *sprt_dev);
TARGET_EXT ksint32_t fwk_device_driver_match(struct fwk_device *sprt_dev, struct fwk_bus_type *sprt_bus_type, void *ptr_data);

#endif /*!< __FWK_BUSTYPE_H_ */
