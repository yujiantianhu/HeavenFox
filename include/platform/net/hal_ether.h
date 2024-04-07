/*
 * Hardware Abstraction Layer Net Interface
 *
 * File Name:   hal_ether.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.06
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_ETHER_H_
#define __HAL_ETHER_H_

/*!< The includes */
#include <platform/net/hal_if.h>

/*!< The defines */
/*!< EthernetII information Header */
struct hal_ethii_hdr
{
    kuint8_t h_dest[NET_MAC_ETH_ALEN];                      /*!< destination MAC address, ETH_ALEN = 6 */
    kuint8_t h_source[NET_MAC_ETH_ALEN];                    /*!< source MAC address, ETH_ALEN = 6 */

    kuint16_t h_proto;                                      /*!< network layer protocol type(IP, ARP, ...) */
};

#endif /*!< __HAL_ETHER_H_ */
