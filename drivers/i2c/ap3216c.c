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
#include <kernel/sleep.h>

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
    struct fwk_i2c_client *sptr_client;

    kint32_t devnum;
    struct fwk_cdev *sptr_cdev;
    struct fwk_device *sptr_idev;

} ap3216c_drv_info_t;

/*!< API function */
/*!
 * @brief  ap3216c_write_value
 * @param  none
 * @retval none
 * @note   write data by i2c
 */
static kuint16_t ap3216c_write_value(struct ap3216c_drv_info *sptr_info, kuint8_t reg, kuint8_t value)
{
    struct fwk_i2c_msg sgtc_msgs = {};
    kuint8_t buf[2] = { reg, value };
    kint32_t retval;

    sgtc_msgs.addr = sptr_info->sptr_client->addr;
    sgtc_msgs.flags = 0;
    sgtc_msgs.ptr_buf = buf;
    sgtc_msgs.len = sizeof(buf);

    retval = fwk_i2c_transfer(sptr_info->sptr_client, &sgtc_msgs, 1);
    if (retval)
        return (AP3216C_DATA_MAX + 1);

    return value;
}

/*!
 * @brief  ap3216c_read_value
 * @param  none
 * @retval none
 * @note   read data by i2c
 */
static kuint16_t ap3216c_read_value(struct ap3216c_drv_info *sptr_info, kuint8_t reg)
{
    struct fwk_i2c_msg sgtc_msgs[2] = {};
    kuint8_t value = 0;
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
        return (AP3216C_DATA_MAX + 1);

    return value;
}

/*!
 * @brief  read ap3216c's ir register
 * @param  sptr_info
 * @retval ir's data
 * @note   none
 */
static kint32_t ap3216c_read_ir(struct ap3216c_drv_info *sptr_info)
{
    kuint16_t value[2];

    value[0] = ap3216c_read_value(sptr_info, AP3216C_REG_IR_LOW);
    if (value[0] > AP3216C_DATA_MAX)
        return -ER_RDATA_FAILD;
    
    if (AP3216C_IS_IR_VALID(value[0]))
        return -ER_INVALID;

    value[1] = ap3216c_read_value(sptr_info, AP3216C_REG_IR_HIGH);
    if (value[1] > AP3216C_DATA_MAX)
        return -ER_RDATA_FAILD;

    return AP3216C_IR_DATA(value[0], value[1]);
}

/*!
 * @brief  read ap3216c's als register
 * @param  sptr_info
 * @retval als's data
 * @note   none
 */
static kint32_t ap3216c_read_als(struct ap3216c_drv_info *sptr_info)
{
    kuint16_t value[2];

    value[0] = ap3216c_read_value(sptr_info, AP3216C_REG_ALS_LOW);
    if (value[0] > AP3216C_DATA_MAX)
        return -ER_RDATA_FAILD;

    value[1] = ap3216c_read_value(sptr_info, AP3216C_REG_ALS_HIGH);
    if (value[1] > AP3216C_DATA_MAX)
        return -ER_RDATA_FAILD;

    return AP3216C_ALS_DATA(value[0], value[1]);
}

/*!
 * @brief  read ap3216c's ps register
 * @param  sptr_info
 * @retval ps's data
 * @note   none
 */
static kint32_t ap3216c_read_ps(struct ap3216c_drv_info *sptr_info)
{
    kuint16_t value[2];

    value[0] = ap3216c_read_value(sptr_info, AP3216C_REG_PS_LOW);
    if (value[0] > AP3216C_DATA_MAX)
        return -ER_RDATA_FAILD;
    
    if (AP3216C_IS_PS_VALID(value[0]))
        return -ER_INVALID;

    value[1] = ap3216c_read_value(sptr_info, AP3216C_REG_PS_HIGH);
    if (value[1] > AP3216C_DATA_MAX)
        return -ER_RDATA_FAILD;

    if (AP3216C_IS_PS_VALID(value[1]))
        return -ER_INVALID;

    return AP3216C_PS_DATA(value[0], value[1]);
}

/*!
 * @brief  initialize ap3216c
 * @param  sptr_info
 * @retval errno
 * @note   none
 */
static kint32_t ap3216c_init(struct ap3216c_drv_info *sptr_info)
{
    kuint16_t value;

    /*!< reset ap3216c */
    value = ap3216c_write_value(sptr_info, AP3216C_REG_SYSCONFIG, AP3216C_RESET);
    if (value > AP3216C_DATA_MAX)
        return -ER_FAILD;

    /*!< wait for ap3216c reset finished */
	msleep(50);
    
    /*!< configure ap3216c, enable als, ps and ir */
	value = ap3216c_write_value(sptr_info, AP3216C_REG_SYSCONFIG, AP3216C_ALS_IR_PS_ACTIVE);
    if (value > AP3216C_DATA_MAX)
        return -ER_FAILD;

    /*!< verify configuration register*/
	value = ap3216c_read_value(sptr_info, AP3216C_REG_SYSCONFIG);
    if (value > AP3216C_DATA_MAX)
        return -ER_RDATA_FAILD;
    if (value != AP3216C_ALS_IR_PS_ACTIVE)
        return -ER_FAILD;

    return ER_NORMAL;
}

