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
#include <platform/input/fwk_input.h>
#include <kernel/workqueue.h>

/*!< The defines */
#define TSC2007_DRVIVER_MAJOR                       (220)

/* tsc2007, index:
 * bit0:        X
 * bit1:        M
 * bit2 ~ bit3: PD0 ~ PD1
 * bit4 ~ bit7: C0 ~ C3
 */
/* C3 ~ C0 */
#define TSC2007_CTRL_MEASURE_TEMP0		            (0x00 << 4)         /* 0000 */
#define TSC2007_CTRL_MEASURE_AUX		            (0x02 << 4)         /* 0010 */
#define TSC2007_CTRL_MEASURE_TEMP1		            (0x04 << 4)         /* 0100 */
#define TSC2007_CTRL_ACTIVATE_XN		            (0x08 << 4)         /* 1000 */
#define TSC2007_CTRL_ACTIVATE_YN		            (0x09 << 4)         /* 1001 */
#define TSC2007_CTRL_ACTIVATE_YP_XN		            (0x0A << 4)         /* 1010 */
#define TSC2007_CTRL_SETUP			                (0x0B << 4)         /* 1011 */
#define TSC2007_CTRL_MEASURE_X		                (0x0C << 4)         /* 1100 */
#define TSC2007_CTRL_MEASURE_Y		                (0x0D << 4)         /* 1101 */
#define TSC2007_CTRL_MEASURE_Z1		                (0x0E << 4)         /* 1110 */
#define TSC2007_CTRL_MEASURE_Z2		                (0x0F << 4)         /* 1111 */

/* PD1 ~ PD0 */
#define TSC2007_PWR_POWER_OFF_IRQ_EN	            (0x00 << 2)         /* 00 */
#define TSC2007_PWR_ADC_ON_IRQ_DIS0		            (0x01 << 2)         /* 01 */
#define TSC2007_PWR_ADC_OFF_IRQ_EN		            (0x02 << 2)         /* 10 */
#define TSC2007_PWR_ADC_ON_IRQ_DIS1		            (0x03 << 2)         /* 11 */

/* M : MODE */
#define TSC2007_MODE_12BIT			                (0x00 << 1)         /* 0 */
#define TSC2007_MODE_8BIT			                (0x01 << 1)         /* 1 */

/* 实际定义 */
#define	TSC_MAX_12BIT			                    (4095 + 1)          /* max: 4096 - 1 = 4095 */

#define TSC_ADC_ON_12BIT    \
            (TSC2007_MODE_12BIT | TSC2007_PWR_ADC_ON_IRQ_DIS0)          /* 12bit ADC, Disable INT IRQ */

#define TSC_INDEX_READ_X	\
            (TSC_ADC_ON_12BIT | TSC2007_CTRL_MEASURE_X)                 /* 12bit ADC, Disable IRQ, Reg X */

#define TSC_INDEX_READ_Y	\
            (TSC_ADC_ON_12BIT | TSC2007_CTRL_MEASURE_Y)                 /* 12bit ADC, Disable IRQ, Reg Y */

#define TSC_INDEX_READ_Z1	\
            (TSC_ADC_ON_12BIT | TSC2007_CTRL_MEASURE_Z1)                /* 12bit ADC, Disable IRQ, Reg Z1 */

#define TSC_INDEX_READ_Z2	\
            (TSC_ADC_ON_12BIT | TSC2007_CTRL_MEASURE_Z2)                /* 12bit ADC, Disable IRQ, Reg Z2 */

#define TSC_INDEX_PWRDOWN	\
            (TSC2007_MODE_12BIT | TSC2007_PWR_ADC_OFF_IRQ_EN)           /* 12bit ADC, PWR DOWN, Enable IRQ */

typedef struct tsc2007_data
{
    kuint16_t x;
    kuint16_t y;
    kuint16_t z1;
    kuint16_t z2;
    kuint32_t pressure;

    kuint16_t x_max;
    kuint16_t y_max;

    struct tsc2007_drv_info *sprt_info;

} tsc2007_data_t;

typedef struct tsc2007_drv_info
{
    kchar_t *name;
    struct fwk_i2c_client *sprt_client;

    struct fwk_gpio_desc *sprt_gdesc;
    kint32_t irq;

    kuint32_t plate_ohms;
    struct tsc2007_data sgrt_data;

    kbool_t is_touch;
    kbool_t is_can_read;

    kint32_t devnum;
    struct fwk_cdev *sprt_cdev;

    struct workqueue sgrt_wq;

} tsc2007_drv_info_t;

/*!< API function */
/*!
 * @brief  tsc2007_read_value
 * @param  None
 * @retval None
 * @note   rtscr读数据
 */
