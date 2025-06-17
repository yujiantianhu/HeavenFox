/*
 * Template of EEPROM Driver : at24cxx
 *
 * File Name:   at24cxx.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.06.07
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/base/fwk_platdrv.h>
#include <platform/base/fwk_uaccess.h>
#include <platform/gpio/fwk_gpiodesc.h>
#include <platform/i2c/fwk_i2c_dev.h>
#include <platform/i2c/fwk_i2c_core.h>
#include <platform/i2c/fwk_i2c_algo.h>
#include <platform/base/fwk_cdev.h>
#include <platform/base/fwk_chrdev.h>
#include <platform/base/fwk_inode.h>
#include <platform/base/fwk_fs.h>
#include <platform/base/fwk_fcntl.h>
#include <platform/i2c/fwk_eeprom.h>
#include <kernel/sleep.h>
#include <kernel/spinlock.h>

/*!< The defines */
#define AT24CXX_DRVIVER_MAJOR                       (222)

#define AT24C01_PAGE_SIZE                           (8)
#define AT24C01_PAGE_COUNT                          (16)

/*!<
 * at24c01: 1kbits = 128bytes
 * at24cxx: 2kbits = 256bytes
 */
#define AT24CXX_PAGE_OFFSET(addr, size)             ((addr) / (size))

typedef struct at24cxx_drv_info
{
    kchar_t *name;
    struct fwk_i2c_client *sptr_client;

    kint32_t devnum;
    struct fwk_cdev *sptr_cdev;
    struct fwk_device *sptr_idev;

    kuint32_t page_size;
    kuint32_t total_size;

} at24cxx_drv_info_t;

/*!< API function */
/*!
 * @brief   at24cxx write
 * @param   sptr_info
 * @retval  errno
 * @note    none
 */
static kint32_t at24cxx_write_eeprom(struct at24cxx_drv_info *sptr_info, kuint8_t *buffer, kssize_t size)
{
#if 0
    kint32_t retval = ER_NORMAL;

    for (kint32_t index = 1; index < size; index++)
    {
        retval = fwk_i2c_write_byte_data(sptr_info->sptr_client, 
                                (*buffer) + index - 1, *(buffer + index));
        if (retval < 0)
            break;
    }

    return retval;

#else
    struct fwk_i2c_msg sgtc_msgs = {};

    sgtc_msgs.addr = sptr_info->sptr_client->addr;
    sgtc_msgs.flags = 0;
    sgtc_msgs.ptr_buf = buffer;
    sgtc_msgs.len = size;

    return fwk_i2c_transfer(sptr_info->sptr_client, &sgtc_msgs, 1);

#endif
}

/*!
 * @brief   at24cxx read
 * @param   sptr_info
 * @retval  errno
 * @note    none
 */
static kint32_t at24cxx_read_eeprom(struct at24cxx_drv_info *sptr_info, kuint8_t *buffer, kssize_t size)
{
#if 0
    kint32_t retval = ER_NORMAL;

    for (kint32_t index = 1; index < size; index++)
    {
        retval = fwk_i2c_read_byte_data(sptr_info->sptr_client, (*buffer) + index - 1);
        if (retval < 0)
            break;

        *(buffer + index) = (kuint8_t)retval;
    }

    return retval;

#else
    struct fwk_i2c_msg sgtc_msgs[2] = {};

    sgtc_msgs[0].addr = sptr_info->sptr_client->addr;
    sgtc_msgs[0].flags = 0;
    sgtc_msgs[0].ptr_buf = buffer;
    sgtc_msgs[0].len = 1;

    sgtc_msgs[1].addr = sptr_info->sptr_client->addr;
    sgtc_msgs[1].flags |= FWK_I2C_M_RD;
    sgtc_msgs[1].ptr_buf = buffer + 1;
    sgtc_msgs[1].len = size - 1;

    return fwk_i2c_transfer(sptr_info->sptr_client, &sgtc_msgs[0], ARRAY_SIZE(sgtc_msgs));

#endif
}

