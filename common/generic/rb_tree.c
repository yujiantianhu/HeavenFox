/*!
 * Red-Black Tree Interface
 *
 * File Name:   rb_tree.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.07.20
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/generic.h>
#include <common/mem_manage.h>
#include <common/rb_tree.h>

/*!< The defines */
typedef void (*rbt_node_rotate_fn)(struct rbt_root *sptr_root, struct rbt_node *sptr_node);

/*!< API functions */
/*!
 * @brief   初始化节点
 * @param   sptr_node
 * @retval  无
 * @note    主要是初始化sptr_node->sptr_parent为自身, 即新节点首先是一个无效节点
 */
void rbt_node_init(struct rbt_node *sptr_node)
{
    sptr_node->color = RBT_NODE_RED;
    sptr_node->sptr_parent = sptr_node;
    sptr_node->sptr_left = sptr_node->sptr_right = mr_nullptr;
}

/*!
 * @brief   以节点sptr_node为中心左旋
 * @param   sptr_root: 根节点
 * @param   sptr_node: 中心节点 (即下面举例的"a")
 * @retval  无
 * @note    左旋是为了维持红黑树的整体高度, 在不破坏树的有序性的前提下调整结构
 *          左旋的方法 (中序):
 *          例:
 *              a                       c
 *             / \                    /   \
 *            b   c         --->     a     e
 *               / \                / \
 *              d   e              b   d
 *          1) 右子节点的序号一定大于父节点, 左子节点的序号一定小于父节点, 即: b < a < d < c < e;
 *          2) 父节点a下移, 右子节点c上移, 即右子节点取代父节点的位置;
 *          3) a成为新父节点c的左子节点 (因为a < c)
 *          3) c原来的左子节点d成为a的右子节点 (因为a < d < c);
 *          4) 左旋后顺序不变, 仍为: b < a < d < c < e
 */
void rbt_node_left_rotate(struct rbt_root *sptr_root, struct rbt_node *sptr_node)
{
    /*!
     * 几个节点需要调整关系:
     *  1) a和d之间: a的右子节点和d的父节点;
     *  2) a和c之间: a的父节点和c的左子节点;
     *  3) c与a以前的父节点;
     */
    struct rbt_node *sptr_right, *sptr_parent;

    /*!< 1) 连接a和d */
    sptr_right = sptr_node->sptr_right;
    sptr_node->sptr_right = sptr_right->sptr_left;
    if (sptr_node->sptr_right)
        sptr_node->sptr_right->sptr_parent = sptr_node;

    /*!< 2) 连接a和c */
    sptr_parent = rbt_node_parent(sptr_node);
    sptr_node->sptr_parent = sptr_right;
    sptr_right->sptr_left = sptr_node;

    /*!< 3) 连接c和a以前的父节点 */
    sptr_right->sptr_parent = sptr_parent;

    /*!< 父节点存在, 说明a并非根节点 */
    if (sptr_parent)
    {
        /*!< 如果a曾是parent的左子节点 */
        if (sptr_parent->sptr_left == sptr_node)
            sptr_parent->sptr_left = sptr_right;
        else
            sptr_parent->sptr_right = sptr_right;
    }
    /*!< 父节点不存在, a为根节点; 需注册到sptr_root */
    else
    {
        sptr_root->sptr_node = sptr_right;
    }
}

/*!
 * @brief   以节点sptr_node为中心右旋
 * @param   sptr_root: 根节点
 * @param   sptr_node: 中心节点 (即下面举例的"c")
 * @retval  无
 * @note    右旋是为了维持红黑树的整体高度, 在不破坏树的有序性的前提下调整结构
 *          右旋的方法 (中序):
 *          例:
 *                 c                    a
 *               /   \                /   \
 *              a     e     --->     b     c
 *             / \                        / \
 *            b   d                      d   e
 *          1) 右子节点的序号一定大于父节点, 左子节点的序号一定小于父节点, 即: b < a < d < c < e;
 *          2) 父节点c下移, 左子节点a上移, 即左子节点取代父节点的位置;
 *          3) c成为新父节点a的右子节点 (因为a < c)
 *          3) a原来的右子节点d成为c的左子节点 (因为a < d < c);
 *          4) 右旋后顺序不变, 仍为: b < a < d < c < e
 */
