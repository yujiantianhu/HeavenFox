/*
 * Hardware Abstraction Layer Net Interface
 *
 * File Name:   hal_ip.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.06
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_IP_H_
#define __HAL_IP_H_

/*!< The includes */
#include <platform/net/hal_if.h>

/*!< The defines */
#define NET_IPV4_FLAG										(0x04)

/*!< IP information Header */
struct hal_ip_hdr {
#if IS_BOARD_ENDIAN_LIT                       				/*!< Little-endian mode, low address storage low bytes */
    kuint8_t ihl : 4,                                       /*!< Low address, so here is the low 4 bits, the length of the IP frame is stored: ihl * 4, generally ihl = 0x5, and the calculation result is exactly 20 */
			 version : 4;                                   /*!< High address, so here is 4 bits high, storing the IP protocol version, if IPv4, then version = 0x4*/
#elif IS_BOARD_ENDIAN_BIG                       			/*!< Big-endian mode, low addresses store high bytes; In contrast to the little-endian, the version is reversed with ihl */
    kuint8_t version : 4,
  			 ihl : 4;
#endif

    kuint8_t	tos;                                        /*!< service type */
    kuint16_t	tot_len;                                    /*!< frame lenth */
    kuint16_t	id;                                         /*!< identifier */
    kuint16_t	frag_off;                                   /*!< the upper 4 bits are flags, and the lower 12 bits are the offsets */
    kuint8_t	ttl;                                        /*!< survival */
    kuint8_t	protocol;                                   /*!< the upper layer (transport layer) protocol, in the case of TCP, then protocol = 0x06 */
    kuint16_t	check;                                      /*!< CRC */
    kuint32_t	saddr;                                      /*!< source ip address */
    kuint32_t	daddr;                                      /*!< destination IP address */
};

#endif /*!< __HAL_IP_H_ */