/*!
 * @brief  driver open
 * @param  sptr_inode, sptr_file
 * @retval errno
 * @note   none
 */
static kint32_t ap3216c_driver_open(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    struct ap3216c_drv_info *sptr_info;

    sptr_info = sptr_inode->sptr_cdev->privData;
    sptr_file->private_data = sptr_info;

    ap3216c_init(sptr_info);

    return ER_NORMAL;
}

/*!
 * @brief  driver close
 * @param  sptr_inode, sptr_file
 * @retval errno
 * @note   none
 */
static kint32_t ap3216c_driver_close(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    sptr_file->private_data = mr_nullptr;

    return ER_NORMAL;
}

/*!
 * @brief  driver read
 * @param  sptr_file, buffer, size
 * @retval size
 * @note   none
 */
static kssize_t ap3216c_driver_read(struct fwk_file *sptr_file, kbuffer_t *buffer, kssize_t size)
{
    struct ap3216c_drv_info *sptr_info;
    struct ap3216c_data sgtc_data;

    if (size < sizeof(sgtc_data))
        return -ER_RETRY;

    sptr_info = sptr_file->private_data;

    sgtc_data.ir = ap3216c_read_ir(sptr_info);
    if (sgtc_data.ir < 0)
        sgtc_data.ir = 0;

    sgtc_data.als = ap3216c_read_als(sptr_info);
    if (sgtc_data.ir < 0)
        sgtc_data.ir = 0;

    sgtc_data.ps = ap3216c_read_ps(sptr_info);
    if (sgtc_data.ir < 0)
        sgtc_data.ir = 0;

    fwk_copy_to_user(buffer, &sgtc_data, sizeof(sgtc_data));

    return sizeof(sgtc_data);
}

static const struct fwk_file_oprts sgtc_ap3216c_driver_oprts =
{
    .open = ap3216c_driver_open,
    .close = ap3216c_driver_close,
    .read = ap3216c_driver_read,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   ap3216c_driver_probe
 * @param   sptr_client
 * @retval  errno
 * @note    none
 */
static kint32_t ap3216c_driver_probe(struct fwk_i2c_client *sptr_client, const struct fwk_i2c_device_id *sptr_id)
{
    struct ap3216c_drv_info *sptr_info;
    struct fwk_device *sptr_idev;
    kint32_t devnum;

    sptr_info = kzalloc(sizeof(*sptr_info), GFP_KERNEL);
    if (!isValid(sptr_info))
        return -ER_NOMEM;

    devnum = MKE_DEV_NUM(AP3216C_DRVIVER_MAJOR, 0);
    sptr_info->devnum = devnum;
    sptr_info->name = "ap3216c";
    sptr_info->sptr_client = sptr_client;

    if (fwk_register_chrdev(devnum, 1, sptr_info->name))
        goto fail1;

    sptr_info->sptr_cdev = fwk_cdev_alloc(&sgtc_ap3216c_driver_oprts);
    if (!isValid(sptr_info->sptr_cdev))
        goto fail2;

    if (fwk_cdev_add(sptr_info->sptr_cdev, devnum, 1))
        goto fail3;

    sptr_idev = fwk_device_create(NR_TYPE_CHRDEV, devnum, "ap3216c");
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
 * @brief   ap3216c_driver_remove
 * @param   sptr_client
 * @retval  errno
 * @note    none
 */
static kint32_t ap3216c_driver_remove(struct fwk_i2c_client *sptr_client)
{
    struct ap3216c_drv_info *sptr_info;

    sptr_info = fwk_i2c_get_client_data(sptr_client);

    fwk_device_destroy(sptr_info->sptr_idev);
    fwk_cdev_del(sptr_info->sptr_cdev);
    kfree(sptr_info->sptr_cdev);
    fwk_unregister_chrdev(sptr_info->devnum, 0);
    kfree(sptr_info);
    fwk_i2c_set_client_data(sptr_client, mr_nullptr);

    return ER_NORMAL;
}

static const struct fwk_i2c_device_id sgtc_ap3216c_driver_ids[] =
{
    { .name = "ap3216c", .driver_data = -1 },
    {},
};

/*!< device id for device-tree */
static const struct fwk_of_device_id sgtc_ap3216c_driver_dts[] =
{
	{ .compatible = "liteon,ap3216c", },
	{},
};

/*!< platform instance */
static struct fwk_i2c_driver sgtc_ap3216c_driver =
{
	.probe	= ap3216c_driver_probe,
	.remove	= ap3216c_driver_remove,

    .sptr_id_table = sgtc_ap3216c_driver_ids,
	
	.sgtc_driver =
	{
		.name 	= "ap3216c,env-sensor",
		.id 	= -1,
		.sptr_of_match_table = sgtc_ap3216c_driver_dts,
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
	return fwk_i2c_add_driver(&sgtc_ap3216c_driver);
}

/*!
 * @brief   ap3216c_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit ap3216c_driver_exit(void)
{
	fwk_i2c_del_driver(&sgtc_ap3216c_driver);
}

IMPORT_DRIVER_INIT(ap3216c_driver_init);
IMPORT_DRIVER_EXIT(ap3216c_driver_exit);

/*!< end of file */
