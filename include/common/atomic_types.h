/*
 * Baisc Types Defines
 *
 * File Name:   atomic_types.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.25
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __ATOMIC_TYPES_H
#define __ATOMIC_TYPES_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include "basic_types.h"

/*!< The defines */
struct atomic
{
    /*!< 
     * atomic structure must have only one member (which is "counter")!!!
     * otherwise, functions such as "atomic_set_bit", "atomic_clear_bit" will generate incorrect calls!!!
     */
    kuint32_t counter;
};
typedef struct atomic srt_atomic_t;

#define DECLARE_ATOMIC(name)            struct atomic name = { .counter = 0 }
#define ATOMIC_SET(ptr, val)            do { (ptr)->counter = (val); } while (0)
#define ATOMIC_READ(ptr)                ((ptr)->counter)
#define ATOMIC_INIT()                   { .counter = 0 }

/*!< The functions */
/*!
 * @brief   atomic_get_val
 * @param   sptr_atomic
 * @retval  none
 * @note    get atomic
 */
static inline kuint32_t atomic_get_val(struct atomic *sptr_atomic)
{
    kuint32_t result;
    
    __asm__ __volatile__ (
        " ldrex %0, [%1]        \n\t"
        : "=r"(result)
        : "r"(&sptr_atomic->counter)
    );
    
    return result;
}

/*!
 * @brief   atomic_test_val
 * @param   sptr_atomic
 * @retval  none
 * @note    get atomic
 */
static inline kbool_t atomic_test_val(struct atomic *sptr_atomic)
{
    kuint32_t result;
    
    __asm__ __volatile__ (
        " ldrex %0, [%1]        \n\t"
        : "=r"(result)
        : "r"(&sptr_atomic->counter)
    );
    
    return !!result;
}

/*!
 * @brief   atomic_set_val
 * @param   val, sptr_atomic
 * @retval  none
 * @note    set atomic
 */
static inline void atomic_set_val(srt_atomic_t *sptr_atomic, kuint32_t val)
{
    kutype_t flag;
    kuint32_t result;

    __asm__ __volatile__ (
        " 1:	                \n\t"
        "   ldrex %0, [%3]		\n\t"
        "	mov %0, %2		    \n\t"
        "	strex %1, %0, [%3]	\n\t"
        "	teq %1, #0x0		\n\t"
        "	bne 1b				\n\t"
        : "=&r"(result), "=&r"(flag)
        : "r"(val), "r"(&sptr_atomic->counter)
        : "cc"
    );
}

/*!
 * @brief   atomic_add
 * @param   i, sptr_atomic
 * @retval  none
 * @note    add of atomic
 */
static inline void atomic_add(kint32_t i, srt_atomic_t *sptr_atomic)
{
    kutype_t flag;
    kuint32_t result;

    __asm__ __volatile__ (
        " 1:	                \n\t"
        "   ldrex %0, [%3]		\n\t"
        "	add %0, %0, %2		\n\t"
        "	strex %1, %0, [%3]	\n\t"
        "	teq %1, #0x0		\n\t"
        "	bne 1b				\n\t"
        : "=&r"(result), "=&r"(flag)
        : "r"(i), "r"(&sptr_atomic->counter)
        : "cc"
    );
}

/*!
 * @brief   atomic_sub
 * @param   i, sptr_atomic
 * @retval  none
 * @note    subtract of atomic
 */
static inline void atomic_sub(kint32_t i, srt_atomic_t *sptr_atomic)
{
    kutype_t flag;
    kuint32_t result;

    __asm__ __volatile__ (
        " 1:	                \n\t"
        "   ldrex %0, [%3]		\n\t"
        "	cmp %0, #0			\n\t"
        "	moveq %2, #0		\n\t"
        "	sub %0, %0, %2		\n\t"
        "	strex %1, %0, [%3]	\n\t"
        "	teq %1, #0x0		\n\t"
        "	bne 1b				\n\t"
        : "=&r"(result), "=&r"(flag)
        : "r"(i), "r"(&sptr_atomic->counter)
        : "cc"
    );
}

/*!
 * @brief   atomic_mul
 * @param   i, sptr_atomic
 * @retval  none
 * @note    multiply of atomic
 */
static inline void atomic_mul(kint32_t i, srt_atomic_t *sptr_atomic)
{
    kutype_t flag;
    kuint32_t result;

    __asm__ __volatile__ (
        " 1:		            \n\t"
        "   ldrex %0, [%3]		\n\t"
        "	mul %0, %0, %2		\n\t"
        "	strex %1, %0, [%3]	\n\t"
        "	teq %1, #0x0		\n\t"
        "	bne 1b				\n\t"
        : "=&r"(result), "=&r"(flag)
        : "r"(i), "r"(&sptr_atomic->counter)
        : "cc"
    );
}

/*!
 * @brief   atomic_sdiv
 * @param   i, sptr_atomic
 * @retval  none
 * @note    signed divide of atomic
 */
static inline void atomic_udiv(kuint32_t i, srt_atomic_t *sptr_atomic)
{
#if 0
    kutype_t flag;
    kuint32_t result;

    __asm__ __volatile__ (
        " 1:	                \n\t"
        "   ldrex %0, [%3]		\n\t"
        "	sdiv %0, %0, %2		\n\t"
        "	strex %1, %0, [%3]	\n\t"
        "	teq %1, #0x0		\n\t"
        "	bne 1b				\n\t"
        : "=&r"(result), "=&r"(flag)
        : "r"(i), "r"(&sptr_atomic->counter)
        : "cc"
    );

#else
    srt_atomic_t sgtc_temp = {};

    while (sptr_atomic->counter >= i)
    {
        atomic_sub(i, sptr_atomic);
        atomic_add(1, &sgtc_temp);
    }

    sptr_atomic->counter = sgtc_temp.counter;

#endif
}

