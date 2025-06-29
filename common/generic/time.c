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
volatile kutime_t jiffies = JIFFIES_INITVAL;
volatile kuint64_t jiffies_all = 0;
volatile kutime_t jiffies_out = 0;

volatile kutime_t *ptr_systick_counter = mr_nullptr;
kutime_t g_systick_freq = FREQ_INIT_VAL;
kbool_t g_is_systick_up = true;

kutime_t g_hrtime_over_cnt = 0;

/*!< ---------------------------------------------------------- */
volatile kutime_t *ptr_hrtimer_counter = mr_nullptr;
volatile kutime_t *ptr_hrtimer_counter2 = mr_nullptr;
kutime_t g_hrtimer_freq = FREQ_INIT_VAL;
kbool_t g_is_hrtimer_up = true;

/*!< ---------------------------------------------------------- */
kutime_t g_delay_timer_counter = 0;
struct time_clock sgtc_systime_clock;

static kuint32_t g_simple_delay_timer = 0;
static kuint32_t g_simple_timeout_cnt = 0;

static struct ktime_tick sgtc_ktime_jiffies;
static struct ktime_tick sgtc_ktime_htick;
static struct ktime_tick sgtc_ktime_inactive;

/*!< API function */
/*!
 * @brief   Read current systick
 * @param   none
 * @retval  tick
 * @note    get the time register's current value
 */
__weak kutime_t ktime_systick(void)
{
    volatile kutime_t *time_cnt = ptr_systick_counter;
    return time_cnt ? (IS_SYSTICK_UPINC() ? (*time_cnt) : (SYSTICK_MAX - (*time_cnt))) : 0;
}

/*!
 * @brief   Read high time current tick
 * @param   none
 * @retval  tick
 * @note    get the time register's current value
 */
__weak khrtime_t ktime_hrtick(void)
{
    volatile kutime_t *time_cnt1 = ptr_hrtimer_counter;
    volatile kutime_t *time_cnt2 = ptr_hrtimer_counter2;

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
    volatile kutime_t *time_cnt1 = ptr_hrtimer_counter;
    volatile kutime_t *time_cnt2 = ptr_hrtimer_counter2;
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

    /*!< 
     * If khrtime_ticks is called by irq_handler, g_hrtime_over_cnt maybe still old;
     * because g_hrtime_over_cnt only increase in IRQ callback, not real-time,
     * it means that g_hrtime_over_cnt will be delayed.
     * khrtime_check_overcnt() will read interrupt status register
     */
    over_count = khrtime_check_overcnt() ? (g_hrtime_over_cnt + 1) : g_hrtime_over_cnt;
    tick_l = IS_HRTIMER_UPINC() ? (*time_cnt1) : (HRTIMER_MAX - (*time_cnt1));

    return (over_count * (khrtime_t)HRTIMER_MAX) + tick_l;
}

/*!
 * @brief   Check if hrtime counter out
 * @param   none
 * @retval  none
 * @note    true or false
 */
__weak kbool_t khrtime_check_overcnt(void)
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
 * @brief   simple_delay_timer_initial
 * @param   none
 * @retval  none
 * @note    delay timer counter initial
 */
void simple_delay_timer_initial(void)
{
    g_simple_delay_timer = TIMER_DELAY_COUNTER_INIT;
    g_simple_timeout_cnt = TIMER_DELAY_COUNTER_INIT;

    g_delay_timer_counter = TIMER_DELAY_COUNTER_INIT;
}

/*!
 * @brief   simple_delay_timer_runs
 * @param   none
 * @retval  none
 * @note    delay timer counter excute
 */
void simple_delay_timer_runs(void)
{
    if ((g_simple_delay_timer++) >= TIMER_DELAY_COUNTER_MAX)
    {
        g_simple_delay_timer = TIMER_DELAY_COUNTER_INIT;
        g_simple_timeout_cnt = ((g_simple_timeout_cnt >= 255) ? 
                            TIMER_DELAY_COUNTER_INIT : (g_simple_timeout_cnt + 1));
    }

    g_delay_timer_counter = mr_bit_mask(g_simple_timeout_cnt, 
                            ~TIMER_DELAY_COUNTER_MAX, 24U) + g_simple_delay_timer;
}

/*!
 * @brief   delay_cnt
 * @param   n
 * @retval  none
 * @note    delay n counters
 */
void delay_cnt(kuint32_t n)
{
    while (n--)
        mr_delay_nop();
}

/*!
 * @brief   delay_s
 * @param   n_s
 * @retval  none
 * @note    delay n_s seconds
 */
__weak void delay_s(kuint32_t n_s)
{
    while (n_s--)
        delay_cnt(DELAY_SIMPLE_COUNTER_PER_S);
}

/*!
 * @brief   delay_ms
 * @param   n_ms
 * @retval  none
 * @note    delay n_ms miliseconds
 */
__weak void delay_ms(kuint32_t n_ms)
{
    while (n_ms--)
        delay_cnt(DELAY_SIMPLE_COUNTER_PER_MS);
}

/*!
 * @brief   delay_us
 * @param   n_us
 * @retval  none
 * @note    delay n_us microseconds
 */
