/*
 * NetWork UDP Interface
 *
 * File Name:   fwk_udp.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.12.13
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/queue.h>
#include <platform/base/fwk_mempool.h>
#include <platform/base/fwk_uaccess.h>
#include <platform/net/fwk_lwip.h>
#include <platform/net/fwk_netif.h>
#include <kernel/wait.h>
#include <kernel/sched.h>
#include <kernel/spinlock.h>

/*!< The globals */
struct lwip_udp_data
{
    struct udp_pcb *sptr_upcb;
    struct pbuf *sptr_buf;

    ip_addr_t *sptr_ipaddr;
    kuint16_t port;

    struct pq_data sgtc_pqd;
};

struct lwip_udp_priv
{
    struct pq_queue *sptr_pq;

    struct wait_queue_head sgtc_wqh;
    kbool_t isComing;

    struct spin_lock sgtc_lock;
    struct atomic sgtc_ref;
    kbool_t is_dead;
};

/*!< API functions */
/*!
 * @brief   release lwip_udp_data
 * @param   sptr_pqd (member of ring queue)
 * @retval  none
 * @note    none
 */
static void lwip_udp_raw_free(struct pq_data *sptr_pqd)
{
    struct lwip_udp_data *sptr_data;

    sptr_data = mr_container_of(sptr_pqd, struct lwip_udp_data, sgtc_pqd);
    pbuf_free(sptr_data->sptr_buf);
    kfree(sptr_data);
}

/*!
 * @brief   check the size of recv buffer is enough
 * @param   sptr_pqd (member of ring queue)
 * @param   limit (size of recv buffer)
 * @retval  1: enough; 0: no
 * @note    none
 */
static kbool_t lwip_udp_raw_check(struct pq_data *sptr_pqd, kusize_t limit)
{
    struct lwip_udp_data *sptr_data;

    sptr_data = mr_container_of(sptr_pqd, struct lwip_udp_data, sgtc_pqd);
    return !!(sptr_data->sptr_buf->len <= limit);
}

/*!
 * @brief   get every rx data from queue with poll ways
 * @param   sptr_upcb, len (buffer's length)
 * @retval  udp data
 * @note    none
 */
static struct lwip_udp_data *lwip_udp_raw_poll(struct udp_pcb *sptr_upcb, kusize_t len)
{
    struct lwip_udp_priv *sptr_priv = (struct lwip_udp_priv *)sptr_upcb->recv_arg;
    struct pq_queue *sptr_pq = sptr_priv->sptr_pq;
    struct pq_data *sptr_pqd;

    spin_lock_irqsave(&sptr_priv->sgtc_lock);
    sptr_pqd = pq_dequeue_with_chk(sptr_pq, len);
    spin_unlock_irqrestore(&sptr_priv->sgtc_lock);

    if (isValid(sptr_pqd))
        return mr_container_of(sptr_pqd, struct lwip_udp_data, sgtc_pqd);

    return sptr_pqd ? ERR_PTR(-ER_LACK) : mr_nullptr;
}

/*!
 * @brief   recv callback
 * @param   sptr_upcb, arg, ...
 * @retval  none
 * @note    none
 */
static void __lwip_udp_raw_recv(void *arg, struct udp_pcb *sptr_upcb, struct pbuf *sptr_buf,
                            const ip_addr_t *sptr_ipaddr, u16_t port)
{
    struct lwip_udp_priv *sptr_priv = (struct lwip_udp_priv *)arg;
    struct pq_queue *sptr_pq;
    struct lwip_udp_data *sptr_data;

    if (!sptr_buf || !sptr_priv ||
        sptr_priv->is_dead)
    {
        pbuf_free(sptr_buf);
        return;
    }

    atomic_inc(&sptr_priv->sgtc_ref);
    
    sptr_data = kmalloc(sizeof(*sptr_data), GFP_ATOMIC);
    if (!isValid(sptr_data))
    {
        pbuf_free(sptr_buf);
        atomic_dec(&sptr_priv->sgtc_ref);
        return;
    }

    sptr_data->sptr_upcb = sptr_upcb;
    sptr_data->sptr_ipaddr = (ip_addr_t *)sptr_ipaddr;
    sptr_data->port = mr_ntohs(port);
    sptr_data->sptr_buf = sptr_buf;

    sptr_data->sgtc_pqd.release = lwip_udp_raw_free;
    sptr_data->sgtc_pqd.dequeue_chk = lwip_udp_raw_check;

    spin_lock_irqsave(&sptr_priv->sgtc_lock);
    sptr_pq = sptr_priv->sptr_pq;
    pq_enqueue(sptr_pq, &sptr_data->sgtc_pqd);

    sptr_priv->isComing = true;
    spin_unlock_irqrestore(&sptr_priv->sgtc_lock);

    wake_up_interruptible(&sptr_priv->sgtc_wqh);
    atomic_dec(&sptr_priv->sgtc_ref);
}

/*!
 * @brief   called by socket_recvfrom
 * @param   sptr_upcb, buf, ...
 * @retval  size
 * @note    read with blocking
 */
