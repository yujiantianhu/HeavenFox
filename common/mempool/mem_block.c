/*
 * Memory Block Management
 *
 * File Name:   mem_block.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.01.19
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/error_types.h>
#include <common/mem_manage.h>

/*!< The defines */

/*!< The functions */
static struct mem_block *check_employ_memory(void *ptr_head, void *ptr_mem);
static void *alloc_spare_memory(struct mem_info *sptr_info, kusize_t size);
static void free_employ_memory(struct mem_info *sptr_info, void *ptr_mem);

/*!< API function */
/*!
 * @brief   select a correct hash list
 * @param   sptr_info, total_size
 * @retval  mem_hash
 * @note    determine a hash list to add a new block
 */
static struct mem_hash *memory_block_get_hash(struct mem_info *sptr_info, kusize_t total_size)
{
    kint32_t index = NR_MEM_HighBytes;

    if (total_size >= NR_MEM_HighLimit)
        return &sptr_info->sgtc_hash[NR_MEM_HighBytes];

    if (total_size < NR_MEM_LowerLimit)
        return &sptr_info->sgtc_hash[NR_MEM_LowerBytes];

    while (!(total_size & (1U << index)))
        index--;

    return &sptr_info->sgtc_hash[index];
}

/*!
 * @brief   add the new block to hash list
 * @param   sptr_info, sptr_hash, sptr_block
 * @retval  none
 * @note    if sptr_hash is valid, add to it directly
 */
static void memory_block_attach(struct mem_info *sptr_info, struct mem_hash *sptr_hash, struct mem_block *sptr_block)
{
    struct mem_block *sptr_per;

    if (!sptr_block->remain)
        return;

    if (!sptr_hash)
        sptr_hash = memory_block_get_hash(sptr_info, sptr_block->remain);
    
    init_list_head(&sptr_block->sgtc_link);

    /*!< case 1: list is empty */
    if (mr_list_empty(&sptr_hash->sgtc_list))
    {
        list_head_add_head(&sptr_hash->sgtc_list, &sptr_block->sgtc_link);
        return;
    }

    /*!< case 2: head < adress of block < tail */
    foreach_list_next_entry(sptr_per, &sptr_hash->sgtc_list, sgtc_link)
    {
        /*!< low address in front for easy access next time */
        if (sptr_block < sptr_per)
        {
            list_head_add_tail(&sptr_per->sgtc_link, &sptr_block->sgtc_link);
            return;
        }
    }

    /*!< case 3: block's address is the largest */
    list_head_add_tail(&sptr_hash->sgtc_list, &sptr_block->sgtc_link);
}

/*!
 * @brief   detached from hash list
 * @param   sptr_block
 * @retval  none
 * @note    none
 */
static void memory_block_detach(struct mem_block *sptr_block)
{
    list_head_del(&sptr_block->sgtc_link);
}

/*!
 * @brief   memory_block_create
 * @param   sptr_info, mem_addr, size
 * @retval  none
 * @note    build memory block
 */
kint32_t memory_block_create(struct mem_info *sptr_info, kuaddr_t mem_addr, kusize_t size)
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

    sptr_info->sptr_mem	= sptr_block;
    for (kint32_t index = 0; index < NR_MEM_NUM; index++)
    {
        struct mem_hash *sptr_hash;

        sptr_hash = &sptr_info->sgtc_hash[index];
        init_list_head(&sptr_hash->sgtc_list);
    }

    sptr_info->alloc = alloc_spare_memory;
    sptr_info->free = free_employ_memory;

    memory_block_attach(sptr_info, mr_nullptr, sptr_block);
    return ER_NORMAL;
}

/*!
 * @brief   memory_block_destroy
 * @param   sptr_info
 * @retval  none
 * @note    destroy memory block which is created
 */
void memory_block_destroy(struct mem_info *sptr_info)
{
    if (!isValid(sptr_info))
        return;

    /*!< clear all memory blocks */
    kmemzero((void *)sptr_info->base, sptr_info->lenth);
    kmemzero((void *)sptr_info, sizeof(struct mem_info));
}

/*!
 * @brief   find a hash list with enough space
 * @param   sptr_info, index(index of hash list), total_size
 * @retval  none
 * @note    none
 */
static struct mem_block *__memory_block_get_avaliable(struct mem_info *sptr_info, kint32_t index, kusize_t total_size)
{
    struct mem_block *sptr_block;
    struct mem_hash *sptr_hash = &sptr_info->sgtc_hash[index];

