/*
 * Template of Character Device : I2C of imx6ull
 *
 * File Name:   imx_i2c.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.10
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/base/fwk_platdrv.h>
#include <platform/base/fwk_uaccess.h>
#include <platform/clk/fwk_clk.h>
#include <platform/i2c/fwk_i2c_dev.h>
#include <platform/i2c/fwk_i2c_core.h>
#include <platform/i2c/fwk_i2c_algo.h>
#include <kernel/mutex.h>
#include <kernel/wait.h>

#include <imx6/imx6ull_pins.h>
#include <imx6/imx6ull_periph.h>

/*!< The defines */
typedef struct imx_i2c_drv_data
{
	struct fwk_i2c_adapter sgtc_adap;
	void *reg;
    kuint32_t freq;

    struct fwk_clk *sptr_clk;
    kbool_t is_lastMsgs;

    kint32_t irq;
    kbool_t is_wake;

    struct wait_queue_head sgtc_wqh;
    struct mutex_lock sgtc_lock;

} srt_imx_i2c_drv_data_t;

/*!< address register */
struct imx_i2c_iar_reg
{
    __IO kuint16_t rw    : 1;                       /*!< bit 0 */
    __IO kuint16_t addr  : 7;                       /*!< bit 1:7 */
    __IO kuint16_t rsvd1 : 8;                       /*!< bit 8:15 */
};

/*!< frequency register */
struct imx_i2c_ifr_reg
{
    __IO kuint16_t freq  : 6;                       /*!< bit 0:5 */
    __IO kuint16_t rsvd1 : 2;                       /*!< bit 6:7 */
    __IO kuint16_t rsvd2 : 8;                       /*!< bit 8:15 */
};

/*!< control register */
struct imx_i2c_icr_reg
{
    __IO kuint16_t rsvd1 : 2;                       /*!< bit 0:1 reserved */
    __IO kuint16_t rsta  : 1;                       /*!< bit 2   repeat start: 1, generate signal */
    __IO kuint16_t txak  : 1;                       /*!< bit 3   transfer(send) ack: 0, ack; 1, noack */
    __IO kuint16_t mtx   : 1;                       /*!< bit 4   transfer direction: 0, rx;  1, tx */
    __IO kuint16_t msta  : 1;                       /*!< bit 5   master/slave enable: 0, slave; 1, master */
    __IO kuint16_t iien  : 1;                       /*!< bit 6   interrupt enable, active high */
    __IO kuint16_t ien   : 1;                       /*!< bit 7   iic enable, active high */
    __IO kuint16_t rsvd2 : 8;                       /*!< bit 8:15 */
};

/*!< status register */
struct imx_i2c_isr_reg
{
    __IO kuint16_t rxak  : 1;                       /*!< bit 0   transfer(recv) ack: 0, ack; 1, noack */
    __IO kuint16_t iif   : 1;                       /*!< bit 1   iic interrupt suspend, active high */
    __IO kuint16_t rsvd1 : 1;                       /*!< bit 2   reserved */
    __IO kuint16_t srw   : 1;                       /*!< bit 3   slave r/w flag: 0: write to slave, 1: read from slave */
    __IO kuint16_t ial   : 1;                       /*!< bit 4   arbitration loss bit: active high */
    __IO kuint16_t ibb   : 1;                       /*!< bit 5   iic bus busy flag; 1: busy, 0: idle */
    __IO kuint16_t iaas  : 1;                       /*!< bit 6   1 indicates that iar.addr is the address of slave */
    __IO kuint16_t icf   : 1;                       /*!< bit 7   data transmission flag: 0, is transfering; 1, transfer finished */
    __IO kuint16_t rsvd2 : 8;                       /*!< bit 8:15 */
};

/*!< data register */
struct imx_i2c_idr_reg
{
    __IO kuint16_t data  : 8;                       /*!< bit 0:7 */
    __IO kuint16_t rsvd1 : 8;                       /*!< bit 8:15 */
};

__align(4) struct imx_i2c_reg
{
	struct imx_i2c_iar_reg sgtc_iar;
    kuint8_t rsvd_0[2];

	struct imx_i2c_ifr_reg sgtc_ifr;
    kuint8_t rsvd_1[2];

	struct imx_i2c_icr_reg sgtc_icr;
    kuint8_t rsvd_2[2];

	struct imx_i2c_isr_reg sgtc_isr;
    kuint8_t rsvd_3[2];

	struct imx_i2c_idr_reg sgtc_idr;
};

