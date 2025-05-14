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
    print_err("\r\n");
    print_err("Program Aborted. Here is Error Information:\r\n");

    print_err("---> assertion: %s\r\n", __assertion);
    print_err("---> file     : %s\r\n", __file);
    print_err("---> line     : %d\r\n", __line);
    print_err("---> function : %s\r\n", __function);

    if (mr_current)
    {
        struct thread *sptr_thread = mr_current;
        struct scheduler_context_regs *sptr_regs = thread_get_context(sptr_thread->sptr_attr);

        if (*sptr_thread->name)
            print_err("current thread name: %s\r\n", sptr_thread->name);
        print_err("current thread id: %d ==== < === > \r\n\t", sptr_thread->tid);

        for (kint32_t idx = 0; idx < 9; idx++)
        {
            if (idx < 10)
                print_err("r%d:    0x%x\r\n\t", idx, *((kuaddr_t *)(&sptr_regs->r0) + idx));
            else
                print_err("r%d:   0x%x\r\n\t",  idx, *((kuaddr_t *)(&sptr_regs->r0) + idx));
        }

        print_err(	"lr:    0x%x\r\n\t"
                    "sp:    0x%x\r\n\t"
                    "pc:    0x%x\r\n\t"
                    "spsr:  0x%x\r\n\t"
                    "flag:  0x%x\r\n\t",
                    sptr_regs->lr, sptr_regs->sp, sptr_regs->pc, sptr_regs->psr, sptr_regs->flags);
    }

    print_err("\r\n");
    print_err("Please check for errors in time !\r\n");

    /*!< quit program */
    while (true)
    {}
}

/* end of file */
