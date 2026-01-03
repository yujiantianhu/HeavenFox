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
#include <platform/net/fwk_netif.h>
#include <platform/net/fwk_ip.h>
#include <platform/net/fwk_icmp.h>
#include <platform/net/fwk_udp.h>
#include <platform/net/fwk_tcp.h>
#include <platform/net/fwk_socket.h>

#include <kernel/thread.h>
#include <kernel/sched.h>

/*!< The defines */
struct fwk_netif_tcb
{
    struct fwk_sk_buff_head sgtc_head;
    
    void *args;
    void (*pfunc_rx)(void *rxq, void *args);
};

/*!< The globals */
static struct fwk_sk_buff_head sgtc_fwk_skb_rx_lists;
static struct spin_lock sgtc_fwk_netif_rx_lock = SPIN_LOCK_INIT();

#if (CONFIG_NET_RX_SOFTIRQ)
static struct fwk_netif_tcb *sptr_fwk_netif_rx_tcb;
#endif

/*!< API functions */
/*!
 * @brief   convert ip string to integer
 * @param   ip addr
 * @retval  ip
 * @note    example:
 *          input format can be: 
 *              "192.168.253.231"
 *              "0xC0.0xA8.0xFD.0xE7"
 *              "0xc0.0xa8.0xfd.0xe7"
 *          output format:
 *              (192 << 24) | (168 << 16) | (253 << 8) | 231
 */
kuint32_t fwk_inet_addr(const kchar_t *addr)
{
    kchar_t temp[12];
    kchar_t *p, *str;
    kuint8_t len = 0;
    kuint32_t val = 0, offset = 3, byte = 0;

    str = (kchar_t *)addr;
    for (p = str; str && (*str); p++) 
    {
        len = (kuint8_t)(p - str);

        if ((*p == '.') || (*p == '\0')) 
        {
            if (!len)
                goto fail;

            kstrncpy(temp, str, len);
            str = p + 1;
            temp[len] = '\0';

            ascii_to_dec(temp, (kint32_t *)&byte);
            /*!< 0 ~ 255 */
            if (byte > 255)
                goto fail;

            val |= (byte << (8 * offset));
            if (!offset) 
            {
                /*!< avoid the 4th '.' exsits*/
                if (*p)
                    goto fail;

                break;
            }

            if (!(*p) || !(*str))
                goto fail;

            offset--;
            continue;
        }

        if (len > sizeof(temp))
            goto fail;
    }

    return mr_htonl(val);

fail:
    print_err("%s: input argument error!\r\n", __FUNCTION__);
    return 0;
}

/*!
 * @brief   convert integer to ip string
 * @param   ip 
 * @retval  ip str
 * @note    example:
 *          input format can be: 
 *              (192 << 24) | (168 << 16) | (253 << 8) | 231
 *          output format:
 *              "192.168.253.231"
 */
kchar_t *fwk_inet_ntoa(kchar_t *inet_str, struct fwk_sockaddr_in *sptr_saddr)
{
    kuint32_t inet_addr;
    kuint32_t val;
    kuint32_t len, offset = 0;
    kuint32_t idx;

#define INET_IP_BYTES                   4

    inet_addr = mr_ntohl(sptr_saddr->sin_addr.s_addr);

    for (idx = 0; idx < INET_IP_BYTES; idx++)
    {
        val = (inet_addr >> (((INET_IP_BYTES - 1) - idx) << 3)) & 0xff;
        len = uint_to_str(&inet_str[offset], val);

        offset += len;
        if (idx != (INET_IP_BYTES - 1))
            inet_str[offset++] = '.';
        else
            inet_str[offset] = '\0';
    }

    return inet_str;

#undef INET_IP_BYTES
}

/*!
 * @brief   set random value
 * @param   buf (destination value)
 * @retval  none
 * @note    none
 */
void fwk_inet_random_addr(kuint8_t *buf, kusize_t lenth)
{
    for (kuint32_t idx = 0; idx < lenth; idx++)
        *(buf++) = (kuint8_t)random_val();
}

/*!
 * @brief   prepare to calculate the checksum (ICMP/UDP/TCP)
 * @param   msg (tcp/udp message)
 * @retval  check sum
 * @note    none
 */
