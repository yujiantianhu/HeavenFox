/*
 * Kernel Thread Management Defines
 *
 * File Name:   thread.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.09
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef _KEL_THREAD_H_
#define _KEL_THREAD_H_

/*!< The includes */
#include <common/generic.h>
#include <common/list_types.h>
#include <common/api_string.h>
#include <common/io_stream.h>
#include <common/time.h>
#include <board/board_common.h>
#include <boot/boot_text.h>
#include <boot/implicit_call.h>
#include <platform/kmem_pool.h>
#include <kernel/kernel.h>
#include <kernel/context.h>
#include <kernel/sleep.h>

/*!< The defines */
typedef ksint32_t real_thread_t;

/*!< maximum number of threads that can be created */
#define KEL_THREAD_MAX_NUM						(128)

/*!< minimum space for thread stack (unit: byte) */
#define KEL_THREAD_STACK8(byte)					(mrt_align4(byte) >> 0)
#define KEL_THREAD_STACK16(half)				(mrt_align4(half) >> 1)
#define KEL_THREAD_STACK32(word)				(mrt_align4(word) >> 2)

/*!< 1 page = 4 kbytes; half page = (1 / 2) page; quarter = (1 / 4) page */
#define KEL_THREAD_STACK_PAGE(page)				(KEL_THREAD_STACK32(((kuint32_t)(page)) << 12))
#define KEL_THREAD_STACK_HALF(page)				(KEL_THREAD_STACK32(((kuint32_t)(page)) << 11))
#define KEL_THREAD_STACK_QUAR(page)				(KEL_THREAD_STACK32(((kuint32_t)(page)) << 10))

#define KEL_THREAD_STACK_MIN					KEL_THREAD_STACK8(1024)
#define KEL_THREAD_STACK_DEFAULT				KEL_THREAD_STACK8(2056)

/*!<
 * tid base 
 * 0 ~ 31: kernel thread; 31 ~ KEL_THREAD_MAX_NUM: user thread
 */
#define KEL_THREAD_TID_START					(32)

#define KEL_THREAD_TID_IDLE                     (0)                 /*!< idle thread */
#define KEL_THREAD_TID_BASE                     (1)                 /*!< kernel thread (parent) */
#define KEL_THREAD_TID_INIT                     (2)                 /*!< init thread */
#define KEL_THREAD_TID_TIME                     (3)                 /*!< timer thread */

/*!<
 * priority
 * kernel thread requires higher priority (80 ~ 99)
 * The higher the value, the higher the priority
 */
#define KEL_THREAD_PROTY_START					(0)
#define KEL_THREAD_PROTY_DEFAULT				(80)
#define KEL_THREAD_PROTY_MAX					(99)

#define KEL_THREAD_PROTY_IDLE				    (1)
#define KEL_THREAD_PROTY_KERNEL					(KEL_THREAD_PROTY_MAX)
#define KEL_THREAD_PROTY_INIT					(KEL_THREAD_PROTY_KERNEL - 1)

/*!< time slice */
#define KEL_THREAD_TIME_DEFUALT                 (100)               /*!< unit: ms */

/*!< policy */
enum __ERT_KEL_THREAD_DETACH
{
	/*!< joinable */
	KEL_THREAD_CREATE_JOINABLE = 0,

	/*!< detached */
	KEL_THREAD_CREATE_DETACHED
};

/*!<
 * inheritance policy
 * whether to inherit the attributes of the parent thread, this setting takes effect during the thread creation phase
 */
enum __ERT_KEL_THREAD_SCHED
{
	/*!< inherit */
	KEL_THREAD_INHERIT_SCHED = 0,

	/*!< explicit */
	KEL_THREAD_EXPLICIT_SCHED
};

/*!< schedule policy */
enum __ERT_KEL_THREAD_POLICY
{
	/*!< normal */
	KEL_THREAD_SCHED_OTHER = 0,

	/*!< preemptive schedule */
	KEL_THREAD_SCHED_FIFO,

	/*!< polling schedule */
	KEL_THREAD_SCHED_RR
};

struct kel_sched_param
{
    ksint32_t sched_priority;
    ksint32_t sched_curpriority;

    union
    {
    	ksint32_t reserved[8];

        struct
        {
    		ksint32_t __ss_low_priority;
    		ksint32_t __ss_max_repl;
            struct time_spec __ss_repl_period;
            struct time_spec __ss_init_budget;

        } sgrt_ss;

    } ugrt_ss;

#define mrt_sched_low_priority   			    ugrt_ss.sgrt_ss.__ss_low_priority
#define mrt_sched_max_repl					    ugrt_ss.sgrt_ss.__ss_max_repl
#define mrt_sched_repl_period				    ugrt_ss.sgrt_ss.__ss_repl_period
#define mrt_sched_init_budget				    ugrt_ss.sgrt_ss.__ss_init_budget
};

