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
#include <platform/irq/fwk_irq_chip.h>
#include <platform/irq/fwk_irq_domain.h>
#include <platform/irq/fwk_irq_types.h>

#include <imx6/imx6ull_periph.h>

/*!< The defines */
typedef struct imx_gpio_port
{
	void *base;
	kint32_t id;
	kint32_t irq_high;
	kint32_t irq_low;
	struct fwk_gpio_chip sgtc_gpiochip;
	struct fwk_irq_domain *sptr_irqdomain;

	kint32_t irq_base;
	struct fwk_irq_generic *sptr_irqgc;

	struct list_head sgtc_link;

} srt_imx_gpio_port_t;

/*!< The globals */
static DECLARE_LIST_HEAD(sgtc_imx_gpio_ports_list);

/*!< API function */
/*!
 * @brief   isr
 * @param   ptrDev
 * @retval  errno
 * @note    none
 */
static irq_return_t imx_gpiochip_driver_isr(kint32_t irq, void *ptrDev)
{
	struct imx_gpio_port *sptr_port;
	struct fwk_irq_domain *sptr_domain;
	srt_hal_imx_gpio_t *sptr_reg;
	kbool_t irq_ena, irq_sta;
	kuint32_t idx;
	kint32_t pin_irq;

	sptr_port = (struct imx_gpio_port *)ptrDev;
	sptr_reg = (srt_hal_imx_gpio_t *)sptr_port->base;
	sptr_domain = sptr_port->sptr_irqdomain;
	
	for (idx = 0; idx < 32; idx++)
	{
		irq_ena = !!mr_getbitl(mr_bit(idx), &sptr_reg->IMR);
		irq_sta = !!mr_getbitl(mr_bit(idx), &sptr_reg->ISR);

		if (!irq_ena || !irq_sta)
			continue;
		
		pin_irq = fwk_irq_get_by_domain(sptr_domain, idx);
		fwk_do_irq_handler(pin_irq);

		/*!< clear status bit */
		mr_setbitl(mr_bit(idx), &sptr_reg->ISR);
	}

	return ER_NORMAL;
}

/*!
 * @brief   set the trigger ways of gpio irq
 * @param   sptr_data, type
 * @retval  errno
 * @note    none
 */
static kint32_t imx_gpiochip_driver_irq_set_type(struct fwk_irq_data *sptr_data, kuint32_t type)
{
	struct imx_gpio_port *sptr_port;
	struct fwk_irq_generic *sptr_gc;
	srt_hal_imx_gpio_t *sptr_reg;
	struct fwk_gpio_desc *sptr_desc;
	kuint32_t offset, bitmask = 0;
	kbool_t is_clear = false, active_low;

	offset = sptr_data->hwirq;
	sptr_gc = fwk_irq_get_generic_data(sptr_data);
	sptr_port = sptr_gc->private_data;
	sptr_reg = (srt_hal_imx_gpio_t *)sptr_port->base;

	sptr_desc = fwk_gpiochip_get_desc(&sptr_port->sgtc_gpiochip, offset);
	if (!sptr_desc)
		return -ER_NODEV;

	active_low = fwk_gpio_desc_check_flags(sptr_desc, NR_FWK_GPIODESC_ACTIVE_LOW);

	switch (type)
	{
		case IRQ_TYPE_EDGE_RISING:
			bitmask = active_low ? HAL_IMX_ICR_FALLING_EDGE : HAL_IMX_ICR_RISING_EDGE;
			break;

		case IRQ_TYPE_EDGE_FALLING:
			bitmask = active_low ? HAL_IMX_ICR_RISING_EDGE : HAL_IMX_ICR_FALLING_EDGE;
			break;

		case IRQ_TYPE_EDGE_BOTH:
			bitmask = HAL_IMX_ICR_MASK + 1;
			break;

		case IRQ_TYPE_LEVEL_LOW:
			bitmask = active_low ? HAL_IMX_ICR_HIGH_LEVEL : HAL_IMX_ICR_LOW_LEVEL;
			break;

		case IRQ_TYPE_LEVEL_HIGH:
			bitmask = active_low ? HAL_IMX_ICR_LOW_LEVEL : HAL_IMX_ICR_HIGH_LEVEL;
			break;

		default:
			return -ER_INVALID;
	}

	mr_clrbitl(mr_bit(offset), &sptr_reg->EDGE_SEL);

	if (bitmask > HAL_IMX_ICR_MASK)
	{
		is_clear = true;
		mr_setbitl(mr_bit(offset), &sptr_reg->EDGE_SEL);
	}

	if (offset >= 16)
	{
		offset = (offset - 16) << 1;
		mr_clrbitl(mr_bit_nr(HAL_IMX_ICR_MASK, offset), &sptr_reg->ICR2);
		
		if (!is_clear)
			mr_setbitl(mr_bit_nr(bitmask, offset), &sptr_reg->ICR2);
	}
	else
	{
		offset <<= 1;
		mr_clrbitl(mr_bit_nr(HAL_IMX_ICR_MASK, offset), &sptr_reg->ICR1);
		
		if (!is_clear)
			mr_setbitl(mr_bit_nr(bitmask, offset), &sptr_reg->ICR1);
	}

	return ER_NORMAL;
}

