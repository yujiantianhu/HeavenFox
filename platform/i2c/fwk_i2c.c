/*
 * Hardware Abstraction Layer I2C Interface
 *
 * File Name:   fwk_i2c.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.07
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/base/fwk_platform.h>
#include <platform/base/fwk_platdev.h>
#include <platform/base/fwk_platdrv.h>
#include <platform/i2c/fwk_i2c_dev.h>
#include <platform/i2c/fwk_i2c_core.h>
#include <platform/i2c/fwk_i2c_algo.h>

/*!< The globals */
extern struct fwk_bus_type sgtc_fwk_i2c_bus_type;
extern struct fwk_device_type sgtc_fwk_i2c_adapter_type;
extern struct fwk_device_type sgtc_fwk_i2c_client_type;

/*!< API function */
/*!
 * @brief   get the id_table that is matched
 * @param   sptr_idTable, sptr_client
 * @retval  id_table
 * @note    none
 */
static const struct fwk_i2c_device_id *fwk_i2c_match_id(const struct fwk_i2c_device_id *sptr_idTable, struct fwk_i2c_client *sptr_client)
{
    while (sptr_idTable->name && *(sptr_idTable->name))
    {
        if (!kstrcmp((char *)sptr_client->name, (char *)sptr_idTable->name))
            return sptr_idTable;

        sptr_idTable++;
    }

    return mr_nullptr;
}

/*!
 * @brief   Match device and driver
 * @param   device, driver
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_i2c_device_match(struct fwk_device *sptr_dev, struct fwk_driver *sptr_drv)
{
    struct fwk_i2c_client *sptr_client;
    struct fwk_i2c_driver *sptr_driver;
    struct fwk_device_node *sptr_np;

    if (sptr_dev->sptr_type != &sgtc_fwk_i2c_client_type)
        return -ER_CHECKERR;

    sptr_client = mr_container_of(sptr_dev, struct fwk_i2c_client, sgtc_dev);
    sptr_driver = mr_container_of(sptr_drv, struct fwk_i2c_driver, sgtc_driver);

    /*!<
     * Devices cannot have the same name as each other; Drivers cannot have the same name from one drive to another
     * Therefore, if you match the device name, it is not possible to match one driver with multiple devices
     * This function should be done by idTable
     */

    /*!< Match Priority 1: Device Tree */
    sptr_np = fwk_of_node_try_matches(sptr_dev->sptr_node, sptr_drv->sptr_of_match_table, mr_nullptr);
    if (isValid(sptr_np) && (sptr_dev->sptr_node == sptr_np))
        return ER_NORMAL;

    /*!< Match Priority 2: idTable */
    if (sptr_driver->sptr_id_table)
        return fwk_i2c_match_id(sptr_driver->sptr_id_table, sptr_client) ? ER_NORMAL : -ER_NOTFOUND;

    return -ER_NOTFOUND;
}

/*!
 * @brief   Probe after matching successfully
 * @param   device
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_i2c_device_probe(struct fwk_device *sptr_dev)
{
    struct fwk_driver  *sptr_drv;
    struct fwk_i2c_client *sptr_client;
    struct fwk_i2c_driver *sptr_driver;
    const struct fwk_i2c_device_id *sptr_idTable;

    sptr_drv = sptr_dev->sptr_driver;
    if (!sptr_drv)
        return -ER_FAULT;

    sptr_client = mr_container_of(sptr_dev, struct fwk_i2c_client, sgtc_dev);
    sptr_driver = mr_container_of(sptr_drv, struct fwk_i2c_driver, sgtc_driver);
    sptr_idTable = fwk_i2c_match_id(sptr_driver->sptr_id_table, sptr_client);

    /*!< get irq */
    if (!sptr_client->irq)
    {
        sptr_client->irq = fwk_of_irq_get(sptr_dev->sptr_node, 0);
        if (sptr_client->irq < 0)
            sptr_client->irq = -1;
    }

    if (sptr_driver->probe)
        return sptr_driver->probe(sptr_client, sptr_idTable) ? -ER_PERMIT : ER_NORMAL;

    return ER_NORMAL;
}

