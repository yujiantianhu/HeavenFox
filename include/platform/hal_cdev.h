/*
 * Character Device Interface
 *
 * File Name:   hal_cdev.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.18
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_CDEV_H_
#define __HAL_CDEV_H_

/*!< The includes */
#include <platform/hal_basic.h>
#include <platform/hal_fs.h>
#include <platform/hal_kobj.h>

/*!< The defines */
struct hal_cdev
{
	kuint32_t devNum;
	kuint32_t count;

	struct hal_file_oprts *sprt_oprts;
	struct hal_cdev *sprt_next;

	void *privData;
};

/*!< The functions */
TARGET_EXT ksint32_t hal_cdev_init(struct hal_cdev *sprt_cdev, const struct hal_file_oprts *sprt_oprts);
TARGET_EXT struct hal_cdev *hal_cdev_alloc(const struct hal_file_oprts *sprt_oprts);
TARGET_EXT ksint32_t hal_cdev_add(struct hal_cdev *sprt_cdev, kuint32_t devNum, kuint32_t count);
TARGET_EXT ksint32_t hal_cdev_del(struct hal_cdev *sprt_cdev);

#endif /*!< __HAL_CDEV_H_ */
