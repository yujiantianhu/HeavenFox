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

#ifndef __THREAD_H_
#define __THREAD_H_

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/generic.h>
#include <common/list_types.h>
#include <common/api_string.h>
#include <common/io_stream.h>
#include <common/time.h>
#include <board/board.h>
#include <boot/boot_text.h>
#include <boot/implicit_call.h>
#include <platform/base/fwk_mempool.h>
#include <kernel/kernel.h>
#include <kernel/context.h>
#include <kernel/sleep.h>

/*!< The defines */
/*!< maximum number of threads that can be created */
#define THREAD_MAX_NUM						(1024)

/*!< minimum space for thread stack (unit: byte) */
#define THREAD_STACK8(byte)				    (mr_align4(byte) >> 0)
#define THREAD_STACK16(half)				(mr_align4(half) >> 1)
#define THREAD_STACK32(word)				(mr_align4(word) >> 2)

/*!< 1 page = 4 kbytes; half page = (1 / 2) page; quarter = (1 / 4) page */
#define THREAD_STACK_PAGE(page)			    ((kutype_t)(page) << 12U)
#define THREAD_STACK_HALF(page)			    ((kutype_t)(page) << 11U)
#define THREAD_STACK_QUAR(page)			    ((kutype_t)(page) << 10U)

#define THREAD_STACK_MIN					THREAD_STACK_HALF(1)
#define THREAD_STACK_DEFAULT				THREAD_STACK_PAGE(2)
#define THREAD_STACK_ALIGN                  THREAD_STACK_PAGE(1)

#define THREAD_STACK_DEFINE(name, size)     kuint8_t name[size]  __align(THREAD_STACK_ALIGN)

/*!<
 * tid base 
 * 0 ~ 31: fixed tid for kernel thread;
 * 32 ~ 127: dynamic tid for kernel thread;
 * 128 ~ THREAD_MAX_NUM: user thread
 */
#define THREAD_TID_IDLE                     (0)                 /*!< idle thread */
#define THREAD_TID_BASE                     (1)                 /*!< kernel thread (parent) */
#define THREAD_TID_INIT                     (2)                 /*!< init thread */

#define THREAD_TID_SOCKRX                   (19)
#define THREAD_TID_SOCKTX                   (20)

#define THREAD_TID_DYNC                     (32)                /*!< dynamic kernel thread */
#define THREAD_TID_USER					    (128)               /*!< user/application thread */

/*!<
 * priority
 * kernel thread requires higher priority (1 ~ 19)
 * The lower the value, the higher the priority
 */
#define THREAD_PROTY_NUM                    (100)
#define THREAD_PROTY_START					(99)
#define THREAD_PROTY_DEFAULT				(80)
#define THREAD_PROTY_MAX					(1)
#define THREAD_PROTY_MIN                    (99)
#define THREAD_PRORY_NONE                   (0)

#define __THREAD_CHECK_PRIO(prio)           CMP_MAX2(THREAD_PROTY_MAX, CMP_MIN2(prio, THREAD_PROTY_MIN))
#define __THREAD_IS_LOW_PRIO(prio, prio2)	((prio2) < (prio))
#define __THREAD_HIGHER_DEFAULT(val)		(THREAD_PROTY_DEFAULT - (val))	

#define THREAD_PROTY_IDLE				    (98)
#define THREAD_PROTY_KERNEL				    (32)
#define THREAD_PROTY_INIT					(THREAD_PROTY_KERNEL + 1)

#define THREAD_PROTY_TERM                   __THREAD_HIGHER_DEFAULT(0)

#define THREAD_PROTY_SOCKRX                 (19)
#define THREAD_PROTY_SOCKTX                 (20)
#define THREAD_PROTY_KSOFTIRQD              (THREAD_PROTY_MAX)
#define THREAD_PROTY_IRQ                    (THREAD_PROTY_MAX + 1)
#define THREAD_PROTY_KWORKER                (THREAD_PROTY_MAX + 1)
#define THREAD_PROTY_KMEMP				    (THREAD_PROTY_KERNEL + 2)

