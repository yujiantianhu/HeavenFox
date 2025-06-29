/*
 * Loopback Ethernet Device
 *
 * File Name:   loopback.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.12.10
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>

#include <platform/net/fwk_if.h>
#include <platform/net/fwk_netdev.h>
#include <platform/net/fwk_ip.h>
#include <platform/net/fwk_arp.h>
#include <platform/net/fwk_skbuff.h>
#include <platform/net/fwk_ether.h>
#include <platform/net/fwk_icmp.h>
#include <platform/net/fwk_udp.h>

/*!< The defines */
struct loopback_drv_data
{
    struct fwk_device sgtc_dev;
    struct fwk_net_device *sptr_ndev;
};

#define LOOPBACK_MAC_ADDR                           { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }

/*!< The globals */
static struct loopback_drv_data sgtc_loopback_drv_data;

/*!< The functions */

/*!< API function */
/*!
 * @brief   rx handler
 * @param   sptr_ndev, buffer, len
 * @retval  size received
 * @note    none
 */
static kssize_t loopback_driver_recv(struct fwk_net_device *sptr_ndev, void *buffer, kssize_t len)
{
    struct fwk_sk_buff *sptr_skb;
    void *data;
    kuint32_t head_len;

    head_len = SKB_DATA_HEAD_LEN(NET_ETHER_HDR_LEN);
    sptr_skb = fwk_alloc_skb(len + 2 * head_len, GFP_KERNEL);
    if (!isValid(sptr_skb))
        return -ER_NOMEM;

    fwk_skb_reserve(sptr_skb, head_len);
    data = fwk_skb_put(sptr_skb, len);
    if (!isValid(data))
        goto fail;

    memcpy(data, buffer, sptr_skb->len);
    sptr_skb->protocol = fwk_eth_type_trans(sptr_skb, sptr_ndev);
    sptr_skb->sptr_ndev = sptr_ndev;

    fwk_skb_set_mac_header(sptr_skb, 0);
    fwk_skb_set_network_header(sptr_skb, NET_ETHER_HDR_LEN);

    /*!< commit to rx queue */
    fwk_netif_rx(sptr_skb);

    sptr_ndev->last_rx = jiffies;
    sptr_ndev->sgtc_stats.rx_packets++;
    sptr_ndev->sgtc_stats.rx_bytes += sptr_skb->len;

    return sptr_skb->len;

fail:
    fwk_free_skb(sptr_skb);
    return -ER_FAILD;
}

/*!
 * @brief   loopback: tx ---> rx
 * @param   sptr_ndev, buffer, len
 * @retval  size received
 * @note    none
 */
