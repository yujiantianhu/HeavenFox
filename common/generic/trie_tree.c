/*
 * Trie Tree General Function
 *
 * File Name:   trie_tree.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.05
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/generic.h>
#include <common/trie_tree.h>
#include <common/io_stream.h>
#include <platform/fwk_mempool.h>

/*!< The defines */
#define IS_STRING_END(offset)           (-1 == (offset))
#define IS_STRING_ERR(offset)           (-2 == (offset))

/*!< API function */
__weak ksint32_t get_trie_node_branch(ksint8_t ch)
{
    if (!ch)
        return -1;

    else if ((ch < ' ') || (ch > '~'))
        return -2;

    return (ch - ' ');
}

struct trie_node **create_trie_branch(struct trie_tree *sprt_tree, struct trie_node *sprt_node, kuint32_t size)
{
    struct trie_node **sprt_branches;

    sprt_branches = (struct trie_node **)sprt_tree->alloc(size * sizeof(*sprt_branches));
    if (!isValid(sprt_branches))
        return mrt_nullptr;

    for (kuint32_t i = 0; i < size; i++)
        sprt_branches[i] = mrt_nullptr;

    if (sprt_node)
        sprt_node->sprt_branches = sprt_branches;

    return sprt_branches;
}

struct trie_node *allocate_trie_node(struct trie_tree *sprt_tree, struct trie_node *sprt_par, struct trie_node **sprt_branches)
{
    struct trie_node *sprt_node;

    sprt_node = (struct trie_node *)sprt_tree->alloc(sizeof(*sprt_node));
    if (!isValid(sprt_node))
        return mrt_nullptr;
    
    sprt_node->sprt_parent = sprt_par;
    sprt_node->sprt_branches = sprt_branches;
    sprt_node->sprt_link = mrt_nullptr;

    return sprt_node;
}

struct trie_node *find_trie_node(struct trie_tree *sprt_tree, const char *name)
{
    struct trie_node *sprt_node;
    const kstring_t *str = name;
    ksint32_t offset = 0;

    foreach_trie_tree(sprt_node, sprt_tree, offset) {
        offset = sprt_tree->get(*(str++));

        if (IS_STRING_END(offset))
            break;
        else if (IS_STRING_ERR(offset))
            return ERR_PTR(-NR_IS_FAULT);
    }

    if (sprt_node)
        return sprt_node->sprt_link ? sprt_node : mrt_nullptr;

    print_debug("find node failed, name is: %s\n", name);

    return mrt_nullptr;
}

struct trie_link *trie_tree_look_up(struct trie_tree *sprt_tree, const kstring_t *name)
{
    struct trie_node *sprt_node;

    sprt_node = find_trie_node(sprt_tree, name);
    if (IS_ERR(sprt_node) || !sprt_node)
        return mrt_nullptr;

    return sprt_node->sprt_link;
}

void trie_node_add(struct trie_tree *sprt_tree, const kstring_t *name, struct trie_link *sprt_link)
{
    struct trie_node *sprt_node, *sprt_temp;
    const kstring_t *str = name;
    kuint32_t lenth = strlen(name);
    kuint32_t i;
    ksint32_t offset;

    if (!sprt_link)
        return;

    sprt_node = find_trie_node(sprt_tree, name);

    /*!< found or error */
    if (sprt_node)
        return;

    for (i = 0, sprt_node = &sprt_tree->sgrt_node; i < lenth; i++) {
        offset = sprt_tree->get(*(str + i));

        if (!sprt_node->sprt_branches) {
            if (!create_trie_branch(sprt_tree, sprt_node, sprt_tree->size))
                return;
        }

        if (!sprt_node->sprt_branches[offset]) {
            sprt_temp = allocate_trie_node(sprt_tree, sprt_node, mrt_nullptr);
            if (!isValid(sprt_temp))
                return;

            sprt_node->sprt_branches[offset] = sprt_temp;
        }

        print_debug("%s: i = %d, sprt_node: %p, offset: %c\n", __FUNCTION__, i, sprt_node->sprt_branches[offset], offset + 'a');

        sprt_link->depth = i;
        sprt_node = sprt_node->sprt_branches[offset];
    }

    sprt_node->sprt_link = sprt_link;

    print_debug("add new node succeuss, name is: %s\n", name);
}

static void __del_trie_node(struct trie_tree *sprt_tree, struct trie_node *sprt_node, struct trie_node *sprt_child)
{
    kuint32_t i, count = 0;
    
    if (!sprt_node)
        return;

	/*!< just for leaf node */
    if (!sprt_child && sprt_node->sprt_branches)
        return;

    if (sprt_node->sprt_branches) {
        for (i = 0; i < sprt_tree->size; i++) {
            if (sprt_node->sprt_branches[i] == sprt_child) {
                sprt_node->sprt_branches[i] = mrt_nullptr;

                if (count)
                    goto out;
            }

            if (sprt_node->sprt_branches[i])
                count++;
        }

        if (count)
            goto out;

        sprt_tree->free(sprt_node->sprt_branches);
        sprt_node->sprt_branches = mrt_nullptr;
    }

    if (!sprt_node->sprt_link) {
        __del_trie_node(sprt_tree, sprt_node->sprt_parent, sprt_node);

        if (sprt_node != &sprt_tree->sgrt_node)
            sprt_tree->free(sprt_node);
    }

out:
    return;
}

void trie_node_del(struct trie_tree *sprt_tree, const kstring_t *name)
{
    struct trie_node *sprt_node;
    const kstring_t *str = name;
    kuint32_t lenth = strlen(name);
    kuint32_t i;
    ksint32_t offset = 0;

    for (i = 0, sprt_node = &sprt_tree->sgrt_node; i < lenth; i++) {
        if (!sprt_node || !sprt_node->sprt_branches)
            return;
        
        offset = sprt_tree->get(*(str + i));
        if (IS_STRING_ERR(offset))
            return;

        sprt_node = sprt_node->sprt_branches[offset];
    }

    if (!sprt_node ||
        !sprt_node->sprt_link ||
        (sprt_node == &sprt_tree->sgrt_node))
        return;

    sprt_node->sprt_link = mrt_nullptr;
    __del_trie_node(sprt_tree, sprt_node, mrt_nullptr);
}

/*!< end of file */