static kuint16_t tsc2007_read_value(struct tsc2007_drv_info *sprt_info, kuint8_t reg)
{
    struct fwk_i2c_msg sgrt_msgs[2];
    kuint16_t value = 0;

    sgrt_msgs[0].addr = sprt_info->sprt_client->addr;
    sgrt_msgs[0].flags = 0;
    sgrt_msgs[0].ptr_buf = &reg;
    sgrt_msgs[0].len = 1;

    sgrt_msgs[1].addr = sprt_info->sprt_client->addr;
    sgrt_msgs[1].flags |= FWK_I2C_M_RD;
    sgrt_msgs[1].ptr_buf = &value;
    sgrt_msgs[1].len = sizeof(value);

    if (fwk_i2c_transfer(sprt_info->sprt_client, &sgrt_msgs[0], ARRAY_SIZE(sgrt_msgs)))
        return (TSC_MAX_12BIT + 1);

    return (TO_CONVERT_BYTE16(value) >> 4);
}

/**
 * @brief  tsc2007_pwroff_en_irq
 * @param  None
 * @retval None
 * @note   关闭节能模式, 打开触屏中断
**/
static kuint16_t tsc2007_pwroff_en_irq(struct tsc2007_drv_info *sprt_info)
{
    return tsc2007_read_value(sprt_info, TSC_INDEX_PWRDOWN);
}

/**
 * @brief  tsc2007_initial
 * @param  None
 * @retval None
 * @note   tsc2007初始化
**/
static kuint16_t tsc2007_initial(struct tsc2007_data *sprt_data)
{
    sprt_data->pressure = 0;
    sprt_data->x_max = TSC_MAX_12BIT;
    sprt_data->y_max = TSC_MAX_12BIT;
    sprt_data->x = sprt_data->x_max;
    sprt_data->y = sprt_data->y_max;

    tsc2007_pwroff_en_irq(sprt_data->sprt_info);

    return 0;
}

/*!
 * @brief  tsc2007_read_AD_value
 * @param  None
 * @retval None
 * @note   rtscr读取全部AD值
 */
static kbool_t tsc2007_read_ad_value(struct tsc2007_data *sprt_data)
{
    struct tsc2007_drv_info *sprt_info;

    /* tsc2007故障: IRQ不断关闭、打开, 将令触屏持续陷入中断循环(关闭(0) ===> 打开(1) ===> 关闭(0)), 直至触屏抬起 */

    sprt_info = sprt_data->sprt_info;

    /* 采集数据时关闭IRQ */
    sprt_data->pressure = 0;

    sprt_data->x  = tsc2007_read_value(sprt_info, TSC_INDEX_READ_X);
    sprt_data->y  = tsc2007_read_value(sprt_info, TSC_INDEX_READ_Y);
    sprt_data->z1 = tsc2007_read_value(sprt_info, TSC_INDEX_READ_Z1);
    sprt_data->z2 = tsc2007_read_value(sprt_info, TSC_INDEX_READ_Z2);

    /* 停止AD采样, 开启IRQ */
    tsc2007_pwroff_en_irq(sprt_info);

    if ((sprt_data->x >= TSC_MAX_12BIT) || (sprt_data->y >= TSC_MAX_12BIT))
        return false;

    if ((sprt_data->z1 >= TSC_MAX_12BIT) || (sprt_data->z2 >= TSC_MAX_12BIT))
        return false;

    /*!<
     * pressure calculate
     * method1: p = Px * (sprt_data->x / TSC_MAX_12BIT) * (sprt_data->z2 / sprt_data->z1 - 1), Px will be 660.
     * method2: p = Px * (x) * (TSC_MAX_12BIT / z1 -1) / TSC_MAX_12BIT - Py * (1 - y / 4096)
     * 
     * note: sprt_data->z1 is needed to be dived, it can not be zero!
     */
    if ((sprt_data->z1 > 10) && (sprt_data->z2 < (TSC_MAX_12BIT - 10)))
    {
		sprt_data->pressure  = sprt_data->z2 - sprt_data->z1;
		sprt_data->pressure *= sprt_data->x;
		sprt_data->pressure *= sprt_info->plate_ohms;
		sprt_data->pressure /= sprt_data->z1;
		sprt_data->pressure  = (sprt_data->pressure + 2047) >> 12;
    }

    return true;
}

static irq_return_t tsc2007_touch_isr(void *ptrDev)
{
    struct tsc2007_drv_info *sprt_info;

    sprt_info = (struct tsc2007_drv_info *)ptrDev;
    schedule_work(&sprt_info->sgrt_wq);

    return 0;
}