/*!
 * @brief   hardware irqnumber translate
 * @param   sptr_domain: parent interrupt controller
 * @param   sptr_intc: parent interrupt controller's device-node
 * @param   intspec: of_handle_args::args[]
 * @param   intsize: property "interrupts" cell number
 * @retval  none
 * @note    none
 */
static kint32_t imx_gpiochip_irq_domain_xlate(struct fwk_irq_domain *sptr_domain, struct fwk_device_node *sptr_intc,
				const kuint32_t *intspec, kuint32_t intsize, kuint32_t *out_hwirq, kuint32_t *out_type)
{
	if (sptr_domain->sptr_node != sptr_intc)
		return -ER_INVALID;
	if (intsize != 2)
		return -ER_INVALID;
	if (intspec[0] >= 32)
		return -ER_INVALID;

	/*!< gpio: 0 ~ 31 */
	*out_hwirq = intspec[0];
    *out_type = intspec[1];

	return ER_NORMAL;
}

static const struct fwk_irq_domain_ops sgtc_imx_gpiochip_hierarchy_ops = 
{
	.xlate = imx_gpiochip_irq_domain_xlate,
	.alloc = mr_nullptr,
	.free = mr_nullptr,
};

/*!
 * @brief   gpio chip intc probe
 * @param   sptr_pdev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_gpiochip_driver_probe_intc(struct fwk_platdev *sptr_pdev)
{
	struct imx_gpio_port *sptr_port;
	struct fwk_device_node *sptr_node, *sptr_par;
	struct fwk_irq_domain *sptr_intc;
	struct fwk_irq_domain *sptr_domain;
	kint32_t virq, irq_base;
	struct fwk_irq_generic *sptr_gc;
	struct fwk_irq_chip *sptr_chip;
	srt_hal_imx_gpio_t *sptr_reg;

	sptr_node = sptr_pdev->sgtc_dev.sptr_node;
	sptr_port = fwk_platform_get_drvdata(sptr_pdev);
	sptr_reg = (srt_hal_imx_gpio_t *)sptr_port->base;
	sptr_chip = &sgtc_fwk_irq_generic_chip;

	if (!fwk_of_find_property(sptr_node, "interrupt-controller", mr_nullptr))
		return -ER_NSUPPORT;

	sptr_par = fwk_of_irq_parent(sptr_node);
	if (!isValid(sptr_par))
		return -ER_NOTFOUND;
	
	sptr_intc = fwk_of_irq_host(sptr_par);
	if (!isValid(sptr_intc))
		return -ER_NOTFOUND;

	sptr_domain = fwk_irq_domain_add_hierarchy(sptr_intc, sptr_node, 
									32, &sgtc_imx_gpiochip_hierarchy_ops, mr_nullptr);
	if (!isValid(sptr_domain))
		return -ER_FAILD;

	irq_base = sptr_port->id << 5;
	virq = fwk_irq_domain_alloc_irqs(sptr_domain, irq_base, 0, 32);
	if (virq < 0)
		goto fail1;

	sptr_port->irq_base = virq;
	sptr_port->sptr_irqdomain = sptr_domain;

	sptr_gc = kzalloc(sizeof(*sptr_gc), GFP_KERNEL);
	if (!isValid(sptr_gc))
		goto fail2;

	sptr_gc->sgtc_chip.irq_mask = sptr_chip->irq_mask;
	sptr_gc->sgtc_chip.irq_unmask = sptr_chip->irq_unmask;
	sptr_gc->sgtc_chip.irq_ack = sptr_chip->irq_ack;
	sptr_gc->sgtc_chip.irq_set_type = imx_gpiochip_driver_irq_set_type;
	sptr_gc->manage_reg = (kuaddr_t)&sptr_reg->IMR;
	sptr_gc->status_reg = (kuaddr_t)&sptr_reg->ISR;
	sptr_gc->private_data = sptr_port;

	fwk_irq_setup_generic_chip(sptr_port->irq_base, 32, sptr_gc, mr_nullptr);
	sptr_port->sptr_irqgc = sptr_gc;

	return ER_NORMAL;

fail2:
	fwk_irq_domain_free_irqs(sptr_domain);
	sptr_port->sptr_irqdomain = mr_nullptr;
fail1:
	fwk_irq_domain_del_hierarchy(sptr_domain);
	kfree(sptr_domain);

	return -ER_ERROR;
}

/*!
 * @brief   gpio chip intc remove
 * @param   sptr_pdev
 * @retval  errno
 * @note    none
 */
