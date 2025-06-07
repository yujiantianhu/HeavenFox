/*
 * Memory Allocate Management
 *
 * File Name:   mem_kalloc.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.10.02
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <boot/boot_text.h>
#include <platform/fwk_mempool.h>
#include <kernel/sched.h>
#include <kernel/wait.h>
#include <kernel/spinlock.h>

/*!< The defines */
struct fwk_mempool
{
    const kchar_t *name;
    kuint32_t mask;
    struct mem_info *sptr_info;

    struct wait_queue_head sgtc_wqh;
    struct spin_lock sgtc_lock;
};

#define FWK_MEMPOOL_FIXDATA             0
#define FWK_MEMPOOL_KERNEL              1
#define FWK_MEMPOOL_DMA                 2
#define FWK_MEMPOOL_FB_DRAM             3
#define FWK_MEMPOOL_SK_BUFF             4
#define FWK_MEMPOOL_TYPE_MAX            5

/*!< The globals */
static struct mem_info sgtc_kernel_mem_info[FWK_MEMPOOL_TYPE_MAX] = {};

static struct fwk_mempool sgtc_kernel_mempool[FWK_MEMPOOL_TYPE_MAX] =
{
    {
        .name = "fixed data",
        .mask = NR_KMEM_FIXDATA,
        .sptr_info = &sgtc_kernel_mem_info[FWK_MEMPOOL_FIXDATA],
    },
    {
        .name = "kernel heap",
        .mask = NR_KMEM_NORMAL,
        .sptr_info = &sgtc_kernel_mem_info[FWK_MEMPOOL_KERNEL],
    },
    {
        .name = "dma",
        .mask = NR_KMEM_DMA_AREA,
        .sptr_info = &sgtc_kernel_mem_info[FWK_MEMPOOL_DMA],
    },
    {
        .name = "framebuffer",
        .mask = NR_KMEM_FBUFFER,
        .sptr_info = &sgtc_kernel_mem_info[FWK_MEMPOOL_FB_DRAM],
    },
    {
        .name = "network",
        .mask = NR_KMEM_SK_BUFF,
        .sptr_info = &sgtc_kernel_mem_info[FWK_MEMPOOL_SK_BUFF],
    },
};

/*!< API function */
/*!
 * @brief   fwk_mempool_initial
 * @param   none
 * @retval  none
 * @note    kernel memory block initial
 */
kbool_t fwk_mempool_initial(void)
{
    struct fwk_mempool *sptr_pool;
    struct mem_info *sptr_info;
    kint32_t retval;

    /*!< ------------------------------------------------------------ */
    sptr_pool = &sgtc_kernel_mempool[FWK_MEMPOOL_KERNEL];
    sptr_info = sptr_pool->sptr_info;
    retval = memory_block_create(sptr_info, MEMORY_POOL_BASE, MEMORY_POOL_SIZE);
    if (retval)
        return false;
    
    init_waitqueue_head(&sptr_pool->sgtc_wqh);
    spin_lock_init(&sptr_pool->sgtc_lock);

    /*!< ------------------------------------------------------------ */
    sptr_pool = &sgtc_kernel_mempool[FWK_MEMPOOL_DMA];
    sptr_info = sptr_pool->sptr_info;
    memory_simple_block_create(sptr_info, DMA_AREA_BASE, DMA_AREA_SIZE);
    init_waitqueue_head(&sptr_pool->sgtc_wqh);
    spin_lock_init(&sptr_pool->sgtc_lock);

    /*!< ------------------------------------------------------------ */
    sptr_pool = &sgtc_kernel_mempool[FWK_MEMPOOL_FB_DRAM];
    sptr_info = sptr_pool->sptr_info;
    memory_simple_block_create(sptr_info, FBUFFER_DRAM_BASE, FBUFFER_DRAM_SIZE);
    init_waitqueue_head(&sptr_pool->sgtc_wqh);
    spin_lock_init(&sptr_pool->sgtc_lock);

    /*!< ------------------------------------------------------------ */
    sptr_pool = &sgtc_kernel_mempool[FWK_MEMPOOL_SK_BUFF];
    sptr_info = sptr_pool->sptr_info;
    memory_block_create(sptr_info, SK_BUFFER_BASE, SK_BUFFER_SIZE);
    init_waitqueue_head(&sptr_pool->sgtc_wqh);
    spin_lock_init(&sptr_pool->sgtc_lock);

    return true;
}

/*!
 * @brief   memory_block_self_defines
 * @param   none
 * @retval  none
 * @note    memory block initial
 */
kbool_t memory_block_self_defines(kint32_t flags, kuaddr_t base, kusize_t size)
{
    struct fwk_mempool *sptr_pool = mr_nullptr;
    struct mem_info *sptr_info;
    kuint32_t index;

    if (flags < 0)
        sptr_pool = &sgtc_kernel_mempool[FWK_MEMPOOL_KERNEL];
    else
    {
        for (index = 0; index < FWK_MEMPOOL_TYPE_MAX; index++)
        {
            if (flags & sgtc_kernel_mempool[index].mask)
            {
                if (!sptr_pool)
                    sptr_pool = &sgtc_kernel_mempool[index];
                else
                    return false;
            }
        }
    }

    if (!sptr_pool)
        return false;

    sptr_info = sptr_pool->sptr_info;
    if (isValid(sptr_info->sptr_mem))
        return false;

    memory_simple_block_create(sptr_info, base, size);
    init_waitqueue_head(&sptr_pool->sgtc_wqh);
    spin_lock_init(&sptr_pool->sgtc_lock);

    return true;
}

