/*
 * Platform Bus Interface of Hardware Abstraction Layer
 *
 * File Name:   fwk_platform.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.25
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_platform.h>
#include <platform/base/fwk_platdev.h>
#include <platform/base/fwk_platdrv.h>
#include <platform/base/fwk_pinctrl.h>
#include <platform/base/fwk_inode.h>

/*!<
 * One device can only be matched with one driver
 * One driver can be used for multiple devices
 */

/*!< API function */
/*!
 * @brief   Match device and driver
 * @param   device, driver
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_platform_match(struct fwk_device *sptr_dev, struct fwk_driver *sptr_driver)
{
    struct fwk_platdev *sptr_platdev;
    struct fwk_platdrv *sptr_platdrv;
    struct fwk_id_table *sptr_idTable;
    struct fwk_device_node *sptr_np;
    kuint32_t idTable_cnt;

    sptr_platdev = mr_container_of(sptr_dev, struct fwk_platdev, sgtc_dev);
    sptr_platdrv = mr_container_of(sptr_driver, struct fwk_platdrv, sgtc_driver);

    /*!<
     * Devices cannot have the same name as each other; Drivers cannot have the same name from one drive to another
     * Therefore, if you match the device name, it is not possible to match one driver with multiple devices
     * This function should be done by idTable
     */

    /*!< Matching priority 1: override, where override is set, all other matching methods will be invalidated */
    if (sptr_platdev->driver_override)
        return ((!kstrcmp(sptr_platdev->driver_override, sptr_driver->name)) ? ER_NORMAL : (-ER_FAILD));

    /*!< Match Priority 2: Device Tree */
    sptr_np = fwk_of_node_try_matches(sptr_dev->sptr_node, sptr_driver->sptr_of_match_table, mr_nullptr);
    if (isValid(sptr_np) && (sptr_dev->sptr_node == sptr_np))
        return ER_NORMAL;

    /*!< Match Priority 3: idTable */
    for (idTable_cnt = 0; idTable_cnt < sptr_platdrv->num_idTable; idTable_cnt++)
    {
        sptr_idTable = sptr_platdrv->sptr_idTable + idTable_cnt;

        if (!kstrcmp((char *)sptr_platdev->name, (char *)sptr_idTable->name))
            return ER_NORMAL;
    }

    /*!< Match Priority 4: Name */
    /*!< 
     * Each device is not allowed to have the same name;
     * So you can exit immediately after finding it, and there will be no more devices with the same name in the future */
    if (!kstrcmp((char *)sptr_platdev->name, (char *)sptr_driver->name))
        return ER_NORMAL;

    return -ER_NOTFOUND;
}

/*!
 * @brief   Probe after matching successfully
 * @param   device
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_platform_probe(struct fwk_device *sptr_dev)
{
    struct fwk_driver  *sptr_driver;
    struct fwk_platdev *sptr_platdev;
    struct fwk_platdrv *sptr_platdrv;

    sptr_driver	= sptr_dev->sptr_driver;
    if (!sptr_driver)
        return -ER_NOTFOUND;

    sptr_platdev = mr_container_of(sptr_dev, struct fwk_platdev, sgtc_dev);
    sptr_platdrv = mr_container_of(sptr_driver, struct fwk_platdrv, sgtc_driver);

    if ((!sptr_platdrv->probe) || (0 > sptr_platdrv->probe(sptr_platdev)))
    {
        print_warn("device driver probe anomaly, driver is: %s\r\n", sptr_platdrv->sgtc_driver.name);
        return -ER_PERMIT;
    }

    return ER_NORMAL;
}

/*!
 * @brief   Remove on bus
 * @param   device
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_platform_remove(struct fwk_device *sptr_dev)
{
    struct fwk_driver  *sptr_driver;
    struct fwk_platdev *sptr_platdev;
    struct fwk_platdrv *sptr_platdrv;

    sptr_driver	= sptr_dev->sptr_driver;
    if (!sptr_driver)
        return -ER_NOTFOUND;

    sptr_platdev = mr_container_of(sptr_dev, struct fwk_platdev, sgtc_dev);
    sptr_platdrv = mr_container_of(sptr_driver, struct fwk_platdrv, sgtc_driver);

    if ((!sptr_platdrv->remove) || (0 > sptr_platdrv->remove(sptr_platdev)))
        return -ER_PERMIT;

    return ER_NORMAL;
}

static struct fwk_SysPrivate sgtc_fwk_platform_SysPriv =
{
    .sptr_bus = &sgtc_fwk_platform_bus_type,

    .sgtc_list_devices	= LIST_HEAD_INIT(&sgtc_fwk_platform_SysPriv.sgtc_list_devices),
    .sgtc_device_lock   = RW_LOCK_INIT(),

    .sgtc_list_drivers	= LIST_HEAD_INIT(&sgtc_fwk_platform_SysPriv.sgtc_list_drivers),
    .sgtc_driver_lock   = RW_LOCK_INIT(),
};

struct fwk_bus_type sgtc_fwk_platform_bus_type =
{
    .name	= "platform",

    .match	= fwk_platform_match,
    .probe	= fwk_platform_probe,
    .remove	= fwk_platform_remove,

    .sptr_SysPriv = &sgtc_fwk_platform_SysPriv,
};

/*!
 * @brief   Probe operation
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_device_driver_probe(struct fwk_device *sptr_dev)
{
    struct fwk_bus_type *sptr_bus_type;
    kint32_t retval;

    sptr_bus_type = sptr_dev->sptr_bus;

    /*!< bind pinctrl before probe */
    if (fwk_pinctrl_bind_pins(sptr_dev))
        print_warn("bind pinctrl faild, device is: %s\r\n", mr_dev_get_name(sptr_dev));

    if (sptr_bus_type->probe)
    {
        retval = sptr_bus_type->probe(sptr_dev);
        if (retval)
        {
            fwk_pinctrl_unbind_pins(sptr_dev);
            print_err("probe device and driver failed! device is: %s\r\n", sptr_dev->init_name);

            return retval;
        }

        return ER_NORMAL;
    }

    return -ER_NSUPPORT;
}