/*!
 * @brief   at24cxx write
 * @param   sptr_info
 * @retval  errno
 * @note    none
 */
static kint32_t __at24cxx_driver_write(struct at24cxx_drv_info *sptr_info, struct fwk_eeprom *sptr_eep)
{
    kuint8_t *buffer, *ptr_from;
    kuint32_t end_addr, last_left, last_page;
    kuint32_t first_page, first_left, pages;
    kuint32_t idx;
    kint32_t retval = ER_NORMAL;

    if (!sptr_eep->buf || !sptr_eep->size)
        return -ER_EMPTY;

    end_addr = sptr_eep->addr + sptr_eep->size;
    if (end_addr >= sptr_info->total_size)
        return -ER_MORE;

    buffer = kmalloc(sptr_eep->size + 1, GFP_KERNEL);
    if (!isValid(buffer))
        return -ER_NOMEM;

    ptr_from = sptr_eep->buf;
    first_left = last_left = 0;

    /*!< get the page offset of the address will be read */
    last_page = AT24CXX_PAGE_OFFSET(end_addr, sptr_info->page_size);
    last_left = end_addr - last_page * sptr_info->page_size;

    first_page = AT24CXX_PAGE_OFFSET(sptr_eep->addr, sptr_info->page_size);
    if (first_page != last_page)
        first_left = (first_page + 1) * sptr_info->page_size - sptr_eep->addr;

    pages = (last_page > (first_page + 1)) ? (last_page - first_page - 1) : 0;
    
    if (first_left)
    {
        fwk_copy_from_user(buffer + 1, ptr_from, first_left);

        *buffer = (kuint8_t)sptr_eep->addr + (kuint8_t)(ptr_from - sptr_eep->buf);
        retval = at24cxx_write_eeprom(sptr_info, buffer, first_left + 1);
        if (retval)
            goto END;

        ptr_from += first_left;
    }

    for (idx = 0; idx < pages; idx++)
    {
        ptr_from += idx * sptr_info->page_size;
        fwk_copy_from_user(buffer + 1, ptr_from, sptr_info->page_size);

        *buffer = (kuint8_t)sptr_eep->addr + (kuint8_t)(ptr_from - sptr_eep->buf);
        retval = at24cxx_write_eeprom(sptr_info, buffer, sptr_info->page_size + 1);
        if (retval)
            goto END;
    }
    ptr_from += idx * sptr_info->page_size;

    if (last_left)
    {
        fwk_copy_from_user(buffer + 1, ptr_from, last_left);

        *buffer = (kuint8_t)sptr_eep->addr + (kuint8_t)(ptr_from - sptr_eep->buf);
        retval = at24cxx_write_eeprom(sptr_info, buffer, last_left + 1);
        if (retval)
            goto END;
    }

END:
    kfree(buffer);
    return retval ? retval : sptr_eep->size;
}

/*!
 * @brief   at24cxx read
 * @param   sptr_info
 * @retval  errno
 * @note    none
 */
