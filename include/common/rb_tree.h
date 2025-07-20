/*
 * Red-Black Tree Interface
 *
 * File Name:   rb_tree.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.07.20
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __RB_TREE_H
#define __RB_TREE_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>

/*!< The defines */
/*!< 节点颜色 */
#define RBT_NODE_RED                                0x00
#define RBT_NODE_BLACK                              0x01

struct rbt_node 
{
    struct rbt_node *sptr_parent;                   /*!< 父节点 (如果一个节点没有父节点, 说明它是一个根节点; 如果一个节点的父节点是节点本身, 说明这是一个无效节点) */
    struct rbt_node *sptr_left;                     /*!< 左子节点 */
    struct rbt_node *sptr_right;                    /*!< 右子节点 */

    kuint8_t color;                                 /*!< 当前节点的颜色 (红与黑) */
};

/*!< Root node: 单独构造一个结构体, 以便快速定位和管理 */
struct rbt_root
{
    /*!< Root node must be black */
    struct rbt_node *sptr_node;
};

/*!< Get parent data pointer */
#define mr_rbt_entry(pos, member)                   mr_container_of((pos)->member, typeof(*(pos)), member)

#define INIT_RBT_NODE(node) \
    {   \
        .sptr_parent = (node),    \
        .sptr_left = mr_nullptr,    \
        .sptr_right = mr_nullptr,   \
        .color = RBT_NODE_BLACK,    \
    }

#define DECLARE_RBT_NODE(name)  \
    struct rbt_node name = INIT_RBT_NODE(&name)

/*!< 获取父节点 */
#define rbt_node_parent(sptr_nd)                    ((sptr_nd)->sptr_parent)
/*!< 设置父节点 */
#define rbt_node_set_parent(sptr_nd, sptr_par)      ((sptr_nd)->sptr_parent = (sptr_par))
/*!< 是否为根节点: 特点是sptr_parent为NULL */
#define rbt_node_is_root(sptr_nd)                   (!(sptr_nd)->sptr_parent)
/*!< 是否为叶节点: 特点是不含子节点 */
#define rbt_node_is_leaf(sptr_nd)                   (!(sptr_nd)->sptr_left && !(sptr_left)->sptr_right)
/*!< 是否为无效节点 */
#define rbt_node_is_invalid(sptr_nd)                ((sptr_nd)->sptr_parent == sptr_nd)
/*!< 获取节点颜色 */
#define rbt_node_color(sptr_nd)                     ((sptr_nd)->color)
/*!< 判断节点是否为红色 */
#define rbt_node_is_red(sptr_nd)                    ((sptr_nd)->color == RBT_NODE_RED)
/*!< 判断节点是否为黑色 */
#define rbt_node_is_black(sptr_nd)                  ((sptr_nd)->color == RBT_NODE_BLACK)
/*!< 设置节点颜色 */
#define rbt_node_set_color(sptr_nd, _color)         ((sptr_nd)->color = (_color))
/*!< 设置节点为红色 */
#define rbt_node_set_red(sptr_nd)                   rbt_node_set_color(sptr_nd, RBT_NODE_RED)
/*!< 设置节点为黑色 */
#define rbt_node_set_black(sptr_nd)                 rbt_node_set_color(sptr_nd, RBT_NODE_BLACK)

/*!< The functions */
extern void rbt_node_init(struct rbt_node *sptr_node);

extern void rbt_node_left_rotate(struct rbt_root *sptr_root, struct rbt_node *sptr_node);
extern void rbt_node_right_rotate(struct rbt_root *sptr_root, struct rbt_node *sptr_node);

extern struct rbt_node *rbt_get_first(struct rbt_root *sptr_root, struct rbt_node *sptr_base);
extern struct rbt_node *rbt_get_last(struct rbt_root *sptr_root, struct rbt_node *sptr_base);
extern struct rbt_node *rbt_next_node(struct rbt_node *sptr_node);
extern struct rbt_node *rbt_prev_node(struct rbt_node *sptr_node);

