/*
 * Platform Bus Interface of Hardware Abstraction Layer
 *
 * File Name:   fwk_platdrv.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.23
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_PLATDRV_H_
#define __FWK_PLATDRV_H_

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_platform.h>
#include <platform/fwk_platdev.h>

/*!< The defines */
struct fwk_id_table
{
	kstring_t *name;
	ksint32_t driver_data;
};

struct fwk_platdrv
{
	ksint32_t (*probe) 	(struct fwk_platdev *sprt_dev);
	ksint32_t (*remove) (struct fwk_platdev *sprt_dev);

	struct fwk_id_table *sprt_idTable;
	kusize_t num_idTable;

	struct fwk_driver sgrt_driver;
};

/*!< The functions */
TARGET_EXT ksint32_t fwk_register_platdriver(struct fwk_platdrv *sprt_platdrv);
TARGET_EXT ksint32_t fwk_unregister_platdriver(struct fwk_platdrv *sprt_platdrv);

/*!< API function */
/*!
 * @brief   fwk_platform_set_drvdata
 * @param   sprt_dev, ptrData
 * @retval  errno
 * @note    none
 */
static inline void fwk_platform_set_drvdata(struct fwk_platdev *sprt_dev, void *ptrData)
{
	sprt_dev->sgrt_dev.privData = ptrData;
}

/*!
 * @brief   fwk_platform_get_drvdata
 * @param   sprt_dev
 * @retval  privData
 * @note    none
 */
static inline void *fwk_platform_get_drvdata(struct fwk_platdev *sprt_dev)
{
	return sprt_dev->sgrt_dev.privData;
}

#endif /*!< __FWK_PLATDRV_H_ */
