/*
 * NetWork Interface
 *
 * File Name:   fwk_lwip.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.11.23
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/net/fwk_if.h>
#include <platform/net/fwk_netdev.h>
#include <platform/net/fwk_skbuff.h>

/*!< The defines */

/*!< The globals */


/*!< API functions */
/*!
 * @brief   allocate skb
 * @param   data_size: length (tail - data)
 * @param   flags (GFP_KERNEL/GFP_ATOMIC, but allocated from GFP_SOCK)
 * @retval  skb
 * @note    none
 */
struct fwk_sk_buff *fwk_alloc_skb(kuint32_t data_size, nrt_gfp_t flags)
{
    struct fwk_sk_buff *sptr_skb;
    kuint8_t *data;
    kusize_t skb_size;

    skb_size  = mr_align(sizeof(*sptr_skb), sizeof(kutype_t));
    data_size = mr_align(data_size, sizeof(kutype_t));
    sptr_skb  = kmalloc(skb_size + data_size, GFP_GET_FLAG(flags) | GFP_SOCK);
    if (!isValid(sptr_skb))
        return ERR_PTR(-ER_NOMEM);

    data = (kuint8_t *)sptr_skb + skb_size;

    memset(sptr_skb, 0, mr_offsetof(struct fwk_sk_buff, tail));
    sptr_skb->truesize = data_size;
    sptr_skb->head = sptr_skb->data = sptr_skb->tail = data;
    sptr_skb->end = sptr_skb->head + data_size;

    sptr_skb->mac_header = (typeof(sptr_skb->mac_header))(~0U);
    sptr_skb->network_header = (typeof(sptr_skb->network_header))(~0U);
    sptr_skb->transport_header = (typeof(sptr_skb->transport_header))(~0U);
    
    atomic_set_val(&sptr_skb->users, 1);
    fwk_skb_list_init((struct fwk_sk_buff_head *)sptr_skb);

    return sptr_skb;
}

/*!
 * @brief   release skb
 * @param   sptr_skb
 * @retval  none
 * @note    none
 */
void fwk_free_skb(struct fwk_sk_buff *sptr_skb)
{
    if (!sptr_skb)
        return;
    if (atomic_get_val(&sptr_skb->users) > 1)
        return;

    kfree(sptr_skb);
}

/*!
 * @brief   add skb to skb_list
 * @param   sptr_head, sptr_skb
 * @retval  errno
 * @note    none
 */
kint32_t fwk_skb_enqueue(struct fwk_sk_buff_head *sptr_head, struct fwk_sk_buff *sptr_skb)
{
    return fwk_skb_add_tail(sptr_head, sptr_skb);
}

/*!
 * @brief   del skb from skb_list
 * @param   sptr_head
 * @retval  errno
 * @note    none
 */
struct fwk_sk_buff *fwk_skb_dequeue(struct fwk_sk_buff_head *sptr_head)
{
    struct fwk_sk_buff *sptr_skb;

    sptr_skb = mr_skuff_next_entry(sptr_head);
    if (sptr_skb)
        fwk_skb_unlink(sptr_head, sptr_skb);

    return sptr_skb;
}

/*!< end of file */
