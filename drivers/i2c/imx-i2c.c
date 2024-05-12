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
#include <platform/fwk_basic.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/fwk_platdrv.h>
#include <platform/fwk_uaccess.h>
#include <platform/clk/fwk_clk.h>

#include <platform/i2c/fwk_i2c_dev.h>
#include <platform/i2c/fwk_i2c_core.h>
#include <platform/i2c/fwk_i2c_algo.h>

/*!< The defines */
typedef struct imx_i2c_drv_data
{
	struct fwk_i2c_adapter sgrt_adap;
	void *reg;

    struct fwk_clk *sprt_clk;
    kbool_t is_lastMsgs;

    ksint32_t irq;
    kbool_t is_wake;

} srt_imx_i2c_drv_data_t;

/*!< 地址寄存器 */
struct imx_i2c_iar_reg
{
    __IO kuint16_t rw    : 1;                       /*!< bit 0 */
    __IO kuint16_t addr  : 7;                       /*!< bit 1:7 */
    __IO kuint16_t rsvd1 : 8;                       /*!< bit 8:15 */
};

/*!< 频率寄存器 */
struct imx_i2c_ifr_reg
{
    __IO kuint16_t freq  : 6;                       /*!< bit 0:5 */
    __IO kuint16_t rsvd1 : 2;                       /*!< bit 6:7 */
    __IO kuint16_t rsvd2 : 8;                       /*!< bit 8:15 */
};

/*!< 控制寄存器 */
struct imx_i2c_icr_reg
{
    __IO kuint16_t rsvd1 : 2;                       /*!< bit 0:1 保留 */
    __IO kuint16_t rsta  : 1;                       /*!< bit 2   重复开始信号: 1, 产生信号 */
    __IO kuint16_t txak  : 1;                       /*!< bit 3   传输应答使能: 0, ack; 1, noack */
    __IO kuint16_t mtx   : 1;                       /*!< bit 4   传输方向选择: 0, rx;  1, tx */
    __IO kuint16_t msta  : 1;                       /*!< bit 5   主从模式选择: 0, slave; 1, master */
    __IO kuint16_t iien  : 1;                       /*!< bit 6   中断使能, 为1有效 */
    __IO kuint16_t ien   : 1;                       /*!< bit 7   iic使能, 为1有效 */
    __IO kuint16_t rsvd2 : 8;                       /*!< bit 8:15 */
};

/*!< 状态寄存器 */
struct imx_i2c_isr_reg
{
    __IO kuint16_t rxak  : 1;                       /*!< bit 0   应答信号标志位: 0, ack; 1, noack */
    __IO kuint16_t iif   : 1;                       /*!< bit 1   iic中断挂起标志位, 为1有效 */
    __IO kuint16_t rsvd1 : 1;                       /*!< bit 2   保留 */
    __IO kuint16_t srw   : 1;                       /*!< bit 3   从机读写标志位: 0表示给从机写数据, 1表示读数据 */
    __IO kuint16_t ial   : 1;                       /*!< bit 4   仲裁丢失位, 为1有效 */
    __IO kuint16_t ibb   : 1;                       /*!< bit 5   iic总线忙标志位, 为1表示忙, 0表示空闲 */
    __IO kuint16_t iaas  : 1;                       /*!< bit 6   为1表示iar.addr为从设备地址 */
    __IO kuint16_t icf   : 1;                       /*!< bit 7   数据传输状态位: 0, 正在传输; 1, 传输完成 */
    __IO kuint16_t rsvd2 : 8;                       /*!< bit 8:15 */
};

/*!< 数据寄存器 */
struct imx_i2c_idr_reg
{
    __IO kuint16_t data  : 8;                       /*!< bit 0:7 */
    __IO kuint16_t rsvd1 : 8;                       /*!< bit 8:15 */
};

__align(4) struct imx_i2c_reg
{
	struct imx_i2c_iar_reg sgrt_iar;
	struct imx_i2c_ifr_reg sgrt_ifr;
	struct imx_i2c_icr_reg sgrt_icr;
	struct imx_i2c_isr_reg sgrt_isr;
	struct imx_i2c_idr_reg sgrt_idr;
};

