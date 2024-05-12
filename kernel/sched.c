/*
 * Kernel Schedualer Interface
 *
 * File Name:   sched.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.09
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <kernel/kernel.h>
#include <kernel/sched.h>

/*!< The globals */
kuint32_t g_sched_preempt_cnt = 0;

/*!< TCB */
static struct kel_thread_table sgrt_kel_thread_Tabs =
{
	.max_tidarr		= 0,
	.max_tids		= KEL_THREAD_MAX_NUM,
	.max_tidset		= 0,
	.ref_tidarr		= 0,

	.sgrt_ready		= LIST_HEAD_INIT(&sgrt_kel_thread_Tabs.sgrt_ready),
	.sgrt_suspend	= LIST_HEAD_INIT(&sgrt_kel_thread_Tabs.sgrt_suspend),
	.sgrt_sleep		= LIST_HEAD_INIT(&sgrt_kel_thread_Tabs.sgrt_ready),

	.sprt_work		= mrt_nullptr,
	.sprt_tids		= mrt_nullptr,
	.sprt_tid_array	= { mrt_nullptr }
};

/*!< The defines */
#define KEL_TABS_THREAD_HANDLER(tid)				__KEL_THREAD_HANDLER(&sgrt_kel_thread_Tabs, tid)
#define KEL_TABS_RUNNING_THREAD						__KEL_THREAD_RUNNING_LIST(&sgrt_kel_thread_Tabs)
#define KEL_TABS_READY_LIST							__KEL_THREAD_READY_LIST(&sgrt_kel_thread_Tabs)
#define KEL_TABS_SUSPEND_LIST						__KEL_THREAD_SUSPEND_LIST(&sgrt_kel_thread_Tabs)
#define KEL_TABS_SLEEP_LIST							__KEL_THREAD_SLEEP_LIST(&sgrt_kel_thread_Tabs)

/*!< set thread status */
#define KEL_SET_THREAD_STATUS(tid, value)	\
	do {	\
		if ((value) < NR_THREAD_STATUS_MAX)	\
		{	\
			struct kel_thread *sprt_task = KEL_TABS_THREAD_HANDLER(tid);	\
			sprt_task->to_status = (value);	\
		}	\
	} while (0)

#define KEL_SYNC_THREAD_STATUS(tid, value)	\
	do {	\
		if ((value) < NR_THREAD_STATUS_MAX)	\
		{	\
			struct kel_thread *sprt_task = KEL_TABS_THREAD_HANDLER(tid);	\
			sprt_task->status = (value);	\
            sprt_task->to_status = NR_THREAD_NONE;    \
		}	\
	} while (0)

/* get thread status */
#define KEL_GET_THREAD_STATUS(tid)	\
({	\
	struct kel_thread *sprt_task = KEL_TABS_THREAD_HANDLER(tid);	\
	sprt_task->status;	\
})

/*!< The functions */
static ksint32_t __kel_sched_find_thread(real_thread_t tid, struct list_head *sprt_head);
static ksint32_t __kel_sched_add_status_list(struct kel_thread *sprt_thread, struct list_head *sprt_head);
static void __kel_sched_del_status_list(struct kel_thread *sprt_thread, struct list_head *sprt_head);
static ksint32_t kel_sched_despoil_work_role(real_thread_t tid);
static ksint32_t kel_sched_reinstall_work_role(real_thread_t tid);
static ksint32_t kel_sched_add_ready_list(real_thread_t tid);
static ksint32_t kel_sched_detach_ready_list(real_thread_t tid);
static ksint32_t kel_sched_add_suspend_list(real_thread_t tid);
static ksint32_t kel_sched_detach_suspend_list(real_thread_t tid);
static ksint32_t kel_sched_add_sleep_list(real_thread_t tid);
static ksint32_t kel_sched_detach_sleep_list(real_thread_t tid);

/* -------------------------------------------------------------------------- */
/*!< API functions */
/*!
 * @brief	get current thread from tcb
 * @param  	tid
 * @retval 	running thread
 * @note   	none
 */
struct kel_thread *get_current_thread(void)
{
    return KEL_TABS_RUNNING_THREAD;
}

/*!
 * @brief	get thread from tcb
 * @param  	tid
 * @retval 	thread
 * @note   	none
 */
struct kel_thread *get_thread_handle(real_thread_t tid)
{
    return KEL_TABS_THREAD_HANDLER(tid);
}

