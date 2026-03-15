/*
 * Terminal Core API: Command ping
 *
 * File Name:   ping.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.06.25
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/net/fwk_if.h>
#include <platform/net/fwk_netif.h>
#include <platform/net/fwk_icmp.h>
#include <platform/net/fwk_ip.h>
#include <platform/net/fwk_socket.h>
#include <platform/net/fwk_lwip.h>
#include <kernel/sched.h>
#include <kernel/spinlock.h>

/*!< The defines */
#define PING_MAX_SEQNUM                                 32

struct lwip_icmp_queue
{
    khrtime_t tx_time;
    khrtime_t rx_time;

    void *buf;
    kusize_t len;
    kuint16_t seq_num;

    ip_addr_t sgtc_ipsrc;
    ip_addr_t sgtc_ipdst;

    struct timer_list sgtc_tm;
    kbool_t is_timeout;
    kuint8_t timeout_cnt;
};

struct lwip_icmp_data
{
    struct raw_pcb *sptr_pcb;
    kuint8_t type;
    
    kuint16_t seq_num;
    struct lwip_icmp_queue sgtc_seq[PING_MAX_SEQNUM];

    struct atomic sgtc_ref;
    kbool_t is_dead;
};

/*!< The globals */

/*!< The functions */

/*!< API functions */
/*!
 * @brief   recv timeout
 * @param   args: sptr_seq
 * @retval  none
 * @note    none
 */
static void __lwip_icmp_raw_timeout(kuint32_t args)
{
    struct lwip_icmp_queue *sptr_seq = (struct lwip_icmp_queue *)args;
    kchar_t *ip_addr;

    if (ip_addr_get_ip4_u32(&sptr_seq->sgtc_ipsrc) == 0)
    {
        ip_addr = ipaddr_ntoa(&sptr_seq->sgtc_ipdst);
        printk("From %s icmp_seq=%u Destination Host Unreachable\r\n", ip_addr, sptr_seq->seq_num);

        sptr_seq->is_timeout = true;
    }
    else
    {
        if (!sptr_seq->timeout_cnt)
            mod_timer(&sptr_seq->sgtc_tm, jiffies + secs_to_jiffies(10U));
        else if (sptr_seq->buf)
        {
            ip_addr = ipaddr_ntoa(&sptr_seq->sgtc_ipsrc);
            printk("From %s icmp_seq=%u Source Host Packets not read long time, auto free now\r\n",
                ip_addr, sptr_seq->seq_num);

            kfree(sptr_seq->buf);
            sptr_seq->buf = mr_nullptr;
        }

        sptr_seq->timeout_cnt++;
    }
}

/*!
 * @brief   recv callback
 * @param   sptr_pcb, arg, ...
 * @retval  errno
 * @note    none
 */
static kuint8_t __lwip_icmp_raw_recv(void *arg, struct raw_pcb *sptr_pcb, 
                            struct pbuf *sptr_buf, const ip_addr_t *sptr_ip)
{
    struct fwk_ip_hdr *sptr_iphdr;
    struct fwk_icmp_hdr *sptr_hdr;
    struct lwip_icmp_data *sptr_data;
    struct lwip_icmp_queue *sptr_seq;

    if (sptr_buf->tot_len < (NET_IP_HDR_LEN + NET_ICMP_HDR_LEN))
        return 0;

    /*!< Get data and icmp header */
    sptr_data = (struct lwip_icmp_data *)sptr_pcb->recv_arg;
    if (!sptr_data || sptr_data->is_dead)
        return 0;

    /*!< Get ip header */
    sptr_iphdr = (struct fwk_ip_hdr *)sptr_buf->payload;
    if (pbuf_header(sptr_buf, -NET_IP_HDR_LEN))
        return 0;

    atomic_inc(&sptr_data->sgtc_ref);
    sptr_hdr = (struct fwk_icmp_hdr *)sptr_buf->payload;

