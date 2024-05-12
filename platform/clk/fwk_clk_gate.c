/*
 * Hardware Abstraction Layer Clock Interface
 *
 * File Name:   fwk_clk_gate.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.03
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/of/fwk_of.h>
#include <platform/clk/fwk_clk.h>
#include <platform/clk/fwk_clk_gate.h>

/*!< API function */
srt_fwk_clk_t *fwk_clk_gate_register(srt_fwk_clk_t *sprt_clk, const srt_fwk_clk_ops_t *sprt_ops,
                                const kstring_t *name, const kstring_t *parent, void *reg, kuint8_t shift)
{
    srt_fwk_clk_gate_t *sprt_gate;
    srt_fwk_clk_init_data_t sgrt_init;
    srt_fwk_clk_t *sprt_rc;

    sprt_gate = kzalloc(sizeof(*sprt_gate), GFP_KERNEL);
    if (!isValid(sprt_gate))
        return mrt_nullptr;

    sgrt_init.name = name;
    sgrt_init.parent_names = &parent;
    sgrt_init.num_parents = 1;
    sgrt_init.sprt_ops = sprt_ops;

    sprt_gate->reg = reg;
    sprt_gate->bit_idx = shift;
    sprt_gate->flags = 0;
    sprt_gate->sgrt_hw.sprt_init = (const srt_fwk_clk_init_data_t *)&sgrt_init;
    sprt_gate->sgrt_hw.sprt_clk = sprt_clk;

    /*!< register clk */
    sprt_rc = fwk_clk_register(mrt_nullptr, &sprt_gate->sgrt_hw);
    if (!isValid(sprt_rc))
        kfree(sprt_gate);

    return sprt_rc;
}

void fwk_clk_gate_unregister(srt_fwk_clk_t *sprt_clk)
{
    srt_fwk_clk_hw_t *sprt_hw;
    srt_fwk_clk_gate_t *sprt_gate;

    sprt_hw = fwk_clk_to_hw(sprt_clk);
    sprt_gate = mrt_container_of(sprt_hw, typeof(*sprt_gate), sgrt_hw);

    fwk_clk_unregister(sprt_clk);
    kfree(sprt_gate);
}

/*!< end of file */
