/*
 * Main Initialization
 *
 * File Name:   main.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <configs/configs.h>
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <common/atomic_types.h>
#include <common/io_stream.h>
#include <common/time.h>
#include <arch/setup.h>
#include <boot/implicit_call.h>
#include <boot/board_init.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/base/fwk_fcntl.h>
#include <platform/irq/fwk_irq.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/instance.h>
#include <fs/fs_intr.h>
#include <fs/fs_fatfs.h>

/*!< The globals */
static struct tag_params *sptr_tag_params;
static struct thread sgtc_percpu_default_thread[CONFIG_CORE_NUM] __section(".data") = {};

/*!< The defines */
#define mr_tag_params_get()    \
({    \
    struct tag_params *sptr_param;  \
    __asm__ __volatile__ (  \
        " str r2, [%0]  \n\t"   \
        : \
        : "r"(&sptr_param)   \
        : "cc","memory" \
    );  \
    sptr_param; \
})

/*!< API functions */
/*!
 * @brief  populate params
 * @param  sptr_params
 * @retval none
 * @note   none
 */
void setup_tag_params(struct tag_params *sptr_params)
{
    while (sptr_params->sgtc_hdr.type != (-1))
    {
        switch (sptr_params->sgtc_hdr.type)
        {
            case TAG_PARAM_VIDEO:
                sptr_fwk_video_params = &sptr_params->u.sgtc_vdp;
                break;
            case TAG_PARAM_FDT:
                sptr_fwk_fdt_params = &sptr_params->u.sgtc_fdt;
                break;

            default: break;
        }

        sptr_params = TAG_PARAM_NEXT(sptr_params);
    }
}

/*!
 * @brief  setup machine
 * @param  none
 * @retval none
 * @note   cpu param populate
 */
void setup_machine(struct tag_params *sptr_params)
{
    setup_tag_params(sptr_params);

    /*!< build device-tree */
    setup_machine_fdt(sptr_fwk_fdt_params);
}

/*!
 * @brief  setup thread
 * @param  none
 * @retval none
 * @note   none
 */
void setup_thread(void)
{
    kuint32_t cpuid = get_cpu_id();
    struct thread *sptr_thread = &sgtc_percpu_default_thread[cpuid];

    __setup_thread(sptr_thread);

    sptr_thread->tid = -1;
    sptr_thread->cpu = cpuid;
    __SYNC_THREAD_STATE(sptr_thread, NR_THREAD_RUNNING);
    sprintk(sptr_thread->name, "cpu%u_virtual_thread", cpuid);

    SET_PERCPU_CURRENT((kutype_t)sptr_thread | 0x01);
    g_kernel_preempt_enable = true;
}

/*!
 * @brief  start_kernel
 * @param  none
 * @retval none
 * @note   kernel main
 */
void start_kernel(void)
{
    sptr_tag_params = mr_tag_params_get();

    /*!< close irq */
    local_irq_disable();

    /*!< initial memory pool */
    fwk_mempool_initial();

    /*!< initial I/O interface */
    iostream_init();

    /*!< initial arch */
    setup_arch();

    /*!< populate params from bootloader */
    setup_machine(sptr_tag_params);

    /*!< board initcall */
    if (run_machine_initcall())
        goto fail;

    /*!< initial irq */
    initIRQ();

    /*!< systick init */
    systime_init();

    /*!< file system */
    if (filesystem_initcall())
        goto fail;

    /*!< populate device node after initializing hardware */
    if (fwk_of_platform_populate_init())
        goto fail;

    /*!< softirq init */
    fwk_softirq_init();

    /*!< enable interrupt */
    local_irq_enable();

#if CONFIG_SCHDULE
    scheduler_init();

    /*!< kernel init */
    if (system_kernel_initcall())
        goto fail;

// #ifdef CONFIG_TEST
//     debug_init();                           /*!< create debug test task */
// #else

    /*!< create thread */
    if (kthread_init())
        goto fail;
// #endif

    rest_init();
    print_info("initial system finished, start scheduler now\r\n");

    /*!< start */
    schedule_thread();

#endif

    for (;;) {
        /*!< do nothing */
    }

fail:
    print_info("start kernel failed!\r\n");
    mr_assert(false);
}

/*!
 * @brief  secondary_start_kernel
 * @param  none
 * @retval none
 * @note   kernel main for another cpu
 */
void secondary_start_kernel(void)
{
    kuint32_t cpuid = get_cpu_id();

    /*!< close irq */
    local_irq_disable();

    /*!< initial irq */
    initIRQ();

    /*!< systick init */
    arch_systick_init();

    /*!< enable interrupt */
    local_irq_enable();

#if CONFIG_SCHDULE
    /*!< create thread */
    if (kthread_init())
        goto fail;

    rest_init();
    print_info("initial system finished, secondary (cpuid: %u) start scheduler now\r\n", cpuid);

    /*!< start */
    schedule_thread();

#endif

    for (;;) {
        /*!< do nothing */
    }

fail:
    print_info("secondary (cpuid: %u) start kernel failed!\r\n", cpuid);
    mr_assert(false);
}

/* end of file */
