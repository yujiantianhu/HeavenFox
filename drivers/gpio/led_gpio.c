/*
 * Template of Character Device : LED
 *
 * File Name:   led_gpio.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.13
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/base/fwk_cdev.h>
#include <platform/base/fwk_chrdev.h>
#include <platform/base/fwk_inode.h>
#include <platform/base/fwk_fs.h>
#include <platform/of/fwk_of.h>
#include <platform/base/fwk_platdrv.h>
#include <platform/base/fwk_uaccess.h>
#include <platform/gpio/fwk_gpiodesc.h>

/*!< The defines */
struct led_drv_data
{
    kchar_t *ptrName;
    kuint32_t major;
    kuint32_t minor;
    struct fwk_gpio_desc *sptr_gdesc;

    struct fwk_cdev *sptr_cdev;
    struct fwk_device *sptr_idev;

    void *ptrData;
};

#define LED_DRIVER_NAME								"ledgpio"
#define LED_DRIVER_MAJOR							(NR_LED_MAJOR)

/*!< API function */
/*!
 * @brief   led_driver_open
 * @param   sptr_inode, sptr_file
 * @retval  errno
 * @note    none
 */
static kint32_t led_driver_open(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    struct led_drv_data *sptr_data;

    sptr_data = sptr_inode->sptr_cdev->privData;
    sptr_file->private_data = sptr_data;

    return 0;
}

/*!
 * @brief   led_driver_close
 * @param   sptr_inode, sptr_file
 * @retval  errno
 * @note    none
 */
static kint32_t led_driver_close(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    sptr_file->private_data = mr_nullptr;

    return 0;
}

/*!
 * @brief   led_driver_write
 * @param   sptr_file, ptrBuffer, size
 * @retval  errno
 * @note    none
 */
static kssize_t led_driver_write(struct fwk_file *sptr_file, const kbuffer_t *ptrBuffer, kssize_t size)
{
    struct led_drv_data *sptr_data;
    kuint8_t value;

    sptr_data = (struct led_drv_data *)sptr_file->private_data;

    fwk_copy_from_user(&value, ptrBuffer, 1);
    fwk_gpio_set_value(sptr_data->sptr_gdesc, !!value);

    return 0;
}

/*!
 * @brief   led_driver_read
 * @param   sptr_file, ptrBuffer, size
 * @retval  errno
 * @note    none
 */
static kssize_t led_driver_read(struct fwk_file *sptr_file, kbuffer_t *ptrBuffer, kssize_t size)
{
    return 0;
}

/*!< led-template driver operation */
const struct fwk_file_oprts sgtc_led_driver_oprts =
{
    .open	= led_driver_open,
    .close	= led_driver_close,
    .write	= led_driver_write,
    .read	= led_driver_read,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   led_driver_probe
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t led_driver_probe(struct fwk_platdev *sptr_pdev)
{
    struct led_drv_data *sptr_data;
    struct fwk_gpio_desc *sptr_gdesc;
    struct fwk_cdev *sptr_cdev;
    struct fwk_device *sptr_idev;
    kuint32_t devnum;
    kint32_t retval;

    sptr_gdesc = fwk_gpio_desc_get(&sptr_pdev->sgtc_dev, "led1", 0);
    if (!isValid(sptr_gdesc))
        return -ER_NODEV;

    fwk_gpio_set_direction_output(sptr_gdesc, 0);
    fwk_gpio_set_value(sptr_gdesc, 0);

    devnum = MKE_DEV_NUM(LED_DRIVER_MAJOR, 0);
    retval = fwk_register_chrdev(devnum, 1, LED_DRIVER_NAME);
    if (retval < 0)
        goto fail1;

    sptr_cdev = fwk_cdev_alloc(&sgtc_led_driver_oprts);
    if (!isValid(sptr_cdev))
        goto fail2;

    retval = fwk_cdev_add(sptr_cdev, devnum, 1);
    if (retval < 0)
        goto fail3;

    sptr_idev = fwk_device_create(NR_TYPE_CHRDEV, devnum, LED_DRIVER_NAME);
    if (!isValid(sptr_idev))
        goto fail4;
    
    sptr_data = (struct led_drv_data *)kzalloc(sizeof(struct led_drv_data), GFP_KERNEL);
    if (!isValid(sptr_data))
        goto fail5;

    sptr_data->ptrName = LED_DRIVER_NAME;
    sptr_data->major = GET_DEV_MAJOR(devnum);
    sptr_data->minor = GET_DEV_MINOR(devnum);
    sptr_data->sptr_cdev = sptr_cdev;
    sptr_data->sptr_gdesc = sptr_gdesc;
    sptr_data->sptr_idev = sptr_idev;

    sptr_cdev->privData = sptr_data;
    fwk_platform_set_drvdata(sptr_pdev, sptr_data);

    return ER_NORMAL;

fail5:
    fwk_device_destroy(sptr_idev);
fail4:
    fwk_cdev_del(sptr_cdev);
fail3:
    kfree(sptr_cdev);
fail2:
    fwk_unregister_chrdev(devnum, 1);
fail1:
    fwk_gpio_desc_put(sptr_gdesc);

    return -ER_FAILD;
}

/*!
 * @brief   led_driver_remove
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t led_driver_remove(struct fwk_platdev *sptr_pdev)
{
    struct led_drv_data *sptr_data;
    kuint32_t devnum;

    sptr_data = (struct led_drv_data *)fwk_platform_get_drvdata(sptr_pdev);
    if (!isValid(sptr_data))
        return -ER_NULLPTR;

    devnum = MKE_DEV_NUM(sptr_data->major, sptr_data->minor);

    fwk_device_destroy(sptr_data->sptr_idev);
    fwk_cdev_del(sptr_data->sptr_cdev);
    kfree(sptr_data->sptr_cdev);
    fwk_unregister_chrdev(devnum, 1);

    fwk_gpio_desc_put(sptr_data->sptr_gdesc);

    kfree(sptr_data);
    fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);

    return ER_NORMAL;
}

/*!< device id for device-tree */
static const struct fwk_of_device_id sgtc_led_driver_id[] =
{
    { .compatible = "fsl,led-gpio", },
    {},
};

/*!< platform instance */
static struct fwk_platdrv sgtc_led_platdriver =
{
    .probe	= led_driver_probe,
    .remove	= led_driver_remove,
    
    .sgtc_driver =
    {
        .name 	= LED_DRIVER_NAME,
        .id 	= -1,
        .sptr_of_match_table = sgtc_led_driver_id,
    },
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   led_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init led_driver_init(void)
{
    return fwk_register_platdriver(&sgtc_led_platdriver);
}

/*!
 * @brief   led_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit led_driver_exit(void)
{
    fwk_unregister_platdriver(&sgtc_led_platdriver);
}

IMPORT_DRIVER_INIT(led_driver_init);
IMPORT_DRIVER_EXIT(led_driver_exit);

/*!< end of file */