void rbt_node_right_rotate(struct rbt_root *sptr_root, struct rbt_node *sptr_node)
{
    /*!
     * 几个节点需要调整关系:
     *  1) c和d之间: c的左子节点和d的父节点;
     *  2) c和a之间: c的父节点和a的右子节点;
     *  3) c与a以前的父节点;
     */
    struct rbt_node *sptr_left, *sptr_parent;

    /*!< 1) 连接c和d */
    sptr_left = sptr_node->sptr_left;
    sptr_node->sptr_left = sptr_left->sptr_right;
    if (sptr_node->sptr_left)
        sptr_node->sptr_left->sptr_parent = sptr_node;

    /*!< 2) 连接c和a */
    sptr_parent = rbt_node_parent(sptr_node);
    sptr_node->sptr_parent = sptr_left;
    sptr_left->sptr_right = sptr_node;

    /*!< 3) 连接c和a以前的父节点 */
    sptr_left->sptr_parent = sptr_parent;

    /*!< 父节点存在, 说明c并非根节点 */
    if (sptr_parent)
    {
        /*!< 如果a曾是parent的左子节点 */
        if (sptr_parent->sptr_left == sptr_node)
            sptr_parent->sptr_left = sptr_left;
        else
            sptr_parent->sptr_right = sptr_left;
    }
    /*!< 父节点不存在, a为根节点; 需注册到sptr_root */
    else
    {
        sptr_root->sptr_node = sptr_left;
    }
}

/*!
 * @brief   获取树中最小的节点 (最初的节点)
 * @param   sptr_root
 * @param   sptr_base: 搜索起点(某棵树的树顶), 传入NULL时, 默认从根节点开始遍历
 * @retval  最小的节点
 * @note    中序遍历, 从根节点或指定节点开始, 不断读取左子节点
 */
struct rbt_node *rbt_get_first(struct rbt_root *sptr_root, struct rbt_node *sptr_base)
{
    struct rbt_node *sptr_per, *sptr_rlt = mr_nullptr;

    /*!< 优先使用sptr_base */
    sptr_per = sptr_base ? sptr_base : (sptr_root ? sptr_root->sptr_node : mr_nullptr);
    while (sptr_per)
    {
        sptr_rlt = sptr_per;
        sptr_per = sptr_per->sptr_left;
    }

    return sptr_rlt;
}

/*!
 * @brief   获取树中最大的节点 (最末的节点)
 * @param   sptr_root
 * @param   sptr_base: 搜索起点(某棵树的树顶), 传入NULL时, 默认从根节点开始遍历
 * @retval  最大的节点
 * @note    中序遍历, 从根节点或指定节点开始, 不断读取右子节点
 */
struct rbt_node *rbt_get_last(struct rbt_root *sptr_root, struct rbt_node *sptr_base)
{
    struct rbt_node *sptr_per, *sptr_rlt = mr_nullptr;

    /*!< 优先使用sptr_base */
    sptr_per = sptr_base ? sptr_base : (sptr_root ? sptr_root->sptr_node : mr_nullptr);
    while (sptr_per)
    {
        sptr_rlt = sptr_per;
        sptr_per = sptr_per->sptr_right;
    }

    return sptr_rlt;
}

/*!
 * @brief   获取下一个节点 (后继节点)
 * @param   sptr_node
 * @retval  后继节点
 * @note    中序遍历, sptr_node的下一个节点一定比sptr_node大
 *          情况1: sptr_node有右子树, 则对右子树向左向下查找
 *              a) 右子树有左子树, 则不断向左向下取到最末端的节点 (该节点为整棵树中第一个比sptr_node大的节点);
 *              b) 右子树没有左子树, 则右子树的起点直接当成后继节点;
 *          情况2: sptr_node没有右子树, 则沿着父节点向上向右查找
 *              a) 如果当前节点在父节点的左子树中, 则父节点一定比sptr_node大;
 *              b) 如果当前节点在父节点的右子树中, 则父节点一定比sptr_node小, 则继续向上向右查找;
 */
