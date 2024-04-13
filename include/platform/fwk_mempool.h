/*
 * Memory Control For Kernel
 *
 * File Name:   fwk_mempool.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.10.02
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_MEMPOOL_H
#define __FWK_MEMPOOL_H

/*!< The includes */
#include <common/generic.h>
#include <common/mem_manage.h>

/*!< The defines */
typedef enum ert_fwk_mempool
{
    NR_KMem_GfpZero = mrt_bit(0),
    NR_KMem_GfpKernel = mrt_bit(1),

} ert_fwk_mempool_t;

#define GFP_ZERO                                NR_KMem_GfpZero
#define GFP_KERNEL                              NR_KMem_GfpKernel

/*!< The functions */
TARGET_EXT kbool_t fwk_mempool_initial(void);
TARGET_EXT void *kmalloc(size_t __size, ert_fwk_mempool_t flags);
TARGET_EXT void *kzalloc(size_t __size, ert_fwk_mempool_t flags);
TARGET_EXT void kfree(void *__ptr);

TARGET_EXT kbool_t memory_block_self_defines(kuaddr_t base, kusize_t size);
TARGET_EXT void memory_block_self_destroy(void);

#endif  /* __FWK_MEMPOOL_H */
