/*
 * Simple Delay General Function
 *
 * File Name:   delay.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.10.09
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <configs/configs.h>
#include <common/time.h>
#include <common/mem_manage.h>
#include <platform/irq/fwk_irq_types.h>
#include <kernel/spinlock.h>

/*!< The defines */
#define DELAY_SIMPLE_COUNTER_PER_MS								(0x7ff)
#define DELAY_SIMPLE_COUNTER_PER_S								(DELAY_SIMPLE_COUNTER_PER_MS * 1000)
#define DELAY_SIMPLE_COUNTER_PER_US								(DELAY_SIMPLE_COUNTER_PER_MS / 1000)

struct ktime_tick
{
    struct list_head sgtc_list;
    struct spin_lock sgtc_lock;

    struct timer_list *sptr_first;
};

/*!< The globals */
/*!< ---------------------------------------------------------- */
kutime_t g_delay_freq_cnt = CONFIG_CPU_FREQ / 100U;
struct time_clock sgtc_systime_clock;

/*!< ---------------------------------------------------------- */
volatile kutime_t jiffies = JIFFIES_INITVAL;
volatile kuint64_t jiffies_all = 0;
volatile kutime_t jiffies_out = 0;

struct ktime_manage sgtc_ksystick_manage =
{
    .freq = FREQ_INIT_VAL,
    .is_up = true,
};

struct ktime_manage sgtc_khrtime_manage =
{
    .freq = FREQ_INIT_VAL,
    .is_up = true,
};

/*!< ---------------------------------------------------------- */
static struct ktime_tick sgtc_ktime_jiffies;

static struct ktime_tick sgtc_ktime_htick;
static struct ktime_tick sgtc_ktime_pending;
static struct ktime_tick sgtc_ktime_inactive;

static struct ktime_tick *sgtr_ktime_lists[] =
{
    [NR_KTIMER_COUNTING] = &sgtc_ktime_htick,
    [NR_KTIMER_PENDING ] = &sgtc_ktime_pending,
    [NR_KTIMER_INACTIVE] = &sgtc_ktime_inactive,
};

/*!< API function */
/*!
 * @brief   Read current systick
 * @param   none
 * @retval  tick
 * @note    get the time register's current value
 */
__weak kutime_t ktime_systick(void)
{
    volatile kutime_t *time_cnt = SYSTICK_PTR->count;
    return time_cnt ? (IS_SYSTICK_UPINC() ? (*time_cnt) : (SYSTICK_MAX - (*time_cnt))) : 0;
}

/*!
 * @brief   start hrtimer
 * @param   none
 * @retval  none
 * @note    open compare interrupt
 */
__weak void khrtime_event_enable(void)
{

}

/*!
 * @brief   stop hrtimer
 * @param   none
 * @retval  none
 * @note    close compare interrupt
 */
__weak void khrtime_event_disable(void)
{

}

/*!
 * @brief   Read high time current tick
 * @param   none
 * @retval  tick
 * @note    get the time register's current value
 */
__weak khrtime_t ktime_hrtick(void)
{
    struct ktime_manage *sptr_tmn = HRTIMER_PTR;
    volatile kutime_t *time_cnt1 = sptr_tmn->count;
    volatile kutime_t *time_cnt2 = sptr_tmn->count2;

    if (mr_unlikely(!time_cnt1))
        return 0;

    if (time_cnt2)
    {
        kutime_t tick_l, tick_h;

        do {
            tick_h = *time_cnt2;
            tick_l = *time_cnt1;

        } while (tick_h != (*time_cnt2));

        return ((((khrtime_t)tick_h) << 32ULL) | tick_l);
    }

    return IS_HRTIMER_UPINC() ? (*time_cnt1) : (HRTIMER_MAX - (*time_cnt1));
}

/*!
 * @brief   Read high time total tick
 * @param   none
 * @retval  tick
 * @note    get the time register's current value
 */