struct kel_thread_attr
{
	ksint32_t detachstate;     										/*!< refer to "__ERT_KEL_THREAD_DETACH" */
	ksint32_t schedpolicy;     										/*!< refer to "__ERT_KEL_THREAD_SCHED" */
    struct kel_sched_param sgrt_param;  							/*!< schedule parameters */
    ksint32_t inheritsched;    										/*!< refer to "__ERT_KEL_THREAD_POLICY" */
    ksint32_t scope;           										/*!< the scope of threads */
    kssize_t guardsize;       										/*!< the size of the alert buffer at the end of the thread stack */

    void *ptr_stack_start;											/*!< thread stack address base (from dynamic allocation) */
    kuaddr_t stack_addr;       										/*!< thread stack top, 8 byte anlignment  */
    kusize_t stacksize;       										/*!< thread stack size (unit: byte), the minimum can be set to KEL_THREAD_STACK_MIN */
};
typedef struct kel_thread_attr srt_kel_thread_attr_t;

/*!< The defines */
TARGET_EXT ksint32_t kernel_thread_create(real_thread_t *ptr_id, 
                                                srt_kel_thread_attr_t *sprt_attr, 
                                                void *(*pfunc_start_routine) (void *), 
                                                void *ptr_args);

TARGET_EXT ksint32_t real_thread_create(real_thread_t *ptr_id, 
                                                srt_kel_thread_attr_t *sprt_attr, 
                                                void *(*pfunc_start_routine) (void *), 
                                                void *ptr_args);

TARGET_EXT ksint32_t kernel_thread_idle_create(srt_kel_thread_attr_t *sprt_attr, 
                                                void *(*pfunc_start_routine) (void *), 
                                                void *ptr_args);

TARGET_EXT ksint32_t kernel_thread_base_create(srt_kel_thread_attr_t *sprt_attr, 
                                                void *(*pfunc_start_routine) (void *), 
                                                void *ptr_args);
                                                
TARGET_EXT ksint32_t kernel_thread_init_create(srt_kel_thread_attr_t *sprt_attr, 
                                                void *(*pfunc_start_routine) (void *), 
                                                void *ptr_args);
                                                
TARGET_EXT ksint32_t kernel_thread_time_create(srt_kel_thread_attr_t *sprt_attr, 
                                                void *(*pfunc_start_routine) (void *), 
                                                void *ptr_args);

TARGET_EXT void *real_thread_attr_init(srt_kel_thread_attr_t *sprt_attr);
TARGET_EXT void *real_thread_attr_revise(srt_kel_thread_attr_t *sprt_attr);
TARGET_EXT void real_thread_attr_destroy(srt_kel_thread_attr_t *sprt_attr);

/*!< API functions */
/*!
 * @brief	get current priority
 * @param  	sprt_attr
 * @retval 	priority
 * @note   	none
 */
static inline kuint32_t real_thread_get_priority(srt_kel_thread_attr_t *sprt_attr)
{
	return sprt_attr->sgrt_param.sched_curpriority;
}

/*!
 * @brief	set priority
 * @param  	sprt_attr, priority
 * @retval 	none
 * @note   	none
 */
static inline void real_thread_set_priority(srt_kel_thread_attr_t *sprt_attr, kuint32_t priority)
{
	sprt_attr->sgrt_param.sched_priority = (priority <= KEL_THREAD_PROTY_MAX) ? priority : KEL_THREAD_PROTY_MAX;
    sprt_attr->sgrt_param.sched_curpriority = sprt_attr->sgrt_param.sched_priority;
}

/*!
 * @brief	set time slice
 * @param  	sprt_attr, time
 * @retval 	none
 * @note   	none
 */
static inline void real_thread_set_time_slice(srt_kel_thread_attr_t *sprt_attr, kutime_t mseconds)
{
    struct time_spec sgrt_tm;
    
    msecs_to_time_spec(&sgrt_tm, mseconds);
    memcpy(&sprt_attr->sgrt_param.mrt_sched_init_budget, &sgrt_tm, sizeof(sgrt_tm));
}

/*!
 * @brief	get time slice (to milseconds)
 * @param  	sprt_attr
 * @retval 	milseconds
 * @note   	none
 */
static inline kuint32_t real_thread_get_sched_msecs(srt_kel_thread_attr_t *sprt_attr)
{
    return time_spec_to_msecs(&sprt_attr->sgrt_param.mrt_sched_init_budget);
}

/*!
 * @brief	set stack size
 * @param  	sprt_attr, stacksize
 * @retval 	none
 * @note   	stack-size of each thread must more than KEL_THREAD_STACK_MIN
 */
static inline void real_thread_attr_setstacksize(srt_kel_thread_attr_t *sprt_attr, kusize_t stacksize)
{
	sprt_attr->stacksize = (stacksize >= KEL_THREAD_STACK_MIN) ? stacksize : KEL_THREAD_STACK_MIN;
}

/*!
 * @brief	get stack size
 * @param  	sprt_attr
 * @retval 	stack size
 * @note   	none
 */
static inline kuint32_t real_thread_attr_getstacksize(srt_kel_thread_attr_t *sprt_attr)
{
	return sprt_attr->stacksize;
}

