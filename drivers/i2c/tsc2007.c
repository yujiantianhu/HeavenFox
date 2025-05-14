/*
 * Template of Touch Screen Device : tsc2007
 *
 * File Name:   tsc2007.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.12
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/fwk_platdrv.h>
#include <platform/fwk_uaccess.h>
#include <platform/gpio/fwk_gpiodesc.h>
#include <platform/i2c/fwk_i2c_dev.h>
#include <platform/i2c/fwk_i2c_core.h>
#include <platform/i2c/fwk_i2c_algo.h>
#include <platform/fwk_cdev.h>
#include <platform/fwk_chrdev.h>
#include <platform/fwk_inode.h>
#include <platform/fwk_fs.h>
#include <platform/fwk_fcntl.h>
#include <platform/input/fwk_input.h>
#include <kernel/wait.h>
#include <kernel/workqueue.h>

/*!< The defines */
#define TSC2007_DRVIVER_MAJOR                       (220)

/*!< tsc2007, index:
 * bit0:        X
 * bit1:        M
 * bit2 ~ bit3: PD0 ~ PD1
 * bit4 ~ bit7: C0 ~ C3
 */
/*!< C3 ~ C0 */
#define TSC2007_CTRL_MEASURE_TEMP0		            (0x00 << 4)         /*!< 0000 */
#define TSC2007_CTRL_MEASURE_AUX		            (0x02 << 4)         /*!< 0010 */
#define TSC2007_CTRL_MEASURE_TEMP1		            (0x04 << 4)         /*!< 0100 */
#define TSC2007_CTRL_ACTIVATE_XN		            (0x08 << 4)         /*!< 1000 */
#define TSC2007_CTRL_ACTIVATE_YN		            (0x09 << 4)         /*!< 1001 */
#define TSC2007_CTRL_ACTIVATE_YP_XN		            (0x0A << 4)         /*!< 1010 */
#define TSC2007_CTRL_SETUP			                (0x0B << 4)         /*!< 1011 */
#define TSC2007_CTRL_MEASURE_X		                (0x0C << 4)         /*!< 1100 */
#define TSC2007_CTRL_MEASURE_Y		                (0x0D << 4)         /*!< 1101 */
#define TSC2007_CTRL_MEASURE_Z1		                (0x0E << 4)         /*!< 1110 */
#define TSC2007_CTRL_MEASURE_Z2		                (0x0F << 4)         /*!< 1111 */

/*!< PD1 ~ PD0 */
#define TSC2007_PWR_POWER_OFF_IRQ_EN	            (0x00 << 2)         /*!< 00 */
#define TSC2007_PWR_ADC_ON_IRQ_DIS0		            (0x01 << 2)         /*!< 01 */
#define TSC2007_PWR_ADC_OFF_IRQ_EN		            (0x02 << 2)         /*!< 10 */
#define TSC2007_PWR_ADC_ON_IRQ_DIS1		            (0x03 << 2)         /*!< 11 */

/*!< M : MODE */
#define TSC2007_MODE_12BIT			                (0x00 << 1)         /*!< 0 */
#define TSC2007_MODE_8BIT			                (0x01 << 1)         /*!< 1 */

/*!< 实际定义 */
#define	TSC_MAX_12BIT			                    (4095 + 1)          /*!< max: 4096 - 1 = 4095 */

#define TSC_ADC_ON_12BIT    \
            (TSC2007_MODE_12BIT | TSC2007_PWR_ADC_ON_IRQ_DIS0)          /*!< 12bit ADC, Disable INT IRQ */

#define TSC_INDEX_READ_X	\
            (TSC_ADC_ON_12BIT | TSC2007_CTRL_MEASURE_X)                 /*!< 12bit ADC, Disable IRQ, Reg X */

#define TSC_INDEX_READ_Y	\
            (TSC_ADC_ON_12BIT | TSC2007_CTRL_MEASURE_Y)                 /*!< 12bit ADC, Disable IRQ, Reg Y */

#define TSC_INDEX_READ_Z1	\
            (TSC_ADC_ON_12BIT | TSC2007_CTRL_MEASURE_Z1)                /*!< 12bit ADC, Disable IRQ, Reg Z1 */

