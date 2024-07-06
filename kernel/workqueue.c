/*
 * Work Queue Interface
 *
 * File Name:   workqueue.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.25
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/workqueue.h>

/*!< The defines */


/*!< The functions */


/*!< API functions */
/*!
 * @brief   add a new sprt_wq to global work_queue_head
 * @param   sprt_wq
 * @retval  none
 * @note    none
 */
__weak void schedule_work(struct workqueue *sprt_wq)
{

}

/*!< end of file */
