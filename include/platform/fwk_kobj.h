/*
 * Platform Object Defines
 *
 * File Name:   fwk_kboj.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.18
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_KOBJ_H_
#define __FWK_KOBJ_H_

/*!< The includes */
#include <platform/fwk_basic.h>

/*!< The defines */
struct fwk_probes
{
	kuint32_t devNum;
	kuint32_t range;

	void *data;
	struct fwk_probes *sprt_next;
};

struct fwk_kobj_map
{
	struct fwk_probes *sprt_probes[DEVICE_MAX_NUM];
};

/*!< The globals */
TARGET_EXT struct fwk_kobj_map *sprt_fwk_chrdev_map;
TARGET_EXT struct fwk_kobj_map *sprt_fwk_blkdev_map;
TARGET_EXT struct fwk_kobj_map *sprt_fwk_netdev_map;

/*!< The functions */
TARGET_EXT ksint32_t fwk_kobj_init(void);
TARGET_EXT void fwk_kobj_del(void);
TARGET_EXT ksint32_t fwk_kobj_map(struct fwk_kobj_map *domain, kuint32_t devNum, kuint32_t range, void *data);
TARGET_EXT ksint32_t fwk_kobj_unmap(struct fwk_kobj_map *domain, kuint32_t devNum, kuint32_t range);
TARGET_EXT void *fwk_kobj_lookUp(struct fwk_kobj_map *domain, kuint32_t devNum);

#endif /*!< __FWK_KOBJ_H_ */