#define TSC_INDEX_READ_Z2	\
            (TSC_ADC_ON_12BIT | TSC2007_CTRL_MEASURE_Z2)                /*!< 12bit ADC, Disable IRQ, Reg Z2 */

#define TSC_INDEX_PWRDOWN	\
            (TSC2007_MODE_12BIT | TSC2007_PWR_ADC_OFF_IRQ_EN)           /*!< 12bit ADC, PWR DOWN, Enable IRQ */

typedef struct tsc2007_data
{
    kuint16_t x;
    kuint16_t y;
    kuint16_t z1;
    kuint16_t z2;
    kuint32_t pressure;

    kuint16_t x_max;
    kuint16_t y_max;

    struct tsc2007_drv_info *sptr_info;

} tsc2007_data_t;

typedef struct tsc2007_drv_info
{
    kchar_t *name;
    struct fwk_i2c_client *sptr_client;

    struct fwk_gpio_desc *sptr_gdesc;
    kint32_t irq;

    kuint32_t plate_ohms;
    struct tsc2007_data sgtc_data;

    kbool_t is_touch;
    kbool_t is_can_read;

    kint32_t devnum;
    struct fwk_cdev *sptr_cdev;
    struct fwk_device *sptr_idev;

    struct workqueue sgtc_work;
    struct wait_queue_head sgtc_wqh;

} tsc2007_drv_info_t;

/*!< API function */
/*!
 * @brief   tsc2007_read_value
 * @param   none
 * @retval  none
 * @note    read tsc2007 with i2c
 */
static kuint16_t tsc2007_read_value(struct tsc2007_drv_info *sptr_info, kuint8_t reg)
{
    struct fwk_i2c_msg sgtc_msgs[2] = {};
    kuint16_t value = 0;
    kint32_t retval;

    sgtc_msgs[0].addr = sptr_info->sptr_client->addr;
    sgtc_msgs[0].flags = 0;
    sgtc_msgs[0].ptr_buf = &reg;
    sgtc_msgs[0].len = 1;

    sgtc_msgs[1].addr = sptr_info->sptr_client->addr;
    sgtc_msgs[1].flags |= FWK_I2C_M_RD;
    sgtc_msgs[1].ptr_buf = &value;
    sgtc_msgs[1].len = sizeof(value);

    retval = fwk_i2c_transfer(sptr_info->sptr_client, &sgtc_msgs[0], ARRAY_SIZE(sgtc_msgs));
    if (retval)
        return (TSC_MAX_12BIT + 1);

    return (TO_CONVERT_BYTE16(value) >> 4);
}

/**
 * @brief   tsc2007_pwroff_en_irq
 * @param   none
 * @retval  none
 * @note    close power-saving mode, enable tsc2007 irq
**/
static kuint16_t tsc2007_pwroff_en_irq(struct tsc2007_drv_info *sptr_info)
{
    return tsc2007_read_value(sptr_info, TSC_INDEX_PWRDOWN);
}

/**
 * @brief   tsc2007_initial
 * @param   none
 * @retval  none
 * @note    initial tsc2007
**/
static kuint16_t tsc2007_initial(struct tsc2007_data *sptr_data)
{
    sptr_data->pressure = 0;
    sptr_data->x_max = TSC_MAX_12BIT;
    sptr_data->y_max = TSC_MAX_12BIT;
    sptr_data->x = sptr_data->x_max;
    sptr_data->y = sptr_data->y_max;

    tsc2007_pwroff_en_irq(sptr_data->sptr_info);

    return 0;
}

/*!
 * @brief   tsc2007_read_AD_value
 * @param   none
 * @retval  none
 * @note    read abs-x, abs-y, abs-z
 */
