/*
 * Character Device Interface
 *
 * File Name:   hal_chrdev.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.19
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_CHRDEV_H_
#define __HAL_CHRDEV_H_

/*!< The includes */
#include <platform/hal_basic.h>

/*!< The defines */
struct hal_char_device
{
	kuint32_t major;
	kuint32_t baseminor;
	kuint32_t count;
	ksint8_t name[DEVICE_NAME_LEN];

	/*!< All devices with the same primary device number form a list */
	struct hal_char_device *sprt_next;

};

/*!< The globals */
TARGET_EXT struct hal_char_device *sgrt_hal_chrdevs[];

/*!< The functions */
TARGET_EXT ksint32_t hal_alloc_chrdev(kuint32_t *devNum, kuint32_t baseminor, kuint32_t count, const kstring_t *name);
TARGET_EXT ksint32_t hal_register_chrdev(kuint32_t devNum, kuint32_t count, const kstring_t *name);
TARGET_EXT ksint32_t hal_unregister_chrdev(kuint32_t devNum, kuint32_t count);

#endif /*!< __HAL_CHRDEV_H_ */
