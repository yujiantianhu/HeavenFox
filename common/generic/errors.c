/*
 * Errors and Eceptions General Function
 *
 * File Name:   errors.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.10.15
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/generic.h>
#include <common/api_string.h>
#include <common/io_stream.h>
#include <kernel/sched.h>
#include <kernel/thread.h>

/*!< API function */
/*!
 * @brief   deal_assert_fail
 * @param   __assertion, __file, __line, __function
 * @retval  none
 * @note    assertion solution
 */
void deal_assert_fail(const kchar_t *__assertion, const kchar_t *__file,
                               kuint32_t __line, const kchar_t *__function)
{
    kprintf(PRINT_LEVEL_ERR"\r\n");
    kprintf(PRINT_LEVEL_ERR"Program Aborted. Here is Error Information:\r\n");

    kprintf(PRINT_LEVEL_ERR"---> assertion: %s\r\n", __assertion);
    kprintf(PRINT_LEVEL_ERR"---> file     : %s\r\n", __file);
    kprintf(PRINT_LEVEL_ERR"---> line     : %d\r\n", __line);
    kprintf(PRINT_LEVEL_ERR"---> function : %s\r\n", __function);

    if (mr_current)
    {
        struct thread *sptr_thread = mr_current;
        struct scheduler_context_regs *sptr_regs = thread_get_context(sptr_thread->sptr_attr);

        if (*sptr_thread->name)
            kprintf(PRINT_LEVEL_ERR"current thread name: %s\r\n", sptr_thread->name);
        kprintf(PRINT_LEVEL_ERR"current thread id: %d ==== < === > \r\n\t", sptr_thread->tid);

        for (kint32_t idx = 0; idx < 9; idx++)
        {
            if (idx < 10)
                kprintf(PRINT_LEVEL_ERR"r%d:    0x%x\r\n\t", idx, *((kuaddr_t *)(&sptr_regs->r0) + idx));
            else
                kprintf(PRINT_LEVEL_ERR"r%d:   0x%x\r\n\t",  idx, *((kuaddr_t *)(&sptr_regs->r0) + idx));
        }

        kprintf(PRINT_LEVEL_ERR
                	"lr:    0x%x\r\n\t"
                    "sp:    0x%x\r\n\t"
                    "pc:    0x%x\r\n\t"
                    "spsr:  0x%x\r\n\t"
                    "flag:  0x%x\r\n\t",
                    sptr_regs->lr, sptr_regs->sp, sptr_regs->pc, sptr_regs->psr, sptr_regs->flags);
    }

    kprintf(PRINT_LEVEL_ERR"\r\n");
    kprintf(PRINT_LEVEL_ERR"Please check for errors in time !\r\n");

    /*!< quit program */
    while (true)
    {}
}

/* end of file */
