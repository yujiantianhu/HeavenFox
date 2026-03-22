/*
 * Kernel Defines For ARMv7
 *
 * File Name:   kernel.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.06.11
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <kernel/kernel.h>
#include <kernel/thread.h>
#include <kernel/sched.h>

/*!< The defines */


/*!< The functions */

/*!< The globals */
/*!< it must be initialized to 0 !!! do no rely on .bss */
DEFINE_PER_CPU(kuint32_t, g_percpu_slave_wake) __section(".data") = {};

/*!< API functions */
/*!
 * @brief   wake up another cpu (smp)
 * @param   none
 * @retval  none
 * @note    none
 */
void smp_slave_init(void)
{
    foreach_percpu(kuint32_t, cpuid)
        *SPEC_CPU_READ(g_percpu_slave_wake, cpuid) = true;

    mr_dsb();
    mr_sev();
    mr_dsb();
}

/*!< end of file */
