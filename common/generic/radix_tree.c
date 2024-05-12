/*
 * Radix Tree General Function
 *
 * File Name:   radix_tree.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.06
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/generic.h>
#include <common/radix_tree.h>
#include <common/io_stream.h>
#include <platform/fwk_mempool.h>

/*!< The defines */

/*!< API functions */
static kuint16_t radix_tree_numlen(kuint32_t number)
{
    ksint16_t i;
    kuint16_t count = 0;

    for (i = 31; i >= 0; i--) {
        if (number & (1UL << i))
            break;

        count++;
    }

    count = mrt_ralign(count, MAX_BRANCH);

    return (32 - count);
}

struct radix_node *allocate_radix_node(struct radix_tree *sprt_tree, struct radix_node *sprt_par)
{
    struct radix_node *sprt_node;

    sprt_node = (struct radix_node *)sprt_tree->alloc(sizeof(*sprt_node));
    if (!isValid(sprt_node))
        return mrt_nullptr;
    
    sprt_node->sprt_parent = sprt_par;
    sprt_node->sprt_link = mrt_nullptr;

    return sprt_node;
}

struct radix_node *find_radix_node(struct radix_tree *sprt_tree, kuint32_t number)
{
    struct radix_node *sprt_node;
    kuint32_t value = number;
    kuint16_t lenth = radix_tree_numlen(number);
    kuint16_t i = 0, offset = 0;

    foreach_radix_tree(sprt_node, sprt_tree, offset) {
        if (i >= lenth)
            break;

        i += MAX_BRANCH;
        offset = sprt_tree->get(value);
        value >>= MAX_BRANCH;
    }

    if (sprt_node)
        return sprt_node->sprt_link ? sprt_node : mrt_nullptr;

    print_debug("find node failed, index is: 0x%x\n", number);

    return mrt_nullptr;
}

struct radix_link *radix_tree_look_up(struct radix_tree *sprt_tree, kuint32_t number)
{
    struct radix_node *sprt_node;

    sprt_node = find_radix_node(sprt_tree, number);
    if (!sprt_node)
        return mrt_nullptr;

    return sprt_node->sprt_link;
}

void radix_tree_add(struct radix_tree *sprt_tree, kuint32_t number, struct radix_link *sprt_link)
{
    struct radix_node *sprt_node, *sprt_temp;
    kuint32_t value = number;
    kuint16_t lenth = radix_tree_numlen(number);
    kuint16_t i, offset = 0;

    if (!sprt_link)
        return;

    if (find_radix_node(sprt_tree, number))
        return;

    for (i = 0, sprt_node = &sprt_tree->sgrt_node; i < lenth; i += MAX_BRANCH) {
        offset = sprt_tree->get(value);
        value >>= MAX_BRANCH;

        if (!sprt_node->sgrt_branches[offset]) {
            sprt_temp = allocate_radix_node(sprt_tree, sprt_node);
            if (!isValid(sprt_temp))
                return;

            sprt_node->sgrt_branches[offset] = sprt_temp;
        }

        print_debug("%s: lenth: %d, i = %d, sprt_node: %p, offset: 0x%x\n", __FUNCTION__, lenth, i, sprt_node->sgrt_branches[offset], offset);

        sprt_link->depth = i;
        sprt_node = sprt_node->sgrt_branches[offset];
    }

    sprt_node->sprt_link = sprt_link;

    print_debug("add new node successfully, index is: 0x%x\n", number);
}

static void __del_radix_node(struct radix_tree *sprt_tree, struct radix_node *sprt_node, struct radix_node *sprt_child)
{
    kuint32_t i, count = 0;
    
    if (!sprt_node)
        return;

    for (i = 0; i < (1 << MAX_BRANCH); i++) {
        if (sprt_node->sgrt_branches[i] == sprt_child) {
            sprt_node->sgrt_branches[i] = mrt_nullptr;

            if (count)
                goto out;
        }

        if (sprt_node->sgrt_branches[i])
            count++;
    }

    if (count)
        goto out;

    if (!sprt_node->sprt_link) {
        __del_radix_node(sprt_tree, sprt_node->sprt_parent, sprt_node);

        if (sprt_node != &sprt_tree->sgrt_node)
            sprt_tree->free(sprt_node);
    }

out:
    return;
}

void radix_tree_del(struct radix_tree *sprt_tree, kuint32_t number)
{
    struct radix_node *sprt_node;
    kuint32_t value = number;
    kuint32_t lenth = radix_tree_numlen(number);
    kuint32_t i, offset = 0;

    for (i = 0, sprt_node = &sprt_tree->sgrt_node; i < lenth; i += MAX_BRANCH) {
        if (!sprt_node)
            return;
        
        offset = sprt_tree->get(value);
        value >>= MAX_BRANCH;
        sprt_node = sprt_node->sgrt_branches[offset];
    }

    if (!sprt_node ||
        !sprt_node->sprt_link ||
        (sprt_node == &sprt_tree->sgrt_node)) {
        print_debug("nothing needs to delete\n");
        return;
    }

    sprt_node->sprt_link = mrt_nullptr;
    __del_radix_node(sprt_tree, sprt_node, mrt_nullptr);
}

/*!< end of file */
