/*
 * ARM V7 Exception API Function
 *
 * File Name:   exception.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __EXCEPTION_H
#define __EXCEPTION_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <common/atomic_types.h>
#include <boot/core.h>

/*!< The defines */
#define IRQ_NESTED_MAX                              (9)

struct exception_info
{
    kuaddr_t prefecth_abort_addr;                   /*!< offset: 0x00 */
    kuaddr_t data_abort_addr;                       /*!< offset: 0x04 */
    kuaddr_t undefined_abort_addr;                  /*!< offset: 0x08 */
    kuaddr_t irq_addr[IRQ_NESTED_MAX];              /*!< offset: 0x0c */

    kuaddr_t abort_cur_sp;                          /*!< offset: 0x0c + 0x4 * IRQ_NESTED_MAX */
    kuaddr_t irq_cur_sp[IRQ_NESTED_MAX];            /*!< offset: ... */

#define EXCEP_PREFECTH_ABT_OFFSET                   (0x00)
#define EXCEP_DATA_ABT_OFFSET                       (EXCEP_PREFECTH_ABT_OFFSET + sizeof(kuaddr_t))
#define EXCEP_UND_ABT_OFFSET                        (EXCEP_DATA_ABT_OFFSET + sizeof(kuaddr_t))
#define EXCEP_IRQ_OFFSET                            (EXCEP_UND_ABT_OFFSET + sizeof(kuaddr_t))
#define EXCEP_ABT_CUR_SP_OFFSET                     (EXCEP_IRQ_OFFSET + (sizeof(kuaddr_t) * IRQ_NESTED_MAX))
#define EXCEP_IRQ_CUR_SP_OFFSET                     (EXCEP_ABT_CUR_SP_OFFSET + sizeof(kuaddr_t))
};

/*!< The globals */
// extern kuint32_t g_interrupt_flags[];
extern struct exception_info sgtc_excep_info[];

/*!< The defines */
#define IRQ_BIT                                     (0x02)
#define FIQ_BIT                                     (0x04)
#define SWI_BIT                                     (0x08)
#define UND_ABORT_BIT                               (0x10)
#define PREFETCH_ABORT_BIT                          (0x20)
#define DATA_ABORT_BIT                              (0x40)
#define UNUSED_BIT                                  (0x80)

/*!< The functions */
extern void exec_undefined_handler(kutype_t _sp, kutype_t _lr);
extern void exec_prefetch_abort_handler(kutype_t _sp, kutype_t _lr);
extern void exec_data_abort_handler(kutype_t _sp, kutype_t _lr);
extern void exec_unused_handler(kutype_t _sp, kutype_t _lr);

#ifdef __cplusplus
    }
#endif

#endif /* __EXCEPTION_H */
