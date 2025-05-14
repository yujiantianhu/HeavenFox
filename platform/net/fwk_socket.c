/*
 * NetWork Interface
 *
 * File Name:   fwk_socket.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.11.23
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_fcntl.h>
#include <platform/net/fwk_if.h>
#include <platform/net/fwk_netif.h>
#include <platform/net/fwk_socket.h>
#include <kernel/mutex.h>
#include <kernel/rw_lock.h>

/*!< The defines */

/*!< The globals */
struct fwk_network_if_ops *sptr_fwk_network_if_oprts = mr_nullptr;

static struct mutex_lock sgtc_socket_mutex = MUTEX_LOCK_INIT();
static struct rw_lock sgtc_network_mutex = RW_LOCK_INIT();
static DECLARE_LIST_HEAD(sgtc_fwk_network_nodes);
static DECLARE_RADIX_TREE(sgtc_sockets_radix_tree, default_malloc, kfree);
static kuint32_t g_allocated_sockets[mr_align(NET_SOCKETS_MAX, RET_BITS_PER_INT) / RET_BITS_PER_INT] = { 0 };

#define mr_socket_to_object(sockfd)    \
            radix_tree_next_entry(&sgtc_sockets_radix_tree, struct fwk_network_object, sgtc_radix, sockfd)

/*!< API functions */
/*!
 * @brief   find sptr_if by name or ip
 * @param   name, sptr_ip
 * @retval  sptr_if
 * @note    none
 */
struct fwk_network_if *network_find_node(const kchar_t *name, struct fwk_sockaddr_in *sptr_ip)
{
    struct fwk_network_if *sptr_if;

    rd_lock(&sgtc_network_mutex);
    foreach_list_next_entry(sptr_if, &sgtc_fwk_network_nodes, sgtc_link)
    {
        if (name && (!strcmp(sptr_if->ifname, name)))
        {
            rd_unlock(&sgtc_network_mutex);
            return sptr_if;
        }

        if ((sptr_ip) && 
            (sptr_if->sgtc_ip.sin_addr.s_addr == sptr_ip->sin_addr.s_addr))
        {
            rd_unlock(&sgtc_network_mutex);
            return sptr_if;
        }
    }

    rd_unlock(&sgtc_network_mutex);
    return mr_nullptr;
}

/*!
 * @brief   get next sptr_if from list
 * @param   sptr_if (base)
 * @retval  sptr_if
 * @note    none
 */
struct fwk_network_if *network_next_node(struct fwk_network_if *sptr_if)
{
    if (!sptr_if)
        return mr_list_first_valid_entry(&sgtc_fwk_network_nodes, struct fwk_network_if, sgtc_link);
    if (mr_list_head_until(sptr_if, &sgtc_fwk_network_nodes, sgtc_link))
        return mr_nullptr;

    return mr_list_next_entry(sptr_if, sgtc_link);
}

/*!
 * @brief   start/enable net node
 * @param   name, sptr_ip, sptr_gw, sptr_mask
 * @retval  errno
 * @note    create sptr_if for per network node
 */
kint32_t net_link_up(const kchar_t *name, struct fwk_sockaddr_in *sptr_ip, 
                    struct fwk_sockaddr_in *sptr_gw, struct fwk_sockaddr_in *sptr_mask)
{
    struct fwk_network_if *sptr_if;
    struct fwk_network_if_ops *sptr_ops;

    sptr_ops = sptr_fwk_network_if_oprts;

    if ((!sptr_ops) || 
        (!sptr_ops->link_up) ||
        (!sptr_ops->link_down) ||
        (!sptr_ops->recv) ||
        (!sptr_ops->send))
        return -ER_NSUPPORT;

    /*!< ip is exsisted */
    if (network_find_node(name, sptr_ip))
        return -ER_EXISTED;

    sptr_if = kzalloc(sizeof(*sptr_if), GFP_KERNEL);
    if (!isValid(sptr_if))
        return PTR_ERR(sptr_if);

    if (sptr_ip)
        memcpy(&sptr_if->sgtc_ip, sptr_ip, sizeof(*sptr_ip));
    if (sptr_gw)
        memcpy(&sptr_if->sgtc_gw, sptr_gw, sizeof(*sptr_gw));
    if (sptr_mask)
        memcpy(&sptr_if->sgtc_netmask, sptr_mask, sizeof(*sptr_mask));

    strcpy(sptr_if->ifname, name);
    sptr_if->sptr_oprts = sptr_ops;

    if (sptr_if->sptr_oprts->link_up(sptr_if))
        goto fail;

    wr_lock(&sgtc_network_mutex);
    list_head_add_tail(&sgtc_fwk_network_nodes, &sptr_if->sgtc_link);
    wr_unlock(&sgtc_network_mutex);

    return ER_NORMAL;
    
fail:
    kfree(sptr_if);
    return -ER_FAILD;
}

/*!
 * @brief   change ip
 * @param   name, sptr_ip
 * @retval  errno
 * @note    none
 */