static kbool_t tsc2007_read_ad_value(struct tsc2007_data *sptr_data)
{
    struct tsc2007_drv_info *sptr_info;

    sptr_info = sptr_data->sptr_info;

    sptr_data->pressure = 0;
    sptr_data->x  = tsc2007_read_value(sptr_info, TSC_INDEX_READ_X);
    sptr_data->y  = tsc2007_read_value(sptr_info, TSC_INDEX_READ_Y);
    sptr_data->z1 = tsc2007_read_value(sptr_info, TSC_INDEX_READ_Z1);
    sptr_data->z2 = tsc2007_read_value(sptr_info, TSC_INDEX_READ_Z2);

    /*!< stop adc, enable irq */
    tsc2007_pwroff_en_irq(sptr_info);

    if ((sptr_data->x >= TSC_MAX_12BIT) || (sptr_data->y >= TSC_MAX_12BIT))
        return false;

    if ((sptr_data->z1 >= TSC_MAX_12BIT) || (sptr_data->z2 >= TSC_MAX_12BIT))
        return false;

    /*!<
     * pressure calculate
     * method1: p = Px * (sptr_data->x / TSC_MAX_12BIT) * (sptr_data->z2 / sptr_data->z1 - 1), Px will be 660.
     * method2: p = Px * (x) * (TSC_MAX_12BIT / z1 -1) / TSC_MAX_12BIT - Py * (1 - y / 4096)
     * 
     * note: sptr_data->z1 is needed to be dived, it can not be zero!
     */
    if ((sptr_data->z1 > 10) && (sptr_data->z2 < (TSC_MAX_12BIT - 10)))
    {
		sptr_data->pressure  = sptr_data->z2 - sptr_data->z1;
		sptr_data->pressure *= sptr_data->x;
		sptr_data->pressure *= sptr_info->plate_ohms;
		sptr_data->pressure /= sptr_data->z1;
		sptr_data->pressure  = (sptr_data->pressure + 2047) >> 12;
    }

    return true;
}

/*!
 * @brief   tsc2007 irq handler
 * @param   ptrDev
 * @retval  none
 * @note    upper isr: start half isr
 */
static irq_return_t tsc2007_touch_isr(void *ptrDev)
{
    struct tsc2007_drv_info *sptr_info;

    sptr_info = (struct tsc2007_drv_info *)ptrDev;
    schedule_work(&sptr_info->sgtc_work);

    return 0;
}

/*!
 * @brief   tsc2007 irq handler
 * @param   ptrDev
 * @retval  none
 * @note    bottom isr: ADC
 */
static void tsc2007_touch_half_isr(struct workqueue *sptr_wq)
{
    struct tsc2007_drv_info *sptr_info;
    struct tsc2007_data *sptr_data;
    kuint16_t x_value, y_value;
    kuint16_t x_max_value, y_max_value, x_min_value, y_min_value;

    sptr_info = mr_container_of(sptr_wq, typeof(*sptr_info), sgtc_work);
    sptr_data = &sptr_info->sgtc_data;

    x_value = y_value = x_max_value = y_max_value = 0;
    x_min_value = sptr_data->x_max;
    y_min_value = sptr_data->y_max;

    /*!< 
     * take the average of six consecutive samples:
     * Only calculating in the pressed state, and will be not performed when the touch screen is lifted
     */
    for (kuint8_t sample_cnt = 0; sample_cnt < 6; sample_cnt++)
    {
        sptr_info->is_touch = fwk_gpio_get_value(sptr_info->sptr_gdesc);
        if (!sptr_info->is_touch)
            goto fail;

        /*!< return false: fail; return true: success */
        if (!tsc2007_read_ad_value(sptr_data))
            goto fail;

        /*!< if there is no pressure, touch screen is lifted */
        if (!sptr_data->pressure)
            goto fail;

        x_value += sptr_data->x;
        y_value += sptr_data->y;

        x_max_value = (x_max_value < sptr_data->x) ? sptr_data->x : x_max_value;
        y_max_value = (y_max_value < sptr_data->y) ? sptr_data->y : y_max_value;
        x_min_value = (x_min_value > sptr_data->x) ? sptr_data->x : x_min_value;
        y_min_value = (y_min_value > sptr_data->x) ? sptr_data->x : y_min_value;
    }

    /*!< give up the max and the min */
    x_value  -= (x_max_value + x_min_value);
    y_value  -= (y_max_value + y_min_value);
    x_value >>= 2;
    y_value >>= 2;

    /*!< save the result */
    sptr_data->x = x_value;
    sptr_data->y = y_value;

    sptr_info->is_can_read = true;
    wake_up(&sptr_info->sgtc_wqh);

    return;

fail:
    sptr_data->x = sptr_data->y = 0;
    return;
}

