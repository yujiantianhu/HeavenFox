/*
 * Interrupt Interface Defines
 *
 * File Name:   fwk_irq_types.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.11.14
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/irq/fwk_irq_types.h>
#include <platform/irq/fwk_irq_chip.h>
#include <platform/irq/fwk_irq_domain.h>
#include <platform/of/fwk_of.h>
#include <kernel/thread.h>
#include <kernel/sched.h>
#include <kernel/spinlock.h>

/*!< The defines */
struct fwk_irq_group
{
    tid_t tid;
    srt_atomic_t sgrt_rec;

    struct fwk_irq_action sgrt_action;
    irq_handler_t thread_fn;
};

/*!< The globals */

/*!< API function */
/*!
 * @brief  fwk_default_irq_isr
 * @param  none
 * @retval none
 * @note   default irq handler
 */
static irq_return_t fwk_default_irq_isr(void *ptrDev)
{
    return NR_IRQ_NONE;
}

/*!
 * @brief  fwk_request_irq
 * @param  none
 * @retval none
 * @note   irq register
 */
void *fwk_find_irq_action(kint32_t irq, const kchar_t *name, void *ptrDev)
{
    struct fwk_irq_desc *sprt_desc;
    struct fwk_irq_action *sprt_action;
    kint32_t retval = 0;

    sprt_desc = fwk_irq_to_desc(irq);
    if (!isValid(sprt_desc))
        return mrt_nullptr;

    spin_lock_irqsave(&sprt_desc->sgrt_lock);

    foreach_list_next_entry(sprt_action, &sprt_desc->sgrt_action, sgrt_link)
    {
        if (name)
            retval = strncmp(sprt_action->name, name, FWK_IRQ_DESC_NAME_LENTH);

        if (!retval && (sprt_action->ptrArgs == ptrDev))
        {
            spin_unlock_irqrestore(&sprt_desc->sgrt_lock);
            return sprt_action;
        }
    }

    spin_unlock_irqrestore(&sprt_desc->sgrt_lock);
    return mrt_nullptr;
}

/*!
 * @brief  irq bottom program
 * @param  args: the pointer of "struct fwk_irq_group"
 * @retval insignificant
 * @note   call thread_fn, and suspend itself
 */
static void *irq_thread(void *args)
{
    struct fwk_irq_group *sprt_grp;

    sprt_grp = (struct fwk_irq_group *)args;

    for (;;)
    {
        /*!< no additional judgement on whether thread_fn exsits */
        sprt_grp->thread_fn(sprt_grp->sgrt_action.ptrArgs);
        atomic_dec(&sprt_grp->sgrt_rec);

        if (!ATOMIC_READ(&sprt_grp->sgrt_rec))
            schedule_self_suspend();
    }

    return sprt_grp->sgrt_action.ptrArgs;
}

/*!
 * @brief  request irq action (upper and bottom handler)
 * @param  none
 * @retval none
 * @note   irq register
 */
kint32_t fwk_request_threaded_irq(kint32_t irq, irq_handler_t handler, irq_handler_t thread_fn, 
                                kuint32_t flags, const kchar_t *name, void *ptrDev)
{
    struct fwk_irq_group *sprt_grp;
    struct fwk_irq_desc *sprt_desc;
    struct fwk_irq_action *sprt_action;
    kuint32_t len = kstrlen(name);

    if ((!name) || (!ptrDev))
        return -ER_FAULT;

    if (fwk_find_irq_action(irq, name, ptrDev))
        return -ER_EXISTED;

    sprt_desc = fwk_irq_to_desc(irq);
    if (!isValid(sprt_desc))
        return -ER_NOMEM;

    sprt_grp = (struct fwk_irq_group *)kzalloc(sizeof(*sprt_grp), GFP_KERNEL);
    if (!isValid(sprt_grp))
        return -ER_NOMEM;

    sprt_action = &sprt_grp->sgrt_action;
    sprt_action->handler = handler ? handler : fwk_default_irq_isr;
    sprt_action->flags = flags;
    sprt_action->ptrArgs = ptrDev;

    if (len >= sizeof(sprt_action->name))
        goto fail;

    sprt_grp->tid = -1;
    ATOMIC_SET(&sprt_grp->sgrt_rec, 0);
    sprt_grp->thread_fn = thread_fn;

    if (thread_fn)
    {
        mrt_preempt_disable();

        sprt_grp->tid = kernel_thread_create(-1, mrt_nullptr, irq_thread, sprt_grp);
        if (sprt_grp->tid < 0)
        {
            mrt_preempt_enable();
            goto fail;
        }

        schedule_thread_suspend(sprt_grp->tid);
        thread_set_priority(thread_attr_get(sprt_grp->tid), THREAD_PROTY_IRQ);
        mrt_preempt_enable();
    }
    
    kstrcpy(sprt_action->name, name);
    
    fwk_irq_set_type(irq, flags);

    spin_lock_irqsave(&sprt_desc->sgrt_lock);
    list_head_add_tail(&sprt_desc->sgrt_action, &sprt_action->sgrt_link);
    spin_unlock_irqrestore(&sprt_desc->sgrt_lock);

    fwk_enable_irq(irq);

    return ER_NORMAL;

fail:
    kfree(sprt_grp);
    return -ER_CHECKERR;
}

