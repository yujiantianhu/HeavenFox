/*
 * Thread Management Interface
 *
 * File Name:   thread.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.09
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <kernel/thread.h>
#include <kernel/sched.h>

/*!< API functions */
/*!
 * @brief	create a new thread
 * @param  	...
 * @retval 	err code
 * @note   	configure attribute and thread
 */
static kint32_t __real_thread_create(real_thread_t *ptr_id, kint32_t base, struct real_thread_attr *sprt_attr,
                                    void *(*pfunc_start_routine) (void *), void *ptr_args, kuint32_t flags)
{
	real_thread_t tid;
	struct real_thread *sprt_thread;
	struct real_thread_attr *sprt_it_attr;
	kuint32_t i_start, count;
	kint32_t retval;

	/*!< check if is user thread */
	if ((REAL_THREAD_USER & flags) == REAL_THREAD_USER)
	{
		i_start = REAL_THREAD_TID_START;
		count	= REAL_THREAD_MAX_NUM - REAL_THREAD_TID_START;
	}
	/*!< kernel thread */
	else
	{
		i_start = 0;
		count	= REAL_THREAD_TID_START;
	}
    
    if ((base >= i_start) && (base < (i_start + count)))
    {
        i_start = base;
        count   = 1;
    }

	/*!< find a free tid */
	tid = get_unused_tid_from_scheduler(i_start, count);
	if (tid < 0)
		goto fail;

	sprt_it_attr = sprt_attr;

	if (!sprt_attr)
	{
		sprt_it_attr = (struct real_thread_attr *)kzalloc(sizeof(struct real_thread_attr), GFP_KERNEL);
		if (!isValid(sprt_it_attr))
			goto fail;

		/*!< initialize attr */
		if (!real_thread_attr_init(sprt_it_attr))
			goto fail2;
	}

	/*!< check if attr valid */
	if (!real_thread_attr_revise(sprt_it_attr))
		goto fail3;

	/*!< create new dynamic thread */
	sprt_thread = (struct real_thread *)kzalloc(sizeof(struct real_thread), GFP_KERNEL);
	if (!isValid(sprt_thread))
		goto fail3;

	sprt_thread->tid 			= tid;
	sprt_thread->sprt_attr 		= sprt_it_attr;
	sprt_thread->start_routine 	= pfunc_start_routine;
	sprt_thread->ptr_args		= ptr_args;

	/*!< add to ready list */
	retval = register_new_thread(sprt_thread, tid);
	if (retval < 0)
		goto fail4;

    if (ptr_id)
        *ptr_id = tid;
    
	return ER_NORMAL;

fail4:
	kfree(sprt_thread);
fail3:
	real_thread_attr_destroy(sprt_it_attr);
fail2:
	if (!isValid(sprt_attr))
		kfree(sprt_it_attr);
fail:
	return -ER_FAULT;
}

/*!
 * @brief	create kernel thread
 * @param  	...
 * @retval 	err code
 * @note   	none
 */
kint32_t __kernel_thread_create(real_thread_t *ptr_id, kint32_t base, struct real_thread_attr *sprt_attr, void *(*pfunc_start_routine) (void *), void *ptr_args)
{
	return __real_thread_create(ptr_id, base, sprt_attr, pfunc_start_routine, ptr_args, 0);
}

/*!
 * @brief	create user thread
 * @param  	...
 * @retval 	err code
 * @note   	none
 */
kint32_t __real_user_thread_create(real_thread_t *ptr_id, kint32_t base, struct real_thread_attr *sprt_attr, void *(*pfunc_start_routine) (void *), void *ptr_args)
{
	return __real_thread_create(ptr_id, base, sprt_attr, pfunc_start_routine, ptr_args, REAL_THREAD_USER);
}

/*!
 * @brief	create kernel thread
 * @param  	...
 * @retval 	err code
 * @note   	none
 */
kint32_t kernel_thread_create(real_thread_t *ptr_id, struct real_thread_attr *sprt_attr, void *(*pfunc_start_routine) (void *), void *ptr_args)
{
	return __kernel_thread_create(ptr_id, -1, sprt_attr, pfunc_start_routine, ptr_args);
}

/*!
 * @brief	create user thread
 * @param  	...
 * @retval 	err code
 * @note   	none
 */
kint32_t real_thread_create(real_thread_t *ptr_id, struct real_thread_attr *sprt_attr, void *(*pfunc_start_routine) (void *), void *ptr_args)
{
	return __real_user_thread_create(ptr_id, -1, sprt_attr, pfunc_start_routine, ptr_args);
}

/*!
 * @brief	create idle thread
 * @param  	...
 * @retval 	err code
 * @note   	tid is fixed to REAL_THREAD_TID_IDLE
 */
