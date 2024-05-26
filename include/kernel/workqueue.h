/*
 * Kernel Thread Work Queue Defines
 *
 * File Name:   workqueue.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.25
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __KEL_WORKQUEUE_H_
#define __KEL_WORKQUEUE_H_

/*!< The includes */
#include <kernel/kernel.h>
#include <kernel/thread.h>
#include <kernel/mutex.h>

/*!< The defines */
typedef struct workqueue srt_workqueue_t;
typedef void (*func_work_t) (struct workqueue *);

typedef struct workqueue
{
    func_work_t func;
    kuaddr_t data;

    struct list_head sgrt_link;

} srt_workqueue_t;

#define INIT_WORK(sprt_wq, _func)  \
    do {    \
        (sprt_wq)->func = _func; \
        (sprt_wq)->data = 0; \
        init_list_head(&(sprt_wq)->sgrt_link);  \
    } while (0)

#define DECLARE_WORK(name, _func)  \
    struct workqueue name = {   \
        .func = _func,   \
        .data = 0,   \
        .sgrt_link = LIST_HEAD_INIT(&(name).sgrt_link), \
    }

typedef struct workqueue_head
{
    struct list_head sgrt_work;

} srt_workqueue_head_t;

#define INIT_WORKQUEUE_HEAD(sprt_wqh)   \
    do {    \
        init_list_head(&(sprt_wqh)->sgrt_link); \
    } while (0)

#define DECLARE_WORKQUEUE(name) \
    struct workqueue_head name = {  \
        .sgrt_work = LIST_HEAD_INIT(&(name).sgrt_work),  \
    }

#define foreach_workqueue_safe(sprt_wq, sprt_temp, sprt_wqh)    \
    foreach_list_next_entry_safe(sprt_wq, sprt_temp, &(sprt_wqh)->sgrt_work, sgrt_link)

/*!< The functions */
TARGET_EXT void schedule_work(struct workqueue *sprt_wq);

/*!< API functions */
static inline void queue_work(struct workqueue_head *sprt_wqh, struct workqueue *sprt_wq)
{
    if (!sprt_wqh || !sprt_wq)
        return;

    if (!list_head_for_each(&sprt_wqh->sgrt_work, &sprt_wq->sgrt_link))
        return;

    list_head_add_tail(&sprt_wqh->sgrt_work, &sprt_wq->sgrt_link);
}

static inline void detach_work(struct workqueue *sprt_wq)
{
    if (!sprt_wq)
        return;

    list_head_del(&sprt_wq->sgrt_link);
}

static inline void detach_work_safe(struct workqueue_head *sprt_wqh, struct workqueue *sprt_wq)
{
    if (!sprt_wqh || !sprt_wq)
        return;

    list_head_del_safe(&sprt_wqh->sgrt_work, &sprt_wq->sgrt_link);
}

static inline kbool_t is_workqueue_empty(struct workqueue_head *sprt_wqh)
{
    return mrt_list_head_empty(&sprt_wqh->sgrt_work);
}

#endif /* __KEL_WORKQUEUE_H_ */
