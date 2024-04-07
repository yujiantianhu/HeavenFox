/*
 * Template of Character Device : LED
 *
 * File Name:   led_template.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.11.26
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/hal_basic.h>
#include <platform/hal_cdev.h>
#include <platform/hal_chrdev.h>
#include <platform/hal_inode.h>
#include <platform/hal_fs.h>
#include <platform/of/hal_of.h>
#include <platform/hal_platdrv.h>
#include <platform/hal_uaccess.h>

/*!< The defines */
struct led_template_drv
{
	kstring_t *ptrName;
	kuint32_t major;
	kuint32_t minor;
	kuint32_t value[3];

	struct hal_cdev *sprt_cdev;
	kuaddr_t output_address;

	void *ptrData;
};

#define LED_TEMPLATE_DRIVER_NAME							"led-template"
#define LED_TEMPLATE_DRIVER_MAJOR							253

/*!< API function */
/*!
 * @brief   led_template_driver_open
 * @param   sprt_inode, sprt_file
 * @retval  errno
 * @note    none
 */
static ksint32_t led_template_driver_open(struct hal_inode *sprt_inode, struct hal_file *sprt_file)
{
	struct led_template_drv *sprt_privdata;

	sprt_privdata = sprt_inode->sprt_cdev->privData;
	sprt_file->private_data = sprt_privdata;

	return 0;
}

/*!
 * @brief   led_template_driver_close
 * @param   sprt_inode, sprt_file
 * @retval  errno
 * @note    none
 */
static ksint32_t led_template_driver_close(struct hal_inode *sprt_inode, struct hal_file *sprt_file)
{
	sprt_file->private_data = mrt_nullptr;

	return 0;
}

/*!
 * @brief   led_template_driver_write
 * @param   sprt_file, ptrBuffer, size
 * @retval  errno
 * @note    none
 */
static kssize_t led_template_driver_write(struct hal_file *sprt_file, const ksbuffer_t *ptrBuffer, kssize_t size)
{
	struct led_template_drv *sprt_privdata;
	kuint8_t value;

	sprt_privdata = (struct led_template_drv *)sprt_file->private_data;

	hal_copy_from_user(&value, ptrBuffer, 1);
	if (value)
	{
		mrt_setbitl(mrt_bit(sprt_privdata->value[1]), sprt_privdata->output_address);
	}
	else
	{
		mrt_clrbitl(mrt_bit(sprt_privdata->value[1]), sprt_privdata->output_address);
	}

	return 0;
}

/*!
 * @brief   led_template_driver_read
 * @param   sprt_file, ptrBuffer, size
 * @retval  errno
 * @note    none
 */
static kssize_t led_template_driver_read(struct hal_file *sprt_file, ksbuffer_t *ptrBuffer, kssize_t size)
{
	return 0;
}