kint32_t kernel_thread_idle_create(struct real_thread_attr *sprt_attr, void *(*pfunc_start_routine) (void *), void *ptr_args)
{  
    return __kernel_thread_create(mrt_nullptr, REAL_THREAD_TID_IDLE, sprt_attr, pfunc_start_routine, ptr_args);
}

/*!
 * @brief	create base thread
 * @param  	...
 * @retval 	err code
 * @note   	tid is fixed to REAL_THREAD_TID_BASE
 */
kint32_t kernel_thread_base_create(struct real_thread_attr *sprt_attr, void *(*pfunc_start_routine) (void *), void *ptr_args)
{  
    return __kernel_thread_create(mrt_nullptr, REAL_THREAD_TID_BASE, sprt_attr, pfunc_start_routine, ptr_args);
}

/*!
 * @brief	create init thread
 * @param  	...
 * @retval 	err code
 * @note   	tid is fixed to REAL_THREAD_TID_INIT
 */
kint32_t kernel_thread_init_create(struct real_thread_attr *sprt_attr, void *(*pfunc_start_routine) (void *), void *ptr_args)
{  
    return __kernel_thread_create(mrt_nullptr, REAL_THREAD_TID_INIT, sprt_attr, pfunc_start_routine, ptr_args);
}

/*!
 * @brief	create time thread
 * @param  	...
 * @retval 	err code
 * @note   	tid is fixed to REAL_THREAD_TID_TIME
 */
kint32_t kernel_thread_time_create(struct real_thread_attr *sprt_attr, void *(*pfunc_start_routine) (void *), void *ptr_args)
{  
    return __kernel_thread_create(mrt_nullptr, REAL_THREAD_TID_TIME, sprt_attr, pfunc_start_routine, ptr_args);
}

/*!
 * @brief	initial attribute
 * @param  	sprt_attr: attr
 * @retval 	none
 * @note   	none
 */
void *real_thread_attr_init(struct real_thread_attr *sprt_attr)
{
	void *ptr_stack;

	memset(sprt_attr, 0, sizeof(struct real_thread_attr));

	/*!< set default parmeters */
	/*!< detach state: join */
	sprt_attr->detachstate = REAL_THREAD_CREATE_JOINABLE;
	/*!< inherit policy: inherit */
	sprt_attr->inheritsched	= REAL_THREAD_INHERIT_SCHED;
	/*!< schedule policy: preempt */
	sprt_attr->schedpolicy = REAL_THREAD_SCHED_FIFO;

	/*!< stack: 128bytes */
	ptr_stack = kzalloc(REAL_THREAD_STACK_DEFAULT, GFP_KERNEL);
	if (!isValid(ptr_stack))
		return mrt_nullptr;

	real_thread_set_stack(sprt_attr, ptr_stack, ptr_stack, REAL_THREAD_STACK_DEFAULT);
	real_thread_set_priority(sprt_attr, REAL_THREAD_PROTY_DEFAULT);
    real_thread_set_time_slice(sprt_attr, REAL_THREAD_TIME_DEFUALT);

	return (void *)sprt_attr->stack_addr;
}

/*!
 * @brief	check attribute
 * @param  	sprt_attr: attr
 * @retval 	none
 * @note   	none
 */
void *real_thread_attr_revise(struct real_thread_attr *sprt_attr)
{
	void *ptr_stack;

	if (!sprt_attr)
		return mrt_nullptr;

	if (!sprt_attr->sgrt_param.sched_priority)
		real_thread_set_priority(sprt_attr, REAL_THREAD_PROTY_DEFAULT);
    
    if (mrt_is_timespec_empty(&sprt_attr->sgrt_param.mrt_sched_init_budget))
        real_thread_set_time_slice(sprt_attr, REAL_THREAD_TIME_DEFUALT);

	if (!sprt_attr->stack_addr)
	{
		/*!< stack: 128bytes */
		ptr_stack = kzalloc(REAL_THREAD_STACK_DEFAULT, GFP_KERNEL);
		if (!isValid(ptr_stack))
			return mrt_nullptr;		

		real_thread_set_stack(sprt_attr, ptr_stack, ptr_stack, REAL_THREAD_STACK_DEFAULT);
	}

	return (void *)sprt_attr->stack_addr;
}

/*!
 * @brief	destroy attribute
 * @param  	sprt_attr: attr
 * @retval 	none
 * @note   	none
 */
void real_thread_attr_destroy(struct real_thread_attr *sprt_attr)
{
	if (!sprt_attr)
		return;

	if (sprt_attr->ptr_stack_start)
		kfree(sprt_attr->ptr_stack_start);
	
	memset(sprt_attr, 0, sizeof(struct real_thread_attr));
}

/*!< end of file */