kint32_t network_set_ip(const kchar_t *name, struct fwk_sockaddr_in *sptr_ip)
{
    struct fwk_network_if *sptr_if;

    if ((!name) || (!sptr_ip))
        return -ER_INVALID;

    sptr_if = network_find_node(name, mr_nullptr);
    if (!sptr_if)
        return -ER_NODEV;

    if (network_find_node(mr_nullptr, sptr_ip))
        return -ER_EXISTED;

    memcpy(&sptr_if->sgtc_ip, sptr_ip, sizeof(*sptr_ip));
    return ER_NORMAL;
}

/*!
 * @brief   close net node
 * @param   name
 * @retval  errno
 * @note    none
 */
kint32_t net_link_down(const kchar_t *name)
{
    struct fwk_network_if *sptr_if;

    sptr_if = network_find_node(name, mr_nullptr);
    if (!sptr_if)
        return -ER_NODEV;

    if (sptr_if->sptr_oprts->link_down(sptr_if))
        return -ER_FAILD;

    fwk_netif_close(name);

    wr_lock(&sgtc_network_mutex);
    list_head_del(&sptr_if->sgtc_link);
    wr_unlock(&sgtc_network_mutex);

    kfree(sptr_if);
    return ER_NORMAL;
}

/*!
 * @brief   create sptr_socket, and allocate a descripter (sockfd)
 * @param   domain: AP_INET/PF_INET
 * @param   type: NR_SOCK_STREAM/NR_SOCK_DGRAM
 * @retval  sockfd
 * @note    none
 */
kint32_t network_socket(kint32_t domain, kint32_t type, kint32_t protocol)
{
    struct fwk_network_object *sptr_obj;
    struct fwk_network_com *sptr_socket;
    struct radix_tree *sptr_rtree;
    kint32_t index;

    mutex_lock(&sgtc_socket_mutex);
    index = bitmap_find_first_zero_bit(g_allocated_sockets, 0, NET_SOCKETS_NUM);
    if (index < 0)
    {
        mutex_unlock(&sgtc_socket_mutex);
        return -ER_FULL;
    }

    bitmap_set_nr_bit_valid(g_allocated_sockets, index, NET_SOCKETS_NUM, 1);
    mutex_unlock(&sgtc_socket_mutex);

    sptr_obj = kzalloc(sizeof(*sptr_obj), GFP_KERNEL);
    if (!isValid(sptr_obj))
    {
        mutex_lock(&sgtc_socket_mutex);
        bitmap_set_nr_bit_zero(g_allocated_sockets, index, NET_SOCKETS_NUM, 1);
        mutex_unlock(&sgtc_socket_mutex);

        return PTR_ERR(sptr_obj);
    }

    sptr_rtree  = &sgtc_sockets_radix_tree;
    sptr_socket = &sptr_obj->sgtc_socket;
    
    sptr_socket->domain = domain;
    sptr_socket->type = type;
    sptr_socket->protocol = protocol;

    spin_lock(&sptr_rtree->sgtc_lock);
    radix_tree_add(sptr_rtree, index, &sptr_obj->sgtc_radix);
    spin_unlock(&sptr_rtree->sgtc_lock);
    
    return (index + NETWORK_SOCKETS_BASE);
}

/*!
 * @brief   destrot sptr_socket
 * @param   sockfd
 * @retval  none
 * @note    none
 */
void network_close(kint32_t sockfd)
{
    struct fwk_network_object *sptr_obj;
    struct fwk_network_if *sptr_if;
    struct radix_tree *sptr_rtree;
    kint32_t index;

    index = sockfd - NETWORK_SOCKETS_BASE;
    if ((index < 0) ||
        (index >= NET_SOCKETS_NUM))
        return;

    sptr_obj = mr_socket_to_object(index);
    if (sptr_obj)
    {
        sptr_if = sptr_obj->sptr_if;
        sptr_rtree = &sgtc_sockets_radix_tree;

        if (sptr_if && sptr_if->sptr_oprts->exit)
           sptr_if->sptr_oprts->exit(&sptr_obj->sgtc_socket);

        spin_lock(&sptr_rtree->sgtc_lock);
        radix_tree_del(&sgtc_sockets_radix_tree, index);
        spin_unlock(&sptr_rtree->sgtc_lock);

        mutex_lock(&sgtc_socket_mutex);
        bitmap_set_nr_bit_zero(g_allocated_sockets, index, NET_SOCKETS_NUM, 1);
        mutex_unlock(&sgtc_socket_mutex);

        kfree(sptr_obj);
    }
}

/*!
 * @brief   bind ip for sptr_socket
 * @param   sockfd, sptr_addr, ...
 * @retval  errno
 * @note    none
 */
