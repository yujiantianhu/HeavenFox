/*
 * Hardware Abstraction Layer Clock Interface
 *
 * File Name:   fwk_clk_provider.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.06
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_CLK_PROVIDER_H_
#define __FWK_CLK_PROVIDER_H_

/*!< The includes */
#include <platform/fwk_basic.h>
#include "fwk_clk.h"

/*!< The defines */
typedef struct fwk_clk_one_cell
{
	struct fwk_clk *sprt_clks;
	kuint32_t clks_size;

} srt_fwk_clk_one_cell_t;

/*!< The functions */
TARGET_EXT srt_fwk_clk_t *fwk_of_clk_src_onecell_get(srt_fwk_of_phandle_args_t *sprt_args, void *data);
TARGET_EXT ksint32_t fwk_clk_add_provider(srt_fwk_device_node_t *sprt_node, 
                    srt_fwk_clk_t *(*get)(srt_fwk_of_phandle_args_t *, void *), void *data);
TARGET_EXT void fwk_clk_del_provider(srt_fwk_device_node_t *sprt_node);
TARGET_EXT srt_fwk_clk_t *fwk_clk_provider_look_up(srt_fwk_of_phandle_args_t *sprt_args);


#endif /*!< __FWK_CLK_PROVIDER_H_ */