/*!
 * @brief   Remove on bus
 * @param   device
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_i2c_device_remove(struct fwk_device *sptr_dev)
{
    struct fwk_driver  *sptr_drv;
    struct fwk_i2c_client *sptr_client;
    struct fwk_i2c_driver *sptr_driver;

    sptr_drv = sptr_dev->sptr_driver;
    if (!sptr_drv)
        return -ER_FAULT;

    sptr_client = mr_container_of(sptr_dev, struct fwk_i2c_client, sgtc_dev);
    sptr_driver = mr_container_of(sptr_drv, struct fwk_i2c_driver, sgtc_driver);

    if ((!sptr_driver->remove) || (0 > sptr_driver->remove(sptr_client)))
        return -ER_PERMIT;

    return ER_NORMAL;
}

static struct fwk_bus_private sgtc_fwk_i2c_device_buspriv =
{
    .sptr_bus = &sgtc_fwk_i2c_bus_type,
};

struct fwk_bus_type sgtc_fwk_i2c_bus_type =
{
    .name	= "i2c-bus",

    .match	= fwk_i2c_device_match,
    .probe	= fwk_i2c_device_probe,
    .remove	= fwk_i2c_device_remove,

    .sptr_buspriv = &sgtc_fwk_i2c_device_buspriv,
};

struct fwk_device_type sgtc_fwk_i2c_adapter_type =
{
    .name = "i2c-adapter-type",
};

struct fwk_device_type sgtc_fwk_i2c_client_type =
{
    .name = "i2c-client-type",
};

/*!
 * @brief   register i2c client
 * @param   sptr_client
 * @retval  errno
 * @note    none
 */
kint32_t fwk_register_i2c_device(struct fwk_i2c_client *sptr_client)
{
    sptr_client->sgtc_dev.sptr_bus = &sgtc_fwk_i2c_bus_type;

    return fwk_device_add(&sptr_client->sgtc_dev);
}

/*!
 * @brief   unregister i2c client
 * @param   sptr_client
 * @retval  errno
 * @note    none
 */
kint32_t fwk_unregister_i2c_device(struct fwk_i2c_client *sptr_client)
{
    return fwk_device_del(&sptr_client->sgtc_dev);
}

/*!
 * @brief   register i2c driver
 * @param   sptr_driver
 * @retval  errno
 * @note    none
 */
kint32_t fwk_i2c_register_driver(struct fwk_i2c_driver *sptr_driver)
{
    sptr_driver->sgtc_driver.sptr_bus = &sgtc_fwk_i2c_bus_type;

    return fwk_driver_register(&sptr_driver->sgtc_driver);
}

/*!
 * @brief   unregister i2c driver
 * @param   sptr_driver
 * @retval  errno
 * @note    none
 */
kint32_t fwk_i2c_unregister_driver(struct fwk_i2c_driver *sptr_driver)
{
    return fwk_driver_unregister(&sptr_driver->sgtc_driver);
}

/*!
 * @brief   create and register i2c client
 * @param   sptr_adap, sptr_info
 * @retval  sptr_client
 * @note    none
 */
struct fwk_i2c_client *fwk_i2c_new_device(struct fwk_i2c_adapter *sptr_adap, struct fwk_i2c_board_info const *sptr_info)
{
    struct fwk_i2c_client *sptr_client;
    kint32_t retval;

    sptr_client = kzalloc(sizeof(*sptr_client), GFP_KERNEL);
    if (!isValid(sptr_client))
        return mr_nullptr;

    sptr_client->sptr_adapter = sptr_adap;
    sptr_client->irq = sptr_info->irq;
    sptr_client->addr = sptr_info->addr;
    sptr_client->flags = sptr_info->flags;
    kstrlcpy(sptr_client->name, sptr_info->type, sizeof(sptr_client->name));

    sptr_client->sgtc_dev.sptr_parent = &sptr_adap->sgtc_dev;
    sptr_client->sgtc_dev.sptr_bus = &sgtc_fwk_i2c_bus_type;
    sptr_client->sgtc_dev.sptr_type = &sgtc_fwk_i2c_client_type;
    sptr_client->sgtc_dev.sptr_node = sptr_info->sptr_node;
    mr_dev_set_name(&sptr_client->sgtc_dev, "i2c%d-%#x", 
                sptr_adap->nr, sptr_client->addr | ((sptr_client->flags & FWK_I2C_M_TEN) ? 0xa000 : 0));

    /*!< verify machine address */
    if (sptr_client->flags & FWK_I2C_M_TEN)
    {
        /*!< 10bits, 0x000 ~ 0x3ff */
        if (sptr_client->addr > 0x3ff)
            goto fail;
    }
    else
    {
        /*!< 7bits, 0x01 ~ 0x7f */
        if (!sptr_client->addr || (sptr_client->addr > 0x7f))
            goto fail;
    }

    retval = fwk_device_add(&sptr_client->sgtc_dev);
    if (retval)
        goto fail;

    wr_lock(&sptr_adap->sgtc_lock);
    list_head_add_tail(&sptr_adap->sgtc_clients, &sptr_client->sgtc_link);
    wr_unlock(&sptr_adap->sgtc_lock);

    return sptr_client;

fail:
    mr_dev_del_name(&sptr_client->sgtc_dev);
    kfree(sptr_client);

    return mr_nullptr;
}

/*!
 * @brief   unregister i2c client
 * @param   sptr_client
 * @retval  errno
 * @note    none
 */