enum __ERT_IMX_I2C_ACK_STATUS
{
    NR_I2C_ACK = 0,
    NR_I2C_NACK = 1,
};

/*!< The globals */
/*!<
 * PLL2 = 528 MHz
 * PLL2_PFD2 = 528 MHz * 18 / 24 = 396 MHz
 * IPG_CLK_ROOT = (PLL2_PFD2 / ahb_podf) / ipg_podf = (396 MHz / 3) / 2 = 66MHz
 * PER_CLK_ROOT = IPG_CLK_ROOT / perclk_podf = 66MHz / 1 = 66MHz
 * I2C clock Frequency = (PERCLK_ROOT frequency)/(division factor corresponding to IFDR)
 */
static const kuint16_t g_imx_i2c_ifdr_field[][2] =
{
    { 0x00, 30 },   { 0x10, 288 },  { 0x20, 22 },   { 0x30, 160 },
    { 0x01, 32 },   { 0x11, 320 },  { 0x21, 24 },   { 0x31, 192 },
    { 0x02, 36 },   { 0x12, 384 },  { 0x22, 26 },   { 0x32, 224 },
    { 0x03, 42 },   { 0x13, 480 },  { 0x23, 28 },   { 0x33, 256 },
    { 0x04, 48 },   { 0x14, 576 },  { 0x24, 32 },   { 0x34, 320 },
    { 0x05, 52 },   { 0x15, 640 },  { 0x25, 36 },   { 0x35, 384 },
    { 0x06, 60 },   { 0x16, 768 },  { 0x26, 40 },   { 0x36, 448 },
    { 0x07, 72 },   { 0x17, 960 },  { 0x27, 44 },   { 0x37, 512 },
    { 0x08, 80 },   { 0x18, 1152 }, { 0x28, 48 },   { 0x38, 640 },
    { 0x09, 88 },   { 0x19, 1280 }, { 0x29, 56 },   { 0x39, 768 },
    { 0x0a, 104 },  { 0x1a, 1536 }, { 0x2a, 64 },   { 0x3a, 896 },
    { 0x0b, 128 },  { 0x1b, 1920 }, { 0x2b, 72 },   { 0x3b, 1024 },
    { 0x0c, 144 },  { 0x1c, 2304 }, { 0x2c, 80 },   { 0x3c, 1280 },
    { 0x0d, 160 },  { 0x1d, 2560 }, { 0x2d, 96 },   { 0x3d, 1536 },
    { 0x0e, 192 },  { 0x1e, 3072 }, { 0x2e, 112 },  { 0x3e, 1792 },
    { 0x0f, 240 },  { 0x1f, 3840 }, { 0x2f, 128 },  { 0x3f, 2048 },
};

/*!< API function */
/*!
 * @brief   find ifdr-field from g_imx_i2c_ifdr_field
 * @param   freq
 * @retval  ifdr-field
 * @note    none
 */
static kint16_t imx_i2c_find_frequency(kuint32_t freq)
{
    kusize_t num_field = ARRAY_SIZE(g_imx_i2c_ifdr_field);
    kuint16_t div, field, last_temp, cur_temp; 
    kuint16_t idx, idx_satisfy;

    /*!< ipg_clk: 66MHz */
    div = 66000000 / freq;
    last_temp = (kuint16_t)(~0);
    idx_satisfy = last_temp;

    for (idx = 0; idx < num_field; idx++)
    {
        field = g_imx_i2c_ifdr_field[idx][1];
        if (field == div)
        {
            idx_satisfy = idx;
            break;
        }

        cur_temp = mr_usub(field, div);
        if (cur_temp < last_temp)
        {
            last_temp = cur_temp;
            idx_satisfy = idx;
        }
    }

    if (idx_satisfy < num_field)
        return g_imx_i2c_ifdr_field[idx_satisfy][0];

    return -ER_NOTFOUND;
}

/*!
 * @brief   send ack/nack after writting
 * @param   sptr_i2c, ack
 * @retval  none
 * @note    none
 */
#define mr_imx_i2c_set_ack(sptr_i2c, ack)   \
    do {    \
        if ((ack) == NR_I2C_NACK) \
            (sptr_i2c)->sgtc_icr.txak = true; \
        else if ((ack) == NR_I2C_ACK) \
            (sptr_i2c)->sgtc_icr.txak = false;    \
        mr_barrier();   \
    } while (0)

