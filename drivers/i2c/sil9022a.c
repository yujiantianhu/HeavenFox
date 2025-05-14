/*
 * Template of HDMI Phy Driver : sil9022a
 *
 * File Name:   sil9022a.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.04.05
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
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
#include <platform/notifier/fwk_notifier.h>
#include <platform/video/fwk_fbmem.h>

/*!< The defines */
/*!< Register */
#define SIL9022A_CHIPID                             (0x00)
#define SIL9022A_SYSTEM                             (0x08)
#define SIL9022A_VIDFMT                             (0x09)
#define SIL9022A_INPUT                              (0x0A)
#define SIL9022A_TMDS                               (0x0B)

typedef struct sil9022a_drv_info
{
    kchar_t *name;
    struct fwk_i2c_client *sptr_client;

    struct fwk_device sgtc_dev;
    struct fwk_notifier_block sgtc_nb;

} sil9022a_drv_info_t;

/*!< API function */
/*!
 * @brief  sil9022a_write_value
 * @param  none
 * @retval none
 * @note   write data by i2c
 */
static kuint16_t sil9022a_write_value(struct sil9022a_drv_info *sptr_drv, kuint8_t reg, kuint8_t value)
{
    struct fwk_i2c_msg sgtc_msgs = {};
    kuint8_t buf[2] = { reg, value };
    kint32_t retval;

    sgtc_msgs.addr = sptr_drv->sptr_client->addr;
    sgtc_msgs.flags = 0;
    sgtc_msgs.ptr_buf = buf;
    sgtc_msgs.len = sizeof(buf);

    retval = fwk_i2c_transfer(sptr_drv->sptr_client, &sgtc_msgs, 1);
    if (retval)
        return (0xff + 1);

    return value;
}

/*!
 * @brief  sil9022a_read_value
 * @param  none
 * @retval none
 * @note   read data by i2c
 */
static kuint16_t sil9022a_read_value(struct sil9022a_drv_info *sptr_drv, kuint8_t reg)
{
    struct fwk_i2c_msg sgtc_msgs[2] = {};
    kuint8_t value = 0;
    kint32_t retval;

    sgtc_msgs[0].addr = sptr_drv->sptr_client->addr;
    sgtc_msgs[0].flags = 0;
    sgtc_msgs[0].ptr_buf = &reg;
    sgtc_msgs[0].len = 1;

    sgtc_msgs[1].addr = sptr_drv->sptr_client->addr;
    sgtc_msgs[1].flags |= FWK_I2C_M_RD;
    sgtc_msgs[1].ptr_buf = &value;
    sgtc_msgs[1].len = sizeof(value);

    retval = fwk_i2c_transfer(sptr_drv->sptr_client, &sgtc_msgs[0], ARRAY_SIZE(sgtc_msgs));
    if (retval)
        return (0xff + 1);

    return value;
}

/*!
 * @brief  initialize sil9022a
 * @param  sptr_drv
 * @retval errno
 * @note   none
 */
static kint32_t sil9022a_init(struct sil9022a_drv_info *sptr_drv, struct fwk_fb_notifier_param *sptr_param)
{
    struct fwk_fb_var_screen_info *sptr_var;

    /*!< Read ID */
    if (sil9022a_read_value(sptr_drv, SIL9022A_CHIPID) != 0x90)
        return -ER_IOERR;

    sptr_var = sptr_param->sptr_var;

    /*!< Configuration */
    /*!< Enable all modules */
    sil9022a_write_value(sptr_drv, SIL9022A_SYSTEM, 0x1f);

    if ((sptr_var->xres == 1920) &&
        (sptr_var->yres == 1080))
    {
        /*!< 1080@60Hz */
        sil9022a_write_value(sptr_drv, SIL9022A_VIDFMT, 0x03);
    }

    if ((sptr_var->bits_per_pixel == 24) ||
        (sptr_var->bits_per_pixel == 32))
    {
        /*!< RGB888 */
        sil9022a_write_value(sptr_drv, SIL9022A_INPUT, 0x00);
    }

    /*!< Enable all TMDS channels */
    sil9022a_write_value(sptr_drv, SIL9022A_TMDS, 0x03);

    return ER_NORMAL;
}

/*!
 * @brief   notifier callback
 * @param   sptr_nb
 * @retval  errno
 * @note    none
 */