static void __fwk_i2c_unregister_device(struct fwk_i2c_client *sptr_client)
{
    list_head_del(&sptr_client->sgtc_link);
    fwk_device_del(&sptr_client->sgtc_dev);
}

/*!
 * @brief   unregister i2c client
 * @param   sptr_client
 * @retval  errno
 * @note    none
 */
void fwk_i2c_unregister_device(struct fwk_i2c_client *sptr_client)
{
    wr_lock(&sptr_client->sptr_adapter->sgtc_lock);
    __fwk_i2c_unregister_device(sptr_client);
    wr_unlock(&sptr_client->sptr_adapter->sgtc_lock);
}

/*!
 * @brief   register the i2c client in sptr_adap
 * @param   sptr_adap, sptr_node
 * @retval  sptr_client
 * @note    none
 */
static struct fwk_i2c_client *fwk_of_i2c_register_device(struct fwk_i2c_adapter *sptr_adap, struct fwk_device_node *sptr_node)
{
    struct fwk_i2c_board_info sgtc_bi = {};
    kuint32_t reg = 0;

    if (fwk_of_modalias_node(sptr_node, sgtc_bi.type, sizeof(sgtc_bi.type)))
        return ERR_PTR(-ER_NOTFOUND);

    if (fwk_of_property_read_u32(sptr_node, "reg", &reg))
        return ERR_PTR(-ER_NOTFOUND);

    if (!reg)
        return ERR_PTR(-ER_EMPTY);

    sgtc_bi.sptr_node = sptr_node;
    sgtc_bi.addr = (kuint16_t)reg;

    return fwk_i2c_new_device(sptr_adap, &sgtc_bi);
}

/*!
 * @brief   register all i2c clients in sptr_adap
 * @param   sptr_adap
 * @retval  none
 * @note    none
 */
void fwk_of_i2c_register_devices(struct fwk_i2c_adapter *sptr_adap)
{
    struct fwk_device_node *sptr_node;

    if (!sptr_adap->sgtc_dev.sptr_node)
        return;

    foreach_fwk_of_child(sptr_adap->sgtc_dev.sptr_node, sptr_node)
    {
        if (fwk_of_device_is_avaliable(sptr_node))
            fwk_of_i2c_register_device(sptr_adap, sptr_node);
    }
}

/*!
 * @brief   register i2c adapter
 * @param   sptr_adap
 * @retval  errno
 * @note    none
 */
kint32_t fwk_i2c_register_adapter(struct fwk_i2c_adapter *sptr_adap)
{
    kint32_t retval;

    if (!sptr_adap->sptr_algo || !(*sptr_adap->name))
        return -ER_FAULT;

    if (!sptr_adap->timeout)
        sptr_adap->timeout = TICK_HZ;

    init_list_head(&sptr_adap->sgtc_clients);
    mr_dev_set_name(&sptr_adap->sgtc_dev, "i2c-%d", sptr_adap->nr);

    sptr_adap->sgtc_dev.sptr_bus = &sgtc_fwk_i2c_bus_type;
    sptr_adap->sgtc_dev.sptr_type = &sgtc_fwk_i2c_adapter_type;

    rw_lock_init(&sptr_adap->sgtc_lock);

    retval = fwk_device_add(&sptr_adap->sgtc_dev);
    if (retval)
        goto fail;

    fwk_of_i2c_register_devices(sptr_adap);

    return ER_NORMAL;

fail:
    mr_dev_del_name(&sptr_adap->sgtc_dev);
    return retval;
}

/*!
 * @brief   register i2c adapter
 * @param   sptr_adap
 * @retval  errno
 * @note    none
 */
kint32_t fwk_i2c_add_adapter(struct fwk_i2c_adapter *sptr_adap)
{
    if (sptr_adap->id < 0)
    {
        sptr_adap->nr = fwk_of_get_alias_id(sptr_adap->sgtc_dev.sptr_node);
        if (sptr_adap->nr < 0)
            return sptr_adap->nr;
    }
    else
        sptr_adap->nr = sptr_adap->id;

    return fwk_i2c_register_adapter(sptr_adap);
}

/*!
 * @brief   unregister i2c adapter
 * @param   sptr_adap
 * @retval  none
 * @note    none
 */
void fwk_i2c_del_adapter(struct fwk_i2c_adapter *sptr_adap)
{
    struct fwk_i2c_client *sptr_client, *sptr_temp;

    wr_lock(&sptr_adap->sgtc_lock);

    foreach_list_next_entry_safe(sptr_client, sptr_temp, &sptr_adap->sgtc_clients, sgtc_link)
    {
        __fwk_i2c_unregister_device(sptr_client);
        kfree(sptr_client);
    }

    wr_unlock(&sptr_adap->sgtc_lock);
    fwk_device_del(&sptr_adap->sgtc_dev);
}