kuint16_t fwk_transport_csum(kuint8_t *msg, kuint16_t data_len)
{
    kuint16_t data, idx;
    kuint32_t chksum = 0;
    kbool_t is_odd;

    is_odd = !!(data_len & 0x01);
    data_len &= ~(kuint16_t)0x01;

    for (idx = 0; idx < data_len; idx += 2) 
    {
        /*!< data is saved by big endian */
        data = (msg[idx] << 8) | msg[idx + 1];
        /*!< if cpu is little endian, swap high and low bytes */
        chksum += mr_htons(data);
    }

    if (is_odd)
        chksum += msg[data_len];

    chksum = ((chksum & 0xffff0000) >> 16) + (chksum & 0x0000ffff);
    chksum = ((chksum & 0xffff0000) >> 16) + (chksum & 0x0000ffff);

    return ((kuint16_t)(~chksum));
}

/*!
 * @brief   prepare to calculate the checksum (ICMP)
 * @param   msg (icmp message)
 * @retval  check sum
 * @note    none
 */
static kuint32_t __fwk_icmp_csum_before(kuint8_t *msg)
{
    struct fwk_icmp_hdr *sptr_icmphdr;

    sptr_icmphdr = (struct fwk_icmp_hdr *)msg;
    sptr_icmphdr->check_sum = 0;

    return 0;
}

/*!
 * @brief   prepare to calculate the checksum (TCP/UDP)
 * @param   sptr_iphdr, msg (tcp/udp message)
 * @retval  check sum
 * @note    none
 */
static kuint32_t __fwk_tcp_udp_csum_before(struct fwk_ip_hdr *sptr_iphdr, kuint8_t *msg)
{
    kuint16_t data, idx;
    kuint16_t data_len;
    kuint32_t chksum = 0;
    struct fwk_ip_fakehdr sgtc_fhdr;
    kuint8_t *fake_msg;

    data_len = mr_htons(sptr_iphdr->tot_len) - sptr_iphdr->ihl * 4;

    sgtc_fhdr.saddr = sptr_iphdr->saddr;
    sgtc_fhdr.daddr = sptr_iphdr->daddr;
    sgtc_fhdr.proto = sptr_iphdr->protocol;
    sgtc_fhdr.zero = 0;

    sgtc_fhdr.len = mr_htons(data_len);
    fake_msg = (kuint8_t *)&sgtc_fhdr;

    for (idx = 0; idx < sizeof(sgtc_fhdr); idx += 2) 
    {
        data = (fake_msg[idx] << 8) | fake_msg[idx + 1];
        chksum += data;
    }

    if (sptr_iphdr->protocol == NET_IP_PROTO_TCP)
    {
        struct fwk_tcp_hdr *sptr_tcphdr = (struct fwk_tcp_hdr *)msg;
        sptr_tcphdr->check_sum = 0;
    }
    else
    {
        struct fwk_udp_hdr *sptr_udphdr = (struct fwk_udp_hdr *)msg;
        sptr_udphdr->check_sum = 0;
    }

    return chksum;
}

/*!
 * @brief   prepare to calculate the checksum (IP)
 * @param   sptr_iphdr
 * @retval  check sum
 * @note    none
 */
kuint16_t fwk_ip_network_csum(struct fwk_ip_hdr *sptr_iphdr)
{
    kuint16_t data, idx;
    kuint32_t chksum = 0;
    kuint8_t *msg = (kuint8_t *)sptr_iphdr;

    sptr_iphdr->check = 0;
    for (idx = 0; idx < (sptr_iphdr->ihl * 4); idx += 2) 
    {
        data = (msg[idx] << 8) | msg[idx + 1];
        chksum += data;
    }

    chksum = ((chksum & 0xffff0000) >> 16) + (chksum & 0x0000ffff);
    return mr_htons((kuint16_t)(~chksum));
}

/*!
 * @brief   prepare to calculate the checksum (ICMP/UDP/TCP)
 * @param   sptr_iphdr, msg (tcp/udp message)
 * @retval  check sum
 * @note    none
 */