static void tsc2007_touch_half_isr(struct workqueue *sprt_wq)
{
    struct tsc2007_drv_info *sprt_info;
    struct tsc2007_data *sprt_data;
    kuint16_t x_value, y_value;
    kuint16_t x_max_value, y_max_value, x_min_value, y_min_value;

    sprt_info = mrt_container_of(sprt_wq, typeof(*sprt_info), sgrt_wq);
    sprt_data = &sprt_info->sgrt_data;

    x_value = y_value = x_max_value = y_max_value = 0;
    x_min_value = sprt_data->x_max;
    y_min_value = sprt_data->y_max;

    /* 连续采样六次取平均: 仅用于按压状态下, 抬起时不做此计算 */
    for (kuint8_t sample_cnt = 0; sample_cnt < 6; sample_cnt++)
    {
        sprt_info->is_touch = fwk_gpio_get_value(sprt_info->sprt_gdesc);
        if (!sprt_info->is_touch)
            goto fail;

        /* return false: fail; return true: success */
        if (!tsc2007_read_ad_value(sprt_data))
            goto fail;

        /* 若读取失败或压力值为0, 认定为触屏抬起或误中断, 不做AD采样计算 */
        if (!sprt_data->pressure)
            goto fail;

        x_value += sprt_data->x;
        y_value += sprt_data->y;

        x_max_value = (x_max_value < sprt_data->x) ? sprt_data->x : x_max_value;
        y_max_value = (y_max_value < sprt_data->y) ? sprt_data->y : y_max_value;
        x_min_value = (x_min_value > sprt_data->x) ? sprt_data->x : x_min_value;
        y_min_value = (y_min_value > sprt_data->x) ? sprt_data->x : y_min_value;
    }

    /* 减去最大值和最小值, 剩余四个采样值取平均 */
    x_value  -= (x_max_value + x_min_value);
    y_value  -= (y_max_value + y_min_value);
    x_value >>= 2;
    y_value >>= 2;

    /* 保存最终采样值 */
    sprt_data->x = x_value;
    sprt_data->y = y_value;

    sprt_info->is_can_read = true;

    return;

fail:
    sprt_data->x = sprt_data->y = 0;
    return;
}

static kint32_t tsc2007_driver_open(struct fwk_inode *sprt_inode, struct fwk_file *sprt_file)
{
    struct tsc2007_drv_info *sprt_info;

    sprt_info = sprt_inode->sprt_cdev->privData;
    sprt_file->private_data = sprt_info;

    fwk_enable_irq(sprt_info->irq);
    tsc2007_initial(&sprt_info->sgrt_data);

    return NR_IS_NORMAL;
}

static kint32_t tsc2007_driver_close(struct fwk_inode *sprt_inode, struct fwk_file *sprt_file)
{
    struct tsc2007_drv_info *sprt_info;

    sprt_info = sprt_inode->sprt_cdev->privData;
    fwk_disable_irq(sprt_info->irq);

    sprt_file->private_data = mrt_nullptr;

    return NR_IS_NORMAL;
}

static kssize_t tsc2007_driver_read(struct fwk_file *sprt_file, kbuffer_t *buffer, kssize_t size)
{
    struct tsc2007_drv_info *sprt_info;
    struct tsc2007_data *sprt_data;
    struct fwk_input_event sgrt_event[4] = {};
    kusize_t bytes = sizeof(sgrt_event);

    sprt_info = sprt_file->private_data;
    if (!sprt_info->is_can_read)
        return -NR_IS_NREADY;

    if (size < bytes)
        return -NR_IS_UNVALID;

    sprt_data = &sprt_info->sgrt_data;

    fwk_input_set_event(&sgrt_event[0], NR_INPUT_TYPE_KEY, NR_INPUT_BTN_TOUCH, sprt_info->is_touch);
    fwk_input_set_event(&sgrt_event[1], NR_INPUT_TYPE_ABS, NR_INPUT_ABS_X, sprt_data->x);
    fwk_input_set_event(&sgrt_event[2], NR_INPUT_TYPE_ABS, NR_INPUT_ABS_Y, sprt_data->y);
    fwk_input_set_event(&sgrt_event[3], NR_INPUT_TYPE_SYN, NR_INPUT_SYN_REPORT, true);

    fwk_copy_to_user(buffer, &sgrt_event, bytes);
    sprt_info->is_can_read = false;

    return bytes;
}

