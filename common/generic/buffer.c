/*
 * Buffer
 *
 * File Name:   buffer.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.05.19
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/generic.h>
#include <common/buffer.h>
#include <platform/base/fwk_mempool.h>

/*!< The defines */

/*!< The globals */

/*!< API function */
/*!
 * @brief   Allocate buffer
 * @param   sptr_pq
 * @param   type: ring buffer or not, it can be "NR_PQ_BUF_DROP" or "NR_PQ_BUF_RING"
 * @retval  lenth
 * @note    none
 */
struct pq_buffer *pq_buffer_create(kint32_t type, kusize_t buf_len)
{
    struct pq_buffer *sptr_pq;

    if (!buf_len)
        return mr_nullptr;

    sptr_pq = kmalloc(sizeof(*sptr_pq) + buf_len * sizeof(kuint8_t), GFP_KERNEL);
    if (!isValid(sptr_pq))
        return mr_nullptr;

    sptr_pq->type = type;
    sptr_pq->buf_start = (void *)sptr_pq + sizeof(*sptr_pq);
    sptr_pq->tot_len = buf_len;
    sptr_pq->head = sptr_pq->tail = sptr_pq->len = 0;

    return sptr_pq;
}

/*!
 * @brief   Release buffer
 * @param   sptr_pq
 * @retval  lenth
 * @note    none
 */
void pq_buffer_destroy(struct pq_buffer *sptr_pq)
{
    kfree(sptr_pq);
}

/*!
 * @brief   Write
 * @param   sptr_pq
 * @param   data: will be append to pq_buffer
 * @retval  none
 * @note    none
 */
static void __pq_buffer_write(struct pq_buffer *sptr_pq, const void *data, kusize_t len)
{
    kusize_t size_dived = len;

    if ((sptr_pq->head + len) > sptr_pq->tot_len)
        size_dived = sptr_pq->tot_len - sptr_pq->head;

    memcpy(sptr_pq->buf_start + sptr_pq->head, data, size_dived);
    if (size_dived)
        memcpy(sptr_pq->buf_start, data + size_dived, len - size_dived);
}

/*!
 * @brief   Read
 * @param   sptr_pq
 * @param   buffer: used to save data read
 * @retval  none
 * @note    none
 */
static void __pq_buffer_read(struct pq_buffer *sptr_pq, void *buffer, kusize_t len)
{
    kusize_t size_dived = len;

    if ((sptr_pq->tail + len) > sptr_pq->tot_len)
        size_dived = sptr_pq->tot_len - sptr_pq->tail;

    memcpy(buffer, sptr_pq->buf_start + sptr_pq->tail, size_dived);
    if (size_dived)
        memcpy(buffer + size_dived, sptr_pq->buf_start, len - size_dived);
}

/*!
 * @brief   Write append
 * @param   sptr_pq
 * @param   data: will be append to pq_buffer
 * @retval  lenth
 * @note    if len is larger than tot_len, return right away
 */
kssize_t pq_buffer_write(struct pq_buffer *sptr_pq, const void *data, kusize_t len)
{
    if ((sptr_pq->len + len) < sptr_pq->tot_len)
        sptr_pq->len += len;
    else
    {
        if (sptr_pq->type != NR_PQ_BUF_RING)
            return -ER_FULL;
        if (len > sptr_pq->tot_len)
            return -ER_MORE;

        /*!< tail offset */
        sptr_pq->tail = (sptr_pq->tail + len - (sptr_pq->tot_len - sptr_pq->len)) % sptr_pq->tot_len;
        sptr_pq->len = sptr_pq->tot_len;
    }

    __pq_buffer_write(sptr_pq, data, len);
    sptr_pq->head = (sptr_pq->head + len) % sptr_pq->tot_len;

    return (kssize_t)len;
}

/*!
 * @brief   Read
 * @param   sptr_pq
 * @param   buffer: used to save data read
 * @retval  lenth
 * @note    if len is smaller than tot_len, just read len bytes
 */
kssize_t pq_buffer_read(struct pq_buffer *sptr_pq, void *buffer, kusize_t len)
{
    kusize_t size = CMP_MIN2(len, sptr_pq->len);

    if (!buffer || !size)
        return 0;
    
    __pq_buffer_read(sptr_pq, buffer, size);

    sptr_pq->tail = (sptr_pq->tail + size) % sptr_pq->tot_len;
    sptr_pq->len -= size;

    return (kssize_t)size;
}