/*!
 * @brief   recv ack/nack after reading
 * @param   sptr_i2c
 * @retval  none
 * @note    none
 */
#define mr_imx_i2c_get_ack(sptr_i2c)                        ((sptr_i2c)->sgtc_isr.rxak ? NR_I2C_NACK : NR_I2C_ACK)

/*!
 * @brief   write data to i2c register
 * @param   sptr_i2c, data
 * @retval  none
 * @note    none
 */
#define mr_imx_i2c_write_data(sptr_i2c, value)              do { (sptr_i2c)->sgtc_idr.data = (value); mr_barrier(); } while (0)

/*!
 * @brief   read data from i2c register
 * @param   sptr_i2c
 * @retval  none
 * @note    none
 */
#define mr_imx_i2c_read_data(sptr_i2c)                      ((sptr_i2c)->sgtc_idr.data)

/*!< waitting condition */
#define mr_imx_i2c_already_comp(sptr_data, sptr_i2c)   \
    ((sptr_i2c)->sgtc_isr.icf && ((sptr_i2c)->sgtc_isr.iif || (sptr_data)->is_wake))

/*!
 * @brief   wait for transferring finished
 * @param   sptr_adap, timeout (unit: ms)
 * @retval  none
 * @note    none
 */
static kbool_t imx_i2c_adap_wait_complete(struct fwk_i2c_adapter *sptr_adap, kuint32_t timeout)
{
    struct imx_i2c_drv_data *sptr_data;
    struct imx_i2c_reg *sptr_i2c;

	sptr_data = fwk_i2c_adapter_get_drvdata(sptr_adap);
	sptr_i2c = (struct imx_i2c_reg *)sptr_data->reg;

    wait_event_interruptible_timeout(&sptr_data->sgtc_wqh, 
                                    mr_imx_i2c_already_comp(sptr_data, sptr_i2c), 
                                    msecs_to_jiffies(timeout));
    sptr_data->is_wake = false;

    return true;
}

/*!
 * @brief   wait for i2c become idle
 * @param   sptr_i2c, ways
 * @retval  none
 * @note    if i2c is busy, schedule to another thread
 */
static kint32_t imx_i2c_adap_for_busy(struct imx_i2c_reg *sptr_i2c, kbool_t ways)
{
    kutime_t expires = jiffies + msecs_to_jiffies(500);

    for (;;)
    {
        /*!< ways = 1: i2c is running, bus should be busy */
        if (ways && sptr_i2c->sgtc_isr.ibb)
            break;

        /*!< ways = 0: i2c is stopped, bus should be idle */
        if (!ways && !sptr_i2c->sgtc_isr.ibb)
            break;

        if (mr_time_after(jiffies, expires))
            return -ER_TIMEOUT;

        schedule_thread();
    }

    return ER_NORMAL;
}

/*!
 * @brief   clear interrupt flag
 * @param   sptr_i2c
 * @retval  none
 * @note    none
 */
#define mr_imx_i2c_clear_intr(sptr_i2c) \
    do {    \
        (sptr_i2c)->sgtc_isr.iif = false; \
        mr_barrier();   \
    } while (0)

/*!
 * @brief   imx_i2c_adap_check_ack
 * @param   none
 * @retval  none
 * @note    check i2c function
 */
static kbool_t imx_i2c_adap_check_ack(struct imx_i2c_reg *sptr_i2c)
{
    /*!< 1. check arbitration loss bit */
    if (sptr_i2c->sgtc_isr.ial)
    {
        sptr_i2c->sgtc_isr.ial = false;

        /*!< reboot i2c */
        sptr_i2c->sgtc_icr.ien = false;
        sptr_i2c->sgtc_icr.ien = true;

        return false;
    }

    /*!< 2. no ack */
    if (NR_I2C_NACK == mr_imx_i2c_get_ack(sptr_i2c))
        return false;

    return true;
}

/*!
 * @brief   imx_i2c_adap_start
 * @param   none
 * @retval  none
 * @note    iic start
 *          when bus is idle, SCL & SDA are high level, if SDA is falling, indicating that i2c will start
 */
