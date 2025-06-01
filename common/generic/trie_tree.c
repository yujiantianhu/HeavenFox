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
#include <common/api_string.h>
#include <platform/fwk_mempool.h>

/*!< The defines */
#define IS_STRING_END(offset)           (-1 == (offset))
#define IS_STRING_ERR(offset)           (-2 == (offset))

/*!< API function */
/*!
 * @brief   get the branch location of character
 * @param   ch
 * @retval  none
 * @note    for example: ch == '9', the location is 9
 */
__weak kint32_t get_trie_node_branch(kchar_t ch)
{
    if (!ch)
        return -1;

    else if ((ch < ' ') || (ch > '~'))
        return -2;

    return (ch - ' ');
}

/*!
 * @brief   create a group of branches of trie_node
 * @param   sptr_tree, sptr_node
 * @retval  none
 * @note    none
 */
struct trie_node **create_trie_branch(struct trie_tree *sptr_tree, struct trie_node *sptr_node, kuint32_t size)
{
    struct trie_node **sptr_branches;

    sptr_branches = (struct trie_node **)sptr_tree->alloc(size * sizeof(*sptr_branches));
    if (!isValid(sptr_branches))
        return mr_nullptr;

    for (kuint32_t i = 0; i < size; i++)
        sptr_branches[i] = mr_nullptr;

    if (sptr_node)
        sptr_node->sptr_branches = sptr_branches;

    return sptr_branches;
}

/*!
 * @brief   allocate a new trie_node
 * @param   sptr_tree, sptr_par, sptr_branches
 * @retval  none
 * @note    none
 */
struct trie_node *allocate_trie_node(struct trie_tree *sptr_tree, struct trie_node *sptr_par, struct trie_node **sptr_branches)
{
    struct trie_node *sptr_node;

    sptr_node = (struct trie_node *)sptr_tree->alloc(sizeof(*sptr_node));
    if (!isValid(sptr_node))
        return mr_nullptr;
    
    sptr_node->sptr_parent = sptr_par;
    sptr_node->sptr_branches = sptr_branches;
    sptr_node->sptr_link = mr_nullptr;

    return sptr_node;
}

/*!
 * @brief   find a trie_node in sptr_tree
 * @param   sptr_tree, name
 * @retval  none
 * @note    none
 */
struct trie_node *find_trie_node(struct trie_tree *sptr_tree, const char *name)
{
    struct trie_node *sptr_node;
    const kchar_t *str = name;
    kint32_t offset = 0;

    foreach_trie_tree(sptr_node, sptr_tree, offset) 
    {
        offset = sptr_tree->get(*(str++));

        if (IS_STRING_END(offset))
            break;
        else if (IS_STRING_ERR(offset))
            return ERR_PTR(-ER_FAULT);
    }

    if (sptr_node)
        return sptr_node->sptr_link ? sptr_node : mr_nullptr;

    print_debug("find node failed, name is: %s\r\n", name);

    return mr_nullptr;
}

/*!
 * @brief   find a trie_node in sptr_tree
 * @param   sptr_tree, name
 * @retval  none
 * @note    none
 */
struct trie_link *trie_tree_look_up(struct trie_tree *sptr_tree, const kchar_t *name)
{
    struct trie_node *sptr_node;

    sptr_node = find_trie_node(sptr_tree, name);
    if (IS_ERR(sptr_node) || !sptr_node)
        return mr_nullptr;

    return sptr_node->sptr_link;
}

/*!
 * @brief   add a new trie_node to sptr_tree
 * @param   sptr_tree, name
 * @retval  none
 * @note    none
 */
void trie_node_add(struct trie_tree *sptr_tree, const kchar_t *name, struct trie_link *sptr_link)
{
    struct trie_node *sptr_node, *sptr_temp;
    const kchar_t *str = name;
    kuint32_t lenth = kstrlen(name);
    kuint32_t i;
    kint32_t offset;

    if (!sptr_link)
        return;

    sptr_node = find_trie_node(sptr_tree, name);

    /*!< found or error */
    if (sptr_node)
        return;

    for (i = 0, sptr_node = &sptr_tree->sgtc_node; i < lenth; i++) 
    {
        offset = sptr_tree->get(*(str + i));

        if (!sptr_node->sptr_branches) 
        {
            if (!create_trie_branch(sptr_tree, sptr_node, sptr_tree->size))
                return;
        }

        if (!sptr_node->sptr_branches[offset]) 
        {
            sptr_temp = allocate_trie_node(sptr_tree, sptr_node, mr_nullptr);
            if (!isValid(sptr_temp))
                return;

            sptr_node->sptr_branches[offset] = sptr_temp;
        }

        print_debug("%s: i = %d, sptr_node: %p, offset: %c\r\n", __FUNCTION__, i, sptr_node->sptr_branches[offset], offset + 'a');

        sptr_link->depth = i;
        sptr_node = sptr_node->sptr_branches[offset];
    }

    sptr_node->sptr_link = sptr_link;

    print_debug("add new node succeuss, name is: %s\r\n", name);
}

/*!
 * @brief   delete sptr_node from sptr_tree with recursion
 * @param   sptr_tree, sptr_node
 * @retval  none
 * @note    none
 */
static void __del_trie_node(struct trie_tree *sptr_tree, struct trie_node *sptr_node, struct trie_node *sptr_child)
{
    kuint32_t i, count = 0;
    
    if (!sptr_node)
        return;

	/*!< just for leaf node */
    if (!sptr_child && sptr_node->sptr_branches)
        return;

    if (sptr_node->sptr_branches) 
    {
        for (i = 0; i < sptr_tree->size; i++) 
        {
            if (sptr_node->sptr_branches[i] == sptr_child) 
            {
                sptr_node->sptr_branches[i] = mr_nullptr;

                if (count)
                    goto out;
            }

            if (sptr_node->sptr_branches[i])
                count++;
        }

        if (count)
            goto out;

        sptr_tree->free(sptr_node->sptr_branches);
        sptr_node->sptr_branches = mr_nullptr;
    }

    if (!sptr_node->sptr_link) 
    {
        __del_trie_node(sptr_tree, sptr_node->sptr_parent, sptr_node);

        if (sptr_node != &sptr_tree->sgtc_node)
            sptr_tree->free(sptr_node);
    }

out:
    return;
}

/*!
 * @brief   delete sptr_node from sptr_tree
 * @param   sptr_tree, name
 * @retval  none
 * @note    none
 */
void trie_node_del(struct trie_tree *sptr_tree, const kchar_t *name)
{
    struct trie_node *sptr_node;
    const kchar_t *str = name;
    kuint32_t lenth = kstrlen(name);
    kuint32_t i;
    kint32_t offset = 0;

    for (i = 0, sptr_node = &sptr_tree->sgtc_node; i < lenth; i++) 
    {
        if (!sptr_node || !sptr_node->sptr_branches)
            return;
        
        offset = sptr_tree->get(*(str + i));
        if (IS_STRING_ERR(offset))
            return;

        sptr_node = sptr_node->sptr_branches[offset];
    }

    if (!sptr_node ||
        !sptr_node->sptr_link ||
        (sptr_node == &sptr_tree->sgtc_node))
        return;

    sptr_node->sptr_link = mr_nullptr;
    __del_trie_node(sptr_tree, sptr_node, mr_nullptr);
}

/*!< end of file */
