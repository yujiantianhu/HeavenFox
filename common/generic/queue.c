/*
 * Queue
 *
 * File Name:   queue.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.11.24
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/generic.h>
#include <common/queue.h>
#include <platform/base/fwk_mempool.h>

/*!< The defines */

/*!< The globals */

/*!< API function */
/*!
 * @brief   create queue
 * @param   type: NR_PQ_DROP/NR_PQ_RING
 * @param   data_len: length of queue member
 * @retval  pq_queue
 * @note    none
 */
struct pq_queue *pq_queue_create(kint32_t type, kusize_t data_len)
{
    struct pq_queue *sptr_pq;

    sptr_pq = kmalloc(sizeof(*sptr_pq) + data_len * sizeof(struct pq_data *), GFP_KERNEL);
    if (!isValid(sptr_pq))
        return mr_nullptr;

    sptr_pq->type = type;
    sptr_pq->tot_len = data_len;
    sptr_pq->head = sptr_pq->tail = sptr_pq->len = 0;

    return sptr_pq;
}

/*!
 * @brief   destroy queue
 * @param   sptr_pq
 * @retval  none
 * @note    sptr_pqd->release: function used to destroy sptr_pqd
 */
void pq_queue_destroy(struct pq_queue *sptr_pq)
{
    struct pq_data *sptr_pqd;

    if (!sptr_pq->len)
        goto END;

    while (sptr_pq->len--)
    {
        sptr_pqd = sptr_pq->sptr_data[sptr_pq->tail];
        sptr_pq->tail = (sptr_pq->tail + 1) % sptr_pq->tot_len;

        if (sptr_pqd->release)
            sptr_pqd->release(sptr_pqd);
    }

END:
    kfree(sptr_pq);
}

/*!
 * @brief   queue add new member
 * @param   sptr_pq, sptr_data (new)
 * @retval  error code
 * @note    if it is a ring queue and the queue is full, delete the oldest member
 */
kint32_t pq_enqueue(struct pq_queue *sptr_pq, struct pq_data *sptr_data)
{
    if (sptr_pq->len < sptr_pq->tot_len)
        sptr_pq->len++;
    else
    {
        if (sptr_pq->type != NR_PQ_RING)
            return -ER_FULL;
        else
        {
            struct pq_data *sptr_old;

            sptr_old = sptr_pq->sptr_data[sptr_pq->tail];
            sptr_pq->tail = (sptr_pq->tail + 1) % sptr_pq->tot_len;

            if (sptr_old->release)
                sptr_old->release(sptr_old);
        }
    }

    sptr_pq->sptr_data[sptr_pq->head] = sptr_data;
    sptr_pq->head = (sptr_pq->head + 1) % sptr_pq->tot_len;       

    return ER_NORMAL;
}

/*!
 * @brief   get member from queue
 * @param   sptr_pq
 * @retval  member
 * @note    none
 */
void *pq_dequeue(struct pq_queue *sptr_pq)
{
    struct pq_data *sptr_pqd;

    if (!sptr_pq->len)
        return mr_nullptr;

    sptr_pqd = sptr_pq->sptr_data[sptr_pq->tail];
    sptr_pq->tail = (sptr_pq->tail + 1) % sptr_pq->tot_len;
    sptr_pq->len--;

    return sptr_pqd;
}

/*!
 * @brief   get member from queue after checking
 * @param   sptr_pq
 * @param   limit: the size of buffer (if buffer is too small, forbiden dequeue)
 * @retval  member
 * @note    none
 */
void *pq_dequeue_with_chk(struct pq_queue *sptr_pq, kusize_t limit)
{
    struct pq_data *sptr_pqd;

    if (!sptr_pq->len)
        return mr_nullptr;

    sptr_pqd = sptr_pq->sptr_data[sptr_pq->tail];
    if (sptr_pqd->dequeue_chk)
    {
        if (!sptr_pqd->dequeue_chk(sptr_pqd, limit))
            return ERR_PTR(-ER_LACK);
    }

    sptr_pq->tail = (sptr_pq->tail + 1) % sptr_pq->tot_len;
    sptr_pq->len--;

    return sptr_pqd;
}

/*!
 * @brief   just read member in queue
 * @param   sptr_pq
 * @retval  member
 * @note    none
 */
void *pq_lookback(struct pq_queue *sptr_pq, kint32_t *base)
{
    kuint32_t cur_index = 0;

    if (!sptr_pq->len)
        return mr_nullptr;

    if (*base < 0)
        cur_index = (sptr_pq->head + sptr_pq->tot_len - 1) % sptr_pq->tot_len;
    else
    {
        cur_index = ((*base) + sptr_pq->tot_len - 1) % sptr_pq->tot_len;

        if (sptr_pq->head < sptr_pq->tail)
        {
            if ((cur_index < sptr_pq->tail) &&
                (cur_index >= sptr_pq->head))
                goto fail;
        }
        else
        {
            if ((cur_index < sptr_pq->tail) ||
                (cur_index >= sptr_pq->head))
                goto fail;
        }
    }

    *base = cur_index;
    return sptr_pq->sptr_data[cur_index];

fail:
    return mr_nullptr;
}

/*!
 * @brief   just read member in queue
 * @param   sptr_pq
 * @retval  member
 * @note    none
 */
void *pq_lookfront(struct pq_queue *sptr_pq, kint32_t *base)
{
    kuint32_t cur_index = 0;

    if (!sptr_pq->len)
        return mr_nullptr;

    if (*base < 0)
    {
//      cur_index = sptr_pq->tail % sptr_pq->tot_len;
        return mr_nullptr;
    }

//  else
//  {
        cur_index = ((*base) + 1) % sptr_pq->tot_len;

        if (sptr_pq->head < sptr_pq->tail)
        {
            if ((cur_index < sptr_pq->tail) &&
                (cur_index >= sptr_pq->head))
                goto fail;
        }
        else
        {
            if ((cur_index < sptr_pq->tail) ||
                (cur_index >= sptr_pq->head))
                goto fail;
        }
//  }

    *base = cur_index;
    return sptr_pq->sptr_data[cur_index];

fail:
    return mr_nullptr;
}

/*!
 * @brief   get current number of members
 * @param   sptr_pq
 * @retval  lenth
 * @note    none
 */
kint32_t pq_queue_get_size(struct pq_queue *sptr_pq)
{
    return sptr_pq->len;
}

/* end of file */
