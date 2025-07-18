/*
 * ZYNQ7 Global Timer Initial
 *
 * File Name:   hrtimer.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.06.21
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/time.h>
#include <arch/setup.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/irq/fwk_irq_types.h>
#include <zynq7/zynq7_periph.h>

/*!< The defines */
#define ZYNQ7_GLBTIMER0_FREQ_HZ                 (XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2)

/*!< Register */
#define GT_COUNTER_LOW_REG                      (0x00)
#define GT_COUNTER_HIGH_REG                     (0x04)
#define GT_CONTROL_REG                          (0x08)
#define GT_INT_STATUS_REG                       (0x0C)
#define GT_COMP_LOW_REG                         (0x10)
#define GT_COMP_HIGH_REG                        (0x14)
#define GT_AUTO_INC_REG                         (0x18)

/*!< GT_CONTROL_REG */
#define GT_CTRL_ENABLE                          (0x01)
#define GT_CTRL_COMP_ENABLE                     (0x02)
#define GT_CTRL_IRQ_ENABLE                      (0x04)
#define GT_CTRL_AUTO_INC                        (0x08)

/*!< GT_INT_STATUS_REG */
/*!< Compare interrupt */
#define GT_INT_STATUS_COMP                      (0x01)
/*!< AUto increment interrupt */
#define GT_INT_STATUS_AUTOINC                   (0x02)

struct khrtime_board_data
{
    struct fwk_device_node *sptr_node;
    kint32_t irq;
    kuaddr_t base;
};

/*!< The globals */
static const struct fwk_of_device_id sgtc_zynq7_global_timer_ids[] =
{
    { .compatible = "arm,cortex-a9-global-timer" },
    {},
};

struct khrtime_board_data sgtc_zynq7_khrtime_data;

/*!< API function */
/*!
 * @brief   Read high time total tick
 * @param   none
 * @retval  tick
 * @note    get the time register's current value
 */
khrtime_t khrtime_ticks(void)
{
    struct khrtime_board_data *sptr_data;
    kuaddr_t base;
    kuint32_t high, low, high2;

    sptr_data = &sgtc_zynq7_khrtime_data;
    base = sptr_data->base;

loop:
    high = mr_readl(base + GT_COUNTER_HIGH_REG);
    low = mr_readl(base + GT_COUNTER_LOW_REG);
    high2 = mr_readl(base + GT_COUNTER_HIGH_REG);

    if (mr_unlikely(high2 != high))
        goto loop;

    return (((khrtime_t)high << 32) | low);
}

/*!
 * @brief   load compare value to hardware
 * @param   expires (unit: tick)
 * @retval  none
 * @note    called by systick interrupt
 */
void khrtime_reload_cnt(khrtime_t expires)
{
    struct khrtime_board_data *sptr_data;
    kuaddr_t base;
    kuint32_t reg_value, flags;
    khrtime_t cur_tick;

    sptr_data = &sgtc_zynq7_khrtime_data;
    base = sptr_data->base;

    mr_local_irq_save(flags);

    /*!< Disable interrupt before setting compare value to avoid irq occured */
    reg_value = GT_CTRL_ENABLE;
    mr_writel(reg_value, base + GT_CONTROL_REG);

    cur_tick = khrtime_ticks() + USEC_TO_HRTICK(1);
    if (expires <= cur_tick)
        expires = cur_tick;

    /*!< COMP REG will be changed, ignore invalid and out-of-time compare event */
    mr_writel(GT_INT_STATUS_COMP, sptr_data->base + GT_INT_STATUS_REG);

    /*!<
     * Set compare value
     * Writting GT_COMP_HIGH_REG may cause hardware fault (GT_CONTROL_REG's bit[1:3] will be changed unexpectely)
     * It's necessary to re-write GT_CONTROL_REG after setting GT_COMP_HIGH_REG
     */
    mr_writel(expires, base + GT_COMP_LOW_REG);
    mr_writel(expires >> 32, base + GT_COMP_HIGH_REG);

    /*!< Do not use mr_readl() to get reg_value and load with it, because it maybe not avaliable (bit[1:3] are unstable ?) */
    reg_value |= (GT_CTRL_COMP_ENABLE | GT_CTRL_IRQ_ENABLE);
    mr_writel(reg_value, base + GT_CONTROL_REG);

    mr_local_irq_restore(flags);
}