__weak khrtime_t khrtime_ticks(void)
{
    struct ktime_manage *sptr_tmn = HRTIMER_PTR;
    volatile kutime_t *time_cnt1 = sptr_tmn->count;
    volatile kutime_t *time_cnt2 = sptr_tmn->count2;
    kutime_t tick_l, tick_h;
    kutime_t over_count;

    if (mr_unlikely(!time_cnt1))
        return 0;

    if (time_cnt2)
    {
        do {
            tick_h = *time_cnt2;
            tick_l = *time_cnt1;

        } while (tick_h != (*time_cnt2));

        return ((((khrtime_t)tick_h) << 32ULL) | tick_l);
    }

    /*!< Check if counter is over max, and return over count */
    over_count = khrtime_check_overcnt();
    tick_l = IS_HRTIMER_UPINC() ? (*time_cnt1) : (HRTIMER_MAX - (*time_cnt1));

    return (over_count * (khrtime_t)HRTIMER_MAX) + tick_l;
}

/*!
 * @brief   Check if hrtime counter out
 * @param   none
 * @retval  none
 * @note    true or false
 */
__weak kuint32_t khrtime_check_overcnt(void)
{
    return false;
}

/*!
 * @brief   Parse to time_val
 * @param   sptr_tval
 * @retval  none
 * @note    Convert total ticks to secs.usecs
 */
__weak void ktime_to_spec(struct time_val *sptr_tval)
{
    if (mr_unlikely(HRTIMER_FREQ == FREQ_INIT_VAL))
        memset(sptr_tval, 0, sizeof(*sptr_tval));
    else
    {
        khrtime_t ticks = khrtime_ticks();

        sptr_tval->tv_sec = HRTICK_TO_SEC(ticks);
        sptr_tval->tv_usec = HRTICK_TO_USEC(ticks - SEC_TO_HRTICK(sptr_tval->tv_sec));
    }
}

/*!
 * @brief   initialize delay freq cnt (unit: Hz)
 * @param   _O0/1/2_divider: divider of O0/1/2 optimize class
 * @retval  none
 * @note    CONFIG_CPU_FREQ / divider (divider may be only a test value)
 */
void init_delay_freq(kutime_t _O0_divider, kutime_t _O1_divider, kutime_t _O2_divider)
{
#if (CONFIG_OPTIMIZE_CLASS == 0)
    g_delay_freq_cnt = CONFIG_CPU_FREQ / _O0_divider;
#elif (CONFIG_OPTIMIZE_CLASS == 1)
    g_delay_freq_cnt = CONFIG_CPU_FREQ / _O1_divider;
#else
    g_delay_freq_cnt = CONFIG_CPU_FREQ / _O2_divider;
#endif
}

/*!
 * @brief   delay (unit: s)
 * @param   seconds
 * @retval  none
 * @note    none
 */
void delay(kuint32_t seconds)
{
    if (HRTIMER_PTR->count)
    {
        khrtime_t target_tick = khrtime_ticks() + SEC_TO_HRTICK(seconds);
        while (target_tick > khrtime_ticks())
            mr_nop();
    }
    else
    {
        /*!< Simply delay. Time will be greater than milseconds if IRQ is triggered during "ticks--" */
        kuint64_t ticks = seconds * g_delay_freq_cnt;
        while (ticks--)
            mr_nop();
    }
}

/*!
 * @brief   delay (unit: ms)
 * @param   milseconds
 * @retval  none
 * @note    none
 */
void mdelay(kuint32_t milseconds)
{
    if (HRTIMER_PTR->count)
    {
        khrtime_t target_tick = khrtime_ticks() + MSEC_TO_HRTICK(milseconds);
        while (target_tick > khrtime_ticks())
            mr_nop();
    }
    else
    {
        /*!< Simply delay. Time will be greater than milseconds if IRQ is triggered during "ticks--" */
        kuint64_t ticks = milseconds * (g_delay_freq_cnt / 1000U);
        while (ticks--)
            mr_nop();
    }
}

