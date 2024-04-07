/*
 * Platform Bus Interface of Hardware Abstraction Layer
 *
 * File Name:   hal_platdrv.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.23
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_PLATDRV_H_
#define __HAL_PLATDRV_H_

/*!< The includes */
#include <platform/hal_basic.h>
#include <platform/hal_platform.h>
#include <platform/hal_platdev.h>

/*!< The defines */
struct hal_id_table
{
	kstring_t *name;
	ksint32_t driver_data;
};

struct hal_platdrv
{
	ksint32_t (*probe) 	(struct hal_platdev *sprt_dev);
	ksint32_t (*remove) (struct hal_platdev *sprt_dev);

	struct hal_id_table *sprt_idTable;
	kusize_t num_idTable;

	struct hal_driver sgrt_driver;
};

/*!< The functions */
TARGET_EXT ksint32_t hal_register_platdriver(struct hal_platdrv *sprt_platdrv);
TARGET_EXT ksint32_t hal_unregister_platdriver(struct hal_platdrv *sprt_platdrv);

/*!< API function */
/*!
 * @brief   hal_platform_set_drvdata
 * @param   sprt_device, ptrData
 * @retval  errno
 * @note    none
 */
static inline void hal_platform_set_drvdata(struct hal_platdev *sprt_dev, void *ptrData)
{
	sprt_dev->sgrt_device.privData = ptrData;
}

/*!
 * @brief   hal_platform_get_drvdata
 * @param   sprt_device
 * @retval  privData
 * @note    none
 */
static inline void *hal_platform_get_drvdata(struct hal_platdev *sprt_dev)
{
	return sprt_dev->sgrt_device.privData;
}

#endif /*!< __HAL_PLATDRV_H_ */
