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
#include <kernel/spinlock.h>

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
    "TIMER", "NET_TX", "NET_RX", "TASKLET", "SCHEDULE", "RCU"
};

static kuint32_t g_fwk_softirq_event = 0, g_fwk_softirq_count = 0;
static struct fwk_tasklet_head sgtc_fwk_tasklet_head;

/*!< The functions */
extern void wake_up_ksoftirqd(void);

/*!< API function */
/*!
 * @brief   check if allow re-enter
 * @param   none
 * @retval  g_fwk_softirq_count
 * @note    none
 */
kuint32_t fwk_softirq_avaliable(void)
{
    return g_fwk_softirq_count;
}

/*!
 * @brief   close local softirq
 * @param   g_fwk_softirq_count++
 * @retval  none
 * @note    none
 */
void local_bh_disable(void)
{
    kutype_t flags;

    mr_local_irq_save(flags);
    g_fwk_softirq_count++;
    mr_barrier();
    mr_local_irq_restore(flags);
}

/*!
 * @brief   open local softirq
 * @param   g_fwk_softirq_count--
 * @retval  none
 * @note    none
 */
void local_bh_enable(void)
{
    kutype_t flags;

    mr_local_irq_save(flags);

    if (g_fwk_softirq_count)
        g_fwk_softirq_count--;

    mr_barrier();
    mr_local_irq_restore(flags);
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
    kuint32_t pending, nr = 0;
    kutime_t end;
    kutype_t flags;

    if (!g_fwk_softirq_event || g_fwk_softirq_count)
        return;

    mr_local_irq_save(flags);

    /*!< Local irq is opened, but not allow preempting (disable scheduler) */
    if (IS_IN_INTERRUPT())
        mr_preempt_disable();

    /*!< Avoid enter again */
    g_fwk_softirq_count++;

    end = jiffies + msecs_to_jiffies(2);
    pending = g_fwk_softirq_event;

restart:
    g_fwk_softirq_event = 0;
    mr_local_irq_enable();

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

    if (IS_IN_INTERRUPT())
    {
        mr_local_irq_disable();

        /*!< new event occur */
        pending = g_fwk_softirq_event;
        if (pending && (jiffies < end))
            goto restart;

        mr_preempt_enable();

        /*!< wake up "ksoftirqd" */
        if (pending)
            wake_up_ksoftirqd();
    }

    g_fwk_softirq_count--;
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
    g_fwk_softirq_event |= mr_bit(nr);

    mr_local_irq_restore(flags);
}

/*!
 * @brief   cancel softirq action
 * @param   nr: irq number (__ERT_SOFTIRQ_EVENT)
 * @retval  none
 * @note    none
 */
void fwk_cancel_softirq(kint32_t nr)
{
    kutype_t flags;

    mr_local_irq_save(flags);

//  SOFTIRQ_CALL(nr);
    g_fwk_softirq_event &= ~mr_bit(nr);

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
    struct fwk_tasklet *sptr_list;
    struct fwk_tasklet *sptr_item;

    mr_local_irq_disable();
    sptr_list = sgtc_fwk_tasklet_head.sptr_head;
    sgtc_fwk_tasklet_head.sptr_head = mr_nullptr;
    sgtc_fwk_tasklet_head.sptr_tail = &sgtc_fwk_tasklet_head.sptr_head;
    mr_local_irq_enable();

    while (sptr_list)
    {
        sptr_item = sptr_list;
        sptr_list = sptr_list->sptr_next;

        /*!< Do tasklet */
        if (ATOMIC_READ(&sptr_item->count))
        {
            sptr_item->func(sptr_item->data);
            atomic_dec(&sptr_item->count);
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
    ATOMIC_SET(&sptr_tsk->count, 0);
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
    if (ATOMIC_READ(&sptr_tsk->count))
    {
        mr_local_irq_restore(flags);
        return;
    }

    sptr_list = &sgtc_fwk_tasklet_head;
    sptr_tsk->sptr_next = mr_nullptr;
    *sptr_list->sptr_tail = sptr_tsk;
    sptr_list->sptr_tail = &sptr_tsk->sptr_next;
    atomic_inc(&sptr_tsk->count);

    mr_local_irq_restore(flags);
    fwk_raise_softirq(NR_SOFTIRQ_TASKLET);
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
    struct fwk_tasklet_head *sptr_list = &sgtc_fwk_tasklet_head;

    sptr_list->sptr_head = mr_nullptr;
    sptr_list->sptr_tail = &sptr_list->sptr_head;

    fwk_open_softirq(NR_SOFTIRQ_TASKLET, fwk_tasklet_action);
}

/* end of file */
