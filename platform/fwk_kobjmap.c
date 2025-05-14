/*
 * Platform Object Maps Defines
 *
 * File Name:   fwk_kbojmap.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.18
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_kobj.h>
#include <kernel/mutex.h>

/*!< The globals */
struct fwk_kobj_map *sptr_fwk_chrdev_map;
struct fwk_kobj_map *sptr_fwk_blkdev_map;

/*!< API function */
/*!
 * @brief   fwk_kobjmap_init
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t __plat_init fwk_kobjmap_init(void)
{
    sptr_fwk_chrdev_map = (struct fwk_kobj_map *)kmalloc(sizeof(struct fwk_kobj_map), GFP_KERNEL);
    if (!isValid(sptr_fwk_chrdev_map))
        goto fail1;

    sptr_fwk_blkdev_map = (struct fwk_kobj_map *)kmalloc(sizeof(struct fwk_kobj_map), GFP_KERNEL);
    if (!isValid(sptr_fwk_blkdev_map))
        goto fail2;

    /*!< Initialize the character device matching table */
    mutex_init(&sptr_fwk_chrdev_map->sgtc_mutex);
    memset(sptr_fwk_chrdev_map->sptr_probes, 0, sizeof(sptr_fwk_chrdev_map->sptr_probes));

    /*!< Initialize the block device matching table */
    mutex_init(&sptr_fwk_blkdev_map->sgtc_mutex);
    memset(sptr_fwk_blkdev_map->sptr_probes, 0, sizeof(sptr_fwk_blkdev_map->sptr_probes));

    return ER_NORMAL;

fail2:
    kfree(sptr_fwk_chrdev_map);
    sptr_fwk_chrdev_map = mr_nullptr;
fail1:
    return -ER_NOMEM;
}

/*!
 * @brief   fwk_kobjmap_del
 * @param   none
 * @retval  none
 * @note    none
 */
void __plat_exit fwk_kobjmap_del(void)
{
    struct fwk_probes *sptr_prev;
    struct fwk_probes *sptr_list;
    kusize_t mapsize;
    kuint32_t i;

    /*!< Destroy the character device matching table */
    mapsize	= sptr_fwk_chrdev_map ? ARRAY_SIZE(sptr_fwk_chrdev_map->sptr_probes) : 0;
    for (i = 0; i < mapsize; i++)
    {
        mr_list_delete_all(sptr_fwk_chrdev_map->sptr_probes[i],
                                        sptr_prev, sptr_list);
        sptr_fwk_chrdev_map->sptr_probes[i] = mr_nullptr;
    }
    kfree(sptr_fwk_chrdev_map);
    sptr_fwk_chrdev_map = mr_nullptr;

    /*!< Destroy the block device matching table */
    mapsize	= sptr_fwk_blkdev_map ? ARRAY_SIZE(sptr_fwk_blkdev_map->sptr_probes) : 0;
    for (i = 0; i < mapsize; i++)
    {
        mr_list_delete_all(sptr_fwk_blkdev_map->sptr_probes[i],
                                        sptr_prev, sptr_list);
        sptr_fwk_blkdev_map->sptr_probes[i] = mr_nullptr;
    }
    kfree(sptr_fwk_blkdev_map);
    sptr_fwk_blkdev_map = mr_nullptr;
}

