/*
 * List Defines (Singly List and Doubly List)
 *
 * File Name:   list_types.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.26
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __LIST_TYPES_H
#define __LIST_TYPES_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include "generic.h"

/*!< The defines */
/*!< Singly List */
struct list
{
    struct list *sptr_next;
};
typedef struct list srt_list_t;

#define LIST_INIT()	\
{	\
    .sptr_next = mr_nullptr,	\
}

#define DECLARE_LIST(list)	\
    struct list list = LIST_INIT

#define DECLARE_LIST_PTR(list)	\
    struct list *list = mr_nullptr

/*!< search */
#define foreach_list_next(head, tail, ptr_list, ptr_next)	\
    for (ptr_list = head; (ptr_list != tail); ptr_list = (ptr_list)->ptr_next)

#define foreach_list_from_head(head, tail, ptr_list)			foreach_list_next(head, tail, ptr_list, sptr_next)
#define foreach_list_from_next(head, tail, ptr_list)			foreach_list_next((head)->sptr_next, tail, ptr_list, sptr_next)

/*!< searching for "tail = head" */
#define foreach_list_even_head(head, ptr_list)					foreach_list_from_head(head, head, ptr_list)
#define foreach_list_even_next(head, ptr_list)					foreach_list_from_next(head, head, ptr_list)

/*!< searching for "tail = mr_nullptr" */
#define foreach_list_odd(head, ptr_list, ptr_next)				foreach_list_next(head, mr_nullptr, ptr_list, ptr_next)
#define foreach_list_odd_head(head, ptr_list)					foreach_list_from_head(head, mr_nullptr, ptr_list)
#define foreach_list_odd_next(head, ptr_list)					foreach_list_from_next(head, mr_nullptr, ptr_list)

#define mr_list_add_tail(tail, list)							do { (list)->sptr_next = mr_nullptr; (tail)->sptr_next = (list); } while (0)
#define mr_list_add_head(head, list)							do { (list)->sptr_next = (head)->sptr_next; (head)->sptr_next = (list); } while (0)
#define mr_list_del_any(prev, list)							    do { (prev)->sptr_next = (list)->sptr_next; } while (0)

/*!< Doubly List */
struct list_head
{
    struct list_head *sptr_prev;
    struct list_head *sptr_next;
};
typedef struct list_head srt_list_head_t;

#define LIST_HEAD_INIT(list)	\
{	\
    .sptr_prev = (list),	\
    .sptr_next = (list),	\
}

#define DECLARE_LIST_HEAD(list)	\
                        struct list_head list = LIST_HEAD_INIT(&list)

/*!< list pointer defines */
#define DECLARE_LIST_HEAD_PTR_INIT(ptr_list, list)	\
                        struct list_head *ptr_list = list;
#define DECLARE_LIST_HEAD_PTR(ptr_list)							DECLARE_LIST_HEAD_PTR_INIT(ptr_list, mr_nullptr)

/*!< get every member from list */
#define mr_next_list_head(head, list)							(list = ((list)->sptr_next != head) ? (list)->sptr_next : mr_nullptr)

/*!< judge if the list has only head */
#define IS_LIST_HEAD_SELF_HEAD(ptr_list)	\
    (((ptr_list)->sptr_prev == (ptr_list)) && ((ptr_list)->sptr_next == (ptr_list)))
#define mr_list_head_empty(head)								IS_LIST_HEAD_SELF_HEAD(head)

/*!< get parent of every member from list */
#define mr_list_head_parent(parent, list, type, member)	\
({	\
    mr_next_list_head(parent, list);	\
    (isValid(list)) ? mr_container_of(list, type, member) : mr_nullptr;	\
})

/*!< two-way retrieval */
#define foreach_list_head(ptr_left, ptr_right, ptr_head)	\
    for (ptr_right = (ptr_head)->sptr_next, ptr_left = (ptr_head)->sptr_prev;	\
        ((ptr_right != ptr_head) && (ptr_left != ptr_head));	\
        ptr_right = (ptr_right)->sptr_next, ptr_left = (ptr_left)->sptr_prev)

