/*
 * Radix Tree Interface
 *
 * File Name:   radix_tree.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.06
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __RADIX_TREE_H
#define __RADIX_TREE_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <kernel/spinlock.h>

/*!< The defines */
#define MAX_BRANCH                                      (3)

typedef struct radix_link 
{
    kuint32_t depth;

} srt_radix_link_t;

typedef struct radix_node 
{
    struct radix_node *sptr_parent;
    struct radix_node *sgtc_branches[1 << MAX_BRANCH];  /*!< 00, 01, 10, 11 */

    struct radix_link *sptr_link;

} srt_radix_node_t;

typedef struct radix_tree 
{
    kuint16_t (*get) (kuint32_t);
    void *(*alloc) (kusize_t size);
    void (*free) (void *ptr);

    struct radix_node sgtc_node;
    struct spin_lock sgtc_lock;

} srt_radix_tree_t;

/*!< The functions */
extern kuint16_t get_radix_node_branch(kuint32_t number);
extern struct radix_node *allocate_radix_node(struct radix_tree *sptr_tree, struct radix_node *sptr_par);
extern struct radix_node *find_radix_node(struct radix_tree *sptr_tree, kuint32_t number);
extern struct radix_link *radix_tree_look_up(struct radix_tree *sptr_tree, kuint32_t number);
extern void radix_tree_add(struct radix_tree *sptr_tree, kuint32_t number, struct radix_link *sptr_link);
extern void radix_tree_del(struct radix_tree *sptr_tree, kuint32_t number);

/*!< The defines */
#define DECLARE_RADIX_TREE(name, alloc_func, free_func) \
    struct radix_tree name = {   \
        .get = get_radix_node_branch,    \
        .alloc = alloc_func,    \
        .free = free_func,  \
        .sgtc_node = {},    \
        .sgtc_lock = SPIN_LOCK_INIT(),   \
    }

#define foreach_radix_tree(sptr_node, sptr_tree, offset) \
    for (sptr_node = &(sptr_tree)->sgtc_node, offset = 0; \
         sptr_node; \
         sptr_node = (sptr_node)->sgtc_branches[offset])

#define radix_tree_entry(pos, type, member) \
    (pos ? mr_container_of(pos, type, member) : mr_nullptr)

#define radix_tree_next_entry(tree, type, member, number)  \
({  \
    struct radix_link *sptr_link = radix_tree_look_up(tree, number);  \
    sptr_link ? mr_container_of(sptr_link, type, member) : mr_nullptr; \
})

#ifdef __cplusplus
    }
#endif

#endif /* __RADIX_TREE_H */