struct rbt_node *rbt_next_node(struct rbt_node *sptr_node)
{
    struct rbt_node *sptr_right, *sptr_par;
    struct rbt_node *sptr_cur = sptr_node;

    /*!< 无效节点 */
    if (rbt_node_is_invalid(sptr_node))
        return mr_nullptr;

    /*!< 1. 有右子节点, 则解析右子树 */
    sptr_right = sptr_node->sptr_right;
    if (sptr_right)
    {
        /*!< 若sptr_right->sptr_left为空, 则不存在左子树, sptr_right即后继节点; 否则取出左子树的最小节点 */
        return rbt_get_first(mr_nullptr, sptr_right);
    }

    /*!< 2. 无右子节点, 则解析父节点 */
    sptr_par = rbt_node_parent(sptr_node);

    /*!< 找到第一个拥有右子树的父/祖先节点, 该父/祖先节点即第一个比sptr_node大的节点 */
    while (sptr_par && (sptr_cur == sptr_par->sptr_right))
    {
        sptr_cur = sptr_par;
        sptr_par = rbt_node_parent(sptr_par);
    }
    
    return sptr_par;
}

/*!
 * @brief   获取上一个节点 (前驱节点)
 * @param   sptr_node
 * @retval  前驱节点
 * @note    中序遍历, sptr_node的上一个节点一定比sptr_node小
 *          情况1: sptr_node有左子树, 则对左子树向右向下查找
 *              a) 左子树有右子树, 则不断向右向下取到最末端的节点 (该节点为整棵树中第一个比sptr_node小的节点);
 *              b) 左子树没有右子树, 则左子树的起点直接当成前驱节点;
 *          情况2: sptr_node没有左子树, 则沿着父节点向上向左查找
 *              a) 如果当前节点在父节点的右子树中, 则父节点一定比sptr_node小;
 *              b) 如果当前节点在父节点的左子树中, 则父节点一定比sptr_node大, 则继续向上向左查找;
 */
struct rbt_node *rbt_prev_node(struct rbt_node *sptr_node)
{
    struct rbt_node *sptr_left, *sptr_par;
    struct rbt_node *sptr_cur = sptr_node;

    /*!< 无效节点 */
    if (rbt_node_is_invalid(sptr_node))
        return mr_nullptr;

    /*!< 1. 有左子节点, 则解析左子树 */
    sptr_left = sptr_node->sptr_left;
    if (sptr_left)
    {
        /*!< 若sptr_left->sptr_right为空, 则不存在右子树, sptr_left即前驱节点; 否则取出右子树的最大节点 */
        return rbt_get_last(mr_nullptr, sptr_left);
    }

    /*!< 2. 无左子节点, 则解析父节点 */
    sptr_par = rbt_node_parent(sptr_node);

    /*!< 找到第一个拥有左子树的父/祖先节点, 该父/祖先节点即第一个比sptr_node小的节点 */
    while (sptr_par && (sptr_cur == sptr_par->sptr_left))
    {
        sptr_cur = sptr_par;
        sptr_par = rbt_node_parent(sptr_par);
    }
    
    return sptr_par;
}