enum __ERT_IMX_I2C_ACK_STATUS
{
    NR_I2C_ACK = 0,
    NR_I2C_NACK = 1,
};

/*!< API function */
static void imx_i2c_adap_set_ack(struct imx_i2c_reg *sprt_i2c, kuint32_t ack)
{
    if (ack == NR_I2C_NACK)
        sprt_i2c->sgrt_icr.txak = true;
    else if (ack == NR_I2C_ACK)
        sprt_i2c->sgrt_icr.txak = false;
}

static kuint32_t imx_i2c_adap_get_ack(struct imx_i2c_reg *sprt_i2c)
{
    return sprt_i2c->sgrt_isr.rxak ? NR_I2C_NACK : NR_I2C_ACK;
}

static void imx_i2c_adap_write_data(struct imx_i2c_reg *sprt_i2c, kuint8_t data)
{
    sprt_i2c->sgrt_idr.data = data;
}

static kuint8_t imx_i2c_adap_read_data(struct imx_i2c_reg *sprt_i2c)
{
    return sprt_i2c->sgrt_idr.data;
}

static kbool_t imx_i2c_adap_wait_complete(struct fwk_i2c_adapter *sprt_adap, kuint32_t timeout)
{
    struct imx_i2c_drv_data *sprt_data;
    struct imx_i2c_reg *sprt_i2c;

	sprt_data = fwk_i2c_adapter_get_drvdata(sprt_adap);
	sprt_i2c = (struct hal_imx_i2c *)sprt_data->reg;

    while (timeout--) 
    {
        if (sprt_i2c->sgrt_isr.icf || sprt_data->is_wake)
            return true;
    }

    return false;
}

static kbool_t imx_i2c_adap_is_busy(struct imx_i2c_reg *sprt_i2c)
{
    kuint32_t timeout = 100;

    while (timeout--)
    {
        if (!sprt_i2c->sgrt_isr.ibb)
            return false;
    }

    return true;
}

static void imx_i2c_adap_clear_intr(struct imx_i2c_reg *sprt_i2c)
{
    sprt_i2c->sgrt_isr.iif = false;
}

/*!
 * @brief  imx_i2c_adap_check_ack
 * @param  None
 * @retval None
 * @note   iic功能检查
 */
static kbool_t imx_i2c_adap_check_ack(struct imx_i2c_reg *sprt_i2c)
{
    /*!< 1. 仲裁丢失位检查 */
    if (sprt_i2c->sgrt_isr.ial)
    {
        sprt_i2c->sgrt_isr.ial = false;

        /*!< 应重启iic */
        sprt_i2c->sgrt_icr.ien = false;
        sprt_i2c->sgrt_icr.ien = true;

        return false;
    }

    /*!< 2. 无应答 */
    if (NR_I2C_NACK == imx_i2c_adap_get_ack(sprt_i2c))
        return false;

    return true;
}

/*!
 * @brief  imx_i2c_adap_start
 * @param  None
 * @retval None
 * @note   iic起始位
 * 总线空闲时, SCL & SDA均为高电平, 此时若SDA产生下降沿, 表示iic开始
 */
static ksint32_t imx_i2c_adap_start(struct fwk_i2c_adapter *sprt_adap)
{
    struct imx_i2c_drv_data *sprt_data;
    struct imx_i2c_reg *sprt_i2c;

	sprt_data = fwk_i2c_adapter_get_drvdata(sprt_adap);
	sprt_i2c = (struct hal_imx_i2c *)sprt_data->reg;

    /*!< open clock */
    fwk_clk_prepare_enable(sprt_data->sprt_clk);

    sprt_i2c->sgrt_icr.ien = true;
    mrt_resetw(&sprt_i2c->sgrt_isr);

    /*!< 判断iic总线是否处于忙状态. 空闲: 0, 忙: 1 */
    if (imx_i2c_adap_is_busy(sprt_i2c))
        goto fail;

    /*!< 设置iic工作在主机模式, 且传输方向为发送 */
    sprt_i2c->sgrt_icr.msta = true;
    sprt_i2c->sgrt_icr.mtx  = true;
//  sprt_i2c->sgrt_icr.iien = true;

    imx_i2c_adap_set_ack(sprt_i2c, NR_I2C_NACK);

    /*!< 等待传输完成 */
    if (!imx_i2c_adap_wait_complete(sprt_adap, 100))
        goto fail;

    return NR_IS_NORMAL;

fail:
    fwk_clk_disable_unprepare(sprt_data->sprt_clk);
    return -NR_IS_NREADY;
}

