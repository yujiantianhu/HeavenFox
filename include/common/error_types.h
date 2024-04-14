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
typedef ksint32_t 	kstatus_t;

/*!< error code */
enum __ERT_ERROR_CODE
{
	/*!< --------------------------------------------------------- */
	/*!< 0: well */
	NR_isNormal = 0,
	NR_isWell 	= NR_isNormal,

	/*!< --------------------------------------------------------- */
	/*!< use with negative */
	NR_isAnyErr,
	NR_isRetry,
	NR_isPermit,
	NR_isMemErr,
	NR_isUnvalid,
	NR_isNullPtr,
	NR_isWildPtr,
	NR_isArgFault,
	NR_isArrayOver,
	NR_isNotAlign,
	NR_isExisted,
	NR_isNotFound,

	/*!< hardware */
	NR_isIOErr,
	NR_isNotSupport,
	NR_isNotReady,
	NR_isNotSuccess,
	NR_isTimeOut,

	/*!< comunication */
	NR_isSendFail,
	NR_isRecvFail,
	NR_isSendCmdFail,
	NR_isRecvCmdFail,
	NR_isSendDataFail,
	NR_isRecvDataFail,
	NR_isTransErr,
	NR_isTransRequst,
	NR_isTransBusy,
	NR_isCheckErr,
	NR_isTransStopFail,					
};

/*!< The functions */
TARGET_EXT void deal_assert_fail(const kstring_t *__assertion, const kstring_t *__file,
			   					kuint32_t __line, const kstring_t *__function)
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

#ifdef __cplusplus
	}
#endif

#endif /* __ERROR_TYPES_H */