/*!
 * @brief   delay (unit: us)
 * @param   useconds
 * @retval  none
 * @note    none
 */
void udelay(kuint32_t useconds)
{
    if (HRTIMER_PTR->count)
    {
        khrtime_t target_tick = khrtime_ticks() + USEC_TO_HRTICK(useconds);
        while (target_tick > khrtime_ticks())
            mr_nop();
    }
    else
    {
        /*!< Simply delay. Time will be greater than milseconds if IRQ is triggered during "ticks--" */
        kuint64_t ticks = useconds * (g_delay_freq_cnt / 1000000U);
        while (ticks--)
            mr_nop();
    }
}

/*!
 * @brief   total ms ===> y-m-d h:m:s
 * @param   milseconds
 * @retval  none
 * @note    none
 */
void msecs_to_timeclock(struct time_clock *sptr_tclk, kutype_t milseconds)
{
    kuint32_t temp;

    sptr_tclk->milsecond = udiv_remainder(milseconds, 1000);
    temp = udiv_integer(milseconds, 1000);
 
    sptr_tclk->second = udiv_remainder(temp, 60);
    temp = udiv_integer(temp, 60);

    sptr_tclk->minute = udiv_remainder(temp, 60);
    temp = udiv_integer(temp, 60);

    sptr_tclk->hour = udiv_remainder(temp, 60);
    temp = udiv_integer(temp, 60);

    sptr_tclk->day = udiv_remainder(temp, 24);
    temp = udiv_integer(temp, 24);

    sptr_tclk->month = udiv_remainder(temp, 30);
    temp = udiv_integer(temp, 30);

    sptr_tclk->year = udiv_remainder(temp, 12);
}

/*!
 * @brief   initial timer
 * @param   sptr_timer: timer
 * @param	entry: timeout function
 * @param	data: argument for timeout function
 * @retval  none
 * @note    none
 */
void setup_timer(struct timer_list *sptr_timer, void (*entry)(kuint32_t), kuint32_t data)
{
    if (!isValid(sptr_timer))
        return;

    mr_setup_timer(sptr_timer, entry, data);
}

/*!
 * @brief   add timer to global list
 * @param   sptr_timer
 * @retval  none
 * @note    systick interrupt will traverses the global list
 */
void add_timer(struct timer_list *sptr_timer)
{
    struct ktime_tick *sptr_list;
    struct spin_lock *sptr_lock;

    if ((!isValid(sptr_timer)) || 
        (!sptr_timer->expires))
        return;

    sptr_list = &sgtc_ktime_jiffies;
    sptr_lock = &sptr_list->sgtc_lock;

    spin_lock_irqsave(sptr_lock);
    list_head_add_tail(&sptr_list->sgtc_list, &sptr_timer->sgtc_link);
    spin_unlock_irqrestore(sptr_lock);
}

/*!
 * @brief   delete timer from global list
 * @param   sptr_timer
 * @retval  none
 * @note    none
 */
void del_timer(struct timer_list *sptr_timer)
{
    struct ktime_tick *sptr_list;
    struct spin_lock *sptr_lock;

    if (!isValid(sptr_timer))
        return;

    sptr_list = &sgtc_ktime_jiffies;
    sptr_lock = &sptr_list->sgtc_lock;

    spin_lock_irqsave(sptr_lock);
    list_head_del(&sptr_timer->sgtc_link);
    spin_unlock_irqrestore(sptr_lock);
}

/*!
 * @brief   find timer from global list
 * @param   sptr_timer
 * @retval  none
 * @note    1: found; 0: not found
 */
kbool_t find_timer(struct timer_list *sptr_timer)
{
    struct ktime_tick *sptr_list;
    struct timer_list *sptr_any;

    sptr_list = &sgtc_ktime_jiffies;

    foreach_list_next_entry(sptr_any, &sptr_list->sgtc_list, sgtc_link)
    {
        if (sptr_timer == sptr_any)
            return true;
    }

    return false;
}

