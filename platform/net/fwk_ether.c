/*
 * NetWork Interface
 *
 * File Name:   fwk_ether.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.12.09
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/net/fwk_if.h>
#include <platform/net/fwk_ether.h>
#include <platform/net/fwk_netdev.h>
#include <platform/net/fwk_skbuff.h>

/*!< The defines */

/*!< The globals */


/*!< API functions */
/*!
 * @brief   get network protocol
 * @param   sptr_skb, sptr_ndev
 * @retval  protocol
 * @note    none
 */
kint16_t fwk_eth_type_trans(struct fwk_sk_buff *sptr_skb, struct fwk_net_device *sptr_ndev)
{
    struct fwk_eth_hdr *sptr_ethdr;
    kuint16_t proto;

    sptr_ethdr = (struct fwk_eth_hdr *)sptr_skb->data;
    sptr_skb->sptr_ndev = sptr_ndev;

//  fwk_skb_reset_mac_header(sptr_skb);
//  fwk_skb_pull(sptr_skb, NET_ETHER_HDR_LEN);
    
    proto = sptr_ethdr->h_proto & mr_htons(0xFF00);
    if (proto >= mr_htons(NET_ETH_PROTO_802_3_MIN))
        return sptr_ethdr->h_proto;

    return NET_ETH_PROTO_802_2;
}

/*!< end of file */