extern void rbt_insert_color(struct rbt_root *sptr_root, struct rbt_node *sptr_node);
extern void rbt_erase_color(struct rbt_root *sptr_root, struct rbt_node *sptr_parent, struct rbt_node *sptr_child);
extern void rbt_add_node(struct rbt_node *sptr_node, struct rbt_node *sptr_parent, struct rbt_node **sptr_pos);
extern void rbt_del_node(struct rbt_root *sptr_root, struct rbt_node *sptr_node);
extern void rbt_replace_node(struct rbt_root *sptr_root, struct rbt_node *sptr_old, struct rbt_node *sptr_new);

/*!< The defines */
/*! @note head: root */
#define mr_rbt_first_entry(head, type, member)   \
({  \
    struct rbt_node *_node = rbt_get_first(head, mr_nullptr);   \
    _node ? mr_container_of(_node, type, member) : mr_nullptr;  \
})

/*! @note head: root */
#define mr_rbt_last_entry(head, type, member)   \
({  \
    struct rbt_node *_node = rbt_get_last(head, mr_nullptr);   \
    _node ? mr_container_of(_node, type, member) : mr_nullptr;  \
})

/*! @note head: root */
#define mr_rbt_next_entry(pos, head, member)   \
({  \
    struct rbt_node *_node = rbt_next_node(&(pos)->member);   \
    _node ? mr_container_of(_node, typeof(*(pos)), member) : mr_nullptr;  \
})

/*! @note head: root */
#define mr_rbt_prev_entry(pos, head, member)   \
({  \
    struct rbt_node *_node = rbt_prev_node(&(pos)->member);   \
    _node ? mr_container_of(_node, typeof(*(pos)), member) : mr_nullptr;  \
})

/*! @note head: root */
#define foreach_next_rbt_node(pos, head, member)  \
    for (pos = mr_rbt_first_entry(head, typeof(*(pos)), member);   \
         pos;  \
         pos = mr_rbt_next_entry(pos, head, member))

/*! @note head: root */
#define foreach_prev_rbt_node(pos, head, member)  \
    for (pos = mr_rbt_last_entry(head, typeof(*(pos)), member);   \
         pos;  \
         pos = mr_rbt_prev_entry(pos, head, member))

/*!
 * @brief   Insert new node
 * @param   pos: data pointer
 * @param   head: root
 * @param   key_member: value member. like "pos->key_member"
 * @param   node_member: rbt_node member. like "pos->node_member"
 */
#define mr_rbt_insert_node(pos, head, key_member, node_member)  \
do {    \
    struct rbt_node **_node = &((head)->sptr_node);    \
    struct rbt_node *_parent = mr_nullptr;  \
    \
    while (_node && (*_node)) {  \
        typeof(*(pos)) *_pos = mr_container_of(*_node, typeof(*(pos)), node_member);    \
        _parent = *_node;  \
        \
        if ((pos)->key_member < _pos->key_member)  \
            _node = &((*_node)->sptr_left);   \
        else if ((pos)->key_member > _pos->key_member) \
            _node = &((*_node)->sptr_right);  \
        else    \
            _node = mr_nullptr; \
    }   \
    \
    if (_node) {    \
        rbt_add_node(&(pos)->node_member, _parent, _node); \
        rbt_insert_color(head, &(pos)->node_member);  \
    }   \
} while (0)

/*!
 * @brief   delete node
 * @param   pos: data pointer
 * @param   head: root
 * @param   node_member: rbt_node member. like "pos->node_member"
 */
#define mr_rbt_delete_node(pos, head, node_member)  \
do {    \
    if (mr_likely(pos))    \
        rbt_del_node(head, &(pos)->node_member);    \
} while (0)

/*!
 * @brief   find parent entry
 * @param   key: value
 * @param   head: root
 * @param   type: parent entry's type
 * @param   key_member: value member. like "pos->key_member"
 * @param   node_member: rbt_node member. like "pos->node_member"
 */
#define mr_rbt_find_entry(key, head, type, key_member, node_member)   \
({    \
    struct rbt_node *_node = (head)->sptr_node;  \
    type *_found = mr_nullptr;  \
    \
    while (_node) { \
        type *_pos = mr_container_of(_node, type, node_member);    \
        \
        if ((key) < _pos->key_member)    \
            _node = _node->sptr_left; \
        else if ((key) > _pos->key_member)   \
            _node = _node->sptr_right;    \
        else {   \
            _found = _pos;    \
            break;  \
        }   \
    }   \
    \
    _found;    \
})

#ifdef __cplusplus
    }
#endif

#endif /* __RB_TREE_H */
