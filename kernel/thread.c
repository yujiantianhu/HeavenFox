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
static kint32_t __thread_create(tid_t *ptr_id, kint32_t base, struct thread_attr *sptr_attr,
                                void *(*pfunc_start_routine) (void *), void *ptr_args, kuint32_t flags)
{
    tid_t tid;
    struct thread *sptr_thread;
    struct thread_attr *sptr_it_attr;
    kuint32_t i_start = THREAD_TID_DYNC;
    kuint32_t count = THREAD_TID_USER - THREAD_TID_DYNC;
    kint32_t retval;

    mr_preempt_disable();

    /*!< check if is user thread */
    if ((THREAD_USER & flags) == THREAD_USER)
    {
        i_start = THREAD_TID_USER;
        count	= THREAD_MAX_NUM - THREAD_TID_USER;
    }

    /*!< fixed tid */
    if ((base >= 0) && (base < THREAD_TID_DYNC))
    {
        i_start = base;
        count   = 1;
    }

    /*!< find a free tid */
    tid = get_unused_tid_from_scheduler(i_start, count);
    if (tid < 0)
        goto fail;

    sptr_it_attr = sptr_attr;

    if (!sptr_attr)
    {
        sptr_it_attr = (struct thread_attr *)kmalloc(sizeof(struct thread_attr), GFP_KERNEL);
        if (!isValid(sptr_it_attr))
            goto fail;

        /*!< initialize attr */
        if (!thread_attr_init(sptr_it_attr))
            goto fail2;
    }

    /*!< check if attr valid */
    if (!thread_attr_revise(sptr_it_attr))
        goto fail3;

    /*!< create new dynamic thread */
    sptr_thread = (struct thread *)kzalloc(sizeof(struct thread), GFP_KERNEL);
    if (!isValid(sptr_thread))
        goto fail3;

    sptr_thread->tid 			= tid;
    sptr_thread->sptr_attr 		= sptr_it_attr;
    sptr_thread->start_routine 	= pfunc_start_routine;
    sptr_thread->ptr_args		= ptr_args;

    /*!< add to ready list */
    retval = register_new_thread(sptr_thread, tid);
    if (retval < 0)
        goto fail4;

    if (ptr_id)
        *ptr_id = tid;
    
    mr_preempt_enable();
    return ER_NORMAL;

fail4:
    kfree(sptr_thread);
fail3:
    thread_attr_destroy(sptr_it_attr);
fail2:
    if (!isValid(sptr_attr))
        kfree(sptr_it_attr);
fail:
    mr_preempt_enable();
    return -ER_FAULT;
}

/*!
 * @brief	create kernel thread
 * @param  	...
 * @retval 	err code
 * @note   	none
 */
kint32_t __kernel_thread_create(tid_t *ptr_id, kint32_t base, 
                        struct thread_attr *sptr_attr, void *(*pfunc_start_routine) (void *), void *ptr_args)
{
    return __thread_create(ptr_id, base, 
                                sptr_attr, pfunc_start_routine, ptr_args, 0);
}

/*!
 * @brief	create user thread
 * @param  	...
 * @retval 	err code
 * @note   	none
 */
kint32_t __real_user_thread_create(tid_t *ptr_id, kint32_t base, 
                        struct thread_attr *sptr_attr, void *(*pfunc_start_routine) (void *), void *ptr_args)
{
    return __thread_create(ptr_id, base, 
                                sptr_attr, pfunc_start_routine, ptr_args, THREAD_USER);
}

/*!
 * @brief	create kernel thread
 * @param  	...
 * @retval 	err code
 * @note   	none
 */
tid_t kernel_thread_create(tid_t tid, struct thread_attr *sptr_attr, 
                        void *(*pfunc_start_routine) (void *), void *ptr_args)
{
    kint32_t retval;

    if (tid < 0)
    {
        tid_t new_tid;

        retval = __kernel_thread_create(&new_tid, -1, 
                                    sptr_attr, pfunc_start_routine, ptr_args);
        return retval ? -1 : new_tid;
    }

    retval = __kernel_thread_create(mr_nullptr, tid, 
                                sptr_attr, pfunc_start_routine, ptr_args);
    return retval ? -1 : tid;
}

/*!
 * @brief	create user thread
 * @param  	...
 * @retval 	err code
 * @note   	none
 */
kint32_t thread_create(tid_t *ptr_id, struct thread_attr *sptr_attr, 
                        void *(*pfunc_start_routine) (void *), void *ptr_args)
{
    return __real_user_thread_create(ptr_id, -1, 
                                sptr_attr, pfunc_start_routine, ptr_args);
}

/*!
 * @brief	create idle thread
 * @param  	...
 * @retval 	err code
 * @note   	tid is fixed to THREAD_TID_IDLE
 */
