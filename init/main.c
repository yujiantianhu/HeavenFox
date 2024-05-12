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
#include <common/io_stream.h>
#include <common/time.h>
#include <boot/implicit_call.h>
#include <boot/board_init.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/fwk_fcntl.h>
#include <platform/irq/fwk_irq.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/instance.h>

/*!< API functions */
/*!
 * @brief  start_kernel
 * @param  none
 * @retval none
 * @note   kernel main
 */
void start_kernel(void)
{
    print_info("start kernel ...... \n");

    /*!< disable interrupt */
    mrt_disable_global_irq();

    /*!< initial memory pool */
    fwk_mempool_initial();

    /*!< build device-tree */
    setup_machine_fdt((void *)CONFIG_DEVICE_TREE_BASE);

    /*!< board initcall */
    if (run_machine_initcall())
        goto fail;

    /*!< initial irq */
    initIRQ();

    /*!< systick init */
	board_init_systick();

    /*!< populate device node after initializing hardware */
    fwk_of_platform_populate_init();

    /* platform initcall */
    if (run_platform_initcall())
        goto fail;

    /*!< enable interrupt */
    mrt_enable_global_irq();

    /*!< create thread */
    if (kthread_init())
        goto fail;

    rest_init();

    print_info("initial system finished, start scheduler now\n");

    /*!< start */
    real_thread_schedule();

fail:
    print_info("start kernel failed!\n");

    for (;;)
    {};
}

/* end of file */
