/*
 * Platform Notifier Interface
 *
 * File Name:   fwk_notifier.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.04.05
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_NOTIFIER_H_
#define __FWK_NOTIFIER_H_

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <kernel/mutex.h>

/*!< The defines */
/*!< notifier chain head */
struct fwk_notifier_chain
{
    struct list_head sgtc_nbs;
    struct mutex_lock sgtc_lock;
};

#define BLOCKING_NOTIFIER_HEAD(name)    \
    struct fwk_notifier_chain name
#define BLOCKING_NOTIFIER_DECLARE(name) \
    extern struct fwk_notifier_chain name

struct fwk_notifier_block;
typedef kint32_t (*notifier_fn_t)(struct fwk_notifier_block *sptr_nb, kuint32_t event, void *args);

/*!< notifier chain iterm */
struct fwk_notifier_block
{
    notifier_fn_t notifier_call;
    notifier_fn_t pengding_call;
    kuint32_t expect_event;
    void *data;

    struct list_head sgtc_link;
};

/*!< The globals */

/*!< The functions */
extern kint32_t fwk_blocking_notifier_chain_register(
                        struct fwk_notifier_chain *sptr_chain, struct fwk_notifier_block *sptr_nb);
extern void fwk_blocking_notifier_chain_unregister(
                        struct fwk_notifier_chain *sptr_chain, struct fwk_notifier_block *sptr_nb);
extern kint32_t fwk_blocking_notifier_call_chain(
                        struct fwk_notifier_chain *sptr_chain, kuint32_t event, void *args);
extern kint32_t fwk_blocking_pengding_call_chain(
                        struct fwk_notifier_chain *sptr_chain, kuint32_t event, void *args);

/*!< API functions */
/*!
 * @brief   initial blocking_notifier_chain
 * @param   sptr_chain
 * @retval  none
 * @note    none
 */
static inline void fwk_blocking_notifier_chain_init(struct fwk_notifier_chain *sptr_chain)
{
    init_list_head(&sptr_chain->sgtc_nbs);
    mutex_init(&sptr_chain->sgtc_lock);
}

#ifdef __cplusplus
    }
#endif

#endif /* __FWK_NOTIFIER_H_ */
