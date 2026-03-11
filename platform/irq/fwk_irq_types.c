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
#include <platform/base/fwk_basic.h>
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
    srt_atomic_t sgtc_rec;

    kint32_t irq;
    struct fwk_irq_action sgtc_action;
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
static irq_return_t fwk_default_irq_isr(kint32_t irq, void *ptrDev)
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
    struct fwk_irq_desc *sptr_desc;
    struct fwk_irq_action *sptr_action;
    kutype_t flags;
    kint32_t retval = 0;

    sptr_desc = fwk_irq_to_desc(irq);
    if (!isValid(sptr_desc))
        return mr_nullptr;

    spin_lock_irqsave(&sptr_desc->sgtc_lock, &flags);

    foreach_list_next_entry(sptr_action, &sptr_desc->sgtc_action, sgtc_link)
    {
        if (name)
            retval = kstrncmp(sptr_action->name, name, FWK_IRQ_DESC_NAME_LENTH);

        if (!retval && (sptr_action->ptrArgs == ptrDev))
        {
            spin_unlock_irqrestore(&sptr_desc->sgtc_lock, flags);
            return sptr_action;
        }
    }

    spin_unlock_irqrestore(&sptr_desc->sgtc_lock, flags);
    return mr_nullptr;
}

/*!
 * @brief  irq bottom program
 * @param  args: the pointer of "struct fwk_irq_group"
 * @retval insignificant
 * @note   call thread_fn, and suspend itself
 */
static void *irq_thread(void *args)
{
    struct fwk_irq_group *sptr_grp;

    sptr_grp = (struct fwk_irq_group *)args;

    for (;;)
    {
        /*!
         * @note Init to 1
         * even if the upper half irq handler is triggered multiple times, 
         * the bottom half is excuted only once
         */
        atomic_set_val(&sptr_grp->sgtc_rec, 1);

        /*!
         * @note
         * If upper half irq handler is triggered during the excution of thread_fn, 
         * then thread_fn needs to excute again to response event 
         */
        while (true)
        {
            atomic_dec(&sptr_grp->sgtc_rec);

            /*!< no additional judgement on whether thread_fn exsits */
            sptr_grp->thread_fn(sptr_grp->irq, sptr_grp->sgtc_action.ptrArgs);
            if (!atomic_get_val(&sptr_grp->sgtc_rec))
                break;
        }

        schedule_self_suspend();
    }

    return sptr_grp->sgtc_action.ptrArgs;
}

/*!
 * @brief  request irq action (upper and bottom handler)
 * @param  none
 * @retval none
 * @note   irq register
 */
kint32_t __fwk_request_threaded_irq(kint32_t irq, irq_handler_t handler, irq_handler_t thread_fn, 
                                kuint32_t flags, kuint32_t cpu_affinity, const kchar_t *name, void *args)
{
    struct fwk_irq_group *sptr_grp;
    struct fwk_irq_desc *sptr_desc;
    struct fwk_irq_action *sptr_action;
    kuint32_t len = kstrlen(name);
    kutype_t lock_flags;

    if ((!name) || (!args))
        return -ER_FAULT;

    /*!< same name && same args */
    if (fwk_find_irq_action(irq, name, args))
        return -ER_EXISTED;

    sptr_desc = fwk_irq_to_desc(irq);
    if (!isValid(sptr_desc))
        return -ER_NOMEM;

    sptr_grp = (struct fwk_irq_group *)kzalloc(sizeof(*sptr_grp), GFP_KERNEL);
    if (!isValid(sptr_grp))
        return -ER_NOMEM;

    sptr_action = &sptr_grp->sgtc_action;
    sptr_action->handler = handler ? handler : fwk_default_irq_isr;
    sptr_action->flags = flags;
    sptr_action->cpu_affinity = cpu_affinity ? cpu_affinity : CPU_AFFINITY_DEFAULT;
    sptr_action->ptrArgs = args;

    if (len >= sizeof(sptr_action->name))
        goto fail;

    sptr_grp->tid = -1;
    atomic_set_val(&sptr_grp->sgtc_rec, 0);
    sptr_grp->irq = irq;
    sptr_grp->thread_fn = thread_fn;

    if (thread_fn)
    {
        kchar_t name[24];
        struct thread_attr sgtc_attr;

        memset(&sgtc_attr, 0, sizeof(sgtc_attr));
        thread_set_cpuaffinity(&sgtc_attr, sptr_action->cpu_affinity);

        mr_preempt_disable();

        sptr_grp->tid = kernel_thread_create(-1, &sgtc_attr, irq_thread, sptr_grp);
        if (sptr_grp->tid < 0)
        {
            mr_preempt_enable();
            goto fail;
        }

        schedule_thread_suspend(sptr_grp->tid);

        sprintk(name, "threaded_irq-%d", irq);
        thread_set_name(sptr_grp->tid, (const kchar_t *)name);
        thread_set_priority(thread_attr_get(sptr_grp->tid), THREAD_PROTY_IRQ);
        mr_preempt_enable();
    }
    
    kstrcpy(sptr_action->name, name);
    fwk_irq_set_type(irq, flags);

    spin_lock_irqsave(&sptr_desc->sgtc_lock, &lock_flags);
    list_head_add_tail(&sptr_desc->sgtc_action, &sptr_action->sgtc_link);
    spin_unlock_irqrestore(&sptr_desc->sgtc_lock, lock_flags);

    fwk_enable_irq(irq);

    return ER_NORMAL;

fail:
    kfree(sptr_grp);
    return -ER_CHECKERR;
}

