/*
 * ARM V7 Compiler Linker API Function
 *
 * File Name:   compiler.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.01.07
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/basic_types.h>
#include <common/error_types.h>

/*!< The globals */

/*!< The functions */
// extern void __libc_init_array(void);

/*!< API functions */
/*!
 * @brief   __aeabi_unwind_cpp_pr0
 * @param   none
 * @retval  none
 * @note    dummy functions to avoid linker complaints
 */
__weak void __aeabi_unwind_cpp_pr0(void)
{
}

/*!
 * @brief   __aeabi_unwind_cpp_pr1
 * @param   none
 * @retval  none
 * @note    dummy functions to avoid linker complaints
 */
__weak void __aeabi_unwind_cpp_pr1(void)
{
}

/*!
 * @brief   __aeabi_unwind_cpp_pr2
 * @param   none
 * @retval  none
 * @note    dummy functions to avoid linker complaints
 */
__weak void __aeabi_unwind_cpp_pr2(void)
{
}

#if CONFIG_NOSTDLIB
void *__dso_handle = (void *)&__dso_handle;

/*!
 * @brief	init (call global constructor)
 * @param  	none
 * @retval 	none
 * @note   	none
 */
__weak void _init(void)
{
}

/*!
 * @brief	abort
 * @param  	none
 * @retval 	none
 * @note   	none
 */
__weak void abort(void)
{
    mr_assert(false);
    for (;;);
}

/*!
 * @brief	exit
 * @param  	none
 * @retval 	none
 * @note   	none
 */
__weak void _exit(int status)
{
    (void)status;
    abort();
}

#endif