kuint16_t fwk_ip_transport_csum(struct fwk_ip_hdr *sptr_iphdr, kuint8_t *msg)
{
    kuint16_t data, idx;
    kuint16_t data_len;
    kuint32_t chksum = 0;
    kbool_t is_odd;

    switch (sptr_iphdr->protocol)
    {
        case NET_IP_PROTO_UDP:
        case NET_IP_PROTO_TCP:
            chksum = __fwk_tcp_udp_csum_before(sptr_iphdr, msg);
            break;

        case NET_IP_PROTO_ICMP:
            chksum = __fwk_icmp_csum_before(msg);
            break;

        default: break;
    }

    data_len = mr_htons(sptr_iphdr->tot_len) - sptr_iphdr->ihl * 4;
    is_odd = !!(data_len & 0x01);

    data_len &= ~(kuint16_t)0x01;
    for (idx = 0; idx < data_len; idx += 2) 
    {
        data = (msg[idx] << 8) | msg[idx + 1];
        chksum += data;
    }

    if (is_odd)
        chksum += msg[data_len];

    chksum = ((chksum & 0xffff0000) >> 16) + (chksum & 0x0000ffff);
    return mr_htons((kuint16_t)(~chksum));
}

/*!
 * @brief   prepare to calculate the checksum
 * @param   sptr_iphdr, offset (0 or sptr_iphdr->ihl)
 * @retval  check sum
 * @note    none
 */
kuint16_t fwk_ip_slow_csum(struct fwk_ip_hdr *sptr_iphdr, kuint16_t offset)
{
    if (offset < sptr_iphdr->ihl)
        return fwk_ip_network_csum(sptr_iphdr);

    return fwk_ip_transport_csum(sptr_iphdr, ((kuint8_t *)sptr_iphdr) + (sptr_iphdr->ihl * 4));
}

/*!
 * @brief   get global rx list
 * @param   none
 * @retval  rx list
 * @note    none
 */
struct fwk_sk_buff_head *fwk_netif_rxq_get(void)
{
    return &sgtc_fwk_skb_rx_lists;
}

/*!< -------------------------------------------------------------------- */
/*!
 * @brief   network open (called by net_link_up())
 * @param   name
 * @retval  errno
 * @note    name ---> sptr_ndev ---> driver_open
 */
kint32_t fwk_netif_open(const kchar_t *name)
{
    struct fwk_net_device *sptr_ndev;
    const struct fwk_netdev_ops *sptr_ops;

    sptr_ndev = fwk_ifname_to_ndev(name);
    if (!isValid(sptr_ndev))
        return PTR_ERR(sptr_ndev);

    sptr_ops = sptr_ndev->sptr_netdev_oprts;
    if (sptr_ops->ndo_open)
    {
        if (sptr_ops->ndo_open(sptr_ndev))
            return -ER_FAILD;
    }

    return ER_NORMAL;
}

/*!
 * @brief   network close (called by net_link_down())
 * @param   name
 * @retval  errno
 * @note    name ---> sptr_ndev ---> driver_stop
 */
kint32_t fwk_netif_close(const kchar_t *name)
{
    struct fwk_net_device *sptr_ndev;
    const struct fwk_netdev_ops *sptr_ops;

    sptr_ndev = fwk_ifname_to_ndev(name);
    if (!isValid(sptr_ndev))
        return PTR_ERR(sptr_ndev);

    sptr_ops = sptr_ndev->sptr_netdev_oprts;
    if (sptr_ops->ndo_stop)
    {
        if (sptr_ops->ndo_stop(sptr_ndev))
            return -ER_FAILD;
    }

    return ER_NORMAL;
}

/*!
 * @brief   network ioctl (called by virt_ioctl())
 * @param   request (cmd), args (address of buffer)
 * @retval  errno
 * @note    none
 */
kint32_t fwk_netif_ioctl(kuint32_t request, kuaddr_t args)
{
    struct fwk_ifreq *sptr_ifr;
    struct fwk_net_device *sptr_ndev;
    const struct fwk_netdev_ops *sptr_ops;

    sptr_ifr = (struct fwk_ifreq *)args;
    sptr_ndev = fwk_ifname_to_ndev(sptr_ifr->mr_ifr_name);
    if (!isValid(sptr_ndev))
        return PTR_ERR(sptr_ndev);

    /*!< if sptr_ops->ndo_do_ioctl defined, use sptr_ops->ndo_do_ioctl */
    sptr_ops = sptr_ndev->sptr_netdev_oprts;
    if (sptr_ops->ndo_do_ioctl)
    {
        if (!sptr_ops->ndo_do_ioctl(sptr_ndev, sptr_ifr, (kint32_t)request))
            goto out;
    }

    /*!< default operations */
    switch (request)
    {
        case NETWORK_IFR_GET_HWADDR:
            kmemcpy(sptr_ifr->mr_ifr_hwaddr.sa_data, sptr_ndev->dev_addr, NET_MAC_ETH_ALEN);
            break;

        case NETWORK_IFR_GET_MTU:
            sptr_ifr->mr_ifr_mtu = sptr_ndev->mtu;
            break;

        default:
            return -ER_INVALID;
    }

out:
    return ER_NORMAL;
}