/*!
 * @brief   modilfy timer period
 * @param   sptr_timer
 * @param	expires: period
 * @retval  none
 * @note    if timer has not been added to list, add it right away
 */
void mod_timer(struct timer_list *sptr_timer, kutime_t expires)
{
    if (!isValid(sptr_timer))
        return;

    sptr_timer->expires = get_safe_expires(expires);
    
#if 0
    if (!find_timer(sptr_timer)) {
#else
    if (mr_list_empty(&sptr_timer->sgtc_link)) {
#endif
        add_timer(sptr_timer);
    }
}

/*!
 * @brief   timer handler
 * @param   none
 * @param	none
 * @retval  none
 * @note    called by systick interrupt
 */
void do_timer_event(void)
{
    struct ktime_tick *sptr_list;
    struct timer_list *sptr_timer, *sptr_temp;
    kutime_t expires_bak;

    sptr_list = &sgtc_ktime_jiffies;
    foreach_list_next_entry_safe(sptr_timer, sptr_temp, &sptr_list->sgtc_list, sgtc_link)
    {
        if (!sptr_timer->expires || 
            !sptr_timer->entry)
            continue;

        if (mr_time_after_eq(jiffies, sptr_timer->expires))
        {
            expires_bak = sptr_timer->expires;
            sptr_timer->entry(sptr_timer->data);
            
            if (expires_bak == sptr_timer->expires)
                del_timer(sptr_timer);
        }
    }
}

/*!
 * @brief   load compare value to hardware
 * @param   expires (unit: tick)
 * @retval  none
 * @note    called by systick interrupt
 */
__weak void khrtime_reload_cnt(khrtime_t expires)
{
}

/*!< get timer_list with the smallest expires */
#define HRTIMER_SELECT(sptr_cur, sptr_new)  \
    ((sptr_cur) ? ((sptr_cur)->expires < (sptr_new)->expires ? (sptr_cur) : (sptr_new)) : (sptr_new))

/*!
 * @brief   initial high timer
 * @param   sptr_timer: timer
 * @param	entry: timeout function
 * @param	data: argument for timeout function
 * @retval  none
 * @note    none
 */
void setup_hrtimer(struct hrtimer_list *sptr_timer, void (*entry)(kuint32_t), kuint32_t data)
{
    if (!sptr_timer)
        return;

    mr_setup_hrtimer(sptr_timer, entry, data);
}

/*!
 * @brief   compare and select the first event will be resolved
 * @param   sptr_timer
 * @retval  none
 * @note    load expires to hardware
 */
void __load_hrtimer(struct hrtimer_list *sptr_timer, struct hrtimer_list **sptr_first)
{
    /*!< No event exists, using for the first time. Start hrtimer before loading cnt */
    if (!(*sptr_first))
        khrtime_event_enable();

    if (!(*sptr_first) ||
        (sptr_timer->expires < (*sptr_first)->expires))
    {
        *sptr_first = sptr_timer;
        khrtime_reload_cnt(sptr_timer->expires);
    }
}

/*!
 * @brief   add timer to hight time list
 * @param   sptr_timer
 * @retval  none
 * @note    systick interrupt will traverses the hight time list
 */
void add_hrtimer(struct hrtimer_list *sptr_timer)
{
    struct ktime_tick *sptr_list;
    struct spin_lock *sptr_lock;

    if ((!sptr_timer) || 
        (!sptr_timer->expires) ||
        (!sptr_timer->entry) ||
        (sptr_timer->status != NR_KTIMER_COUNTING))
        return;

    sptr_list = &sgtc_ktime_htick;
    sptr_lock = &sptr_list->sgtc_lock;

    spin_lock_irqsave(sptr_lock);
    __load_hrtimer(sptr_timer, (struct hrtimer_list **)&sptr_list->sptr_first);
    list_head_add_tail(&sptr_list->sgtc_list, &sptr_timer->sgtc_link);
    spin_unlock_irqrestore(sptr_lock);
}

/*!
 * @brief   delete timer from hight time list
 * @param   sptr_timer
 * @retval  none
 * @note    none
 */
void del_hrtimer(struct hrtimer_list *sptr_timer)
{
    struct ktime_tick *sptr_list;
    struct spin_lock *sptr_lock;
    struct hrtimer_list *sptr_per;
    struct hrtimer_list *sptr_next = mr_nullptr;

    if (mr_unlikely(!sptr_timer))
        return;

    sptr_list = &sgtc_ktime_htick;
    sptr_lock = &(sgtr_ktime_lists[sptr_timer->status]->sgtc_lock);

    spin_lock_irqsave(sptr_lock);
    list_head_del(&sptr_timer->sgtc_link);

    if (sptr_timer->status == NR_KTIMER_COUNTING)
    {
        if (mr_unlikely((struct timer_list *)sptr_timer == sptr_list->sptr_first))
        {
            foreach_list_next_entry(sptr_per, &sptr_list->sgtc_list, sgtc_link)
                sptr_next = HRTIMER_SELECT(sptr_next, sptr_per);

            sptr_list->sptr_first = (struct timer_list *)sptr_next;

            if (sptr_next)
                khrtime_reload_cnt(sptr_next->expires);
            else
                khrtime_event_disable();
        }
    }

    spin_unlock_irqrestore(sptr_lock);
}

/*!
 * @brief   modilfy timer period
 * @param   sptr_timer
 * @param	expires: period
 * @retval  none
 * @note    if timer has not been added to list, add it right away
 */
void mod_hrtimer(struct hrtimer_list *sptr_timer, khrtime_t expires)
{
    if (mr_unlikely(!sptr_timer))
        return;

    sptr_timer->expires = expires;

    if (mr_list_empty(&sptr_timer->sgtc_link))
        add_hrtimer(sptr_timer);
}

/*!
 * @brief   timer counter is over, check all expires
 * @param   none
 * @retval  none
 * @note    called by hrtimer interrupt
 */
void check_hrtimer(void)
{
}

/*!
 * @brief   tick handler (softirq)
 * @param   none
 * @param	none
 * @retval  none
 * @note    called by softirq (bottom isr)
 */
void do_hrtimer_action(kint32_t event)
{
    struct hrtimer_list *sptr_timer, *sptr_temp;
    struct hrtimer_list *sptr_next = mr_nullptr;
    struct spin_lock *sptr_lock;
    khrtime_t expires_bak;
    DECLARE_LIST_HEAD(sgtc_active);
    DECLARE_LIST_HEAD(sgtc_pending);
    DECLARE_LIST_HEAD(sgtc_inactive);
    
    sptr_lock = &sgtc_ktime_pending.sgtc_lock;
    spin_lock_irqsave(sptr_lock);
    list_head_splice_init(&sgtc_pending, &sgtc_ktime_pending.sgtc_list);
    spin_unlock_irqrestore(sptr_lock);

    foreach_list_next_entry_safe(sptr_timer, sptr_temp, &sgtc_pending, sgtc_link)
    {
        expires_bak = sptr_timer->expires;
        sptr_timer->entry(sptr_timer->data);

        list_head_del(&sptr_timer->sgtc_link);
        if (sptr_timer->expires <= expires_bak)
        {
            /*!< One-shot event, move to inactive list (thread context will call "del_hrtimer" to detach it) */
            sptr_timer->status = NR_KTIMER_INACTIVE;
            list_head_add_tail(&sgtc_inactive, &sptr_timer->sgtc_link);
        }
        else
        {
            /*!< Period event, select event with the smallest expires, and move to active list */
            sptr_next = HRTIMER_SELECT(sptr_next, sptr_timer);
            sptr_timer->status = NR_KTIMER_COUNTING;
            list_head_add_tail(&sgtc_active, &sptr_timer->sgtc_link);
        }
    }

    /*!< Add to global active list */
    if (sptr_next)
    {
        struct ktime_tick *sptr_list = &sgtc_ktime_htick;

        spin_lock_irqsave(&sptr_list->sgtc_lock);
        __load_hrtimer(sptr_next, (struct hrtimer_list **)&sptr_list->sptr_first);
        list_head_split_tail(&sptr_list->sgtc_list, &sgtc_active);
        spin_unlock_irqrestore(&sptr_list->sgtc_lock);
    }

    /*!< Add to global inactive list */
    if (!mr_list_empty(&sgtc_inactive))
    {
        struct ktime_tick *sptr_cast = &sgtc_ktime_inactive;

        spin_lock_irqsave(&sptr_cast->sgtc_lock);
        list_head_split_tail(&sptr_cast->sgtc_list, &sgtc_inactive);
        spin_unlock_irqrestore(&sptr_cast->sgtc_lock);
    }
}

/*!
 * @brief   tick handler
 * @param   none
 * @param	none
 * @retval  none
 * @note    called by hrtimer interrupt (upper isr)
 */
void do_hrtime_event(void)
{
    struct ktime_tick *sptr_list, *sptr_pending;
    struct hrtimer_list *sptr_timer, *sptr_temp;
    khrtime_t cur_tick;
    struct hrtimer_list *sptr_next = mr_nullptr;

    sptr_list = &sgtc_ktime_htick;
    sptr_pending = &sgtc_ktime_pending;

    /*!< Get tick real-time (deal with more events, within 1us) */
    cur_tick = khrtime_ticks() + USEC_TO_HRTICK(1);

    /*!< Deal with events */
    foreach_list_next_entry_safe(sptr_timer, sptr_temp, &sptr_list->sgtc_list, sgtc_link)
    {
        /*!< Timeout */
        if (cur_tick >= sptr_timer->expires)
        {
            sptr_timer->status = NR_KTIMER_PENDING;
            list_head_del(&sptr_timer->sgtc_link);
            list_head_add_tail(&sptr_pending->sgtc_list, &sptr_timer->sgtc_link);
        }
        else
        {
            sptr_next = HRTIMER_SELECT(sptr_next, sptr_timer);
        }
    }

    sptr_list->sptr_first = (struct timer_list *)sptr_next;
    if (!sptr_next)
        khrtime_event_disable();
    else
        khrtime_reload_cnt(sptr_next->expires);

#if CONFIG_HRTIMER_SOFTIRQ
    /*!< Schedule softirq */
    fwk_raise_softirq(NR_SOFTIRQ_TIMER);
#else
    do_hrtimer_action(NR_SOFTIRQ_TIMER);
#endif
}
#undef  HRTIMER_SELECT

/*!
 * @brief   system timer init
 * @param   none
 * @retval  none
 * @note    called by "start_kernel"
 */
void systime_init(void)
{
    struct ktime_tick *sptr_tick;
    kusize_t list_num = ARRAY_SIZE(sgtr_ktime_lists);

    sptr_tick = &sgtc_ktime_jiffies;
    sptr_tick->sptr_first = mr_nullptr;
    init_list_head(&sptr_tick->sgtc_list);
    spin_lock_init(&sptr_tick->sgtc_lock);

    while (list_num--)
    {
        sptr_tick = sgtr_ktime_lists[list_num];

        sptr_tick->sptr_first = mr_nullptr;
        init_list_head(&sptr_tick->sgtc_list);
        spin_lock_init(&sptr_tick->sgtc_lock);
    }

    fwk_open_softirq(NR_SOFTIRQ_TIMER, do_hrtimer_action);
}

/* end of file */
