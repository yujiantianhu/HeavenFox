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
#include <platform/fwk_basic.h>
#include <platform/fwk_fcntl.h>
#include <platform/net/fwk_if.h>
#include <platform/net/fwk_netdev.h>
#include <platform/net/fwk_socket.h>
#include <platform/net/fwk_netif.h>
#include <platform/net/fwk_ip.h>
#include <platform/net/fwk_arp.h>
#include <platform/net/fwk_ether.h>
#include <platform/net/fwk_icmp.h>
#include <platform/net/fwk_udp.h>
#include <platform/net/fwk_tcp.h>
#include <platform/net/fwk_lwip.h>
#include <kernel/thread.h>
#include <kernel/sched.h>

/*!< The defines */
struct fwk_lwip_data
{
    struct netif sgtc_netif;
    void *ndev;

    tid_t txd;
    struct fwk_sk_buff_head sgtc_txq;
};

/*!< The globals */

/*!< API functions */
/*!
 * @brief   get size according to the proto type
 * @param   proto
 * @retval  size
 * @note    none
 */
static kssize_t lwip_get_ip_proto_size(kuint8_t proto)
{
    kssize_t size = -1;

    switch (proto)
    {
        case NET_IP_PROTO_ICMP:
            size = NET_ICMP_HDR_LEN;
            break;
        case NET_IP_PROTO_UDP:
            size = NET_UDP_HDR_LEN;
            break;
        case NET_IP_PROTO_TCP:
            size = NET_TCP_HDR_LEN;
            break;
        default: break;
    }

    return size;
}

/*!
 * @brief   convert pbuf to skb, and add to tx queue
 * @param   sptr_netif, sptr_buf
 * @retval  errno
 * @note    none
 */
static err_t lwip_lowlevel_output(struct netif *sptr_netif, struct pbuf *sptr_buf)
{
    struct fwk_network_if *sptr_if;
    struct fwk_lwip_data *sptr_data;
    struct fwk_sk_buff *sptr_skb;
    struct pbuf *sptr_per, *sptr_cur;
    struct fwk_eth_hdr *sptr_ethhdr;
    struct fwk_ip_hdr *sptr_iphdr;
    kuint32_t head_len;
    kssize_t transport_len = 0;

    if (!sptr_buf->tot_len)
        return ERR_OK;

    sptr_if = (struct fwk_network_if *)sptr_netif->state;
    sptr_data = (struct fwk_lwip_data *)sptr_if->private_data;

    for (sptr_per = sptr_buf; sptr_per && sptr_per->len;)
    {
        sptr_ethhdr = (struct fwk_eth_hdr *)sptr_per->payload;
    
        head_len = SKB_DATA_HEAD_LEN(NET_ETHER_HDR_LEN);
        sptr_skb = fwk_alloc_skb(sptr_per->len + 2 * head_len, GFP_KERNEL);
        if (!isValid(sptr_skb))
        {
            print_err("%s: allocate skb failed!\r\n", __FUNCTION__);
            goto END;
        }

        fwk_skb_reserve(sptr_skb, head_len);
        fwk_skb_put(sptr_skb, sptr_per->len);
        sptr_skb->sptr_ndev = sptr_data->ndev;
        sptr_skb->protocol = sptr_ethhdr->h_proto;

        fwk_skb_set_mac_header(sptr_skb, 0);
        fwk_skb_set_network_header(sptr_skb, NET_ETHER_HDR_LEN);

        switch (mr_htons(sptr_ethhdr->h_proto))
        {
            case NET_ETH_PROTO_IP:
                sptr_iphdr = (struct fwk_ip_hdr *)((void *)sptr_ethhdr + NET_ETHER_HDR_LEN);
                transport_len = lwip_get_ip_proto_size(sptr_iphdr->protocol);
                if (transport_len < 0)
                    goto fail;

                fwk_skb_set_transport_header(sptr_skb, NET_ETHER_HDR_LEN + NET_IP_HDR_LEN);
                break;

            case NET_ETH_PROTO_ARP:
                fwk_skb_set_transport_header(sptr_skb, NET_ETHER_HDR_LEN + NET_ARP_HDR_LEN);
                break;

            default: 
                print_err("%s: unable to recognize network layer protocol (%d)!\r\n", 
                        __FUNCTION__, mr_htons(sptr_ethhdr->h_proto));
                goto fail;
        }

        sptr_skb->data_len = sptr_skb->len - (fwk_skb_transport_offset(sptr_skb) + transport_len);

        /*!< copy pbuf to skb */
        memcpy(sptr_skb->data, sptr_per->payload, sptr_skb->len);
        fwk_skb_add_tail(&sptr_data->sgtc_txq, sptr_skb);

        goto END;
        
    fail:
        fwk_free_skb(sptr_skb);
    END:
        sptr_cur = sptr_per;
        sptr_per = sptr_cur->next;

        /*!< PBUF_RAW will be released by lwip-lib sources code */
        if (sptr_cur->type == PBUF_POOL)
            pbuf_free(sptr_cur);
    }

    return ERR_OK;
}