/*!
 * @brief	set name to current thread
 * @param  	name: thread name
 * @retval 	none
 * @note   	none
 */
void real_thread_set_name(const kstring_t *name)
{
	struct kel_thread *sprt_work;

	sprt_work = KEL_TABS_RUNNING_THREAD;

	memset(sprt_work->name, 0, KEL_THREAD_NAME_SIZE);
	strncpy(sprt_work->name, name, KEL_THREAD_NAME_SIZE);
}

/*!
 * @brief	find a free tid
 * @param  	i_start: base
 * @param	count: limit
 * @retval 	none
 * @note   	none
 */
real_thread_t kel_sched_get_unused_tid(kuint32_t i_start, kuint32_t count)
{
	kuint32_t i;

	for (i = i_start; i < (i_start + count); i++)
	{
		if (!KEL_TABS_THREAD_HANDLER(i))
			return i;
	}

	return -NR_IS_MORE;
}

/*!
 * @brief	suspend current thread
 * @param  	none
 * @retval 	0: fail; 1: succuess
 * @note   	suspend current thread, and switch to next
 */
void kel_sched_self_suspend(void)
{
	KEL_SET_THREAD_STATUS(KEL_TABS_RUNNING_THREAD->tid, NR_THREAD_SUSPEND);
    real_thread_schedule();
}

/*!
 * @brief	suspend another thread
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	none
 */
ksint32_t kel_sched_thread_suspend(real_thread_t tid)
{
    if (tid == KEL_TABS_RUNNING_THREAD->tid)
    {
        kel_sched_self_suspend();
        return NR_IS_NORMAL;
    }

    KEL_SET_THREAD_STATUS(tid, NR_THREAD_SUSPEND);
	return kel_sched_switch_thread(tid);
}

/*!
 * @brief	wake up another thread
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	if target thread is suspending, wake it up and add to ready list
 */
ksint32_t kel_sched_thread_wakeup(real_thread_t tid)
{
	if (NR_THREAD_SUSPEND != KEL_GET_THREAD_STATUS(tid))
		return -NR_IS_UNVALID;

    KEL_SET_THREAD_STATUS(tid, NR_THREAD_READY);
	return kel_sched_switch_thread(tid);
}

/*!
 * @brief	check if ready list is empty
 * @param  	none
 * @retval 	1: empty; 0: not empty
 * @note   	none
 */
kbool_t is_ready_thread_empty(void)
{
    return !!mrt_list_head_empty(KEL_TABS_READY_LIST);
}

/*!
 * @brief	check if suspend list is empty
 * @param  	none
 * @retval 	1: empty; 0: not empty
 * @note   	none
 */
kbool_t is_suspend_thread_empty(void)
{
    return !!mrt_list_head_empty(KEL_TABS_SUSPEND_LIST);
}

/*!
 * @brief	check if sleep list is empty
 * @param  	none
 * @retval 	1: empty; 0: not empty
 * @note   	none
 */
kbool_t is_sleep_thread_empty(void)
{
    return !!mrt_list_head_empty(KEL_TABS_SLEEP_LIST);
}

/*!
 * @brief	get the highest ready thread (if ready list is not empty)
 * @param  	none
 * @retval 	first thread
 * @note   	none
 */
struct kel_thread *get_first_ready_thread(void)
{
    kbool_t existed = mrt_list_head_empty(KEL_TABS_READY_LIST);
    return existed ? mrt_nullptr : mrt_list_first_entry(KEL_TABS_READY_LIST, struct kel_thread, sgrt_link);
}

/*!
 * @brief	get the highest suspend thread (if suspend list is not empty)
 * @param  	none
 * @retval 	first thread
 * @note   	none
 */
struct kel_thread *get_first_suspend_thread(void)
{
    kbool_t existed = mrt_list_head_empty(KEL_TABS_SUSPEND_LIST);
    return existed ? mrt_nullptr : mrt_list_first_entry(KEL_TABS_SUSPEND_LIST, struct kel_thread, sgrt_link);
}

/*!
 * @brief	get the highest sleep thread (if sleep list is not empty)
 * @param  	none
 * @retval 	first thread
 * @note   	none
 */
