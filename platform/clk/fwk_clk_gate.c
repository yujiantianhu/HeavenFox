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
/*!
 * @brief   create and register a new clk gate
 * @param   sptr_clk, sptr_ops, ...
 * @retval  sptr_clk
 * @note    if sptr_clk is not NULL, it will use sptr_clk and does not create new sptr_clk (sptr_rc = sptr_clk);
 *          otherwise, a new sptr_rc will be allocated
 */
struct fwk_clk *fwk_clk_gate_register(struct fwk_clk *sptr_clk, const struct fwk_clk_ops *sptr_ops,
                                const kchar_t *name, const kchar_t *parent, void *reg, kuint8_t shift)
{
    struct fwk_clk_gate *sptr_gate;
    struct fwk_clk_init_data sgtc_init;
    struct fwk_clk *sptr_rc;

    sptr_gate = kzalloc(sizeof(*sptr_gate), GFP_KERNEL);
    if (!isValid(sptr_gate))
        return mr_nullptr;

    sgtc_init.name = name;
    sgtc_init.parent_names = &parent;
    sgtc_init.num_parents = 1;
    sgtc_init.sptr_ops = sptr_ops;

    sptr_gate->reg = reg;
    sptr_gate->bit_idx = shift;
    sptr_gate->flags = 0;
    sptr_gate->sgtc_hw.sptr_init = (const struct fwk_clk_init_data *)&sgtc_init;
    sptr_gate->sgtc_hw.sptr_clk = sptr_clk;

    /*!< register clk */
    sptr_rc = fwk_clk_register(mr_nullptr, &sptr_gate->sgtc_hw);
    if (!isValid(sptr_rc))
        kfree(sptr_gate);

    return sptr_rc;
}

/*!
 * @brief   unregister and destroy clk gate
 * @param   sptr_clk
 * @retval  none
 * @note    none
 */
void fwk_clk_gate_unregister(struct fwk_clk *sptr_clk)
{
    struct fwk_clk_hw *sptr_hw;
    struct fwk_clk_gate *sptr_gate;

    sptr_hw = fwk_clk_to_hw(sptr_clk);
    sptr_gate = mr_container_of(sptr_hw, typeof(*sptr_gate), sgtc_hw);

    fwk_clk_unregister(sptr_clk);
    kfree(sptr_gate);
}

/*!< end of file */