/*!
 * @brief  fwk_free_irq
 * @param  none
 * @retval none
 * @note   irq unregister
 */
void __fwk_free_irq(kint32_t irq, void *args)
{
    struct fwk_irq_group *sptr_grp;
    struct fwk_irq_desc *sptr_desc;
    struct fwk_irq_action *sptr_action;
    kutype_t flags;

    if ((irq < 0) || (!args))
        return;

    sptr_desc = fwk_irq_to_desc(irq);
    if (!isValid(sptr_desc))
        return;

    fwk_disable_irq(irq);

    sptr_action = fwk_find_irq_action(irq, mr_nullptr, args);
    if (isValid(sptr_action))
    {
        sptr_grp = mr_container_of(sptr_action, struct fwk_irq_group, sgtc_action);

        spin_lock_irqsave(&sptr_desc->sgtc_lock, &flags);
        list_head_del(&sptr_action->sgtc_link);
        spin_unlock_irqrestore(&sptr_desc->sgtc_lock, flags);

        /*!< let irq_thread to sleep, and destroy it later (by "kernel_thread") */
        if (sptr_grp->tid >= 0)
            schedule_thread_sleep(sptr_grp->tid);

        kfree(sptr_grp);
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
    struct fwk_irq_group *sptr_grp;
    struct fwk_irq_desc *sptr_desc;
    struct fwk_irq_action *sptr_action, *sptr_temp;
    kutype_t flags;

    sptr_desc = fwk_irq_to_desc(irq);
    if (!isValid(sptr_desc))
        return;

    spin_lock_irqsave(&sptr_desc->sgtc_lock, &flags);

    foreach_list_next_entry_safe(sptr_action, sptr_temp, &sptr_desc->sgtc_action, sgtc_link)
    {
        sptr_grp = mr_container_of(sptr_action, struct fwk_irq_group, sgtc_action);

        list_head_del(&sptr_action->sgtc_link);

        /*!< let irq_thread to sleep, and destroy it later (by "kernel_thread") */
        if (sptr_grp->tid >= 0)
            schedule_thread_sleep(sptr_grp->tid);
        kfree(sptr_grp);
    }

    spin_unlock_irqrestore(&sptr_desc->sgtc_lock, flags);
}

/*!
 * @brief   fwk_do_irq_handler
 * @param   none
 * @retval  none
 * @note    excute irq handler
 */
void fwk_do_irq_handler(kint32_t softIrq)
{
    struct fwk_irq_group *sptr_grp;
    struct fwk_irq_desc *sptr_desc;
    struct fwk_irq_action *sptr_action;
    kuint32_t cpuid = get_cpu_id();
    kint32_t retval;

    if (softIrq < 0)
        return;

    sptr_desc = fwk_irq_to_desc(softIrq);
    if (!isValid(sptr_desc))
        return;
        
    foreach_list_next_entry(sptr_action, &sptr_desc->sgtc_action, sgtc_link)
    {
        /*!< cpu_affinity is used for PPI usually, do not recommend to use it for SPI */
        if (0 == (sptr_action->cpu_affinity & CPU_AFFINITY_SINGEL(cpuid)))
            continue;

        retval = sptr_action->handler ? sptr_action->handler(softIrq, sptr_action->ptrArgs) : NR_IRQ_WAKE_THREAD;
        switch (retval)
        {
            case NR_IRQ_HANDLED:
                break;

            case NR_IRQ_WAKE_THREAD:
                sptr_grp = mr_container_of(sptr_action, struct fwk_irq_group, sgtc_action);
                if (sptr_grp->tid >= 0)
                {
                    atomic_inc(&sptr_grp->sgtc_rec);
                    schedule_thread_wakeup(sptr_grp->tid);
                }

                break;

            default:
                break;
        }
    }
}

/* end of file */