/*!< led-template driver operation */
const struct hal_file_oprts sgrt_led_template_driver_oprts =
{
	.open	= led_template_driver_open,
	.close	= led_template_driver_close,
	.write	= led_template_driver_write,
	.read	= led_template_driver_read,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   led_template_driver_probe
 * @param   sprt_dev
 * @retval  errno
 * @note    none
 */
static ksint32_t led_template_driver_probe(struct hal_platdev *sprt_dev)
{
	struct led_template_drv *sprt_privdata;
	struct hal_device_node *sprt_node, *sprt_root, *sprt_muxc, *sprt_pins;
	struct hal_cdev *sprt_cdev;
	kuint32_t pin_handle, gpio_reg[2], pin_muxc[6];
	kuint32_t led_value[3], devnum, i;
	ksint32_t retval;

	sprt_root = hal_of_find_node_by_path("/");
	sprt_node = sprt_dev->sgrt_device.sprt_node;

	for (i = 0; i < ARRAY_SIZE(led_value); i++)
	{
		hal_of_property_read_u32_index(sprt_node, "led1-gpios", i, &led_value[i]);
	}

	/*!< -------------------------------------------------------------------- */
	/*!< pinctrl */
	sprt_muxc = hal_of_find_node_by_name(sprt_root, "iomuxc");
	hal_of_property_read_u32_array(sprt_muxc, "reg", gpio_reg, ARRAY_SIZE(gpio_reg));

	hal_of_property_read_u32(sprt_node, "pinctrl-0", &pin_handle);
	sprt_pins = hal_of_find_node_by_phandle(sprt_muxc, pin_handle);
	hal_of_property_read_u32_array(sprt_pins, "fsl,pins", pin_muxc, ARRAY_SIZE(pin_muxc));

	/*!< enbale gpio clock */
	mrt_setbitl(mrt_bit(26) | mrt_bit(27), 0x20C4000u + 0x6C);

	/*!< initial gpio pinmux */
	/*!< mux */
	mrt_writel(pin_muxc[3], gpio_reg[0] + pin_muxc[0]);
	/*!< pad */
	mrt_writel(pin_muxc[5], gpio_reg[0] + pin_muxc[1]);
	/*!< input */
	mrt_writel(pin_muxc[4], gpio_reg[0] + pin_muxc[2]);

	/*!< gpio */
	sprt_pins = hal_of_find_node_by_phandle(sprt_root, led_value[0]);
	hal_of_property_read_u32_array(sprt_pins, "reg", gpio_reg, ARRAY_SIZE(gpio_reg));

	/*!< set Gpio Pin Direction Output */
	mrt_setbitl(mrt_bit(led_value[1]), gpio_reg[0] + 0x4);
	/*!< set Gpio Default Level */
	mrt_clrbitl(mrt_bit(led_value[1]), gpio_reg[0]);

	/*!< -------------------------------------------------------------------- */
	devnum = MKE_DEV_NUM(LED_TEMPLATE_DRIVER_MAJOR, 0);
	retval = hal_register_chrdev(devnum, 1, "led-template");
	if (mrt_isErr(retval))
	{
		goto fail1;
	}

	sprt_cdev = hal_cdev_alloc(&sgrt_led_template_driver_oprts);
	if (!mrt_isValid(sprt_cdev))
	{
		goto fail2;
	}

	retval = hal_cdev_add(sprt_cdev, devnum, 1);
	if (mrt_isErr(retval))
	{
		goto fail3;
	}

	retval = hal_device_create(Ert_HAL_TYPE_CHRDEV, devnum, LED_TEMPLATE_DRIVER_NAME);
	if (mrt_isErr(retval))
	{
		goto fail4;
	}

	sprt_privdata = (struct led_template_drv *)kzalloc(sizeof(struct led_template_drv), GFP_KERNEL);
	if (!mrt_isValid(sprt_privdata))
	{
		goto fail5;
	}

	sprt_privdata->ptrName = LED_TEMPLATE_DRIVER_NAME;
	sprt_privdata->major = GET_DEV_MAJOR(devnum);
	sprt_privdata->minor = GET_DEV_MINOR(devnum);
	sprt_privdata->sprt_cdev = sprt_cdev;
	sprt_privdata->output_address = gpio_reg[0];
	memcpy(sprt_privdata->value, led_value, sizeof(sprt_privdata->value));

	sprt_cdev->privData = sprt_privdata;
	hal_platform_set_drvdata(sprt_dev, sprt_privdata);

	return mrt_retval(Ert_isWell);

fail5:
	hal_device_destroy(LED_TEMPLATE_DRIVER_NAME);
fail4:
	hal_cdev_del(sprt_cdev);
fail3:
	kfree(sprt_cdev);
fail2:
	hal_unregister_chrdev(devnum, 1);
fail1:
	return mrt_retval(Ert_isNotSuccess);
}

/*!
 * @brief   led_template_driver_remove
 * @param   sprt_dev
 * @retval  errno
 * @note    none
 */
static ksint32_t led_template_driver_remove(struct hal_platdev *sprt_dev)
{
	struct led_template_drv *sprt_privdata;
	kuint32_t devnum;

	sprt_privdata = (struct led_template_drv *)hal_platform_get_drvdata(sprt_dev);
	if (!mrt_isValid(sprt_privdata))
	{
		return mrt_retval(Ert_isNullPtr);
	}

	devnum = MKE_DEV_NUM(sprt_privdata->major, sprt_privdata->minor);

	hal_device_destroy(sprt_privdata->ptrName);
	hal_cdev_del(sprt_privdata->sprt_cdev);
	kfree(sprt_privdata->sprt_cdev);
	hal_unregister_chrdev(devnum, 1);

	kfree(sprt_privdata);
	hal_platform_set_drvdata(sprt_dev, mrt_nullptr);

	return mrt_retval(Ert_isWell);
}

/*!< device id for device-tree */
static struct hal_of_device_id sgrt_led_template_driver_id[] =
{
	{ .compatible = "fsl, topeet, ledgpio", },
	{},
};

/*!< platform instance */
static struct hal_platdrv sgrt_led_template_platdriver =
{
	.probe	= led_template_driver_probe,
	.remove	= led_template_driver_remove,
	
	.sgrt_driver =
	{
		.name 	= LED_TEMPLATE_DRIVER_NAME,
		.id 	= -1,
		.sprt_of_match_table = sgrt_led_template_driver_id,
	},
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   led_template_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
ksint32_t __hal_init led_template_driver_init(void)
{
	return hal_register_platdriver(&sgrt_led_template_platdriver);
}

/*!
 * @brief   led_template_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __hal_exit led_template_driver_exit(void)
{
	hal_unregister_platdriver(&sgrt_led_template_platdriver);
}

IMPORT_DRIVER_INIT(led_template_driver_init);
IMPORT_DRIVER_EXIT(led_template_driver_exit);

/*!< end of file */