kint32_t sil9022a_hdmi_action(struct fwk_notifier_block *sptr_nb, kuint32_t event, void *args)
{
    struct sil9022a_drv_info *sptr_drv;
    struct fwk_fb_notifier_param *sptr_param;

    sptr_drv = (struct sil9022a_drv_info *)sptr_nb->data;
    sptr_param = (struct fwk_fb_notifier_param *)args;

    switch (event)
    {
        case FB_NOTIFIER_HDMI_OPEN:
            sil9022a_init(sptr_drv, sptr_param);
            break;

        case FB_NOTIFIER_HDMI_CLOSE:
            break;

        default: break;
    }

    return ER_NORMAL;
}

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   sil9022a_driver_probe
 * @param   sptr_client
 * @retval  errno
 * @note    none
 */
static kint32_t sil9022a_driver_probe(struct fwk_i2c_client *sptr_client, const struct fwk_i2c_device_id *sptr_id)
{
    struct sil9022a_drv_info *sptr_drv;

    sptr_drv = kzalloc(sizeof(*sptr_drv), GFP_KERNEL);
    if (!isValid(sptr_drv))
        return -ER_NOMEM;

    sptr_drv->name = "sil9022a";
    sptr_drv->sptr_client = sptr_client;

    fwk_device_initial(&sptr_drv->sgtc_dev);
    sptr_drv->sgtc_dev.sptr_parent = &sptr_client->sgtc_dev;
    mr_dev_set_name(&sptr_drv->sgtc_dev, sptr_drv->name);
    fwk_device_add(&sptr_drv->sgtc_dev);

    fwk_i2c_set_client_data(sptr_client, sptr_drv);

    /*!< Configure Notifier Chain */
    sptr_drv->sgtc_nb.data = sptr_drv;
    sptr_drv->sgtc_nb.notifier_call = sil9022a_hdmi_action;
    sptr_drv->sgtc_nb.expect_event = FB_NOTIFIER_HDMI_OPEN | FB_NOTIFIER_HDMI_CLOSE;
    init_list_head(&sptr_drv->sgtc_nb.sgtc_link);
    fwk_blocking_notifier_chain_register(&sgtc_fbmem_notifier_chain, &sptr_drv->sgtc_nb);

	return ER_NORMAL;
}

/*!
 * @brief   sil9022a_driver_remove
 * @param   sptr_client
 * @retval  errno
 * @note    none
 */
static kint32_t sil9022a_driver_remove(struct fwk_i2c_client *sptr_client)
{
    struct sil9022a_drv_info *sptr_drv;

    sptr_drv = fwk_i2c_get_client_data(sptr_client);

    fwk_blocking_notifier_chain_unregister(&sgtc_fbmem_notifier_chain, &sptr_drv->sgtc_nb);
    kfree(sptr_drv);
    fwk_i2c_set_client_data(sptr_client, mr_nullptr);

    return ER_NORMAL;
}

static const struct fwk_i2c_device_id sgtc_sil9022a_driver_ids[] =
{
    { .name = "sil9022a", .driver_data = -1 },
    {},
};

/*!< device id for device-tree */
static const struct fwk_of_device_id sgtc_sil9022a_driver_dts[] =
{
	{ .compatible = "silicon,sil9022a", },
	{},
};

/*!< platform instance */
static struct fwk_i2c_driver sgtc_sil9022a_driver =
{
	.probe	= sil9022a_driver_probe,
	.remove	= sil9022a_driver_remove,

    .sptr_id_table = sgtc_sil9022a_driver_ids,
	
	.sgtc_driver =
	{
		.name 	= "sil9022a,hdmi-phy",
		.id 	= -1,
		.sptr_of_match_table = sgtc_sil9022a_driver_dts,
	},
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   sil9022a_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init sil9022a_driver_init(void)
{
	return fwk_i2c_add_driver(&sgtc_sil9022a_driver);
}

/*!
 * @brief   sil9022a_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit sil9022a_driver_exit(void)
{
	fwk_i2c_del_driver(&sgtc_sil9022a_driver);
}

IMPORT_DRIVER_INIT(sil9022a_driver_init);
IMPORT_DRIVER_EXIT(sil9022a_driver_exit);

/*!< end of file */