static kint32_t imx_i2c_adap_start(struct fwk_i2c_adapter *sptr_adap)
{
    struct imx_i2c_drv_data *sptr_data;
    struct imx_i2c_reg *sptr_i2c;

    sptr_data = fwk_i2c_adapter_get_drvdata(sptr_adap);
    sptr_i2c = (struct imx_i2c_reg *)sptr_data->reg;

    /*!< open clock */
    fwk_clk_prepare_enable(sptr_data->sptr_clk);

    sptr_i2c->sgtc_isr.iif = false;
    sptr_i2c->sgtc_isr.ial = false;

    sptr_i2c->sgtc_icr.ien = true;
    mr_resetw(&sptr_i2c->sgtc_isr);

    delay_ms(1);
    
    /*!< configure iic to work on master mode */
    sptr_i2c->sgtc_icr.msta = true;
    
    /*!< judge if i2c is busy. idle: 0, busy: 1 */
    if (imx_i2c_adap_for_busy(sptr_i2c, true))
        goto fail;

    /*!< set direction to "send" */
    sptr_i2c->sgtc_icr.mtx  = true;
//  sptr_i2c->sgtc_icr.iien = true;

    mr_imx_i2c_set_ack(sptr_i2c, NR_I2C_NACK);

    /*!< wait for transferring finished */
//  if (!imx_i2c_adap_wait_complete(sptr_adap, 100))
//      goto fail;

    return ER_NORMAL;

fail:
    fwk_clk_disable_unprepare(sptr_data->sptr_clk);
    return -ER_NREADY;
}

/*!
 * @brief   imx_i2c_adap_restart
 * @param   none
 * @retval  none
 * @note    iic repeat start
 *          when bus is idle, SCL & SDA are high level, if SDA is falling, indicating that i2c will start
 */
static kint32_t imx_i2c_adap_restart(struct fwk_i2c_adapter *sptr_adap)
{
    struct imx_i2c_drv_data *sptr_data;
    struct imx_i2c_reg *sptr_i2c;

	sptr_data = fwk_i2c_adapter_get_drvdata(sptr_adap);
	sptr_i2c = (struct imx_i2c_reg *)sptr_data->reg;

    /*!< only master mode can send restart signal */
    if (!sptr_i2c->sgtc_icr.msta)
        return -ER_CHECKERR;

    /*!< if i2c is running, bus will be busy; otherwise, i2c bus is stopped */
    if (imx_i2c_adap_for_busy(sptr_i2c, true))
        return -ER_WILDPTR;

    /*!< set restart */
    sptr_i2c->sgtc_icr.rsta = true;
    delay_ms(1);

    return ER_NORMAL;
}

/*!
 * @brief   imx_i2c_adap_stop
 * @param   none
 * @retval  none
 * @note    iic stop
 *          when i2c bus is working, the SDA changes when the SCL is Low; 
 *          otherwise, if the SDA changes when the SCL is high, it indicate that i2c will be stopped;
 *          finally, SDA and SCL will stay high level, i2c bus resumes to idle status
 */
static void imx_i2c_adap_stop(struct fwk_i2c_adapter *sptr_adap)
{    
    struct imx_i2c_drv_data *sptr_data;
    struct imx_i2c_reg *sptr_i2c;

	sptr_data = fwk_i2c_adapter_get_drvdata(sptr_adap);
	sptr_i2c = (struct imx_i2c_reg *)sptr_data->reg;

    /*!< clear configuration */
    sptr_i2c->sgtc_icr.msta = false;
    sptr_i2c->sgtc_icr.mtx  = false;
    sptr_i2c->sgtc_icr.txak = false;

    /*!< wait for becoming not busy */
    imx_i2c_adap_for_busy(sptr_i2c, false);

    sptr_i2c->sgtc_icr.ien = false;
    sptr_i2c->sgtc_icr.iien = false;

    fwk_clk_disable_unprepare(sptr_data->sptr_clk);
}

/*!
 * @brief  imx_i2c_adap_write
 * @param  none
 * @retval none
 * @note   i2c write
 */
