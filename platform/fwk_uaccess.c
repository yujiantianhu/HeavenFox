/*
 * User Space and Kernel Interface
 *
 * File Name:   fwk_uaccess.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.02
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_uaccess.h>

/*!< API function */
/*!
 * @brief   fwk_copy_from_user
 * @param   none
 * @retval  none
 * @note    none
 */
kusize_t fwk_copy_from_user(void *ptr_dst, const void *ptr_user, kusize_t size)
{
	memcpy(ptr_dst, ptr_user, size);

	return size;
}

/*!
 * @brief   fwk_copy_to_user
 * @param   none
 * @retval  none
 * @note    none
 */
kusize_t fwk_copy_to_user(void *ptr_user, void *ptr_Src, kusize_t size)
{
	memcpy(ptr_user, ptr_Src, size);

	return size;
}

/*!< end of file */