    switch (sptr_hdr->type)
    {
        case ICMP_ER:
            /*!< For ping, reply must be received immediately, otherwise it will be considered a failure */
            if ((sptr_hdr->u.sgtc_echo.id == NET_ICMP_PING_ID) &&
                (sptr_hdr->u.sgtc_echo.seq_no == mr_ntohs(sptr_data->seq_num)))
            {
                khrtime_t cur_tick;
                kuint32_t interval;
                kuint16_t seq_no;
                kchar_t *ip_addr;

                /*!< Get current ticks first (recieve reply time) */
                cur_tick = khrtime_ticks();

                seq_no = sptr_hdr->u.sgtc_echo.seq_no;
                sptr_seq = &sptr_data->sgtc_seq[seq_no % PING_MAX_SEQNUM];

                if (!ip_addr_cmp(&sptr_seq->sgtc_ipdst, sptr_ip))
                    goto out;

                memcpy(&sptr_seq->sgtc_ipsrc, sptr_ip, sizeof(*sptr_ip));
                sptr_seq->timeout_cnt = 0;

                if (sptr_seq->is_timeout)
                    goto out;

                sptr_seq->len = mr_ntohs(sptr_iphdr->tot_len);
                sptr_seq->buf = kmalloc(sptr_seq->len, GFP_ATOMIC);
                if (!isValid(sptr_seq->buf))
                    goto out;

                memcpy(sptr_seq->buf, sptr_iphdr, sptr_seq->len);
                sptr_seq->rx_time = cur_tick;
                interval = (kuint32_t)HRTICK_TO_USEC(sptr_seq->rx_time - sptr_seq->tx_time);

                ip_addr = ipaddr_ntoa(sptr_ip);
                printk("%u bytes from %s: icmp_seq=%u ttl=%u time=%u.%-03u ms\r\n", 
                    sptr_seq->len, ip_addr, seq_no, sptr_iphdr->ttl, 
                    interval / 1000, interval % 1000);

                pbuf_free(sptr_buf);
                atomic_dec(&sptr_data->sgtc_ref);

                return 1;
            }
            break;

        default: break;
    }

out:
    pbuf_header(sptr_buf, NET_IP_HDR_LEN);
    atomic_dec(&sptr_data->sgtc_ref);

    return 0;
}

/*!
 * @brief   called by socket_recvfrom
 * @param   sptr_pcb, buf, ...
 * @retval  size
 * @note    read with non-blocking
 */
kssize_t lwip_icmp_raw_recvfrom(struct raw_pcb *sptr_pcb, void *buf, 
                            kusize_t size, ip_addr_t *sptr_src)
{
    struct fwk_ip_hdr *sptr_iphdr;
    struct lwip_icmp_data *sptr_data;
    struct lwip_icmp_queue *sptr_seq;
    kuint16_t seq_no, iphdr_len;
    kusize_t real_len;

    if (size < NET_ICMP_HDR_LEN)
        return -ER_INVALID;

    sptr_data = (struct lwip_icmp_data *)sptr_pcb->recv_arg;
    if (!sptr_data || sptr_data->is_dead)
        return -ER_FORBID;

    atomic_inc(&sptr_data->sgtc_ref);

    seq_no = mr_ntohs(sptr_data->seq_num);
    sptr_seq = &sptr_data->sgtc_seq[seq_no % PING_MAX_SEQNUM];

    /*!< If buf is valid, it indicates that no timeout event */
    sptr_iphdr = (struct fwk_ip_hdr *)sptr_seq->buf;
    if (!sptr_iphdr)
    {
        atomic_dec(&sptr_data->sgtc_ref);
        return -ER_EMPTY;
    }

    /*!< Delete timer_list now after reading buf (timeout callback may free buf) */
    del_timer(&sptr_seq->sgtc_tm);
    if (sptr_seq->timeout_cnt > 1)
    {
        atomic_dec(&sptr_data->sgtc_ref);
        return -ER_TIMEOUT;
    }

    iphdr_len = sptr_iphdr->ihl * 4;
    real_len = CMP_MIN2(size, (sptr_seq->len - iphdr_len));

    /*!< Copy ICMP header and data, real_len is the size of ICMP */
    memcpy(buf, (void *)sptr_iphdr + iphdr_len, real_len);
    memcpy(sptr_src, &sptr_seq->sgtc_ipsrc, sizeof(*sptr_src));
    mr_smp_mb();

    kfree(sptr_seq->buf);
    memset(sptr_seq, 0, sizeof(*sptr_seq));

    atomic_dec(&sptr_data->sgtc_ref);
    return real_len;
}

/*!
 * @brief   Send icmp packets
 * @param   type: NET_PROTO_ICMP_ECHO
 * @param   data_size: PING_DATA_SIZE
 */