/*!
 * @brief	set stack
 * @param  	sprt_attr: thread attibute
 * @param   ptr_dync: stack which allocated by dynamic (kmalloc/malloc)
 * @param   ptr_stack: stack base (low address)
 * @param   stacksize: total size of stack
 * @retval 	stack address (top of stack)
 * @note   	if stack is allocated by mempool, ptr_dync should set to be ptr_stack, so that it can be released by sprt_attr->ptr_stack_start
 */
static inline void *real_thread_set_stack(srt_kel_thread_attr_t *sprt_attr, void *ptr_dync, void *ptr_stack, kusize_t stacksize)
{
	struct kel_context_regs *sprt_regs;

	if (!mrt_isValid(ptr_stack) || (stacksize < KEL_THREAD_STACK_MIN))
	{
		return mrt_nullptr;
	}

	/*!< check: ptr_dync just should be NULL or ptr_stack */
	if (mrt_isValid(ptr_dync) && (ptr_dync != ptr_stack))
	{
		return mrt_nullptr;
	}

	/*!< if the stack is defined in a static storage area, ptr_dync should be NULL; Otherwise, the address of ptr_stack should be passed in */
	sprt_attr->ptr_stack_start = ptr_dync;

	/*!< pointer to stack top with 4 byte alignment */
	sprt_attr->stack_addr = (kuaddr_t)((kuint8_t *)ptr_stack + stacksize);
	sprt_attr->stack_addr = (sprt_attr->stack_addr - 16) & (~0x07);
	sprt_attr->stack_addr -= sizeof(struct kel_context_regs);
	sprt_attr->stack_addr &= ~0x07;
	sprt_attr->stacksize = stacksize;

	sprt_regs = (struct kel_context_regs *)sprt_attr->stack_addr;
	memset(sprt_regs, 0, sizeof(struct kel_context_regs));

	return (void *)sprt_attr->stack_addr;
}

/*!
 * @brief	get address of stack_addr
 * @param  	sprt_attr
 * @retval 	&sprt_attr->stack_addr
 * @note   	stack = *(&sprt_attr->stack_addr) (excluding kel_context_regs)
 */
static inline kuaddr_t real_thread_get_stack(srt_kel_thread_attr_t *sprt_attr)
{
	return (kuaddr_t)(&sprt_attr->stack_addr);
}

/*!
 * @brief	set detach state
 * @param  	sprt_attr, detach state
 * @retval 	none
 * @note   	none
 */
static inline void real_thread_attr_setdetachstate(srt_kel_thread_attr_t *sprt_attr, kuint32_t state)
{
	sprt_attr->detachstate = state;
}

/*!
 * @brief	get detach state
 * @param  	sprt_attr
 * @retval 	detach state
 * @note   	none
 */
static inline kuint32_t real_thread_attr_getdetachstate(srt_kel_thread_attr_t *sprt_attr)
{
	return sprt_attr->detachstate;
}

/*!
 * @brief	set inherit policy
 * @param  	sprt_attr, sched
 * @retval 	none
 * @note   	none
 */
static inline void real_thread_attr_setinheritsched(srt_kel_thread_attr_t *sprt_attr, kuint32_t sched)
{
	sprt_attr->inheritsched	= sched;
}

/*!
 * @brief	get inherit policy
 * @param  	sprt_attr
 * @retval 	inherit policy
 * @note   	none
 */
static inline kuint32_t real_thread_attr_getinheritsched(srt_kel_thread_attr_t *sprt_attr)
{
	return sprt_attr->inheritsched;
}

/*!
 * @brief	set schedule policy
 * @param  	sprt_attr, schedule policy
 * @retval 	none
 * @note   	none
 */
static inline void real_thread_attr_setschedpolicy(srt_kel_thread_attr_t *sprt_attr, kuint32_t policy)
{
	sprt_attr->schedpolicy = policy;
}

/*!
 * @brief	set schedule policy
 * @param  	sprt_attr
 * @retval 	schedule policy
 * @note   	none
 */
static inline kuint32_t real_thread_attr_getschedpolicy(srt_kel_thread_attr_t *sprt_attr)
{
	return sprt_attr->schedpolicy;
}

/*!
 * @brief	set schedule parameters
 * @param  	sprt_attr, sprt_param
 * @retval 	none
 * @note   	copy param to attribute
 */
static inline void real_thread_attr_setschedparam(srt_kel_thread_attr_t *sprt_attr, struct kel_sched_param *sprt_param)
{
	memcpy(&sprt_attr->sgrt_param, sprt_param, sizeof(struct kel_sched_param));
}

/*!
 * @brief	get schedule parameters
 * @param  	sprt_attr, sprt_param
 * @retval 	none
 * @note   	copy param from attribute
 */
static inline void real_thread_attr_getschedparam(srt_kel_thread_attr_t *sprt_attr, struct kel_sched_param *sprt_param)
{
	memcpy(sprt_param, &sprt_attr->sgrt_param, sizeof(struct kel_sched_param));
}


#endif /* _KEL_THREAD_H_ */