struct kel_thread *get_first_sleep_thread(void)
{
    kbool_t existed = mrt_list_head_empty(KEL_TABS_SLEEP_LIST);
    return existed ? mrt_nullptr : mrt_list_first_entry(KEL_TABS_SLEEP_LIST, struct kel_thread, sgrt_link);
}

/*!
 * @brief	switch thread from one status to another status
 * @param  	tid: target thread
 * @param	src: current status
 * @param	dst: target status
 * @retval 	err code
 * @note   	only the running thread need to save context; and only the ready thread maybe need to restore context
 */ 
ksint32_t kel_sched_switch_thread(real_thread_t tid)
{
    kuint32_t src, dst;
	ksint32_t retval;
    
    src = KEL_TABS_THREAD_HANDLER(tid)->status;
    dst = KEL_TABS_THREAD_HANDLER(tid)->to_status;

	/*!<
	 * thread switch:
	 * (At all times, it is necessary to ensure that at least one thread (including idle threads) is running)
	 * running ---> ready/suspend/sleep
	 * ready ---> running/suspend/sleep
	 * suspend ---> ready/sleep
	 * sleep ---> ready/suspend
	 *
	 * (only running and ready status can be switched to any status)
	 */
	if ((src == dst) || (dst >= NR_THREAD_STATUS_MAX))
		goto fail;

	/*!< for idle thread, only ready and running status can be chosen */
	if ((tid == KEL_THREAD_TID_IDLE) && ((dst != NR_THREAD_RUNNING) && (dst != NR_THREAD_READY)))
		goto fail;

	/*!< detached from current list */
	switch (src)
	{
		case NR_THREAD_RUNNING:
			if (0 > kel_sched_reinstall_work_role(tid))
				goto fail;
			
			break;

		case NR_THREAD_READY:
			kel_sched_detach_ready_list(tid);
			break;

		case NR_THREAD_SUSPEND:
			kel_sched_detach_suspend_list(tid);
			break;

		case NR_THREAD_SLEEP:
			kel_sched_detach_sleep_list(tid);
			break;

		default:
			break;
	}

	/*!< add to new list */
	switch (dst)
	{
		case NR_THREAD_RUNNING:
			retval = kel_sched_despoil_work_role(tid);
			break;

		case NR_THREAD_READY:
			retval = kel_sched_add_ready_list(tid);
			break;

		case NR_THREAD_SUSPEND:
			retval = kel_sched_add_suspend_list(tid);
			break;

		case NR_THREAD_SLEEP:
			retval = kel_sched_add_sleep_list(tid);
			break;

		default:
			retval = -NR_IS_ERROR;
			break;
	}

	if (retval < 0)
	{
		print_warn("switch thread failed ! current and target status is : %d, %d\n", src, dst);
		return retval;
	}

	if (!KEL_TABS_RUNNING_THREAD)
	{
		print_err("no thread is running !!! dangerous action !!!\n");
		return retval;
	}

	/*!< update thread status */
	KEL_SYNC_THREAD_STATUS(tid, dst);

	return NR_IS_NORMAL;
    
fail:
    KEL_SYNC_THREAD_STATUS(tid, src);
    return -NR_IS_UNVALID;
}

/*!
 * @brief	change thread status to running
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	only ready status can be switched to running!!!
 */
static ksint32_t kel_sched_despoil_work_role(real_thread_t tid)
{
	struct kel_thread *sprt_thread;
	struct kel_thread *sprt_running;
	ksint32_t retval;

	sprt_thread = KEL_TABS_THREAD_HANDLER(tid);

	if (!sprt_thread)
		return -NR_IS_FAULT;

	/*!< only ready status can be switched to running */
	if (NR_THREAD_READY != sprt_thread->status)
		return -NR_IS_UNVALID;

	/*!< get current */
	sprt_running = KEL_TABS_RUNNING_THREAD;

	if (sprt_running)
	{
		/*!< current thread add to ready list */
		retval = kel_sched_add_ready_list(sprt_running->tid);
		if (retval < 0)
			return retval;

		KEL_SYNC_THREAD_STATUS(sprt_running->tid, NR_THREAD_READY);
	}

	/*!< update current */
	KEL_TABS_RUNNING_THREAD = sprt_thread;

	return NR_IS_NORMAL;
}

/*!
 * @brief	change current thread status
 * @param  	none
 * @retval 	err code
 * @note   	running ---> xxx
 */
