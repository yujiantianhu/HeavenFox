/*
 * Soft Interrupt Interface Defines
 *
 * File Name:   irq_domain.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.05.29
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <configs/configs.h>
#include <platform/base/fwk_basic.h>
#include <platform/irq/fwk_irq_domain.h>
#include <platform/irq/fwk_irq.h>
#include <platform/irq/fwk_irq_types.h>
#include <platform/of/fwk_of.h>
#include <kernel/preempt.h>
#include <kernel/spinlock.h>
#include <kernel/sched.h>

/*!< The defines */
struct fwk_tasklet_head
{
    struct fwk_tasklet *sptr_head;
    struct fwk_tasklet **sptr_tail;
};

/*!< The globals */
static struct fwk_softirq_action sgtc_fwk_softirq_actions[NR_SOFTIRQ_NUM];
static const kchar_t *sgtc_fwk_softirq_name[NR_SOFTIRQ_NUM] __unused =
{
    [NR_SOFTIRQ_TIMER   ] = "TIMER",
    [NR_SOFTIRQ_NET_TX  ] = "NET_TX",
    [NR_SOFTIRQ_NET_RX  ] = "NET_RX",
    [NR_SOFTIRQ_TASKLET ] = "TASKLET",
    [NR_SOFTIRQ_SCHEDULE] = "SCHEDULE"
};

static DEFINE_PER_CPU_INIT(kuint32_t, g_fwk_softirq_event);
static DEFINE_PER_CPU(struct fwk_tasklet_head, sgtc_fwk_tasklet_head);

#define mr_this_cpu_softirq_events()                (*THIS_CPU_READ(g_fwk_softirq_event))
#define mr_or_this_cpu_softirq_events(nr)           do { (*THIS_CPU_READ(g_fwk_softirq_event)) |= (1UL << (nr)); } while (0)
#define mr_clr_this_cpu_softirq_events()            do { (*THIS_CPU_READ(g_fwk_softirq_event)) = 0; } while (0)

/*!< The functions */
extern void wake_up_ksoftirqd(void);

/*!< API function */
/*!
 * @brief   check if allow re-enter
 * @param   none
 * @retval  1: avaliable; 0: not avaliable
 * @note    none
 */
kbool_t fwk_softirq_avaliable(void)
{
    return (!IS_SOFTIRQ_LOCKED() && mr_this_cpu_softirq_events());
}

/*!
 * @brief   fwk_handle_softirq
 * @param   none
 * @retval  none
 * @note    this function can be called by irq_handler or thread "ksoftirqd"
 */
void fwk_handle_softirq(void)
{
    struct fwk_softirq_action *sptr_act;
    kuint32_t pending, nr;
    kutime_t end;
    kutype_t flags;

    mr_local_irq_save(flags);
    pending = mr_this_cpu_softirq_events();

    if (!pending || IS_SOFTIRQ_LOCKED())
        goto ret;

    /*!< Avoid enter again */
    local_bh_disable();
    end = jiffies + msecs_to_jiffies(2);

restart:
    mr_barrier();
    mr_clr_this_cpu_softirq_events();

    mr_local_irq_enable();

    nr = 0;
    sptr_act = &sgtc_fwk_softirq_actions[0];

    /*!< excute per event */
    while (pending && (nr < NR_SOFTIRQ_NUM))
    {
        if ((pending & 0x01) &&
            sptr_act[nr].action)
            sptr_act[nr].action(nr);

        nr++;
        pending >>= 1;
    }

    mr_local_irq_disable();

    if (IN_INTERRUPT())
    {
        /*!< new event occur */
        pending = mr_this_cpu_softirq_events();
        if (pending && (jiffies < end))
            goto restart;

        /*!< wake up "ksoftirqd" */
        if (pending)
            wake_up_ksoftirqd();
    }

    local_bh_enable();

ret:
    mr_local_irq_restore(flags);
}

/*!
 * @brief   Register softirq action
 * @param   nr: irq number (__ERT_SOFTIRQ_EVENT)
 * @param   action: irq handler
 * @retval  none
 * @note    none
 */