static kssize_t loopback_driver_recycle(struct fwk_net_device *sptr_ndev, void *buffer, kssize_t len)
{
    struct fwk_eth_hdr *sptr_ethdr;
    kuint8_t mac_address[NET_MAC_ETH_ALEN];
    kuint32_t ipaddr;
    kssize_t recv_len;

    sptr_ethdr = (struct fwk_eth_hdr *)buffer;

    /*!< swap source and destination mac address */
    kmemcpy(mac_address, sptr_ethdr->h_dest, NET_MAC_ETH_ALEN);
    kmemcpy(sptr_ethdr->h_dest, sptr_ethdr->h_source, NET_MAC_ETH_ALEN);
    kmemcpy(sptr_ethdr->h_source, mac_address, NET_MAC_ETH_ALEN);

    switch (mr_htons(sptr_ethdr->h_proto))
    {
        case NET_ETH_PROTO_IP:
        {
            struct fwk_ip_hdr *sptr_iphdr;
            sptr_iphdr = (struct fwk_ip_hdr *)(buffer + sizeof(*sptr_ethdr));

            switch (sptr_iphdr->protocol)
            {
                case NET_IP_PROTO_ICMP:
                {
                    struct fwk_icmp_hdr *sptr_icmphdr;
                    sptr_icmphdr = (struct fwk_icmp_hdr *)((kuint8_t *)sptr_iphdr + sizeof(*sptr_iphdr));

                    /*!< ICMP <0x00: rely; 0x08: ping> */
                    sptr_icmphdr->type = NET_PROTO_ICMP_ER;
                    sptr_icmphdr->check_sum = fwk_ip_slow_csum(sptr_iphdr, sptr_iphdr->ihl);

                    break;
                }
                case NET_IP_PROTO_UDP:
                {
                    struct fwk_udp_hdr *sptr_udphdr;
                    kuint16_t port;

                    sptr_udphdr = (struct fwk_udp_hdr *)((kuint8_t *)sptr_iphdr + sizeof(*sptr_iphdr));

                    port = sptr_udphdr->dst_port;
                    sptr_udphdr->src_port = sptr_udphdr->dst_port;
                    sptr_udphdr->dst_port = port;

                    sptr_udphdr->check_sum = fwk_ip_slow_csum(sptr_iphdr, sptr_iphdr->ihl);

                    break;
                }
                /*!< tcp requires establishing connection first, not suitable for loopback */
                default:
                    return -ER_INVALID;
            }

            /*!< swap source and destination ip address */
            ipaddr = sptr_iphdr->daddr;
            sptr_iphdr->daddr = sptr_iphdr->saddr;
            sptr_iphdr->saddr = ipaddr;

            sptr_iphdr->check = fwk_ip_slow_csum(sptr_iphdr, 0);

            break;
        }
        case NET_ETH_PROTO_ARP:
        {
            struct fwk_arp_hdr *sptr_arphdr;
            sptr_arphdr = (struct fwk_arp_hdr *)(buffer + sizeof(*sptr_ethdr));

            kmemcpy(mac_address, sptr_arphdr->mac_src, NET_MAC_ETH_ALEN);
            kmemcpy(sptr_arphdr->mac_src, sptr_ndev->dev_addr, NET_MAC_ETH_ALEN);
            kmemcpy(sptr_arphdr->mac_dst, mac_address, NET_MAC_ETH_ALEN);

            /*!< swap source and destination ip address */
            u32_set2u8(&ipaddr, &sptr_arphdr->ip_dst);
            u32_set2u8(&sptr_arphdr->ip_dst, &sptr_arphdr->ip_src);
            u32_set2u8(&sptr_arphdr->ip_src, &ipaddr);

            sptr_arphdr->opcode = mr_htons(NET_ARPOP_REPLY);

            break;
        }
        default: 
            return -ER_INVALID;
    }

    recv_len = loopback_driver_recv(sptr_ndev, buffer, len);
    return (recv_len > 0) ? len : (-ER_TRXERR);
}

/*!
 * @brief   tx handler
 * @param   sptr_ndev, buffer, len
 * @retval  size sent
 * @note    none
 */
static kssize_t loopback_driver_send(struct fwk_net_device *sptr_ndev, void *buffer, kssize_t len)
{
    /*!< loopback */
    return loopback_driver_recycle(sptr_ndev, buffer, len);
}

/*!
 * @brief   init
 * @param   sptr_ndev
 * @retval  errno
 * @note    called on fwk_register_netdevice()
 */
static kint32_t loopback_init(struct fwk_net_device *sptr_ndev)
{
    kuint8_t mac_address[NET_MAC_ETH_ALEN] = LOOPBACK_MAC_ADDR;

    kmemcpy(sptr_ndev->dev_addr, mac_address, NET_MAC_ETH_ALEN);
    return ER_NORMAL;
}

/*!
 * @brief   open
 * @param   sptr_ndev
 * @retval  errno
 * @note    called on link_up()
 */
static kint32_t loopback_open(struct fwk_net_device *sptr_ndev)
{
    sptr_ndev->sgtc_stats.tx_bytes = 0;
    sptr_ndev->sgtc_stats.tx_packets = 0;
    sptr_ndev->sgtc_stats.rx_bytes = 0;
    sptr_ndev->sgtc_stats.rx_packets = 0;

    fwk_netif_start_queue(sptr_ndev);
    return ER_NORMAL;
}

/*!
 * @brief   stop
 * @param   sptr_ndev
 * @retval  errno
 * @note    called on link_down()
 */
