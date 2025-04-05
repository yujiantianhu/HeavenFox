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
    struct fwk_i2c_client *sprt_client;

    struct fwk_device sgrt_dev;
    struct fwk_notifier_block sgrt_nb;

} sil9022a_drv_info_t;

/*!< API function */
/*!
 * @brief  sil9022a_write_value
 * @param  none
 * @retval none
 * @note   write data by i2c
 */
static kuint16_t sil9022a_write_value(struct sil9022a_drv_info *sprt_drv, kuint8_t reg, kuint8_t value)
{
    struct fwk_i2c_msg sgrt_msgs;
    kuint8_t buf[2] = { reg, value };
    kint32_t retval;

    sgrt_msgs.addr = sprt_drv->sprt_client->addr;
    sgrt_msgs.flags = 0;
    sgrt_msgs.ptr_buf = buf;
    sgrt_msgs.len = sizeof(buf);

    retval = fwk_i2c_transfer(sprt_drv->sprt_client, &sgrt_msgs, 1);
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
static kuint16_t sil9022a_read_value(struct sil9022a_drv_info *sprt_drv, kuint8_t reg)
{
    struct fwk_i2c_msg sgrt_msgs[2];
    kuint8_t value = 0;
    kint32_t retval;

    sgrt_msgs[0].addr = sprt_drv->sprt_client->addr;
    sgrt_msgs[0].flags = 0;
    sgrt_msgs[0].ptr_buf = &reg;
    sgrt_msgs[0].len = 1;

    sgrt_msgs[1].addr = sprt_drv->sprt_client->addr;
    sgrt_msgs[1].flags |= FWK_I2C_M_RD;
    sgrt_msgs[1].ptr_buf = &value;
    sgrt_msgs[1].len = sizeof(value);

    retval = fwk_i2c_transfer(sprt_drv->sprt_client, &sgrt_msgs[0], ARRAY_SIZE(sgrt_msgs));
    if (retval)
        return (0xff + 1);

    return value;
}

/*!
 * @brief  initialize sil9022a
 * @param  sprt_drv
 * @retval errno
 * @note   none
 */
static kint32_t sil9022a_init(struct sil9022a_drv_info *sprt_drv, struct fwk_fb_notifier_param *sprt_param)
{
    struct fwk_fb_var_screen_info *sprt_var;

    /*!< Read ID */
    if (sil9022a_read_value(sprt_drv, SIL9022A_CHIPID) != 0x90)
        return -ER_IOERR;

    sprt_var = sprt_param->sprt_var;

    /*!< Configuration */
    /*!< Enable all modules */
    sil9022a_write_value(sprt_drv, SIL9022A_SYSTEM, 0x1f);

    if ((sprt_var->xres == 1920) &&
        (sprt_var->yres == 1080))
    {
        /*!< 1080@60Hz */
        sil9022a_write_value(sprt_drv, SIL9022A_VIDFMT, 0x03);
    }

    if ((sprt_var->bits_per_pixel == 24) ||
        (sprt_var->bits_per_pixel == 32))
    {
        /*!< RGB888 */
        sil9022a_write_value(sprt_drv, SIL9022A_INPUT, 0x00);
    }

    /*!< Enable all TMDS channels */
    sil9022a_write_value(sprt_drv, SIL9022A_TMDS, 0x03);

    return ER_NORMAL;
}

/*!
 * @brief   notifier callback
 * @param   sprt_nb
 * @retval  errno
 * @note    none
 */
kint32_t sil9022a_hdmi_action(struct fwk_notifier_block *sprt_nb, kuint32_t event, void *args)
{
    struct sil9022a_drv_info *sprt_drv;
    struct fwk_fb_notifier_param *sprt_param;

    sprt_drv = (struct sil9022a_drv_info *)sprt_nb->data;
    sprt_param = (struct fwk_fb_notifier_param *)args;

    switch (event)
    {
        case FB_NOTIFIER_HDMI_OPEN:
            sil9022a_init(sprt_drv, sprt_param);
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
 * @param   sprt_client
 * @retval  errno
 * @note    none
 */
static kint32_t sil9022a_driver_probe(struct fwk_i2c_client *sprt_client, const struct fwk_i2c_device_id *sprt_id)
{
    struct sil9022a_drv_info *sprt_drv;

    sprt_drv = kzalloc(sizeof(*sprt_drv), GFP_KERNEL);
    if (!isValid(sprt_drv))
        return -ER_NOMEM;

    sprt_drv->name = "sil9022a";
    sprt_drv->sprt_client = sprt_client;

    fwk_device_initial(&sprt_drv->sgrt_dev);
    sprt_drv->sgrt_dev.sprt_parent = &sprt_client->sgrt_dev;
    mrt_dev_set_name(&sprt_drv->sgrt_dev, sprt_drv->name);
    fwk_device_add(&sprt_drv->sgrt_dev);

    fwk_i2c_set_client_data(sprt_client, sprt_drv);

    /*!< Configure Notifier Chain */
    sprt_drv->sgrt_nb.data = sprt_drv;
    sprt_drv->sgrt_nb.notifier_call = sil9022a_hdmi_action;
    sprt_drv->sgrt_nb.expect_event = FB_NOTIFIER_HDMI_OPEN | FB_NOTIFIER_HDMI_CLOSE;
    init_list_head(&sprt_drv->sgrt_nb.sgrt_link);
    fwk_blocking_notifier_chain_register(&sgrt_fbmem_notifier_chain, &sprt_drv->sgrt_nb);

	return ER_NORMAL;
}

/*!
 * @brief   sil9022a_driver_remove
 * @param   sprt_client
 * @retval  errno
 * @note    none
 */
static kint32_t sil9022a_driver_remove(struct fwk_i2c_client *sprt_client)
{
    struct sil9022a_drv_info *sprt_drv;

    sprt_drv = fwk_i2c_get_client_data(sprt_client);

    fwk_blocking_notifier_chain_unregister(&sgrt_fbmem_notifier_chain, &sprt_drv->sgrt_nb);
    kfree(sprt_drv);
    fwk_i2c_set_client_data(sprt_client, mrt_nullptr);

    return ER_NORMAL;
}

static const struct fwk_i2c_device_id sgrt_sil9022a_driver_ids[] =
{
    { .name = "sil9022a", .driver_data = -1 },
    {},
};

/*!< device id for device-tree */
static const struct fwk_of_device_id sgrt_sil9022a_driver_dts[] =
{
	{ .compatible = "silicon,sil9022a", },
	{},
};

/*!< platform instance */
static struct fwk_i2c_driver sgrt_sil9022a_driver =
{
	.probe	= sil9022a_driver_probe,
	.remove	= sil9022a_driver_remove,

    .sprt_id_table = sgrt_sil9022a_driver_ids,
	
	.sgrt_driver =
	{
		.name 	= "sil9022a,hdmi-phy",
		.id 	= -1,
		.sprt_of_match_table = sgrt_sil9022a_driver_dts,
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
	return fwk_i2c_add_driver(&sgrt_sil9022a_driver);
}

/*!
 * @brief   sil9022a_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit sil9022a_driver_exit(void)
{
	fwk_i2c_del_driver(&sgrt_sil9022a_driver);
}

IMPORT_DRIVER_INIT(sil9022a_driver_init);
IMPORT_DRIVER_EXIT(sil9022a_driver_exit);

/*!< end of file */
