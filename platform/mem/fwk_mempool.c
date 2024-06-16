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

/*!< The globals */
struct fwk_mempool
{
	struct mem_info *sprt_info;
	struct wait_queue_head sgrt_wqh;
	struct spin_lock sgrt_lock;
};

static struct mem_info sgrt_kernel_mem_info =
{
	.base	= 0,
	.lenth	= 0,

	.sprt_mem = mrt_nullptr,
};

static struct fwk_mempool sgrt_kernel_mempool =
{
	.sprt_info = &sgrt_kernel_mem_info,
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
	struct fwk_mempool *sprt_pool = &sgrt_kernel_mempool;
	struct mem_info *sprt_info = sprt_pool->sprt_info;

	if (isValid(sprt_info->sprt_mem))
		return false;

	memory_simple_block_create(sprt_info, 
							   MEMORY_POOL_BASE, 
							   MEMORY_POOL_SIZE);

	init_waitqueue_head(&sprt_pool->sgrt_wqh);
	spin_lock_init(&sprt_pool->sgrt_lock);

	return true;
}

/*!
 * @brief   memory_block_self_defines
 * @param   none
 * @retval  none
 * @note    memory block initial
 */
kbool_t memory_block_self_defines(kuaddr_t base, kusize_t size)
{
	struct fwk_mempool *sprt_pool = &sgrt_kernel_mempool;
	struct mem_info *sprt_info = sprt_pool->sprt_info;

	if (isValid(sprt_info->sprt_mem))
		return false;

	memory_simple_block_create(sprt_info, base, size);

	return true;
}

/*!
 * @brief   memory_block_self_destroy
 * @param   none
 * @retval  none
 * @note    memory block destroy
 */
void memory_block_self_destroy(void)
{
	struct fwk_mempool *sprt_pool = &sgrt_kernel_mempool;
	struct mem_info *sprt_info = sprt_pool->sprt_info;

	memory_simple_block_destroy(sprt_info);
	init_waitqueue_head(&sprt_pool->sgrt_wqh);
	spin_lock_init(&sprt_pool->sgrt_lock);
}

/*!
 * @brief   kmalloc
 * @param   __size
 * @retval  none
 * @note    kernel memory pool allocate
 */
__weak void *kmalloc(size_t __size, ert_fwk_mempool_t flags)
{
	struct fwk_mempool *sprt_pool = &sgrt_kernel_mempool;
	struct mem_info *sprt_info = sprt_pool->sprt_info;
	void *p = mrt_nullptr;

	if (!(flags & NR_KMEM_NORMAL))
		return p;
	
	if (flags & NR_KMEM_WAIT)
		wait_event(&sprt_pool->sgrt_wqh, !spin_is_locked(&sprt_pool->sgrt_lock));

	spin_lock_irqsave(&sprt_pool->sgrt_lock);

	p = alloc_spare_simple_memory(sprt_info->sprt_mem, __size);
	if (!isValid(p))
	{
		p = mrt_nullptr;
		goto END;
	}

	if (flags & NR_KMEM_ZERO)
		memory_reset(p, __size);

END:
	spin_unlock_irqrestore(&sprt_pool->sgrt_lock);

	return p;
}

/*!
 * @brief   kcalloc
 * @param   __size, __n
 * @retval  none
 * @note    kernel memory pool allocate (array)
 */
__weak void *kcalloc(size_t __size, size_t __n, ert_fwk_mempool_t flags)
{
	return kmalloc(__size * __n, flags);
}

/*!
 * @brief   kzalloc
 * @param   __size
 * @retval  none
 * @note    kernel memory pool allocate, and reset automatically
 */
__weak void *kzalloc(size_t __size, ert_fwk_mempool_t flags)
{
	return kmalloc(__size, flags | GFP_ZERO);
}

/*!
 * @brief   kfree
 * @param   __ptr
 * @retval  none
 * @note    kernel memory pool free
 */
__weak void kfree(void *__ptr)
{
	struct fwk_mempool *sprt_pool = &sgrt_kernel_mempool;
	struct mem_info *sprt_info = sprt_pool->sprt_info;

	if (__ptr < (void *)sprt_info->base)
		return;
	if (__ptr >= (void *)(sprt_info->base + sprt_info->lenth))
		return;

	spin_lock_irqsave(&sprt_pool->sgrt_lock);
	free_employ_simple_memory(sprt_info->sprt_mem, __ptr);
	spin_unlock_irqrestore(&sprt_pool->sgrt_lock);
}

/* end of file */
