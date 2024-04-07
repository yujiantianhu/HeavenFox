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
typedef ksint32_t 	err_return_t;

/*!< error code */
enum __ERT_ERROR_CODE
{
	/*!< --------------------------------------------------------- */
	/*!< 0 ~ 31: use with positive */
	Ert_isNone = 0U,
	Ert_isWell,
	Ert_isRetry,
	Ert_isPermit,

	Ert_isPosMax = 31U,

	/*!< --------------------------------------------------------- */
	/*!< 32 ~ ?: use with negative */
	Ert_isAnyErr = 32U,
	Ert_isMemErr,
	Ert_isUnvalid,
	Ert_isNullPtr,
	Ert_isWildPtr,
	Ert_isArgFault,
	Ert_isArrayOver,
	Ert_isNotAlign,
	Ert_isExisted,
	Ert_isNotFound,

	/*!< hardware */
	Ert_isIOErr,
	Ert_isNotSupport,
	Ert_isNotReady,
	Ert_isNotSuccess,
	Ert_isTimeOut,

	/*!< comunication */
	Ert_isSendFail,
	Ert_isRecvFail,
	Ert_isSendCmdFail,
	Ert_isRecvCmdFail,
	Ert_isSendDataFail,
	Ert_isRecvDataFail,
	Ert_isTransErr,
	Ert_isTransRequst,
	Ert_isTransBusy,
	Ert_isCheckErr,
	Ert_isTransStopFail,
	
#define mrt_retval(code)							(((code) > Ert_isPosMax) ? (-(code)) : (code))
#define mrt_isErr(code)								((code) < 0)
#define mrt_ecode_to_kbool(code)					(((code) < 0) ? Ert_false : Ert_true)
#define mrt_kbool_to_ecode(flag, code1, code2)		((flag) ? mrt_retval(code1) : mrt_retval(code2))							
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