/*!
 * @brief   start hrtimer
 * @param   none
 * @retval  none
 * @note    open compare
 */
void khrtime_event_enable(void)
{
    struct khrtime_board_data *sptr_data;
    kuint32_t reg_value, flags;

    sptr_data = &sgtc_zynq7_khrtime_data;

    mr_local_irq_save(flags);

    mr_writel(~0U, sptr_data->base + GT_COMP_LOW_REG);
    mr_writel(~0U, sptr_data->base + GT_COMP_HIGH_REG);
    mr_writel(GT_INT_STATUS_COMP, sptr_data->base + GT_INT_STATUS_REG);

    reg_value = mr_readl(sptr_data->base + GT_CONTROL_REG);
    reg_value |= (GT_CTRL_COMP_ENABLE | GT_CTRL_IRQ_ENABLE);
    mr_writel(reg_value, sptr_data->base + GT_CONTROL_REG);

    mr_local_irq_restore(flags);
}

/*!
 * @brief   stop hrtimer
 * @param   none
 * @retval  none
 * @note    close compare
 */
void khrtime_event_disable(void)
{
    struct khrtime_board_data *sptr_data;
    kuint32_t reg_value, flags;

    sptr_data = &sgtc_zynq7_khrtime_data;

    mr_local_irq_save(flags);

    reg_value = mr_readl(sptr_data->base + GT_CONTROL_REG);
    reg_value &= ~(GT_CTRL_COMP_ENABLE | GT_CTRL_IRQ_ENABLE);
    mr_writel(reg_value, sptr_data->base + GT_CONTROL_REG);

    mr_local_irq_restore(flags);
}

/*!< -------------------------------------------------------------------- */
/*!
 * @brief   Enable global timer
 * @param   BaseAddress: address of timer
 * @retval  none
 * @note    Set compare enable and timer enable bit
 */
void zynq7_gt_start(kuaddr_t BaseAddress)
{
    kuint32_t reg_value;

    /*!< Clear counter */
    mr_writel(0U, BaseAddress + GT_COUNTER_HIGH_REG);
    mr_writel(0U, BaseAddress + GT_COUNTER_LOW_REG);

    /*!< Avoid clearing GT_CTRL_IRQ_ENABLE */
    reg_value = mr_readl(BaseAddress + GT_CONTROL_REG);
    reg_value &= ~GT_CTRL_AUTO_INC;
    reg_value |= (GT_CTRL_ENABLE | GT_CTRL_COMP_ENABLE);
    mr_writel(reg_value, BaseAddress + GT_CONTROL_REG);
}

/*!
 * @brief   Disable global timer
 * @param   BaseAddress: address of timer
 * @retval  none
 * @note    Clear GT_CONTROL_REG
 */
void zynq7_gt_stop(kuaddr_t BaseAddress)
{
    mr_writel(0, BaseAddress + GT_CONTROL_REG);
}

/*!
 * @brief   Enable comparing
 * @param   BaseAddress: address of timer
 * @retval  none
 * @note    Set GT_CTRL_COMP_ENABLE
 */
void zynq7_gt_comp_start(kuaddr_t BaseAddress)
{
    kuint32_t reg_value;

    reg_value = mr_readl(BaseAddress + GT_CONTROL_REG);
    reg_value |= GT_CTRL_COMP_ENABLE;
    mr_writel(reg_value, BaseAddress + GT_CONTROL_REG);
}

/*!
 * @brief   Disable comparing
 * @param   BaseAddress: address of timer
 * @retval  none
 * @note    Clear GT_CTRL_COMP_ENABLE
 */
void zynq7_gt_comp_stop(kuaddr_t BaseAddress)
{
    kuint32_t reg_value;

    reg_value = mr_readl(BaseAddress + GT_CONTROL_REG);
    reg_value &= ~GT_CTRL_COMP_ENABLE;
    mr_writel(reg_value, BaseAddress + GT_CONTROL_REG);
}

/*!
 * @brief   Enable interrupt for global timer
 * @param   BaseAddress: address of timer
 * @retval  none
 * @note    Set GT_CTRL_IRQ_ENABLE
 */
void zynq7_gt_irq_enable(kuaddr_t BaseAddress)
{
    kuint32_t reg_value;

    reg_value = mr_readl(BaseAddress + GT_CONTROL_REG);
    reg_value |= GT_CTRL_IRQ_ENABLE;
    mr_writel(reg_value, BaseAddress + GT_CONTROL_REG);
}