/*!
 * @brief   send skb
 * @param   sptr_skb
 * @retval  size sent
 * @note    none
 */
static netdev_tx_t __fwk_dev_queue_xmit(struct fwk_sk_buff *sptr_skb)
{
    struct fwk_net_device *sptr_ndev;
    const struct fwk_netdev_ops *sptr_ops;
    netdev_tx_t retval;
    
    sptr_ndev = sptr_skb->sptr_ndev;
    sptr_ops = sptr_ndev->sptr_netdev_oprts;

    if (!sptr_ops->ndo_start_xmit)
        return -ER_TRXERR;

    retval = sptr_ops->ndo_start_xmit(sptr_skb, sptr_ndev);
    return retval;
}

/*!
 * @brief   send skb
 * @param   sptr_skb
 * @retval  size sent
 * @note    none
 */
kint32_t fwk_dev_queue_xmit(struct fwk_sk_buff *sptr_skb)
{
    return __fwk_dev_queue_xmit(sptr_skb);
}

/*!
 * @brief   add skb received to global rx list
 * @param   sptr_skb
 * @retval  errno
 * @note    the function will wake up rx_thread
 */
kint32_t fwk_netif_wake_rx(struct fwk_sk_buff *sptr_skb)
{
    struct spin_lock *sptr_lock;
    kutype_t flags;

    sptr_lock = &sgtc_fwk_netif_rx_lock;
    spin_lock_irqsave(sptr_lock, &flags);

    if (!fwk_skb_enqueue(fwk_netif_rxq_get(), sptr_skb))
    {
        spin_unlock_irqrestore(sptr_lock, flags);
        schedule_thread_wakeup(THREAD_TID_SOCKRX);
        
        return ER_NORMAL;
    }

    spin_unlock_irqrestore(sptr_lock, flags);
    return -ER_FAILD;
}

/*!
 * @brief   add skb received to global rx list
 * @param   sptr_skb
 * @retval  errno
 * @note    the function will wake up rx_thread or softirq (if configure "CONFIG_NET_RX_SOFTIRQ")
 */
kint32_t fwk_netif_rx(struct fwk_sk_buff *sptr_skb)
{
    struct spin_lock *sptr_lock;
    kutype_t flags;

    sptr_lock = &sgtc_fwk_netif_rx_lock;
    spin_lock_irqsave(sptr_lock, &flags);

    if (!fwk_skb_enqueue(fwk_netif_rxq_get(), sptr_skb))
    {
        spin_unlock_irqrestore(sptr_lock, flags);

    #if (CONFIG_NET_RX_SOFTIRQ)
        fwk_raise_softirq(NR_SOFTIRQ_NET_RX);
    #else
        schedule_thread_wakeup(THREAD_TID_SOCKRX);
    #endif
    
        return ER_NORMAL;
    }

    spin_unlock_irqrestore(sptr_lock, flags);
    return -ER_FAILD;
}

/*!< ----------------------------------------------------------------------- */
#if (CONFIG_NET_RX_SOFTIRQ)
static struct spin_lock sgtc_cpu_netif_rx_lock = SPIN_LOCK_INIT();

/*!
 * @brief   rx action for softirq
 * @param   nr (softirq event)
 * @retval  none
 * @note    if skb_list is empty, return right away
 */