kssize_t lwip_icmp_raw_send(struct raw_pcb *sptr_pcb, 
                        const ip_addr_t *sptr_dest, const void *buf, kusize_t size)
{
    struct lwip_icmp_data *sptr_data;
    struct fwk_icmp_hdr *sptr_hdr;
    struct pbuf *sptr_buf;
    struct lwip_icmp_queue *sptr_seq;
    kuint16_t seq_no;
    err_t ret;

    sptr_data = (struct lwip_icmp_data *)sptr_pcb->recv_arg;
    if (!sptr_data || sptr_data->is_dead)
        return -ER_FORBID;

    atomic_inc(&sptr_data->sgtc_ref);

    sptr_buf = pbuf_alloc(PBUF_IP, size, PBUF_RAM);
    if (!sptr_buf)
    {
        atomic_dec(&sptr_data->sgtc_ref);
        return -ER_NOMEM;
    }

    /*!< Just one packet */
    if (sptr_buf->next || 
       (sptr_buf->len != sptr_buf->tot_len))
        goto fail;

    memcpy(sptr_buf->payload, buf, size);

    sptr_hdr = (struct fwk_icmp_hdr *)sptr_buf->payload;

    seq_no = sptr_hdr->u.sgtc_echo.seq_no;
    sptr_seq = &sptr_data->sgtc_seq[seq_no % PING_MAX_SEQNUM];

    /*!< Clear previous data */
    del_timer(&sptr_seq->sgtc_tm);
    if (sptr_seq->buf)
        kfree(sptr_seq->buf);

    memset(sptr_seq, 0, sizeof(*sptr_seq));
    sptr_seq->seq_num = seq_no;
    ip_addr_copy_from_ip4(sptr_seq->sgtc_ipdst, *sptr_dest);
    setup_timer(&sptr_seq->sgtc_tm, __lwip_icmp_raw_timeout, (kuint32_t)sptr_seq);

    sptr_data->type = sptr_hdr->type;
    sptr_data->seq_num = mr_htons(seq_no);

    ret = raw_sendto(sptr_pcb, sptr_buf, sptr_dest);
    if (ret < 0)
        goto fail;

    /*!< Save current time and start timer */
    sptr_seq->tx_time = khrtime_ticks();
    mod_timer(&sptr_seq->sgtc_tm, jiffies + msecs_to_jiffies(1000U));

    if ((sptr_data->type == NET_PROTO_ICMP_ECHO) && 
        (seq_no == 1))
    {
        kchar_t *ip_addr;

        ip_addr = ipaddr_ntoa(&sptr_seq->sgtc_ipdst);
        printk("PING %s %u bytes of data.\r\n", ip_addr, size);
    }

    atomic_dec(&sptr_data->sgtc_ref);
    return size;

fail:
    pbuf_free(sptr_buf);
    atomic_dec(&sptr_data->sgtc_ref);

    return -ER_FAILD;
}

/*!
 * @brief   icmp init
 * @param   none
 * @retval  sptr_pcb
 * @note    called by "socket_bind"
 */
struct raw_pcb *lwip_icmp_raw_init(void)
{
    struct lwip_icmp_data *sptr_data;
    struct raw_pcb *sptr_pcb;
    ip_addr_t ip;
    kint32_t seq = PING_MAX_SEQNUM;

    sptr_data = kzalloc(sizeof(*sptr_data), GFP_KERNEL);
    if (!isValid(sptr_data))
        return ERR_PTR(-ER_NOMEM);

    sptr_pcb = raw_new(IP_PROTO_ICMP);
    if (!sptr_pcb)
    {
        kfree(sptr_data);
        return ERR_PTR(-ER_FAILD);
    }

    sptr_data->sptr_pcb = sptr_pcb;
    while (seq--)
    {
        struct lwip_icmp_queue *sptr_seq;

        sptr_seq = &sptr_data->sgtc_seq[seq];
        setup_timer(&sptr_seq->sgtc_tm, mr_nullptr, 0);
    }

    ip4_addr_set_any(&ip);
    raw_recv(sptr_pcb, __lwip_icmp_raw_recv, sptr_data);
    raw_bind(sptr_pcb, (const ip_addr_t *)&ip);

    return sptr_pcb;
}

/*!
 * @brief   icmp deinit
 * @param   sptr_pcb
 * @retval  none
 * @note    called by "socket_close"
 */
void lwip_icmp_raw_exit(struct raw_pcb *sptr_pcb)
{
    struct lwip_icmp_data *sptr_data;
    struct lwip_icmp_queue *sptr_seq;
    kint32_t seq = PING_MAX_SEQNUM;
    kutype_t flags;

    sptr_data = (struct lwip_icmp_data *)sptr_pcb->recv_arg;
    sptr_seq = &sptr_data->sgtc_seq[0];

    while (atomic_get_val(&sptr_data->sgtc_ref))
        schedule_thread();

    local_irq_save(&flags);
    sptr_data->is_dead = true;
    local_irq_restore(&flags);

    raw_recv(sptr_pcb, mr_nullptr, mr_nullptr);
    raw_remove(sptr_pcb);

    while (seq--)
    {
        if (sptr_seq->buf)
            kfree(sptr_seq->buf);

        sptr_seq->buf = mr_nullptr;
        del_timer(&sptr_seq->sgtc_tm);
        sptr_seq++;
    }

    kfree(sptr_data);
}

/*!< end of file */