/*!
 * @brief   driver open
 * @param   sptr_inode, sptr_file
 * @retval  errno
 * @note    initial and enbale irq when driver is opened
 */
static kint32_t tsc2007_driver_open(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    struct tsc2007_drv_info *sptr_info;

    sptr_info = sptr_inode->sptr_cdev->privData;
    sptr_file->private_data = sptr_info;

    fwk_enable_irq(sptr_info->irq);
    tsc2007_initial(&sptr_info->sgtc_data);

    return ER_NORMAL;
}

/*!
 * @brief   driver close
 * @param   sptr_inode, sptr_file
 * @retval  errno
 * @note    disbale irq when driver is closed
 */
static kint32_t tsc2007_driver_close(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    struct tsc2007_drv_info *sptr_info;

    sptr_info = sptr_inode->sptr_cdev->privData;
    fwk_disable_irq(sptr_info->irq);

    sptr_file->private_data = mr_nullptr;

    return ER_NORMAL;
}

/*!
 * @brief   driver read
 * @param   sptr_file, buffer, size
 * @retval  size
 * @note    none
 */
static kssize_t tsc2007_driver_read(struct fwk_file *sptr_file, kbuffer_t *buffer, kssize_t size)
{
    struct tsc2007_drv_info *sptr_info;
    struct tsc2007_data *sptr_data;
    struct fwk_input_event sgtc_event[4] = {};
    kusize_t bytes = sizeof(sgtc_event);

    sptr_info = sptr_file->private_data;

	if (!(sptr_file->mode & O_NONBLOCK))
		wait_event(&sptr_info->sgtc_wqh, sptr_info->is_can_read);
    else
    {
        if (!sptr_info->is_can_read)
            return -ER_NREADY;
    }

    if (size < bytes)
        return -ER_INVALID;

    sptr_data = &sptr_info->sgtc_data;

    fwk_input_set_event(&sgtc_event[0], NR_INPUT_TYPE_KEY, NR_INPUT_BTN_TOUCH, sptr_info->is_touch);
    fwk_input_set_event(&sgtc_event[1], NR_INPUT_TYPE_ABS, NR_INPUT_ABS_X, sptr_data->x);
    fwk_input_set_event(&sgtc_event[2], NR_INPUT_TYPE_ABS, NR_INPUT_ABS_Y, sptr_data->y);
    fwk_input_set_event(&sgtc_event[3], NR_INPUT_TYPE_SYN, NR_INPUT_SYN_REPORT, true);

    fwk_copy_to_user(buffer, &sgtc_event, bytes);
    sptr_info->is_can_read = false;

    return bytes;
}