static void imx_gpiochip_driver_remove_intc(struct fwk_platdev *sptr_pdev)
{
	struct imx_gpio_port *sptr_port;
	struct fwk_irq_domain *sptr_domain;

	sptr_port = fwk_platform_get_drvdata(sptr_pdev);
	sptr_domain = sptr_port->sptr_irqdomain;
	sptr_port->sptr_irqdomain = mr_nullptr;

	fwk_irq_shutdown_generic_chip(sptr_port->irq_base, 32);
	kfree(sptr_port->sptr_irqgc);
	sptr_port->sptr_irqgc = mr_nullptr;
	
	fwk_irq_domain_free_irqs(sptr_domain);
	fwk_irq_domain_del_hierarchy(sptr_domain);
	kfree(sptr_domain);
}

/*!
 * @brief   gpio number to irq number
 * @param   sptr_chip, offset(gpio number)
 * @retval  errno
 * @note    none
 */
static kint32_t imx_gpiochip_driver_to_irq(struct fwk_gpio_chip *sptr_chip, kuint32_t offset)
{
	struct imx_gpio_port *sptr_port;
	struct fwk_irq_domain *sptr_domain;

	if (!sptr_chip || (offset >= sptr_chip->ngpios))
		return -ER_NODEV;

	sptr_port = mr_container_of(sptr_chip, struct imx_gpio_port, sgtc_gpiochip);
	sptr_domain = sptr_port->sptr_irqdomain;

	return fwk_irq_domain_find_map(sptr_domain, offset, 0);
}

/*!
 * @brief   get gpio direction configuration
 * @param   sptr_chip, offset(gpio number)
 * @retval  errno
 * @note    none
 */
static kint32_t imx_gpiochip_driver_get_direction(struct fwk_gpio_chip *sptr_chip, kuint32_t offset)
{
	struct imx_gpio_port *sptr_port;
	srt_hal_imx_gpio_t *sptr_reg;

	if (!sptr_chip || (offset >= sptr_chip->ngpios))
		return -ER_NODEV;

	sptr_port = mr_container_of(sptr_chip, struct imx_gpio_port, sgtc_gpiochip);
	sptr_reg = (srt_hal_imx_gpio_t *)sptr_port->base;
	if (!sptr_reg)
		return -ER_NODEV;

    return mr_getbitl(mr_bit(offset), &sptr_reg->GDIR) ? FWK_GPIO_DIR_OUT : FWK_GPIO_DIR_IN;
}

/*!
 * @brief   set gpio direction to input
 * @param   sptr_chip, offset(gpio number)
 * @retval  errno
 * @note    none
 */
static kint32_t imx_gpiochip_driver_direction_input(struct fwk_gpio_chip *sptr_chip, kuint32_t offset)
{
	struct imx_gpio_port *sptr_port;
	srt_hal_imx_gpio_t *sptr_reg;

	if (!sptr_chip || (offset >= sptr_chip->ngpios))
		return -ER_NODEV;

	sptr_port = mr_container_of(sptr_chip, struct imx_gpio_port, sgtc_gpiochip);
	sptr_reg = (srt_hal_imx_gpio_t *)sptr_port->base;
	if (!sptr_reg)
		return -ER_NODEV;
	
	mr_clrbitl(mr_bit(offset), &sptr_reg->GDIR);

    return ER_NORMAL;
}

/*!
 * @brief   set gpio direction to output
 * @param   sptr_chip, offset(gpio number)
 * @retval  errno
 * @note    none
 */