void fwk_open_softirq(kint32_t nr, void (*action)(kint32_t event))
{
    struct fwk_softirq_action *sptr_act = sgtc_fwk_softirq_actions;

    if (!action || 
        ((nr < 0) || (nr > NR_SOFTIRQ_NUM)) ||
        sptr_act[nr].action)
        return;

    sptr_act[nr].action = action;
}

/*!
 * @brief   trigger softirq action
 * @param   nr: irq number (__ERT_SOFTIRQ_EVENT)
 * @retval  none
 * @note    none
 */
void fwk_raise_softirq(kint32_t nr)
{
    kutype_t flags;

    mr_local_irq_save(flags);

//  SOFTIRQ_CALL(nr);
    mr_or_this_cpu_softirq_events(nr);

    mr_local_irq_restore(flags);
}

/*!
 * @brief   Tasklet callback
 * @param   nr: irq number (__ERT_SOFTIRQ_EVENT)
 * @param   args: argument
 * @retval  irq retval
 * @note    Duplicate entries are not permitted !!!
 */
static void fwk_tasklet_action(kint32_t nr)
{
    struct fwk_tasklet_head *sptr_tasklet = THIS_CPU_READ(sgtc_fwk_tasklet_head);
    struct fwk_tasklet *sptr_list;
    struct fwk_tasklet *sptr_item;

    mr_local_irq_disable();
    sptr_list = sptr_tasklet->sptr_head;
    sptr_tasklet->sptr_head = mr_nullptr;
    sptr_tasklet->sptr_tail = &sptr_tasklet->sptr_head;
    mr_local_irq_enable();

    while (sptr_list)
    {
        sptr_item = sptr_list;
        sptr_list = sptr_list->sptr_next;

        /*!< Do tasklet */
        if (atomic_get_val(&sptr_item->count))
        {
            atomic_dec(&sptr_item->count);
            sptr_item->func(sptr_item->data);
        }
    }
}

/*!
 * @brief   Tasklet initialization
 * @param   sptr_tsk: tasklet structure
 * @param   func: callback
 * @param   data: argument for func
 * @retval  none
 * @note    none
 */
void fwk_tasklet_init(struct fwk_tasklet *sptr_tsk, void (*func)(kutype_t args), kutype_t data)
{
    sptr_tsk->func = func;
    sptr_tsk->data = data;
    sptr_tsk->state = 0;
    atomic_set_val(&sptr_tsk->count, 0);
    sptr_tsk->sptr_next = mr_nullptr;
}

/*!
 * @brief   Schedule tasklet excution
 * @param   sptr_tsk: tasklet structure
 * @retval  none
 * @note    none
 */
void fwk_tasklet_schedule(struct fwk_tasklet *sptr_tsk)
{
    struct fwk_tasklet_head *sptr_list;
    kutype_t flags;

    mr_local_irq_save(flags);

    /*!< Has scheduled and does not excute */
    if (atomic_get_val(&sptr_tsk->count))
    {
        mr_local_irq_restore(flags);
        return;
    }

    sptr_list = THIS_CPU_READ(sgtc_fwk_tasklet_head);
    sptr_tsk->sptr_next = mr_nullptr;
    *sptr_list->sptr_tail = sptr_tsk;
    sptr_list->sptr_tail = &sptr_tsk->sptr_next;
    atomic_inc(&sptr_tsk->count);

    fwk_raise_softirq(NR_SOFTIRQ_TASKLET);
    mr_local_irq_restore(flags);
}

/*!
 * @brief   Remove tasklet excution
 * @param   sptr_tsk: tasklet structure
 * @retval  none
 * @note    none
 */
void fwk_tasklet_kill(struct fwk_tasklet *sptr_tsk)
{

}

/*!
 * @brief   Softirq init
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_init fwk_softirq_init(void)
{
    struct fwk_tasklet_head *sptr_list = &sgtc_fwk_tasklet_head[0];

    foreach_percpu(kuint32_t, cpuid)
    {
        sptr_list->sptr_head = mr_nullptr;
        sptr_list->sptr_tail = &sptr_list->sptr_head;

        sptr_list++;
    }

    fwk_open_softirq(NR_SOFTIRQ_TASKLET, fwk_tasklet_action);
}

/* end of file */
