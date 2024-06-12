/*
 * Gerneral Interface : CharDev
 *
 * File Name:   chardev.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.06.10
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_inode.h>
#include <platform/fwk_fs.h>
#include <platform/fwk_chrdev.h>

/*!< The defines */


/*!< API function */
/*!
 * @brief   chardev_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init chardev_init(void)
{
	return fwk_chrdev_init();
}

/*!
 * @brief   chardev_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit chardev_exit(void)
{
	fwk_chrdev_exit();
}

IMPORT_PLATFORM_INIT(chardev_init);
IMPORT_PLATFORM_EXIT(chardev_exit);

/*!< end of file */
