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
#include <configs/configs.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/context.h>

/*!< The globals */


/*!< API function */
/*!
 * @brief   get abort information
 * @param   none
 * @retval  errno
 * @note    parse to sptr_regs
 */
__weak kint32_t abort_info_get(struct context_regs *sptr_regs)
{
    return -ER_EMPTY;
}

/*!
 * @brief   get abort lr
 * @param   none
 * @retval  errno
 * @note    parse to sptr_regs
 */
__weak void abort_src_get(struct context_regs *sptr_regs, kutype_t *_lr_src, kutype_t *_sp_src)
{
}

/*!
 * @brief   deal_assert_fail
 * @param   __assertion, __file, __line, __function
 * @retval  none
 * @note    assertion solution
 */
void deal_assert_fail(const kchar_t *__assertion, kbool_t is_down,
                const kchar_t *__file, kuint32_t __line, const kchar_t *__function)
{
    struct context_regs sgtc_regs;

    print_sync("\r\n");
    print_sync(PRINT_LEVEL_ERR"Program Aborted. Here is Error Information:\r\n");

    print_sync(PRINT_LEVEL_ERR"---> assertion: %s\r\n", __assertion);
    print_sync(PRINT_LEVEL_ERR"---> file     : %s\r\n", __file);
    print_sync(PRINT_LEVEL_ERR"---> line     : %d\r\n", __line);
    print_sync(PRINT_LEVEL_ERR"---> function : %s\r\n", __function);

    if (!abort_info_get(&sgtc_regs))
    {
        kutype_t _lr_before, _sp_before;

        print_sync("\r\n");
        print_sync(PRINT_LEVEL_ERR"abort register:\r\n");
        print_sync(PRINT_LEVEL_ERR"    r0: 0x%08x, r1: 0x%08x, r2: 0x%08x\r\n", sgtc_regs.r0, sgtc_regs.r1, sgtc_regs.r2);
        print_sync(PRINT_LEVEL_ERR"    r3: 0x%08x, r4: 0x%08x, r5: 0x%08x\r\n", sgtc_regs.r3, sgtc_regs.r4, sgtc_regs.r5);
        print_sync(PRINT_LEVEL_ERR"    r6: 0x%08x, r7: 0x%08x, r8: 0x%08x\r\n", sgtc_regs.r6, sgtc_regs.r7, sgtc_regs.r8);
        print_sync(PRINT_LEVEL_ERR"    r9: 0x%08x, sl: 0x%08x, fp: 0x%08x\r\n", sgtc_regs.r9, sgtc_regs.r10, sgtc_regs.r11);

        print_sync(PRINT_LEVEL_ERR"    ip: 0x%08x, sp: 0x%08x\r\n", sgtc_regs.r12, sgtc_regs.sp);
        print_sync(PRINT_LEVEL_ERR"    lr: 0x%08x, pc: 0x%08x\r\n", sgtc_regs.lr, sgtc_regs.pc);

        abort_src_get(&sgtc_regs, &_lr_before, &_sp_before);
        print_sync("\r\n");

        print_sync(PRINT_LEVEL_ERR"sp   before abort: 0x%08x\r\n", _sp_before);
        print_sync(PRINT_LEVEL_ERR"lr   before abort: 0x%08x\r\n", _lr_before);
        print_sync(PRINT_LEVEL_ERR"spsr before abort: 0x%08x\r\n", sgtc_regs.psr);
    }

    if (THREAD_VALID())
    {
        struct thread *sptr_thread = mr_current;

    #if ((defined(CONFIG_CONTEXT_EX) && (CONFIG_CONTEXT_EX)) || (CONFIG_CONTEXT_MORE))
        struct context_regs *sptr_regs = thread_get_context(sptr_thread->sptr_attr);
    #endif

        print_sync("\r\n");

        if (*sptr_thread->name)
            print_sync(PRINT_LEVEL_ERR"current thread name: %s\r\n", sptr_thread->name);
        print_sync(PRINT_LEVEL_ERR"current thread id: %d\r\n", sptr_thread->tid);

    #if (defined(CONFIG_CONTEXT_EX) && (CONFIG_CONTEXT_EX))
        for (kint32_t idx = 0; idx < 9; idx++)
                print_sync(PRINT_LEVEL_ERR"    prev thread's r%-2d: 0x%08x\r\n", idx, *((kuaddr_t *)(&sptr_regs->r0) + idx));
    #endif

    #if CONFIG_CONTEXT_MORE
        print_sync(PRINT_LEVEL_ERR "super register:\r\n\t"
                	"prev thread's lr:    0x%08x\r\n\t"
                    "prev thread's sp:    0x%08x\r\n\t"
                    "prev thread's pc:    0x%08x\r\n\t"
                    "prev thread's spsr:  0x%08x\r\n\t"
                    "prev thread's flag:  0x%08x\r\n\t",
                    sptr_regs->lr, sptr_regs->sp, sptr_regs->pc, sptr_regs->psr, sptr_regs->flags);
    #endif
    }

    print_sync("\r\n");
    print_sync(PRINT_LEVEL_ERR"Please check for errors in time !\r\n");

    /*!< quit program */
    while (is_down)
    {}
}

/* end of file */
