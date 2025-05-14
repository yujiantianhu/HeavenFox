/*
 * Platform Bus Interface of Hardware Abstraction Layer
 *
 * File Name:   fwk_platdev.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.23
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_platform.h>
#include <platform/fwk_platdev.h>
#include <kernel/rw_lock.h>

/*!< The defines */
struct fwk_platdev_object
{
    struct fwk_platdev sgtc_platdev;
    kchar_t name[];
};

/*!< The globals */
static struct fwk_device_type sgtc_fwk_platform_dev_type =
{
    .name = "platform-type",
};

static DECLARE_LIST_HEAD(sgtc_fwk_devices);
static struct rw_lock sgtc_fwk_devices_lock = RW_LOCK_INIT();

/*!< The functions */
static kint32_t fwk_device_attach(struct fwk_device *sptr_dev, struct fwk_bus_type *sptr_bus_type);
static kint32_t fwk_device_detach(struct fwk_device *sptr_dev);
static kint32_t fwk_device_to_bus(struct fwk_device *sptr_dev, struct fwk_bus_type *sptr_bus_type);
static kint32_t fwk_bus_del_device(struct fwk_device *sptr_dev, struct fwk_bus_type *sptr_bus_type);

/*!< API function */
/*!
 * @brief   release platform device
 * @param   sptr_dev
 * @retval  error code
 * @note    none
 */
static kint32_t fwk_platdevice_release(struct fwk_device *sptr_dev)
{
    struct fwk_platdev *sptr_platdev;
    struct fwk_platdev_object *sptr_platobj;
    kint32_t retval;

    if (!sptr_dev)
        return -ER_NODEV;

    sptr_platdev = mr_container_of(sptr_dev, struct fwk_platdev, sgtc_dev);
    sptr_platobj = mr_container_of(sptr_platdev, struct fwk_platdev_object, sgtc_platdev);

    retval = fwk_device_del(sptr_dev);
    if (!retval)
        kfree(sptr_platobj);

    return retval;
}

/*!
 * @brief   allocate platform device
 * @param   name, id
 * @retval  platform device pointer
 * @note    none
 */
struct fwk_platdev *fwk_platdevice_alloc(const kchar_t *name, kint32_t id)
{
    struct fwk_platdev_object *sptr_platobj;
    struct fwk_platdev *sptr_platdev;

    sptr_platobj = kzalloc(sizeof(*sptr_platobj) + strlen(name) + 1, GFP_KERNEL);
    if (!isValid(sptr_platobj))
        return ERR_PTR(-ER_NOMEM);

    sptr_platdev = &sptr_platobj->sgtc_platdev;
    if (fwk_device_initial(&sptr_platdev->sgtc_dev))
        goto fail;

    sptr_platdev->id = id;
    sptr_platdev->name = sptr_platobj->name;
    sptr_platdev->sgtc_dev.release = fwk_platdevice_release;

    return sptr_platdev;

fail:
    kfree(sptr_platobj);
    return ERR_PTR(-ER_FAILD);
}

/*!
 * @brief   add to platform bus
 * @param   sptr_platdev, sptr_node
 * @retval  error code
 * @note    none
 */
kint32_t fwk_platdevice_add(struct fwk_platdev *sptr_platdev)
{
    if (!sptr_platdev)
        return -ER_NODEV;

    if (sptr_platdev->id >= 0)
        mr_dev_set_name(&sptr_platdev->sgtc_dev, "%s-%d", sptr_platdev->name, sptr_platdev->id);
    else
        mr_dev_set_name(&sptr_platdev->sgtc_dev, "%s", sptr_platdev->name);

    sptr_platdev->sgtc_dev.sptr_bus = &sgtc_fwk_platform_bus_type;
    sptr_platdev->sgtc_dev.sptr_type = &sgtc_fwk_platform_dev_type;

    return fwk_device_add(&sptr_platdev->sgtc_dev);
}

/*!
 * @brief   Register Platform Device
 * @param   sptr_platdev
 * @retval  Register Result
 * @note    Should be used at initcall
 */
kint32_t fwk_register_platdevice(struct fwk_platdev *sptr_platdev)
{
    return fwk_platdevice_add(sptr_platdev);
}

/*!
 * @brief   Unregister Platform Device
 * @param   sptr_platdev
 * @retval  Unregister Result
 * @note    Should be used at exitcall
 */
kint32_t fwk_unregister_platdevice(struct fwk_platdev *sptr_platdev)
{
    if (sptr_platdev->sgtc_dev.release)
        return sptr_platdev->sgtc_dev.release(&sptr_platdev->sgtc_dev);

    return -ER_INVALID;
}

/*!< --------------------------------------------------------------------------
                            Device Local Interface						
 --------------------------------------------------------------------------- */
/*!
 * @brief   find device from the global list
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_device_find(struct fwk_device *sptr_dev)
{
    struct fwk_device *sptr_leaf;

    rd_lock(&sgtc_fwk_devices_lock);

    foreach_list_next_entry(sptr_leaf, &sgtc_fwk_devices, sgtc_leaf)
    {
        if (sptr_leaf == sptr_dev)
            goto succ;

        if (!strcmp(mr_dev_get_name(sptr_leaf), mr_dev_get_name(sptr_dev)))
            goto succ;
    }

    rd_unlock(&sgtc_fwk_devices_lock);
    return -ER_NOTFOUND;

succ:
    rd_unlock(&sgtc_fwk_devices_lock);
    return ER_NORMAL;
}

/*!
 * @brief   Device Match Interface
 * @param   device, platform-bus
 * @retval  errno
 * @note    Device actively matches driver
 */
