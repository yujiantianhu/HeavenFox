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
#include <platform/fwk_mempool.h>
#include <platform/fwk_uaccess.h>
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
    struct tcp_pcb *sprt_tpcb;
    struct pq_queue *sprt_pq;

    nrt_tpcb_state_t state;
};

struct lwip_tcp_data
{
    struct tcp_pcb *sprt_tpcb;
    struct pbuf *sprt_buf;

    struct pq_data sgrt_pqd;
};

/*!< API functions */
/*!
 * @brief   release lwip_tcp_data
 * @param   sprt_pqd (member of ring queue)
 * @retval  none
 * @note    none
 */
static void lwip_tcp_data_free(struct pq_data *sprt_pqd)
{
    struct lwip_tcp_data *sprt_data;

    sprt_data = mrt_container_of(sprt_pqd, struct lwip_tcp_data, sgrt_pqd);
    pbuf_free(sprt_data->sprt_buf);
    kfree(sprt_data);
}

/*!
 * @brief   check the size of recv buffer is enough
 * @param   sprt_pqd (member of ring queue)
 * @param   limit (size of recv buffer)
 * @retval  1: enough; 0: no
 * @note    none
 */
static kbool_t lwip_tcp_data_check(struct pq_data *sprt_pqd, kusize_t limit)
{
    struct lwip_tcp_data *sprt_data;

    sprt_data = mrt_container_of(sprt_pqd, struct lwip_tcp_data, sgrt_pqd);
    return !!(sprt_data->sprt_buf->len <= limit);
}

/*!
 * @brief   get every rx data from queue with poll ways
 * @param   sprt_tpcb, len (buffer's length)
 * @retval  tcp data
 * @note    none
 */
static struct lwip_tcp_data *lwip_tcp_raw_poll(struct tcp_pcb *sprt_tpcb, kusize_t len)
{
    struct lwip_tcp_group *sprt_tgrp;
    struct pq_queue *sprt_pq;
    struct pq_data *sprt_pqd;

    sprt_tgrp = (struct lwip_tcp_group *)sprt_tpcb->callback_arg;
    if (!sprt_tgrp)
        return mrt_nullptr;

    sprt_pq = sprt_tgrp->sprt_pq;
    sprt_pqd = pq_dequeue_with_chk(sprt_pq, len);
    if (isValid(sprt_pqd))
        return mrt_container_of(sprt_pqd, struct lwip_tcp_data, sgrt_pqd);

    return sprt_pqd ? ERR_PTR(-ER_LACK) : mrt_nullptr;
}

/*!
 * @brief   called by socket_recvfrom
 * @param   sprt_tpcb, buf, ...
 * @retval  size
 * @note    read with blocking
 */
kssize_t lwip_tcp_raw_recv(struct tcp_pcb *sprt_tpcb, void *buf, kusize_t size)
{
    struct lwip_tcp_data *sprt_data;
    void *payload;
    kssize_t len;

    if (!size)
        return -ER_LACK;

    /*!< read one frame */
    do {
        sprt_data = lwip_tcp_raw_poll(sprt_tpcb, size);
        if (PTR_ERR(sprt_data) == (-ER_LACK))
        {
            print_err("%s: recv buffer is too small\r\n", __FUNCTION__);
            return -ER_LACK;
        }
        if (!sprt_data)
            continue;

        payload = sprt_data->sprt_buf->payload;
        len = sprt_data->sprt_buf->len;
        if (len)
            fwk_copy_to_user(buf, payload, len);

        lwip_tcp_data_free(&sprt_data->sgrt_pqd);
        break;

    } while (1);

    return len;
}

/*!
 * @brief   called by socket_sendto
 * @param   sprt_tpcb, buf, ...
 * @retval  size
 * @note    send (application layer ---> lwip ---> drivers)
 */
