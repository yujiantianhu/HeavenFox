/*
 * Hardware Abstraction Layer Net Interface
 *
 * File Name:   fwk_netdev.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.06
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/net/fwk_netdev.h>
#include <platform/fwk_platform.h>
#include <platform/fwk_platdev.h>
#include <kernel/mutex.h>

/*!< The defines */
#define NETDEV_IF_INS_MAX                   ((kuint32_t)256U)

/*!< The globals */
static struct mutex_lock sgtc_netdev_ins_mutex = MUTEX_LOCK_INIT();
static struct mutex_lock sgtc_fwk_netdev_mutex = MUTEX_LOCK_INIT();
static DECLARE_LIST_HEAD(sgtc_fwk_net_device_list);
static kuint32_t g_fwk_allocated_ins[mr_num_align(NETDEV_IF_INS_MAX, RET_BITS_PER_INT) / RET_BITS_PER_INT] = { 0 };

/*!< API function */
/*!
 * @brief   get next sptr_ndev from list
 * @param   sptr_ndev (base)
 * @retval  sptr_ndev
 * @note    none
 */
struct fwk_net_device *next_netdevice(struct fwk_net_device *sptr_ndev)
{
    if (!sptr_ndev)
        return mr_list_first_valid_entry(&sgtc_fwk_net_device_list, struct fwk_net_device, sgtc_link);
    if (mr_list_head_until(sptr_ndev, &sgtc_fwk_net_device_list, sgtc_link))
        return mr_nullptr;

    return mr_list_next_entry(sptr_ndev, sgtc_link);
}

/*!
 * @brief   net name refactoring
 * @param   sptr_ndev
 * @retval  errno
 * @note    such as "enet%d" ===> enet0, enet1, ...
 */
static kint32_t fwk_net_validate_name(struct fwk_net_device *sptr_ndev)
{
    kchar_t new_name[NET_IFNAME_SIZE];
    kchar_t *name = sptr_ndev->name;
    kchar_t *p;
    kint32_t index = sptr_ndev->ifindex;

    p = kstrchr(name, '%');
    if (!p)
        return ER_NORMAL;

    *(name + (kuint32_t)(p - name)) = '\0';
    if (!(*(p + 1)) || ((*(p + 1)) != 'd'))
        goto END;

    if (index < 0)
    {
        mutex_lock(&sgtc_netdev_ins_mutex);
        index = bitmap_find_first_zero_bit(g_fwk_allocated_ins, 0, NETDEV_IF_INS_MAX);
        if (index < 0) 
        {
            mutex_unlock(&sgtc_netdev_ins_mutex);
            goto END;
        }

        bitmap_set_nr_bit_valid(g_fwk_allocated_ins, index, NETDEV_IF_INS_MAX, 1);
        mutex_unlock(&sgtc_netdev_ins_mutex);

        sptr_ndev->ifindex = index;
        sptr_ndev->priv_flags |= NR_NETDEV_PRIV_AINDEX;
    }

    sprintk(new_name, "%s%d", name, index);
    kstrncpy(name, new_name, NET_IFNAME_SIZE);

END:
    return ER_NORMAL;
}

/*!
 * @brief   return the net name number
 * @param   sptr_ndev
 * @retval  none
 * @note    none
 */
static void fwk_net_invalidate_name(struct fwk_net_device *sptr_ndev)
{
    kchar_t old_name[NET_IFNAME_SIZE];
    kchar_t *name = sptr_ndev->name;
    kchar_t *p;
    kint32_t index = sptr_ndev->ifindex;

    if (sptr_ndev->priv_flags & NR_NETDEV_PRIV_AINDEX)
    {
        memset(old_name, 0, NET_IFNAME_SIZE);
        sprintk(old_name, "%d", index);

        p = name + kstrlen(name) - kstrlen(old_name);
        *p = '%';
        *(p + 1) = '\0';

        mutex_lock(&sgtc_netdev_ins_mutex);
        bitmap_set_nr_bit_zero(g_fwk_allocated_ins, index, NETDEV_IF_INS_MAX, 1);
        mutex_unlock(&sgtc_netdev_ins_mutex);

        sptr_ndev->ifindex = -1;
        sptr_ndev->priv_flags &= (~NR_NETDEV_PRIV_AINDEX);
    }
}

/*!
 * @brief   Allocate network device
 * @param   none
 * @retval  none
 * @note    none
 */