/*!< preempt period */
#define THREAD_PREEMPT_PERIOD               (10)                /*!< unit: ms */

/*!< time slice */
#define THREAD_TIME_DEFUALT                 (40)				/*!< unit: ms */
#define THREAD_TIME_KMEMP                   (20)				/*!< unit: ms */

#define THREAD_SWITCH_TIME                  (10)                /*!< thread switch interval, maybe 10us */

/*!< policy */
enum __ERT_THREAD_DETACH
{
    /*!< joinable */
    THREAD_CREATE_JOINABLE = 0,

    /*!< detached */
    THREAD_CREATE_DETACHED
};

/*!<
 * inheritance policy
 * whether to inherit the attributes of the parent thread, this setting takes effect during the thread creation phase
 */
enum __ERT_THREAD_SCHED
{
    /*!< inherit */
    THREAD_INHERIT_SCHED = 0,

    /*!< explicit */
    THREAD_EXPLICIT_SCHED
};

/*!< schedule policy */
enum __ERT_THREAD_POLICY
{
    /*!< normal */
    THREAD_SCHED_OTHER = 0,

    /*!< preemptive schedule */
    THREAD_SCHED_FIFO,

    /*!< polling schedule */
    THREAD_SCHED_RR
};

struct scheduler_param
{
    kint32_t priority;                          /*!< temprory priority */
    kint32_t cur_priority;                      /*!< current priority */
    kint32_t ori_priority;                      /*!< original priority */    

    struct time_spec init_budget;               /*!< time slice */
};

struct thread_attr
{
    kint32_t detachstate;                       /*!< refer to "__ERT_THREAD_DETACH" */
    kint32_t schedpolicy;                       /*!< refer to "__ERT_THREAD_SCHED" */
    kint32_t inheritsched;                      /*!< refer to "__ERT_THREAD_POLICY" */

    struct scheduler_param sgtc_param;          /*!< schedule parameters */

    void *ptr_stack_start;                      /*!< thread stack address base (from dynamic allocation) */
    kuaddr_t stack_addr;                        /*!< thread stack top, 8 byte anlignment  */
    kusize_t stacksize;                         /*!< thread stack size (unit: byte), the minimum can be set to THREAD_STACK_MIN */

    struct mem_info sgtc_pool;                  /*!< thread memory pool */
};
typedef struct thread_attr srt_thread_attr_t;

/*!< The defines */
extern tid_t kernel_thread_create(tid_t tid, 
                                struct thread_attr *sptr_attr, 
                                void *(*pfunc_start_routine) (void *), 
                                void *ptr_args);

extern kint32_t thread_create(tid_t *ptr_id, 
                                struct thread_attr *sptr_attr, 
                                void *(*pfunc_start_routine) (void *), 
                                void *ptr_args);

extern kint32_t kernel_thread_idle_create(struct thread_attr *sptr_attr, 
                                void *(*pfunc_start_routine) (void *), 
                                void *ptr_args);

extern kint32_t kernel_thread_base_create(struct thread_attr *sptr_attr, 
                                void *(*pfunc_start_routine) (void *), 
                                void *ptr_args);
                                                
extern kint32_t kernel_thread_init_create(struct thread_attr *sptr_attr, 
                                void *(*pfunc_start_routine) (void *), 
                                void *ptr_args);

extern kint32_t thread_quit(tid_t tid);
extern kint32_t thread_destory(tid_t tid);
extern void *thread_attr_init(struct thread_attr *sptr_attr);
extern void *thread_attr_revise(struct thread_attr *sptr_attr);
extern void thread_attr_destroy(struct thread_attr *sptr_attr);
extern struct thread_attr *thread_attr_get(tid_t tid);
extern void *thread_set_stack(struct thread_attr *sptr_attr, 
                                    void *ptr_dync, void *ptr_stack, kusize_t stacksize);