/*!
 * @brief   红黑树插入节点后, 调整颜色
 * @param   sptr_root: 根节点
 * @param   sptr_node: 新插入的节点
 * @param   none
 * @note    可与rbt_insert_node配套使用, 用来调整插入后颜色的变更
 * 
 *          1. 每个节点必须是红色或黑色;
 *          2. 根节点必须是黑色
 *          3. 所有叶子节点 (NULL, 如新插入节点sptr_node的sptr_left和sptr_right)视为黑色;
 *          4. 红色节点不能有红色子节点, 即不能存在连续的红色节点;
 *          5. 从任意节点到其每个叶子节点的路径包含相同数量的黑色节点;
 *          6. 树全黑 (所有节点均为黑色) 时高度最小 (特性5限制了各路径高度); 红黑交替时高度最大 (在特性5基础上多了红色节点, 且因为特性4限制了红色数量);
 *          7. 新插入的节点视为红色
 *          
 *          a) 新插入的节点是根节点, 说明这是一棵空树, 将节点颜色变黑即可;
 *          b) 若父节点为黑色, 插入一个红色节点不会影响黑色的数量, 符合特性4、5, 无需调整;
 *                  黑
 *                /    \
 *              黑      黑
 *                       \
 *                        红
 * 
 *          c) 若父节点为红色, 插入新节点后违反特性4; 需将父节点变更为黑色;
 *                  黑                              黑
 *                /    \                         /     \
 *              NULL    红          --->       NULL     黑
 *                       \                               \
 *                        红                              红
 * 
 *             显然, 父节点变黑后, 右子树的黑高增大, 需要将叔节点所在子树的黑高也增大, 以保证特性5.
 *             >> 如果叔节点为空, 左子树只能往上将祖父节点变成红色. 但如此一来, 可能违反特性4 (祖父节点与曾祖父节点)
 *                可考虑左旋降低右侧高度, 之后父节点替代祖父节点的位置, 而祖父节点成为父节点的左子节点. 旋转后高度平衡
 * 
 *                  黑                              红                                  黑
 *                /    \                         /      \           (左旋)            /     \
 *              NULL    红          --->       NULL     黑          ----->          红       红
 *                       \                               \                          /
 *                        红                              红                       NULL
 *
 *          d)  >> 如果叔节点不为空, 那它一定是红色节点 (黑色节点无法平衡), 将叔节点变为黑色后, 两侧高度平衡;
 *                 但祖父节点所在的整棵子树黑高 + 1，只能也将祖父节点改为红色, 以维持原来的高度.
 *                 变更后, 若曾祖父节点为红色, 同上一步骤 (将祖父节点当成新节点, 递归处理);
 *                 变更后, 若曾祖父节点为红色, 同步骤b, 调整结束
 *                  黑                              红      
 *                /    \                         /     \    
 *              红      红          --->        黑      黑          --->    递归    --->    ...
 *                       \                               \  
 *                        红                              红
 */
