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
    while (Ert_true);
}

/*!
 * @brief   exec_prefetch_abort_handler
 * @param   none
 * @retval  none
 * @note    prefetch abort exception
 */
void exec_prefetch_abort_handler(void)
{
    while (Ert_true);
}

/*!
 * @brief   exec_data_abort_handler
 * @param   none
 * @retval  none
 * @note    data abort exception
 */
void exec_data_abort_handler(void)
{
    while (Ert_true);
}

/*!
 * @brief   exec_unused_handler
 * @param   none
 * @retval  none
 * @note    unused exception
 */
void exec_unused_handler(void)
{
    while (Ert_true);
}

/* end of file*/