/*!
 * @brief   fwk_kobj_map
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_kobj_map(struct fwk_kobj_map *sptr_domain, kuint32_t devNum, kuint32_t range, void *data)
{
    struct fwk_probes *sptr_probe;
    kuint32_t major;
    kuint32_t majorCnt;
    kuint32_t i;
    kusize_t probeMax;

    if (!isValid(sptr_domain) || (!isValid(data)))
        return -ER_FAULT;

    /*!< The maximum number of primary devices that can be supported */
    probeMax = ARRAY_SIZE(sptr_domain->sptr_probes);

    /*!< The number of master devices, that is, the number of array members occupied by probes */
    major = GET_DEV_MAJOR(devNum);
    major = mr_ret_min2(probeMax, major);
    majorCnt = GET_DEV_MAJOR(devNum + range) - major + 1;
    majorCnt = CMP_GT2(major + majorCnt, probeMax, probeMax - major, majorCnt);

    sptr_probe = (struct fwk_probes *)kzalloc(sizeof(struct fwk_probes) * majorCnt, GFP_KERNEL);
    if (!isValid(sptr_probe))
        return -ER_NOMEM;

    mutex_lock(&sptr_domain->sgtc_mutex);

    for (i = 0; i < majorCnt; i++)
    {
        struct fwk_probes *sptr_Temp;
        struct fwk_probes **sptr_Dst;
        kuint32_t index;

        index = (major + i) % probeMax;
        sptr_Dst = &sptr_domain->sptr_probes[index];
        sptr_Temp = &sptr_probe[i];

        /*!<
         * Regardless of whether the master devices are the same, as long as the same driver calls this API, the data of the master devices is the same
         * Each sptr_Temp belongs to the same (devNum ~ devNum + range) range, and the data is the same; and:
         * 		1) kobj doesn't need to worry about invalid or duplicate device numbers, as this step has already been done in the fwk_register_chrdev
         * 		2) Apply for a sptr_Temp according to the main device number, the purpose is to fill the probes array, 
         * 		so as to facilitate the quick location of the master device number;
         * 		3) When looking for a device, you just need to determine if the device number is located in (devNum ~ devNum + range), 
         * 		you don't have to think about anything else, because:
         * 			fwk_register_chrdev have ensured that this range of device numbers cannot be illegal!
         */
        sptr_Temp->data = data;
        sptr_Temp->devNum = devNum;
        sptr_Temp->range = range;

        /*!< Sorting: Sort by range, from smallest to largest */
        /*!< Actually, the ranking here doesn't make much sense, and it's okay not to rank */
        while ((isValid(*sptr_Dst)) && ((*sptr_Dst)->range < range))
            sptr_Dst = &(*(sptr_Dst))->sptr_next;

        sptr_Temp->sptr_next = *sptr_Dst;
        *sptr_Dst = sptr_Temp;
    }

    mutex_unlock(&sptr_domain->sgtc_mutex);
    return ER_NORMAL;
}

/*!
 * @brief   fwk_kobj_unmap
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_kobj_unmap(struct fwk_kobj_map *sptr_domain, kuint32_t devNum, kuint32_t range)
{
    struct fwk_probes *sptr_Rlt;
    kuint32_t major;
    kuint32_t majorCnt;
    kuint32_t i;
    kusize_t probeMax;

    if (!isValid(sptr_domain))
        return -ER_FAULT;

    /*!< The maximum number of primary devices that can be supported */
    probeMax = ARRAY_SIZE(sptr_domain->sptr_probes);

    /*!< The number of master devices, that is, the number of array members occupied by probes */
    major = GET_DEV_MAJOR(devNum);
    major = mr_ret_min2(probeMax, major);
    majorCnt = GET_DEV_MAJOR(devNum + range) - major + 1;
    majorCnt = CMP_GT2(major + majorCnt, probeMax, probeMax - major, majorCnt);

    mutex_lock(&sptr_domain->sgtc_mutex);

    for (i = 0, sptr_Rlt = mr_nullptr; i < majorCnt; i++)
    {
        struct fwk_probes *sptr_Temp;
        struct fwk_probes **sptr_Dst;
        kuint32_t index;

        index = (major + i) % probeMax;
        sptr_Dst = &sptr_domain->sptr_probes[index];

        while (*sptr_Dst)
        {
            if (((*sptr_Dst)->devNum == devNum) && ((*sptr_Dst)->range == range))
            {
                /*!< Delete the list node */
                sptr_Temp = *sptr_Dst;
                *sptr_Dst = sptr_Temp->sptr_next;

                sptr_Rlt = sptr_Rlt ? sptr_Rlt : sptr_Temp;
                break;
            }

            sptr_Dst = &(*(sptr_Dst))->sptr_next;
        }
    }

    mutex_unlock(&sptr_domain->sgtc_mutex);

    if (sptr_Rlt)
        kfree(sptr_Rlt);

    return ER_NORMAL;
}

/*!
 * @brief   fwk_kobjmap_lookup
 * @param   none
 * @retval  none
 * @note    none
 */
void *fwk_kobjmap_lookup(struct fwk_kobj_map *sptr_domain, kuint32_t devNum)
{
    struct fwk_probes *sptr_Temp;
    kuint32_t index;
    void *data;
    kusize_t probeMax;

    if (!isValid(sptr_domain))
        return mr_nullptr;

    /*!< The maximum number of primary devices that can be supported */
    probeMax = ARRAY_SIZE(sptr_domain->sptr_probes);
    index = GET_DEV_MAJOR(devNum);

    mutex_lock(&sptr_domain->sgtc_mutex);

    for (sptr_Temp = sptr_domain->sptr_probes[index % probeMax]; 
         sptr_Temp; 
         sptr_Temp = sptr_Temp->sptr_next)
    {
        /*!< Already found? */
        /*!< It is determined by the range of device numbers */
        if ((devNum >= sptr_Temp->devNum) && (devNum <= (sptr_Temp->devNum + sptr_Temp->range - 1)))
            break;
    }

    mutex_unlock(&sptr_domain->sgtc_mutex);
    data = sptr_Temp ? sptr_Temp->data : mr_nullptr;

    return data;
}

/*!< end of file */