static kint32_t imx_gpiochip_driver_direction_output(struct fwk_gpio_chip *sptr_chip, kuint32_t offset, kint32_t value)
{
	struct imx_gpio_port *sptr_port;
	srt_hal_imx_gpio_t *sptr_reg;

	if (!sptr_chip || (offset >= sptr_chip->ngpios))
		return -ER_NODEV;

	sptr_port = mr_container_of(sptr_chip, struct imx_gpio_port, sgtc_gpiochip);
	sptr_reg = (srt_hal_imx_gpio_t *)sptr_port->base;
	if (!sptr_reg)
		return -ER_NODEV;
	
	mr_setbitl(mr_bit(offset), &sptr_reg->GDIR);

	/*!< if it is active high, set low level to initialize gpio */
	if (sptr_chip->set)
		sptr_chip->set(sptr_chip, offset, value);

    return ER_NORMAL;
}

/*!
 * @brief   set gpio direction to input
 * @param   sptr_chip, offset(gpio number)
 * @retval  errno
 * @note    none
 */
static kint32_t imx_gpiochip_driver_get(struct fwk_gpio_chip *sptr_chip, kuint32_t offset)
{
	struct imx_gpio_port *sptr_port;
	srt_hal_imx_gpio_t *sptr_reg;

	if (!sptr_chip || (offset >= sptr_chip->ngpios))
		return -ER_NODEV;

	sptr_port = mr_container_of(sptr_chip, struct imx_gpio_port, sgtc_gpiochip);
	sptr_reg = (srt_hal_imx_gpio_t *)sptr_port->base;
	if (!sptr_reg)
		return -ER_NODEV;

    return !!mr_getbitl(mr_bit(offset), &sptr_reg->DR);
}

/*!
 * @brief   set gpio velue
 * @param   sptr_chip, offset(gpio number), value(0 or 1)
 * @retval  none
 * @note    none
 */
static void imx_gpiochip_driver_set(struct fwk_gpio_chip *sptr_chip, kuint32_t offset, kint32_t value)
{
	struct imx_gpio_port *sptr_port;
	srt_hal_imx_gpio_t *sptr_reg;

	if (!sptr_chip || (offset >= sptr_chip->ngpios) || (value < 0))
		return;

	sptr_port = mr_container_of(sptr_chip, struct imx_gpio_port, sgtc_gpiochip);
	sptr_reg = (srt_hal_imx_gpio_t *)sptr_port->base;
	if (!sptr_reg)
		return;

	if (value)
		mr_setbitl(mr_bit(offset), &sptr_reg->DR);
	else
		mr_clrbitl(mr_bit(offset), &sptr_reg->DR);
}

/*!
 * @brief   gpio chip probe
 * @param   sptr_pdev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_gpiochip_driver_probe_gc(struct fwk_platdev *sptr_pdev)
{
	struct imx_gpio_port *sptr_port;
	struct fwk_device_node *sptr_node;
	struct fwk_gpio_chip *sptr_gc;
	kint32_t gpio_base;
	kuint32_t count = 0;

	sptr_node = sptr_pdev->sgtc_dev.sptr_node;
	sptr_port = fwk_platform_get_drvdata(sptr_pdev);

	if (!fwk_of_find_property(sptr_node, "gpio-controller", mr_nullptr))
		return -ER_NSUPPORT;

	if (fwk_of_property_read_u32(sptr_node, "#gpio-cells", &count))
		return -ER_NSUPPORT;

	sptr_gc = &sptr_port->sgtc_gpiochip;
	gpio_base = sptr_port->id << 5;

	if (fwk_gpiochip_init(sptr_gc, & sptr_pdev->sgtc_dev, gpio_base, 32 >> 3))
		return -ER_FAILD;

	sptr_gc->of_gpio_n_cells = count;
	sptr_gc->to_irq = imx_gpiochip_driver_to_irq;
	sptr_gc->get_direction = imx_gpiochip_driver_get_direction;
	sptr_gc->direction_input = imx_gpiochip_driver_direction_input;
	sptr_gc->direction_output = imx_gpiochip_driver_direction_output;
	sptr_gc->get = imx_gpiochip_driver_get;
	sptr_gc->set = imx_gpiochip_driver_set;

	return fwk_gpiochip_add(sptr_gc);
}

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   imx_gpiochip_driver_probe
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_gpiochip_driver_probe(struct fwk_platdev *sptr_pdev)
{
	struct imx_gpio_port *sptr_port;
	struct fwk_device_node *sptr_node;
	srt_hal_imx_gpio_t *sptr_reg;

	sptr_node = sptr_pdev->sgtc_dev.sptr_node;

	sptr_port = kzalloc(sizeof(*sptr_port), GFP_KERNEL);
	if (!isValid(sptr_port))
		return -ER_NOMEM;

	/*!< id = 0 ~ 5, e.g. gpio1 ~ gpio6 */
	sptr_port->id = (sptr_pdev->id < 0) ? fwk_of_get_alias_id(sptr_node) : sptr_pdev->id;
	if (sptr_port->id < 0)
		goto fail1;

	/*!< get gpio register base address */
	sptr_port->base = fwk_of_iomap(sptr_node, 0);
	if (!isValid(sptr_port->base))
		goto fail1;

	/*!< get gpio interrupt number */
	sptr_port->irq_low = fwk_platform_get_irq(sptr_pdev, 0);
	sptr_port->irq_high = fwk_platform_get_irq(sptr_pdev, 1);

	/*!< request irq (the domain is gpc) */
	if (sptr_port->irq_low >= 0)
	{
		if (fwk_request_irq(sptr_port->irq_low, imx_gpiochip_driver_isr, IRQ_TYPE_NONE, "imx-gpio-low", sptr_port))
			goto fail2;
	}
	
	if (sptr_port->irq_high >= 0)
	{
		if (fwk_request_irq(sptr_port->irq_high, imx_gpiochip_driver_isr, IRQ_TYPE_NONE, "imx-gpio-high", sptr_port))
			goto fail3;
	}

	/*!< initial gpio register */
	sptr_reg = (srt_hal_imx_gpio_t *)sptr_port->base;
	mr_writel(~0, &sptr_reg->ISR);
	mr_writel(0, &sptr_reg->IMR);

	fwk_platform_set_drvdata(sptr_pdev, sptr_port);

	/*!< gpio is also a irq-controller */
	if (imx_gpiochip_driver_probe_intc(sptr_pdev))
		goto fail4;

	/*!< set to be a gpio-controller */
	if (imx_gpiochip_driver_probe_gc(sptr_pdev))
		goto fail5;

	list_head_add_tail(&sgtc_imx_gpio_ports_list, &sptr_port->sgtc_link);

	return ER_NORMAL;

