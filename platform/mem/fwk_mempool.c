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
#include <platform/base/fwk_mempool.h>
#include <kernel/sched.h>
#include <kernel/wait.h>
#include <kernel/spinlock.h>

/*!< The defines */
struct fwk_mem_info
{
    struct mem_info sgtc_info;
    struct m_area sgtc_maxrec;
};

struct fwk_mempool
{
    const kchar_t *name;
    kuint32_t mask;
    struct fwk_mem_info *sptr_mn;

    struct wait_queue_head sgtc_wqh;
    struct spin_lock sgtc_lock;
};

/*!< The globals */
static struct fwk_mem_info sgtc_mempool_info[NR_FWK_MEMPOOL_TYPE_MAX] = {};

/*!< Memory Pool Area */
static struct fwk_mempool sgtc_kernel_mempool[NR_FWK_MEMPOOL_TYPE_MAX] =
{
    [NR_FWK_MEMPOOL_KERNEL] = {
        .name = "kernel heap",
        .mask = NR_KMEM_NORMAL,
        .sptr_mn = &sgtc_mempool_info[NR_FWK_MEMPOOL_KERNEL],
    },

    [NR_FWK_MEMPOOL_DMA] = {
        .name = "dma",
        .mask = NR_KMEM_DMA_AREA,
        .sptr_mn = &sgtc_mempool_info[NR_FWK_MEMPOOL_DMA],
    },

    [NR_FWK_MEMPOOL_SK_BUFF] = {
        .name = "network",
        .mask = NR_KMEM_SK_BUFF,
        .sptr_mn = &sgtc_mempool_info[NR_FWK_MEMPOOL_SK_BUFF],
    },

    [NR_FWK_MEMPOOL_FB_DRAM] = {
        .name = "framebuffer",
        .mask = NR_KMEM_FBUFFER,
        .sptr_mn = &sgtc_mempool_info[NR_FWK_MEMPOOL_FB_DRAM],
    },

    [NR_FWK_MEMPOOL_FIXDATA] = {
        .name = "fixed data",
        .mask = NR_KMEM_FIXDATA,
        .sptr_mn = &sgtc_mempool_info[NR_FWK_MEMPOOL_FIXDATA],
    },
};

/*!< The functions */
extern void wake_up_kmemp_thread(void);
extern void kmemp_list_add(struct fwk_memp_list *sptr_memp);

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
    sptr_pool = &sgtc_kernel_mempool[NR_FWK_MEMPOOL_KERNEL];
    sptr_info = &sptr_pool->sptr_mn->sgtc_info;
    retval = memory_block_create(sptr_info, MEMORY_POOL_BASE, MEMORY_POOL_SIZE);
    if (retval)
        return false;
    
    init_waitqueue_head(&sptr_pool->sgtc_wqh);
    spin_lock_init(&sptr_pool->sgtc_lock);

    /*!< ------------------------------------------------------------ */
    sptr_pool = &sgtc_kernel_mempool[NR_FWK_MEMPOOL_DMA];
    sptr_info = &sptr_pool->sptr_mn->sgtc_info;
    memory_block_create(sptr_info, DMA_AREA_BASE, DMA_AREA_SIZE);
    init_waitqueue_head(&sptr_pool->sgtc_wqh);
    spin_lock_init(&sptr_pool->sgtc_lock);

    /*!< ------------------------------------------------------------ */
    sptr_pool = &sgtc_kernel_mempool[NR_FWK_MEMPOOL_FB_DRAM];
    sptr_info = &sptr_pool->sptr_mn->sgtc_info;
    memory_simple_block_create(sptr_info, FBUFFER_DRAM_BASE, FBUFFER_DRAM_SIZE);
    init_waitqueue_head(&sptr_pool->sgtc_wqh);
    spin_lock_init(&sptr_pool->sgtc_lock);

    /*!< ------------------------------------------------------------ */
    sptr_pool = &sgtc_kernel_mempool[NR_FWK_MEMPOOL_SK_BUFF];
    sptr_info = &sptr_pool->sptr_mn->sgtc_info;
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
    kuint8_t area;

    area = (flags <= 0) ? GFP_GET_AREA(NR_KMEM_KERNEL) : GFP_GET_AREA(flags);
    if (mr_unlikely(!isPower2(area)))
        return mr_nullptr;

    index = ffs_u8(area) - 1;
    if (mr_unlikely(index >= NR_FWK_MEMPOOL_TYPE_MAX))
        return mr_nullptr;

    sptr_pool = &sgtc_kernel_mempool[index];
    sptr_info = &sptr_pool->sptr_mn->sgtc_info;

    if (mr_unlikely(isValid(sptr_info->sptr_mem)))
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
    kuint8_t area;

    area = (flags <= 0) ? GFP_GET_AREA(NR_KMEM_KERNEL) : GFP_GET_AREA(flags);
    if (mr_unlikely(!isPower2(area)))
        return;

    index = ffs_u8(area) - 1;
    if (mr_unlikely(index >= NR_FWK_MEMPOOL_TYPE_MAX))
        return;

    sptr_pool = &sgtc_kernel_mempool[index];
    sptr_info = &sptr_pool->sptr_mn->sgtc_info;

    if (mr_unlikely(!isValid(sptr_info->sptr_mem)))
        return;

    memory_simple_block_destroy(sptr_info);
    init_waitqueue_head(&sptr_pool->sgtc_wqh);
    spin_lock_init(&sptr_pool->sgtc_lock);
}