/*!< search forward */
#define foreach_list_head_forward(ptr_list, ptr_head)	\
    for (ptr_list = (ptr_head)->sptr_next; ptr_list != ptr_head; ptr_list = (ptr_list)->sptr_next)

/*!< search backward */
#define foreach_list_head_backward(ptr_list, ptr_head)	\
    for (ptr_list = (ptr_head)->sptr_prev; ptr_list != ptr_head; ptr_list = (ptr_list)->sptr_prev)

#define mr_list_first_entry(head, type, member)				    mr_container_of((head)->sptr_next, type, member)
#define mr_list_last_entry(head, type, member)					mr_container_of((head)->sptr_prev, type, member)
#define mr_list_next_entry(pos, member)						    mr_container_of((pos)->member.sptr_next, typeof(*(pos)), member)
#define mr_list_prev_entry(pos, member)						    mr_container_of((pos)->member.sptr_prev, typeof(*(pos)), member)

#define mr_list_head_until(pos, head, member)					((pos)->member.sptr_next == (head))
#define mr_list_first_valid_entry(ptr, type, member)			(mr_list_head_empty(ptr) ? mr_nullptr : mr_list_first_entry(ptr, type, member))

/*!<
 * usage: 
 * 	struct dev {
 * 		struct list_head list;
 * 	}
 * 
 *  struct devices {
 * 		struct list_head head;
 * 	}
 * 
 *  struct dev sgtc_dev;
 *  struct devices sgtc_devices;
 * 
 * 	===> list_head_add_tail(&sgtc_devices.head, &sgtc_dev.list);
 *  ===> struct dev *sptr_dev;
 *  ===> foreach_list_entry(sptr_dev, &sgtc_devices.head, list)
 */
#define foreach_list_next_entry(pos, head, member)	\
    for (pos = mr_list_first_entry(head, typeof(*pos), member);	\
         &(pos->member) != (head);	\
         pos = mr_list_next_entry(pos, member))

#define foreach_list_prev_entry(pos, head, member)	\
    for (pos = mr_list_last_entry(head, typeof(*pos), member);	\
         &(pos->member) != (head);	\
         pos = mr_list_prev_entry(pos, member))

/* get list and next list, and then delete current list from list_head */
#define foreach_list_next_entry_safe(pos, temp, head, member)	\
    for (pos = mr_list_first_entry(head, typeof(*pos), member),	\
        temp = mr_list_next_entry(pos, member);	\
         &(pos->member) != (head);	\
         pos = temp, temp = mr_list_next_entry(temp, member))

/* get list and prev list, and then delete current list from list_head */
#define foreach_list_prev_entry_safe(pos, temp, head, member)	\
    for (pos = mr_list_last_entry(head, typeof(*pos), member),	\
        temp = mr_list_prev_entry(pos, member);	\
         &(pos->member) != (head);	\
         pos = temp, temp = mr_list_prev_entry(temp, member))

/*!< API functions */
/*!
 * @brief   initialize list
 * @param   sptr_list
 * @retval  none
 * @note    none
 */
static inline void init_list_head(struct list_head *sptr_list)
{
    sptr_list->sptr_next = sptr_list;
    sptr_list->sptr_prev = sptr_list;
}

/*!
 * @brief   check if the member is existed in target list
 * @param   sptr_head, sptr_list
 * @retval  errno
 * @note    none
 */
static inline kint32_t list_head_for_each(struct list_head *sptr_head, struct list_head *sptr_list)
{
    struct list_head *ptr_right;
    struct list_head *ptr_left;

    foreach_list_head(ptr_left, ptr_right, sptr_head)
    {
        if ((ptr_left == sptr_list) || (ptr_right == sptr_list))
            return ER_NORMAL;

        if (ptr_left == ptr_right)
            break;
    }

    return -ER_ERROR;
}

