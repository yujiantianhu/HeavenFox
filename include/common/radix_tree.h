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

/*!< The includes */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>

/*!< The defines */
#define MAX_BRANCH                                      (4)

typedef struct radix_link {
    kuint32_t depth;

} srt_radix_link_t;

typedef struct radix_node {
    struct radix_node *sprt_parent;
    struct radix_node *sgrt_branches[1 << MAX_BRANCH];  /*!< 00, 01, 10, 11 */

    struct radix_link *sprt_link;

} srt_radix_node_t;

typedef struct radix_tree {
    kuint16_t (*get) (kuint32_t);
    void *(*alloc) (kusize_t size);
    void (*free) (void *ptr);

    struct radix_node sgrt_node;

} srt_radix_tree_t;

/*!< The functions */
TARGET_EXT struct radix_node *allocate_radix_node(struct radix_tree *sprt_tree, struct radix_node *sprt_par);
TARGET_EXT struct radix_node *find_radix_node(struct radix_tree *sprt_tree, kuint32_t number);
TARGET_EXT struct radix_link *radix_tree_look_up(struct radix_tree *sprt_tree, kuint32_t number);
TARGET_EXT void radix_tree_add(struct radix_tree *sprt_tree, kuint32_t number, struct radix_link *sprt_link);
TARGET_EXT void radix_tree_del(struct radix_tree *sprt_tree, kuint32_t number);

/*!< API functions */
static inline kuint16_t get_radix_node_branch(kuint32_t number)
{
    return (number & ((1 << MAX_BRANCH) - 1));
}

#define DECLARE_RADIX_TREE(name, alloc_func, free_func) \
    struct radix_tree name = {   \
        .get = get_radix_node_branch,    \
        .alloc = alloc_func,    \
        .free = free_func,  \
        .sgrt_node = { NULL, { NULL }, NULL },    \
    }

#define foreach_radix_tree(sprt_node, sprt_tree, offset) \
    for (sprt_node = &(sprt_tree)->sgrt_node, offset = 0; \
         sprt_node; \
         sprt_node = (sprt_node)->sgrt_branches[offset])

#define radix_tree_entry(tree, type, member, number)  \
({  \
    void *ptr_member = (void *)radix_tree_look_up(tree, number);  \
    ptr_member ? mrt_container_of(ptr_member, type, member) : NULL; \
})

#endif /* __RADIX_TREE_H */