/*!
 * @brief   kmget_area_record
 * @param   area_index: refer to "__ERT_FWK_MEMPOOL_INDEX"
 * @retval  sgtc_maxrec
 * @note    read memory record about the maximum address allocated
 */
struct m_area *kmget_area_record(kuint32_t area_index)
{
    return ((area_index < NR_FWK_MEMPOOL_TYPE_MAX) ? 
                &sgtc_kernel_mempool[area_index].sptr_mn->sgtc_maxrec : mr_nullptr);
}

/*!
 * @brief   kmget_area_base_address
 * @param   area_index: refer to "__ERT_FWK_MEMPOOL_INDEX"
 * @retval  none
 * @note    read memory base address
 */
kuaddr_t kmget_area_base_address(kuint32_t area_index)
{
    return ((area_index < NR_FWK_MEMPOOL_TYPE_MAX) ? 
                sgtc_kernel_mempool[area_index].sptr_mn->sgtc_info.base : 0);
}

/*!
 * @brief   kmget_area_total_size
 * @param   area_index: refer to "__ERT_FWK_MEMPOOL_INDEX"
 * @retval  none
 * @note    read memory total lenth
 */
kssize_t kmget_area_total_size(kuint32_t area_index)
{
    return ((area_index < NR_FWK_MEMPOOL_TYPE_MAX) ? 
                sgtc_kernel_mempool[area_index].sptr_mn->sgtc_info.lenth : -1);
}

/*!
 * @brief   kmget_area_label
 * @param   area_index: refer to "__ERT_FWK_MEMPOOL_INDEX"
 * @retval  none
 * @note    read name
 */
const kchar_t *kmget_area_label(kuint32_t area_index)
{
    return ((area_index < NR_FWK_MEMPOOL_TYPE_MAX) ? 
                sgtc_kernel_mempool[area_index].name : mr_nullptr);
}

/*!
 * @brief   kmget_size
 * @param   flags
 * @retval  none
 * @note    read memory total lenth
 */
kssize_t kmget_size(nrt_gfp_t flags)
{
    struct fwk_mempool *sptr_pool = mr_nullptr;
    kuint32_t index;
    kuint8_t area = GFP_GET_AREA(flags);

    if (mr_unlikely(!isPower2(area)))
        return -ER_INVALID;

    index = ffs_u8(area) - 1;
    if (mr_unlikely(index >= NR_FWK_MEMPOOL_TYPE_MAX))
        return -ER_INVALID;

    sptr_pool = &sgtc_kernel_mempool[index];
    return sptr_pool->sptr_mn->sgtc_info.lenth;
}

/*!
 * @brief   kmalloc
 * @param   __size
 * @retval  none
 * @note    kernel memory pool allocate
 */
void *kmalloc(size_t __size, nrt_gfp_t flags)
{
    struct fwk_mempool *sptr_pool = mr_nullptr;
    struct mem_info *sptr_info;
    void *p = mr_nullptr;
    struct m_area *sptr_record;
    kuint32_t index;
    kuint8_t area = GFP_GET_AREA(flags);
    struct m_area sgtc_real;
    void *address_end;

    if (mr_unlikely(!isPower2(area)))
        return mr_nullptr;

    index = ffs_u8(area) - 1;
    if (mr_unlikely(index >= NR_FWK_MEMPOOL_TYPE_MAX))
        return mr_nullptr;

    sptr_pool = &sgtc_kernel_mempool[index];
    sptr_info = &sptr_pool->sptr_mn->sgtc_info;

    if (mr_unlikely(!sptr_info->alloc))
        return mr_nullptr;
    
    if (mr_unlikely(flags & NR_KMEM_WAIT))
        wait_event(&sptr_pool->sgtc_wqh, !spin_is_locked(&sptr_pool->sgtc_lock));

    sptr_record = &sptr_pool->sptr_mn->sgtc_maxrec;
    address_end = sptr_record->base + sptr_record->size;

    spin_lock_irqsave(&sptr_pool->sgtc_lock);

    p = sptr_info->alloc(sptr_info, __size, &sgtc_real);
    if (mr_unlikely(!isValid(p)))
    {
        spin_unlock_irqrestore(&sptr_pool->sgtc_lock);
        return mr_nullptr;
    }

    /*!< Record the maximum p */
    if ((sgtc_real.base + sgtc_real.size) > address_end)
        memcpy(sptr_record, &sgtc_real, sizeof(sgtc_real));
    
    spin_unlock_irqrestore(&sptr_pool->sgtc_lock);

    if (flags & NR_KMEM_ZERO)
        kmemzero(p, __size);

    return p;
}

/*!
 * @brief   kcalloc
 * @param   __size, __n
 * @retval  none
 * @note    kernel memory pool allocate (array)
 */