kint32_t network_bind(kint32_t sockfd, const struct fwk_sockaddr *sptr_addr, fwk_socklen_t addrlen)
{
    struct fwk_network_object *sptr_obj;
    struct fwk_network_if *sptr_if;
    struct fwk_network_com *sptr_socket;
    struct fwk_sockaddr_in *sptr_sin;
    kint32_t index;

    index = sockfd - NETWORK_SOCKETS_BASE;
    if ((index < 0) ||
        (index >= NET_SOCKETS_NUM))
        return -ER_INVALID;

    sptr_obj = mr_socket_to_object(index);
    if (!sptr_obj)
        return -ER_EMPTY;

    sptr_sin = (struct fwk_sockaddr_in *)sptr_addr;
    sptr_if = network_find_node(mr_nullptr, sptr_sin);
    if (!sptr_if)
        return -ER_IOERR;

    sptr_obj->sptr_if = sptr_if;
    sptr_socket = &sptr_obj->sgtc_socket;
    if (sptr_socket->domain != sptr_sin->sin_family)
        return -ER_CHECKERR;

    memcpy(&sptr_socket->sgtc_sin, sptr_sin, addrlen);

    if (sptr_if->sptr_oprts->init)
        sptr_if->sptr_oprts->init(sptr_socket);

    return 0;
}

/*!
 * @brief   wait server response
 * @param   sockfd, sptr_addr, ...
 * @retval  errno
 * @note    none
 */
kint32_t network_accept(kint32_t sockfd, struct fwk_sockaddr *sptr_addr, fwk_socklen_t *addrlen)
{
    kint32_t index;

    index = sockfd - NETWORK_SOCKETS_BASE;
    if ((index < 0) ||
        (index >= NET_SOCKETS_NUM))
        return -ER_INVALID;

    return 0;
}

/*!
 * @brief   send msg (udp)
 * @param   sockfd, buf, len, ...
 * @retval  size
 * @note    none
 */
kssize_t network_sendto(kint32_t sockfd, const void *buf, kssize_t len, 
                        kint32_t flags, const struct fwk_sockaddr *sptr_dest, fwk_socklen_t addrlen)
{
    struct fwk_network_object *sptr_obj;
    struct fwk_network_if *sptr_if;
    kint32_t index;

    index = sockfd - NETWORK_SOCKETS_BASE;
    if ((index < 0) ||
        (index >= NET_SOCKETS_NUM))
        return -ER_INVALID;

    sptr_obj = mr_socket_to_object(index);
    if (!sptr_obj)
        return -ER_EMPTY;

    sptr_if = sptr_obj->sptr_if;
    if (sptr_if->sptr_oprts->sendto)
        return sptr_if->sptr_oprts->sendto(&sptr_obj->sgtc_socket, buf, len, flags, sptr_dest, addrlen);

    return -ER_FORBID;
}

/*!
 * @brief   recv msg (udp)
 * @param   sockfd, buf, len, ...
 * @retval  size
 * @note    none
 */
kssize_t network_recvfrom(kint32_t sockfd, void *buf, size_t len, 
                        kint32_t flags, struct fwk_sockaddr *sptr_src, fwk_socklen_t *addrlen)
{
    struct fwk_network_object *sptr_obj;
    struct fwk_network_if *sptr_if;
    kint32_t index;

    index = sockfd - NETWORK_SOCKETS_BASE;
    if ((index < 0) ||
        (index >= NET_SOCKETS_NUM))
        return -ER_INVALID;

    sptr_obj = mr_socket_to_object(index);
    if (!sptr_obj)
        return -ER_EMPTY;

    sptr_if = sptr_obj->sptr_if;
    if (sptr_if->sptr_oprts->recvfrom)
        return sptr_if->sptr_oprts->recvfrom(&sptr_obj->sgtc_socket, buf, len, flags, sptr_src, addrlen);

    return -ER_FORBID;
}

/*!< ----------------------------------------------------------------- */
/*!
 * @brief   net_socket
 * @param   none
 * @retval  none
 * @note    The interface is provided for use by the application layer
 */
kint32_t net_socket(kint32_t domain, kint32_t type, kint32_t protocol)
{
    return network_socket(domain, type, protocol);
}

/*!
 * @brief   bind socket
 * @param   sockfd, sptr_addr, addrlen, ...
 * @retval  errno
 * @note    none
 */
kint32_t socket_bind(kint32_t sockfd, const struct fwk_sockaddr *sptr_addr, fwk_socklen_t addrlen)
{
    return network_bind(sockfd, sptr_addr, addrlen);
}

/*!
 * @brief   send msg (udp)
 * @param   sockfd, buf, len, ...
 * @retval  size
 * @note    none
 */
kssize_t socket_sendto(kint32_t sockfd, const void *buf, kssize_t len, 
                        kint32_t flags, const struct fwk_sockaddr *sptr_dest, fwk_socklen_t addrlen)
{
    return network_sendto(sockfd, buf, len, flags, sptr_dest, addrlen);
}

/*!
 * @brief   recv msg (udp)
 * @param   sockfd, buf, len, ...
 * @retval  size
 * @note    none
 */
kssize_t socket_recvfrom(kint32_t sockfd, void *buf, size_t len, 
                        kint32_t flags, struct fwk_sockaddr *sptr_src, fwk_socklen_t *addrlen)
{
    return network_recvfrom(sockfd, buf, len, flags, sptr_src, addrlen);
}

/*!< end of file */