static const struct fwk_file_oprts sgtc_tsc2007_driver_oprts =
{
    .open = tsc2007_driver_open,
    .close = tsc2007_driver_close,
    .read = tsc2007_driver_read,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   driver probe
 * @param   sptr_client, sptr_id
 * @retval  errno
 * @note    none
 */
static kint32_t tsc2007_driver_probe(struct fwk_i2c_client *sptr_client, const struct fwk_i2c_device_id *sptr_id)
{
    struct tsc2007_drv_info *sptr_info;
    struct fwk_device_node *sptr_node;
    struct fwk_device *sptr_idev;
    kint32_t devnum;

    sptr_node = sptr_client->sgtc_dev.sptr_node;

    sptr_info = kzalloc(sizeof(*sptr_info), GFP_KERNEL);
    if (!isValid(sptr_info))
        return -ER_NOMEM;

    sptr_info->sptr_gdesc = fwk_gpio_desc_get(&sptr_client->sgtc_dev, "tsc-int", FWK_GPIO_DIR_IN);
    if (!isValid(sptr_info->sptr_gdesc)) 
        goto fail1;

    sptr_info->irq = sptr_client->irq;
    if (sptr_info->irq < 0)
        goto fail2;

    if (fwk_of_property_read_u32(sptr_node, "ti,x-plate-ohms", &sptr_info->plate_ohms))
        sptr_info->plate_ohms = 660;

    devnum = MKE_DEV_NUM(TSC2007_DRVIVER_MAJOR, 0);
    sptr_info->devnum = devnum;
    sptr_info->name = "tsc2007";
    sptr_info->sptr_client = sptr_client;
    init_waitqueue_head(&sptr_info->sgtc_wqh);
    INIT_WORK(&sptr_info->sgtc_work, tsc2007_touch_half_isr);

    if (fwk_request_irq(sptr_info->irq, tsc2007_touch_isr, IRQ_TYPE_EDGE_RISING | IRQ_TYPE_EDGE_FALLING, sptr_info->name, sptr_info))
        goto fail2;

    fwk_disable_irq(sptr_info->irq);

    if (fwk_register_chrdev(devnum, 1, sptr_info->name))
        goto fail3;

    sptr_info->sptr_cdev = fwk_cdev_alloc(&sgtc_tsc2007_driver_oprts);
    if (!isValid(sptr_info->sptr_cdev))
        goto fail4;

    if (fwk_cdev_add(sptr_info->sptr_cdev, devnum, 1))
        goto fail5;

    sptr_idev = fwk_device_create(NR_TYPE_CHRDEV, devnum, "%s%d", "input/event", 1);
    if (!isValid(sptr_idev))
        goto fail6;

    sptr_info->sptr_idev = sptr_idev;
    sptr_info->sptr_cdev->privData = sptr_info;
    sptr_info->sgtc_data.sptr_info = sptr_info;
    fwk_i2c_set_client_data(sptr_client, sptr_info);

	return ER_NORMAL;
    
fail6:
    fwk_cdev_del(sptr_info->sptr_cdev);
fail5:
    kfree(sptr_info->sptr_cdev);
fail4:
    fwk_unregister_chrdev(devnum, 0);
fail3:
    fwk_free_irq(sptr_info->irq, sptr_info);
fail2:
    fwk_gpio_desc_put(sptr_info->sptr_gdesc);
fail1:
    kfree(sptr_info);

    return -ER_ERROR;
}

/*!
 * @brief   driver remove
 * @param   sptr_client
 * @retval  errno
 * @note    none
 */
static kint32_t tsc2007_driver_remove(struct fwk_i2c_client *sptr_client)
{
    struct tsc2007_drv_info *sptr_info;

    sptr_info = fwk_i2c_get_client_data(sptr_client);

    fwk_device_destroy(sptr_info->sptr_idev);
    fwk_cdev_del(sptr_info->sptr_cdev);
    kfree(sptr_info->sptr_cdev);
    fwk_unregister_chrdev(sptr_info->devnum, 0);
    fwk_free_irq(sptr_info->irq, sptr_info);
    fwk_gpio_desc_put(sptr_info->sptr_gdesc);
    kfree(sptr_info);
    fwk_i2c_set_client_data(sptr_client, mr_nullptr);

    return ER_NORMAL;
}

static const struct fwk_i2c_device_id sgtc_tsc2007_driver_ids[] =
{
    { .name = "tsc2007", .driver_data = -1 },
    {},
};

/*!< device id for device-tree */
static const struct fwk_of_device_id sgtc_tsc2007_driver_dts[] =
{
	{ .compatible = "ti,i2c-tsc2007", },
	{},
};

/*!< platform instance */
static struct fwk_i2c_driver sgtc_tsc2007_driver =
{
	.probe	= tsc2007_driver_probe,
	.remove	= tsc2007_driver_remove,

    .sptr_id_table = sgtc_tsc2007_driver_ids,
	
	.sgtc_driver =
	{
		.name 	= "tsc2007,touch screen",
		.id 	= -1,
		.sptr_of_match_table = sgtc_tsc2007_driver_dts,
	},
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   driver init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init tsc2007_driver_init(void)
{
	return fwk_i2c_add_driver(&sgtc_tsc2007_driver);
}

/*!
 * @brief   driver exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit tsc2007_driver_exit(void)
{
	fwk_i2c_del_driver(&sgtc_tsc2007_driver);
}

IMPORT_DRIVER_INIT(tsc2007_driver_init);
IMPORT_DRIVER_EXIT(tsc2007_driver_exit);

/*!< end of file */
