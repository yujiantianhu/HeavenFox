/*
 * Hardware Abstraction Layer I2C Interface
 *
 * File Name:   fwk_i2c_dev.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.16
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_I2C_DEV_H_
#define __FWK_I2C_DEV_H_

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/base/fwk_platform.h>
#include <platform/of/fwk_of.h>
#include <platform/i2c/fwk_i2c_core.h>

/*!< The defines */
#define FWK_I2C_NAME_SIZE									(32)

struct fwk_i2c_device_id
{
	kchar_t *name;
	kint32_t driver_data;
};

struct fwk_i2c_board_info
{
	kchar_t type[FWK_I2C_NAME_SIZE];	     				/*!< device name */
	kuint16_t flags;				 						/*!< property */
	kuint16_t addr;											/*!< device slave address */
	void *ptr_platform_data;								/*!< device private data */

	struct fwk_device_node *sptr_node;
	kint32_t irq;											/*!< Interrupt number */
};

struct fwk_i2c_client
{
	kuint16_t flags;										/*!< div., see below */
	kuint16_t addr;											/*!< chip address - NOTE: 7bit */
															/*!< addresses are stored in the _LOWER_ 7 bits */
	kchar_t name[FWK_I2C_NAME_SIZE];
	struct fwk_i2c_adapter *sptr_adapter;					/*!< the adapter we sit on */
	struct fwk_i2c_driver *sptr_driver;						/*!< and our access routines */
	struct fwk_device sgtc_dev;		    					/*!< the device structure */
	kint32_t irq;			        						/*!< irq issued by device	*/
	struct list_head sgtc_detected;
	struct list_head sgtc_link;
};

struct fwk_i2c_driver
{
	kint32_t (*attach_adapter) (struct fwk_i2c_adapter *);
	kint32_t (*detach_adapter) (struct fwk_i2c_adapter *);

	/*!< Standard driver model interfaces */
	kint32_t (*probe) (struct fwk_i2c_client *, const struct fwk_i2c_device_id *);
	kint32_t (*remove) (struct fwk_i2c_client *);

	/*!< driver model interfaces that don't relate to enumeration  */
	void (*shutdown) (struct fwk_i2c_client *);
	kint32_t (*suspend) (struct fwk_i2c_client *, kint32_t mesg);
	kint32_t (*resume) (struct fwk_i2c_client *);
	kint32_t (*command) (struct fwk_i2c_client *client, kuint32_t cmd, void *ptr_arg);

	struct fwk_driver sgtc_driver;
	const struct fwk_i2c_device_id *sptr_id_table;

	/*!< Device detection callback for automatic device creation */
	kint32_t (*detect) (struct fwk_i2c_client *, struct fwk_i2c_board_info *);
	const kuint32_t *ptr_address_list;
	struct list_head sgtc_clients;
};

/*!< The functions */
kint32_t fwk_register_i2c_device(struct fwk_i2c_client *sptr_client);
kint32_t fwk_unregister_i2c_device(struct fwk_i2c_client *sptr_client);

struct fwk_i2c_client *fwk_i2c_new_device(struct fwk_i2c_adapter *sptr_adap, struct fwk_i2c_board_info const *sptr_info);
void fwk_i2c_unregister_device(struct fwk_i2c_client *sptr_client);
void fwk_of_i2c_register_devices(struct fwk_i2c_adapter *sptr_adap);

kint32_t fwk_i2c_register_adapter(struct fwk_i2c_adapter *sptr_adap);
kint32_t fwk_i2c_add_adapter(struct fwk_i2c_adapter *sptr_adap);
void fwk_i2c_del_adapter(struct fwk_i2c_adapter *sptr_adap);

kint32_t fwk_i2c_register_driver(struct fwk_i2c_driver *sptr_driver);
kint32_t fwk_i2c_unregister_driver(struct fwk_i2c_driver *sptr_driver);

/*!< API functions */
/*!
 * @brief   register i2c client
 * @param   sptr_client
 * @retval  errno
 * @note    none
 */
static inline kint32_t fwk_i2c_add_device(struct fwk_i2c_client *sptr_client)
{
	return fwk_register_i2c_device(sptr_client);
}

/*!
 * @brief   unregister i2c client
 * @param   sptr_client
 * @retval  errno
 * @note    none
 */
static inline kint32_t fwk_i2c_del_device(struct fwk_i2c_client *sptr_client)
{
	return fwk_unregister_i2c_device(sptr_client);
}

/*!
 * @brief   register i2c driver
 * @param   sptr_driver
 * @retval  errno
 * @note    none
 */
static inline kint32_t fwk_i2c_add_driver(struct fwk_i2c_driver *sptr_driver)
{
	return fwk_i2c_register_driver(sptr_driver);
}

/*!
 * @brief   unregister i2c driver
 * @param   sptr_driver
 * @retval  errno
 * @note    none
 */
static inline kint32_t fwk_i2c_del_driver(struct fwk_i2c_driver *sptr_driver)
{
	return fwk_i2c_unregister_driver(sptr_driver);
}

/*!
 * @brief   save driver data to i2c device
 * @param   sptr_adap, data
 * @retval  none
 * @note    none
 */
static inline void fwk_i2c_set_client_data(struct fwk_i2c_client *sptr_client, void *data)
{
	sptr_client->sgtc_dev.privData = data;
}

/*!
 * @brief   get driver data from i2c device
 * @param   sptr_adap
 * @retval  none
 * @note    none
 */
static inline void *fwk_i2c_get_client_data(struct fwk_i2c_client *sptr_client)
{
	return sptr_client->sgtc_dev.privData;
}

#ifdef __cplusplus
    }
#endif

#endif /*!< __FWK_I2C_DEV_H_ */