static void fwk_netif_rx_action(kint32_t nr)
{
    struct fwk_netif_tcb *sptr_tcb;
    struct fwk_sk_buff_head *sptr_head;
    struct spin_lock *sptr_sklock, *sptr_cpulock;
    kutype_t flags;

    sptr_head = fwk_netif_rxq_get();
    sptr_tcb = sptr_fwk_netif_rx_tcb;
    sptr_sklock = &sgtc_fwk_netif_rx_lock;
    sptr_cpulock = &sgtc_cpu_netif_rx_lock;

    spin_lock_irqsave(sptr_sklock, &flags);
    if (mr_skbuff_list_empty(sptr_head))
    {
        spin_unlock_irqrestore(sptr_sklock, flags);
        return;
    }

    fwk_skb_split(&sptr_tcb->sgtc_head, sptr_head);
    fwk_skb_list_init(sptr_head);

    spin_unlock_irqrestore(sptr_sklock, flags);

    spin_lock_bh(sptr_cpulock);
    if (mr_likely(sptr_tcb->pfunc_rx))
        sptr_tcb->pfunc_rx(&sptr_tcb->sgtc_head, sptr_tcb->args);

    spin_unlock_bh(sptr_cpulock);
}

#else
/*!
 * @brief   rx thread
 * @param   args (for callback function)
 * @retval  args
 * @note    if skb_list is empty, sleep all the time
 */
static void *fwk_netif_rx_entry(void *args)
{
    struct fwk_netif_tcb *sptr_tcb;
    struct fwk_sk_buff_head *sptr_head;
    struct spin_lock *sptr_lock;
    kutype_t flags;

    sptr_head = fwk_netif_rxq_get();
    sptr_tcb = (struct fwk_netif_tcb *)args;
    sptr_lock = &sgtc_fwk_netif_rx_lock;

    for (;;)
    {
        while (true)
        {
            spin_lock_irqsave(sptr_lock, &flags);
            if (mr_skbuff_list_empty(sptr_head))
            {
                spin_unlock_irqrestore(sptr_lock, flags);
                break;
            }        

            fwk_skb_split(&sptr_tcb->sgtc_head, sptr_head);
            fwk_skb_list_init(sptr_head);

            spin_unlock_irqrestore(sptr_lock, flags);

            if (sptr_tcb->pfunc_rx)
                sptr_tcb->pfunc_rx(&sptr_tcb->sgtc_head, sptr_tcb->args);      
        }

        schedule_self_suspend();
    }

    return args;
}
#endif

/*!
 * @brief   netif initalization
 * @param   pfunc_rx: callback
 * @param   args (for callback function)
 * @retval  none
 * @note    create rx thread (only one for per cpu)
 */
void fwk_netif_init(void (*pfunc_rx)(void *rxq, void *args), void *args)
{
    struct fwk_netif_tcb *sptr_tcb;
    struct fwk_sk_buff_head *sptr_head;
    struct fwk_sockaddr_in sgtc_ip, sgtc_gw, sgtc_netmask;

    sptr_tcb = kmalloc(sizeof(*sptr_tcb), GFP_KERNEL);
    if (!isValid(sptr_tcb))
        return;

    sptr_tcb->args = args;
    sptr_tcb->pfunc_rx = pfunc_rx;
    fwk_skb_list_init(&sptr_tcb->sgtc_head);

    sptr_head = fwk_netif_rxq_get();
    fwk_skb_list_init(sptr_head);

#if (CONFIG_NET_RX_SOFTIRQ)
    sptr_fwk_netif_rx_tcb = sptr_tcb;
    fwk_open_softirq(NR_SOFTIRQ_NET_RX, fwk_netif_rx_action);
#else
    kernel_thread_create(THREAD_TID_SOCKRX, mr_nullptr, fwk_netif_rx_entry, sptr_tcb);
    thread_set_priority(mr_tid_attr(THREAD_TID_SOCKRX), THREAD_PROTY_SOCKRX);
    thread_set_name(THREAD_TID_SOCKRX, "netif_rx");
#endif

    /*!< IP any */
    sgtc_ip.sin_addr.s_addr = fwk_inet_addr("0.0.0.0");
    sgtc_gw.sin_addr.s_addr = fwk_inet_addr("0.0.0.0");
    sgtc_netmask.sin_addr.s_addr = fwk_inet_addr("255.255.255.0");
    net_link_up("dummy", &sgtc_ip, &sgtc_gw, &sgtc_netmask);

    /*!< register command */
    term_cmd_add_ifconfig();
    term_cmd_add_ping();
}

/*!< end of file */