void rbt_insert_color(struct rbt_root *sptr_root, struct rbt_node *sptr_node)
{
    struct rbt_node *sptr_uncle, *sptr_gpar;
    struct rbt_node *sptr_parent;
    struct rbt_node *sptr_cur = sptr_node;

    /*!< 情况a、b不满足while条件, 无需调整; 可用于情况c、d */
    while ((sptr_parent = rbt_node_parent(sptr_cur)) && rbt_node_is_red(sptr_parent))
    {
        /*!< 祖父节点 (如果父节点为根节点, 那它一定是黑色, 不可能进while循环; 故sptr_node一定存在祖父节点) */
        sptr_gpar = rbt_node_parent(sptr_parent);
        /*!< 叔节点: 父节点的另一侧 */
        sptr_uncle = (sptr_parent == sptr_gpar->sptr_left) ? sptr_gpar->sptr_right : sptr_gpar->sptr_left;

        /*!< 情况d */
        if (sptr_uncle && rbt_node_is_red(sptr_uncle))
        {
            rbt_node_set_black(sptr_parent);
            rbt_node_set_black(sptr_uncle);

            /*!< 祖节点改为红色, 以维持祖节点所在子树的黑高; 此时祖节点类似于sptr_node, 可使用递归 */
            rbt_node_set_red(sptr_gpar);
            sptr_cur = sptr_gpar;
        }
        /*!< 情况c */
        else
        {
            struct rbt_node *sptr_tmp;

            /*!< 
            * 当sptr_cur == sptr_node时, 叔节点如果不是红色, 那一定是空节点;
            * 否则叔节点为黑色节点
            * 新节点的父节点变色后, 高度失衡, 而叔节点已经是黑色, 无法通过变更颜色更改黑高
            * 可通过旋转以维持高度
            */

            /*!< 2.1 父节点为左子节点, 为方便快速调整, 子节点也应尽量在左子树, 即父子都是左子树 */
            if (sptr_gpar->sptr_left == sptr_parent)
            {
                /*!< 若当前节点已经在左子树, 满足需求; 如在右子树, 需左旋至左侧 */
                if (sptr_parent->sptr_right == sptr_cur)
                {
                    rbt_node_left_rotate(sptr_root, sptr_parent);

                    /*!< 左旋后父子关系交换 */
                    sptr_tmp = sptr_parent;
                    sptr_parent = sptr_cur;
                    sptr_cur = sptr_tmp;
                }

                /*!< 父子都已在左子树, 可整体右旋; 右旋后祖节点成为父节点, 而父节点成为祖节点 */
                rbt_node_right_rotate(sptr_root, sptr_gpar);
            }
            else
            {
                /*!< 同上 */
                if (sptr_parent->sptr_left == sptr_cur)
                {
                    rbt_node_right_rotate(sptr_root, sptr_parent);

                    sptr_tmp = sptr_parent;
                    sptr_parent = sptr_cur;
                    sptr_cur = sptr_tmp;
                }

                rbt_node_left_rotate(sptr_root, sptr_gpar);
            }

            rbt_node_set_black(sptr_parent);
            rbt_node_set_red(sptr_gpar);
        }
    }

    /*!< 不需要判断是哪种情况, 根节点一定为黑 */
    rbt_node_set_black(sptr_root->sptr_node);
}

/*!
 * @brief   擦除节点颜色
 * @param   sptr_node: 被删节点的接替者
 * @param   sptr_root: 根节点
 * @param   sptr_parent: *sptr_node的父节点 (节点删除后重新调整的位置). sptr_child可能为空节点, 需要外部提供父节点指针
 * @param   sptr_slibling: *sptr_node的兄弟
 * @param   positive_rotate: 若*sptr_node是左子节点, 则positive_rotate应为左旋函数指针; 否则为右旋函数指针
 * @param   negative_rotate: 若*sptr_node是左子节点, 则negative_rotate应为右旋函数指针; 否则为左旋函数指针
 * @retval  none
 * @note    sptr_child = *sptr_node, sptr_child是被删节点的子节点(可能为NULL)或后继节点
 *          1) 若被删除的节点为红色, 本函数不应被调用 (删除红节点不影响红黑树特性, 直接用sptr_child顶替即可);
 *                  黑                          黑
 *                /    \            --->      /    \
 *              红(*)  NULL                 NULL   NULL
 * 
 *          2) 若被删除的节点为黑色, 删除后子树黑高减少, 违反特性
 *              2.1) 若sptr_child为红色, 直接变为黑色, 即可维持黑高不变; sptr_child必然是叶节点, 没有子树, 更改其颜色不影响其他;
 *                  黑                          黑                          黑
 *                /    \            --->      /    \            --->      /    \
 *              黑(*)   黑                   红     黑                    黑    黑
 *             /
 *            红
 * 
 *              2.2) 若sptr_child为黑色, 父节点为红色, 兄弟节点为黑色, 且兄弟没有子节点 (或子节点均为黑色), 
 *                   则将兄弟节点变为红色, 父节点变为黑色, 调整后子树整体黑高不变;
 *                  红                          红                          黑
 *                /    \            --->      /    \            --->      /    \
 *              黑(*)   黑                   NULL   黑                   NULL   红
 *             /
 *            NULL
 * 
 *              2.3) 若sptr_child为黑色, 父节点为黑色, 兄弟节点为黑色, 且兄弟没有子节点 (或子节点均为黑色), 
 *                   则将兄弟节点变为红色, 之后子树黑高少于删除前, 需递归处理父节点 (将父节点当成sptr_child, 重新判断)
 *                  黑                          黑                          黑
 *                /    \            --->      /    \            --->      /    \
 *              黑(*)   黑                   NULL   黑                   NULL   红
 *             /
 *            NULL
 * 
 *              2.4) 若sptr_child为黑色, 兄弟节点为红色, 可将兄弟节点变为黑色(父节点可能为黑色, 导致兄弟侧黑高增大), 然后左旋/右旋;
 *                   旋转后兄弟换人, 重新递归判断
 *              2.5) 若sptr_child为黑色, 父节点为黑色, 兄弟节点为黑色, 且兄弟有子节点, 则将兄弟节点的一侧子节点变红后旋转
 *                  黑                          黑                          黑
 *                /    \                      /    \           (左旋)     /    \
 *              黑(*)   黑          --->     NULL   黑          --->     黑     黑
 *             /      /   \                       /   \                /   \
 *            NULL   红    红                    红    红             NULL  红
 */