/*!
 * @brief   i2c transfer
 * @param   sptr_client, sptr_msgs, num (the number of sptr_msgs)
 * @retval  errno
 * @note    none
 */
kint32_t fwk_i2c_transfer(struct fwk_i2c_client *sptr_client, struct fwk_i2c_msg *sptr_msgs, kint32_t num)
{
    const struct fwk_i2c_algo *sptr_algo = sptr_client->sptr_adapter->sptr_algo;

    if (sptr_algo->master_xfer)
        return sptr_algo->master_xfer(sptr_client->sptr_adapter, sptr_msgs, num);

    return -ER_NSUPPORT;
}

/*!
 * @brief   i2c write byte
 * @param   sptr_client: i2c device
 * @param   reg: register will operate
 * @param   value: data will be write to reg
 * @retval  errno
 * @note    none
 */
kint32_t fwk_i2c_check_slave(struct fwk_i2c_client *sptr_client)
{
    const struct fwk_i2c_algo *sptr_algo = sptr_client->sptr_adapter->sptr_algo;
    struct fwk_i2c_msg sgtc_msgs;

    if (!sptr_algo ||
        !sptr_algo->master_xfer)
        return -ER_NSUPPORT;

    sgtc_msgs.addr = sptr_client->addr;
    sgtc_msgs.flags = 0;
    sgtc_msgs.ptr_buf = mr_nullptr;
    sgtc_msgs.len = 0;

    return sptr_algo->master_xfer(sptr_client->sptr_adapter, &sgtc_msgs, 1);
}

/*!
 * @brief   i2c write byte
 * @param   sptr_client: i2c device
 * @param   reg: register will operate
 * @param   value: data will be write to reg
 * @retval  errno
 * @note    none
 */
kint32_t fwk_i2c_write_byte_data(struct fwk_i2c_client *sptr_client, kuint8_t reg, kuint8_t value)
{
    const struct fwk_i2c_algo *sptr_algo = sptr_client->sptr_adapter->sptr_algo;
    struct fwk_i2c_msg sgtc_msgs;
    kuint8_t buf[2];

    if (!sptr_algo ||
        !sptr_algo->master_xfer)
        return -ER_NSUPPORT;

    buf[0] = reg;
    buf[1] = value;

    sgtc_msgs.addr = sptr_client->addr;
    sgtc_msgs.flags = 0;
    sgtc_msgs.ptr_buf = buf;
    sgtc_msgs.len = sizeof(buf);

    return sptr_algo->master_xfer(sptr_client->sptr_adapter, &sgtc_msgs, 1);
}

/*!
 * @brief   i2c read byte
 * @param   sptr_client: i2c device
 * @param   reg: register will operate
 * @retval  errno/value
 * @note    none
 */
kint32_t fwk_i2c_read_byte_data(struct fwk_i2c_client *sptr_client, kuint8_t reg)
{
    const struct fwk_i2c_algo *sptr_algo = sptr_client->sptr_adapter->sptr_algo;
    struct fwk_i2c_msg sgtc_msgs[2];
    kuint8_t value = 0;
    kint32_t retval;

    if (!sptr_algo ||
        !sptr_algo->master_xfer)
        return -ER_NSUPPORT;

    sgtc_msgs[0].addr = sptr_client->addr;
    sgtc_msgs[0].flags = 0;
    sgtc_msgs[0].ptr_buf = &reg;
    sgtc_msgs[0].len = 1;

    sgtc_msgs[1].addr = sptr_client->addr;
    sgtc_msgs[1].flags = FWK_I2C_M_RD;
    sgtc_msgs[1].ptr_buf = &value;
    sgtc_msgs[1].len = sizeof(value);

    retval = sptr_algo->master_xfer(sptr_client->sptr_adapter, &sgtc_msgs[0], ARRAY_SIZE(sgtc_msgs));
    return retval ? retval : value;
}

/*!< --------------------------------------------------------------------------- */
/*!
 * @brief   i2c init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __plat_init fwk_i2c_global_init(void)
{
    struct fwk_bus_private *sptr_buspriv = &sgtc_fwk_i2c_device_buspriv;

    sptr_buspriv->sptr_bus = &sgtc_fwk_i2c_bus_type,

    init_list_head(&sptr_buspriv->sgtc_list_devices);
    init_list_head(&sptr_buspriv->sgtc_list_drivers);

    rw_lock_init(&sptr_buspriv->sgtc_device_lock);
    rw_lock_init(&sptr_buspriv->sgtc_driver_lock);

    return ER_NORMAL;
}

/*!
 * @brief   i2c exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __plat_exit fwk_i2c_global_exit(void)
{

}

IMPORT_LATE_INIT(fwk_i2c_global_init);
IMPORT_LATE_EXIT(fwk_i2c_global_exit);

/*!< end of file */