static const struct fwk_file_oprts sgrt_tsc2007_driver_oprts =
{
    .open = tsc2007_driver_open,
    .close = tsc2007_driver_close,
    .read = tsc2007_driver_read,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   imx_i2c_driver_probe
 * @param   sprt_dev
 * @retval  errno
 * @note    none
 */
static kint32_t tsc2007_driver_probe(struct fwk_i2c_client *sprt_client, const struct fwk_i2c_device_id *sprt_id)
{
    struct tsc2007_drv_info *sprt_info;
    struct fwk_device_node *sprt_node;
    kint32_t devnum;

    sprt_node = sprt_client->sgrt_dev.sprt_node;

    sprt_info = kzalloc(sizeof(*sprt_info), GFP_KERNEL);
    if (!isValid(sprt_info))
        return -NR_IS_NOMEM;

    sprt_info->sprt_gdesc = fwk_gpio_desc_get(&sprt_client->sgrt_dev, "tsc-int", FWK_GPIO_DIR_IN);
    if (!isValid(sprt_info->sprt_gdesc)) 
        goto fail1;

    sprt_info->irq = sprt_client->irq;
    if (sprt_info->irq < 0)
        goto fail2;

    if (fwk_of_property_read_u32(sprt_node, "ti,x-plate-ohms", &sprt_info->plate_ohms))
        sprt_info->plate_ohms = 660;

    devnum = MKE_DEV_NUM(TSC2007_DRVIVER_MAJOR, 0);
    sprt_info->devnum = devnum;
    sprt_info->name = "tsc2007";
    sprt_info->sprt_client = sprt_client;
    INIT_WORK(&sprt_info->sgrt_wq, tsc2007_touch_half_isr);

    if (fwk_request_irq(sprt_info->irq, tsc2007_touch_isr, IRQ_TYPE_EDGE_RISING | IRQ_TYPE_EDGE_FALLING, sprt_info->name, sprt_info))
        goto fail2;

//  fwk_disable_irq(sprt_info->irq);

    if (fwk_register_chrdev(devnum, 1, sprt_info->name))
        goto fail3;

    sprt_info->sprt_cdev = fwk_cdev_alloc(&sgrt_tsc2007_driver_oprts);
    if (!isValid(sprt_info->sprt_cdev))
        goto fail4;

    if (fwk_cdev_add(sprt_info->sprt_cdev, devnum, 1))
        goto fail5;

    if (fwk_device_create(NR_TYPE_CHRDEV, devnum, "input/event0"))
        goto fail6;

    sprt_info->sprt_cdev->privData = sprt_info;
    sprt_info->sgrt_data.sprt_info = sprt_info;
    fwk_i2c_set_client_data(sprt_client, sprt_info);

    tsc2007_initial(&sprt_info->sgrt_data);

	return NR_IS_NORMAL;
    
fail6:
    fwk_cdev_del(sprt_info->sprt_cdev);
fail5:
    kfree(sprt_info->sprt_cdev);
fail4:
    fwk_unregister_chrdev(devnum, 0);
fail3:
    fwk_free_irq(sprt_info->irq, sprt_info);
fail2:
    fwk_gpio_desc_put(sprt_info->sprt_gdesc);
fail1:
    kfree(sprt_info);

    return -NR_IS_ERROR;
}

/*!
 * @brief   imx_i2c_driver_remove
 * @param   sprt_dev
 * @retval  errno
 * @note    none
 */
static kint32_t tsc2007_driver_remove(struct fwk_i2c_client *sprt_client)
{
    struct tsc2007_drv_info *sprt_info;

    sprt_info = fwk_i2c_get_client_data(sprt_client);

    fwk_device_destroy(sprt_info->name);
    fwk_cdev_del(sprt_info->sprt_cdev);
    kfree(sprt_info->sprt_cdev);
    fwk_unregister_chrdev(sprt_info->devnum, 0);
    fwk_free_irq(sprt_info->irq, sprt_info);
    fwk_gpio_desc_put(sprt_info->sprt_gdesc);
    kfree(sprt_info);
    fwk_i2c_set_client_data(sprt_client, mrt_nullptr);

    return NR_IS_NORMAL;
}

static const struct fwk_i2c_device_id sgrt_tsc2007_driver_ids[] =
{
    { .name = "tsc2007", .driver_data = -1 },
    {},
};

/*!< device id for device-tree */
static const struct fwk_of_device_id sgrt_tsc2007_driver_dts[] =
{
	{ .compatible = "ti,i2c-tsc2007", },
	{},
};

/*!< platform instance */
static struct fwk_i2c_driver sgrt_tsc2007_driver =
{
	.probe	= tsc2007_driver_probe,
	.remove	= tsc2007_driver_remove,

    .sprt_id_table = sgrt_tsc2007_driver_ids,
	
	.sgrt_driver =
	{
		.name 	= "tsc2007,touch screen",
		.id 	= -1,
		.sprt_of_match_table = sgrt_tsc2007_driver_dts,
	},
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   imx_i2c_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init tsc2007_driver_init(void)
{
	return fwk_i2c_add_driver(&sgrt_tsc2007_driver);
}

/*!
 * @brief   imx_i2c_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit tsc2007_driver_exit(void)
{
	fwk_i2c_del_driver(&sgrt_tsc2007_driver);
}

IMPORT_DRIVER_INIT(tsc2007_driver_init);
IMPORT_DRIVER_EXIT(tsc2007_driver_exit);

/*!< end of file */