extern kint32_t thread_create_mempool(struct thread_attr *sptr_attr, void *base, kusize_t size);
extern void thread_release_mempool(struct thread_attr *sptr_attr);
extern void *tmalloc(size_t __size, nrt_gfp_t flags);
extern void *tcalloc(size_t __size, size_t __n, nrt_gfp_t flags);
extern void *tzalloc(size_t __size, nrt_gfp_t flags);
extern void tfree(void *__ptr);

/*!< API functions */
/*!
 * @brief	get current priority
 * @param  	sptr_attr
 * @retval 	priority
 * @note   	none
 */
__force_inline 
static inline kuint32_t thread_get_priority(struct thread_attr *sptr_attr)
{
    return sptr_attr->sgtc_param.cur_priority;
}

/*!
 * @brief	get original priority
 * @param  	sptr_attr
 * @retval 	priority
 * @note   	none
 */
__force_inline 
static inline kuint32_t thread_get_ori_priority(struct thread_attr *sptr_attr)
{
    return sptr_attr->sgtc_param.ori_priority;
}

/*!
 * @brief	get dync priority
 * @param  	sptr_attr
 * @retval 	priority
 * @note   	none
 */
__force_inline 
static inline kuint32_t thread_get_rt_priority(struct thread_attr *sptr_attr)
{
    return sptr_attr->sgtc_param.priority;
}

/*!
 * @brief   set thread priority
 * @param   sptr_thread
 * @param   priority
 * @retval  none
 * @note    none
 */
__force_inline 
static inline void thread_set_inherit_priority(struct thread_attr *sptr_attr, kuint32_t priority)
{
    struct scheduler_param *sptr_param = &sptr_attr->sgtc_param;
    sptr_param->priority = __THREAD_CHECK_PRIO(priority);
}

/*!
 * @brief	set priority
 * @param  	sptr_attr, priority
 * @retval 	none
 * @note   	none
 */
__force_inline 
static inline void thread_set_priority(struct thread_attr *sptr_attr, kuint32_t priority)
{
    struct scheduler_param *sptr_param = &sptr_attr->sgtc_param;
//  kint32_t ori_prio = sptr_param->ori_priority;

    sptr_param->ori_priority = __THREAD_CHECK_PRIO(priority);
    sptr_param->priority = sptr_param->ori_priority;
}

/*!
 * @brief	update priority
 * @param  	sptr_attr
 * @retval 	none
 * @note   	none
 */
__force_inline 
static inline void thread_sync_priority(struct thread_attr *sptr_attr)
{
    struct scheduler_param *sptr_param = &sptr_attr->sgtc_param;
    sptr_param->cur_priority = sptr_param->priority;
}

/*!
 * @brief	set time slice
 * @param  	sptr_attr, time
 * @retval 	none
 * @note   	none
 */
static inline void thread_set_time_slice(struct thread_attr *sptr_attr, kutime_t mseconds)
{
    struct time_spec sgtc_tm;
    
    msecs_to_time_spec(&sgtc_tm, mseconds);
    memcpy(&sptr_attr->sgtc_param.init_budget, &sgtc_tm, sizeof(sgtc_tm));
}

/*!
 * @brief	get time slice (to milseconds)
 * @param  	sptr_attr
 * @retval 	milseconds
 * @note   	none
 */
static inline kuint32_t thread_get_sched_msecs(struct thread_attr *sptr_attr)
{
    return time_spec_to_msecs(&sptr_attr->sgtc_param.init_budget);
}

/*!
 * @brief	set stack size
 * @param  	sptr_attr, stacksize
 * @retval 	none
 * @note   	stack-size of each thread must more than THREAD_STACK_MIN
 */
__force_inline 
static inline void thread_attr_setstacksize(struct thread_attr *sptr_attr, kusize_t stacksize)
{
    sptr_attr->stacksize = stacksize;
}

/*!
 * @brief	get stack size
 * @param  	sptr_attr
 * @retval 	stack size
 * @note   	none
 */
__force_inline 
static inline kuint32_t thread_attr_getstacksize(struct thread_attr *sptr_attr)
{
    return sptr_attr->stacksize;
}

