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

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/base/fwk_kobj.h>
#include <platform/of/fwk_of.h>
#include <platform/irq/fwk_irq_types.h>
#include <kernel/rw_lock.h>

/*!< The defines */
struct fwk_pinctrl_dev_info;
struct fwk_dev_pm_ops;

typedef struct fwk_SysPrivate
{
    struct fwk_bus_type *sptr_bus;

    /*!< Device list */
    struct list_head sgtc_list_devices;
    struct rw_lock sgtc_device_lock;

    /*!< Driver list */
    struct list_head sgtc_list_drivers;
    struct rw_lock sgtc_driver_lock;
    
} srt_fwk_SysPrivate_t;

typedef struct fwk_device
{
    kchar_t *init_name;
    struct fwk_device *sptr_parent;

    struct fwk_bus_type *sptr_bus;
    struct fwk_device_type *sptr_type;
    struct list_head sgtc_link;
    struct list_head sgtc_leaf;

    struct fwk_driver *sptr_driver;
    struct fwk_kobject sgtc_kobj;

    struct fwk_device_node *sptr_node;
    kint32_t (*release)	(struct fwk_device *sptr_dev);

    struct fwk_pinctrl_dev_info *sptr_pctlinfo;
    void *privData;

} srt_fwk_device_t;

typedef struct fwk_driver
{
    kchar_t *name;
    kint32_t id;

    kint32_t matches;

    const struct fwk_of_device_id *sptr_of_match_table;

    struct fwk_bus_type *sptr_bus;
    struct list_head sgtc_link;

    kint32_t (*probe)	(struct fwk_device *sptr_dev);
    kint32_t (*remove)	(struct fwk_device *sptr_dev);

    struct fwk_device_oprts *sptr_oprts;

} srt_fwk_driver_t;

typedef struct fwk_bus_type
{
    kchar_t *name;

    kint32_t (*match)	(struct fwk_device *sptr_dev, struct fwk_driver *sptr_driver);
    kint32_t (*probe)	(struct fwk_device *sptr_dev);
    kint32_t (*remove)	(struct fwk_device *sptr_dev);

    struct fwk_SysPrivate *sptr_SysPriv;

} srt_fwk_bus_type_t;

typedef struct fwk_device_type 
{
    const kchar_t *name;
    void (*release)(struct fwk_device *sptr_dev);

    const struct fwk_dev_pm_ops *sptr_pm;

} srt_fwk_device_type_t;

#define FWK_GET_BUS_DEVICE(bus)								(&(bus)->sptr_SysPriv->sgtc_list_devices)
#define FWK_GET_BUS_DRIVER(bus)								(&(bus)->sptr_SysPriv->sgtc_list_drivers)

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
#define FWK_NEXT_DEVICE(parent, list)						mr_list_parent(parent, list, struct fwk_device, sgtc_link)
#define FWK_NEXT_DRIVER(parent, list)						mr_list_parent(parent, list, struct fwk_driver, sgtc_link)

#define __BUS_DEVICE_RD_LOCK(bus)                           rd_lock(&(bus)->sptr_SysPriv->sgtc_device_lock)
#define __BUS_DEVICE_RD_UNLOCK(bus)                         rd_unlock(&(bus)->sptr_SysPriv->sgtc_device_lock)
#define __BUS_DEVICE_WR_LOCK(bus)                           wr_lock(&(bus)->sptr_SysPriv->sgtc_device_lock)
#define __BUS_DEVICE_WR_UNLOCK(bus)                         wr_unlock(&(bus)->sptr_SysPriv->sgtc_device_lock)

#define __BUS_DRIVER_RD_LOCK(bus)                           rd_lock(&(bus)->sptr_SysPriv->sgtc_driver_lock)
#define __BUS_DRIVER_RD_UNLOCK(bus)                         rd_unlock(&(bus)->sptr_SysPriv->sgtc_driver_lock)
#define __BUS_DRIVER_WR_LOCK(bus)                           wr_lock(&(bus)->sptr_SysPriv->sgtc_driver_lock)
#define __BUS_DRIVER_WR_UNLOCK(bus)                         wr_unlock(&(bus)->sptr_SysPriv->sgtc_driver_lock)

/*!< The globals */
extern struct fwk_bus_type sgtc_fwk_platform_bus_type;

/*!< The functions */
extern kint32_t fwk_device_driver_probe(struct fwk_device *sptr_dev);
extern kint32_t fwk_device_driver_remove(struct fwk_device *sptr_dev);
extern kint32_t fwk_device_driver_match(struct fwk_device *sptr_dev, struct fwk_bus_type *sptr_bus_type, void *ptr_data);

extern kint32_t fwk_device_initial(struct fwk_device *sptr_dev);
extern struct fwk_device *fwk_device_create(kuint32_t type, kuint32_t devNum, kchar_t *fmt, ...);
extern kint32_t fwk_device_destroy(struct fwk_device *sptr_dev);

/*!< API functions */
/*!
 * @brief   get device name
 * @param   sptr_dev
 * @retval  name
 * @note    none
 */
static inline kchar_t *fwk_dev_get_name(struct fwk_device *sptr_dev)
{
    if (sptr_dev->init_name)
        return sptr_dev->init_name;

    return fwk_kobject_get_name(&sptr_dev->sgtc_kobj);
}

/*!
 * @brief   set device name
 * @param   sptr_dev, name
 * @retval  none
 * @note    none
 */
static inline void fwk_dev_set_name(struct fwk_device *sptr_dev, kchar_t *name, ...)
{
    va_list sptr_list;

    va_start(sptr_list, name);
    fwk_kobject_set_name_args(&sptr_dev->sgtc_kobj, name, sptr_list);
    va_end(sptr_list);
}

/*!
 * @brief   release device name
 * @param   sptr_dev
 * @retval  none
 * @note    none
 */
static inline void fwk_dev_del_name(struct fwk_device *sptr_dev)
{
    fwk_kobject_del_name(&sptr_dev->sgtc_kobj);
}

#define mr_dev_get_name(dev)								fwk_dev_get_name(dev)
#define mr_dev_set_name(dev, fmt, ...)						fwk_dev_set_name(dev, fmt, ##__VA_ARGS__)
#define mr_dev_del_name(dev)								fwk_dev_del_name(dev)

#ifdef __cplusplus
    }
#endif

#endif /*!< __FWK_BUSTYPE_H_ */