static struct rbt_node *__rbt_erase_color(struct rbt_node **sptr_node, struct rbt_root *sptr_root, 
                                struct rbt_node *sptr_parent, struct rbt_node *sptr_slibling,
                                rbt_node_rotate_fn positive_rotate, rbt_node_rotate_fn negative_rotate)
{
    struct rbt_node *sptr_schild, *sptr_sbrother;
    kuint8_t is_left = (sptr_slibling == sptr_parent->sptr_right);

    /*!< 情况2.4 兄弟节点为红色, 将其变为黑色后左旋 */
    if (rbt_node_is_red(sptr_slibling))
    {
        rbt_node_set_black(sptr_slibling);
        rbt_node_set_red(sptr_parent);
        positive_rotate(sptr_root, sptr_parent);

        /*!< 左旋后, sptr_cur的兄弟节点换人 */
        sptr_slibling = is_left ? sptr_parent->sptr_right : sptr_parent->sptr_left;
    }

    sptr_schild = is_left ? sptr_slibling->sptr_left : sptr_slibling->sptr_right;
    sptr_sbrother = is_left ? sptr_slibling->sptr_right : sptr_slibling->sptr_left;

    /*!< 情况2.2、2.3 兄弟节点为黑色, 且兄弟节点的子节点也为黑色, 可变更兄弟节点为红色 */
    if ((!sptr_schild || rbt_node_is_black(sptr_schild)) &&
        (!sptr_sbrother || rbt_node_is_black(sptr_sbrother)))
    {
        rbt_node_set_red(sptr_slibling);
        *sptr_node = sptr_parent;

        return rbt_node_parent(*sptr_node);
    }

    /*!< 兄弟节点的右子节点为黑色 */
    if (!sptr_sbrother || rbt_node_is_black(sptr_sbrother)) 
    {
        rbt_node_set_black(sptr_schild);
        rbt_node_set_red(sptr_slibling);
        negative_rotate(sptr_root, sptr_slibling);

        /*!< 旋转后更新指针 (sptr_schild不再使用, 无需更新) */
        sptr_slibling = is_left ? sptr_parent->sptr_right : sptr_parent->sptr_left;
        sptr_sbrother = is_left ? sptr_slibling->sptr_right : sptr_slibling->sptr_left;
    }
    
    /*!< 兄弟节点的右子节点是红色 */
    sptr_slibling->color = sptr_parent->color;
    rbt_node_set_black(sptr_parent);
    rbt_node_set_black(sptr_sbrother);
    positive_rotate(sptr_root, sptr_parent);
    *sptr_node = sptr_root->sptr_node;

    return mr_nullptr;
}

/*!
 * @brief   擦除节点颜色
 * @param   sptr_root: 根节点
 * @param   sptr_parent: sptr_child的父节点 (节点删除后重新调整的位置). sptr_child可能为空节点, 需要外部提供父节点指针
 * @param   sptr_child: 被删除节点的子节点 (也可能是被移动的后继节点的子节点). 它最终会上位, 顶替被删除的位置
 * @retval  none
 * @note    需配合rbt_delete_node使用, 此时节点已被删除. sptr_child是它的子节点(可能为NULL)或后继节点
 *          可查看__rbt_erase_color原理
 */
