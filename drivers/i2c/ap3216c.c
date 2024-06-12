/*
 * Template of Environment Sensor Driver : ap3216c
 *
 * File Name:   ap3216c.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.06.06
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

/*!< The defines */
#define AP3216C_DRVIVER_MAJOR                       (221)

/*!< AP3216C register (address) */
/*!<
--------------------------------------------------------------
    Register  | field   |  value  | meanning
--------------------------------------------------------------
    0x00                            System Configuration (Default : 0x00)
                bit[2:0]            System Mode (Default : 000)
                            000:    Power down (Default)
                            001:    ALS function active
                            010:    PS+IR function active
                            011:    ALS and PS+IR functions active
                            100:    SW reset
                            101:    ALS function once
                            110:    PS+IR function once
                            111:    ALS and PS+IR functions once

    0x01                            INT Status
                bit1                PS Int (Read only) (Default : 0)
                            0:      Interrupt is cleared or not triggered yet
                            1:      Interrupt is triggered
                bit0                ALS Int (Read only) (Default : 0)
                            0:      Interrupt is cleared or not triggered yet
                            1:      Interrupt is triggered

    0x02                            INT Clear Manner 0 Clear Manner (Default : 0)
                            0:      INT is automatically cleared by reading data registers
                            1:      Software clear after writing 1 into address 0x01 each bit

    0x0A                            IR Data Low 
                bit7                IR overflow (Read only)
                            0:      Valid IR and PS data
                            1:      Invalid IR and PS data
                bit[1:0]            (Read only) IR lower byte of ADC output
    0x0B                            IR Data High 
                bit[7:0]            (Read only) IR higher byte of ADC output

    0x0C                            ALS Data Low 
                bit[7:0]            (Read only) ALS lower byte of ADC output
    0x0D                            ALS Data High 
                bit[7:0]            (Read only) ALS higher byte of ADC output

    0x0E                            PS Data Low 
                bit7                Object detect (Read only)
                            0:      The object leaving
                            1:      The object closed
                bit6                IR overflow (Read only)
                            0:      Valid IR, PS data and object detected
                            1:      Invalid IR, PS data and object detected
                bit[3:0]            (Read only) PS lower byte of ADC output
    0x0F                            PS Data High 
                bit7                Object detect (Read only)
                            0:      The object leaving
                            1:      The object closed
                bit6                IR overflow (Read only)
                            0:      Valid IR, PS data and object detected
                            1:      Invalid IR, PS data and object detected
                bit[5:0]            (Read only) PS higher byte of ADC output
*/

#define AP3216C_REG_SYSCONFIG	                    (0x00)	/*!< configuration register */
#define AP3216C_REG_INTSTATUS	                    (0X01)	/*!< interrupt status register */
#define AP3216C_REG_INTCLEAR	                    (0X02)	/*!< interrupt clear register */

#define AP3216C_REG_IR_LOW	                        (0x0A)	/*!< data register: low byte of IR */
#define AP3216C_REG_IR_HIGH	                        (0x0B)	/*!< data register: high byte of IR	*/
#define AP3216C_REG_ALS_LOW	                        (0x0C)	/*!< data register: low byte of ALS	*/
#define AP3216C_REG_ALS_HIGH	                    (0X0D)	/*!< data register: high byte of ALS */
#define AP3216C_REG_PS_LOW	                        (0X0E)	/*!< data register: low byte of PS */
#define AP3216C_REG_PS_HIGH	                        (0X0F)	/*!< data register: high byte of PS	*/

/*!< data */
#define AP3216C_DATA_MAX                            (255)

#define AP3216C_RESET                               (0x04)
#define AP3216C_ALS_ACTIVE                          (0x01)
#define AP3216C_IR_PS_ACTIVE                        (0x02)
#define AP3216C_ALS_IR_PS_ACTIVE                    (AP3216C_ALS_ACTIVE | AP3216C_IR_PS_ACTIVE)

