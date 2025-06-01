/*
 * ARM V7 Exception API Function
 *
 * File Name:   exception.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <asm/exception.h>
#include <common/error_types.h>
#include <common/io_stream.h>

/*!< The globals*/
kuaddr_t prefecth_abort_addr;
kuaddr_t data_abort_addr;

/*!< API function */
/*!
 * @brief   exec_undefined_handler
 * @param   none
 * @retval  none
 * @note    undefined exception
 */
void exec_undefined_handler(void)
{
    g_interrupt_flags |= (EXCEPTION_BIT | 0x04);

    printk(PRINT_LEVEL_ERR "%s: program instruction undefine\r\n", __FUNCTION__);
//  mr_assert(false);

    g_interrupt_flags &= ~(EXCEPTION_BIT | 0x04);
}

/*!
 * @brief   exec_prefetch_abort_handler
 * @param   none
 * @retval  none
 * @note    prefetch abort exception
 */
void exec_prefetch_abort_handler(void)
{
    g_interrupt_flags |= (EXCEPTION_BIT | 0x0C);

    printk(PRINT_LEVEL_ERR "%s: lr \'0x%x\' cause fault\r\n", __FUNCTION__, prefecth_abort_addr);
//  mr_assert(false);
    
    g_interrupt_flags &= ~(EXCEPTION_BIT | 0x0C);
}

/*!
 * @brief   exec_data_abort_handler
 * @param   none
 * @retval  none
 * @note    data abort exception
 */
void exec_data_abort_handler(void)
{
    g_interrupt_flags |= (EXCEPTION_BIT | 0x10);

    printk(PRINT_LEVEL_ERR "%s: lr \'0x%x\' cause fault\r\n", __FUNCTION__, data_abort_addr);
//  mr_assert(false);
    
    g_interrupt_flags &= ~(EXCEPTION_BIT | 0x10);
}

/*!
 * @brief   exec_unused_handler
 * @param   none
 * @retval  none
 * @note    unused exception
 */
void exec_unused_handler(void)
{
    g_interrupt_flags |= (EXCEPTION_BIT | 0x14);
    mr_assert(false);
    g_interrupt_flags &= ~(EXCEPTION_BIT | 0x14);
}

/* end of file*/