/*!
 * @brief   lwip enet initialization
 * @param   sptr_netif
 * @retval  errno
 * @note    none
 */
static err_t lwip_enet_init(struct netif *sptr_netif)
{
    struct fwk_network_if *sptr_if;
    struct fwk_ifreq sgtc_ifr;
    kint32_t sockfd;
    kint32_t retval;

    sockfd = NET_SOCKET_GENERIC;
    sptr_if = (struct fwk_network_if *)sptr_netif->state;

    sptr_netif->name[0] = 'e';
    sptr_netif->name[1] = 'n';
    sptr_netif->linkoutput = lwip_lowlevel_output;

#if LWIP_IPV4
    sptr_netif->output = etharp_output;
#endif

#if LWIP_IPV6
    sptr_netif->output_ip6 = ethip6_output;
#endif

    /*!< start hardware */
    if (fwk_netif_open(sptr_if->ifname))
        return ERR_IF;

    strcpy(sgtc_ifr.mr_ifr_name, sptr_if->ifname);
    retval = virt_ioctl(sockfd, NETWORK_IFR_GET_HWADDR, &sgtc_ifr);
    if (retval)
        return ERR_IF;

    sptr_netif->hwaddr_len = sizeof(sptr_netif->hwaddr);
    kmemcpy(&sptr_netif->hwaddr[0], sgtc_ifr.mr_ifr_hwaddr.sa_data, sptr_netif->hwaddr_len);

    retval = virt_ioctl(sockfd, NETWORK_IFR_GET_MTU, &sgtc_ifr);
    if (retval)
        return ERR_IF;

    sptr_netif->mtu = sgtc_ifr.mr_ifr_mtu;
    sptr_netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

    netif_set_link_up(sptr_netif);
    return ERR_OK;
}

/*!
 * @brief   lwip enet tx thread: send skbs one by one
 * @param   args: sptr_netif (private argument)
 * @retval  args
 * @note    none
 */
static void *fwk_lwip_tx_entry(void *args)
{
    struct netif *sptr_netif;
    struct fwk_network_if *sptr_if;
    struct fwk_lwip_data *sptr_data;
    struct fwk_sk_buff *sptr_skb;

    sptr_netif = (struct netif *)args;
    sptr_if = (struct fwk_network_if *)sptr_netif->state;
    sptr_data = (struct fwk_lwip_data *)sptr_if->private_data;

    for (;;)
    {
        while ((sptr_skb = fwk_skb_dequeue(&sptr_data->sgtc_txq)))
            fwk_dev_queue_xmit(sptr_skb);

        msleep(1);
    }

    return args;
}

/*!
 * @brief   lwip start (called by net_link_up())
 * @param   sptr_if
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_lwip_link_up(struct fwk_network_if *sptr_if)
{
    struct fwk_lwip_data *sptr_data;
    ip4_addr_t sgtc_ip, sgtc_gw, sgtc_mask;

    sptr_data = kzalloc(sizeof(*sptr_data), GFP_KERNEL);
    if (!isValid(sptr_data))
        return PTR_ERR(sptr_data);

    sgtc_ip.addr = sptr_if->sgtc_ip.sin_addr.s_addr;
    sgtc_gw.addr = sptr_if->sgtc_gw.sin_addr.s_addr;
    sgtc_mask.addr = sptr_if->sgtc_netmask.sin_addr.s_addr;

    sptr_if->private_data = sptr_data;
    sptr_data->ndev = fwk_ifname_to_ndev(sptr_if->ifname);
    if (!sptr_data->ndev)
        goto fail;

    fwk_skb_list_init(&sptr_data->sgtc_txq);

    /*!< save ip address, and call lwip_enet_init */
    netif_add(&sptr_data->sgtc_netif, 
            &sgtc_ip, &sgtc_mask, &sgtc_gw, 
            sptr_if, lwip_enet_init, ethernet_input);

    if (!netif_default)
    {
        /*!< the global variable "netif_default = sptr_netif" */
        netif_set_default(&sptr_data->sgtc_netif);
    }

	/*!< specify that the network if is up */
	netif_set_up(&sptr_data->sgtc_netif);

    sptr_data->txd = kernel_thread_create(-1, mr_nullptr, fwk_lwip_tx_entry, &sptr_data->sgtc_netif);
    thread_set_priority(mr_tid_attr(sptr_data->txd), THREAD_PROTY_SOCKTX);

    return ER_NORMAL;