void rbt_erase_color(struct rbt_root *sptr_root, struct rbt_node *sptr_parent, struct rbt_node *sptr_child)
{
    struct rbt_node *sptr_cur = sptr_child;

    /*!< sptr_cur为黑色节点 */
    while ((!sptr_cur || rbt_node_is_black(sptr_cur)) && 
            sptr_parent && (sptr_cur != sptr_root->sptr_node))
    {
        /*!< 左子树 */
        if (sptr_cur == sptr_parent->sptr_left)
        {
            sptr_parent = __rbt_erase_color(&sptr_cur, sptr_root, sptr_parent, sptr_parent->sptr_right, 
                                        rbt_node_left_rotate, rbt_node_right_rotate);
        }
        else
        {
            sptr_parent = __rbt_erase_color(&sptr_cur, sptr_root, sptr_parent, sptr_parent->sptr_left, 
                                        rbt_node_right_rotate, rbt_node_left_rotate); 
        }
    }

    if (sptr_cur)
        rbt_node_set_black(sptr_cur);
}

/*!
 * @brief   插入新节点
 * @param   sptr_node: 要插入的新节点
 * @param   sptr_parent: 要插入位置的父节点 (与新节点关联)
 * @param   sptr_pos: 要插入sptr_parent的左侧(&sptr_parent->sptr_left)还是右侧(&sptr_parent->sptr_right)
 * @retval  none
 * @note    新插入的节点必然是叶节点, 我们总能找到一个合适的叶来存放它;
 *          新插入的节点颜色为红色;
 *          插入操作可以不考虑是否影响树的高度, 由其他函数进行调整
 */
void rbt_add_node(struct rbt_node *sptr_node, struct rbt_node *sptr_parent, struct rbt_node **sptr_pos)
{
    sptr_node->sptr_parent = sptr_parent;
    sptr_node->color = RBT_NODE_RED;
    sptr_node->sptr_left = sptr_node->sptr_right = mr_nullptr;

    /*!< 插入的位置 */
    *sptr_pos = sptr_node;
}

/*!
 * @brief   删除节点
 * @param   sptr_root: 根节点
 * @param   sptr_node: 要从树中删除的节点
 * @retval  none
 * @note    一个要被删除的节点a, 可能是:
 *          1) 只有一个子节点b(左或右): 直接用b代替a的位置
 *          2) 没有子节点: 直接置NULL
 *          3) 左右节点都在: 用前驱(左子树最大的节点)或后继(右子树最小的节点)替换a均可;
 *              ---> 如同删除前驱或后继节点, 删除a转化为对前驱/后继节点删除
 * 
 *          子节点或后继节点替换sptr_node后, 需重新连接父节点和左右节点
 *          对于情况3, 还需要将后继节点的父节点与子节点进行连接 (后继节点被"删除")
 */