static kint32_t fwk_device_attach(struct fwk_device *sptr_dev, struct fwk_bus_type *sptr_bus_type)
{
    struct fwk_driver *sptr_driver;
    kint32_t retval;

    DECLARE_LIST_HEAD_PTR(sptr_list);
    DECLARE_LIST_HEAD_PTR(sptr_parent);

    /*!< sptr_driver is not null, maybe this device has been matched to driver */
    if (sptr_dev->sptr_driver)
        return ER_NORMAL;

    /*!< check if "match" function defines in platform-bus */
    if (!sptr_bus_type->match)
        return -ER_NSUPPORT;

    FWK_INIT_BUS_DRIVER_LIST(sptr_parent, sptr_list, sptr_bus_type);
    __BUS_DRIVER_RD_LOCK(sptr_bus_type);

    /*!< get driver from bus one after another */
    while ((sptr_driver = FWK_NEXT_DRIVER(sptr_parent, sptr_list)))
    {
        /*!< try to attach this driver */
        retval = fwk_device_driver_match(sptr_dev, sptr_bus_type, sptr_driver);
        if (!retval || (retval == -ER_PERMIT))
        {
            __BUS_DRIVER_RD_UNLOCK(sptr_bus_type);
            return ER_NORMAL;
        }
    }

    __BUS_DRIVER_RD_UNLOCK(sptr_bus_type);
    return -ER_PERMIT;
}

/*!
 * @brief   Device dissolve the relationship with driver 
 * @param   device
 * @retval  errno
 * @note    Device actively leaves driver
 */
static kint32_t fwk_device_detach(struct fwk_device *sptr_dev)
{
    struct fwk_driver *sptr_driver;

    /*!< sptr_driver is null, no driver has been mathced */
    if (!sptr_dev->sptr_driver)
        return ER_NORMAL;

    sptr_driver	= sptr_dev->sptr_driver;

    /*!< prepare to separate */
    fwk_device_driver_remove(sptr_dev);

    /*!< do separattion */
    sptr_dev->sptr_driver = mr_nullptr;
    sptr_driver->matches--;

    return ER_NORMAL;
}

/*!
 * @brief   Add new device to bus
 * @param   device, platform-bus
 * @retval  errno
 * @note    After adding finished, excute device-driver matching
 */
static kint32_t fwk_device_to_bus(struct fwk_device *sptr_dev, struct fwk_bus_type *sptr_bus_type)
{
    kint32_t retval;

    /*!< add to list tail */
    __BUS_DEVICE_WR_LOCK(sptr_bus_type);
    list_head_add_tail(FWK_GET_BUS_DEVICE(sptr_bus_type), &sptr_dev->sgtc_link);
    __BUS_DEVICE_WR_UNLOCK(sptr_bus_type);

    /*!< do device-driver matching */
    retval = fwk_device_attach(sptr_dev, sptr_bus_type);
    return (!retval || (retval == -ER_PERMIT)) ? ER_NORMAL : retval;
}

/*!
 * @brief   delete device on bus
 * @param   device, platform-bus
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_bus_del_device(struct fwk_device *sptr_dev, struct fwk_bus_type *sptr_bus_type)
{
    /*!< do detaching before deleting */
    fwk_device_detach(sptr_dev);

    /*!< delete device */
    __BUS_DEVICE_WR_LOCK(sptr_bus_type);
    list_head_del_safe(FWK_GET_BUS_DEVICE(sptr_bus_type), &sptr_dev->sgtc_link);
    __BUS_DEVICE_WR_UNLOCK(sptr_bus_type);

    return ER_NORMAL;
}

/*!
 * @brief   Register device
 * @param   device
 * @retval  errno
 * @note    none
 */
kint32_t fwk_device_add(struct fwk_device *sptr_dev)
{
    struct fwk_bus_type *sptr_bus_type;
    kint32_t retval;

    sptr_bus_type = sptr_dev->sptr_bus;

    if (!fwk_device_find(sptr_dev))
        return -ER_EXISTED;

    /*!< platform-bus is not exsisted */
    if (sptr_bus_type)
    {
        /*!< device list is not exsisted */
        if (!sptr_bus_type->sptr_SysPriv)
            goto fail;

        /*!< fisrt register */
        sptr_dev->sptr_driver = mr_nullptr;

        /*!< add device to device list (platform-bus) */
        retval = fwk_device_to_bus(sptr_dev, sptr_bus_type);
        if (retval)
            return retval;
    }

    wr_lock(&sgtc_fwk_devices_lock);
    list_head_add_tail(&sgtc_fwk_devices, &sptr_dev->sgtc_leaf);
    wr_unlock(&sgtc_fwk_devices_lock);

    return ER_NORMAL;

fail:
    return -ER_ERROR;
}

/*!
 * @brief   Unregister device
 * @param   device
 * @retval  errno
 * @note    none
 */
kint32_t fwk_device_del(struct fwk_device *sptr_dev)
{
    struct fwk_bus_type *sptr_bus_type;
    kint32_t retval;

    sptr_bus_type = sptr_dev->sptr_bus;

    if (fwk_device_find(sptr_dev))
        return -ER_NOTFOUND;

    /*!< platform-bus is not exsisted */
    if (sptr_bus_type)
    {
        /*!< device list is not exsisted */
        if (!sptr_bus_type->sptr_SysPriv)
            goto fail;

        /*!< delete device on the bus */
        retval = fwk_bus_del_device(sptr_dev, sptr_bus_type);
        if (retval)
            return retval;
    }

    mr_dev_del_name(sptr_dev);

    wr_lock(&sgtc_fwk_devices_lock);
    list_head_del(&sptr_dev->sgtc_leaf);
    wr_unlock(&sgtc_fwk_devices_lock);

    return ER_NORMAL;

fail:
    return -ER_ERROR;
}

/* end of file */
