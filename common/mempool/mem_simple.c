/*
 * Memory Simply Management
 *
 * File Name:   mem_simple.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.25
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/error_types.h>
#include <common/mem_manage.h>

/*!< The functions */
static struct mem_block *check_employ_simple_memory(void *ptr_head, void *ptr_mem);
static void *alloc_spare_simple_memory(struct mem_info *sptr_info, kusize_t size, kint32_t __align_of, struct m_area *sptr_real);
static void free_employ_simple_memory(struct mem_info *sptr_info, void *ptr_mem);

/*!< API function */
/*!
 * @brief   memory_block_create
 * @param   sptr_info, mem_addr, size
 * @retval  none
 * @note    build simple memory block
 */
kint32_t memory_simple_block_create(struct mem_info *sptr_info, kuaddr_t mem_addr, kusize_t size)
{
    struct mem_block *sptr_block;
    kusize_t header_size;

    /*!< header size */
    header_size	= MEM_BLOCK_HEADER_SIZE;

    if ((!isValid(sptr_info)) || (size <= header_size))
        return -ER_INVALID;

    /*!< if sptr_mem is exsited, it is not allow to create again */
    if (isValid(sptr_info->sptr_mem))
        return -ER_INVALID;

    /*!< 8 bytes align */
    sptr_info->base	= mr_num_align8(mem_addr);
    sptr_info->lenth = size - (sptr_info->base - mem_addr);
    
    sptr_block = (struct mem_block *)sptr_info->base;
    sptr_block->base = sptr_info->base + header_size;

    /*!< size = header size + memory size, so we can let sptr_block->lenth equal to total lenth */
    sptr_block->lenth = sptr_info->lenth;
    sptr_block->remain = sptr_info->lenth;
    sptr_block->sptr_prev = mr_nullptr;
    sptr_block->sptr_next = mr_nullptr;
    sptr_block->magic = MEMORY_POOL_MAGIC;
    init_list_head(&sptr_block->sgtc_link);

    sptr_info->sptr_mem	= sptr_block;
    for (kint32_t index = 0; index < NR_MEM_NUM; index++)
    {
        struct mem_hash *sptr_hash;

        sptr_hash = &sptr_info->sgtc_hash[index];
        init_list_head(&sptr_hash->sgtc_list);
    }

    sptr_info->alloc = alloc_spare_simple_memory;
    sptr_info->free = free_employ_simple_memory;

    return ER_NORMAL;
}

/*!
 * @brief   memory_simple_block_destroy
 * @param   sptr_info
 * @retval  none
 * @note    destroy memory block which is created
 */
void memory_simple_block_destroy(struct mem_info *sptr_info)
{
    if (!isValid(sptr_info))
        return;

    /*!< clear all memory blocks */
    kmemzero((void *)sptr_info->base, sptr_info->lenth);
    kmemzero((void *)sptr_info, sizeof(struct mem_info));
}

/*!
 * @brief   alloc_spare_simple_memory
 * @param   ptr_head, size
 * @retval  avaliable memory block pointer
 * @note    allocate spare memory space
 */
