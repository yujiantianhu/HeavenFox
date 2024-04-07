/*
 * Memory Control For Kernel
 *
 * File Name:   kmem_pool.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.10.02
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __KMEM_POOL_H
#define __KMEM_POOL_H

/*!< The includes */
#include <common/generic.h>
#include <common/mem_manage.h>

/*!< The defines */
typedef enum ert_kmem_pool
{
    Ert_KMem_GfpZero = mrt_bit(0),
    Ert_KMem_GfpKernel = mrt_bit(1),

} ert_kmem_pool_t;

#define GFP_ZERO                                Ert_KMem_GfpZero
#define GFP_KERNEL                              Ert_KMem_GfpKernel

/*!< The functions */
TARGET_EXT kbool_t kernel_memblock_initial(void);
TARGET_EXT void *kmalloc(size_t __size, ert_kmem_pool_t flags);
TARGET_EXT void *kzalloc(size_t __size, ert_kmem_pool_t flags);
TARGET_EXT void kfree(void *__ptr);

TARGET_EXT kbool_t memory_block_self_defines(kuaddr_t base, kusize_t size);
TARGET_EXT void memory_block_self_destroy(void);

#endif  /* __KMEM_POOL_H */