fail:
    kfree(sptr_data);
    return -ER_FAILD;
}

/*!
 * @brief   lwip close (called by net_link_down())
 * @param   sptr_if
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_lwip_link_down(struct fwk_network_if *sptr_if)
{
    struct fwk_lwip_data *sptr_data;

    sptr_data = (struct fwk_lwip_data *)sptr_if->private_data;

    if (netif_default == &sptr_data->sgtc_netif)
        netif_set_default(mr_nullptr);

    netif_set_down(&sptr_data->sgtc_netif);
    netif_remove(&sptr_data->sgtc_netif);

    schedule_thread_sleep(sptr_data->txd);
    kfree(sptr_data);
    sptr_if->private_data = mr_nullptr;

    return ER_NORMAL;
}

/*!
 * @brief   lwip init for transport layer (called by socket_bind())
 * @param   sptr_socket
 * @retval  errno
 * @note    bind control block with ip/port/...
 */
static kint32_t fwk_lwip_init(struct fwk_network_com *sptr_socket)
{
    ip_addr_t *sptr_ip = (ip_addr_t *)&sptr_socket->sgtc_sin.sin_addr;
    kuint16_t port = sptr_socket->sgtc_sin.sin_port;
    void *pcb;

    switch (sptr_socket->type)
    {
        case NR_SOCK_STREAM:

            break;

        case NR_SOCK_DGRAM:
            pcb = (void *)lwip_udp_raw_bind(sptr_ip, port);
            if (!isValid(pcb))
                goto fail;

            sptr_socket->private_data = pcb;
        
            break;

        default: goto fail;
    }

    return ER_NORMAL;

fail:
    return -ER_FAILD;
}

/*!
 * @brief   corresonding to fwk_lwip_init
 * @param   sptr_socket
 * @retval  none
 * @note    none
 */
static void fwk_lwip_exit(struct fwk_network_com *sptr_socket)
{

}

/*!
 * @brief   listen port
 * @param   sptr_socket
 * @retval  none
 * @note    none
 */
static void fwk_lwip_listen(struct fwk_network_com *sptr_socket)
{
    struct tcp_pcb *sptr_tcp;

    sptr_tcp = (struct tcp_pcb *)sptr_socket->private_data;
    sptr_tcp = tcp_listen(sptr_tcp);
    sptr_socket->private_data = sptr_tcp;
}

/*!
 * @brief   send message (for tcp)
 * @param   sptr_socket, buf, size
 * @retval  size sent
 * @note    none
 */
static kssize_t fwk_lwip_send(struct fwk_network_com *sptr_socket, const void *buf, kssize_t size)
{
    return 0;
}

/*!
 * @brief   send message (for udp)
 * @param   sptr_socket, buf, size
 * @retval  size sent
 * @note    none
 */
static kssize_t fwk_lwip_sendto(struct fwk_network_com *sptr_socket, const void *buf, kssize_t len, 
                        kint32_t flags, const struct fwk_sockaddr *sptr_dest, fwk_socklen_t addrlen)
{
    struct udp_pcb *sptr_upcb;
    struct fwk_sockaddr_in sgtc_saddr;

    sptr_upcb = (struct udp_pcb *)sptr_socket->private_data;
    memcpy(&sgtc_saddr, sptr_dest, addrlen);

    return lwip_udp_raw_sendto(sptr_upcb, (const ip_addr_t *)&sgtc_saddr.sin_addr, 
                            sgtc_saddr.sin_port, buf, len);
}

/*!
 * @brief   recv message (for tcp)
 * @param   sptr_socket, buf, size
 * @retval  size received
 * @note    none
 */
static kssize_t fwk_lwip_recv(struct fwk_network_com *sptr_socket, void *buf, kssize_t size)
{
    return 0;
}

/*!
 * @brief   recv message (for udp)
 * @param   sptr_socket, buf, size
 * @retval  size received
 * @note    none
 */