    foreach_list_next_entry(sptr_block, &sptr_hash->sgtc_list, sgtc_link) 
    {
        if (sptr_block->remain >= total_size)
            return sptr_block;
    }

    return mr_nullptr;
}

/*!
 * @brief   find a hash list with enough space
 * @param   sptr_info, index(index of hash list), total_size
 * @retval  mem_block
 * @note    if the preferred hash list space is insufficient, select the next hash list
 */
static struct mem_block *memory_block_get_avaliable(struct mem_info *sptr_info, kusize_t total_size)
{
    struct mem_block *sptr_block;
    kint32_t index = NR_MEM_HighBytes;

    if (total_size >= NR_MEM_HighLimit)
        return __memory_block_get_avaliable(sptr_info, NR_MEM_HighBytes, total_size);

    if (total_size < NR_MEM_LowerLimit)
    {
        sptr_block = __memory_block_get_avaliable(sptr_info, NR_MEM_LowerBytes, total_size);
        if (sptr_block)
            return sptr_block;

        total_size = NR_MEM_LowerLimit;
    }

    /*!< for example: total_size = 32, index will be 5 */
    while (!(total_size & (1U << index)))
        index--;

    while (index < NR_MEM_NUM)
    {
        sptr_block = __memory_block_get_avaliable(sptr_info, index, total_size);
        if (sptr_block)
            return sptr_block;

        index++;
    }

    return mr_nullptr;
}

/*!
 * @brief   alloc_spare_memory
 * @param   ptr_head, size
 * @retval  avaliable memory block pointer
 * @note    allocate spare memory space
 */
static void *alloc_spare_memory(struct mem_info *sptr_info, kusize_t size)
{
    struct mem_block *sptr_block;
    struct mem_block *sptr_new;
    kusize_t header_size, lenth, offset;
    void *ptr_mem = mr_nullptr;

    if (!isValid(sptr_info) ||
        !sptr_info->sptr_mem)
        return mr_nullptr;

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

    /*!< find a block with enough space left */
    sptr_block = memory_block_get_avaliable(sptr_info, lenth);
    if (!sptr_block)
        return mr_nullptr;

    memory_block_detach(sptr_block);

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
        memory_block_attach(sptr_info, mr_nullptr, sptr_new);
    }
    else
    {
    	sptr_block->magic = MEMORY_POOL_MAGIC;

        /*!< Update the lenth of memory that is avaliable */
        sptr_block->remain -= lenth;
    }

    memory_block_attach(sptr_info, mr_nullptr, sptr_block);
    return ptr_mem;
}

/*!
 * @brief   check_employ_memory
 * @param   ptr_mem
 * @retval  none
 * @note    check if ptr_mem was allocated from ptr_head
 */
static struct mem_block *check_employ_memory(void *ptr_head, void *ptr_mem)
{
    struct mem_block *sptr_block;

    /*!< Point to the head of info */
    sptr_block = (struct mem_block *)((kuint8_t *)ptr_mem - MEM_BLOCK_HEADER_SIZE);
    if ((!isValid(sptr_block)) || (!IS_MEMORYPOOL_VALID(sptr_block)))
        return mr_nullptr;

#if 1
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
 * @brief   free_employ_memory
 * @param   ptr_mem
 * @retval  none
 * @note    free memory block which is employed
 */
static void free_employ_memory(struct mem_info *sptr_info, void *ptr_mem)
{
    struct mem_block *sptr_prev;
    struct mem_block *sptr_next;
    struct mem_block *sptr_block;

    if (!isValid(ptr_mem))
        return;

    /*!< Point to the head of info */
    sptr_block = check_employ_memory(sptr_info->sptr_mem, ptr_mem);
    if (!sptr_block)
        return;

    sptr_block->magic = 0;

    sptr_prev = sptr_block->sptr_prev;
    sptr_next = sptr_block->sptr_next;

    /*!< Update memory space */
    sptr_block->remain = sptr_block->lenth;
    memory_block_detach(sptr_block);

    if (isValid(sptr_prev))
    {
        /*!< Check if the current memory block is idle, if yes, merge into the last neighboring memory block */
        sptr_prev->lenth += sptr_block->lenth;
        sptr_prev->remain += sptr_block->remain;
        sptr_prev->sptr_next = sptr_next;

        sptr_block->sptr_prev = mr_nullptr;
        sptr_block = sptr_prev;
        memory_block_detach(sptr_block);
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
  
    memory_block_attach(sptr_info, mr_nullptr, sptr_block);
}

/* end of file */