kssize_t lwip_udp_raw_recvfrom(struct udp_pcb *sptr_upcb, void *buf, 
                            kusize_t size, ip_addr_t *sptr_src, u16_t *port)
{
    struct lwip_udp_priv *sptr_priv;
    struct lwip_udp_data *sptr_data;
    void *payload;
    kssize_t len;

    if (!sptr_upcb || !sptr_upcb->recv_arg)
        return -ER_INVALID;

    if (!size)
        return -ER_LACK;

    sptr_priv = (struct lwip_udp_priv *)sptr_upcb->recv_arg;
    if (!sptr_priv || sptr_priv->is_dead)
        return -ER_FORBID;

    atomic_inc(&sptr_priv->sgtc_ref);

    /*!< read one frame */
    do {
        wait_event_interruptible(&sptr_priv->sgtc_wqh, sptr_priv->isComing);
        sptr_priv->isComing = false;

        sptr_data = lwip_udp_raw_poll(sptr_upcb, size);
        if (PTR_ERR(sptr_data) == (-ER_LACK))
        {
            print_err("%s: recv buffer is too small\r\n", __FUNCTION__);

            atomic_dec(&sptr_priv->sgtc_ref);
            return -ER_LACK;
        }
        if (!sptr_data)
            continue;

        payload = sptr_data->sptr_buf->payload;
        len = sptr_data->sptr_buf->len;
        
        memcpy(sptr_src, sptr_data->sptr_ipaddr, sizeof(*sptr_src));
        *port = sptr_data->port;
        if (len)
            fwk_copy_to_user(buf, payload, len);

        lwip_udp_raw_free(&sptr_data->sgtc_pqd);
        break;

    } while (1);

    atomic_dec(&sptr_priv->sgtc_ref);
    return len;
}

/*!
 * @brief   called by socket_sendto
 * @param   sptr_upcb, buf, ...
 * @retval  size
 * @note    send (application layer ---> lwip ---> drivers)
 */
kssize_t lwip_udp_raw_sendto(struct udp_pcb *sptr_upcb, const ip_addr_t *sptr_dest, 
                            u16_t dest_port, const void *buf, kusize_t size)
{
    struct lwip_udp_priv *sptr_priv;
    struct pbuf *sptr_buf;
    err_t err;

    sptr_priv = (struct lwip_udp_priv *)sptr_upcb->recv_arg;
    if (!sptr_priv || sptr_priv->is_dead)
        return -ER_FORBID;

    atomic_inc(&sptr_priv->sgtc_ref);

    sptr_buf = pbuf_alloc(PBUF_TRANSPORT, size, PBUF_POOL);
    if (!sptr_buf)
    {
        print_err("%s: allocate lwip pbuf failed!\r\n", __func__);

        atomic_dec(&sptr_priv->sgtc_ref);
        return -ER_NOMEM;
    }

    memcpy(sptr_buf->payload, buf, size);
    err = udp_sendto(sptr_upcb, sptr_buf, sptr_dest, mr_ntohs(dest_port));
    if (err != ERR_OK)
    {
        pbuf_free(sptr_buf);
        print_err("%s: udp send lwip pbuf failed!\r\n", __func__);

        atomic_dec(&sptr_priv->sgtc_ref);
        return -ER_SDATA_FAILD;
    }

    atomic_dec(&sptr_priv->sgtc_ref);
    return size;
}

/*!
 * @brief   udp pcb init
 * @param   sptr_ip, port
 * @retval  udp_pcb
 * @note    create rx ring queue for application layer
 */
struct udp_pcb *lwip_udp_raw_bind(const ip_addr_t *sptr_ip, u16_t port)
{
    struct udp_pcb *sptr_upcb;
    struct pq_queue *sptr_pq;
    struct lwip_udp_priv *sptr_priv;
    err_t err;

    sptr_priv = (struct lwip_udp_priv *)kmalloc(sizeof(*sptr_priv), GFP_KERNEL);
    if (!isValid(sptr_priv))
        return ERR_PTR(-ER_NOMEM);
    
    sptr_pq = pq_queue_create(NR_PQ_RING, 1024);
    if (!isValid(sptr_pq))
    {
        kfree(sptr_priv);
        return ERR_PTR(-ER_NOMEM);
    }

    sptr_upcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (!sptr_upcb)
        goto fail;

    /*!< 
     * API function "socket_bind" will get "mr_htons(port)", but "udp_bind" will convert port again with "lwip_htons";
     * therefore, port must be convert to it's original format
     */
    err = udp_bind(sptr_upcb, sptr_ip, mr_ntohs(port));
    if (err == ERR_OK) 
    {
        sptr_priv->sptr_pq = sptr_pq;
        sptr_priv->isComing = false;
        spin_lock_init(&sptr_priv->sgtc_lock);
        init_waitqueue_head(&sptr_priv->sgtc_wqh);

        sptr_priv->is_dead = false;
        ATOMIC_SET(&sptr_priv->sgtc_ref, 0);

        udp_recv(sptr_upcb, __lwip_udp_raw_recv, sptr_priv);
        return sptr_upcb;
    }

    udp_remove(sptr_upcb);

fail:
    kfree(sptr_priv);
    pq_queue_destroy(sptr_pq);

    return ERR_PTR(-ER_FAILD);
}

/*!
 * @brief   udp pcb deinit
 * @param   sptr_upcb
 * @retval  none
 * @note    destroy rx ring queue and private data
 */
void lwip_udp_raw_unbind(struct udp_pcb *sptr_upcb)
{
    struct lwip_udp_priv *sptr_priv;
    struct pq_queue *sptr_pq;
    kutype_t flags;

    sptr_priv = (struct lwip_udp_priv *)sptr_upcb->recv_arg;
    sptr_pq = sptr_priv->sptr_pq;

    while (ATOMIC_READ(&sptr_priv->sgtc_ref))
        schedule_thread();

    local_irq_save(&flags);
    sptr_priv->is_dead = true;
    local_irq_restore(&flags);

    udp_remove(sptr_upcb);
    udp_recv(sptr_upcb, mr_nullptr, mr_nullptr);

    pq_queue_destroy(sptr_pq);
    kfree(sptr_priv);
}

/* end of file */
