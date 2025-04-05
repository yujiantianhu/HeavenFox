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
#include <platform/fwk_basic.h>
#include <kernel/mutex.h>

/*!< The defines */
/*!< notifier chain head */
struct fwk_notifier_chain
{
    struct list_head sgrt_nbs;
    struct mutex_lock sgrt_lock;
};

#define __NOTIFIER_CHAIN_INITIALIZE(n)   \
    {   \
        .sgrt_nbs = LIST_HEAD_INIT(&(n).sgrt_nbs),  \
        .sgrt_lock = MUTEX_LOCK_INIT(), \
    }

#define BLOCKING_NOTIFIER_HEAD(name)    \
    struct fwk_notifier_chain name = __NOTIFIER_CHAIN_INITIALIZE(name)
#define BLOCKING_NOTIFIER_DECLARE(name) \
    extern struct fwk_notifier_chain name

struct fwk_notifier_block;
typedef kint32_t (*notifier_fn_t)(struct fwk_notifier_block *sprt_nb, kuint32_t event, void *args);

/*!< notifier chain iterm */
struct fwk_notifier_block
{
    notifier_fn_t notifier_call;
    kuint32_t expect_event;
    void *data;

    struct list_head sgrt_link;
};

/*!< The globals */

/*!< The functions */
extern kint32_t fwk_blocking_notifier_chain_register(
                        struct fwk_notifier_chain *sprt_chain, struct fwk_notifier_block *sprt_nb);
extern void fwk_blocking_notifier_chain_unregister(
                        struct fwk_notifier_chain *sprt_chain, struct fwk_notifier_block *sprt_nb);
extern kint32_t fwk_blocking_notifier_call_chain(
                        struct fwk_notifier_chain *sprt_chain, kuint32_t event, void *args);

#ifdef __cplusplus
    }
#endif

#endif /* __FWK_NOTIFIER_H_ */
