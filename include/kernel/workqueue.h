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

#ifndef __KERNEL_WORKQUEUE_H_
#define __KERNEL_WORKQUEUE_H_

#ifdef __cplusplus
    extern "C" {
#endif

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
    kutype_t data;

    struct list_head sgtc_link;

} srt_workqueue_t;

#define INIT_WORK(sptr_wq, _func)  \
    do {    \
        (sptr_wq)->func = _func; \
        (sptr_wq)->data = 0; \
        init_list_head(&(sptr_wq)->sgtc_link);  \
    } while (0)

#define DECLARE_WORK(name, _func)  \
    struct workqueue name = {   \
        .func = _func,   \
        .data = 0,   \
        .sgtc_link = LIST_HEAD_INIT(&(name).sgtc_link), \
    }

typedef struct workqueue_head
{
    struct list_head sgtc_work;

} srt_workqueue_head_t;

#define INIT_WORKQUEUE_HEAD(sptr_wqh)   \
    do {    \
        init_list_head(&(sptr_wqh)->sgtc_link); \
    } while (0)

#define DECLARE_WORKQUEUE(name) \
    struct workqueue_head name = {  \
        .sgtc_work = LIST_HEAD_INIT(&(name).sgtc_work),  \
    }

#define foreach_workqueue_safe(sptr_wq, sptr_temp, sptr_wqh)    \
    foreach_list_next_entry_safe(sptr_wq, sptr_temp, &(sptr_wqh)->sgtc_work, sgtc_link)

/*!< The functions */
extern void schedule_work(struct workqueue *sptr_wq);

/*!< API functions */
/*!
 * @brief   add sptr_wq to the list of sptr_wqh
 * @param   sptr_wqh, sptr_wq
 * @retval  none
 * @note    none
 */
static inline void queue_work(struct workqueue_head *sptr_wqh, struct workqueue *sptr_wq)
{
    if (!sptr_wqh || !sptr_wq)
        return;

    if (!list_head_for_each(&sptr_wqh->sgtc_work, &sptr_wq->sgtc_link))
        return;

    list_head_add_tail(&sptr_wqh->sgtc_work, &sptr_wq->sgtc_link);
}

/*!
 * @brief   del sptr_wq from the list
 * @param   sptr_wq
 * @retval  none
 * @note    none
 */
static inline void detach_work(struct workqueue *sptr_wq)
{
    if (!sptr_wq)
        return;

    list_head_del(&sptr_wq->sgtc_link);
}

/*!
 * @brief   add sptr_wq to the list of sptr_wqh
 * @param   sptr_wqh, sptr_wq
 * @retval  none
 * @note    none
 */
static inline void detach_work_safe(struct workqueue_head *sptr_wqh, struct workqueue *sptr_wq)
{
    if (!sptr_wqh || !sptr_wq)
        return;

    list_head_del_safe(&sptr_wqh->sgtc_work, &sptr_wq->sgtc_link);
}

/*!
 * @brief   check if the list of sptr_wqh is empty
 * @param   sptr_wqh
 * @retval  empty(true) / false
 * @note    none
 */
static inline kbool_t is_workqueue_empty(struct workqueue_head *sptr_wqh)
{
    return mr_list_head_empty(&sptr_wqh->sgtc_work);
}

#ifdef __cplusplus
    }
#endif

#endif /* __KERNEL_WORKQUEUE_H_ */