static kint32_t __at24cxx_driver_read(struct at24cxx_drv_info *sptr_info, struct fwk_eeprom *sptr_eep)
{
    kuint8_t *buffer, *ptr_to;
    kuint32_t end_addr, last_left, last_page;
    kuint32_t first_page, first_left, pages;
    kuint32_t idx;
    kint32_t retval = ER_NORMAL;

    if (!sptr_eep->buf || !sptr_eep->size)
        return -ER_EMPTY;

    end_addr = sptr_eep->addr + sptr_eep->size;
    if (end_addr >= sptr_info->total_size)
        return -ER_MORE;

    buffer = kmalloc(sptr_eep->size + 1, GFP_KERNEL);
    if (!isValid(buffer))
        return -ER_NOMEM;

    ptr_to = sptr_eep->buf;
    first_left = last_left = 0;

    /*!< get the page offset of the address will be read */
    last_page = AT24CXX_PAGE_OFFSET(end_addr, sptr_info->page_size);
    last_left = end_addr - last_page * sptr_info->page_size;

    first_page = AT24CXX_PAGE_OFFSET(sptr_eep->addr, sptr_info->page_size);
    if (first_page != last_page)
        first_left = (first_page + 1) * sptr_info->page_size - sptr_eep->addr;

    pages = (last_page > (first_page + 1)) ? (last_page - first_page - 1) : 0;
    
    if (first_left)
    {
        *buffer = (kuint8_t)sptr_eep->addr + (kuint8_t)(ptr_to - sptr_eep->buf);
        retval = at24cxx_read_eeprom(sptr_info, buffer, first_left + 1);
        if (retval)
            goto END;

        fwk_copy_to_user(ptr_to, buffer + 1, first_left);
        ptr_to += first_left;
    }

    for (idx = 0; idx < pages; idx++)
    {
        ptr_to += idx * sptr_info->page_size;

        *buffer = (kuint8_t)sptr_eep->addr + (kuint8_t)(ptr_to - sptr_eep->buf);
        retval = at24cxx_read_eeprom(sptr_info, buffer, sptr_info->page_size + 1);
        if (retval)
            goto END;

        fwk_copy_to_user(ptr_to, buffer + 1, sptr_info->page_size);
    }
    ptr_to += idx * sptr_info->page_size;

    if (last_left)
    {
        *buffer = (kuint8_t)sptr_eep->addr + (kuint8_t)(ptr_to - sptr_eep->buf);
        retval = at24cxx_read_eeprom(sptr_info, buffer, last_left + 1);
        if (retval)
            goto END;

        fwk_copy_to_user(ptr_to, buffer + 1, last_left);
    }

END:
    kfree(buffer);
    return retval ? retval : sptr_eep->size;
}

/*!
 * @brief   driver open
 * @param   sptr_inode, sptr_file
 * @retval  errno
 * @note    none
 */
static kint32_t at24cxx_driver_open(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    struct at24cxx_drv_info *sptr_info;

    sptr_info = sptr_inode->sptr_cdev->privData;
    sptr_file->private_data = sptr_info;

    return ER_NORMAL;
}

/*!
 * @brief   driver close
 * @param   sptr_inode, sptr_file
 * @retval  errno
 * @note    none
 */
static kint32_t at24cxx_driver_close(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    sptr_file->private_data = mr_nullptr;

    return ER_NORMAL;
}

/*!
 * @brief   driver ioctl
 * @param   sptr_file, cmd, args
 * @retval  errno
 * @note    none
 */
static kint32_t at24cxx_driver_ioctl(struct fwk_file *sptr_file, kuint32_t cmd, kuaddr_t args)
{
    struct at24cxx_drv_info *sptr_info;
    struct fwk_eeprom *sptr_eep;
    kint32_t retval = ER_NORMAL;
    
    sptr_info = sptr_file->private_data;
    sptr_eep = (struct fwk_eeprom *)args;

    switch (cmd)
    {
        case FWK_EEPROM_READ:
            retval = __at24cxx_driver_read(sptr_info, sptr_eep);
            break;

        case FWK_EEPROM_WRITE:
            retval = __at24cxx_driver_write(sptr_info, sptr_eep);
            break;

        default: break;
    }

    return (retval < 0) ? retval : ER_NORMAL;
}