/*!
 * @brief  imx_i2c_adap_restart
 * @param  None
 * @retval None
 * @note   iic重新开始信号
 * 总线空闲时, SCL & SDA均为高电平, 此时若SDA产生下降沿, 表示iic开始
 */
static ksint32_t imx_i2c_adap_restart(struct fwk_i2c_adapter *sprt_adap)
{
    struct imx_i2c_drv_data *sprt_data;
    struct imx_i2c_reg *sprt_i2c;

	sprt_data = fwk_i2c_adapter_get_drvdata(sprt_adap);
	sprt_i2c = (struct hal_imx_i2c *)sprt_data->reg;

    /*!< 判断iic总线是否处于忙状态. 空闲: 0, 忙: 1 */
    if (imx_i2c_adap_is_busy(sprt_i2c))
        return -NR_IS_NREADY;

    /*!< 仅主机模式可以控制重新发送信号 */
    if (!sprt_i2c->sgrt_icr.msta)
        return -NR_IS_CHECKERR;

    /*!< 传输方向为发送, 并重新发送 */
    sprt_i2c->sgrt_icr.mtx  = true;
    sprt_i2c->sgrt_icr.rsta = true;

    /*!< 等待传输完成 */
    if (imx_i2c_adap_wait_complete(sprt_adap, 100))
        return NR_IS_NORMAL;

    return -NR_IS_TIMEOUT;
}

/*!
 * @brief  imx_i2c_adap_stop
 * @param  None
 * @retval None
 * @note   iic停止位
 * 总线工作时, SCL低电平时SDA变化; 若SCL高电平时SDA变化(上升沿), 表示iic停止;
 * 此时SCL & SDA回到高电平状态, 总线恢复空闲
 */
static void imx_i2c_adap_stop(struct fwk_i2c_adapter *sprt_adap)
{    
    struct imx_i2c_drv_data *sprt_data;
    struct imx_i2c_reg *sprt_i2c;

	sprt_data = fwk_i2c_adapter_get_drvdata(sprt_adap);
	sprt_i2c = (struct hal_imx_i2c *)sprt_data->reg;

    /*!< 清除设置 */
    sprt_i2c->sgrt_icr.msta = false;
    sprt_i2c->sgrt_icr.mtx  = false;
    sprt_i2c->sgrt_icr.txak = false;

    /*!< 等待忙结束: 循环比较费时间 */
    imx_i2c_adap_is_busy(sprt_i2c);

    sprt_i2c->sgrt_icr.ien = false;
    sprt_i2c->sgrt_icr.iien = false;

    fwk_clk_disable_unprepare(sprt_data->sprt_clk);
}

/*!
 * @brief  imx_i2c_adap_write
 * @param  None
 * @retval None
 * @note   iic写时序
 */
static ksint32_t imx_i2c_adap_write(struct imx_i2c_drv_data *sprt_data, kuint16_t slave, const kubuffer_t *buffer, kusize_t size)
{
	struct imx_i2c_reg *sprt_i2c;

	sprt_i2c = (struct hal_imx_i2c *)sprt_data->reg;

    /*!< 设置传输方向为发送 */
    sprt_i2c->sgrt_icr.mtx = true;

    imx_i2c_adap_write_data(sprt_i2c, slave << 1);
    if (!imx_i2c_adap_wait_complete(&sprt_data->sgrt_adap, 100))
        return -NR_IS_TIMEOUT;

    /* 手动清除中断标志位 */
    imx_i2c_adap_clear_intr(sprt_i2c);

    if (!imx_i2c_adap_check_ack(sprt_i2c))
        return -NR_IS_RDATA_FAILD;

    while (size--)
    {
        imx_i2c_adap_write_data(sprt_i2c, *buffer++);

        /*!< 等待数据传输完成 */
        if (!imx_i2c_adap_wait_complete(&sprt_data->sgrt_adap, 100))
            return -NR_IS_TIMEOUT;

        imx_i2c_adap_clear_intr(sprt_i2c);

        if (!imx_i2c_adap_check_ack(sprt_i2c))
            return -NR_IS_RDATA_FAILD;
    }

    return NR_IS_NORMAL;
}