kint32_t kernel_thread_idle_create(struct thread_attr *sptr_attr, 
                                void *(*pfunc_start_routine) (void *), void *ptr_args)
{  
    return __kernel_thread_create(mr_nullptr, THREAD_TID_IDLE, 
                                sptr_attr, pfunc_start_routine, ptr_args);
}

/*!
 * @brief	create base thread
 * @param  	...
 * @retval 	err code
 * @note   	tid is fixed to THREAD_TID_BASE
 */
kint32_t kernel_thread_base_create(struct thread_attr *sptr_attr, 
                                void *(*pfunc_start_routine) (void *), void *ptr_args)
{  
    return __kernel_thread_create(mr_nullptr, THREAD_TID_BASE, 
                                sptr_attr, pfunc_start_routine, ptr_args);
}

/*!
 * @brief	create init thread
 * @param  	...
 * @retval 	err code
 * @note   	tid is fixed to THREAD_TID_INIT
 */
kint32_t kernel_thread_init_create(struct thread_attr *sptr_attr, 
                                void *(*pfunc_start_routine) (void *), void *ptr_args)
{  
    return __kernel_thread_create(mr_nullptr, THREAD_TID_INIT, 
                                sptr_attr, pfunc_start_routine, ptr_args);
}

/*!
 * @brief	destroy thread
 * @param  	tid
 * @retval 	err code
 * @note   	none
 */
kint32_t thread_destory(tid_t tid)
{
    struct thread *sptr_thread;

    sptr_thread = unregister_thread(tid);
    if (IS_ERR(sptr_thread))
        return PTR_ERR(sptr_thread);

    if (mr_nullptr == sptr_thread)
        return ER_NORMAL;

    print_info("\r\nthread \'%s\' (tid: %d) is be destroyed\r\n", sptr_thread->name, sptr_thread->tid);

    kfree(sptr_thread->sptr_attr);
    kfree(sptr_thread);

    return ER_NORMAL;
}

/*!
 * @brief	initial attribute
 * @param  	sptr_attr: attr
 * @retval 	none
 * @note   	none
 */
void *thread_attr_init(struct thread_attr *sptr_attr)
{
    void *ptr_stack;

    memset(sptr_attr, 0, sizeof(struct thread_attr));

    /*!< set default parmeters */
    /*!< detach state: join */
    sptr_attr->detachstate = THREAD_CREATE_JOINABLE;
    /*!< inherit policy: inherit */
    sptr_attr->inheritsched	= THREAD_INHERIT_SCHED;
    /*!< schedule policy: preempt */
    sptr_attr->schedpolicy = THREAD_SCHED_FIFO;

    /*!< stack: 2K */
    ptr_stack = kzalloc(THREAD_STACK_DEFAULT, GFP_KERNEL);
    if (!isValid(ptr_stack))
        return mr_nullptr;

    thread_set_stack(sptr_attr, ptr_stack, ptr_stack, THREAD_STACK_DEFAULT);
    thread_set_priority(sptr_attr, THREAD_PROTY_DEFAULT);
    thread_set_time_slice(sptr_attr, THREAD_TIME_DEFUALT);

    return (void *)sptr_attr->stack_addr;
}

/*!
 * @brief	check attribute
 * @param  	sptr_attr: attr
 * @retval 	none
 * @note   	none
 */
void *thread_attr_revise(struct thread_attr *sptr_attr)
{
    void *ptr_stack;

    if (!sptr_attr)
        return mr_nullptr;

    if (!sptr_attr->sgtc_param.sched_priority)
        thread_set_priority(sptr_attr, THREAD_PROTY_DEFAULT);
    
    if (mr_is_timespec_empty(&sptr_attr->sgtc_param.mr_sched_init_budget))
        thread_set_time_slice(sptr_attr, THREAD_TIME_DEFUALT);

    if (!sptr_attr->stack_addr)
    {
        /*!< stack: 128bytes */
        ptr_stack = kzalloc(THREAD_STACK_DEFAULT, GFP_KERNEL);
        if (!isValid(ptr_stack))
            return mr_nullptr;		

        thread_set_stack(sptr_attr, ptr_stack, ptr_stack, THREAD_STACK_DEFAULT);
    }

    return (void *)sptr_attr->stack_addr;
}

/*!
 * @brief	destroy attribute
 * @param  	sptr_attr: attr
 * @retval 	none
 * @note   	none
 */
void thread_attr_destroy(struct thread_attr *sptr_attr)
{
    if (!sptr_attr)
        return;

    if (sptr_attr->ptr_stack_start)
        kfree(sptr_attr->ptr_stack_start);
    
    memset(sptr_attr, 0, sizeof(struct thread_attr));
}

/*!
 * @brief	get attribute
 * @param  	tid
 * @retval 	sptr_attr
 * @note   	none
 */
struct thread_attr *thread_attr_get(tid_t tid)
{
    if (mr_likely((tid >= 0) && (tid < THREAD_MAX_NUM)))
    {
        struct thread *sptr_th = mr_tid_handle(tid);
        return sptr_th->sptr_attr;
    }

