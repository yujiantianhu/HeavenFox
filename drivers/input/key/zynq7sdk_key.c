/*
 * Template of Character Device (Use Xilinx SDK): LED
 *
 * File Name:   xsdk_key.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.10.27
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_cdev.h>
#include <platform/fwk_chrdev.h>
#include <platform/fwk_inode.h>
#include <platform/fwk_fs.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/fwk_platdrv.h>
#include <platform/fwk_uaccess.h>
#include <platform/gpio/fwk_gpiodesc.h>

#include <zynq7/zynq7_periph.h>
#include <zynq7/xparameters.h>

/*!< The defines */
struct xsdk_key_drv_data
{
    kchar_t *ptrName;
    kuint32_t major;
    kuint32_t minor;
    XGpioPs sgtc_gpio;
    kuint32_t pin;
    kbool_t isActiveL;

    struct fwk_cdev *sptr_cdev;
    struct fwk_device *sptr_idev;

    void *ptrData;
};

#define XSDK_KEY_DRIVER_NAME							"key0"
#define XSDK_KEY_DRIVER_MAJOR							(NR_KEY_MAJOR)

/*!< The functions */
static void xsdk_key_get_value(struct xsdk_key_drv_data *sptr_data, kbool_t *value);

/*!< API function */
/*!
 * @brief   xsdk_key_driver_open
 * @param   sptr_inode, sptr_file
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_key_driver_open(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    struct xsdk_key_drv_data *sptr_data;

    sptr_data = sptr_inode->sptr_cdev->privData;
    sptr_file->private_data = sptr_data;

    return 0;
}

/*!
 * @brief   xsdk_key_driver_close
 * @param   sptr_inode, sptr_file
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_key_driver_close(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    sptr_file->private_data = mr_nullptr;

    return 0;
}

/*!
 * @brief   xsdk_key_driver_write
 * @param   sptr_file, ptrBuffer, size
 * @retval  errno
 * @note    none
 */
static kssize_t xsdk_key_driver_write(struct fwk_file *sptr_file, const kbuffer_t *ptrBuffer, kssize_t size)
{
    return 0;
}

/*!
 * @brief   xsdk_key_driver_read
 * @param   sptr_file, ptrBuffer, size
 * @retval  errno
 * @note    none
 */
static kssize_t xsdk_key_driver_read(struct fwk_file *sptr_file, kbuffer_t *ptrBuffer, kssize_t size)
{
    struct xsdk_key_drv_data *sptr_data;
    kbool_t value;

    sptr_data = (struct xsdk_key_drv_data *)sptr_file->private_data;

    xsdk_key_get_value(sptr_data, &value);
    fwk_copy_to_user(ptrBuffer, &value, 1);

    return 1;
}

/*!< led-template driver operation */
const struct fwk_file_oprts sgtc_xsdk_key_driver_oprts =
{
    .open	= xsdk_key_driver_open,
    .close	= xsdk_key_driver_close,
    .write	= xsdk_key_driver_write,
    .read	= xsdk_key_driver_read,
};

/*!
 * @brief   open or close led
 * @param   sptr_data, value (true: open led; false: close led)
 * @retval  errno
 * @note    none
 */
static void xsdk_key_get_value(struct xsdk_key_drv_data *sptr_data, kbool_t *value)
{
    kint32_t retval;

    retval = XGpioPs_ReadPin(&sptr_data->sgtc_gpio, XGPIOPS_BANK_PIN(0, sptr_data->pin));
    if (retval < 0)
        *value = false;
    else
        *value = sptr_data->isActiveL ? (!retval) : retval;
}

