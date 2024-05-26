/*
 * General Error Type Defines
 *
 * File Name:   error_types.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __ERROR_TYPES_H
#define __ERROR_TYPES_H

/*!< The includes */
#include "basic_types.h"
#include <assert.h>

#ifdef __cplusplus
TARGET_EXT "C" {
#endif

/*!< The defines */
typedef kint32_t 	kstatus_t;

/*!< error code */
enum __ERT_ERROR_CODE
{
	/*!< --------------------------------------------------------- */
	/*!< 0: well */
	NR_IS_NORMAL = 0,

	/*!< --------------------------------------------------------- */
	/*!< use with negative */
	NR_IS_ERROR,
	NR_IS_RETRY,
	NR_IS_PERMIT,
	NR_IS_NOMEM,
	NR_IS_UNVALID,
	NR_IS_EMPTY,
	NR_IS_NULLPTR,
	NR_IS_WILDPTR,
	NR_IS_FAULT,
	NR_IS_MORE,
	NR_IS_NOTALIGN,
	NR_IS_EXISTED,
	NR_IS_NOTFOUND,
	NR_IS_USED,
	NR_IS_LOCKED,

	/*!< hardware */
	NR_IS_IOERR,
	NR_IS_NSUPPORT,
	NR_IS_NREADY,
	NR_IS_FAILD,
	NR_IS_TIMEOUT,
	NR_IS_NODEV,
	NR_IS_BUSY,
	NR_IS_CHECKERR,

	/*!< comunication */
	NR_IS_TRXERR,
	NR_IS_SEND_FAILD,
	NR_IS_RECV_FAILD,
	NR_IS_SCMD_FAILD,
	NR_IS_RCMD_FAILD,
	NR_IS_SDATA_FAILD,
	NR_IS_RDATA_FAILD,
	NR_IS_STOP_FAILD,					
};

/*!< The functions */
TARGET_EXT void deal_assert_fail(const kchar_t *__assertion, const kchar_t *__file,
			   					kuint32_t __line, const kchar_t *__function)
								__THROW __attribute__ ((__noreturn__));
/*!< The defines */
#define mrt_void()									((void)(0))

#ifdef	NDEBUG
#define mrt_assert(x)
#else
#define mrt_assert(x)								((x) ? mrt_void() : deal_assert_fail(#x, __FILE__, __LINE__, __ASSERT_FUNCTION))
#endif

#define mrt_jump_to(label, run_codes)	\
{	\
	run_codes;	\
	goto label;	\
}

/*!< API functions */
__force_inline static inline kbool_t IS_ERR(const void *ptr)
{
	/*!< the highest 4KB (0xfffff000 ~ 0xffffffff) is used to deal with exceptions */
	return (kuaddr_t)ptr >= (kuaddr_t)(-(4UL << 10));
}

__force_inline static inline void *ERR_PTR(kstype_t code)
{
	return (void *)code;
}

__force_inline static inline kstype_t PTR_ERR(const void *ptr)
{
	return ptr ? (kstype_t)ptr : -NR_IS_NOMEM;
}

#ifdef __cplusplus
	}
#endif

#endif /* __ERROR_TYPES_H */
