/*
 * Hardware Abstraction Layer Net Interface
 *
 * File Name:   fwk_if.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.06
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_IF_H_
#define __FWK_IF_H_

/*!< The includes */
#include <platform/fwk_basic.h>

/*!< The defines */
#define NET_UAPI_DEF_IF_IFREQ										1
#define NET_UAPI_DEF_IF_IFMAP										1

#define NET_IFNAME_SIZE												(32)
#define NET_MAX_ADDR_LEN											(32)

#define NET_MAC_ETH_ALEN											(6)

#define NET_PROTO_ETH_P_IP    										0x0800		/*!< IP Protocol */
#define NET_PROTO_ETH_P_ARP   										0x0806		/*!< Address Resolution Protocol */
#define NET_PROTO_ETH_P_RARP  										0x8035		/*!< Reverse Address Resolution Protocol */
#define NET_PROTO_ETH_P_IPV6  										0x86DD		/*!< IPV6 Protocol */

struct fwk_sockaddr
{
	ksint8_t sa_data[14];														/*!< Address data.  */
};

#if NET_UAPI_DEF_IF_IFMAP
struct fwk_ifmap
{
	kuaddr_t mem_start;
	kuaddr_t mem_end;
	kuint16_t base_addr;
	kuint8_t irq;
	kuint8_t dma;
	kuint8_t port;
	/*!< 3 bytes spare */
};
#endif /*!< NET_UAPI_DEF_IF_IFMAP */

#if NET_UAPI_DEF_IF_IFREQ
struct fwk_ifreq
{
#define IFHWADDRLEN	6
	union
	{
		kstring_t	ifrn_name[NET_IFNAME_SIZE];										/*!< if name, e.g. "en0" */

	} urt_fwk_ifr_ifrn;

	union
	{
		struct fwk_sockaddr ifru_addr;
		struct fwk_sockaddr ifru_dstaddr;
		struct fwk_sockaddr ifru_broadaddr;
		struct fwk_sockaddr ifru_netmask;
		struct fwk_sockaddr ifru_hwaddr;
		ksint16_t ifru_flags;
		ksint32_t ifru_ivalue;
		ksint32_t ifru_mtu;
		struct fwk_ifmap ifru_map;
		kstring_t	ifru_slave[NET_IFNAME_SIZE];									/*!< Just fits the size */
		kstring_t	ifru_newname[NET_IFNAME_SIZE];
		void *ifru_data;
//		struct if_settings ifru_settings;

	} urt_fwk_ifr_ifru;
};
#endif /*!< NET_UAPI_DEF_IF_IFREQ */

#define mrt_ifr_name			urt_fwk_ifr_ifrn.ifrn_name					/*!< interface name */
#define mrt_ifr_hwaddr			urt_fwk_ifr_ifru.ifru_hwaddr				/*!< MAC address */
#define	mrt_ifr_addr			urt_fwk_ifr_ifru.ifru_addr					/*!< address */
#define	mrt_ifr_dstaddr			urt_fwk_ifr_ifru.ifru_dstaddr				/*!< other end of p-p lnk	*/
#define	mrt_ifr_broadaddr		urt_fwk_ifr_ifru.ifru_broadaddr				/*!< broadcast address */
#define	mrt_ifr_netmask			urt_fwk_ifr_ifru.ifru_netmask				/*!< interface net mask */
#define	mrt_ifr_flags			urt_fwk_ifr_ifru.ifru_flags					/*!< flags */
#define	mrt_ifr_metric			urt_fwk_ifr_ifru.ifru_ivalue				/*!< metric */
#define	mrt_ifr_mtu				urt_fwk_ifr_ifru.ifru_mtu					/*!< mtu */
#define mrt_ifr_map				urt_fwk_ifr_ifru.ifru_map					/*!< device map */
#define mrt_ifr_slave			urt_fwk_ifr_ifru.ifru_slave					/*!< slave device */
#define	mrt_ifr_data			urt_fwk_ifr_ifru.ifru_data					/*!< for use by interface	*/
#define mrt_ifr_ifindex			urt_fwk_ifr_ifru.ifru_ivalue				/*!< interface index */
#define mrt_ifr_bandwidth		urt_fwk_ifr_ifru.ifru_ivalue    			/*!< link bandwidth */
#define mrt_ifr_qlen			urt_fwk_ifr_ifru.ifru_ivalue				/*!< Queue length */
#define mrt_ifr_newname			urt_fwk_ifr_ifru.ifru_newname				/*!< New name	*/
// #define mrt_ifr_settings		urt_fwk_ifr_ifru.ifru_settings				/*!< Device/proto settings */


#endif /*!< _FWK_MAC_H_ */