/*!< false: valid; true: unvalid */
#define AP3216C_IS_IR_VALID(low)                    (!!((low) & 0x80))
#define AP3216C_IS_PS_VALID(val)                    (!!((val) & 0x40))

#define AP3216C_IR_DATA(low, high)                  (((high) << 2) | ((low) & 0x03))
#define AP3216C_ALS_DATA(low, high)                 (((high) << 8) | (low))
#define AP3216C_PS_DATA(low, high)                  ((((high) & 0x3f) << 4) | ((low) & 0x0f))

typedef struct ap3216c_data
{
    kint32_t ir;
    kint32_t als;
    kint32_t ps;

} srt_ap3216c_data_t;

typedef struct ap3216c_drv_info
{
    kchar_t *name;
    struct fwk_i2c_client *sprt_client;

    kint32_t devnum;
    struct fwk_cdev *sprt_cdev;
    struct fwk_device *sprt_idev;

} ap3216c_drv_info_t;

/*!< API function */
/*!
 * @brief  ap3216c_write_value
 * @param  None
 * @retval None
 * @note   write data by i2c
 */
static kuint16_t ap3216c_write_value(struct ap3216c_drv_info *sprt_info, kuint8_t reg, kuint8_t value)
{
    struct fwk_i2c_msg sgrt_msgs;
    kuint8_t buf[2] = { reg, value };
    kint32_t retval;

    sgrt_msgs.addr = sprt_info->sprt_client->addr;
    sgrt_msgs.flags = 0;
    sgrt_msgs.ptr_buf = buf;
    sgrt_msgs.len = sizeof(buf);

    retval = fwk_i2c_transfer(sprt_info->sprt_client, &sgrt_msgs, 1);
    if (retval)
        return (AP3216C_DATA_MAX + 1);

    return value;
}

/*!
 * @brief  ap3216c_read_value
 * @param  None
 * @retval None
 * @note   read data by i2c
 */
static kuint16_t ap3216c_read_value(struct ap3216c_drv_info *sprt_info, kuint8_t reg)
{
    struct fwk_i2c_msg sgrt_msgs[2];
    kuint8_t value = 0;
    kint32_t retval;

    sgrt_msgs[0].addr = sprt_info->sprt_client->addr;
    sgrt_msgs[0].flags = 0;
    sgrt_msgs[0].ptr_buf = &reg;
    sgrt_msgs[0].len = 1;

    sgrt_msgs[1].addr = sprt_info->sprt_client->addr;
    sgrt_msgs[1].flags |= FWK_I2C_M_RD;
    sgrt_msgs[1].ptr_buf = &value;
    sgrt_msgs[1].len = sizeof(value);

    retval = fwk_i2c_transfer(sprt_info->sprt_client, &sgrt_msgs[0], ARRAY_SIZE(sgrt_msgs));
    if (retval)
        return (AP3216C_DATA_MAX + 1);

    return value;
}

static kint32_t ap3216c_read_ir(struct ap3216c_drv_info *sprt_info)
{
    kuint16_t value[2];

    value[0] = ap3216c_read_value(sprt_info, AP3216C_REG_IR_LOW);
    if (value[0] > AP3216C_DATA_MAX)
        return -ER_RDATA_FAILD;
    
    if (AP3216C_IS_IR_VALID(value[0]))
        return -ER_UNVALID;

    value[1] = ap3216c_read_value(sprt_info, AP3216C_REG_IR_HIGH);
    if (value[1] > AP3216C_DATA_MAX)
        return -ER_RDATA_FAILD;

    return AP3216C_IR_DATA(value[0], value[1]);
}

static kint32_t ap3216c_read_als(struct ap3216c_drv_info *sprt_info)
{
    kuint16_t value[2];

    value[0] = ap3216c_read_value(sprt_info, AP3216C_REG_ALS_LOW);
    if (value[0] > AP3216C_DATA_MAX)
        return -ER_RDATA_FAILD;

    value[1] = ap3216c_read_value(sprt_info, AP3216C_REG_ALS_HIGH);
    if (value[1] > AP3216C_DATA_MAX)
        return -ER_RDATA_FAILD;

    return AP3216C_ALS_DATA(value[0], value[1]);
}