/*!
 * @brief  imx_i2c_adap_read
 * @param  None
 * @retval None
 * @note   iic读时序
 */
static ksint32_t imx_i2c_adap_read(struct imx_i2c_drv_data *sprt_data, kuint16_t slave, kubuffer_t *buffer, kusize_t size)
{
	struct imx_i2c_reg *sprt_i2c;
    kuint32_t count;

	sprt_i2c = (struct hal_imx_i2c *)sprt_data->reg;

    /*!< 设置传输方向为发送 */
    sprt_i2c->sgrt_icr.mtx = true;

    imx_i2c_adap_write_data(sprt_i2c, (slave << 1) | 0x01);
    if (!imx_i2c_adap_wait_complete(&sprt_data->sgrt_adap, 100))
        return -NR_IS_TIMEOUT;

    /*!< 清除中断标志位 */
    imx_i2c_adap_clear_intr(sprt_i2c);

    if (!imx_i2c_adap_check_ack(sprt_i2c))
        return -NR_IS_RDATA_FAILD;

    imx_i2c_adap_set_ack(sprt_i2c, (size > 1) ? NR_I2C_ACK : NR_I2C_NACK);

    /*!< 设置传输方向为接收 */
    sprt_i2c->sgrt_icr.mtx = false;

    /*!< 空读数据, 可清除data里面的内容 */
    imx_i2c_adap_read_data(sprt_i2c);

    for (count = 0; count < size; count++)
    {
        /*!< 等待数据传输完成 */
        if (!imx_i2c_adap_wait_complete(&sprt_data->sgrt_adap, 100))
            return -NR_IS_TIMEOUT;

        imx_i2c_adap_clear_intr(sprt_i2c);

        /*!< 当只剩最后一个数据时, 应终止读, 以防i2c时钟错乱 */
        if (count == (size - 1))
        {
            if (!sprt_data->is_lastMsgs)
                sprt_i2c->sgrt_icr.mtx = true;
            else
            {
                /*!< 停止 */
                sprt_i2c->sgrt_icr.mtx = false;
                sprt_i2c->sgrt_icr.msta = false;

                if (imx_i2c_adap_is_busy(sprt_i2c))
                    return -NR_IS_BUSY;
            }
        }
        /*!< 倒数第2个数据, 发送nack */
        else if (count == (size - 2))
            imx_i2c_adap_set_ack(sprt_i2c, NR_I2C_NACK);
        else
            imx_i2c_adap_set_ack(sprt_i2c, NR_I2C_ACK);

        *(buffer++) = imx_i2c_adap_read_data(sprt_i2c);
    }

    return NR_IS_NORMAL;
}

static ksint32_t __imx_i2c_adap_xfer(struct fwk_i2c_adapter *sprt_adap, struct fwk_i2c_msg *sprt_msg)
{
	struct imx_i2c_drv_data *sprt_data;

	sprt_data = fwk_i2c_adapter_get_drvdata(sprt_adap);

    if (sprt_msg->flags & FWK_I2C_M_RD)
    {
        if (imx_i2c_adap_read(sprt_data, sprt_msg->addr, sprt_msg->ptr_buf, sprt_msg->len))
            return -NR_IS_TRXERR;
    }
    else
        imx_i2c_adap_write(sprt_data, sprt_msg->addr, sprt_msg->ptr_buf, sprt_msg->len);

    return NR_IS_NORMAL;
}

