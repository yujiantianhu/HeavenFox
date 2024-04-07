/*
 * Platform Object Defines
 *
 * File Name:   hal_kboj.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.18
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_KOBJ_H_
#define __HAL_KOBJ_H_

/*!< The includes */
#include <platform/hal_basic.h>

/*!< The defines */
struct hal_probes
{
	kuint32_t devNum;
	kuint32_t range;

	void *data;
	struct hal_probes *sprt_next;
};

struct hal_kobj_map
{
	struct hal_probes *sprt_probes[DEVICE_MAX_NUM];
};

/*!< The globals */
TARGET_EXT struct hal_kobj_map *sprt_hal_chrdev_map;
TARGET_EXT struct hal_kobj_map *sprt_hal_blkdev_map;
TARGET_EXT struct hal_kobj_map *sprt_hal_netdev_map;

/*!< The functions */
TARGET_EXT ksint32_t hal_kobj_init(void);
TARGET_EXT void hal_kobj_del(void);
TARGET_EXT ksint32_t hal_kobj_map(struct hal_kobj_map *domain, kuint32_t devNum, kuint32_t range, void *data);
TARGET_EXT ksint32_t hal_kobj_unmap(struct hal_kobj_map *domain, kuint32_t devNum, kuint32_t range);
TARGET_EXT void *hal_kobj_lookUp(struct hal_kobj_map *domain, kuint32_t devNum);

#endif /*!< __HAL_KOBJ_H_ */