void rbt_del_node(struct rbt_root *sptr_root, struct rbt_node *sptr_node)
{
    struct rbt_node *sptr_parent, *sptr_child;
    kuint8_t color = RBT_NODE_RED;

    /*!< 禁止操作 */
    if (!sptr_node)
    	return;

    /*!< 情况3 */
    if (sptr_node->sptr_left && sptr_node->sptr_right)
    {
        struct rbt_node *sptr_next, *sptr_prev;

        /*!< 取得后继节点 */
        sptr_next = rbt_get_first(mr_nullptr, sptr_node->sptr_right);

        /*!< sptr_next作为后继节点, 一定没有左子节点 */
        sptr_child = sptr_next->sptr_right;
        sptr_parent = rbt_node_parent(sptr_next);
        color = rbt_node_color(sptr_next);

        /*!< 连接父节点(将sptr_next脱链) */
        if (sptr_child)
            rbt_node_set_parent(sptr_child, sptr_parent);

        /*!< 如果父节点就是sptr_node, 那sptr_next一定是sptr_node的右子节点(因为是后继) */
        if (sptr_parent == sptr_node)
        {
            /*!< 修改sptr_node->sptr_right */
            sptr_parent->sptr_right = sptr_child;

            /*!< 未来的父节点(sptr_node最终会被移除, sptr_child的父节点仍为sptr_next) */
            sptr_parent = sptr_next;
        }
        else
        {
            /*!< sptr_next与sptr_node不相邻, 那sptr_next一定在右子树的左下方 */
            sptr_parent->sptr_left = sptr_child;
        }

        /*!< sptr_next顶替sptr_node, 需继承sptr_node的属性 */
        sptr_next->sptr_parent = sptr_node->sptr_parent;
        sptr_next->sptr_left = sptr_node->sptr_left;
        sptr_next->sptr_right = sptr_node->sptr_right;
        sptr_next->color = sptr_node->color;

        /*!< 还需将sptr_node以前的父子节点都指向sptr_next */
        sptr_prev = rbt_node_parent(sptr_node);
        if (sptr_prev)
        {
            if (sptr_prev->sptr_left == sptr_node)
                sptr_prev->sptr_left = sptr_next;
            else
                sptr_prev->sptr_right = sptr_next;
        }
        else
        {
            sptr_root->sptr_node = sptr_next;
        }

        /*!< 右子节点可能被"if (sptr_parent == sptr_node)"修改, 需额外判断非空 */
        rbt_node_set_parent(sptr_node->sptr_left, sptr_next);
        if (sptr_node->sptr_right)
            rbt_node_set_parent(sptr_node->sptr_right, sptr_next);
    }
    else
    {
        /*!< 情况1 (兼容情况2) */
        sptr_child = sptr_node->sptr_left ? sptr_node->sptr_left : sptr_node->sptr_right;
        sptr_parent = rbt_node_parent(sptr_node);
        color = rbt_node_color(sptr_node);

        /*!< 连接父节点 */
        if (sptr_child)
            rbt_node_set_parent(sptr_child, sptr_parent);
        /*!< 连接子节点 */
        if (sptr_parent)
        {
            if (sptr_parent->sptr_left == sptr_node)
                sptr_parent->sptr_left = sptr_child;
            else
                sptr_parent->sptr_right = sptr_child;
        }
        else
        {
            sptr_root->sptr_node = sptr_child;
        }
    }

    /*!< 删除红色节点不影响树的特性, 无需调整颜色 */
    if (color == RBT_NODE_BLACK)
        rbt_erase_color(sptr_root, sptr_parent, sptr_child);

    /*!< 设置为无效节点 */
    sptr_node->sptr_parent = sptr_node;
}

/*!
 * @brief   替换节点
 * @param   sptr_root: 根节点
 * @param   sptr_old: 要被替换的旧节点
 * @param   sptr_new: 用于替换的新节点
 * @retval  none
 * @note    none
 */
void rbt_replace_node(struct rbt_root *sptr_root, struct rbt_node *sptr_old, struct rbt_node *sptr_new)
{
    struct rbt_node *sptr_parent;
    
    /*!< 继承sptr_old原有的连接关系 */
    memcpy(sptr_new, sptr_old, sizeof(*sptr_new));
    
    /*!< 父节点连接新节点 */
    sptr_parent = rbt_node_parent(sptr_old);
    if (sptr_parent) 
    {
        if (sptr_old == sptr_parent->sptr_left)
            sptr_parent->sptr_left = sptr_new;
        else
            sptr_parent->sptr_right = sptr_new;
    } 
    else 
    {
        sptr_root->sptr_node = sptr_new;
    }
    
    /*!< 子节点连接新节点 */
    if (sptr_old->sptr_left)
        rbt_node_set_parent(sptr_old->sptr_left, sptr_new);
    if (sptr_old->sptr_right)
        rbt_node_set_parent(sptr_old->sptr_right, sptr_new);
}

/* end of file */