static kint32_t imx_i2c_adap_write(struct imx_i2c_drv_data *sptr_data, kuint16_t slave, const kubuffer_t *buffer, kusize_t size)
{
	struct imx_i2c_reg *sptr_i2c;

	sptr_i2c = (struct imx_i2c_reg *)sptr_data->reg;

    /*!< configure direction to send */
    sptr_i2c->sgtc_icr.mtx = true;
    mr_barrier();

    mr_imx_i2c_write_data(sptr_i2c, slave << 1);
    if (!imx_i2c_adap_wait_complete(&sptr_data->sgtc_adap, 10))
        return -ER_TIMEOUT;

    /*!< clear interrupt flag */
    mr_imx_i2c_clear_intr(sptr_i2c);

    if (!imx_i2c_adap_check_ack(sptr_i2c))
        return -ER_NOACK;

    while (size--)
    {
        mr_imx_i2c_write_data(sptr_i2c, *buffer++);

        /*!< wait for transmission finished */
        if (!imx_i2c_adap_wait_complete(&sptr_data->sgtc_adap, 10))
            return -ER_TIMEOUT;

        mr_imx_i2c_clear_intr(sptr_i2c);

        if (!imx_i2c_adap_check_ack(sptr_i2c))
            return -ER_NOACK;

        delay_ms(10);
    }

    return ER_NORMAL;
}

/*!
 * @brief  imx_i2c_adap_read
 * @param  none
 * @retval none
 * @note   i2c read
 */
static kint32_t imx_i2c_adap_read(struct imx_i2c_drv_data *sptr_data, kuint16_t slave, kubuffer_t *buffer, kusize_t size)
{
	struct imx_i2c_reg *sptr_i2c;
    kuint32_t count;

	sptr_i2c = (struct imx_i2c_reg *)sptr_data->reg;

    /*!< configure direction to send (write reg at first) */
    sptr_i2c->sgtc_icr.mtx = true;

    mr_imx_i2c_write_data(sptr_i2c, (slave << 1) | 0x01);
    if (!imx_i2c_adap_wait_complete(&sptr_data->sgtc_adap, 10))
        return -ER_TIMEOUT;

    /*!< clear interrupt flag */
    mr_imx_i2c_clear_intr(sptr_i2c);

    if (!imx_i2c_adap_check_ack(sptr_i2c))
        return -ER_NOACK;

    mr_imx_i2c_set_ack(sptr_i2c, (size > 1) ? NR_I2C_ACK : NR_I2C_NACK);

    /*!< configure direction to recv */
    sptr_i2c->sgtc_icr.mtx = false;

    /*!< read dummy to clear i2c->idr automatically */
    mr_imx_i2c_read_data(sptr_i2c);

    for (count = 0; count < size; count++)
    {
        /*!< wait for transmission finished */
        if (!imx_i2c_adap_wait_complete(&sptr_data->sgtc_adap, 10))
            return -ER_TIMEOUT;

        mr_imx_i2c_clear_intr(sptr_i2c);

        /*!< when only the last data left, reading should be stopped to prevent i2c clock misaligned */
        if (count == (size - 1))
        {
            if (!sptr_data->is_lastMsgs)
                sptr_i2c->sgtc_icr.mtx = true;
            else
            {
                /*!< stop */
                sptr_i2c->sgtc_icr.mtx = false;
                sptr_i2c->sgtc_icr.msta = false;

                if (imx_i2c_adap_for_busy(sptr_i2c, false))
                    return -ER_BUSY;
            }
        }
        /*!< if it is the second to last data, send nack */
        else if (count == (size - 2))
            mr_imx_i2c_set_ack(sptr_i2c, NR_I2C_NACK);
        else
            mr_imx_i2c_set_ack(sptr_i2c, NR_I2C_ACK);

        /*!< Delay to wait slave device stable */
        delay_ms(20);
        *(buffer++) = mr_imx_i2c_read_data(sptr_i2c);
    }

    return ER_NORMAL;
}

/*!
 * @brief  i2c transfer
 * @param  sptr_adap, sptr_msg
 * @retval none
 * @note   read/write
 */
static kint32_t __imx_i2c_adap_xfer(struct fwk_i2c_adapter *sptr_adap, struct fwk_i2c_msg *sptr_msg)
{
	struct imx_i2c_drv_data *sptr_data;

	sptr_data = fwk_i2c_adapter_get_drvdata(sptr_adap);

    if (sptr_msg->flags & FWK_I2C_M_RD)
    {
        if (imx_i2c_adap_read(sptr_data, sptr_msg->addr, sptr_msg->ptr_buf, sptr_msg->len))
            return -ER_RXERR;
    }
    else
    {
        if (imx_i2c_adap_write(sptr_data, sptr_msg->addr, sptr_msg->ptr_buf, sptr_msg->len))
            return -ER_TXERR;
    }

    return ER_NORMAL;
}