__weak void delay_us(kuint32_t n_us)
{
    while (n_us--)
        delay_cnt(DELAY_SIMPLE_COUNTER_PER_US);
}

/*!
 * @brief   delay (unit: s)
 * @param   seconds
 * @retval  none
 * @note    none
 */
void delay(kuint32_t seconds)
{
    if (ptr_hrtimer_counter)
    {
        khrtime_t target_tick = khrtime_ticks() + SEC_TO_HRTICK(seconds);
        while (target_tick > khrtime_ticks());
    }
    else
    {
        kuint64_t ticks = seconds * (CONFIG_CPU_FREQ / 200U);
        while (ticks--);
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
    if (ptr_hrtimer_counter)
    {
        khrtime_t target_tick = khrtime_ticks() + MSEC_TO_HRTICK(milseconds);
        while (target_tick > khrtime_ticks());
    }
    else
    {
        kuint64_t ticks = milseconds * (CONFIG_CPU_FREQ / 1000U / 200U);
        while (ticks--);
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
    if (ptr_hrtimer_counter)
    {
        khrtime_t target_tick = khrtime_ticks() + USEC_TO_HRTICK(useconds);
        while (target_tick > khrtime_ticks());
    }
    else
    {
        kuint64_t ticks = useconds * (CONFIG_CPU_FREQ / 1000000U / 200U);
        while (ticks--);
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

    mr_setup_timer(sptr_timer, entry, data);
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
    struct hrtimer_list **sptr_first;

    if ((!sptr_timer) || 
        (!sptr_timer->expires) ||
        (!sptr_timer->entry))
        return;

    sptr_list = &sgtc_ktime_htick;
    sptr_lock = &sptr_list->sgtc_lock;
    sptr_first = (struct hrtimer_list **)&sptr_list->sptr_first;

    spin_lock_irqsave(sptr_lock);

    if (!(*sptr_first) ||
        (sptr_timer->expires < (*sptr_first)->expires))
    {
        *sptr_first = sptr_timer;
        khrtime_reload_cnt(sptr_timer->expires);
    }

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
    sptr_lock = &sptr_list->sgtc_lock;

    spin_lock_irqsave(sptr_lock);
    list_head_del(&sptr_timer->sgtc_link);

    if (mr_unlikely((struct timer_list *)sptr_timer == sptr_list->sptr_first))
    {
        foreach_list_next_entry(sptr_per, &sptr_list->sgtc_list, sgtc_link)
            sptr_next = HRTIMER_SELECT(sptr_next, sptr_per);

        sptr_list->sptr_first = (struct timer_list *)sptr_next;
        khrtime_reload_cnt(sptr_next ? sptr_next->expires : HRTIMER_MAX);
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
 * @brief   tick handler
 * @param   none
 * @param	none
 * @retval  none
 * @note    called by hrtimer interrupt
 */
void do_hrtime_event(void)
{
    struct ktime_tick *sptr_list, *sptr_cast;
    struct hrtimer_list *sptr_timer, *sptr_temp;
    khrtime_t cur_tick;
    struct hrtimer_list *sptr_next = mr_nullptr;
    khrtime_t max_ticks = HRTIMER_MAX;

    sptr_list = &sgtc_ktime_htick;
    sptr_cast = &sgtc_ktime_inactive;

    foreach_list_next_entry_safe(sptr_timer, sptr_temp, &sptr_list->sgtc_list, sgtc_link)
    {
        /*!< Get tick real-time (deal with more events, within 1us) */
        cur_tick = khrtime_ticks() + USEC_TO_HRTICK(1);

        if (mr_time_before(cur_tick, sptr_timer->expires))
            sptr_next = HRTIMER_SELECT(sptr_next, sptr_timer);
        else
        {
            /*!< Do event */
            sptr_timer->entry(sptr_timer->data);
            if (cur_tick < sptr_timer->expires)
                sptr_next = HRTIMER_SELECT(sptr_next, sptr_timer);
            else
            {
                sptr_timer->expires = max_ticks;

                list_head_del(&sptr_timer->sgtc_link);
                list_head_add_tail(&sptr_cast->sgtc_list, &sptr_timer->sgtc_link);
            }
        }
    }
    
    sptr_list->sptr_first = (struct timer_list *)sptr_next;
    khrtime_reload_cnt(sptr_next ? sptr_next->expires : max_ticks);
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

    sptr_tick = &sgtc_ktime_jiffies;
    sptr_tick->sptr_first = mr_nullptr;
    init_list_head(&sptr_tick->sgtc_list);
    spin_lock_init(&sptr_tick->sgtc_lock);

    sptr_tick = &sgtc_ktime_htick;
    sptr_tick->sptr_first = mr_nullptr;
    init_list_head(&sptr_tick->sgtc_list);
    spin_lock_init(&sptr_tick->sgtc_lock);

    sptr_tick = &sgtc_ktime_inactive;
    sptr_tick->sptr_first = mr_nullptr;
    init_list_head(&sptr_tick->sgtc_list);
    spin_lock_init(&sptr_tick->sgtc_lock);
}

/* end of file */