kssize_t lwip_tcp_raw_send(struct tcp_pcb *sprt_tpcb, const void *buf, kusize_t size)
{
    struct lwip_tcp_group *sprt_tgrp;
    err_t err;

    sprt_tgrp = (struct lwip_tcp_group *)sprt_tpcb->callback_arg;
    if (!sprt_tgrp ||
        (sprt_tgrp->state == NR_TCP_RAW_CLOSING))
        return -ER_TRXERR;

    if (size > tcp_sndbuf(sprt_tpcb))
        size = tcp_sndbuf(sprt_tpcb);

    err = tcp_write(sprt_tpcb, buf, size, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK)
    {
        print_err("%s: tcp send data to buffer failed!\r\n", __func__);
        return -ER_SDATA_FAILD;
    }

    err = tcp_output(sprt_tpcb);
    if (err != ERR_OK)
    {
        print_err("%s: tcp send data to hardware failed!\r\n", __func__);
        return -ER_SDATA_FAILD;
    }

    return size;
}

/*!
 * @brief   destroy tcp
 * @param   sprt_tpcb
 * @retval  none
 * @note    none
 */
static void __lwip_tcp_raw_close(struct lwip_tcp_group *sprt_tgrp)
{
    struct tcp_pcb *sprt_tpcb;

    if (!sprt_tgrp)
        return;

    sprt_tpcb = sprt_tgrp->sprt_tpcb;

    tcp_arg(sprt_tpcb, NULL);
    tcp_sent(sprt_tpcb, NULL);
    tcp_recv(sprt_tpcb, NULL);
    tcp_err(sprt_tpcb, NULL);
    tcp_poll(sprt_tpcb, NULL, 0);
    tcp_close(sprt_tpcb);

    pq_queue_destroy(sprt_tgrp->sprt_pq);
    kfree(sprt_tgrp);
}

/*!
 * @brief   error callback
 * @param   sprt_tpcb
 * @retval  none
 * @note    none
 */
static void __lwip_tcp_raw_error(void *arg, err_t err)
{
    struct lwip_tcp_group *sprt_tgrp;

    LWIP_UNUSED_ARG(err);

    sprt_tgrp = (struct lwip_tcp_group *)arg;

    pq_queue_destroy(sprt_tgrp->sprt_pq);
    kfree(sprt_tgrp);
}

/*!
 * @brief   poll callback
 * @param   sprt_tpcb
 * @retval  none
 * @note    none
 */
static err_t __lwip_tcp_raw_poll(void *arg, struct tcp_pcb *sprt_tpcb)
{
    struct lwip_tcp_group *sprt_tgrp;

    sprt_tgrp = (struct lwip_tcp_group *)arg;
    if (!sprt_tgrp)
    {
        /* nothing to be done */
        tcp_abort(sprt_tpcb);
        return ERR_ABRT; 
    }

    if (sprt_tgrp->state == NR_TCP_RAW_CLOSING)
        __lwip_tcp_raw_close(sprt_tgrp);

    return ERR_OK;
}

/*!
 * @brief   sent callback
 * @param   sprt_tpcb
 * @retval  none
 * @note    none
 */
static err_t __lwip_tcp_raw_sent(void *arg, struct tcp_pcb *sprt_tpcb, u16_t len)
{
    struct lwip_tcp_group *sprt_tgrp;

    sprt_tgrp = (struct lwip_tcp_group *)arg;
    if (sprt_tgrp->state == NR_TCP_RAW_CLOSING)
        __lwip_tcp_raw_close(sprt_tgrp);

    return ERR_OK;
}

/*!
 * @brief   recv callback
 * @param   sprt_tpcb, arg, ...
 * @retval  none
 * @note    none
 */