void *kcalloc(size_t __size, size_t __n, nrt_gfp_t flags)
{
    return kmalloc(__size * __n, flags);
}

/*!
 * @brief   kzalloc
 * @param   __size
 * @retval  none
 * @note    kernel memory pool allocate, and reset automatically
 */
void *kzalloc(size_t __size, nrt_gfp_t flags)
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
void kfree(void *__ptr)
{
    struct fwk_mempool *sptr_pool = mr_nullptr;
    struct mem_info *sptr_info = mr_nullptr;
    kuint32_t index;

    for (index = 0; index < NR_FWK_MEMPOOL_TYPE_MAX; index++)
    {
        sptr_pool = &sgtc_kernel_mempool[index];
        sptr_info = &sptr_pool->sptr_mn->sgtc_info;

        if ((__ptr >= (void *)sptr_info->base) &&
            (__ptr <  (void *)(sptr_info->base + sptr_info->lenth)))
            break;
    }

    /*!< not found: out of pool */
    if (index == NR_FWK_MEMPOOL_TYPE_MAX)
        return;

    spin_lock_irqsave(&sptr_pool->sgtc_lock);
    if (sptr_info->free)
        sptr_info->free(sptr_info, __ptr);
    spin_unlock_irqrestore(&sptr_pool->sgtc_lock);
}

/*!
 * @brief   release mem list
 * @param   sptr_memp
 * @retval  none
 * @note    kernel memory pool free (called by "kmemp thread")
 */
static void fwk_memp_release(struct fwk_memp_list *sptr_memp)
{
    struct fwk_mempool *sptr_pool;
    struct mem_info *sptr_info;
    kuaddr_t memp_address;
    kuint32_t index;
    kuint8_t area = GFP_GET_AREA(sptr_memp->gfp_mask);

    if (mr_unlikely(!isPower2(area)))
        return;

    index = ffs_u8(area) - 1;
    if (mr_unlikely(index >= NR_FWK_MEMPOOL_TYPE_MAX))
        return;

    sptr_pool = &sgtc_kernel_mempool[index];
    sptr_info = &sptr_pool->sptr_mn->sgtc_info;

    memp_address = (kuaddr_t)sptr_memp;
    if (mr_unlikely((memp_address < sptr_info->base) ||
        (memp_address >= (sptr_info->base + sptr_info->lenth))))
        return;

    spin_lock_irqsave(&sptr_pool->sgtc_lock);
    if (mr_likely(sptr_info->free))
        sptr_info->free(sptr_info, sptr_memp);
    spin_unlock_irqrestore(&sptr_pool->sgtc_lock);
}

/*!
 * @brief   allocate memory
 * @param   __size
 * @retval  none
 * @note    kernel memory pool allocate
 */
void *fwk_malloc(kusize_t __size, nrt_gfp_t gfp_mask)
{
    struct fwk_memp_list *sptr_memp;

    sptr_memp = kmalloc(FWK_MEMP_SIZE + __size, gfp_mask);
    if (mr_unlikely(!isValid(sptr_memp)))
        return mr_nullptr;

    sptr_memp->magic = FWK_MEMP_MAGIC;
    sptr_memp->gfp_mask = gfp_mask;
    sptr_memp->sptr_next = mr_nullptr;
    sptr_memp->ptr = (void *)sptr_memp + FWK_MEMP_SIZE;
    sptr_memp->release = fwk_memp_release;

    return sptr_memp->ptr;
}

/*!
 * @brief   fwk_zalloc
 * @param   __size
 * @retval  none
 * @note    kernel memory pool allocate, and reset automatically
 */
void *fwk_zalloc(kusize_t __size, nrt_gfp_t gfp_mask)
{
    return fwk_malloc(__size, gfp_mask | GFP_ZERO);
}

/*!
 * @brief   fwk_calloc
 * @param   __size, __n
 * @retval  none
 * @note    kernel memory pool allocate (array)
 */
void *fwk_calloc(kusize_t __size, size_t __n, nrt_gfp_t gfp_mask)
{
    return fwk_malloc(__size * __n, gfp_mask);
}

/*!
 * @brief   fwk_free
 * @param   __ptr
 * @retval  none
 * @note    wake up kmemp thread to release memory
 */
void fwk_free(void *__ptr)
{
    struct fwk_memp_list *sptr_memp;
    kuaddr_t memp_address;

    if (mr_unlikely((kuaddr_t)__ptr < FWK_MEMP_SIZE))
        return;

    memp_address = (kuaddr_t)(__ptr - FWK_MEMP_SIZE);
    if (mr_unlikely(!mr_is_aligned(memp_address, ARCH_PER_SIZE)))
        return;

    sptr_memp = (struct fwk_memp_list *)memp_address;
    if (mr_unlikely((sptr_memp->magic != FWK_MEMP_MAGIC) ||
        (sptr_memp->ptr != __ptr) ||
        (sptr_memp->sptr_next)))
        return;

    kmemp_list_add(sptr_memp);
    wake_up_kmemp_thread();
}

/* end of file */