static irq_return_t imx_i2c_adap_handler(ksint32_t irq, void *ptrDev)
{
    struct imx_i2c_drv_data *sprt_data;
    struct imx_i2c_reg *sprt_i2c;

	sprt_data = fwk_i2c_adapter_get_drvdata(ptrDev);
	sprt_i2c = (struct hal_imx_i2c *)sprt_data->reg;

    if (sprt_i2c->sgrt_isr.iif)
    {
        sprt_data->is_wake = true;
        imx_i2c_adap_clear_intr(sprt_i2c);
    }

    return 0;
}

/*!
 * @brief  imx_i2c_adap_initial
 * @param  None
 * @retval None
 * @note   初始化iic
 */
static void imx_i2c_adap_initial(struct fwk_i2c_adapter *sprt_adap)
{
    struct imx_i2c_drv_data *sprt_data;
    struct imx_i2c_reg *sprt_i2c;

	sprt_data = fwk_i2c_adapter_get_drvdata(sprt_adap);
	sprt_i2c = (struct hal_imx_i2c *)sprt_data->reg;

    /*!< 关闭iic */
    sprt_i2c->sgrt_icr.ien  = false;

    mrt_resetw(&sprt_i2c->sgrt_isr);

    /*!< 设置波特率为100KHz */
    sprt_i2c->sgrt_ifr.freq = 0X15;

    /*!< 启用iic */
    sprt_i2c->sgrt_icr.ien  = true;
}

static ksint32_t imx_i2c_adap_xfer(struct fwk_i2c_adapter *sprt_adap, struct fwk_i2c_msg *sprt_msgs, ksint32_t num)
{
	struct imx_i2c_drv_data *sprt_data;
	struct imx_i2c_reg *sprt_i2c;
    kuint32_t idx;
    ksint32_t retval = -NR_IS_TRXERR;

	sprt_data = fwk_i2c_adapter_get_drvdata(sprt_adap);
	sprt_i2c = (struct hal_imx_i2c *)sprt_data->reg;

    /*!< 等待数据传输完成 */
    if (!imx_i2c_adap_wait_complete(sprt_adap, 100))
        return -NR_IS_TIMEOUT;

    /*!< 清除各种标志位, 开启新一轮传输 */
    sprt_i2c->sgrt_isr.ial = false;
    sprt_i2c->sgrt_isr.iif = false;

    /*!< 开始信号 */
    retval = imx_i2c_adap_start(sprt_adap);
    if (retval)
        return retval;

    /*!< iic功能检查 & 应答信号*/
    if (!imx_i2c_adap_check_ack(sprt_i2c)) 
    {
        retval = -NR_IS_CHECKERR;
        goto END;
    }

    for (idx = 0; idx < num; idx++) 
    {
        sprt_data->is_lastMsgs = ((idx + 1) == num);

        if (idx)
        {
            retval = -NR_IS_TRXERR;

            /*!< 重新开始 */
            imx_i2c_adap_restart(sprt_adap);

            /*!< iic功能检查 & 应答信号*/
            if (!imx_i2c_adap_check_ack(sprt_i2c))
                goto END;

            if (imx_i2c_adap_is_busy(sprt_i2c))
                goto END;
        }

        retval = __imx_i2c_adap_xfer(sprt_adap, sprt_msgs + idx);
        if (retval < 0)
            goto END;
    }

END:
    /*!< 停止信号 */
    imx_i2c_adap_stop(sprt_adap);
    sprt_data->is_lastMsgs = false;

    return retval;
}