struct fwk_net_device *fwk_alloc_netdev_mq(kint32_t sizeof_priv, const kchar_t *name,
                                    void (*setup) (struct fwk_net_device *sptr_ndev), kuint32_t txqs)
{
    struct fwk_net_device *sptr_netdev;
    struct fwk_netdev_queue *sptr_tx;
    kint32_t alloc_size = 0;

    alloc_size = sizeof(*sptr_netdev);
    if (sizeof_priv > 0)
        alloc_size = mr_align(alloc_size, sizeof(kutype_t));
    else
        sizeof_priv = 0;

    sptr_netdev = (struct fwk_net_device *)kzalloc(alloc_size + sizeof_priv, GFP_KERNEL);
    if (!isValid(sptr_netdev))
        return ERR_PTR(-ER_NOMEM);

    sptr_tx = (struct fwk_netdev_queue *)kzalloc(txqs * sizeof(*sptr_tx), GFP_KERNEL);
    if (!isValid(sptr_tx))
    {
        kfree(sptr_netdev);
        return ERR_PTR(-ER_NOMEM);
    }

    /*!< register send queue */
    sptr_netdev->sptr_tx = sptr_tx;
    sptr_netdev->num_tx_queues = txqs;
    sptr_netdev->real_num_tx_queues = txqs;
    sptr_netdev->private_data = (sizeof_priv > 0) ? (((void *)sptr_netdev) + alloc_size) : mr_nullptr;
    sptr_netdev->ifindex = -1;

    kstrncpy(sptr_netdev->name, name, NET_IFNAME_SIZE);
    init_list_head(&sptr_netdev->sgtc_link);

    if (setup)
        setup(sptr_netdev);

    return sptr_netdev;
}

/*!
 * @brief   release network device
 * @param   none
 * @retval  none
 * @note    none
 */
void fwk_free_netdev(struct fwk_net_device *sptr_ndev)
{
    if (!isValid(sptr_ndev))
        return;

    /*!< release send queue first */
    if (isValid(sptr_ndev->sptr_tx))
        kfree(sptr_ndev->sptr_tx);

    kfree(sptr_ndev);
}

/*!
 * @brief   get netdev by name
 * @param   name
 * @retval  netdev
 * @note    none
 */
struct fwk_net_device *fwk_ifname_to_ndev(const kchar_t *name)
{
    struct fwk_net_device *sptr_ndev;

    mutex_lock(&sgtc_fwk_netdev_mutex);
    foreach_list_next_entry(sptr_ndev, &sgtc_fwk_net_device_list, sgtc_link)
    {
        if (!kstrcmp(sptr_ndev->name, name))
        {
            mutex_unlock(&sgtc_fwk_netdev_mutex);
            return sptr_ndev;
        }
    }

    mutex_unlock(&sgtc_fwk_netdev_mutex);
    return mr_nullptr;
}

/*!
 * @brief   register network device
 * @param   sptr_ndev
 * @retval  errno
 * @note    add sptr_ndev to global device list
 */
kint32_t fwk_register_netdevice(struct fwk_net_device *sptr_ndev)
{
    struct fwk_device *sptr_dev;
    kint32_t retval;

    if (!sptr_ndev)
        return -ER_NODEV;

    if (!mr_list_empty(&sptr_ndev->sgtc_link))
        return -ER_CHECKERR;

    fwk_net_validate_name(sptr_ndev);
    if (fwk_ifname_to_ndev(sptr_ndev->name))
        return -ER_EXISTED;

    if (sptr_ndev->sptr_netdev_oprts->ndo_init)
    {
        retval = sptr_ndev->sptr_netdev_oprts->ndo_init(sptr_ndev);
        if (retval)
            return retval;
    }

    sptr_dev = &sptr_ndev->sgtc_dev;
    if (fwk_device_initial(sptr_dev))
        goto fail1;
    
    mr_dev_set_name(sptr_dev, "%s", sptr_ndev->name);
    if (fwk_device_add(sptr_dev))
        goto fail2;

    mutex_lock(&sgtc_fwk_netdev_mutex);
    list_head_add_tail(&sgtc_fwk_net_device_list, &sptr_ndev->sgtc_link);
    mutex_unlock(&sgtc_fwk_netdev_mutex);

    return ER_NORMAL;

fail2:
    mr_dev_del_name(sptr_dev);

fail1:
    if (sptr_ndev->sptr_netdev_oprts->ndo_uninit)
        sptr_ndev->sptr_netdev_oprts->ndo_uninit(sptr_ndev);

    return -ER_FAILD;
}

/*!
 * @brief   unregister network device
 * @param   sptr_ndev
 * @retval  errno
 * @note    del sptr_ndev from global device list
 */
kint32_t fwk_unregister_netdevice(struct fwk_net_device *sptr_ndev)
{
    struct fwk_device *sptr_dev;

    if (!sptr_ndev)
        return -ER_NODEV;

    if (mr_list_empty(&sptr_ndev->sgtc_link))
        return -ER_CHECKERR;

    if (!fwk_ifname_to_ndev(sptr_ndev->name))
        return -ER_NODEV;

    sptr_dev = &sptr_ndev->sgtc_dev;
    mr_dev_del_name(sptr_dev);
    if (sptr_ndev->sptr_netdev_oprts->ndo_uninit)
        sptr_ndev->sptr_netdev_oprts->ndo_uninit(sptr_ndev);

    fwk_net_invalidate_name(sptr_ndev);

    mutex_lock(&sgtc_fwk_netdev_mutex);
    list_head_del(&sptr_ndev->sgtc_link);
    mutex_unlock(&sgtc_fwk_netdev_mutex);

    return ER_NORMAL;
}

/*!< end of file */