/*!
 * @brief	get context
 * @param  	sptr_attr
 * @retval 	context structure
 * @note   	none
 */
__force_inline 
static inline struct context_regs *thread_get_context(struct thread_attr *sptr_attr)
{
    kutype_t base;

    base = sptr_attr->stack_addr + sizeof(struct context_regs);
    base = mr_align(base, 8) - sizeof(struct context_regs);
    
    return (struct context_regs *)base;
}

/*!
 * @brief	get address of stack_addr
 * @param  	sptr_attr
 * @retval 	&sptr_attr->stack_addr
 * @note   	stack = *(&sptr_attr->stack_addr) (excluding context_regs)
 */
__force_inline 
static inline kutype_t thread_get_stack(struct thread_attr *sptr_attr)
{
    return (kutype_t)(&sptr_attr->stack_addr);
}

#if 0
/*!
 * @brief	get address of current thread's stack_addr
 * @param  	none
 * @retval 	base stack address
 * @note   	current thread's stack base address must be "THREAD_STACK_ALIGN" bytes alignment !!!
 */
__force_inline 
static inline kutype_t thread_get_stack_auto(void)
{
    kutype_t stack_base = mr_get_stack();
    return mr_ralign(stack_base, THREAD_STACK_ALIGN);
}
#endif

/*!
 * @brief	set detach state
 * @param  	sptr_attr, detach state
 * @retval 	none
 * @note   	none
 */
__force_inline 
static inline void thread_attr_setdetachstate(struct thread_attr *sptr_attr, kuint32_t state)
{
    sptr_attr->detachstate = state;
}

/*!
 * @brief	get detach state
 * @param  	sptr_attr
 * @retval 	detach state
 * @note   	none
 */
__force_inline 
static inline kuint32_t thread_attr_getdetachstate(struct thread_attr *sptr_attr)
{
    return sptr_attr->detachstate;
}

/*!
 * @brief	set inherit policy
 * @param  	sptr_attr, sched
 * @retval 	none
 * @note   	none
 */
__force_inline 
static inline void thread_attr_setinheritsched(struct thread_attr *sptr_attr, kuint32_t sched)
{
    sptr_attr->inheritsched	= sched;
}

/*!
 * @brief	get inherit policy
 * @param  	sptr_attr
 * @retval 	inherit policy
 * @note   	none
 */
__force_inline 
static inline kuint32_t thread_attr_getinheritsched(struct thread_attr *sptr_attr)
{
    return sptr_attr->inheritsched;
}

/*!
 * @brief	set schedule policy
 * @param  	sptr_attr, schedule policy
 * @retval 	none
 * @note   	none
 */
__force_inline 
static inline void thread_attr_setschedpolicy(struct thread_attr *sptr_attr, kuint32_t policy)
{
    sptr_attr->schedpolicy = policy;
}

/*!
 * @brief	set schedule policy
 * @param  	sptr_attr
 * @retval 	schedule policy
 * @note   	none
 */
__force_inline 
static inline kuint32_t thread_attr_getschedpolicy(struct thread_attr *sptr_attr)
{
    return sptr_attr->schedpolicy;
}

/*!
 * @brief	set schedule parameters
 * @param  	sptr_attr, sptr_param
 * @retval 	none
 * @note   	copy param to attribute
 */
static inline void thread_attr_setschedparam(struct thread_attr *sptr_attr, struct scheduler_param *sptr_param)
{
    memcpy(&sptr_attr->sgtc_param, sptr_param, sizeof(struct scheduler_param));
}

/*!
 * @brief	get schedule parameters
 * @param  	sptr_attr, sptr_param
 * @retval 	none
 * @note   	copy param from attribute
 */
static inline void thread_attr_getschedparam(struct thread_attr *sptr_attr, struct scheduler_param *sptr_param)
{
    memcpy(sptr_param, &sptr_attr->sgtc_param, sizeof(struct scheduler_param));
}

#ifdef __cplusplus
    }
#endif

#endif /* _THREAD_H_ */