/*!
 * @brief   insert a new member behind list head
 * @param   sptr_head, sptr_list
 * @retval  none
 * @note    none
 */
static inline void list_head_add_head(struct list_head *sptr_head, struct list_head *sptr_list)
{
    struct list_head *ptr_next = sptr_head->sptr_next;

    sptr_list->sptr_prev = sptr_head;
    sptr_head->sptr_next = sptr_list;

    sptr_list->sptr_next = ptr_next;
    ptr_next->sptr_prev	 = sptr_list;
}

/*!
 * @brief   insert a new member before list head
 * @param   sptr_head, sptr_list
 * @retval  none
 * @note    none
 */
static inline void list_head_add_tail(struct list_head *sptr_head, struct list_head *sptr_list)
{
    struct list_head *ptr_prev = sptr_head->sptr_prev;

    sptr_list->sptr_prev = ptr_prev;
    ptr_prev->sptr_next	 = sptr_list;

    sptr_list->sptr_next = sptr_head;
    sptr_head->sptr_prev = sptr_list;
}

/*!
 * @brief   delete a member behind list head
 * @param   sptr_head
 * @retval  none
 * @note    none
 */
static inline void list_head_del_head(struct list_head *sptr_head)
{
    struct list_head *ptr_next = sptr_head->sptr_next->sptr_next;

    sptr_head->sptr_next = ptr_next;
    ptr_next->sptr_prev	 = sptr_head;
}

/*!
 * @brief   delete a member behind list head
 * @param   sptr_head
 * @retval  none
 * @note    none
 */
static inline void list_head_del_tail(struct list_head *sptr_head)
{
    struct list_head *ptr_prev = sptr_head->sptr_prev->sptr_prev;

    sptr_head->sptr_prev = ptr_prev;
    ptr_prev->sptr_next	 = sptr_head;
}

/*!
 * @brief   delete a member
 * @param   sptr_list
 * @retval  none
 * @note    none
 */
static inline void list_head_del(struct list_head *sptr_list)
{
    struct list_head *ptr_prev = sptr_list->sptr_prev;
    struct list_head *ptr_next = sptr_list->sptr_next;

    ptr_prev->sptr_next	= ptr_next;
    ptr_next->sptr_prev	= ptr_prev;

    init_list_head(sptr_list);
}

/*!
 * @brief   delete a member which is in the middle of list
 * @param   sptr_list
 * @retval  none
 * @note    none
 */
static inline void list_head_del_safe(struct list_head *sptr_head, struct list_head *sptr_list)
{
    struct list_head *ptr_prev = sptr_list->sptr_prev;
    struct list_head *ptr_next = sptr_list->sptr_next;

    if (!list_head_for_each(sptr_head, sptr_list))
    {
        ptr_prev->sptr_next	= ptr_next;
        ptr_next->sptr_prev	= ptr_prev;

        init_list_head(sptr_list);
    }
}

/*!
 * @brief   copy sptr_src to sptr_dst (e.g. split sptr_src and sptr_dst)
 * @param   sptr_src: source list
 * @param   sptr_dst: destination list
 * @retval  none
 * @note    none
 */
static inline void list_head_splice_init(struct list_head *sptr_src, struct list_head *sptr_dst)
{
    struct list_head *sptr_prev;

    /*!< 
     * sptr_src->sptr_next ---> sptr_dst
     * sptr_dst->sptr_prev ---> sptr_src
     */
    list_head_add_head(sptr_src, sptr_dst);

    /*!< delete sptr_src: sptr_dst becomes to the head */
    sptr_prev = sptr_src->sptr_prev;
    sptr_dst->sptr_prev = sptr_prev;
    sptr_prev->sptr_next = sptr_dst;

    /*!< delete all tails */
    init_list_head(sptr_src);
}

#ifdef __cplusplus
    }
#endif

#endif  /* __LIST_TYPES_H */