static err_t __lwip_tcp_raw_recv(void *arg, struct tcp_pcb *sprt_tpcb, struct pbuf *sprt_buf, err_t err)
{
    struct lwip_tcp_group *sprt_tgrp;
    struct lwip_tcp_data *sprt_data;

    sprt_tgrp = (struct lwip_tcp_group *)arg;

    if (!sprt_tgrp->sprt_pq)
        return ERR_MEM;

    /*!< remote host closed connection */
    if (!sprt_buf)
    {
        sprt_tgrp->state = NR_TCP_RAW_CLOSING;
        __lwip_tcp_raw_close(sprt_tgrp);
        return ERR_OK;
    }

    if (err != ERR_OK)
    {
        pbuf_free(sprt_buf);
        return err;
    }

    sprt_data = kmalloc(sizeof(*sprt_data), GFP_KERNEL);
    if (!isValid(sprt_data))
    {
        tcp_recved(sprt_tpcb, sprt_buf->tot_len);
        pbuf_free(sprt_buf);

        return ERR_MEM;
    }

    sprt_data->sprt_tpcb = sprt_tpcb;
    sprt_data->sprt_buf = sprt_buf;

    sprt_data->sgrt_pqd.release = lwip_tcp_data_free;
    sprt_data->sgrt_pqd.dequeue_chk = lwip_tcp_data_check;

    pq_enqueue(sprt_tgrp->sprt_pq, &sprt_data->sgrt_pqd);
    tcp_recved(sprt_tpcb, sprt_buf->tot_len);

    return ERR_OK;
}

/*!
 * @brief   accept callback
 * @param   sprt_tpcb
 * @retval  none
 * @note    none
 */
static err_t __lwip_tcp_raw_accept(void *arg, struct tcp_pcb *sprt_tpcb, err_t err)
{
    struct lwip_tcp_group *sprt_tgrp;
    struct pq_queue *sprt_pq;

    if ((err != ERR_OK) || (sprt_tpcb == NULL))
        return ERR_VAL;

    sprt_tgrp = kmalloc(sizeof(*sprt_tgrp), GFP_KERNEL);
    if (!isValid(sprt_tgrp))
        return ERR_MEM;
    
    sprt_pq = pq_queue_create(NR_PQ_RING, 1024);
    if (!isValid(sprt_pq))
    {
        kfree(sprt_tgrp);
        return ERR_MEM;
    }

    /*!<
     * Unless this pcb should have NORMAL priority, set its priority now.
     * When running out of pcbs, low priority pcbs can be aborted to create
     * new pcbs of higher priority. 
     */
    tcp_setprio(sprt_tpcb, TCP_PRIO_MIN);
    
    sprt_tgrp->sprt_tpcb = sprt_tpcb;
    sprt_tgrp->sprt_pq = sprt_pq;
    sprt_tgrp->state = NR_TCP_RAW_ACCEPTED;

    tcp_arg(sprt_tpcb, sprt_tgrp);
    tcp_recv(sprt_tpcb, __lwip_tcp_raw_recv);
    tcp_err(sprt_tpcb, __lwip_tcp_raw_error);
    tcp_poll(sprt_tpcb, __lwip_tcp_raw_poll, 0);
    tcp_sent(sprt_tpcb, __lwip_tcp_raw_sent);

    return ERR_OK;
}

/*!
 * @brief   tcp pcb init
 * @param   sprt_ip, port
 * @retval  tcp_pcb
 * @note    create rx ring queue for application layer
 */
struct tcp_pcb *lwip_tcp_raw_bind(const ip_addr_t *sprt_ip, u16_t port)
{
    struct tcp_pcb *sprt_tpcb;
    err_t err;

    sprt_tpcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!sprt_tpcb)
        goto fail;

    /*!< 
     * API function "socket_bind" will get "mrt_htons(port)", but "tcp_bind" will convert port again with "lwip_htons";
     * therefore, port must be convert to it's original format
     */
    err = tcp_bind(sprt_tpcb, sprt_ip, mrt_ntohs(port));
    if (err == ERR_OK)
        return sprt_tpcb;

    tcp_close(sprt_tpcb);

fail:
    return ERR_PTR(-ER_FAILD);
}

/*!
 * @brief   tcp listen
 * @param   sprt_tpcb
 * @retval  none
 * @note    none
 */
struct tcp_pcb *lwip_tcp_raw_listen(struct tcp_pcb *sprt_tpcb)
{
    if (!sprt_tpcb)
        return mrt_nullptr;

    return tcp_listen(sprt_tpcb);
}

/*!
 * @brief   tcp accept
 * @param   sprt_tpcb
 * @retval  none
 * @note    none
 */
kint32_t lwip_tcp_raw_accept(struct tcp_pcb *sprt_tpcb)
{
    tcp_accept(sprt_tpcb, __lwip_tcp_raw_accept);
    return ER_NORMAL;
}

/* end of file */