static void *alloc_spare_simple_memory(struct mem_info *sptr_info, kusize_t size, kint32_t __align_of, struct m_area *sptr_real)
{
    struct mem_block *sptr_start;
    struct mem_block *sptr_block;
    struct mem_block *sptr_new = mr_nullptr;
    kusize_t header_size, lenth, offset;
    void *ptr_mem = mr_nullptr;

    if (!isValid(sptr_info) ||
        !sptr_info->sptr_mem)
        return mr_nullptr;

    sptr_start  = sptr_info->sptr_mem;
    header_size	= MEM_BLOCK_HEADER_SIZE;

    /*!< 8 bytes alignment for memory block lenth */
    /*!< Build complete space: lenth + header size, adapted to sptr_block->lenth */
    lenth  = mr_num_align8(size);
    lenth += header_size;

    /*!< Several methods for dividing memory:
     * 1. After allocating memory, the original memory is not divided, and the original lenth is retained; --->
     *	  it only returns the memory address. When the seconed allocating comes, it will split the remaining memory block, --->
     *    and create a new block (the remaining memory is inclued by the new memory block)
     * 2. Split and seprate the allocated memory inmediately, and two blocks are generated for the split and remaining memory.
     *
     * it takes the first method here.
     */

    for (sptr_block = sptr_start; isValid(sptr_block); sptr_block = sptr_block->sptr_next)
    {
        /*!< Remaining memory is unable to be applied, find next avaliable block */
        if (sptr_block->remain >= lenth)
        {
            /*!< p = base + (lenth - header_size - remain) + header_size = base + lenth - remain */
            offset 	= sptr_block->lenth - sptr_block->remain;
            ptr_mem = (void *)((kuint8_t *)sptr_block->base + offset);

            /*!< Build a new memory block */
            if (sptr_block->lenth != sptr_block->remain)
            {
                /*!< Move to the head of memory block */
                sptr_new = (struct mem_block *)((kuint8_t *)ptr_mem - header_size);
                sptr_new->base = (kuaddr_t)ptr_mem;
                sptr_new->lenth = sptr_block->remain;
                sptr_new->remain = sptr_block->remain - lenth;
                sptr_new->sptr_prev = sptr_block;
                sptr_new->sptr_next = sptr_block->sptr_next;
                sptr_new->magic = MEMORY_POOL_MAGIC;

                sptr_block->lenth = offset;
                sptr_block->remain -= sptr_new->lenth;

                if (sptr_block->sptr_next)
                	sptr_block->sptr_next->sptr_prev = sptr_new;
                
                sptr_block->sptr_next = sptr_new;
                init_list_head(&sptr_new->sgtc_link);
            }
            else
            {
                /*!< Update the lenth of memory that is avaliable */
                sptr_block->remain -= lenth;
            }

            /*!< Record information */
            if (mr_likely(sptr_real))
            {
                sptr_real->base = sptr_new ? (void *)sptr_new : (void *)sptr_block;
                sptr_real->offset = header_size;
                sptr_real->size = lenth;
            }

            break;
        }
    }

    return ptr_mem;
}

/*!
 * @brief   check_employ_simple_memory
 * @param   ptr_mem
 * @retval  none
 * @note    check if ptr_mem was allocated from ptr_head
 */
static struct mem_block *check_employ_simple_memory(void *ptr_head, void *ptr_mem)
{
    struct mem_block *sptr_block;

    /*!< Point to the head of info */
    sptr_block = (struct mem_block *)((kuint8_t *)ptr_mem - MEM_BLOCK_HEADER_SIZE);
    if ((!isValid(sptr_block)) || (!IS_MEMORYPOOL_VALID(sptr_block)))
        return mr_nullptr;

#if 0
    __RESERVED(ptr_head);

    if (sptr_block->base != (kuaddr_t)ptr_mem)
        return mr_nullptr;

    return sptr_block;
    
#else
    /*!< check if ptr_mem was allocated from ptr_head */
    for (struct mem_block *sptr_start = (struct mem_block *)ptr_head; 
        isValid(sptr_start); sptr_start = sptr_start->sptr_next)
    {
        if (sptr_start->base == (kuaddr_t)ptr_mem)
            return sptr_block;
    }

    return mr_nullptr;
#endif
}

/*!
 * @brief   free_employ_simple_memory
 * @param   ptr_mem
 * @retval  none
 * @note    free memory block which is employed
 */
static void free_employ_simple_memory(struct mem_info *sptr_info, void *ptr_mem)
{
    struct mem_block *sptr_prev;
    struct mem_block *sptr_next;
    struct mem_block *sptr_block;

    if (!isValid(ptr_mem))
        return;

    /*!< Point to the head of info */
    sptr_block = check_employ_simple_memory(sptr_info->sptr_mem, ptr_mem);
    if (!sptr_block)
        return;

    sptr_prev = sptr_block->sptr_prev;
    sptr_next = sptr_block->sptr_next;

    /*!< Update memory space */
    sptr_block->remain = sptr_block->lenth;

    if (isValid(sptr_prev))
    {
        /*!< Check if the current memory block is idle, if yes, merge into the last neighboring memory block */
        sptr_prev->lenth += sptr_block->lenth;
        sptr_prev->remain += sptr_block->remain;
        sptr_prev->sptr_next = sptr_next;

        sptr_block = sptr_prev;
    }

    if (isValid(sptr_next))
    {
//      /*!< Check if the next neighboring memory block is idle, if yes, the idle memory block should be merged */
//      if (sptr_next->lenth == sptr_next->remain)
//      {
//          sptr_block->sptr_next = sptr_next->sptr_next;
//          sptr_block->lenth += sptr_next->lenth;
//          sptr_block->remain += sptr_next->remain;
//      }

        sptr_next->sptr_prev = sptr_block;
    }
}

/* end of file */
