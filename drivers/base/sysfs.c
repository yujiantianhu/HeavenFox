/*
 * Gerneral Interface : SysFs
 *
 * File Name:   sysfs.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.06.10
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/base/fwk_inode.h>
#include <platform/base/fwk_fs.h>
#include <platform/base/fwk_fcntl.h>
#include <platform/base/fwk_kobj.h>

/*!< The globals */
struct fwk_kset *sptr_sys_fs;
struct fwk_kset *sptr_sysfs_devices;
struct fwk_kset *sptr_sysfs_drivers;

struct fwk_kset *sptr_devices_fs;
struct fwk_kobject *sptr_kobj_in;
struct fwk_kobject *sptr_kobj_out;
struct fwk_kobject *sptr_kobj_err;

/*!< API function */
/*!
 * @brief   create and initialize /sys/devices, ...
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t sysfs_devices_init(struct fwk_kset *sptr_head)
{
    kint32_t retval = ER_NORMAL;

    sptr_sysfs_devices = fwk_kset_create("devices", &sptr_head->sgtc_kobj);
    if (!isValid(sptr_sysfs_devices))
        return -ER_NOMEM;

    sptr_sysfs_devices->sgtc_kobj.sptr_kset = sptr_head;
    if (fwk_kset_register(sptr_sysfs_devices))
    {
        fwk_kset_destroy(sptr_sysfs_devices);
        return -ER_FAILD;
    }
    
    sptr_sysfs_drivers = fwk_kset_create("drivers", &sptr_head->sgtc_kobj);
    if (!isValid(sptr_sysfs_drivers))
    {
        retval = -ER_NOMEM;
        goto fail;
    }

    sptr_sysfs_drivers->sgtc_kobj.sptr_kset = sptr_head;
    if (fwk_kset_register(sptr_sysfs_drivers))
    {
        fwk_kset_destroy(sptr_sysfs_drivers);
        retval = -ER_FAILD;
        goto fail;
    }

    return ER_NORMAL;

fail:
    fwk_kset_kobject_remove(&sptr_sysfs_devices->sgtc_kobj);
    return retval;
}

/*!
 * @brief   destroy sys
 * @param   none
 * @retval  none
 * @note    none
 */
void sysfs_devices_destroy(void)
{
    fwk_kset_kobject_remove(&sptr_sysfs_devices->sgtc_kobj);
    fwk_kset_kobject_remove(&sptr_sysfs_drivers->sgtc_kobj);

    sptr_sysfs_devices = sptr_sysfs_drivers = mr_nullptr;
}

/*!
 * @brief   create and initialize /dev/stdin, ...
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t devfs_stdio_init(struct fwk_kset *sptr_head)
{
    kint32_t retval;

    sptr_kobj_in = fwk_kobject_create();
    if (!isValid(sptr_kobj_in))
        return -ER_NOMEM;

    sptr_kobj_out = fwk_kobject_create();
    if (!isValid(sptr_kobj_out))
    {
        retval = PTR_ERR(sptr_kobj_out);
        goto fail1;
    }

    sptr_kobj_err = fwk_kobject_create();
    if (!isValid(sptr_kobj_err))
    {
        retval = PTR_ERR(sptr_kobj_out);
        goto fail2;
    }

    sptr_kobj_in->sptr_kset  = sptr_head;
    sptr_kobj_out->sptr_kset = sptr_head;
    sptr_kobj_err->sptr_kset = sptr_head;

    retval = fwk_kobject_add(sptr_kobj_in, &sptr_head->sgtc_kobj, "stdin");
    if (retval)
        goto fail3;

    retval = fwk_kobject_add(sptr_kobj_out, &sptr_head->sgtc_kobj, "stdout");
    if (retval)
        goto fail4;

    retval = fwk_kobject_add(sptr_kobj_err, &sptr_head->sgtc_kobj, "stderr");
    if (retval)
        goto fail5;

    return ER_NORMAL;

fail5:
    fwk_kobject_del(sptr_kobj_out);
fail4:
    fwk_kobject_del(sptr_kobj_in);
fail3:
    kfree(sptr_kobj_err);
fail2:
    kfree(sptr_kobj_out);
fail1:
    kfree(sptr_kobj_in);

    return retval;
}

/*!
 * @brief   destroy dev
 * @param   none
 * @retval  none
 * @note    none
 */
void devfs_stdio_destroy(void)
{
    fwk_kobject_destroy(sptr_kobj_err);
    fwk_kobject_destroy(sptr_kobj_out);
    fwk_kobject_destroy(sptr_kobj_in);

    sptr_kobj_in = sptr_kobj_out = sptr_kobj_err = mr_nullptr;
}

/*!
 * @brief   sysfs_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init sysfs_init(void)
{
    kint32_t retval;

    /*!< create root path */
    if (fwk_kobject_root_init())
        return -ER_FAILD;

    sptr_sys_fs = fwk_kset_create_and_register("sys", mr_nullptr);
    if (!isValid(sptr_sys_fs))
        return -ER_NOMEM;

    sptr_devices_fs = fwk_kset_create_and_register("dev", mr_nullptr);
    if (!isValid(sptr_devices_fs))
        goto fail1;

    retval = sysfs_devices_init(sptr_sys_fs);
    if (retval)
        goto fail2;

    retval = devfs_stdio_init(sptr_devices_fs);
    if (retval)
        goto fail3;

    /*!< initialize file decriptor */
    if (fwk_file_system_init())
        goto fail4;

    return ER_NORMAL;

fail4:
    devfs_stdio_destroy();
fail3:
    sysfs_devices_destroy();
fail2:
    fwk_kset_unregister(sptr_devices_fs);
    kfree(sptr_devices_fs);
fail1:
    fwk_kset_unregister(sptr_sys_fs);
    kfree(sptr_sys_fs);

    return -ER_FAILD;
}

/*!
 * @brief   sysfs_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit sysfs_exit(void)
{
    /*!< do nothing */
}

IMPORT_EARLY_INIT(sysfs_init);
IMPORT_EARLY_EXIT(sysfs_exit);

/*!< end of file */