static kint32_t ap3216c_read_ps(struct ap3216c_drv_info *sprt_info)
{
    kuint16_t value[2];

    value[0] = ap3216c_read_value(sprt_info, AP3216C_REG_PS_LOW);
    if (value[0] > AP3216C_DATA_MAX)
        return -ER_RDATA_FAILD;
    
    if (AP3216C_IS_PS_VALID(value[0]))
        return -ER_UNVALID;

    value[1] = ap3216c_read_value(sprt_info, AP3216C_REG_PS_HIGH);
    if (value[1] > AP3216C_DATA_MAX)
        return -ER_RDATA_FAILD;

    if (AP3216C_IS_PS_VALID(value[1]))
        return -ER_UNVALID;

    return AP3216C_PS_DATA(value[0], value[1]);
}

static kint32_t ap3216c_init(struct ap3216c_drv_info *sprt_info)
{
    kuint16_t value;

    /*!< reset ap3216c */
    value = ap3216c_write_value(sprt_info, AP3216C_REG_SYSCONFIG, AP3216C_RESET);
    if (value > AP3216C_DATA_MAX)
        return -ER_FAILD;

    /*!< wait for ap3216c reset finished */
	delay_ms(50);
    
    /*!< configure ap3216c, enable als, ps and ir */
	value = ap3216c_write_value(sprt_info, AP3216C_REG_SYSCONFIG, AP3216C_ALS_IR_PS_ACTIVE);
    if (value > AP3216C_DATA_MAX)
        return -ER_FAILD;

    /*!< verify configuration register*/
	value = ap3216c_read_value(sprt_info, AP3216C_REG_SYSCONFIG);
    if (value > AP3216C_DATA_MAX)
        return -ER_RDATA_FAILD;
    if (value != AP3216C_ALS_IR_PS_ACTIVE)
        return -ER_FAILD;

    return ER_NORMAL;
}

static kint32_t ap3216c_driver_open(struct fwk_inode *sprt_inode, struct fwk_file *sprt_file)
{
    struct ap3216c_drv_info *sprt_info;

    sprt_info = sprt_inode->sprt_cdev->privData;
    sprt_file->private_data = sprt_info;

    ap3216c_init(sprt_info);

    return ER_NORMAL;
}

static kint32_t ap3216c_driver_close(struct fwk_inode *sprt_inode, struct fwk_file *sprt_file)
{
    sprt_file->private_data = mrt_nullptr;

    return ER_NORMAL;
}

static kssize_t ap3216c_driver_read(struct fwk_file *sprt_file, kbuffer_t *buffer, kssize_t size)
{
    struct ap3216c_drv_info *sprt_info;
    struct ap3216c_data sgrt_data;

    if (size < sizeof(sgrt_data))
        return -ER_RETRY;

    sprt_info = sprt_file->private_data;

    sgrt_data.ir = ap3216c_read_ir(sprt_info);
    if (sgrt_data.ir < 0)
        sgrt_data.ir = 0;

    sgrt_data.als = ap3216c_read_als(sprt_info);
    if (sgrt_data.ir < 0)
        sgrt_data.ir = 0;

    sgrt_data.ps = ap3216c_read_ps(sprt_info);
    if (sgrt_data.ir < 0)
        sgrt_data.ir = 0;

    fwk_copy_to_user(buffer, &sgrt_data, sizeof(sgrt_data));

    return sizeof(sgrt_data);
}

