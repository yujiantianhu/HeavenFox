/*
 * Kernel Scheduler Defines
 *
 * File Name:   sched.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.09
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __KEL_SCHED_H_
#define __KEL_SCHED_H_

/*!< The includes */
#include <kernel/kernel.h>
#include <kernel/context.h>
#include <kernel/thread.h>

/*!< The defines */
#define KEL_THREAD_NAME_SIZE                    (32)

struct kel_thread
{
	/*!< thread name */
	kstring_t name[KEL_THREAD_NAME_SIZE];

	/*!< thread id */
	kuint32_t tid;

	/*!< refer to "__ERT_KEL_BASIC_STATUS" */
	kuint32_t status;
	kuint32_t to_status;

	/*!< thread entry */
	void *(*start_routine) (void *);
	struct kel_thread_attr *sprt_attr;
	void *ptr_args;

	/*!< thread list (to ready/suspend/sleep list) */
	struct list_head sgrt_link;

	/*!< thread time slice (period = sprt_attr->sgrt_param.mrt_sched_init_budget) */
    kutime_t expires;
};

/*!< thread manage table */
struct kel_thread_table
{
	ksint32_t max_tidarr;											/*!< = KEL_THREAD_MAX_NUM */
	ksint32_t max_tids; 											/*!< = KEL_THREAD_MAX_NUM + count of sprt_tids */
	ksint32_t max_tidset;											/*!< the max tid */
	ksint32_t ref_tidarr; 											/*!< number of allocated descriptors in sprt_tid_array */

	struct list_head sgrt_ready;									/*!< ready list head (manage all ready thread) */
	struct list_head sgrt_suspend;									/*!< suspend list head (manage all suspend thread) */
	struct list_head sgrt_sleep;									/*!< sleep list head (manage all sleepy thread) */

	struct kel_thread *sprt_work;									/*!< current thread (status is running) */

	struct kel_thread **sprt_tids;									/*!< if sprt_tid_array is up to max, new thread form mempool */
	struct kel_thread *sprt_tid_array[KEL_THREAD_MAX_NUM];			/*!< thread maximum, tid = 0 ~ KEL_THREAD_MAX_NUM */

#define __KEL_THREAD_HANDLER(ptr, tid)			((ptr)->sprt_tid_array[(tid)])
#define __KEL_THREAD_RUNNING_LIST(ptr)			((ptr)->sprt_work)
#define __KEL_THREAD_READY_LIST(ptr)			(&((ptr)->sgrt_ready))
#define __KEL_THREAD_SUSPEND_LIST(ptr)			(&((ptr)->sgrt_suspend))
#define __KEL_THREAD_SLEEP_LIST(ptr)			(&((ptr)->sgrt_sleep))
};

/*!< The functions */
TARGET_EXT struct kel_thread *get_current_thread(void);
TARGET_EXT struct kel_thread *get_thread_handle(real_thread_t tid);
TARGET_EXT void real_thread_set_name(const kstring_t *name);
TARGET_EXT real_thread_t kel_sched_get_unused_tid(kuint32_t i_start, kuint32_t count);
TARGET_EXT void kel_sched_self_suspend(void);
TARGET_EXT ksint32_t kel_sched_thread_suspend(real_thread_t tid);
TARGET_EXT ksint32_t kel_sched_thread_wakeup(real_thread_t tid);

TARGET_EXT kbool_t is_ready_thread_empty(void);
TARGET_EXT kbool_t is_suspend_thread_empty(void);
TARGET_EXT kbool_t is_sleep_thread_empty(void);
TARGET_EXT struct kel_thread *get_first_ready_thread(void);
TARGET_EXT struct kel_thread *get_first_suspend_thread(void);
TARGET_EXT struct kel_thread *get_first_sleep_thread(void);

TARGET_EXT ksint32_t kel_sched_switch_thread(real_thread_t tid);
TARGET_EXT ksint32_t register_kel_sched_thread(struct kel_thread *sprt_thread, real_thread_t tid);
TARGET_EXT void __real_thread_init_before(void);
TARGET_EXT struct kel_context_info *__real_thread_schedule(void);
TARGET_EXT void real_thread_schedule(void);

/*!< The defines */
#define mrt_current                             get_current_thread()
#define mrt_tid_handle(tid)                     get_thread_handle(tid)

#endif /* __KEL_SCHED_H_ */
