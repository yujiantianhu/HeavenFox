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
/*!
 * @brief   get the number of valid bits (bit = 1)
 * @param   number
 * @retval  none
 * @note    for example: number = 0x1010 (16bits), valid bits = 2; zero bits = 30
 */
static kuint16_t radix_tree_numlen(kuint32_t number)
{
    kuint16_t count, result;
    
    count = parse_valid_u32_bits(number);
    if (count <= MAX_BRANCH)
        return MAX_BRANCH;

    /*!< if it is the n-power of 2, such as 1, 2, 4, 8, ... */
    if (isPower2(MAX_BRANCH))
        return mr_align(count, MAX_BRANCH);
    
    /*!< result = count % default_valids */
    result = udiv_remainder(count, MAX_BRANCH);
    return result ? (count + MAX_BRANCH - result) : count;
}

/*!
 * @brief   update number
 * @param   number
 * @retval  none
 * @note    none
 */
static kuint32_t radix_tree_advance(kuint32_t number)
{
    return (number >> MAX_BRANCH);
}

/*!
 * @brief   get the number of groups
 * @param   number
 * @retval  none
 * @note    for example: number = 0x1010 (16bits), MAX_BRANCH = 2, the groups = 16 / 2 = 8
 */
kuint16_t get_radix_node_branch(kuint32_t number)
{
    return (number & ((1 << MAX_BRANCH) - 1));
}

/*!
 * @brief   allocate one radix_node
 * @param   sptr_tree, sptr_par
 * @retval  none
 * @note    none
 */
struct radix_node *allocate_radix_node(struct radix_tree *sptr_tree, struct radix_node *sptr_par)
{
    struct radix_node *sptr_node;
    kuint32_t i;

    sptr_node = (struct radix_node *)sptr_tree->alloc(sizeof(*sptr_node));
    if (!isValid(sptr_node))
        return mr_nullptr;
    
    sptr_node->sptr_parent = sptr_par;
    sptr_node->sptr_link = mr_nullptr;

    for (i = 0; i < (1 << MAX_BRANCH); i++)
        sptr_node->sgtc_branches[i] = mr_nullptr;

    return sptr_node;
}

/*!
 * @brief   find a radix_node in sptr_tree
 * @param   sptr_tree, number
 * @retval  none
 * @note    none
 */
struct radix_node *find_radix_node(struct radix_tree *sptr_tree, kuint32_t number)
{
    struct radix_node *sptr_node;
    kuint32_t value = number;
    kuint16_t lenth = radix_tree_numlen(number);
    kuint16_t i = 0, offset = 0;

    foreach_radix_tree(sptr_node, sptr_tree, offset)
    {
        if (i >= lenth)
            break;

        i += MAX_BRANCH;
        offset = sptr_tree->get(value);
        value = radix_tree_advance(value);
    }

    if (sptr_node)
        return sptr_node->sptr_link ? sptr_node : mr_nullptr;

//  print_debug("find node failed, index is: 0x%x\r\n", number);

    return mr_nullptr;
}

/*!
 * @brief   find a radix_node in sptr_tree
 * @param   sptr_tree, number
 * @retval  none
 * @note    none
 */
struct radix_link *radix_tree_look_up(struct radix_tree *sptr_tree, kuint32_t number)
{
    struct radix_node *sptr_node;

    sptr_node = find_radix_node(sptr_tree, number);
    if (!sptr_node)
        return mr_nullptr;

    return sptr_node->sptr_link;
}

/*!
 * @brief   add a new radix_node to sptr_tree
 * @param   sptr_tree, number
 * @retval  none
 * @note    none
 */
void radix_tree_add(struct radix_tree *sptr_tree, kuint32_t number, struct radix_link *sptr_link)
{
    struct radix_node *sptr_node, *sptr_temp;
    kuint32_t value = number;
    kuint16_t lenth = radix_tree_numlen(number);
    kuint16_t i, offset = 0;

    if (!sptr_link)
        return;

    if (find_radix_node(sptr_tree, number))
        return;

    for (i = 0, sptr_node = &sptr_tree->sgtc_node; i < lenth; i += MAX_BRANCH) 
    {
        offset = sptr_tree->get(value);
        value = radix_tree_advance(value);

        if (!sptr_node->sgtc_branches[offset]) 
        {
            sptr_temp = allocate_radix_node(sptr_tree, sptr_node);
            if (!isValid(sptr_temp))
                return;

            sptr_node->sgtc_branches[offset] = sptr_temp;
        }

//      print_debug("%s: lenth: %d, i = %d, sptr_node: %p, offset: 0x%x\r\n", __FUNCTION__, lenth, i, sptr_node->sgtc_branches[offset], offset);

        sptr_link->depth = i;
        sptr_node = sptr_node->sgtc_branches[offset];
    }

    if (sptr_node != &sptr_tree->sgtc_node)
        sptr_node->sptr_link = sptr_link;

//  print_debug("add new node successfully, index is: 0x%x\r\n", number);
}

/*!
 * @brief   delete radix_node from sptr_tree with recursion
 * @param   sptr_tree, number
 * @retval  none
 * @note    none
 */
static void __del_radix_node(struct radix_tree *sptr_tree, struct radix_node *sptr_node, struct radix_node *sptr_child)
{
    kuint32_t i, count = 0;
    
    if (!sptr_node)
        return;

    for (i = 0; i < (1 << MAX_BRANCH); i++) 
    {
        if (sptr_node->sgtc_branches[i] == sptr_child) 
        {
            sptr_node->sgtc_branches[i] = mr_nullptr;

            if (count)
                goto out;
        }

        if (sptr_node->sgtc_branches[i])
            count++;
    }

    if (count)
        goto out;

    if (!sptr_node->sptr_link) 
    {
        __del_radix_node(sptr_tree, sptr_node->sptr_parent, sptr_node);

        if (sptr_node != &sptr_tree->sgtc_node)
            sptr_tree->free(sptr_node);
    }

out:
    return;
}

/*!
 * @brief   delete a group of radix_nodes from sptr_tree
 * @param   sptr_tree, number
 * @retval  none
 * @note    none
 */
void radix_tree_del(struct radix_tree *sptr_tree, kuint32_t number)
{
    struct radix_node *sptr_node;
    kuint32_t value = number;
    kuint32_t lenth = radix_tree_numlen(number);
    kuint32_t i, offset = 0;

    for (i = 0, sptr_node = &sptr_tree->sgtc_node; i < lenth; i += MAX_BRANCH) 
    {
        if (!sptr_node)
            return;
        
        offset = sptr_tree->get(value);
        value = radix_tree_advance(value);
        sptr_node = sptr_node->sgtc_branches[offset];
    }

    if (!sptr_node ||
        !sptr_node->sptr_link ||
        (sptr_node == &sptr_tree->sgtc_node)) 
    {
        print_debug("nothing needs to delete\r\n");
        return;
    }

    sptr_node->sptr_link = mr_nullptr;
    __del_radix_node(sptr_tree, sptr_node, mr_nullptr);
}


/*!< end of file */
