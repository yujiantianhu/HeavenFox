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
	print_err("Program Aborted. Here is Error Information:\n");

	print_err("---> assertion: %s\n", __assertion);
	print_err("---> file     : %s\n", __file);
	print_err("---> line     : %d\n", __line);
	print_err("---> function : %s\n", __function);
	print_err("\n");

    if (mrt_current)
    {
		struct scheduler_context_regs *sprt_regs = real_thread_get_context(mrt_current->sprt_attr);

        print_err("current thread id: %d ==== < === > \n", mrt_current->tid);
        print_err(	"r0: 	%x\n\t" 
					"r1: 	%x\n\t"
					"r2: 	%x\n\t"
					"r3: 	%x\n\t"
					"r4: 	%x\n\t"
					"r5: 	%x\n\t"
					"r6: 	%x\n\t"
					"r7: 	%x\n\t"
					"r8: 	%x\n\t"
					"r9: 	%x\n\t"
					"r10: 	%x\n\t"
					"r11: 	%x\n\t"
					"r12: 	%x\n\t"
					"lr: 	%x\n\t"
					"sp: 	%x\n\t"
					"pc: 	%x\n\t"
					"spsr: 	%x\n\t"
					"flag: 	%x\n\t",
					sprt_regs->r0, sprt_regs->r1, sprt_regs->r2, sprt_regs->r3, sprt_regs->r4,  sprt_regs->r5,
					sprt_regs->r6, sprt_regs->r7, sprt_regs->r8, sprt_regs->r9, sprt_regs->r10, sprt_regs->r11,
					sprt_regs->r12, sprt_regs->lr, sprt_regs->sp, sprt_regs->pc, sprt_regs->psr, sprt_regs->flags );
    }

	print_err("Please check for errors in time !\n");

	/*!< quit program */
	while (true)
	{}
}


/* end of file */
