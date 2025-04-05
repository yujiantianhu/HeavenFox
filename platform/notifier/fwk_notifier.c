/*
 * Platform Notifier Interface
 *
 * File Name:   fwk_notifier.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.04.05
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/notifier/fwk_notifier.h>

/*!< API functions */
/*!
 * @brief   register notifier
 * @param   sprt_chain
 * @param   sprt_nb
 * @retval  errno
 * @note    none
 */
kint32_t fwk_blocking_notifier_chain_register(struct fwk_notifier_chain *sprt_chain, struct fwk_notifier_block *sprt_nb)
{
    if (!sprt_chain || !sprt_nb)
        return -ER_NULLPTR;

    if (!mrt_list_head_empty(&sprt_nb->sgrt_link))
        return -ER_EXISTED;

    if (!sprt_nb->notifier_call)
        return -ER_UNVALID;

    mutex_lock(&sprt_chain->sgrt_lock);
    list_head_add_tail(&sprt_chain->sgrt_nbs, &sprt_nb->sgrt_link);
    mutex_unlock(&sprt_chain->sgrt_lock);

    return ER_NORMAL;
}

/*!
 * @brief   unregister notifier
 * @param   sprt_chain
 * @param   sprt_nb
 * @retval  errno
 * @note    none
 */
void fwk_blocking_notifier_chain_unregister(struct fwk_notifier_chain *sprt_chain, struct fwk_notifier_block *sprt_nb)
{
    if (!sprt_nb ||
        mrt_list_head_empty(&sprt_nb->sgrt_link))
        return;

    mutex_lock(&sprt_chain->sgrt_lock);
    list_head_del(&sprt_nb->sgrt_link);
    mutex_unlock(&sprt_chain->sgrt_lock);
}

/*!
 * @brief   call notifier
 * @param   sprt_chain
 * @param   event, args
 * @retval  errno
 * @note    none
 */
kint32_t fwk_blocking_notifier_call_chain(struct fwk_notifier_chain *sprt_chain, kuint32_t event, void *args)
{
    struct fwk_notifier_block *sprt_nb;

    if (!sprt_chain ||
        mrt_list_head_empty(&sprt_chain->sgrt_nbs))
        return -ER_PERMIT;

    mutex_lock(&sprt_chain->sgrt_lock);

    foreach_list_next_entry(sprt_nb, &sprt_chain->sgrt_nbs, sgrt_link)
    {
        if (sprt_nb->expect_event & event)
            sprt_nb->notifier_call(sprt_nb, event, args);
    }

    mutex_unlock(&sprt_chain->sgrt_lock);

    return ER_NORMAL;
}

/* end of file */