/*!
 * @brief  i2c transfer finished --- interrupt handler
 * @param  ptrDev
 * @retval none
 * @note   none
 */
static irq_return_t imx_i2c_adap_isr(kint32_t irq, void *ptrDev)
{
    struct imx_i2c_drv_data *sptr_data;
    struct imx_i2c_reg *sptr_i2c;

	sptr_data = fwk_i2c_adapter_get_drvdata(ptrDev);
	sptr_i2c = (struct imx_i2c_reg *)sptr_data->reg;

    if (sptr_i2c->sgtc_isr.iif)
    {
        sptr_data->is_wake = true;
        mr_imx_i2c_clear_intr(sptr_i2c);

        wake_up_interruptible(&sptr_data->sgtc_wqh);
    }

    return 0;
}

/*!
 * @brief  imx_i2c_adap_initial
 * @param  none
 * @retval none
 * @note   i2c initialization
 */
static void imx_i2c_adap_initial(struct fwk_i2c_adapter *sptr_adap)
{
    struct imx_i2c_drv_data *sptr_data;
    struct imx_i2c_reg *sptr_i2c;
    kint16_t field;

	sptr_data = fwk_i2c_adapter_get_drvdata(sptr_adap);
	sptr_i2c = (struct imx_i2c_reg *)sptr_data->reg;

    /*!< disable i2c */
    sptr_i2c->sgtc_icr.ien = false;

    /*!< write zero to clear */
    mr_resetw(&sptr_i2c->sgtc_isr);

    /*!< config frequency */
    field = imx_i2c_find_frequency(sptr_data->freq);
    if (field < 0)
        return;

    sptr_i2c->sgtc_ifr.freq = field;
    mr_imx_i2c_clear_intr(sptr_i2c);

    /*!< enable i2c */
    sptr_i2c->sgtc_icr.ien = true;

    /*!< clear configuration */
    sptr_i2c->sgtc_icr.msta = false;
    sptr_i2c->sgtc_icr.mtx  = false;
    sptr_i2c->sgtc_icr.txak = false;
    sptr_i2c->sgtc_icr.iien = false;
}

/*!
 * @brief  i2c transfer
 * @param  sptr_adap, sptr_msgs, num (the number of sptr_msgs)
 * @retval none
 * @note   read/write
 */
static kint32_t imx_i2c_adap_xfer(struct fwk_i2c_adapter *sptr_adap, struct fwk_i2c_msg *sptr_msgs, kint32_t num)
{
    static kuint64_t runtick = 0;
	struct imx_i2c_drv_data *sptr_data;
    kuint32_t idx;
    kint32_t retval;

	sptr_data = fwk_i2c_adapter_get_drvdata(sptr_adap);

    /*!< keep a certain interval between two transfer (specially writting) */
    if (runtick > JIFFIES_COUNT())
        msleep(10);
    
    mutex_lock(&sptr_data->sgtc_lock);

    /*!< start */
    retval = imx_i2c_adap_start(sptr_adap);
    if (retval)
        goto out;

    for (idx = 0; idx < num; idx++) 
    {
        sptr_data->is_lastMsgs = ((idx + 1) == num);

        if (idx)
        {
            /*!< repeat */
            retval = imx_i2c_adap_restart(sptr_adap);
            if (retval)
                goto END;
        }

        retval = __imx_i2c_adap_xfer(sptr_adap, sptr_msgs + idx);
        if (retval < 0)
            goto END;
    }

END:
    /*!< stop */
    imx_i2c_adap_stop(sptr_adap);
    sptr_data->is_lastMsgs = false;

out:
    mutex_unlock(&sptr_data->sgtc_lock);
    runtick = JIFFIES_COUNT() + msecs_to_jiffies(10);

    return retval;
}

