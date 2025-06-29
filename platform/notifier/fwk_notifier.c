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
 * @param   sptr_chain
 * @param   sptr_nb
 * @retval  errno
 * @note    none
 */
kint32_t fwk_blocking_notifier_chain_register(struct fwk_notifier_chain *sptr_chain, struct fwk_notifier_block *sptr_nb)
{
    if (!sptr_chain || !sptr_nb)
        return -ER_NULLPTR;

    if (!mr_list_empty(&sptr_nb->sgtc_link))
        return -ER_EXISTED;

    if (!sptr_nb->notifier_call)
        return -ER_INVALID;

    mutex_lock(&sptr_chain->sgtc_lock);
    list_head_add_tail(&sptr_chain->sgtc_nbs, &sptr_nb->sgtc_link);
    mutex_unlock(&sptr_chain->sgtc_lock);

    return ER_NORMAL;
}

/*!
 * @brief   unregister notifier
 * @param   sptr_chain
 * @param   sptr_nb
 * @retval  errno
 * @note    none
 */
void fwk_blocking_notifier_chain_unregister(struct fwk_notifier_chain *sptr_chain, struct fwk_notifier_block *sptr_nb)
{
    if (!sptr_nb ||
        mr_list_empty(&sptr_nb->sgtc_link))
        return;

    mutex_lock(&sptr_chain->sgtc_lock);
    list_head_del(&sptr_nb->sgtc_link);
    mutex_unlock(&sptr_chain->sgtc_lock);
}

/*!
 * @brief   call notifier
 * @param   sptr_chain
 * @param   event, args
 * @retval  errno / chain count
 * @note    none
 */
kint32_t fwk_blocking_notifier_call_chain(struct fwk_notifier_chain *sptr_chain, kuint32_t event, void *args)
{
    struct fwk_notifier_block *sptr_nb, *sptr_temp;
    kint32_t re_event;
    kint32_t count = 0;

    if (!sptr_chain ||
        mr_list_empty(&sptr_chain->sgtc_nbs))
        return -ER_PERMIT;

    mutex_lock(&sptr_chain->sgtc_lock);

    foreach_list_next_entry_safe(sptr_nb, sptr_temp, &sptr_chain->sgtc_nbs, sgtc_link)
    {
        if (sptr_nb->expect_event & event)
        {
            re_event = sptr_nb->notifier_call(sptr_nb, event, args);
            if (re_event == event)
                count++;
        }
    }

    mutex_unlock(&sptr_chain->sgtc_lock);
    return count;
}

/*!
 * @brief   wait pengding
 * @param   sptr_chain
 * @param   event, args
 * @retval  errno / chain count
 * @note    none
 */
kint32_t fwk_blocking_pengding_call_chain(struct fwk_notifier_chain *sptr_chain, kuint32_t event, void *args)
{
    struct fwk_notifier_block *sptr_nb;
    kint32_t re_event;
    kint32_t count = 0;

    if (!sptr_chain ||
        mr_list_empty(&sptr_chain->sgtc_nbs))
        return -ER_PERMIT;

    mutex_lock(&sptr_chain->sgtc_lock);

    foreach_list_next_entry(sptr_nb, &sptr_chain->sgtc_nbs, sgtc_link)
    {
        if (sptr_nb->expect_event & event)
        {
            re_event = sptr_nb->pengding_call(sptr_nb, event, args);
            if (re_event == event)
                count++;
        }
    }

    mutex_unlock(&sptr_chain->sgtc_lock);
    return count;
}

/* end of file */
