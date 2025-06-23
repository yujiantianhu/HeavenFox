/*
 * Delay Function Declare
 *
 * File Name:   time.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.10.09
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __TIME_H
#define __TIME_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <configs/configs.h>
#include <common/list_types.h>

/*!< The globals */
typedef kutype_t kutime_t;
typedef kstype_t kstime_t;
typedef kuint64_t khrtime_t;

extern volatile kutime_t jiffies;
extern volatile kuint64_t jiffies_all;
extern volatile kutime_t jiffies_out;

extern volatile kutime_t *ptr_systick_counter;
extern kutime_t g_systick_freq;
extern kbool_t g_is_systick_up;

extern kutime_t g_hrtime_over_cnt;

extern volatile kutime_t *ptr_hrtimer_counter;
extern volatile kutime_t *ptr_hrtimer_counter2;
extern kutime_t g_hrtimer_freq;
extern kbool_t g_is_hrtimer_up;

extern kutime_t g_delay_timer_counter;

/*!< The defines */
#define TICK_HZ                                             CONFIG_HZ

#define IS_TICKCNT_INC                                      (true)
#define IS_TICKCNT_DEC                                      (false)

#define FREQ_INIT_VAL                                       (1)

/*!< ----------------------------------------------------------- */
/*!< per tick period = (1 / SYSTICK_FREQ) */
#define SYSTICK_FREQ                                        (g_systick_freq)
#define IS_SYSTICK_UPINC()                                  (g_is_systick_up)
#define SYSTICK_CNT()                                       (ptr_systick_counter ? (*ptr_systick_counter) : 0)

/*!< is_up: IS_TICKCNT_INC or IS_TICKCNT_DEC */
#define SYSTICK_INIT(freq, is_up, counter) \
    do {    \
        g_is_systick_up = (is_up); \
        ptr_systick_counter = (volatile kutime_t *)(counter);  \
        g_systick_freq = (freq);  \
    } while (0)

#define SYSTICK_MAX                                         (SYSTICK_FREQ / TICK_HZ)

#define SEC_TO_SYSTICK(sec)                                 ( (sec ) * SYSTICK_FREQ)
#define MSEC_TO_SYSTICK(msec)                               (((msec) * SYSTICK_FREQ + 999UL) / 1000UL)
#define USEC_TO_SYSTICK(usec)                               (((usec) * SYSTICK_FREQ + 999999UL) / 1000000UL)
#define NSEC_TO_SYSTICK(nsec)                               (((nsec) * SYSTICK_FREQ + 999999999UL) / 1000000000UL)
#define SYSTICK_TO_SEC(tick)                                ( (tick) / SYSTICK_FREQ)
#define SYSTICK_TO_MSEC(tick)                               (((tick) * 1000UL) / SYSTICK_FREQ)
#define SYSTICK_TO_USEC(tick)                               (((tick) * 1000000UL) / SYSTICK_FREQ)
#define SYSTICK_TO_NSEC(tick)                               (((tick) * 1000000000UL) / SYSTICK_FREQ)

#define JIFFIES_MAX                                         (0x7fffffffU)
#define JIFFIES_BORDER                                      (JIFFIES_MAX - 1)
#define JIFFIES_INITVAL                                     (0x70000000U - 1U)
/*!< Total jiffies (0 ~ (kuint64_t)(~0ULL)) */
#define JIFFIES_COUNT()                                     (jiffies_all)

/*!< ----------------------------------------------------------- */
/*!< per tick period = (1 / HRTIMER_FREQ) */
#define HRTIMER_FREQ                                        (g_hrtimer_freq)
#define IS_HRTIMER_UPINC()                                  (g_is_hrtimer_up)
#define HRTIMER_CNT()   \
    (ptr_hrtimer_counter ? ((*ptr_hrtimer_counter) |    \
        (ptr_hrtimer_counter2 ? ((kuint64_t)(*ptr_hrtimer_counter2) << 32) : 0)) : 0)