static const struct fwk_file_oprts sgrt_ap3216c_driver_oprts =
{
    .open = ap3216c_driver_open,
    .close = ap3216c_driver_close,
    .read = ap3216c_driver_read,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   imx_i2c_driver_probe
 * @param   sprt_dev
 * @retval  errno
 * @note    none
 */
static kint32_t ap3216c_driver_probe(struct fwk_i2c_client *sprt_client, const struct fwk_i2c_device_id *sprt_id)
{
    struct ap3216c_drv_info *sprt_info;
    struct fwk_device *sprt_idev;
    kint32_t devnum;

    sprt_info = kzalloc(sizeof(*sprt_info), GFP_KERNEL);
    if (!isValid(sprt_info))
        return -ER_NOMEM;

    devnum = MKE_DEV_NUM(AP3216C_DRVIVER_MAJOR, 0);
    sprt_info->devnum = devnum;
    sprt_info->name = "ap3216c";
    sprt_info->sprt_client = sprt_client;

    if (fwk_register_chrdev(devnum, 1, sprt_info->name))
        goto fail1;

    sprt_info->sprt_cdev = fwk_cdev_alloc(&sgrt_ap3216c_driver_oprts);
    if (!isValid(sprt_info->sprt_cdev))
        goto fail2;

    if (fwk_cdev_add(sprt_info->sprt_cdev, devnum, 1))
        goto fail3;

    sprt_idev = fwk_device_create(NR_TYPE_CHRDEV, devnum, "ap3216c");
    if (!isValid(sprt_idev))
        goto fail4;

    sprt_info->sprt_idev = sprt_idev;
    sprt_info->sprt_cdev->privData = sprt_info;
    fwk_i2c_set_client_data(sprt_client, sprt_info);

	return ER_NORMAL;
    
fail4:
    fwk_cdev_del(sprt_info->sprt_cdev);
fail3:
    kfree(sprt_info->sprt_cdev);
fail2:
    fwk_unregister_chrdev(devnum, 0);
fail1:
    kfree(sprt_info);

    return -ER_ERROR;
}

/*!
 * @brief   imx_i2c_driver_remove
 * @param   sprt_dev
 * @retval  errno
 * @note    none
 */
static kint32_t ap3216c_driver_remove(struct fwk_i2c_client *sprt_client)
{
    struct ap3216c_drv_info *sprt_info;

    sprt_info = fwk_i2c_get_client_data(sprt_client);

    fwk_device_destroy(sprt_info->sprt_idev);
    fwk_cdev_del(sprt_info->sprt_cdev);
    kfree(sprt_info->sprt_cdev);
    fwk_unregister_chrdev(sprt_info->devnum, 0);
    kfree(sprt_info);
    fwk_i2c_set_client_data(sprt_client, mrt_nullptr);

    return ER_NORMAL;
}

static const struct fwk_i2c_device_id sgrt_ap3216c_driver_ids[] =
{
    { .name = "ap3216c", .driver_data = -1 },
    {},
};

/*!< device id for device-tree */
static const struct fwk_of_device_id sgrt_ap3216c_driver_dts[] =
{
	{ .compatible = "liteon,ap3216c", },
	{},
};

/*!< platform instance */
static struct fwk_i2c_driver sgrt_ap3216c_driver =
{
	.probe	= ap3216c_driver_probe,
	.remove	= ap3216c_driver_remove,

    .sprt_id_table = sgrt_ap3216c_driver_ids,
	
	.sgrt_driver =
	{
		.name 	= "ap3216c,env-sensor",
		.id 	= -1,
		.sprt_of_match_table = sgrt_ap3216c_driver_dts,
	},
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   ap3216c_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init ap3216c_driver_init(void)
{
	return fwk_i2c_add_driver(&sgrt_ap3216c_driver);
}

/*!
 * @brief   ap3216c_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit ap3216c_driver_exit(void)
{
	fwk_i2c_del_driver(&sgrt_ap3216c_driver);
}

IMPORT_DRIVER_INIT(ap3216c_driver_init);
IMPORT_DRIVER_EXIT(ap3216c_driver_exit);

/*!< end of file */
