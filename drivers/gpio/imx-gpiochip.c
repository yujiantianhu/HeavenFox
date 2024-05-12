/*
 * Template of Character Device : LED
 *
 * File Name:   imx_gpiochip.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.11.26
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/fwk_platdrv.h>
#include <platform/gpio/fwk_gpiochip.h>
#include <platform/gpio/fwk_gpiodesc.h>

#include <asm/imx6/imx6ull_periph.h>

/*!< The defines */
typedef struct imx_gpio_port
{
	void *base;
	ksint32_t id;
	ksint32_t irq_high;
	ksint32_t irq_low;
	struct fwk_gpio_chip sgrt_gpiochip;
	struct fwk_irq_domain *sprt_irqdomain;

	struct list_head sgrt_link;

} srt_imx_gpio_port_t;

/*!< The globals */
static DECLARE_LIST_HEAD(sgrt_imx_gpio_ports_list);

/*!< API function */
/*!
 * @brief   imx_gpiochip_driver_open
 * @param   sprt_inode, sprt_file
 * @retval  errno
 * @note    none
 */
static irq_return_t imx_gpiochip_driver_handler(void *ptrDev)
{
	srt_imx_gpio_port_t *sprt_port;
	srt_fwk_irq_domain_t *sprt_domain;
	srt_hal_imx_gpio_t *sprt_reg;
	kbool_t irq_ena, irq_sta;
	kuint32_t idx, irq;

	sprt_port = (srt_imx_gpio_port_t *)ptrDev;
	sprt_reg = (srt_hal_imx_gpio_t *)sprt_port->base;
	sprt_domain = sprt_port->sprt_irqdomain;
	
	for (idx = 0; idx < 32; idx++)
	{
		irq_ena = !!mrt_getbitl(mrt_bit(idx), &sprt_reg->IMR);
		irq_sta = !!mrt_getbitl(mrt_bit(idx), &sprt_reg->ISR);

		if (!irq_ena || !irq_sta)
			continue;
		
		irq = fwk_irq_get_by_domain(sprt_domain, idx);
		fwk_do_irq_handler(irq);

		/*!< clear status bit */
		mrt_setbitl(mrt_bit(idx), &sprt_reg->ISR);
	}

	return NR_IS_NORMAL;
}

static ksint32_t imx_gpiochip_driver_get_id(srt_fwk_device_node_t *sprt_node)
{
	return fwk_of_get_id(sprt_node, "gpio");
}

/*!
 * @brief   hardware irqnumber translate
 * @param   sprt_domain: parent interrupt controller
 * @param   sprt_intc: parent interrupt controller's device-node
 * @param   intspec: of_handle_args::args[]
 * @param   intsize: property "interrupts" cell number
 * @retval  none
 * @note    none
 */
static ksint32_t imx_gpiochip_irq_domain_xlate(struct fwk_irq_domain *sprt_domain, struct fwk_device_node *sprt_intc,
				const kuint32_t *intspec, kuint32_t intsize, kuint32_t *out_hwirq, kuint32_t *out_type)
{
	if (sprt_domain->sprt_node != sprt_intc)
		return -NR_IS_UNVALID;
	if (intsize != 2)
		return -NR_IS_UNVALID;
	if (intspec[0] >= 32)
		return -NR_IS_UNVALID;

	/*!< gpio: 0 ~ 31 */
	*out_hwirq = intspec[0];
    *out_type = intspec[1];

	return NR_IS_NORMAL;
}

static const srt_fwk_irq_domain_ops_t sgrt_imx_gpiochip_hierarchy_ops = 
{
	.xlate = imx_gpiochip_irq_domain_xlate,
	.alloc = mrt_nullptr,
	.free = mrt_nullptr,
};

