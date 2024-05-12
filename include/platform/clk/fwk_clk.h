/*
 * Hardware Abstraction Layer Gpio Interface
 *
 * File Name:   fwk_clk.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.03
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_CLK_H_
#define __FWK_CLK_H_

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_platform.h>
#include <platform/of/fwk_of.h>

/*!< The defines */
enum __ERT_FWK_CLK_FLAG
{
	NR_FWK_CLK_IS_DYNAMIC = mrt_bit(0),
};

typedef struct fwk_clk
{
	const kstring_t *dev_id;
	const kstring_t *con_id;
	kuint32_t max_rate;
	kuint32_t min_rate;

	struct fwk_clk_core *sprt_core;
	struct list_head sgrt_link;

	kuint8_t flags;

} srt_fwk_clk_t;

typedef struct fwk_clk_core
{
	const kstring_t	*name;
	const struct fwk_clk_ops *sprt_ops;
	struct fwk_clk_hw *sprt_hw;

	struct fwk_clk_core	*sprt_parent;

	const kstring_t	**parent_names;
	struct fwk_clk_core	**sprt_parents;

	kuint32_t rate;

	kuint32_t flags;
	kuint32_t enable_count;
	kuint32_t prepare_count;

	struct list_head sgrt_clks;

} srt_fwk_clk_core_t;

typedef struct fwk_clk_init_data 
{
	const kstring_t *name;
	const struct fwk_clk_ops *sprt_ops;

	const kstring_t **parent_names;
	kuint8_t num_parents;

	kuint32_t flags;

} srt_fwk_clk_init_data_t;

typedef struct fwk_clk_hw 
{
	struct fwk_clk_core *sprt_core;
	struct fwk_clk *sprt_clk;
	const struct fwk_clk_init_data *sprt_init;

} srt_fwk_clk_hw_t;

typedef struct fwk_clk_ops
{
	ksint32_t	(*prepare) (srt_fwk_clk_hw_t *sprt_hw);
	void		(*unprepare) (srt_fwk_clk_hw_t *sprt_hw);
	ksint32_t	(*is_prepared) (srt_fwk_clk_hw_t *sprt_hw);
	void		(*unprepare_unused) (srt_fwk_clk_hw_t *sprt_hw);
	ksint32_t	(*enable) (srt_fwk_clk_hw_t *sprt_hw);
	void		(*disable) (srt_fwk_clk_hw_t *sprt_hw);
	ksint32_t	(*is_enabled) (srt_fwk_clk_hw_t *sprt_hw);
	void		(*disable_unused) (srt_fwk_clk_hw_t *sprt_hw);

	ksint32_t	(*round_rate) (srt_fwk_clk_hw_t *sprt_hw, kuint32_t rate, kuint32_t *parent_rate);

	ksint32_t	(*set_parent) (srt_fwk_clk_hw_t *sprt_hw, kuint8_t index);
	kuint8_t	(*get_parent) (srt_fwk_clk_hw_t *sprt_hw);
	ksint32_t	(*set_rate) (srt_fwk_clk_hw_t *sprt_hw, kuint32_t rate, kuint32_t parent_rate);

	ksint32_t	(*get_phase) (srt_fwk_clk_hw_t *sprt_hw);
	ksint32_t	(*set_phase) (srt_fwk_clk_hw_t *sprt_hw, ksint32_t degrees);
	void		(*init) (srt_fwk_clk_hw_t *sprt_hw);

} srt_fwk_clk_ops_t;

/*!< The functions */
TARGET_EXT srt_fwk_clk_t *fwk_clk_register(srt_fwk_device_t *sprt_dev, srt_fwk_clk_hw_t *sprt_hw);
TARGET_EXT void fwk_clk_unregister(srt_fwk_clk_t *sprt_clk);

TARGET_EXT srt_fwk_clk_t *fwk_clk_config(srt_fwk_clk_t *sprt_clk, 
									srt_fwk_clk_hw_t *sprt_hw, const kstring_t *dev_id, const kstring_t *con_id);
TARGET_EXT srt_fwk_clk_t *fwk_create_clk(srt_fwk_clk_hw_t *sprt_hw, const kstring_t *dev_id, const kstring_t *con_id);
TARGET_EXT ksint32_t fwk_init_clk(srt_fwk_device_t *sprt_dev, srt_fwk_clk_t *sprt_clk);

TARGET_EXT srt_fwk_clk_t *fwk_clk_get(srt_fwk_device_t *sprt_dev, const kstring_t *name);
TARGET_EXT void fwk_clk_put(srt_fwk_clk_t *sprt_clk);
TARGET_EXT void fwk_clk_enable(srt_fwk_clk_t *sprt_clk);
TARGET_EXT void fwk_clk_disable(srt_fwk_clk_t *sprt_clk);
TARGET_EXT void fwk_clk_prepare(srt_fwk_clk_t *sprt_clk);
TARGET_EXT void fwk_clk_unprepare(srt_fwk_clk_t *sprt_clk);
TARGET_EXT kbool_t fwk_clk_is_enabled(srt_fwk_clk_t *sprt_clk);
TARGET_EXT void fwk_clk_prepare_enable(srt_fwk_clk_t *sprt_clk);
TARGET_EXT void fwk_clk_disable_unprepare(srt_fwk_clk_t *sprt_clk);

/*!< API functions */
static inline srt_fwk_clk_hw_t *fwk_clk_to_hw(srt_fwk_clk_t *sprt_clk)
{
    return sprt_clk ? sprt_clk->sprt_core->sprt_hw : mrt_nullptr;
}

#endif /*!< __FWK_CLK_H_ */