#define HRTIMER_MAX                                         (ptr_hrtimer_counter2 ? (kuint64_t)(~0ULL) : (kuint32_t)(~0UL))

/*!< is_up: IS_TICKCNT_INC or IS_TICKCNT_DEC */
#define HRTIMER_INIT(freq, is_up, counter1, counter2) \
    do {    \
        g_is_hrtimer_up = (is_up); \
        ptr_hrtimer_counter = (volatile kutime_t *)(counter1);  \
        ptr_hrtimer_counter2 = (volatile kutime_t *)(counter2); \
        g_hrtimer_freq = (freq);  \
    } while (0)

#define SEC_TO_HRTICK(sec)                                  ( (khrtime_t)(sec ) * HRTIMER_FREQ)
#define MSEC_TO_HRTICK(msec)                                (((khrtime_t)(msec) * HRTIMER_FREQ + 999ULL) / 1000ULL)
#define USEC_TO_HRTICK(usec)                                (((khrtime_t)(usec) * HRTIMER_FREQ + 999999ULL) / 1000000ULL)
#define NSEC_TO_HRTICK(nsec)                                (((khrtime_t)(nsec) * HRTIMER_FREQ + 999999999ULL) / 1000000000ULL)
#define HRTICK_TO_SEC(tick)                                 ( (khrtime_t)(tick) / HRTIMER_FREQ)
#define HRTICK_TO_MSEC(tick)                                (((khrtime_t)(tick) * 1000ULL) / HRTIMER_FREQ)
#define HRTICK_TO_USEC(tick)                                (((khrtime_t)(tick) * 1000000ULL) / HRTIMER_FREQ)
#define HRTICK_TO_NSEC(tick)                                (((khrtime_t)(tick) * 1000000000ULL) / HRTIMER_FREQ)

/*!< ----------------------------------------------------------- */
#define TIMER_DELAY_COUNTER                                 (g_delay_timer_counter)
#define TIMER_DELAY_COUNTER_INIT                            (0U)
#define TIMER_DELAY_COUNTER_MAX                             (0x00ffffffU)                   /*!< = 16777215 */
#define TIMER_DELAY_COUNTER_MAJOR                           ((kuint32_t)(((TIMER_DELAY_COUNTER) >> 24U) & 0x000000ffU))
#define TIMER_DELAY_COUNTER_MINOR                           ((kuint32_t)((TIMER_DELAY_COUNTER) & (TIMER_DELAY_COUNTER_MAX)))

struct time_spec
{
    /*!< time = tv_sec  */
    kutime_t tv_sec;			        	                /*!< seconds */
    kutime_t tv_nsec;		                                /*!< nanoseconds */
};

#define mr_is_timespec_empty(t)                            (((t)->tv_sec == 0) && ((t)->tv_nsec == 0))

struct time_val
{
    /*!< time = tv_sec  */
    kutime_t tv_sec;			        	                /*!< seconds */
    kutime_t tv_usec;		                                /*!< useconds */
};

/*!< Timer event list */
struct timer_list 
{
    struct list_head sgtc_link;
    kutime_t expires;

    void (*entry)(kuint32_t args);
    kuint32_t data;
};

#define TIMER_INITIALIZER(_entry, _expires, _data)		\
{   \
    .expires = _expires,    \
    .entry = _entry,    \
    .data = _data,  \
}

#define DEFINE_TIMER(_name, _entry, _expires, _data)		\
    struct timer_list _name = TIMER_INITIALIZER(_entry, _expires, _data)

#define mr_setup_timer(timer, fn, data)    \
    do {    \
        init_list_head(&(timer)->sgtc_link);  \
        (timer)->entry = (fn); \
        (timer)->data = (data);   \
    } while (0)

#define mr_time_check_type(a, b)   \
    const typeof(a) _a = 0;	\
    const typeof(b) _b = 0;	\
    (void)(&_a == &(a));	\
    (void)(&_b == &(b));

