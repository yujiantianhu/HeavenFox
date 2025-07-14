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
#include <common/mem_manage.h>
#include <kernel/context.h>

/*!< The globals*/
kuaddr_t prefecth_abort_addr;
kuaddr_t data_abort_addr;
kuaddr_t undefined_abort_addr;

kuint32_t g_interrupt_flags = 0;
kutype_t g_abort_cur_sp = 0;

/*!< API function */
/*!
 * @brief   exec_undefined_handler
 * @param   none
 * @retval  none
 * @note    undefined exception
 */
void exec_undefined_handler(void)
{
    SET_EXCEPTION_FLAG(UND_ABORT_BIT);
    
    printk(PRINT_LEVEL_ERR "%s: lr \'0x%x\' cause fault\r\n", __FUNCTION__, undefined_abort_addr);
    mr_assert(false);

    CLR_EXCEPTION_FLAG(UND_ABORT_BIT);
}

/*!
 * @brief   exec_prefetch_abort_handler
 * @param   none
 * @retval  none
 * @note    prefetch abort exception
 */
void exec_prefetch_abort_handler(void)
{
    SET_EXCEPTION_FLAG(PREFETCH_ABORT_BIT);

    printk(PRINT_LEVEL_ERR "%s: lr \'0x%x\' cause fault\r\n", __FUNCTION__, prefecth_abort_addr);
    mr_assert(false);
    
    CLR_EXCEPTION_FLAG(PREFETCH_ABORT_BIT);
}

/*!
 * @brief   exec_data_abort_handler
 * @param   none
 * @retval  none
 * @note    data abort exception
 */
void exec_data_abort_handler(void)
{
    SET_EXCEPTION_FLAG(DATA_ABORT_BIT);

    printk(PRINT_LEVEL_ERR "%s: lr \'0x%x\' cause fault\r\n", __FUNCTION__, data_abort_addr);
    mr_assert(false);
    
    CLR_EXCEPTION_FLAG(DATA_ABORT_BIT);
}

/*!
 * @brief   exec_unused_handler
 * @param   none
 * @retval  none
 * @note    unused exception
 */
void exec_unused_handler(void)
{
    SET_EXCEPTION_FLAG(UNUSED_BIT);
    mr_assert(false);
    CLR_EXCEPTION_FLAG(UNUSED_BIT);
}

/*!< --------------------------------------------------------------- */
/*!
 * @brief   get abort information
 * @param   none
 * @retval  errno
 * @note    parse to sptr_regs
 */
kint32_t abort_info_get(struct context_regs *sptr_regs)
{
    struct context_regs *sptr_sp = (struct context_regs *)g_abort_cur_sp;

    if (!sptr_sp)
        return -ER_EMPTY;

    memcpy(sptr_regs, sptr_sp, sizeof(*sptr_sp));
    return ER_NORMAL;
}

/*!
 * @brief   get abort lr and sp
 * @param   none
 * @retval  errno
 * @note    parse to sptr_regs
 */
void abort_src_get(struct context_regs *sptr_regs, kutype_t *_lr_src, kutype_t *_sp_src)
{
    static kutype_t _lr = 0, _sp = 0, _cpsr = 0;

    _cpsr = __get_cpsr();
    mr_arch_mode_switch(sptr_regs->psr & 0x1f);

    __asm__ __volatile__ (
        " mov %0, lr    \n\t"
        " mov %1, sp    \n\t"
        : "=r"(_lr), "=r"(_sp)
        : 
        : "cc", "memory"
    );

    mr_arch_mode_switch(_cpsr & 0x1f);

    *_lr_src = _lr;
    *_sp_src = _sp;
}

/* end of file*/
