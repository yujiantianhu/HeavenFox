/*
 * NetWork Tcp Interface
 *
 * File Name:   fwk_tcp.c
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

/*!< The globals */
typedef enum __ERT_TCP_RAW_STATE
{
    NR_TCP_RAW_NONE = 0,
    NR_TCP_RAW_ACCEPTED,
    NR_TCP_RAW_RECEIVED,
    NR_TCP_RAW_CONNECTING,
    NR_TCP_RAW_CONNECTED,
    NR_TCP_RAW_CLOSING

} nrt_tpcb_state_t;

struct lwip_tcp_group
{
    struct tcp_pcb *sptr_tpcb;
    struct pq_queue *sptr_pq;

    nrt_tpcb_state_t state;
};

struct lwip_tcp_data
{
    struct tcp_pcb *sptr_tpcb;
    struct pbuf *sptr_buf;

    struct pq_data sgtc_pqd;
};

/*!< API functions */
/*!
 * @brief   release lwip_tcp_data
 * @param   sptr_pqd (member of ring queue)
 * @retval  none
 * @note    none
 */
static void lwip_tcp_data_free(struct pq_data *sptr_pqd)
{
    struct lwip_tcp_data *sptr_data;

    sptr_data = mr_container_of(sptr_pqd, struct lwip_tcp_data, sgtc_pqd);
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
static kbool_t lwip_tcp_data_check(struct pq_data *sptr_pqd, kusize_t limit)
{
    struct lwip_tcp_data *sptr_data;

    sptr_data = mr_container_of(sptr_pqd, struct lwip_tcp_data, sgtc_pqd);
    return !!(sptr_data->sptr_buf->len <= limit);
}

/*!
 * @brief   get every rx data from queue with poll ways
 * @param   sptr_tpcb, len (buffer's length)
 * @retval  tcp data
 * @note    none
 */
static struct lwip_tcp_data *lwip_tcp_raw_poll(struct tcp_pcb *sptr_tpcb, kusize_t len)
{
    struct lwip_tcp_group *sptr_tgrp;
    struct pq_queue *sptr_pq;
    struct pq_data *sptr_pqd;

    sptr_tgrp = (struct lwip_tcp_group *)sptr_tpcb->callback_arg;
    if (!sptr_tgrp)
        return mr_nullptr;

    sptr_pq = sptr_tgrp->sptr_pq;
    sptr_pqd = pq_dequeue_with_chk(sptr_pq, len);
    if (isValid(sptr_pqd))
        return mr_container_of(sptr_pqd, struct lwip_tcp_data, sgtc_pqd);

    return sptr_pqd ? ERR_PTR(-ER_LACK) : mr_nullptr;
}

/*!
 * @brief   called by socket_recvfrom
 * @param   sptr_tpcb, buf, ...
 * @retval  size
 * @note    read with blocking
 */
kssize_t lwip_tcp_raw_recv(struct tcp_pcb *sptr_tpcb, void *buf, kusize_t size)
{
    struct lwip_tcp_data *sptr_data;
    void *payload;
    kssize_t len;

    if (!size)
        return -ER_LACK;

    /*!< read one frame */
    do {
        sptr_data = lwip_tcp_raw_poll(sptr_tpcb, size);
        if (PTR_ERR(sptr_data) == (-ER_LACK))
        {
            print_err("%s: recv buffer is too small\r\n", __FUNCTION__);
            return -ER_LACK;
        }
        if (!sptr_data)
            continue;

        payload = sptr_data->sptr_buf->payload;
        len = sptr_data->sptr_buf->len;
        if (len)
            fwk_copy_to_user(buf, payload, len);

        lwip_tcp_data_free(&sptr_data->sgtc_pqd);
        break;

    } while (1);

    return len;
}

/*!
 * @brief   called by socket_sendto
 * @param   sptr_tpcb, buf, ...
 * @retval  size
 * @note    send (application layer ---> lwip ---> drivers)
 */
kssize_t lwip_tcp_raw_send(struct tcp_pcb *sptr_tpcb, const void *buf, kusize_t size)
{
    struct lwip_tcp_group *sptr_tgrp;
    err_t err;

    sptr_tgrp = (struct lwip_tcp_group *)sptr_tpcb->callback_arg;
    if (!sptr_tgrp ||
        (sptr_tgrp->state == NR_TCP_RAW_CLOSING))
        return -ER_TRXERR;

    if (size > tcp_sndbuf(sptr_tpcb))
        size = tcp_sndbuf(sptr_tpcb);

    err = tcp_write(sptr_tpcb, buf, size, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK)
    {
        print_err("%s: tcp send data to buffer failed!\r\n", __func__);
        return -ER_SDATA_FAILD;
    }

    err = tcp_output(sptr_tpcb);
    if (err != ERR_OK)
    {
        print_err("%s: tcp send data to hardware failed!\r\n", __func__);
        return -ER_SDATA_FAILD;
    }

    return size;
}

/*!
 * @brief   destroy tcp
 * @param   sptr_tpcb
 * @retval  none
 * @note    none
 */
static void __lwip_tcp_raw_close(struct lwip_tcp_group *sptr_tgrp)
{
    struct tcp_pcb *sptr_tpcb;

    if (!sptr_tgrp)
        return;

    sptr_tpcb = sptr_tgrp->sptr_tpcb;

    tcp_arg(sptr_tpcb, NULL);
    tcp_sent(sptr_tpcb, NULL);
    tcp_recv(sptr_tpcb, NULL);
    tcp_err(sptr_tpcb, NULL);
    tcp_poll(sptr_tpcb, NULL, 0);
    tcp_close(sptr_tpcb);

    pq_queue_destroy(sptr_tgrp->sptr_pq);
    kfree(sptr_tgrp);
}

/*!
 * @brief   error callback
 * @param   sptr_tpcb
 * @retval  none
 * @note    none
 */
static void __lwip_tcp_raw_error(void *arg, err_t err)
{
    struct lwip_tcp_group *sptr_tgrp;

    LWIP_UNUSED_ARG(err);

    sptr_tgrp = (struct lwip_tcp_group *)arg;

    pq_queue_destroy(sptr_tgrp->sptr_pq);
    kfree(sptr_tgrp);
}

/*!
 * @brief   poll callback
 * @param   sptr_tpcb
 * @retval  none
 * @note    none
 */
static err_t __lwip_tcp_raw_poll(void *arg, struct tcp_pcb *sptr_tpcb)
{
    struct lwip_tcp_group *sptr_tgrp;

    sptr_tgrp = (struct lwip_tcp_group *)arg;
    if (!sptr_tgrp)
    {
        /* nothing to be done */
        tcp_abort(sptr_tpcb);
        return ERR_ABRT; 
    }

    if (sptr_tgrp->state == NR_TCP_RAW_CLOSING)
        __lwip_tcp_raw_close(sptr_tgrp);

    return ERR_OK;
}

/*!
 * @brief   sent callback
 * @param   sptr_tpcb
 * @retval  none
 * @note    none
 */
static err_t __lwip_tcp_raw_sent(void *arg, struct tcp_pcb *sptr_tpcb, u16_t len)
{
    struct lwip_tcp_group *sptr_tgrp;

    sptr_tgrp = (struct lwip_tcp_group *)arg;
    if (sptr_tgrp->state == NR_TCP_RAW_CLOSING)
        __lwip_tcp_raw_close(sptr_tgrp);

    return ERR_OK;
}

/*!
 * @brief   recv callback
 * @param   sptr_tpcb, arg, ...
 * @retval  none
 * @note    none
 */
static err_t __lwip_tcp_raw_recv(void *arg, struct tcp_pcb *sptr_tpcb, struct pbuf *sptr_buf, err_t err)
{
    struct lwip_tcp_group *sptr_tgrp;
    struct lwip_tcp_data *sptr_data;

    sptr_tgrp = (struct lwip_tcp_group *)arg;

    if (!sptr_tgrp->sptr_pq)
        return ERR_MEM;

    /*!< remote host closed connection */
    if (!sptr_buf)
    {
        sptr_tgrp->state = NR_TCP_RAW_CLOSING;
        __lwip_tcp_raw_close(sptr_tgrp);
        return ERR_OK;
    }

    if (err != ERR_OK)
    {
        pbuf_free(sptr_buf);
        return err;
    }

    sptr_data = kmalloc(sizeof(*sptr_data), GFP_KERNEL);
    if (!isValid(sptr_data))
    {
        tcp_recved(sptr_tpcb, sptr_buf->tot_len);
        pbuf_free(sptr_buf);

        return ERR_MEM;
    }

    sptr_data->sptr_tpcb = sptr_tpcb;
    sptr_data->sptr_buf = sptr_buf;

    sptr_data->sgtc_pqd.release = lwip_tcp_data_free;
    sptr_data->sgtc_pqd.dequeue_chk = lwip_tcp_data_check;

    pq_enqueue(sptr_tgrp->sptr_pq, &sptr_data->sgtc_pqd);
    tcp_recved(sptr_tpcb, sptr_buf->tot_len);

    return ERR_OK;
}

/*!
 * @brief   accept callback
 * @param   sptr_tpcb
 * @retval  none
 * @note    none
 */
static err_t __lwip_tcp_raw_accept(void *arg, struct tcp_pcb *sptr_tpcb, err_t err)
{
    struct lwip_tcp_group *sptr_tgrp;
    struct pq_queue *sptr_pq;

    if ((err != ERR_OK) || (sptr_tpcb == NULL))
        return ERR_VAL;

    sptr_tgrp = kmalloc(sizeof(*sptr_tgrp), GFP_KERNEL);
    if (!isValid(sptr_tgrp))
        return ERR_MEM;
    
    sptr_pq = pq_queue_create(NR_PQ_RING, 1024);
    if (!isValid(sptr_pq))
    {
        kfree(sptr_tgrp);
        return ERR_MEM;
    }

    /*!<
     * Unless this pcb should have NORMAL priority, set its priority now.
     * When running out of pcbs, low priority pcbs can be aborted to create
     * new pcbs of higher priority. 
     */
    tcp_setprio(sptr_tpcb, TCP_PRIO_MIN);
    
    sptr_tgrp->sptr_tpcb = sptr_tpcb;
    sptr_tgrp->sptr_pq = sptr_pq;
    sptr_tgrp->state = NR_TCP_RAW_ACCEPTED;

    tcp_arg(sptr_tpcb, sptr_tgrp);
    tcp_recv(sptr_tpcb, __lwip_tcp_raw_recv);
    tcp_err(sptr_tpcb, __lwip_tcp_raw_error);
    tcp_poll(sptr_tpcb, __lwip_tcp_raw_poll, 0);
    tcp_sent(sptr_tpcb, __lwip_tcp_raw_sent);

    return ERR_OK;
}

/*!
 * @brief   tcp pcb init
 * @param   sptr_ip, port
 * @retval  tcp_pcb
 * @note    create rx ring queue for application layer
 */
struct tcp_pcb *lwip_tcp_raw_bind(const ip_addr_t *sptr_ip, u16_t port)
{
    struct tcp_pcb *sptr_tpcb;
    err_t err;

    sptr_tpcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!sptr_tpcb)
        goto fail;

    /*!< 
     * API function "socket_bind" will get "mr_htons(port)", but "tcp_bind" will convert port again with "lwip_htons";
     * therefore, port must be convert to it's original format
     */
    err = tcp_bind(sptr_tpcb, sptr_ip, mr_ntohs(port));
    if (err == ERR_OK)
        return sptr_tpcb;

    tcp_close(sptr_tpcb);

fail:
    return ERR_PTR(-ER_FAILD);
}

/*!
 * @brief   tcp listen
 * @param   sptr_tpcb
 * @retval  none
 * @note    none
 */
struct tcp_pcb *lwip_tcp_raw_listen(struct tcp_pcb *sptr_tpcb)
{
    if (!sptr_tpcb)
        return mr_nullptr;

    return tcp_listen(sptr_tpcb);
}

/*!
 * @brief   tcp accept
 * @param   sptr_tpcb
 * @retval  none
 * @note    none
 */
kint32_t lwip_tcp_raw_accept(struct tcp_pcb *sptr_tpcb)
{
    tcp_accept(sptr_tpcb, __lwip_tcp_raw_accept);
    return ER_NORMAL;
}

/* end of file */