/*!< a > b ? true : false */
#define mr_time_after(a, b)    \
({  \
    mr_time_check_type(a, b)   \
    ((a) > (b));   \
})

/*!< a >= b ? true : false */
#define mr_time_after_eq(a, b)	\
({  \
    mr_time_check_type(a, b)   \
    ((a) >= (b));   \
})

#define mr_time_before(a, b)                                mr_time_after(b, a)            /*!< a < b ? true : false */
#define mr_time_before_eq(a, b)                             mr_time_after_eq(b, a)         /*!< a <= b ? true : false */

struct hrtimer_list
{
    struct list_head sgtc_link;
    khrtime_t expires;

    void (*entry)(kuint32_t args);
    kuint32_t data;
};

#define DEFINE_HRTIMER(_name, _entry, _expires, _data)		\
    struct hrtimer_list _name = TIMER_INITIALIZER(_entry, _expires, _data)

#define HRTIMER_EXPIRES(interval)                           (khrtime_ticks() + (interval))

struct time_clock 
{
    kuint32_t year;
    kuint8_t month;
    kuint8_t day;
    kuint8_t weak;
    kuint8_t hour;
    kuint8_t minute;
    kuint8_t second;
    kuint16_t milsecond;
};
extern struct time_clock sgtc_systime_clock;

/*!< The functions */
extern void simple_delay_timer_initial(void);
extern void simple_delay_timer_runs(void);

extern void delay_cnt(kuint32_t n);
extern void delay_s(kuint32_t n_s);
extern void delay_ms(kuint32_t n_ms);
extern void delay_us(kuint32_t n_us);
extern void wait_secs(kuint32_t seconds);
extern void wait_msecs(kuint32_t milseconds);
extern void wait_usecs(kuint32_t useconds);
extern void msecs_to_timeclock(struct time_clock *sptr_tclk, kutype_t milseconds);

extern void setup_timer(struct timer_list *sptr_timer, void (*entry)(kuint32_t), kuint32_t data);
extern void add_timer(struct timer_list *sptr_timer);
extern void del_timer(struct timer_list *sptr_timer);
extern kbool_t find_timer(struct timer_list *sptr_timer);
extern void mod_timer(struct timer_list *sptr_timer, kutime_t expires);
extern void do_timer_event(void);

extern void setup_hrtimer(struct hrtimer_list *sptr_timer, void (*entry)(kuint32_t), kuint32_t data);
extern void add_hrtimer(struct hrtimer_list *sptr_timer);
extern void del_hrtimer(struct hrtimer_list *sptr_timer);
extern void mod_hrtimer(struct hrtimer_list *sptr_timer, khrtime_t expires);
extern void check_hrtimer(void);
extern void do_hrtime_event(void);

extern kutime_t ktime_systick(void);
extern khrtime_t ktime_hrtick(void);
extern khrtime_t khrtime_ticks(void);
extern kbool_t khrtime_check_overcnt(void);
extern void ktime_to_spec(struct time_val *sptr_tval);
extern void khrtime_reload_cnt(khrtime_t expires);

extern void systime_init(void);

/*!< API functions */
/*!
 * @brief   jiffies increment
 * @param   none
 * @retval  none
 * @note    for jiffies counter (called by timer irq handler)
 */
static inline void get_time_counter(void)
{
    jiffies = (jiffies >= JIFFIES_MAX) ? 0 : (jiffies + 1);
    jiffies_out = jiffies ? jiffies_out : (jiffies_out + 1);

//  jiffies_all = (jiffies_out * JIFFIES_MAX) + jiffies - JIFFIES_INITVAL;
    jiffies_all++;
}

/*!
 * @brief   mark that hrtimer counter is over (> 0xffffffffU ?)
 * @param   none
 * @retval  none
 * @note    g_hrtime_over_cnt++
 */
