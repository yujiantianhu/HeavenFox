/*
 * PQ Buffer API Declare
 *
 * File Name:   buffer.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.05.19
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __BUFFER_H
#define __BUFFER_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>

/*!< The defines */
enum __ERT_PQ_BUF_TYPE
{
    NR_PQ_BUF_DROP = 0,
    NR_PQ_BUF_RING,
};

struct pq_buffer
{
    kint32_t type;
    void *buf_start;

    kint32_t head;
    kint32_t tail;
    kint32_t len;

    kusize_t tot_len;
};

#define PQ_BUFFER_INIT(_type, _buf_start, _buf_len)    \
    {   \
        .type = _type,  \
        .buf_start = _buf_start,    \
        .head = 0,  \
        .tail = 0,  \
        .len = 0,   \
        .tot_len = _buf_len \
    }

#define PQ_DROP_BUFFER_INIT(_buf_start, _buf_len)               PQ_BUFFER_INIT(NR_PQ_BUF_DROP, _buf_start, _buf_len)
#define PQ_RING_BUFFER_INIT(_buf_start, _buf_len)               PQ_BUFFER_INIT(NR_PQ_BUF_RING, _buf_start, _buf_len)

#define mr_pq_buffer_size(sptr_pqb)                             ((sptr_pqb)->len)

/*!< Per item */
struct pq_message
{
    kusize_t len;
    kubyte_t data[0];
};

/*!< The functions */
extern struct pq_buffer *pq_buffer_create(kint32_t type, kusize_t buf_len);
extern void pq_buffer_destroy(struct pq_buffer *sptr_pq);
extern kssize_t pq_buffer_write(struct pq_buffer *sptr_pq, const void *data, kusize_t len);
extern kssize_t pq_buffer_read(struct pq_buffer *sptr_pq, void *buffer, kusize_t len);
extern kssize_t pq_message_write(struct pq_buffer *sptr_pq, const void *data, kusize_t len);
extern kssize_t pq_message_read(struct pq_buffer *sptr_pq, void *data, kusize_t len);

#ifdef __cplusplus
    }
#endif

#endif /* __BUFFER_H */