/*!
 * @brief   atomic_inc
 * @param   sptr_atomic
 * @retval  none
 * @note    increment of atomic
 */
static inline void atomic_inc(srt_atomic_t *sptr_atomic)
{
    atomic_add(1, sptr_atomic);
}

/*!
 * @brief   atomic_dec
 * @param   sptr_atomic
 * @retval  none
 * @note    decrement of atomic
 */
static inline void atomic_dec(srt_atomic_t *sptr_atomic)
{
    atomic_sub(1, sptr_atomic);
}

/*!
 * @brief   set bit
 * @param   nr， ptr_addr
 * @retval  none
 * @note    *ptr_addr |= (1 << nr)
 */
static inline void atomic_set_bit(kint32_t nr, kuint32_t *ptr_addr)
{
    kutype_t flag;
    kuint32_t result, bitmask;

    __asm__ __volatile__ (
        " 1:	                \n\t"
        "   ldrex %0, [%4]		\n\t"
        "	mov %2, #0x01		\n\t"
        "	lsl %2, %3			\n\t"
        "	orr %0, %0, %2		\n\t"
        "	strex %1, %0, [%4]	\n\t"
        "	teq %1, #0x0		\n\t"
        "	bne 1b				\n\t"
        : "=&r"(result), "=&r"(flag)
        : "r"(bitmask), "r"(nr), "r"(ptr_addr)
        : "cc"
    );
}

/*!
 * @brief   clear bit
 * @param   nr， ptr_addr
 * @retval  none
 * @note    *ptr_addr &= ~(1 << nr)
 */
static inline void atomic_clear_bit(kint32_t nr, kuint32_t *ptr_addr)
{
    kutype_t flag;
    kuint32_t result, bitmask;

    __asm__ __volatile__ (
        " 1:	                \n\t"
        "   ldrex %0, [%4]		\n\t"
        "	mov %2, #0x01		\n\t"
        "	lsl %2, %3			\n\t"
        "	bic %0, %0, %2		\n\t"
        "	strex %1, %0, [%4]	\n\t"
        "	teq %1, #0x0		\n\t"
        "	bne 1b				\n\t"
        : "=&r"(result), "=&r"(flag)
        : "r"(bitmask), "r"(nr), "r"(ptr_addr)
        : "cc"
    );
}

/*!
 * @brief   verify bit[nr] if is set
 * @param   nr， ptr_addr
 * @retval  none
 * @note    !!(*ptr_addr & (1 << nr))
 */
static inline kbool_t atomic_is_bitset(kint32_t nr, kuint32_t *ptr_addr)
{
    kuint32_t result, bitmask;

    __asm__ __volatile__ (
        "   ldr %0, [%3]		\n\t"
        "	mov %1, #0x01		\n\t"
        "	lsl %1, %2			\n\t"
        "	and %0, %0, %1		\n\t"
        : "=&r"(result)
        : "r"(bitmask), "r"(nr), "r"(ptr_addr)
        : "cc"
    );

    return !!result;
}

/*!
 * @brief   set bit mask
 * @param   nr， ptr_addr
 * @retval  none
 * @note    *ptr_addr |= mask
 */
static inline void atomic_set_mask(kint32_t mask, kuint32_t *ptr_addr)
{
    kutype_t flag;
    kuint32_t result;

    __asm__ __volatile__ (
        " 1:	                \n\t"
        "   ldrex %0, [%3]		\n\t"
        "	orr %0, %0, %2		\n\t"
        "	strex %1, %0, [%3]	\n\t"
        "	teq %1, #0x0		\n\t"
        "	bne 1b				\n\t"
        : "=&r"(result), "=&r"(flag)
        : "r"(mask), "r"(ptr_addr)
        : "cc"
    );
}

/*!
 * @brief   clear bit mask
 * @param   nr， ptr_addr
 * @retval  none
 * @note    *ptr_addr &= ~mask
 */
static inline void atomic_clear_mask(kint32_t mask, kuint32_t *ptr_addr)
{
    kutype_t flag;
    kuint32_t result;

    __asm__ __volatile__ (
        " 1:	                \n\t"
        "   ldrex %0, [%3]		\n\t"
        "	bic %0, %0, %2		\n\t"
        "	strex %1, %0, [%3]	\n\t"
        "	teq %1, #0x0		\n\t"
        "	bne 1b				\n\t"
        : "=&r"(result), "=&r"(flag)
        : "r"(mask), "r"(ptr_addr)
        : "cc"
    );
}

/*!
 * @brief   verify mask if is set
 * @param   nr， ptr_addr
 * @retval  none
 * @note    !!(*ptr_addr & mask)
 */
static inline kbool_t atomic_is_set(kint32_t mask, kuint32_t *ptr_addr)
{
    kuint32_t result;

    __asm__ __volatile__ (
        "   ldr %0, [%2]		\n\t"
        "	and %0, %0, %1		\n\t"
        : "=&r"(result)
        : "r"(mask), "r"(ptr_addr)
        : "cc"
    );

    return !!result;
}

#ifdef __cplusplus
    }
#endif

#endif /* __ATOMIC_TYPES_H */