/*!
 * @brief   memory_block_self_destroy
 * @param   none
 * @retval  none
 * @note    memory block destroy
 */
void memory_block_self_destroy(kint32_t flags)
{
    struct fwk_mempool *sptr_pool = mr_nullptr;
    struct mem_info *sptr_info;
    kuint32_t index;

    if (flags < 0)
        sptr_pool = &sgtc_kernel_mempool[FWK_MEMPOOL_KERNEL];
    else
    {
        for (index = 0; index < FWK_MEMPOOL_TYPE_MAX; index++)
        {
            if (flags & sgtc_kernel_mempool[index].mask)
            {
                if (!sptr_pool)
                    sptr_pool = &sgtc_kernel_mempool[index];
                else
                    return;
            }
        }
    }

    if (!sptr_pool)
        return;

    sptr_info = sptr_pool->sptr_info;
    if (!isValid(sptr_info->sptr_mem))
        return;

    memory_simple_block_destroy(sptr_info);
    init_waitqueue_head(&sptr_pool->sgtc_wqh);
    spin_lock_init(&sptr_pool->sgtc_lock);
}

/*!
 * @brief   kmget_size
 * @param   flags
 * @retval  none
 * @note    read memory total lenth
 */
__weak kssize_t kmget_size(nrt_gfp_t flags)
{
    struct fwk_mempool *sptr_pool = mr_nullptr;
    kuint32_t index;

    for (index = 0; index < FWK_MEMPOOL_TYPE_MAX; index++)
    {
        if (flags & sgtc_kernel_mempool[index].mask)
        {
            if (!sptr_pool)
                sptr_pool = &sgtc_kernel_mempool[index];
            else
                return -ER_INVALID;
        }
    }

    if (!sptr_pool)
        return -ER_NOTFOUND;

    return sptr_pool->sptr_info->lenth;
}

/*!
 * @brief   kmalloc
 * @param   __size
 * @retval  none
 * @note    kernel memory pool allocate
 */
__weak void *kmalloc(size_t __size, nrt_gfp_t flags)
{
    struct fwk_mempool *sptr_pool = mr_nullptr;
    struct mem_info *sptr_info;
    void *p = mr_nullptr;
    kuint32_t index;

    for (index = 0; index < FWK_MEMPOOL_TYPE_MAX; index++)
    {
        if (flags & sgtc_kernel_mempool[index].mask)
        {
            if (!sptr_pool)
                sptr_pool = &sgtc_kernel_mempool[index];
            else
                return p;
        }
    }

    if (!sptr_pool)
        return p;
    
    if (flags & NR_KMEM_WAIT)
        wait_event(&sptr_pool->sgtc_wqh, !spin_is_locked(&sptr_pool->sgtc_lock));

    spin_lock_irqsave(&sptr_pool->sgtc_lock);

    sptr_info = sptr_pool->sptr_info;
    if (sptr_info->alloc)
    {
        p = sptr_info->alloc(sptr_info, __size);
        if (!isValid(p))
        {
            p = mr_nullptr;
            goto END;
        }

        if (flags & NR_KMEM_ZERO)
            kmemzero(p, __size);
    }

END:
    spin_unlock_irqrestore(&sptr_pool->sgtc_lock);

    return p;
}

/*!
 * @brief   kcalloc
 * @param   __size, __n
 * @retval  none
 * @note    kernel memory pool allocate (array)
 */
__weak void *kcalloc(size_t __size, size_t __n, nrt_gfp_t flags)
{
    return kmalloc(__size * __n, flags);
}

/*!
 * @brief   kzalloc
 * @param   __size
 * @retval  none
 * @note    kernel memory pool allocate, and reset automatically
 */
__weak void *kzalloc(size_t __size, nrt_gfp_t flags)
{
    return kmalloc(__size, flags | GFP_ZERO);
}

/*!
 * @brief   default malloc
 * @param   size
 * @retval  none
 * @note    none
 */
void *default_malloc(kusize_t size)
{
	return kmalloc(size, GFP_KERNEL);
}

/*!
 * @brief   kfree
 * @param   __ptr
 * @retval  none
 * @note    kernel memory pool free
 */
__weak void kfree(void *__ptr)
{
    struct fwk_mempool *sptr_pool = mr_nullptr;
    struct mem_info *sptr_info = mr_nullptr;
    kuint32_t index;

    for (index = 0; index < FWK_MEMPOOL_TYPE_MAX; index++)
    {
        sptr_pool = &sgtc_kernel_mempool[index];
        sptr_info = sptr_pool->sptr_info;

        if ((__ptr >= (void *)sptr_info->base) &&
            (__ptr <  (void *)(sptr_info->base + sptr_info->lenth)))
            break;
    }

    /*!< not found: out of pool */
    if (index == FWK_MEMPOOL_TYPE_MAX)
        return;

    spin_lock_irqsave(&sptr_pool->sgtc_lock);
    if (sptr_info->free)
        sptr_info->free(sptr_info, __ptr);
    spin_unlock_irqrestore(&sptr_pool->sgtc_lock);
}

/* end of file */