static ksint32_t kel_sched_reinstall_work_role(real_thread_t tid)
{
	struct kel_thread *sprt_thread;

	if (tid != KEL_TABS_RUNNING_THREAD->tid)
		return -NR_IS_UNVALID;

	/*!< no thread ready; current should be set to idle thread */
	if (mrt_list_head_empty(KEL_TABS_READY_LIST))
	{
		KEL_TABS_RUNNING_THREAD = mrt_nullptr;
		return -NR_IS_FAULT;
	}

	/*!< get the first ready thread */
	sprt_thread = mrt_list_first_valid_entry(KEL_TABS_READY_LIST, struct kel_thread, sgrt_link);
	if (sprt_thread)
	{
		/*!< detached from ready list */
		kel_sched_detach_ready_list(sprt_thread->tid);

		/*!< ready thread ---> running */
		KEL_TABS_RUNNING_THREAD = sprt_thread;
		KEL_SYNC_THREAD_STATUS(sprt_thread->tid, NR_THREAD_RUNNING);
	}

	return sprt_thread ? NR_IS_NORMAL : (-NR_IS_FAILD);
}

/*!
 * @brief	change target thread status to ready
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	add to ready list
 */
static ksint32_t kel_sched_add_ready_list(real_thread_t tid)
{
	struct kel_thread *sprt_thread;

	sprt_thread = KEL_TABS_THREAD_HANDLER(tid);

	if (!sprt_thread)
		return -NR_IS_FAULT;

	/*!< avoid duplicate additions */
	if (NR_THREAD_READY == sprt_thread->status)
		return -NR_IS_UNVALID;

	return __kel_sched_add_status_list(sprt_thread, KEL_TABS_READY_LIST);
}

/*!
 * @brief	change target thread status from ready
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	del from ready list. it must be called by kel_sched_switch_thread, do not use alone !!!
 * 			(after detaching from the ready list, the thread will appear in a free state, 
 * 			so this function prohibits external calls to prevent the thread from leaving management and causing memory leakage)
 */
static ksint32_t kel_sched_detach_ready_list(real_thread_t tid)
{
	struct kel_thread *sprt_thread;

	sprt_thread = KEL_TABS_THREAD_HANDLER(tid);

	if (!sprt_thread)
		return -NR_IS_FAULT;

	/*!< check if is in ready status */
	if (NR_THREAD_READY != sprt_thread->status)
	{
		return -NR_IS_UNVALID;
	}

	/*!< delete it */
	__kel_sched_del_status_list(sprt_thread, KEL_TABS_READY_LIST);

	return NR_IS_NORMAL;
}

/*!
 * @brief	change target thread status to suspend
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	add to suspend list
 */
static ksint32_t kel_sched_add_suspend_list(real_thread_t tid)
{
	struct kel_thread *sprt_thread;

	sprt_thread = KEL_TABS_THREAD_HANDLER(tid);

	if (!sprt_thread)
		return -NR_IS_FAULT;

	/*!< avoid duplicate additions */
	if (NR_THREAD_SUSPEND == sprt_thread->status)
		return -NR_IS_UNVALID;

	return __kel_sched_add_status_list(sprt_thread, KEL_TABS_SUSPEND_LIST);
}

/*!
 * @brief	change target thread status from suspend
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	del from suspend list. it must be called by kel_sched_switch_thread, do not use alone !!!
 * 			(after detaching from the suspend list, the thread will appear in a free state, 
 * 			so this function prohibits external calls to prevent the thread from leaving management and causing memory leakage)
 */
static ksint32_t kel_sched_detach_suspend_list(real_thread_t tid)
{
	struct kel_thread *sprt_thread;

	sprt_thread = KEL_TABS_THREAD_HANDLER(tid);

	if (!sprt_thread)
		return -NR_IS_FAULT;

	/*!< check if is in ready status */
	if (NR_THREAD_SUSPEND != sprt_thread->status)
		return -NR_IS_UNVALID;

	/*!< delete it */
	__kel_sched_del_status_list(sprt_thread, KEL_TABS_SUSPEND_LIST);

	return NR_IS_NORMAL;
}

/*!
 * @brief	change target thread status to sleep
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	add to sleep list
 */
