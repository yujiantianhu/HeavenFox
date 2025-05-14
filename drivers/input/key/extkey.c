/*
 * Template of Character Device : LED
 *
 * File Name:   extkey.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.13
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_cdev.h>
#include <platform/fwk_chrdev.h>
#include <platform/fwk_inode.h>
#include <platform/fwk_fs.h>
#include <platform/fwk_fcntl.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/fwk_platdrv.h>
#include <platform/fwk_uaccess.h>
#include <platform/gpio/fwk_gpiodesc.h>
#include <kernel/wait.h>

#include <imx6/imx6ull_periph.h>

/*!< The defines */
struct extkey_drv_data
{
	kchar_t *ptrName;
	kuint32_t major;
	kuint32_t minor;
	struct fwk_gpio_desc *sptr_gdesc;

	struct fwk_cdev *sptr_cdev;
	struct fwk_device *sptr_idev;
	kint32_t irq;

	kbool_t wake;
	struct wait_queue_head sgtc_wqh;

	void *ptrData;
};

#define KEY_DRIVER_NAME								"extkey"
#define KEY_DRIVER_MAJOR							(NR_KEY_MAJOR)

/*!< API function */
static irq_return_t extkey_driver_isr(void *ptrDev)
{
	struct extkey_drv_data *sptr_data;

	sptr_data = (struct extkey_drv_data *)ptrDev;

	sptr_data->wake = true;
	wake_up(&sptr_data->sgtc_wqh);

	return 0;
}

/*!
 * @brief   extkey_driver_open
 * @param   sptr_inode, sptr_file
 * @retval  errno
 * @note    none
 */
static kint32_t extkey_driver_open(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
	struct extkey_drv_data *sptr_data;

	sptr_data = sptr_inode->sptr_cdev->privData;
	sptr_file->private_data = sptr_data;

	fwk_enable_irq(sptr_data->irq);

	return 0;
}

/*!
 * @brief   extkey_driver_close
 * @param   sptr_inode, sptr_file
 * @retval  errno
 * @note    none
 */
static kint32_t extkey_driver_close(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
	struct extkey_drv_data *sptr_data;

	sptr_data = sptr_file->private_data;
	fwk_disable_irq(sptr_data->irq);
	
	sptr_file->private_data = mr_nullptr;

	return 0;
}

/*!
 * @brief   extkey_driver_write
 * @param   sptr_file, ptrBuffer, size
 * @retval  errno
 * @note    none
 */
static kssize_t extkey_driver_write(struct fwk_file *sptr_file, const kbuffer_t *ptrBuffer, kssize_t size)
{
	return 0;
}

/*!
 * @brief   extkey_driver_read
 * @param   sptr_file, ptrBuffer, size
 * @retval  errno
 * @note    none
 */
static kssize_t extkey_driver_read(struct fwk_file *sptr_file, kbuffer_t *ptrBuffer, kssize_t size)
{
	struct extkey_drv_data *sptr_data;
	kuint8_t value;

	sptr_data = (struct extkey_drv_data *)sptr_file->private_data;
	
	if (!(sptr_file->mode & O_NONBLOCK))
		wait_event(&sptr_data->sgtc_wqh, sptr_data->wake);
	else
	{
		if (!sptr_data->wake)
			return -ER_NREADY;
	}
	
	value = fwk_gpio_get_value(sptr_data->sptr_gdesc);
	fwk_copy_to_user(ptrBuffer, &value, 1);

	sptr_data->wake = false;
	
	return 0;
}

/*!< extkey-template driver operation */
const struct fwk_file_oprts sgtc_extkey_driver_oprts =
{
	.open	= extkey_driver_open,
	.close	= extkey_driver_close,
	.write	= extkey_driver_write,
	.read	= extkey_driver_read,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   extkey_driver_probe
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t extkey_driver_probe(struct fwk_platdev *sptr_pdev)
{
	struct extkey_drv_data *sptr_data;
	struct fwk_gpio_desc *sptr_gdesc;
	struct fwk_cdev *sptr_cdev;
	struct fwk_device *sptr_idev;
	kuint32_t devnum;
	kint32_t irq;
	kint32_t retval;

	sptr_gdesc = fwk_gpio_desc_get(&sptr_pdev->sgtc_dev, "key0", 0);
	if (!isValid(sptr_gdesc))
		return -ER_NODEV;

	fwk_gpio_set_direction_input(sptr_gdesc);

	irq = fwk_gpio_desc_to_irq(sptr_gdesc);
	if (irq < 0)
		goto fail1;

	devnum = MKE_DEV_NUM(KEY_DRIVER_MAJOR, 0);
	retval = fwk_register_chrdev(devnum, 1, KEY_DRIVER_NAME);
	if (retval < 0)
		goto fail1;

	sptr_cdev = fwk_cdev_alloc(&sgtc_extkey_driver_oprts);
	if (!isValid(sptr_cdev))
		goto fail2;

	retval = fwk_cdev_add(sptr_cdev, devnum, 1);
	if (retval < 0)
		goto fail3;

	sptr_idev = fwk_device_create(NR_TYPE_CHRDEV, devnum, "%s%d", "input/event", 0);
	if (!isValid(sptr_idev))
		goto fail4;
	
	sptr_data = (struct extkey_drv_data *)kzalloc(sizeof(struct extkey_drv_data), GFP_KERNEL);
	if (!isValid(sptr_data))
		goto fail5;

	sptr_data->ptrName = KEY_DRIVER_NAME;
	sptr_data->major = GET_DEV_MAJOR(devnum);
	sptr_data->minor = GET_DEV_MINOR(devnum);
	sptr_data->sptr_cdev = sptr_cdev;
	sptr_data->sptr_idev = sptr_idev;
	sptr_data->sptr_gdesc = sptr_gdesc;
	sptr_data->irq = irq;
	sptr_cdev->privData = sptr_data;
	init_waitqueue_head(&sptr_data->sgtc_wqh);

	if (fwk_request_irq(irq, extkey_driver_isr, IRQ_TYPE_EDGE_RISING | IRQ_TYPE_EDGE_FALLING, KEY_DRIVER_NAME, sptr_data))
		goto fail6;

	fwk_platform_set_drvdata(sptr_pdev, sptr_data);
	fwk_disable_irq(irq);

	return ER_NORMAL;

fail6:
	kfree(sptr_data);
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
 * @brief   extkey_driver_remove
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t extkey_driver_remove(struct fwk_platdev *sptr_pdev)
{
	struct extkey_drv_data *sptr_data;
	kuint32_t devnum;

	sptr_data = (struct extkey_drv_data *)fwk_platform_get_drvdata(sptr_pdev);
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
static const struct fwk_of_device_id sgtc_extkey_driver_id[] =
{
	{ .compatible = "fsl,extern-key", },
	{},
};

/*!< platform instance */
static struct fwk_platdrv sgtc_extkey_platdriver =
{
	.probe	= extkey_driver_probe,
	.remove	= extkey_driver_remove,
	
	.sgtc_driver =
	{
		.name 	= KEY_DRIVER_NAME,
		.id 	= -1,
		.sptr_of_match_table = sgtc_extkey_driver_id,
	},
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   extkey_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init extkey_driver_init(void)
{
	return fwk_register_platdriver(&sgtc_extkey_platdriver);
}

/*!
 * @brief   extkey_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit extkey_driver_exit(void)
{
	fwk_unregister_platdriver(&sgtc_extkey_platdriver);
}

IMPORT_DRIVER_INIT(extkey_driver_init);
IMPORT_DRIVER_EXIT(extkey_driver_exit);

/*!< end of file */