static kint32_t loopback_stop(struct fwk_net_device *sptr_ndev)
{
    fwk_netif_stop_queue(sptr_ndev);

    sptr_ndev->sgtc_stats.tx_bytes = 0;
    sptr_ndev->sgtc_stats.tx_packets = 0;
    sptr_ndev->sgtc_stats.rx_bytes = 0;
    sptr_ndev->sgtc_stats.rx_packets = 0;

    return ER_NORMAL;
}

/*!
 * @brief   data send (application layer ---> driver layer)
 * @param   sptr_skb, sptr_ndev
 * @retval  size sent
 * @note    called by tx_entry (function calling, or thread/process)
 */
static netdev_tx_t loopback_start_xmit(struct fwk_sk_buff *sptr_skb, struct fwk_net_device *sptr_ndev)
{
    netdev_tx_t size;

    fwk_netif_stop_queue(sptr_ndev);
    sptr_ndev->sgtc_stats.tx_packets++;
    sptr_ndev->sgtc_stats.tx_bytes += sptr_skb->len;

    size = loopback_driver_send(sptr_ndev, sptr_skb->data, sptr_skb->len);
    fwk_free_skb(sptr_skb);

    fwk_netif_wake_queue(sptr_ndev);
    return size;
}

/*!< net device operations*/
static const struct fwk_netdev_ops sgtc_loopback_drv_oprts =
{
    .ndo_init = loopback_init,
    .ndo_open = loopback_open,
    .ndo_stop = loopback_stop,
    .ndo_start_xmit = loopback_start_xmit,
};

/*!
 * @brief   setup
 * @param   sptr_ndev
 * @retval  none
 * @note    called on fwk_alloc_netdev
 */
static void loopback_driver_setup(struct fwk_net_device *sptr_ndev)
{
    sptr_ndev->mtu = 1500;
    sptr_ndev->sptr_netdev_oprts = &sgtc_loopback_drv_oprts;

    fwk_eth_random_addr(sptr_ndev->dev_addr);
    fwk_eth_broadcast_addr(sptr_ndev->broadcast);

    sptr_ndev->tx_queue_len = 1000;
    sptr_ndev->hard_header_len = NET_ETHER_HDR_LEN;
    sptr_ndev->min_header_len = NET_ETHER_HDR_LEN;
}

/*!
 * @brief   loopback_driver_probe
 * @param   sptr_data
 * @retval  errno
 * @note    none
 */
static kint32_t loopback_driver_probe(struct loopback_drv_data *sptr_data)
{
    struct fwk_net_device *sptr_ndev;
    kint32_t retval;

    sptr_ndev = fwk_alloc_netdev(0, "lo", loopback_driver_setup);
    if (!isValid(sptr_ndev))
        return -ER_FAILD;
    
    sptr_ndev->private_data = sptr_data;
    sptr_data->sptr_ndev = sptr_ndev;

    retval = fwk_register_netdevice(sptr_ndev);
    if (retval)
        goto fail;

    print_info("register a new netdevice (Loopback)\r\n");
    return ER_NORMAL;

fail:
    fwk_free_netdev(sptr_ndev);
    return -ER_FAILD;
}

/*!
 * @brief   loopback_driver_remove
 * @param   sptr_data
 * @retval  errno
 * @note    none
 */
static kint32_t loopback_driver_remove(struct loopback_drv_data *sptr_data)
{
    struct fwk_net_device *sptr_ndev;

    sptr_ndev = sptr_data->sptr_ndev;
    if (!isValid(sptr_ndev))
        return -ER_NULLPTR;

    fwk_unregister_netdevice(sptr_ndev);
    fwk_free_netdev(sptr_ndev);

    return ER_NORMAL;
}

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   loopback_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init loopback_driver_init(void)
{
    return loopback_driver_probe(&sgtc_loopback_drv_data);
}

/*!
 * @brief   loopback_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit loopback_driver_exit(void)
{
    loopback_driver_remove(&sgtc_loopback_drv_data);
}

IMPORT_DRIVER_INIT(loopback_driver_init);
IMPORT_DRIVER_EXIT(loopback_driver_exit);

/*!< end of file */