static ksint32_t imx_gpiochip_driver_probe_intc(struct fwk_platdev *sprt_pdev)
{
	srt_imx_gpio_port_t *sprt_port;
	srt_fwk_device_node_t *sprt_node, *sprt_par;
	srt_fwk_irq_domain_t *sprt_intc;
	srt_fwk_irq_domain_t *sprt_domain;
	ksint32_t virq, irq_base;

	sprt_node = sprt_pdev->sgrt_dev.sprt_node;
	sprt_port = fwk_platform_get_drvdata(sprt_pdev);

	if (fwk_of_find_property(sprt_node, "interrupt-controller", mrt_nullptr))
		return -NR_IS_NSUPPORT;

	sprt_par = fwk_of_irq_parent(sprt_node);
	if (!isValid(sprt_par))
		return -NR_IS_NOTFOUND;
	
	sprt_intc = fwk_of_irq_host(sprt_par);
	if (!isValid(sprt_intc))
		return -NR_IS_NOTFOUND;

	sprt_domain = fwk_irq_domain_add_hierarchy(sprt_intc, sprt_node, 
									32, &sgrt_imx_gpiochip_hierarchy_ops, mrt_nullptr);
	if (!isValid(sprt_domain))
		return -NR_IS_FAILD;

	irq_base = sprt_port->id << 5;
	virq = fwk_irq_domain_alloc_irqs(sprt_domain, irq_base, 0, 32);
	if (virq < 0)
		goto fail;

	sprt_port->sprt_irqdomain = sprt_domain;

	return NR_IS_NORMAL;

fail:
	fwk_irq_domain_del_hierarchy(sprt_domain);
	kfree(sprt_domain);

	return -NR_IS_ERROR;
}

static void imx_gpiochip_driver_remove_intc(struct fwk_platdev *sprt_pdev)
{
	srt_imx_gpio_port_t *sprt_port;
	srt_fwk_irq_domain_t *sprt_domain;

	sprt_port = fwk_platform_get_drvdata(sprt_pdev);
	sprt_domain = sprt_port->sprt_irqdomain;
	sprt_port->sprt_irqdomain = mrt_nullptr;

	fwk_irq_domain_free_irqs(sprt_domain, true);
	fwk_irq_domain_del_hierarchy(sprt_domain);
	kfree(sprt_domain);
}

static ksint32_t imx_gpiochip_driver_to_irq(srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset)
{
	srt_imx_gpio_port_t *sprt_port;
	srt_fwk_irq_domain_t *sprt_domain;

	if (!sprt_chip || (offset >= sprt_chip->ngpios))
		return -NR_IS_NODEV;

	sprt_port = mrt_container_of(sprt_chip, srt_imx_gpio_port_t, sgrt_gpiochip);
	sprt_domain = sprt_port->sprt_irqdomain;

	return fwk_irq_domain_find_map(sprt_domain, offset, 0);
}

static ksint32_t imx_gpiochip_driver_get_direction(srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset)
{
	srt_imx_gpio_port_t *sprt_port;
	srt_hal_imx_gpio_t *sprt_reg;

	if (!sprt_chip || (offset >= sprt_chip->ngpios))
		return -NR_IS_NODEV;

	sprt_port = mrt_container_of(sprt_chip, srt_imx_gpio_port_t, sgrt_gpiochip);
	sprt_reg = (srt_hal_imx_gpio_t *)sprt_port->base;
	if (!sprt_reg)
		return -NR_IS_NODEV;

    return mrt_getbitl(mrt_bit(offset), &sprt_reg->GDIR) ? FWK_GPIO_DIR_OUT : FWK_GPIO_DIR_IN;
}

static ksint32_t imx_gpiochip_driver_direction_input(srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset)
{
	srt_imx_gpio_port_t *sprt_port;
	srt_hal_imx_gpio_t *sprt_reg;

	if (!sprt_chip || (offset >= sprt_chip->ngpios))
		return -NR_IS_NODEV;

	sprt_port = mrt_container_of(sprt_chip, srt_imx_gpio_port_t, sgrt_gpiochip);
	sprt_reg = (srt_hal_imx_gpio_t *)sprt_port->base;
	if (!sprt_reg)
		return -NR_IS_NODEV;
	
	mrt_clrbitl(mrt_bit(offset), &sprt_reg->GDIR);

    return NR_IS_NORMAL;
}

static ksint32_t imx_gpiochip_driver_direction_output(srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset, ksint32_t value)
{
	srt_imx_gpio_port_t *sprt_port;
	srt_hal_imx_gpio_t *sprt_reg;

	if (!sprt_chip || (offset >= sprt_chip->ngpios))
		return -NR_IS_NODEV;

	sprt_port = mrt_container_of(sprt_chip, srt_imx_gpio_port_t, sgrt_gpiochip);
	sprt_reg = (srt_hal_imx_gpio_t *)sprt_port->base;
	if (!sprt_reg)
		return -NR_IS_NODEV;
	
	mrt_setbitl(mrt_bit(offset), &sprt_reg->GDIR);

	if ((value & FWK_GPIO_ACTIVE_HIGH) && sprt_chip->set)
		sprt_chip->set(sprt_chip, offset, true);

    return NR_IS_NORMAL;
}