    return mr_nullptr;
}

/*!
 * @brief	set stack
 * @param  	sptr_attr: thread attibute
 * @param   ptr_dync: stack which allocated by dynamic (kmalloc/malloc)
 * @param   ptr_stack: stack base (low address)
 * @param   stacksize: total size of stack
 * @retval 	stack address (top of stack)
 * @note   	if stack is allocated by mempool, ptr_dync should set to be ptr_stack, 
 *          so that it can be released by sptr_attr->ptr_stack_start
 */
void *thread_set_stack(struct thread_attr *sptr_attr, 
                                    void *ptr_dync, void *ptr_stack, kusize_t stacksize)
{
    struct scheduler_context_regs *sptr_regs;

    if (!isValid(ptr_stack) || (stacksize < THREAD_STACK_MIN))
        return mr_nullptr;

    /*!< check: ptr_dync just should be NULL or ptr_stack */
    if (ptr_dync && (ptr_dync != ptr_stack))
        return mr_nullptr;

    if (isValid(sptr_attr->ptr_stack_start))
        kfree(sptr_attr->ptr_stack_start);

    /*!< 
     * if the stack is defined in a static storage area, ptr_dync should be NULL; 
     * Otherwise, the address of ptr_stack should be passed in 
     */
    sptr_attr->ptr_stack_start = ptr_dync;

    /*!< pointer to stack top with 8 bytes alignment */
    sptr_attr->stack_addr = (kutype_t)((kuint8_t *)ptr_stack + stacksize);
    sptr_attr->stack_addr = mr_ralign(sptr_attr->stack_addr - 16, 8);
    sptr_attr->stack_addr -= sizeof(struct scheduler_context_regs);
    sptr_attr->stack_addr = mr_ralign(sptr_attr->stack_addr, 8);
    sptr_attr->stacksize = stacksize;

    sptr_regs = thread_get_context(sptr_attr);
    memset(sptr_regs, 0, sizeof(struct scheduler_context_regs));

    return (void *)sptr_attr->stack_addr;
}

/*!
 * @brief   create memory pool
 * @param   sptr_attr, base, size
 * @retval  none
 * @note    thread memory block create
 */
kint32_t thread_create_mempool(struct thread_attr *sptr_attr, void *base, kusize_t size)
{
    if (!isValid(base))
        return PTR_ERR(base);

    /*!< must 8-byte alignment */
    if (((kuaddr_t)base) & 0x07)
        return -ER_NOTALIGN;
    
    return memory_block_create(&sptr_attr->sgtc_pool, (kuaddr_t)base, size);
}

/*!
 * @brief   destroy memory
 * @param   sptr_attr
 * @retval  none
 * @note    thread memory block destroy
 */
void thread_release_mempool(struct thread_attr *sptr_attr)
{
    if (sptr_attr->sgtc_pool.sptr_mem)
        memory_block_destroy(&sptr_attr->sgtc_pool);
}

/*!
 * @brief   tmalloc
 * @param   __size
 * @retval  none
 * @note    thread memory pool allocate
 */
void *tmalloc(size_t __size, nrt_gfp_t flags)
{
    struct thread *sptr_thread = mr_current;
    struct mem_info *sptr_info;
    void *p = mr_nullptr;

    if (!sptr_thread)
        goto END;

    if (GFP_KERNEL != (flags & GFP_KERNEL))
        goto END;

    sptr_info = &sptr_thread->sptr_attr->sgtc_pool;
    if (sptr_info->alloc)
    {
        p = sptr_info->alloc(sptr_info, __size, mr_nullptr);
        if (!isValid(p))
            return mr_nullptr;

        if (flags & NR_KMEM_ZERO)
            kmemzero(p, __size);
    }

END:
    return p;
}

/*!
 * @brief   tcalloc
 * @param   __size, __n
 * @retval  none
 * @note    thread memory pool allocate (array)
 */
void *tcalloc(size_t __size, size_t __n, nrt_gfp_t flags)
{
    return tmalloc(__size * __n, flags);
}

/*!
 * @brief   tzalloc
 * @param   __size
 * @retval  none
 * @note    thread memory pool allocate, and reset automatically
 */
void *tzalloc(size_t __size, nrt_gfp_t flags)
{
    return tmalloc(__size, flags | GFP_ZERO);
}

/*!
 * @brief   tfree
 * @param   __ptr
 * @retval  none
 * @note    thread memory pool free
 */
void tfree(void *__ptr)
{
    struct thread *sptr_thread = mr_current;
    struct mem_info *sptr_info;

    sptr_info = &sptr_thread->sptr_attr->sgtc_pool;
    if ((__ptr <  (void *)sptr_info->base) ||
        (__ptr >= (void *)(sptr_info->base + sptr_info->lenth)))
        return;

    if (sptr_info->free)
        sptr_info->free(sptr_info, __ptr);
}

/*!< end of file */