static const struct fwk_i2c_algo sgtc_imx_i2c_algo = 
{
	.master_xfer = imx_i2c_adap_xfer,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   imx_i2c_driver_probe
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_i2c_driver_probe(struct fwk_platdev *sptr_pdev)
{
	struct imx_i2c_drv_data *sptr_data;
	struct fwk_i2c_adapter *sptr_adap;
	struct fwk_device *sptr_dev;
	kuaddr_t reg;
	kint32_t retval;

	sptr_data = kzalloc(sizeof(*sptr_data), GFP_KERNEL);
	if (!isValid(sptr_data))
		return -ER_NOMEM;

	sptr_dev = &sptr_pdev->sgtc_dev;
	sptr_adap = &sptr_data->sgtc_adap;
    mutex_init(&sptr_data->sgtc_lock);

	reg = fwk_platform_get_address(sptr_pdev, 0);
	sptr_data->reg = fwk_io_remap((void *)reg, ARCH_PER_SIZE);
	if (!sptr_data->reg)
		goto fail1;

    sptr_data->irq = fwk_platform_get_irq(sptr_pdev, 0);
    if (sptr_data->irq < 0)
        goto fail2;

    retval = fwk_of_property_read_u32(sptr_pdev->sgtc_dev.sptr_node, "clock_frequency", &sptr_data->freq);
    if (retval || !sptr_data->freq)
        sptr_data->freq = 100000;

    sptr_data->sptr_clk = fwk_clk_get(sptr_dev, mr_nullptr);
    if (!sptr_data->sptr_clk)
        goto fail2;

	sptr_adap->sgtc_dev.sptr_parent = sptr_dev;
	sptr_adap->sgtc_dev.sptr_node = sptr_dev->sptr_node;

	sptr_adap->sptr_algo = &sgtc_imx_i2c_algo;
	sptr_adap->algo_data = sptr_data;
	sptr_adap->id = (sptr_pdev->id < 0) ? fwk_of_get_alias_id(sptr_dev->sptr_node) : sptr_pdev->id;
	sprintk(sptr_adap->name, "imx,i2c-%d", sptr_adap->id);
    init_waitqueue_head(&sptr_data->sgtc_wqh);

	retval = fwk_i2c_add_adapter(sptr_adap);
	if (retval < 0)
		goto fail3;

	fwk_platform_set_drvdata(sptr_pdev, sptr_data);
    fwk_i2c_adapter_set_drvdata(sptr_adap, sptr_data);

    fwk_clk_prepare_enable(sptr_data->sptr_clk);
    imx_i2c_adap_initial(sptr_adap);
    fwk_clk_disable_unprepare(sptr_data->sptr_clk);

    if (fwk_request_irq(sptr_data->irq, imx_i2c_adap_isr, IRQ_TYPE_NONE, "imx,i2c", sptr_adap))
        goto fail4;

	return ER_NORMAL;

fail4:
    fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);
    fwk_i2c_del_adapter(sptr_adap);
fail3:
    fwk_clk_put(sptr_data->sptr_clk);
fail2:
	fwk_io_unmap(sptr_data->reg);
fail1:
	kfree(sptr_data);

	return -ER_ERROR;
}

/*!
 * @brief   imx_i2c_driver_remove
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_i2c_driver_remove(struct fwk_platdev *sptr_pdev)
{
	struct imx_i2c_drv_data *sptr_data;

    sptr_data = fwk_platform_get_drvdata(sptr_pdev);
    if (!sptr_data)
        return ER_NORMAL;

    fwk_free_irq(sptr_data->irq, &sptr_data->sgtc_adap);
    fwk_i2c_del_adapter(&sptr_data->sgtc_adap);
    fwk_clk_disable_unprepare(sptr_data->sptr_clk);
    fwk_clk_put(sptr_data->sptr_clk);
    fwk_io_unmap(sptr_data->reg);
    kfree(sptr_data);
    fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);

	return ER_NORMAL;
}

/*!< device id for device-tree */
static const struct fwk_of_device_id sgtc_imx_i2c_driver_ids[] =
{
	{ .compatible = "fsl,imx6ul-i2c", },
	{},
};

/*!< platform instance */
static struct fwk_platdrv sgtc_imx_i2c_platdriver =
{
	.probe	= imx_i2c_driver_probe,
	.remove	= imx_i2c_driver_remove,
	
	.sgtc_driver =
	{
		.name 	= "fsl, i2c",
		.id 	= -1,
		.sptr_of_match_table = sgtc_imx_i2c_driver_ids,
	},
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   imx_i2c_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init imx_i2c_driver_init(void)
{
	return fwk_register_platdriver(&sgtc_imx_i2c_platdriver);
}

/*!
 * @brief   imx_i2c_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit imx_i2c_driver_exit(void)
{
	fwk_unregister_platdriver(&sgtc_imx_i2c_platdriver);
}

IMPORT_PATTERN_INIT(imx_i2c_driver_init);
IMPORT_PATTERN_EXIT(imx_i2c_driver_exit);

/*!< end of file */
