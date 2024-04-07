/*
 * User Space and Kernel Interface
 *
 * File Name:   hal_uaccess.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.02
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_UACCESS_H_
#define __HAL_UACCESS_H_

/*!< The includes */
#include <platform/hal_basic.h>

/*!< The defines */
struct hal_vm_area
{
    kuaddr_t virt_addr;
    kusize_t size;
};

/*!< The functions */
TARGET_EXT kusize_t hal_copy_from_user(void *ptr_dst, const void *ptr_user, kusize_t size);
TARGET_EXT kusize_t hal_copy_to_user(void *ptr_user, void *ptr_Src, kusize_t size);

#endif /*!< __HAL_UACCESS_H_ */