static const struct fwk_i2c_algo sgrt_imx_i2c_algo = 
{
	.master_xfer = imx_i2c_adap_xfer,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   imx_i2c_driver_probe
 * @param   sprt_dev
 * @retval  errno
 * @note    none
 */
static ksint32_t imx_i2c_driver_probe(struct fwk_platdev *sprt_pdev)
{
	struct imx_i2c_drv_data *sprt_data;
	struct fwk_i2c_adapter *sprt_adap;
	struct fwk_device *sprt_dev;
	kuaddr_t reg;
	ksint32_t retval;

	sprt_data = kzalloc(sizeof(*sprt_data), GFP_KERNEL);
	if (!isValid(sprt_data))
		return -NR_IS_NOMEM;

	sprt_dev = &sprt_pdev->sgrt_dev;
	sprt_adap = &sprt_data->sgrt_adap;

	reg = fwk_platform_get_address(sprt_pdev, 0);
	sprt_data->reg = fwk_io_remap((void *)reg);
	if (!sprt_data->reg)
		goto fail1;

    sprt_data->irq = fwk_platform_get_irq(sprt_pdev, 0);
    if (sprt_data->irq < 0)
        goto fail2;

    sprt_data->sprt_clk = fwk_clk_get(sprt_dev, mrt_nullptr);
    if (!sprt_data->sprt_clk)
        goto fail2;

	sprt_adap->sgrt_dev.sprt_parent = sprt_dev;
	sprt_adap->sgrt_dev.sprt_node = sprt_dev->sprt_node;

	sprt_adap->sprt_algo = &sgrt_imx_i2c_algo;
	sprt_adap->algo_data = sprt_data;
	sprt_adap->id = fwk_of_get_id(sprt_dev->sprt_node, "i2c");
	sprintk(sprt_adap->name, "imx,i2c-%d", sprt_adap->id);

	retval = fwk_i2c_add_adapter(sprt_adap);
	if (retval < 0)
		goto fail3;

	fwk_platform_set_drvdata(sprt_pdev, sprt_data);
    fwk_i2c_adapter_set_drvdata(sprt_adap, sprt_data);

    fwk_clk_prepare_enable(sprt_data->sprt_clk);
    imx_i2c_adap_initial(sprt_adap);
    fwk_clk_disable_unprepare(sprt_data->sprt_clk);

    if (fwk_request_irq(sprt_data->irq, imx_i2c_adap_handler, IRQ_TYPE_NONE, "imx,i2c", sprt_adap))
        goto fail3;

	return NR_IS_NORMAL;

fail3:
    fwk_clk_put(sprt_data->sprt_clk);
fail2:
	fwk_io_unmap(sprt_data->reg);
fail1:
	kfree(sprt_data);

	return -NR_IS_ERROR;
}

/*!
 * @brief   imx_i2c_driver_remove
 * @param   sprt_dev
 * @retval  errno
 * @note    none
 */
static ksint32_t imx_i2c_driver_remove(struct fwk_platdev *sprt_pdev)
{
	struct imx_i2c_drv_data *sprt_data;

    sprt_data = fwk_platform_get_drvdata(sprt_pdev);

    fwk_free_irq(sprt_data->irq, &sprt_data->sgrt_adap);
    fwk_i2c_del_adapter(&sprt_data->sgrt_adap);
    fwk_clk_disable_unprepare(sprt_data->sprt_clk);
    fwk_clk_put(sprt_data->sprt_clk);
    fwk_io_unmap(sprt_data->reg);
    kfree(sprt_data);
    fwk_platform_set_drvdata(sprt_pdev, mrt_nullptr);

	return NR_IS_NORMAL;
}

/*!< device id for device-tree */
static const struct fwk_of_device_id sgrt_imx_i2c_driver_ids[] =
{
	{ .compatible = "fsl,imx6ul-i2c", },
	{},
};

/*!< platform instance */
static struct fwk_platdrv sgrt_imx_i2c_platdriver =
{
	.probe	= imx_i2c_driver_probe,
	.remove	= imx_i2c_driver_remove,
	
	.sgrt_driver =
	{
		.name 	= "fsl, i2c",
		.id 	= -1,
		.sprt_of_match_table = sgrt_imx_i2c_driver_ids,
	},
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   imx_i2c_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
ksint32_t __fwk_init imx_i2c_driver_init(void)
{
	return fwk_register_platdriver(&sgrt_imx_i2c_platdriver);
}

/*!
 * @brief   imx_i2c_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit imx_i2c_driver_exit(void)
{
	fwk_unregister_platdriver(&sgrt_imx_i2c_platdriver);
}

IMPORT_DRIVER_INIT(imx_i2c_driver_init);
IMPORT_DRIVER_EXIT(imx_i2c_driver_exit);

/*!< end of file */
