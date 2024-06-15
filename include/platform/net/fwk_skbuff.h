/*
 * Hardware Abstraction Layer Net Interface
 *
 * File Name:   fwk_skbuff.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.06
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_SKBUFF_H_
#define __FWK_SKBUFF_H_

/*!< The includes */
#include <platform/net/fwk_if.h>
#include <platform/net/fwk_netdev.h>

/*!< The defines */
typedef kuint8_t*			sk_buff_data_t;

struct fwk_sk_buff
{
    /*!< These two members must be first. */
    struct fwk_sk_buff *sprt_next;                                      /*!< doubly list, pointing to the latter sk_buff */
    struct fwk_sk_buff *sprt_prev;                                      /*!< doubly list, pointing to the previous sk_buff */

    struct fwk_net_device *sprt_netdev;                                 /*!< net_device */

    /*!< 
     * represents the sum of the length of the data area (tail - data) and the length of the data area of the shard structure. 
     * In fact, the length of the data area in this len is a valid length 
     */
    kuint32_t len;

    /*!< 
     * because the protocol headers are not deleted, only valid protocol headers and packet contents are counted. 
     * For example, when at L3, the protocol header length of L2 is not calculated 
     */
    kuint32_t data_len;                           						/*!< represents only the length of the sharded struct data area，so len = (tail - data) + data_len */
    kuint32_t mac_len;                            						/*!< length of the MAC header */
    kuint32_t hdr_len;                            						/*!< when used for clone, it indicates the head length of skb of clone */

    kuint32_t protocol : 16;                        					/*!< the protocol type of the packet, which identifies whether it is an IP packet, an ARP packet, or some other packet */

    sk_buff_data_t ptr_transport_header;                   				/*!< point to the Layer 4 (Transport Layer, TCP/UDP) header struct */
    sk_buff_data_t ptr_network_header;                     				/*!< point to the Layer 3 (Network Layer, IP/ARP/...) IP header struct */
    sk_buff_data_t ptr_mac_header;                         				/*!< point to the Layer 2 ((Data Link Layer) MAC header */

    /*!< These elements must be at the end, see alloc_skb() for details. */
    sk_buff_data_t ptr_tail;                               				/*!< point to the end of the actual data segment in the packet */
    sk_buff_data_t ptr_end;                                				/*!< point to the end of the entire packet */
    kuint8_t *ptr_head;                              					/*!< point to the start of the entire packet */
    kuint8_t *ptr_data;                              					/*!< point to the start of the actual data segment in the packet */

    /*!< 
     * represents the total length, including the length of the sk_buff itself 
     * and the length of the data area of the shard structure and the shard structure
     */
    kuint32_t truesize;                           						
    kuint32_t users;                              						/*!< The number of times skb has been referenced by clones, which is used for memory requests and cloning */
};


#endif /*!< __FWK_SKBUFF_H_ */