static const struct fwk_file_oprts sgtc_at24cxx_driver_oprts =
{
    .open = at24cxx_driver_open,
    .close = at24cxx_driver_close,
    .unlocked_ioctl = at24cxx_driver_ioctl,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   at24cxx_driver_probe
 * @param   sptr_client, sptr_id
 * @retval  errno
 * @note    none
 */
static kint32_t at24cxx_driver_probe(struct fwk_i2c_client *sptr_client, const struct fwk_i2c_device_id *sptr_id)
{
    struct at24cxx_drv_info *sptr_info;
    struct fwk_device_node *sptr_node;
    struct fwk_device *sptr_idev;
    kint32_t devnum;
    kuint32_t pages, page_size;

    sptr_node = sptr_client->sgtc_dev.sptr_node;

    sptr_info = kzalloc(sizeof(*sptr_info), GFP_KERNEL);
    if (!isValid(sptr_info))
        return -ER_NOMEM;

    devnum = MKE_DEV_NUM(AT24CXX_DRVIVER_MAJOR, 0);
    sptr_info->devnum = devnum;
    sptr_info->name = "at24cxx";
    sptr_info->sptr_client = sptr_client;
    
    if (fwk_of_property_read_u32(sptr_node, "page_size", &page_size))
        page_size = AT24C01_PAGE_SIZE;

    if (fwk_of_property_read_u32(sptr_node, "pages", &pages))
        pages = AT24C01_PAGE_COUNT;

    sptr_info->page_size  = page_size;
    sptr_info->total_size = page_size * pages;

    if (fwk_register_chrdev(devnum, 1, sptr_info->name))
        goto fail1;

    sptr_info->sptr_cdev = fwk_cdev_alloc(&sgtc_at24cxx_driver_oprts);
    if (!isValid(sptr_info->sptr_cdev))
        goto fail2;

    if (fwk_cdev_add(sptr_info->sptr_cdev, devnum, 1))
        goto fail3;

    sptr_idev = fwk_device_create(NR_TYPE_CHRDEV, devnum, "%s", sptr_node->name);
    if (!isValid(sptr_idev))
        goto fail4;

    sptr_info->sptr_idev = sptr_idev;
    sptr_info->sptr_cdev->privData = sptr_info;
    fwk_i2c_set_client_data(sptr_client, sptr_info);

	return ER_NORMAL;
    
fail4:
    fwk_cdev_del(sptr_info->sptr_cdev);
fail3:
    kfree(sptr_info->sptr_cdev);
fail2:
    fwk_unregister_chrdev(devnum, 0);
fail1:
    kfree(sptr_info);

    return -ER_ERROR;
}

/*!
 * @brief   at24cxx_driver_remove
 * @param   sptr_client
 * @retval  errno
 * @note    none
 */
static kint32_t at24cxx_driver_remove(struct fwk_i2c_client *sptr_client)
{
    struct at24cxx_drv_info *sptr_info;

    sptr_info = fwk_i2c_get_client_data(sptr_client);

    fwk_device_destroy(sptr_info->sptr_idev);
    fwk_cdev_del(sptr_info->sptr_cdev);
    kfree(sptr_info->sptr_cdev);
    fwk_unregister_chrdev(sptr_info->devnum, 0);
    kfree(sptr_info);
    fwk_i2c_set_client_data(sptr_client, mr_nullptr);

    return ER_NORMAL;
}

static const struct fwk_i2c_device_id sgtc_at24cxx_driver_ids[] =
{
    { .name = "at24cxx", .driver_data = -1 },
    {},
};

/*!< device id for device-tree */
static const struct fwk_of_device_id sgtc_at24cxx_driver_dts[] =
{
	{ .compatible = "atmel,at24cxx", },
	{},
};

/*!< platform instance */
static struct fwk_i2c_driver sgtc_at24cxx_driver =
{
	.probe	= at24cxx_driver_probe,
	.remove	= at24cxx_driver_remove,

    .sptr_id_table = sgtc_at24cxx_driver_ids,
	
	.sgtc_driver =
	{
		.name 	= "at24cxx,eeprom",
		.id 	= -1,
		.sptr_of_match_table = sgtc_at24cxx_driver_dts,
	},
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   at24cxx_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init at24cxx_driver_init(void)
{
	return fwk_i2c_add_driver(&sgtc_at24cxx_driver);
}

/*!
 * @brief   at24cxx_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit at24cxx_driver_exit(void)
{
	fwk_i2c_del_driver(&sgtc_at24cxx_driver);
}

IMPORT_DRIVER_INIT(at24cxx_driver_init);
IMPORT_DRIVER_EXIT(at24cxx_driver_exit);

/*!< end of file */
