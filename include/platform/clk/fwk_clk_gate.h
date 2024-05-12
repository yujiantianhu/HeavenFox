/*
 * Hardware Abstraction Layer Gpio Interface
 *
 * File Name:   fwk_clk_gate.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.03
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_CLK_GATE_H_
#define __FWK_CLK_GATE_H_

/*!< The includes */
#include <platform/fwk_basic.h>
#include "fwk_clk.h"

/*!< The defines */
typedef struct fwk_clk_gate 
{
	struct fwk_clk_hw sgrt_hw;
	void *reg;
	kuint8_t bit_idx;
	kuint8_t flags;

} srt_fwk_clk_gate_t;

/*!< The functions */
TARGET_EXT srt_fwk_clk_t *fwk_clk_gate_register(srt_fwk_clk_t *sprt_clk, const srt_fwk_clk_ops_t *sprt_ops,
                                const kstring_t *name, const kstring_t *parent, void *reg, kuint8_t shift);
TARGET_EXT void fwk_clk_gate_unregister(srt_fwk_clk_t *sprt_clk);

#endif /*!< __FWK_CLK_GATE_H_ */
