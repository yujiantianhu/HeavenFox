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

/*!< The includes */
#include "basic_types.h"

/*!< The defines */
struct atomic
{
    kuint32_t counter;

};
typedef struct atomic srt_atomic_t;

/*!< The functions */
/*!
 * @brief   atomic_add
 * @param   i, sprt_atomic
 * @retval  none
 * @note    add of atomic
 */
static inline void atomic_add(ksint32_t i, srt_atomic_t *sprt_atomic)
{
	kuint64_t flag;
	ksint32_t result;

	__asm__ __volatile__ (
		" 1:	                \n\t"
        "   ldrex %0, [%3]		\n\t"
		"	add %0, %0, %2		\n\t"
		"	strex %1, %0, [%3]	\n\t"
		"	teq %1, #0x0		\n\t"
		"	bne 1b				\n\t"
		: "=&r"(result), "=&r"(flag)
		: "r"(i), "r"(&sprt_atomic->counter)
		: "cc"
	);
}

/*!
 * @brief   atomic_sub
 * @param   i, sprt_atomic
 * @retval  none
 * @note    subtract of atomic
 */
static inline void atomic_sub(ksint32_t i, srt_atomic_t *sprt_atomic)
{
	kuint64_t flag;
	ksint32_t result;

	__asm__ __volatile__ (
		" 1:	                \n\t"
        "   ldrex %0, [%3]		\n\t"
		"	sub %0, %0, %2		\n\t"
		"	strex %1, %0, [%3]	\n\t"
		"	teq %1, #0x0		\n\t"
		"	bne 1b				\n\t"
		: "=&r"(result), "=&r"(flag)
		: "r"(i), "r"(&sprt_atomic->counter)
		: "cc"
	);
}

/*!
 * @brief   atomic_mul
 * @param   i, sprt_atomic
 * @retval  none
 * @note    multiply of atomic
 */
static inline void atomic_mul(ksint32_t i, srt_atomic_t *sprt_atomic)
{
	kuint64_t flag;
	ksint32_t result;

	__asm__ __volatile__ (
		" 1:		            \n\t"
        "   ldrex %0, [%3]		\n\t"
		"	mul %0, %0, %2		\n\t"
		"	strex %1, %0, [%3]	\n\t"
		"	teq %1, #0x0		\n\t"
		"	bne 1b				\n\t"
		: "=&r"(result), "=&r"(flag)
		: "r"(i), "r"(&sprt_atomic->counter)
		: "cc"
	);
}

/*!
 * @brief   atomic_sdiv
 * @param   i, sprt_atomic
 * @retval  none
 * @note    signed divide of atomic
 */
static inline void atomic_sdiv(ksint32_t i, srt_atomic_t *sprt_atomic)
{
	kuint64_t flag;
	ksint32_t result;

	__asm__ __volatile__ (
		" 1:	                \n\t"
        "   ldrex %0, [%3]		\n\t"
		"	sdiv %0, %0, %2		\n\t"
		"	strex %1, %0, [%3]	\n\t"
		"	teq %1, #0x0		\n\t"
		"	bne 1b				\n\t"
		: "=&r"(result), "=&r"(flag)
		: "r"(i), "r"(&sprt_atomic->counter)
		: "cc"
	);
}

/*!
 * @brief   atomic_inc
 * @param   sprt_atomic
 * @retval  none
 * @note    increment of atomic
 */
static inline void atomic_inc(srt_atomic_t *sprt_atomic)
{
    atomic_add(1, sprt_atomic);
}

/*!
 * @brief   atomic_dec
 * @param   sprt_atomic
 * @retval  none
 * @note    decrement of atomic
 */
static inline void atomic_dec(srt_atomic_t *sprt_atomic)
{
    atomic_sub(1, sprt_atomic);
}

/*!
 * @brief   atomic_set_bit
 * @param   sprt_atomic
 * @retval  none
 * @note    set bit
 */
static inline void atomic_set_bit(ksint32_t nr, void *ptr_addr)
{
	ksint64_t flag;
	ksint32_t result, bitmask;

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
 * @brief   atomic_clear_bit
 * @param   sprt_atomic
 * @retval  none
 * @note    clear bit
 */
static inline void atomic_clear_bit(ksint32_t nr, void *ptr_addr)
{
	ksint64_t flag;
	ksint32_t result, bitmask;

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

#endif /* __ATOMIC_TYPES_H */
