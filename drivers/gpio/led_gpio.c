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
#include <platform/fwk_basic.h>
#include <platform/fwk_cdev.h>
#include <platform/fwk_chrdev.h>
#include <platform/fwk_inode.h>
#include <platform/fwk_fs.h>
#include <platform/of/fwk_of.h>
#include <platform/fwk_platdrv.h>
#include <platform/fwk_uaccess.h>
#include <platform/gpio/fwk_gpiodesc.h>

/*!< The defines */
struct led_drv_data
{
	kchar_t *ptrName;
	kuint32_t major;
	kuint32_t minor;
	struct fwk_gpio_desc *sprt_gdesc;

	struct fwk_cdev *sprt_cdev;
	struct fwk_device *sprt_idev;

	void *ptrData;
};

#define LED_DRIVER_NAME								"ledgpio"
#define LED_DRIVER_MAJOR							(NR_LED_MAJOR)

/*!< API function */
/*!
 * @brief   led_driver_open
 * @param   sprt_inode, sprt_file
 * @retval  errno
 * @note    none
 */
static kint32_t led_driver_open(struct fwk_inode *sprt_inode, struct fwk_file *sprt_file)
{
	struct led_drv_data *sprt_data;

	sprt_data = sprt_inode->sprt_cdev->privData;
	sprt_file->private_data = sprt_data;

	return 0;
}

/*!
 * @brief   led_driver_close
 * @param   sprt_inode, sprt_file
 * @retval  errno
 * @note    none
 */
static kint32_t led_driver_close(struct fwk_inode *sprt_inode, struct fwk_file *sprt_file)
{
	sprt_file->private_data = mrt_nullptr;

	return 0;
}

/*!
 * @brief   led_driver_write
 * @param   sprt_file, ptrBuffer, size
 * @retval  errno
 * @note    none
 */
static kssize_t led_driver_write(struct fwk_file *sprt_file, const kbuffer_t *ptrBuffer, kssize_t size)
{
	struct led_drv_data *sprt_data;
	kuint8_t value;

	sprt_data = (struct led_drv_data *)sprt_file->private_data;

	fwk_copy_from_user(&value, ptrBuffer, 1);
	fwk_gpio_set_value(sprt_data->sprt_gdesc, !!value);

	return 0;
}

/*!
 * @brief   led_driver_read
 * @param   sprt_file, ptrBuffer, size
 * @retval  errno
 * @note    none
 */
static kssize_t led_driver_read(struct fwk_file *sprt_file, kbuffer_t *ptrBuffer, kssize_t size)
{
	return 0;
}

/*!< led-template driver operation */
const struct fwk_file_oprts sgrt_led_driver_oprts =
{
	.open	= led_driver_open,
	.close	= led_driver_close,
	.write	= led_driver_write,
	.read	= led_driver_read,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   led_driver_probe
 * @param   sprt_dev
 * @retval  errno
 * @note    none
 */
static kint32_t led_driver_probe(struct fwk_platdev *sprt_pdev)
{
	struct led_drv_data *sprt_data;
	struct fwk_gpio_desc *sprt_gdesc;
	struct fwk_cdev *sprt_cdev;
	struct fwk_device *sprt_idev;
	kuint32_t devnum;
	kint32_t retval;

	sprt_gdesc = fwk_gpio_desc_get(&sprt_pdev->sgrt_dev, "led1", 0);
	if (!isValid(sprt_gdesc))
		return -ER_NODEV;

	fwk_gpio_set_direction_output(sprt_gdesc, 0);
	fwk_gpio_set_value(sprt_gdesc, 0);

	devnum = MKE_DEV_NUM(LED_DRIVER_MAJOR, 0);
	retval = fwk_register_chrdev(devnum, 1, LED_DRIVER_NAME);
	if (retval < 0)
		goto fail1;

	sprt_cdev = fwk_cdev_alloc(&sgrt_led_driver_oprts);
	if (!isValid(sprt_cdev))
		goto fail2;

	retval = fwk_cdev_add(sprt_cdev, devnum, 1);
	if (retval < 0)
		goto fail3;

	sprt_idev = fwk_device_create(NR_TYPE_CHRDEV, devnum, LED_DRIVER_NAME);
	if (!isValid(sprt_idev))
		goto fail4;
	
	sprt_data = (struct led_drv_data *)kzalloc(sizeof(struct led_drv_data), GFP_KERNEL);
	if (!isValid(sprt_data))
		goto fail5;

	sprt_data->ptrName = LED_DRIVER_NAME;
	sprt_data->major = GET_DEV_MAJOR(devnum);
	sprt_data->minor = GET_DEV_MINOR(devnum);
	sprt_data->sprt_cdev = sprt_cdev;
	sprt_data->sprt_gdesc = sprt_gdesc;
	sprt_data->sprt_idev = sprt_idev;

	sprt_cdev->privData = sprt_data;
	fwk_platform_set_drvdata(sprt_pdev, sprt_data);

	return ER_NORMAL;

fail5:
	fwk_device_destroy(sprt_idev);
fail4:
	fwk_cdev_del(sprt_cdev);
fail3:
	kfree(sprt_cdev);
fail2:
	fwk_unregister_chrdev(devnum, 1);
fail1:
	fwk_gpio_desc_put(sprt_gdesc);

	return -ER_FAILD;
}

/*!
 * @brief   led_driver_remove
 * @param   sprt_dev
 * @retval  errno
 * @note    none
 */
static kint32_t led_driver_remove(struct fwk_platdev *sprt_pdev)
{
	struct led_drv_data *sprt_data;
	kuint32_t devnum;

	sprt_data = (struct led_drv_data *)fwk_platform_get_drvdata(sprt_pdev);
	if (!isValid(sprt_data))
		return -ER_NULLPTR;

	devnum = MKE_DEV_NUM(sprt_data->major, sprt_data->minor);

	fwk_device_destroy(sprt_data->sprt_idev);
	fwk_cdev_del(sprt_data->sprt_cdev);
	kfree(sprt_data->sprt_cdev);
	fwk_unregister_chrdev(devnum, 1);

	fwk_gpio_desc_put(sprt_data->sprt_gdesc);

	kfree(sprt_data);
	fwk_platform_set_drvdata(sprt_pdev, mrt_nullptr);

	return ER_NORMAL;
}

/*!< device id for device-tree */
static const struct fwk_of_device_id sgrt_led_driver_id[] =
{
	{ .compatible = "fsl,led-gpio", },
	{},
};

/*!< platform instance */
static struct fwk_platdrv sgrt_led_platdriver =
{
	.probe	= led_driver_probe,
	.remove	= led_driver_remove,
	
	.sgrt_driver =
	{
		.name 	= LED_DRIVER_NAME,
		.id 	= -1,
		.sprt_of_match_table = sgrt_led_driver_id,
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
	return fwk_register_platdriver(&sgrt_led_platdriver);
}

/*!
 * @brief   led_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit led_driver_exit(void)
{
	fwk_unregister_platdriver(&sgrt_led_platdriver);
}

IMPORT_DRIVER_INIT(led_driver_init);
IMPORT_DRIVER_EXIT(led_driver_exit);

/*!< end of file */