static ksint32_t imx_gpiochip_driver_get(srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset)
{
	srt_imx_gpio_port_t *sprt_port;
	srt_hal_imx_gpio_t *sprt_reg;

	if (!sprt_chip || (offset >= sprt_chip->ngpios))
		return -NR_IS_NODEV;

	sprt_port = mrt_container_of(sprt_chip, srt_imx_gpio_port_t, sgrt_gpiochip);
	sprt_reg = (srt_hal_imx_gpio_t *)sprt_port->base;
	if (!sprt_reg)
		return -NR_IS_NODEV;

    return !!mrt_getbitl(mrt_bit(offset), &sprt_reg->DR);
}

static void imx_gpiochip_driver_set(srt_fwk_gpio_chip_t *sprt_chip, kuint32_t offset, ksint32_t value)
{
	srt_imx_gpio_port_t *sprt_port;
	srt_hal_imx_gpio_t *sprt_reg;

	if (!sprt_chip || (offset >= sprt_chip->ngpios) || (value < 0))
		return;

	sprt_port = mrt_container_of(sprt_chip, srt_imx_gpio_port_t, sgrt_gpiochip);
	sprt_reg = (srt_hal_imx_gpio_t *)sprt_port->base;
	if (!sprt_reg)
		return;

	if (value)
		mrt_setbitl(mrt_bit(offset), &sprt_reg->DR);
	else
		mrt_clrbitl(mrt_bit(offset), &sprt_reg->DR);
}

static ksint32_t imx_gpiochip_driver_probe_gc(struct fwk_platdev *sprt_pdev)
{
	srt_imx_gpio_port_t *sprt_port;
	srt_fwk_device_node_t *sprt_node;
	srt_fwk_gpio_chip_t *sprt_gc;
	ksint32_t gpio_base;
	kuint32_t count = 0;

	sprt_node = sprt_pdev->sgrt_dev.sprt_node;
	sprt_port = fwk_platform_get_drvdata(sprt_pdev);

	if (fwk_of_find_property(sprt_node, "gpio-controller", mrt_nullptr))
		return -NR_IS_NSUPPORT;

	if (fwk_of_property_read_u32(sprt_node, "#gpio-cells", &count))
		return -NR_IS_NSUPPORT;

	sprt_gc = &sprt_port->sgrt_gpiochip;
	gpio_base = sprt_port->id << 5;

	if (fwk_gpiochip_init(sprt_gc, & sprt_pdev->sgrt_dev, gpio_base, 32 >> 3))
		return -NR_IS_FAILD;

	sprt_gc->of_gpio_n_cells = count;
	sprt_gc->to_irq = imx_gpiochip_driver_to_irq;
	sprt_gc->get_direction = imx_gpiochip_driver_get_direction;
	sprt_gc->direction_input = imx_gpiochip_driver_direction_input;
	sprt_gc->direction_output = imx_gpiochip_driver_direction_output;
	sprt_gc->get = imx_gpiochip_driver_get;
	sprt_gc->set = imx_gpiochip_driver_set;

	return fwk_gpiochip_add(sprt_gc);
}

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   imx_gpiochip_driver_probe
 * @param   sprt_dev
 * @retval  errno
 * @note    none
 */