/*!
 * @brief  fwk_request_irq
 * @param  none
 * @retval none
 * @note   irq register
 */
kint32_t fwk_request_irq(kint32_t irq, irq_handler_t handler, kuint32_t flags, const kchar_t *name, void *ptrDev)
{
    return fwk_request_threaded_irq(irq, handler, mrt_nullptr, flags, name, ptrDev);
}

/*!
 * @brief  fwk_free_irq
 * @param  none
 * @retval none
 * @note   irq unregister
 */
void fwk_free_irq(kint32_t irq, void *ptrDev)
{
    struct fwk_irq_group *sprt_grp;
    struct fwk_irq_desc *sprt_desc;
    struct fwk_irq_action *sprt_action;

    if ((irq < 0) || (!ptrDev))
        return;

    sprt_desc = fwk_irq_to_desc(irq);
    if (!isValid(sprt_desc))
        return;

    fwk_disable_irq(irq);

    sprt_action = fwk_find_irq_action(irq, mrt_nullptr, ptrDev);
    if (isValid(sprt_action))
    {
        sprt_grp = mrt_container_of(sprt_action, struct fwk_irq_group, sgrt_action);

        spin_lock_irqsave(&sprt_desc->sgrt_lock);
        list_head_del(&sprt_action->sgrt_link);
        spin_unlock_irqrestore(&sprt_desc->sgrt_lock);

        /*!< let irq_thread to sleep, and destroy it later (by "kernel_thread") */
        if (sprt_grp->tid >= 0)
            schedule_thread_sleep(sprt_grp->tid);

        kfree(sprt_grp);
    }
}

/*!
 * @brief   destroy the content of irq_desc
 * @param   irq
 * @retval  none
 * @note    do not used in irq handler!!!
 */
void fwk_destroy_irq_action(kint32_t irq)
{
    struct fwk_irq_group *sprt_grp;
    struct fwk_irq_desc *sprt_desc;
    struct fwk_irq_action *sprt_action, *sprt_temp;

    sprt_desc = fwk_irq_to_desc(irq);
    if (!isValid(sprt_desc))
        return;

    spin_lock_irqsave(&sprt_desc->sgrt_lock);

    foreach_list_next_entry_safe(sprt_action, sprt_temp, &sprt_desc->sgrt_action, sgrt_link)
    {
        sprt_grp = mrt_container_of(sprt_action, struct fwk_irq_group, sgrt_action);

        list_head_del(&sprt_action->sgrt_link);

        /*!< let irq_thread to sleep, and destroy it later (by "kernel_thread") */
        if (sprt_grp->tid >= 0)
            schedule_thread_sleep(sprt_grp->tid);
        kfree(sprt_grp);
    }

    spin_unlock_irqrestore(&sprt_desc->sgrt_lock);
}

/*!
 * @brief   fwk_do_irq_handler
 * @param   none
 * @retval  none
 * @note    excute irq handler
 */
void fwk_do_irq_handler(kint32_t softIrq)
{
    struct fwk_irq_group *sprt_grp;
    struct fwk_irq_desc *sprt_desc;
    struct fwk_irq_action *sprt_action;
    kint32_t retval;

    if (softIrq < 0)
        return;

    sprt_desc = fwk_irq_to_desc(softIrq);
    if (!isValid(sprt_desc))
        return;
        
    foreach_list_next_entry(sprt_action, &sprt_desc->sgrt_action, sgrt_link)
    {
        retval = sprt_action->handler ? sprt_action->handler(sprt_action->ptrArgs) : NR_IRQ_WAKE_THREAD;
        switch (retval)
        {
            case NR_IRQ_HANDLED:
                break;

            case NR_IRQ_WAKE_THREAD:
                sprt_grp = mrt_container_of(sprt_action, struct fwk_irq_group, sgrt_action);
                if (sprt_grp->tid >= 0)
                {
                    atomic_inc(&sprt_grp->sgrt_rec);
                    schedule_thread_wakeup(sprt_grp->tid);
                }

                break;

            default:
                break;
        }
    }
}

/*!
 * @brief   fwk_handle_softirq
 * @param   none
 * @retval  none
 * @note    excute irq handler
 */
void fwk_handle_softirq(kint32_t softIrq, kuint32_t event)
{
    switch (event)
    {
        case SWI_EVENT_SCHEDULED:
            print_info("trigger NR_EVENT_SCHEDULED \n");
            break;
        
        case SWI_EVENT_SYSCALL:
            print_info("trigger NR_EVENT_SYSCALL \n");
            break;

        default: break;
    }
}

/* end of file */