/*!
 * @brief   configure gpio pin property
 * @param   sptr_pdev, sptr_data
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_key_configure(struct fwk_platdev *sptr_pdev, struct xsdk_key_drv_data *sptr_data)
{
    struct fwk_device_node *sptr_node, *sptr_parent;
    struct fwk_of_phandle_args sgtc_args;
    XGpioPs *sptr_gpio;
    XGpioPs_Config sgtc_cfg;
    kint32_t retval;

    sptr_gpio = &sptr_data->sgtc_gpio;
    sptr_node = sptr_pdev->sgtc_dev.sptr_node;
    if (!isValid(sptr_node))
        return PTR_ERR(sptr_node);

    retval = fwk_of_parse_phandle_with_args(sptr_node, 
                                    "key-gpios", "#gpio-cells", 2, 0, &sgtc_args); 
    if (retval || (!isValid(sgtc_args.sptr_node)))
        return retval;

    /*!< get gpio-controller*/
    sptr_parent = sgtc_args.sptr_node;

    sgtc_cfg.DeviceId = 0;
    sgtc_cfg.BaseAddr = (kuint32_t)fwk_of_iomap(sptr_parent, 0);
    sptr_data->pin = sgtc_args.args[0];
    sptr_data->isActiveL = !!(sgtc_args.args[1] & GPIO_ACTIVE_LOW);

    /*!< config */
    XGpioPs_CfgInitialize(sptr_gpio, &sgtc_cfg, sgtc_cfg.BaseAddr);
    XGpioPs_SetDirectionPin(sptr_gpio, XGPIOPS_BANK_PIN(0, sptr_data->pin), XGPIOPS_PIN_DIR_INPUT);

    return 0;
}

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   xsdk_key_driver_probe
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_key_driver_probe(struct fwk_platdev *sptr_pdev)
{
    struct xsdk_key_drv_data *sptr_data;
    struct fwk_cdev *sptr_cdev;
    struct fwk_device *sptr_idev;
    kuint32_t devnum;
    kint32_t retval;

    devnum = MKE_DEV_NUM(XSDK_KEY_DRIVER_MAJOR, 0);
    retval = fwk_register_chrdev(devnum, 1, XSDK_KEY_DRIVER_NAME);
    if (retval < 0)
        return -ER_FAILD;

    sptr_cdev = fwk_cdev_alloc(&sgtc_xsdk_key_driver_oprts);
    if (!isValid(sptr_cdev))
        goto fail1;

    retval = fwk_cdev_add(sptr_cdev, devnum, 1);
    if (retval < 0)
        goto fail2;

    sptr_idev = fwk_device_create(NR_TYPE_CHRDEV, devnum, XSDK_KEY_DRIVER_NAME);
    if (!isValid(sptr_idev))
        goto fail3;
    
    sptr_data = (struct xsdk_key_drv_data *)kzalloc(sizeof(struct xsdk_key_drv_data), GFP_KERNEL);
    if (!isValid(sptr_data))
        goto fail4;

    if (xsdk_key_configure(sptr_pdev, sptr_data))
        goto fail5;

    sptr_data->ptrName = XSDK_KEY_DRIVER_NAME;
    sptr_data->major = GET_DEV_MAJOR(devnum);
    sptr_data->minor = GET_DEV_MINOR(devnum);
    sptr_data->sptr_cdev = sptr_cdev;
    sptr_data->sptr_idev = sptr_idev;
    sptr_idev->sptr_parent = &sptr_pdev->sgtc_dev;

    sptr_cdev->privData = sptr_data;
    fwk_platform_set_drvdata(sptr_pdev, sptr_data);

    print_info("register a new chardevice (KEY)\r\n");

    return ER_NORMAL;

fail5:
    kfree(sptr_data);
fail4:
    fwk_device_destroy(sptr_idev);
fail3:
    fwk_cdev_del(sptr_cdev);
fail2:
    kfree(sptr_cdev);
fail1:
    fwk_unregister_chrdev(devnum, 1);

    return -ER_FAILD;
}

/*!
 * @brief   xsdk_key_driver_remove
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_key_driver_remove(struct fwk_platdev *sptr_pdev)
{
    struct xsdk_key_drv_data *sptr_data;
    kuint32_t devnum;

    sptr_data = (struct xsdk_key_drv_data *)fwk_platform_get_drvdata(sptr_pdev);
    if (!isValid(sptr_data))
        return -ER_NULLPTR;

    devnum = MKE_DEV_NUM(sptr_data->major, sptr_data->minor);

    fwk_device_destroy(sptr_data->sptr_idev);
    fwk_cdev_del(sptr_data->sptr_cdev);
    kfree(sptr_data->sptr_cdev);
    fwk_unregister_chrdev(devnum, 1);

    kfree(sptr_data);
    fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);

    return ER_NORMAL;
}

/*!< device id for device-tree */
static const struct fwk_of_device_id sgtc_xsdk_key_driver_id[] =
{
    { .compatible = "xlnx,z7-lite,extkey", },
    {},
};

/*!< platform instance */
static struct fwk_platdrv sgtc_xsdk_key_platdriver =
{
    .probe	= xsdk_key_driver_probe,
    .remove	= xsdk_key_driver_remove,
    
    .sgtc_driver =
    {
        .name 	= XSDK_KEY_DRIVER_NAME,
        .id 	= -1,
        .sptr_of_match_table = sgtc_xsdk_key_driver_id,
    },
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   xsdk_key_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init xsdk_key_driver_init(void)
{
    return fwk_register_platdriver(&sgtc_xsdk_key_platdriver);
}

/*!
 * @brief   xsdk_key_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit xsdk_key_driver_exit(void)
{
    fwk_unregister_platdriver(&sgtc_xsdk_key_platdriver);
}

IMPORT_DRIVER_INIT(xsdk_key_driver_init);
IMPORT_DRIVER_EXIT(xsdk_key_driver_exit);

/*!< end of file */