static ksint32_t imx_gpiochip_driver_probe(struct fwk_platdev *sprt_pdev)
{
	srt_imx_gpio_port_t *sprt_port;
	srt_fwk_device_node_t *sprt_node;
	srt_hal_imx_gpio_t *sprt_reg;

	sprt_node = sprt_pdev->sgrt_dev.sprt_node;

	sprt_port = kzalloc(sizeof(*sprt_port), GFP_KERNEL);
	if (!isValid(sprt_port))
		return -NR_IS_NOMEM;

	/*!< id = 0 ~ 4, e.g. gpio1 ~ gpio5 */
	sprt_port->id = imx_gpiochip_driver_get_id(sprt_node);
	if (sprt_port->id < 0)
		goto fail1;

	/*!< get gpio register base address */
	sprt_port->base = fwk_of_iomap(sprt_node, 0);
	sprt_port->base = fwk_io_remap(sprt_port->base);
	if (!isValid(sprt_port->base))
		goto fail1;

	/*!< get gpio interrupt number */
	sprt_port->irq_low = fwk_platform_get_irq(sprt_pdev, 0);
	sprt_port->irq_high = fwk_platform_get_irq(sprt_pdev, 1);

	/*!< request irq (the domain is gpc) */
	if (sprt_port->irq_low >= 0)
	{
		if (fwk_request_irq(sprt_port->irq_low, imx_gpiochip_driver_handler, IRQ_TYPE_NONE, "imx-gpio-low", sprt_port))
			goto fail2;
	}
	
	if (sprt_port->irq_high >= 0)
	{
		if (fwk_request_irq(sprt_port->irq_high, imx_gpiochip_driver_handler, IRQ_TYPE_NONE, "imx-gpio-high", sprt_port))
			goto fail3;
	}

	/*!< initial gpio register */
	sprt_reg = (srt_hal_imx_gpio_t *)sprt_port->base;
	mrt_writel(~0, &sprt_reg->ISR);
	mrt_writel(0, &sprt_reg->IMR);

	fwk_platform_set_drvdata(sprt_pdev, sprt_port);

	/*!< gpio is also a irq-controller */
	if (imx_gpiochip_driver_probe_intc(sprt_pdev))
		goto fail4;

	/*!< set to be a gpio-controller */
	if (imx_gpiochip_driver_probe_gc(sprt_pdev))
		goto fail5;

	list_head_add_tail(&sgrt_imx_gpio_ports_list, &sprt_port->sgrt_link);

	return NR_IS_NORMAL;

fail5:
	imx_gpiochip_driver_remove_intc(sprt_pdev);
fail4:
	fwk_platform_set_drvdata(sprt_pdev, mrt_nullptr);
	if (sprt_port->irq_high >= 0)
		fwk_free_irq(sprt_port->irq_high, sprt_port);
fail3:
	if (sprt_port->irq_low >= 0)
		fwk_free_irq(sprt_port->irq_low, sprt_port);
fail2:
	fwk_io_unmap(sprt_port->base);
fail1:
	kfree(sprt_port);
	return -NR_IS_ERROR;
}

/*!
 * @brief   imx_gpiochip_driver_remove
 * @param   sprt_dev
 * @retval  errno
 * @note    none
 */
static ksint32_t imx_gpiochip_driver_remove(struct fwk_platdev *sprt_pdev)
{
	srt_imx_gpio_port_t *sprt_port;

	sprt_port = fwk_platform_get_drvdata(sprt_pdev);

	imx_gpiochip_driver_remove_intc(sprt_pdev);
	fwk_platform_set_drvdata(sprt_pdev, mrt_nullptr);

	if (sprt_port->irq_high >= 0)
		fwk_free_irq(sprt_port->irq_high, sprt_port);

	if (sprt_port->irq_low >= 0)
		fwk_free_irq(sprt_port->irq_low, sprt_port);
	
	fwk_io_unmap(sprt_port->base);
	list_head_del(&sprt_port->sgrt_link);
	kfree(sprt_port);

	return NR_IS_NORMAL;
}

/*!< device id for device-tree */
static const struct fwk_of_device_id sgrt_imx_gpiochip_driver_id[] =
{
	{ .compatible = "fsl,imx6ul-gpio", },
	{},
};

/*!< platform instance */
static struct fwk_platdrv sgrt_imx_gpiochip_platdriver =
{
	.probe	= imx_gpiochip_driver_probe,
	.remove	= imx_gpiochip_driver_remove,
	
	.sgrt_driver =
	{
		.name 	= "fsl, imx6ull, gpio-controller",
		.id 	= -1,
		.sprt_of_match_table = sgrt_imx_gpiochip_driver_id,
	},
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   imx_gpiochip_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
ksint32_t __fwk_init imx_gpiochip_driver_init(void)
{
	return fwk_register_platdriver(&sgrt_imx_gpiochip_platdriver);
}

/*!
 * @brief   imx_gpiochip_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit imx_gpiochip_driver_exit(void)
{
	fwk_unregister_platdriver(&sgrt_imx_gpiochip_platdriver);
}

IMPORT_DRIVER_INIT(imx_gpiochip_driver_init);
IMPORT_DRIVER_EXIT(imx_gpiochip_driver_exit);

/*!< end of file */