/*!
 * @brief   Disable interrupt for global timer
 * @param   BaseAddress: address of timer
 * @retval  none
 * @note    Clear GT_CTRL_IRQ_ENABLE
 */
void zynq7_gt_irq_disable(kuaddr_t BaseAddress)
{
    kuint32_t reg_value;

    reg_value = mr_readl(BaseAddress + GT_CONTROL_REG);
    reg_value &= ~GT_CTRL_IRQ_ENABLE;
    mr_writel(reg_value, BaseAddress + GT_CONTROL_REG);
}

/*!
 * @brief   Get interrupt status
 * @param   BaseAddress: address of timer
 * @retval  Status
 * @note    none
 */
#define zynq7_gt_intr_get_status(BaseAddress)   \
    mr_readl(BaseAddress + GT_INT_STATUS_REG)

/*!
 * @brief   Clear interrupt status
 * @param   BaseAddress: address of timer
 * @retval  none
 * @note    Set 0xffffffffU to GT_INT_STATUS_REG
 */
#define zynq7_gt_intr_clear_status(BaseAddress) \
    mr_writel(0xffffffffU, BaseAddress + GT_INT_STATUS_REG)

/*!
 * @brief   Global Timer Interrupt Callback
 * @param   irq: irq number of global timer
 * @param   args: struct khrtime_board_data *
 * @retval  irq flags
 * @note    Call do_hrtime_event to deal with hrtimer_list
 */
irq_return_t zynq7_gt_isr(kint32_t irq, void *args)
{
    struct khrtime_board_data *sptr_data;
    kuaddr_t BaseAddress;
    kuint32_t status;

    sptr_data = (struct khrtime_board_data *)args;
    BaseAddress = sptr_data->base;

    /*!< Get and clear interrupt status */
    status = zynq7_gt_intr_get_status(BaseAddress);
    if (!status)
        return NR_IRQ_NONE;

    zynq7_gt_intr_clear_status(BaseAddress);
    mr_dsb();

    if (status & GT_INT_STATUS_COMP)
    {
        /*!
         * @note
         * Write a max value to avoid comparing interrupt ocurred again 
         * while do_hrtime_event() is excuting 
         */
        khrtime_reload_cnt(HRTIMER_MAX);
        do_hrtime_event();
    }

    return NR_IRQ_HANDLED;
}

/*!
 * @brief   Global Timer Initialization
 * @param   none
 * @retval  none
 * @note    Call by board_init_systick()
 */
void zynq7_hrtime_init(void)
{
    struct khrtime_board_data *sptr_data;
    struct fwk_device_node *sptr_node;
    kint32_t irq, retval;
    kuaddr_t base;

    sptr_node = fwk_of_find_matching_node_and_match(mr_nullptr, sgtc_zynq7_global_timer_ids, mr_nullptr);
    if (!isValid(sptr_node))
        return;

    base = (kuaddr_t)fwk_of_iomap(sptr_node, 0);
    irq = fwk_of_irq_get(sptr_node, 0);

    sptr_data = &sgtc_zynq7_khrtime_data;
    sptr_data->sptr_node = sptr_node;
    sptr_data->base = base;
    sptr_data->irq = irq;

    zynq7_gt_stop(base);
    zynq7_gt_irq_disable(base);

    /*!< Period: 3ns */
    HRTIMER_INIT(ZYNQ7_GLBTIMER0_FREQ_HZ,
                 IS_TICKCNT_INC,
                 base + GT_COUNTER_LOW_REG,
                 base + GT_COUNTER_HIGH_REG);

    retval = fwk_request_irq(irq, zynq7_gt_isr, 0, "zynq7-khrtime", sptr_data);
    if (retval)
        return;

    khrtime_reload_cnt(HRTIMER_MAX);
    zynq7_gt_irq_enable(base);
    zynq7_gt_intr_clear_status(base);
    zynq7_gt_start(base);
}

/*!< -------------------------------------------------------------------------- */
/*!
 * @brief   initial high real-time timer
 * @param   none
 * @retval  none
 * @note    initial Timer
 */
void __init arch_hrtimer_init(void)
{
    zynq7_hrtime_init();
}

/* end of file*/