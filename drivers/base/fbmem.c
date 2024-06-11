/*
 * Gerneral Interface : fbmem
 *
 * File Name:   fbmem.c
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
#include <platform/video/fwk_fbmem.h>

/*!< The defines */


/*!< API function */
/*!
 * @brief   fbmem_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init fbmem_init(void)
{
	return fwk_fbmem_init();
}

/*!
 * @brief   fbmem_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit fbmem_exit(void)
{
	fwk_fbmem_exit();
}

IMPORT_PATTERN_INIT(fbmem_init);
IMPORT_PATTERN_EXIT(fbmem_exit);

/*!< end of file */