/*!
 * @brief   Remove operation
 * @param   device
 * @retval  errno
 * @note    none
 */
kint32_t fwk_device_driver_remove(struct fwk_device *sptr_dev)
{
    struct fwk_bus_type *sptr_bus_type = sptr_dev->sptr_bus;

    return sptr_bus_type->remove ? sptr_bus_type->remove(sptr_dev) : (-ER_NSUPPORT);
}

/*!
 * @brief   Match device and driver
 * @param   device, platform-bus, driver
 * @retval  errno
 * @note    none
 */
kint32_t fwk_device_driver_match(struct fwk_device *sptr_dev, struct fwk_bus_type *sptr_bus_type, void *ptr_data)
{
    struct fwk_driver *sptr_driver;
    kint32_t retval;

    sptr_driver	= (struct fwk_driver *)ptr_data;
    retval = sptr_bus_type->match(sptr_dev, sptr_driver);
    if (retval < 0)
        return -ER_NOTFOUND;

    /*!< 
     * Save the device to indicate that the driver has been matched;
     * You only need to operate the device to obtain the driver's information 
     */
    sptr_dev->sptr_driver = sptr_driver;

    return fwk_device_driver_probe(sptr_dev);
}

/*!
 * @brief   device initialization
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
kint32_t fwk_device_initial(struct fwk_device *sptr_dev)
{
    struct fwk_kobject *sptr_kobj;

    init_list_head(&sptr_dev->sgtc_leaf);
    init_list_head(&sptr_dev->sgtc_link);
    fwk_kobject_init(&sptr_dev->sgtc_kobj);

    sptr_kobj = fwk_find_kobject_by_path(mr_nullptr, FWK_PATH_SYS_DEVICE);
    if (!isValid(sptr_kobj))
        return PTR_ERR(sptr_kobj);

    sptr_dev->sgtc_kobj.sptr_kset = mr_fwk_kset_get(sptr_kobj);
    if (!isValid(sptr_dev->sgtc_kobj.sptr_kset))
        return PTR_ERR(sptr_dev->sgtc_kobj.sptr_kset);

    return ER_NORMAL;
}

/*!
 * @brief   fwk_device_create
 * @param   none
 * @retval  none
 * @note    none
 */
struct fwk_device *fwk_device_create(kuint32_t type, kuint32_t devNum, kchar_t *fmt, ...)
{
    struct fwk_kobject *sptr_kobj;
    struct fwk_kobject *sptr_parent;
    struct fwk_device *sptr_dev;
    kchar_t *name;
    va_list sptr_valist;

    if (!fmt)
        return ERR_PTR(-ER_ERROR);

    sptr_parent = fwk_find_kobject_by_path(mr_nullptr, FWK_PATH_DEVICE);
    if (!isValid(sptr_parent))
        return ERR_PTR(-ER_NOTFOUND);

    va_start(sptr_valist, fmt);
    name = vasprintk_safe(fmt, mr_nullptr, sptr_valist);
    va_end(sptr_valist);

    if (!name)
        return ERR_PTR(-ER_NOMEM);

    sptr_kobj = fwk_kobject_populate(sptr_parent, name);
    if (!isValid(sptr_kobj))
        goto fail1;

    if (sptr_kobj->sptr_inode->type == INODE_TYPE_FILE)
        fwk_inode_set_ops(sptr_kobj->sptr_inode, type, devNum);

    sptr_dev = kzalloc(sizeof(*sptr_dev), GFP_KERNEL);
    if (!isValid(sptr_dev))
        goto fail2;

    if (fwk_device_initial(sptr_dev))
        goto fail3;

    mr_dev_set_name(sptr_dev, "%s", name);
    if (fwk_device_add(sptr_dev))
        goto fail4;

    fmt_free(name);
    return sptr_dev;

fail4:
    mr_dev_del_name(sptr_dev);
fail3:
    kfree(sptr_dev);
fail2:
    fwk_kset_kobject_remove(sptr_kobj);
fail1:
    fmt_free(name);
    return ERR_PTR(-ER_FAILD);
}

/*!
 * @brief   fwk_device_destroy
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_device_destroy(struct fwk_device *sptr_dev)
{
    struct fwk_kobject *sptr_parent;
    struct fwk_kobject *sptr_kobj;

    if (!sptr_dev)
        return -ER_NOMEM;

    sptr_parent = fwk_find_kobject_by_path(mr_nullptr, FWK_PATH_DEVICE);
    if (!isValid(sptr_parent))
        return -ER_NOTFOUND;

    sptr_kobj = fwk_find_kobject_by_path(sptr_parent, mr_dev_get_name(sptr_dev));
    if (!isValid(sptr_kobj))
        return -ER_NOTFOUND;

    fwk_device_del(sptr_dev);
    fwk_kset_kobject_remove(sptr_kobj);

    return ER_NORMAL;
}

/*!< end of file */