fail5:
	imx_gpiochip_driver_remove_intc(sptr_pdev);
fail4:
	fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);
	if (sptr_port->irq_high >= 0)
		fwk_free_irq(sptr_port->irq_high, sptr_port);
fail3:
	if (sptr_port->irq_low >= 0)
		fwk_free_irq(sptr_port->irq_low, sptr_port);
fail2:
	fwk_io_unmap(sptr_port->base);
fail1:
	kfree(sptr_port);
	return -ER_ERROR;
}

/*!
 * @brief   imx_gpiochip_driver_remove
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_gpiochip_driver_remove(struct fwk_platdev *sptr_pdev)
{
	struct imx_gpio_port *sptr_port;

	sptr_port = fwk_platform_get_drvdata(sptr_pdev);

	imx_gpiochip_driver_remove_intc(sptr_pdev);
	fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);

	if (sptr_port->irq_high >= 0)
		fwk_free_irq(sptr_port->irq_high, sptr_port);

	if (sptr_port->irq_low >= 0)
		fwk_free_irq(sptr_port->irq_low, sptr_port);
	
	fwk_io_unmap(sptr_port->base);
	list_head_del(&sptr_port->sgtc_link);
	kfree(sptr_port);

	return ER_NORMAL;
}

/*!< device id for device-tree */
static const struct fwk_of_device_id sgtc_imx_gpiochip_driver_id[] =
{
	{ .compatible = "fsl,imx6ul-gpio", },
	{},
};

/*!< platform instance */
static struct fwk_platdrv sgtc_imx_gpiochip_platdriver =
{
	.probe	= imx_gpiochip_driver_probe,
	.remove	= imx_gpiochip_driver_remove,
	
	.sgtc_driver =
	{
		.name 	= "fsl, imx6ull, gpio-controller",
		.id 	= -1,
		.sptr_of_match_table = sgtc_imx_gpiochip_driver_id,
	},
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   imx_gpiochip_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init imx_gpiochip_driver_init(void)
{
	return fwk_register_platdriver(&sgtc_imx_gpiochip_platdriver);
}

/*!
 * @brief   imx_gpiochip_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit imx_gpiochip_driver_exit(void)
{
	fwk_unregister_platdriver(&sgtc_imx_gpiochip_platdriver);
}

IMPORT_PATTERN_INIT(imx_gpiochip_driver_init);
IMPORT_PATTERN_EXIT(imx_gpiochip_driver_exit);

/*!< end of file */