static kssize_t fwk_lwip_recvfrom(struct fwk_network_com *sptr_socket, void *buf, size_t len, 
                        kint32_t flags, struct fwk_sockaddr *sptr_src, fwk_socklen_t *addrlen)
{
    struct udp_pcb *sptr_upcb;
    struct fwk_sockaddr_in sgtc_saddr;
    kssize_t size;

    sptr_upcb = (struct udp_pcb *)sptr_socket->private_data;
    size = lwip_udp_raw_recvfrom(sptr_upcb, buf, len, 
                            (ip_addr_t *)&sgtc_saddr.sin_addr, &sgtc_saddr.sin_port);

    *addrlen = sizeof(sgtc_saddr);
    memcpy(sptr_src, &sgtc_saddr, *addrlen);

    return size;
}

/*!< network device node operations of lwip interface */
static const struct fwk_network_if_ops sgtc_fwk_lwip_if_oprts =
{
    .init       = fwk_lwip_init,
    .exit       = fwk_lwip_exit,
    .listen     = fwk_lwip_listen,

    .send       = fwk_lwip_send,
    .recv       = fwk_lwip_recv,
    .sendto     = fwk_lwip_sendto,
    .recvfrom   = fwk_lwip_recvfrom,

    .link_up    = fwk_lwip_link_up,
    .link_down  = fwk_lwip_link_down,
};

/*!
 * @brief   deal with per skb received
 * @param   sptr_socket, sptr_skb
 * @retval  errno
 * @note    skb ---> pbuf ---> lwip ---> application layer
 */
static err_t lwip_lowlevel_input(struct netif *sptr_netif, struct fwk_sk_buff *sptr_skb)
{
    struct fwk_eth_hdr *sptr_ethhdr;
    struct pbuf *sptr_buf;
    kint32_t head_len;

    head_len = SKB_DATA_HEAD_LEN(NET_ETHER_HDR_LEN);
    sptr_buf = pbuf_alloc(PBUF_RAW, sptr_skb->len + head_len, PBUF_POOL);
    if (!sptr_buf)
    {
        print_err("%s: allocate lwip pbuf failed!\r\n", __FUNCTION__);
        goto END;
    }

    pbuf_header(sptr_buf, -head_len);
    pbuf_take(sptr_buf, sptr_skb->data, sptr_skb->len);
    sptr_ethhdr = (struct fwk_eth_hdr *)fwk_skb_mac_header(sptr_skb);

    switch (mr_htons(sptr_ethhdr->h_proto))
    {
        case NET_ETH_PROTO_IP:
        case NET_ETH_PROTO_ARP:
            if (sptr_netif->input(sptr_buf, sptr_netif) != ERR_OK)
                pbuf_free(sptr_buf);
            break;

        default:
            pbuf_free(sptr_buf);
            break;
    }

END:
    return ERR_OK;
}

/*!
 * @brief   recv callback (called by main loop or rx thread continuously)
 * @param   rxq: global rx queue
 * @retval  none
 * @note    get every skb from rx queue, and deal with it
 */
static void fwk_lwip_input(void *rxq, void *args)
{
    struct fwk_network_if *sptr_if;
    struct fwk_lwip_data *sptr_data;
    struct fwk_sk_buff *sptr_skb;
    struct fwk_sk_buff_head *sptr_rxq;

    sptr_rxq = (struct fwk_sk_buff_head *)rxq;
    while ((sptr_skb = fwk_skb_dequeue(sptr_rxq)))
    {
        sptr_if = mr_nullptr;
        while ((sptr_if = network_next_node(sptr_if)))
        {
            sptr_data = (struct fwk_lwip_data *)sptr_if->private_data;
            if (sptr_data->ndev != (void *)sptr_skb->sptr_ndev)
                continue;

            lwip_lowlevel_input(&sptr_data->sgtc_netif, sptr_skb);

            /*!< Handle all system timeouts for all core protocols */
            sys_check_timeouts();
        }

        fwk_free_skb(sptr_skb);
    }
}

/*!< -------------------------------------------------------------------- */
/*!
 * @brief   lwip interface init
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t __plat_init fwk_lwip_if_init(void)
{
    lwip_init();
    fwk_netif_init(fwk_lwip_input, mr_nullptr);

    network_set_default_ops(&sgtc_fwk_lwip_if_oprts);
    return ER_NORMAL;
}

IMPORT_PLATFORM_INIT(fwk_lwip_if_init);

/*!< end of file */