static ksint32_t kel_sched_add_sleep_list(real_thread_t tid)
{
	struct kel_thread *sprt_thread;

	sprt_thread = KEL_TABS_THREAD_HANDLER(tid);

	if (!sprt_thread)
		return -NR_IS_FAULT;

	/*!< avoid duplicate additions */
	if (NR_THREAD_SLEEP == sprt_thread->status)
		return -NR_IS_UNVALID;

	return __kel_sched_add_status_list(sprt_thread, KEL_TABS_SLEEP_LIST);
}

/*!
 * @brief	change target thread status from sleep
 * @param  	tid: target thread
 * @retval 	err code
 * @note   	del from sleep list. it must be called by kel_sched_switch_thread, do not use alone !!!
 * 			(after detaching from the sleep list, the thread will appear in a free state, 
 * 			so this function prohibits external calls to prevent the thread from leaving management and causing memory leakage)
 */
static ksint32_t kel_sched_detach_sleep_list(real_thread_t tid)
{
	struct kel_thread *sprt_thread;

	sprt_thread = KEL_TABS_THREAD_HANDLER(tid);

	if (!sprt_thread)
		return -NR_IS_FAULT;

	/*!< check if is in ready status */
	if (NR_THREAD_SLEEP != sprt_thread->status)
		return -NR_IS_UNVALID;

	/*!< delete it */
	__kel_sched_del_status_list(sprt_thread, KEL_TABS_SLEEP_LIST);

	return NR_IS_NORMAL;
}

/*!
 * @brief	check if target thread is in the target list
 * @param  	tid: target thread
 * @param	sprt_head: ready/suspend/sleep list
 * @retval 	err code
 * @note   	find thread if it is exsited
 */
static ksint32_t __kel_sched_find_thread(real_thread_t tid, struct list_head *sprt_head)
{
	struct kel_thread *sprt_anyTask;
	struct kel_thread *sprt_thread = KEL_TABS_THREAD_HANDLER(tid); 

	foreach_list_next_entry(sprt_anyTask, sprt_head, sgrt_link)
	{
		if (sprt_anyTask == sprt_thread)
			return tid;
	}

	return -NR_IS_NOTFOUND;
}

/*!
 * @brief	add to target list
 * @param  	sprt_thread: target thread
 * @param	sprt_head: ready/suspend/sleep list
 * @retval 	err code
 * @note   	every thread will be sorted by priority
 */
static ksint32_t __kel_sched_add_status_list(struct kel_thread *sprt_thread, struct list_head *sprt_head)
{
	struct kel_thread *sprt_anyTask;
	kuint32_t iPriority;

	if ((!sprt_thread) || (!sprt_head))
		return -NR_IS_FAULT;

	/*!< get priority */
	iPriority = real_thread_get_priority(sprt_thread->sprt_attr);

	/*!< target list is empty, add directly */
	if (mrt_list_head_empty(sprt_head))
		goto END;

	/*!< fault tolerance mechanism: check if this thread has been added to the list, and exit directly if it has been added */
	if (0 <= __kel_sched_find_thread(sprt_thread->tid, sprt_head))
		return NR_IS_NORMAL;

	/*!< traversing the ready list, inserting new thread into the tail of thread which is the same priority */
	foreach_list_prev_entry(sprt_anyTask, sprt_head, sgrt_link)
	{
		if (iPriority <= real_thread_get_priority(sprt_anyTask->sprt_attr))
		{
			/*!< priority from high to low */
			list_head_add_head(&sprt_anyTask->sgrt_link, &sprt_thread->sgrt_link);
			return NR_IS_NORMAL;
		}
	}

END:
	/*!< target thread has the highest priority */
	list_head_add_head(sprt_head, &sprt_thread->sgrt_link);

	return NR_IS_NORMAL;
}

/*!
 * @brief	delete from target list
 * @param  	sprt_thread: target thread
 * @param	sprt_head: ready/suspend/sleep list
 * @retval 	err code
 * @note   	none
 */
static void __kel_sched_del_status_list(struct kel_thread *sprt_thread, struct list_head *sprt_head)
{
	if ((!sprt_thread) || (!sprt_head))
		return;

	/*!< check if target link is in the list before deleting */
	list_head_del_anyone(sprt_head, &sprt_thread->sgrt_link);
	init_list_head(&sprt_thread->sgrt_link);
}

/*!
 * @brief	register a new thread, which will be added to ready list
 * @param  	sprt_thread: target thread
 * @param	tid: global tcb index
 * @retval 	err code
 * @note   	all new threads should be added to ready list at first
 */