static inline void mark_hrtime_overone(void)
{
    g_hrtime_over_cnt++;
}

/*!
 * @brief   get hrtimer over count
 * @param   none
 * @retval  g_hrtime_over_cnt
 * @note    none
 */
static inline kutime_t get_hrtime_overcount(void)
{
    return g_hrtime_over_cnt;
}

/*!
 * @brief   return safe expires
 * @param   expires: jiffies + count
 * @retval  none
 * @note    none
 */
static inline kutime_t get_safe_expires(kutime_t expires)
{
    kutime_t safe_expires = 0;

    if (jiffies >= JIFFIES_MAX)
        safe_expires = (expires > JIFFIES_MAX) ? (expires - JIFFIES_MAX - 1) : expires;
    else
        safe_expires = CMP_MIN2(expires, JIFFIES_BORDER);

    return safe_expires;
}

/*!
 * @brief   convert jiffies to seconds
 * @param   jiffies
 * @retval  s
 * @note    none
 */
static inline kuint32_t jiffies_to_secs(const kutime_t j)
{
    return (j / TICK_HZ);
}

/*!
 * @brief   convert jiffies to milseconds
 * @param   jiffies
 * @retval  ms
 * @note    none
 */
static inline kuint32_t jiffies_to_msecs(const kutime_t j)
{
    return ((j * 1000) / TICK_HZ);
}

/*!
 * @brief   convert jiffies to micro seconds
 * @param   jiffies
 * @retval  us
 * @note    none
 */
static inline kuint32_t jiffies_to_usecs(const kutime_t j)
{
    return ((j * 1000 * 1000) / TICK_HZ);
}

/*!
 * @brief   convert jiffies to nseconds
 * @param   jiffies
 * @retval  ns
 * @note    none
 */
static inline kuint64_t jiffies_to_nsecs(const kutime_t j)
{
    return ((j * 1000 * 1000 * 1000) / TICK_HZ);
}

/*!
 * @brief   convert seconds to jiffies
 * @param   s
 * @retval  jiffies
 * @note    none
 */
static inline kutime_t secs_to_jiffies(const kuint32_t s)
{
    return (s * TICK_HZ);
}

/*!
 * @brief   convert milseconds to jiffies
 * @param   ms
 * @retval  jiffies
 * @note    none
 */
static inline kutime_t msecs_to_jiffies(const kuint32_t m)
{
    return m ? (((m * TICK_HZ) + 999) / 1000) : 0;
}

/*!
 * @brief   convert micro seconds to jiffies
 * @param   us
 * @retval  jiffies
 * @note    none
 */
static inline kutime_t usecs_to_jiffies(const kuint32_t u)
{
    return u ? (((u * TICK_HZ) + 999999) / 1000000) : 0;
}

/*!
 * @brief   convert nseconds to jiffies
 * @param   ns
 * @retval  jiffies
 * @note    none
 */
static inline kutime_t nsecs_to_jiffies(const kuint64_t n)
{
    return n ? (((n * TICK_HZ) + 999999999) / 1000000000) : 0;
}

/*!
 * @brief   time_spec ---> ms
 * @param   sptr_tm
 * @retval  ms
 * @note    none
 */
static inline kutime_t time_spec_to_msecs(struct time_spec *sptr_tm)
{
    return ((sptr_tm->tv_sec * 1000) + (sptr_tm->tv_nsec / 1000 / 1000));
}

/*!
 * @brief   ms ---> time_spec
 * @param   sptr_tm, m
 * @retval  sptr_tm
 * @note    none
 */
static inline struct time_spec *msecs_to_time_spec(struct time_spec *sptr_tm, const kuint32_t m)
{
    sptr_tm->tv_sec  = m / 1000;
    sptr_tm->tv_nsec = (m - (sptr_tm->tv_sec * 1000)) * 1000 * 1000;
    
    return sptr_tm;
}

#ifdef __cplusplus
    }
#endif

#endif /* __TIME_H */
