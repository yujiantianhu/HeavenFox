/*
 * Hardware Abstraction Layer I2C Interface
 *
 * File Name:   fwk_i2c_core.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.16
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_I2C_CORE_H_
#define __FWK_I2C_CORE_H_

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/base/fwk_platform.h>
#include <kernel/rw_lock.h>

#include "fwk_i2c_algo.h"

/*!< The defines */
struct fwk_i2c_client;

typedef struct fwk_i2c_adapter
{
	kuint32_t id;
	const struct fwk_i2c_algo *sptr_algo; 				/*!< the algorithm to access the bus */
	void *algo_data;

	kint32_t timeout;		         	 				/*!< in jiffies */
	kint32_t retries;
	struct fwk_device sgtc_dev;		         			/*!< the adapter device */

	kint32_t nr;
	kchar_t name[48];

	struct list_head sgtc_clients;
	struct rw_lock sgtc_lock;

} srt_fwk_i2c_adapter_t;

/*!< The functions */
extern kint32_t fwk_i2c_transfer(struct fwk_i2c_client *sptr_client, struct fwk_i2c_msg *sptr_msgs, kint32_t num);
extern kint32_t fwk_i2c_check_slave(struct fwk_i2c_client *sptr_client);
extern kint32_t fwk_i2c_write_byte_data(struct fwk_i2c_client *sptr_client, kuint8_t reg, kuint8_t value);
extern kint32_t fwk_i2c_read_byte_data(struct fwk_i2c_client *sptr_client, kuint8_t reg);

/*!< API functions */
/*!
 * @brief   save driver data to i2c device
 * @param   sptr_adap, data
 * @retval  none
 * @note    none
 */
static inline void fwk_i2c_adapter_set_drvdata(struct fwk_i2c_adapter *sptr_adap, void *data)
{
	sptr_adap->sgtc_dev.privData = data;
}

/*!
 * @brief   get driver data from i2c device
 * @param   sptr_adap
 * @retval  none
 * @note    none
 */
static inline void *fwk_i2c_adapter_get_drvdata(struct fwk_i2c_adapter *sptr_adap)
{
	return sptr_adap->sgtc_dev.privData;
}

#ifdef __cplusplus
    }
#endif

#endif /*!< __FWK_I2C_CORE_H_ */