ksint32_t register_kel_sched_thread(struct kel_thread *sprt_thread, real_thread_t tid)
{
	srt_kel_thread_attr_t *sprt_it_attr;
	ksint32_t retval;

	sprt_it_attr = sprt_thread->sprt_attr;

	if (KEL_TABS_THREAD_HANDLER(tid))
		return -NR_IS_UNVALID;

	/*!< stack must be valid */
	if (!sprt_it_attr->stack_addr)
		return -NR_IS_NOMEM;

	/*!< saved to tcb */
	KEL_TABS_THREAD_HANDLER(tid) = sprt_thread;

	/* initial link */
	init_list_head(&sprt_thread->sgrt_link);

	/*!< add and sorted by priority */
	retval = kel_sched_add_ready_list(tid);
	if (retval < 0)
	{
		KEL_TABS_THREAD_HANDLER(tid) = mrt_nullptr;
		return retval;
	}

	/*!< set to ready status */
    KEL_SYNC_THREAD_STATUS(tid, NR_THREAD_READY);

	return NR_IS_NORMAL;
}

/*!
 * @brief	init thread (ready to running)
 * @param  	none
 * @retval 	none
 * @note   	reload time slice
 */
void __real_thread_init_before(void)
{
    struct kel_thread *sprt_thread = KEL_TABS_RUNNING_THREAD;
    kuint32_t milseconds = real_thread_get_sched_msecs(sprt_thread->sprt_attr);
    
    sprt_thread->expires = jiffies + msecs_to_jiffies(milseconds);
}

/*!
 * @brief	start schedule (ready to running)
 * @param  	none
 * @retval 	none
 * @note   	select a highest priority thread from ready list, and swicth it to running status
 */
struct kel_context_info *__real_thread_schedule(void)
{
	struct kel_thread *sprt_thread;
	struct kel_thread *sprt_prev;
    ksint32_t retval;

	/*!< save to (*.data) section, do not defines in stack */
	static struct kel_context_info sgrt_context;
	static kuint32_t thread_schedule_ref = 0;

	/*!< no ready thread here, unable to start or switch */
	if (mrt_list_head_empty(KEL_TABS_READY_LIST))
		goto fail;

	/*!< get the current thread */
	sprt_prev = KEL_TABS_RUNNING_THREAD;
	if (!sprt_prev)
    {
        if (thread_schedule_ref)
            goto fail;
        else
        {
            /*!< scheduled by "start_kernel" for the first time */
            sprt_prev = mrt_list_first_valid_entry(KEL_TABS_READY_LIST, struct kel_thread, sgrt_link);
            if (!sprt_prev)
                goto fail;
            
            KEL_SET_THREAD_STATUS(sprt_prev->tid, NR_THREAD_RUNNING);
        }
    }
    
    /*!< if not set target status, default to ready */
    if (!sprt_prev->to_status)
        KEL_SET_THREAD_STATUS(sprt_prev->tid, NR_THREAD_READY);

	/*!< select next valid thread */
	retval = kel_sched_switch_thread(sprt_prev->tid);
    sprt_thread = KEL_TABS_RUNNING_THREAD;
	if ((retval < 0) || (!sprt_thread))
		goto fail;
    
	sgrt_context.first = (kuaddr_t)&thread_schedule_ref;
	sgrt_context.entry = (kuaddr_t)&sprt_thread->start_routine;
	sgrt_context.args = (kuaddr_t)&sprt_thread->ptr_args;
	sgrt_context.prev_sp = 0;
	sgrt_context.next_sp = real_thread_get_stack(sprt_thread->sprt_attr);

	if (thread_schedule_ref)
		sgrt_context.prev_sp = real_thread_get_stack(sprt_prev->sprt_attr);

    /*!< address of sgrt_context ===> r0 */
    return &sgrt_context;

fail:
	return mrt_nullptr;
}

/*!
 * @brief	start schedule (ready to running)
 * @param  	none
 * @retval 	none
 * @note   	select a highest priority thread from ready list, and swicth it to running status
 */
void real_thread_schedule(void)
{
    struct kel_context_info *sprt_context;

	mrt_preempt_disable();
    sprt_context = __real_thread_schedule();
    if (!sprt_context)
	{
		mrt_preempt_enable();
        return;
	}

	/*!< sprt_context ===> r0 */
	context_switch(sprt_context);
}

/*!< end of file */
