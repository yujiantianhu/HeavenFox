/*
 * Hardware Abstraction Layer Net Interface
 *
 * File Name:   hal_netdev.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.06
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_NETDEV_H_
#define __HAL_NETDEV_H_

/*!< The includes */
#include <platform/net/hal_if.h>
#include <platform/net/hal_skbuff.h>
#include <platform/hal_platform.h>

/*!< The defines */
typedef ksint32_t netdev_tx_t;

struct hal_netdev_stats
{
	kuint64_t rx_packets;
	kuint64_t tx_packets;
	kuint64_t rx_bytes;
	kuint64_t tx_bytes;
	kuint64_t rx_errors;
	kuint64_t tx_errors;
	kuint64_t rx_dropped;
	kuint64_t tx_dropped;
	kuint64_t multicast;
	kuint64_t collisions;
	kuint64_t rx_length_errors;
	kuint64_t rx_over_errors;
	kuint64_t rx_crc_errors;
	kuint64_t rx_frame_errors;
	kuint64_t rx_fifo_errors;
	kuint64_t rx_missed_errors;
	kuint64_t tx_aborted_errors;
	kuint64_t tx_carrier_errors;
	kuint64_t tx_fifo_errors;
	kuint64_t tx_heartbeat_errors;
	kuint64_t tx_window_errors;
	kuint64_t rx_compressed;
	kuint64_t tx_compressed;
};

struct hal_netdev_queue
{
	struct hal_net_device *sprt_netdev;							/*!< destination/source network device */
	kuint64_t tx_maxrate;
	kuint64_t trans_timeout;										/*!< statistics on the number of times the queue times out */
	kuint64_t trans_start;											/*!< The time of the last sent */
	kuint64_t state;												/*!< state */
};

struct hal_net_device
{
	kstring_t name[NET_IFNAME_SIZE];								/*!< network device name */
	kstring_t *ptr_ifalias;											/*!< network device aliases */

	kuint64_t state;												/*!< network device interface state */
	ksint32_t ifindex;												/*!< network Device Interface Index Value: Network device identifier */

	struct list_head sgrt_netdev;								/*!< network device list */
	struct hal_netdev_stats sgrt_states;						/*!< statistics on network device interfaces */
	const struct hal_netdev_ops *sprt_netdev_oprts;				/*!< network device operation API */
	const struct hal_ethtool_ops *sprt_ethtool_oprts;			/*!< ether tool operation API */

	kuint32_t flags;												/*!< Network device Interface identifier */
	kuint16_t priv_flags;											/*!< Network device Interface identifier; However, it is not visible to user space */

	kuint32_t mtu;													/*!< maximum transmission unit of the network device interface */
	kuint16_t type;													/*!< interface hardware type */
	kuint16_t hard_header_len;										/*!< hardware interface header lenth */
	kuint16_t min_header_len;										/*!< minimum hardware header length */

	kuint32_t uc_promisc;											/*!< unicast mode of the network device interface */
	kuint32_t promiscuity;											/*!< promiscuous mode of the network device interface */
	kuint32_t allmulti;												/*!< full multicast mode for the network device interface */

	kuint8_t *ptr_dev_addr;											/*!< MAC address of the network device interface */

	kuint8_t broadcast[NET_MAX_ADDR_LEN];							/*!< hardware broadcast address */
	struct hal_netdev_queue sgrt_rcu;							/*!< packet receive queue for network device interfaces */
	struct hal_netdev_queue *sprt_tx;							/*!< packet send queue for network device interfaces */

	kuint32_t num_tx_queues;										/*!< number of TX queues allocated at alloc_netdev_mq() time */
	kuint32_t real_num_tx_queues;									/*!< number of TX queues currently active in device */
	kuint64_t tx_queue_len;											/*!< max frame per queue allowned */

	struct hal_device sgrt_dev;
//	struct hal_phy_device *sprt_phydev;
};

struct hal_netdev_ops
{
	ksint32_t (*ndo_init) (struct hal_net_device *sprt_dev);
	void (*ndo_uninit) (struct hal_net_device *sprt_dev);
	ksint32_t (*ndo_open) (struct hal_net_device *sprt_dev);
	ksint32_t (*ndo_stop) (struct hal_net_device *sprt_dev);
	netdev_tx_t (*ndo_start_xmit) (struct hal_sk_buff *sprt_skb, struct hal_net_device *sprt_dev);
	void (*ndo_set_rx_mode) (struct hal_net_device *sprt_dev);
	ksint32_t (*ndo_set_mac_address) (struct hal_net_device *sprt_dev, void *ptr_addr);
	ksint32_t (*ndo_do_ioctl) (struct hal_net_device *sprt_dev, struct hal_ifreq *sprt_ifr, ksint32_t cmd);
	void (*ndo_tx_timeout) (struct hal_net_device *sprt_dev);
	struct hal_netdev_stats* (*ndo_get_stats) (struct hal_net_device *sprt_dev);
	ksint32_t (*ndo_add_slave) (struct hal_net_device *sprt_dev, struct hal_net_device *sprt_slave_dev);
	ksint32_t (*ndo_del_slave) (struct hal_net_device *sprt_dev, struct hal_net_device *sprt_slave_dev);
	ksint32_t (*ndo_set_tx_maxrate) (struct hal_net_device *sprt_dev, ksint32_t queue_index, kuint32_t maxrate);
};

struct hal_ethtool_ops
{
	ksint32_t (*begin) (struct hal_net_device *);
	void (*complete) (struct hal_net_device *);
};

/*!< The functions */
TARGET_EXT struct hal_net_device *hal_alloc_netdev_mq(ksint32_t sizeof_priv, const kstring_t *name,
													void (*setup) (struct hal_net_device *sprt_dev), kuint32_t queue_count);
TARGET_EXT void hal_free_netdev(struct hal_net_device *sprt_dev);


#define hal_alloc_netdev(sizeof_priv, name, setup)	\
		hal_alloc_netdev_mq(sizeof_priv, name, setup, 1)


#endif /*!< __HAL_NETDEV_H_ */