/*!
 * @brief   Write append
 * @param   sptr_pq
 * @param   data: will be append to pq_buffer
 * @retval  lenth
 * @note    none
 */
kssize_t pq_message_write(struct pq_buffer *sptr_pq, const void *data, kusize_t len)
{
    kusize_t len_size = sizeof(len);
    kusize_t real_len = mr_align(len, 4);
    kusize_t require_size = len_size + real_len;
    struct pq_message sgtc_msg;
    kubyte_t fix_data[4] = {};

    if (!data || !len)
        return -ER_NOMEM;

    /*!< Too large */
    if (require_size > sptr_pq->tot_len)
        return -ER_MORE;

    /*!< If true, indicates that sptr_pq->len is not zero */
    while ((sptr_pq->len + require_size) > sptr_pq->tot_len) 
    {
        kusize_t size = CMP_MIN2(len_size, sptr_pq->len);

        if (mr_unlikely(sptr_pq->type != NR_PQ_BUF_RING))
            return -ER_FULL;

        /*!< Indicate that size is equaled to sptr_pq->len */
        if (size < len_size) 
        {
            /*!< Abandon invalid data */
            sptr_pq->tail = (sptr_pq->tail + size) % sptr_pq->tot_len;
            sptr_pq->len -= size;

            break;
        }

        /*!< No return */
        __pq_buffer_read(sptr_pq, &sgtc_msg.len, size);
        
        /*!< Jump over old meesages (sizeof(sgtc_msg.len) + sgtc_msg.len) */
        sptr_pq->tail = (sptr_pq->tail + sgtc_msg.len + size) % sptr_pq->tot_len;
        sptr_pq->len -= (sgtc_msg.len + size);
    }
    
    /*!< Enough space to save new message (len_size + len): sptr_pq->len + require_size < sptr_pq->tot_len */
    __pq_buffer_write(sptr_pq, &real_len, len_size);
    sptr_pq->head = (sptr_pq->head + len_size) % sptr_pq->tot_len;

    __pq_buffer_write(sptr_pq, data, len);
    sptr_pq->head = (sptr_pq->head + len) % sptr_pq->tot_len;

    /*!< If len is not in 4 bytes aligned, fill 0 to it's tail */
    if (real_len != len)
    {
        __pq_buffer_write(sptr_pq, fix_data, real_len - len);
        sptr_pq->head = (sptr_pq->head + real_len - len) % sptr_pq->tot_len;
    }

    sptr_pq->len += require_size;
    return (kssize_t)require_size;
}

/*!
 * @brief   Read
 * @param   sptr_pq
 * @param   buffer: used to save data read
 * @retval  lenth
 * @note    none
 */
kssize_t pq_message_read(struct pq_buffer *sptr_pq, void *data, kusize_t len)
{
    kusize_t require_size = CMP_MIN2(len, sptr_pq->len);
    kusize_t len_size = sizeof(len);
    struct pq_message sgtc_msg;
    kusize_t count = 0;

    if (!data || 
        (require_size <= len_size))
        return 0;

    /*!< Read all */
    if (require_size == sptr_pq->len)
        return pq_buffer_read(sptr_pq, data, require_size);

    /*!< len < sptr_pq->len */
    while (count < require_size)
    {
        __pq_buffer_read(sptr_pq, &sgtc_msg.len, len_size);

        /*!< require_size is too small */
        if ((count + sgtc_msg.len + len_size) > require_size)
            break;

        sptr_pq->tail = (sptr_pq->tail + len_size) % sptr_pq->tot_len;
        memcpy(data, &sgtc_msg.len, len_size);

        __pq_buffer_read(sptr_pq, data + len_size, sgtc_msg.len);
        sptr_pq->tail = (sptr_pq->tail + sgtc_msg.len) % sptr_pq->tot_len;

        sptr_pq->len -= (len_size + sgtc_msg.len);
        data += (len_size + sgtc_msg.len);
        count += (sgtc_msg.len + len_size);
    }

    return (kssize_t)count;
}

/* end of file */
