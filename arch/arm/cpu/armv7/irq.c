/*
 * Interrupt Interface Defines
 *
 * File Name:   irq.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.03.23
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <configs/configs.h>
#include <asm/armv7/ca7_gic.h>

#include <platform/fwk_basic.h>
#include <platform/irq/fwk_irq_domain.h>
#include <platform/irq/fwk_irq.h>
#include <platform/irq/fwk_irq_types.h>
#include <platform/of/fwk_of.h>

/*!< The defines */
#define CA7_MAX_GPC_NR                  (128)

/*!< The functions */
static ksint32_t fwk_gic_of_init(struct fwk_device_node *sprt_node, struct fwk_device_node *sprt_parent);
static ksint32_t fwk_gpc_of_init(struct fwk_device_node *sprt_node, struct fwk_device_node *sprt_parent);

/*!< The globals */
static kuint32_t g_iHal_gic_cnts = 0;
static srt_ca7_gic_t sgrt_gic_global_data[CA7_MAX_GIC_NR] = {0};

const struct fwk_of_device_id sgrt_fwk_irq_intcs_table[] =
{
    { .compatible = "arm,cortex-a7-gic", .data = fwk_gic_of_init },
    { .compatible = "fsl,imx6ul-gpc", .data = fwk_gpc_of_init },
    { .compatible = mrt_nullptr, .data = mrt_nullptr },
};

/*!< API function */
/*!
 * @brief   initial IRQ
 * @param   none
 * @retval  none
 * @note    none
 */
void __plat_init initIRQ(void)
{
#if CONFIG_OF
    fwk_of_irq_init(sgrt_fwk_irq_intcs_table);

#else
    local_irq_initial(CA7_GIC_NULL);

#endif

    printk("initial irq finished\n");
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
static ksint32_t gic_irq_domain_xlate(struct fwk_irq_domain *sprt_domain, struct fwk_device_node *sprt_intc,
				const kuint32_t *intspec, kuint32_t intsize, kuint32_t *out_hwirq, kuint32_t *out_type)
{
	if (sprt_domain->sprt_node != sprt_intc)
		return -NR_IS_UNVALID;
	if (intsize < 3)
		return -NR_IS_UNVALID;

	/* skip over PPI, which is 0 ~ 15 */
	*out_hwirq = intspec[1] + 16;

	/* GIC_SGI: 1; GIC_SPI: 0 ===> skip over SGI */
	if (!intspec[0])
		*out_hwirq += 16;

    /* interrupt trigger type */
	*out_type = intspec[2] & IRQ_TYPE_SENSE_MASK;

	return NR_IS_NORMAL;
}

static const srt_fwk_irq_domain_ops_t sgrt_gic_domain_hierarchy_ops = 
{
	.xlate = gic_irq_domain_xlate,
	.alloc = mrt_nullptr,
	.free = mrt_nullptr,
};

/*!
 * @brief   initial GIC interrupt
 * @param   none
 * @retval  none
 * @note    For simplicity, we only use group0 of GIC
 */
static void fwk_gic_initial(srt_ca7_gic_t *sprt_gic)
{
    srt_ca7_gic_des_t *sprt_dest;
    srt_ca7_gic_cpu_t *sprt_cpu;
    kuint32_t i;
    kuint32_t irqRegs;

    sprt_gic->dest_base = fwk_io_remap(sprt_gic->dest_base);
    if (!isValid(sprt_gic->dest_base))
        return;

    sprt_gic->cpu_base = fwk_io_remap(sprt_gic->cpu_base);
    if (!isValid(sprt_gic->cpu_base))
        return;

    sprt_dest = mrt_get_gic_destributor(sprt_gic);
    sprt_cpu = mrt_get_gic_interface(sprt_gic);

    irqRegs = mrt_mask(sprt_dest->D_TYPER, 0x1fU) + 1;

    if (isValid(sprt_gic))
    {
        /*!< irq number = ((sprt_dest->D_TYPER & 0x1fU) + 1) * 32 */
        sprt_gic->gic_irqs = irqRegs << 5;
        if (sprt_gic->gic_irqs > __CA7_GIC_MAX_IRQS)
            sprt_gic->gic_irqs = __CA7_GIC_MAX_IRQS;
    }

    /*!< On POR, all SPI is in group 0, level-sensitive and using 1-N model */

    /*!< Disable all PPI, SGI and SPI */
    for (i = 0; i < irqRegs; i++)
        mrt_writel(0xffffffffU, &sprt_dest->D_ICENABLER[i]);

    /*!< Make all interrupts have higher priority */
    mrt_writel(mrt_bit_mask(0xffU, 0xffU, 8 - __CA7_GIC_PRIO_BITS), &sprt_cpu->C_PMR);

    /*!< No subpriority, all priority level allows preemption */
    mrt_writel(7 - __CA7_GIC_PRIO_BITS, &sprt_cpu->C_BPR);

    /*!< Enable group0 distribution */
    mrt_writel(1U, &sprt_dest->D_CTLR);

    /*!< Enable group0 signaling */
    mrt_writel(1U, &sprt_cpu->C_CTLR);

    /*!< if program has run to main, do not set VBAR again */
//  __set_cp15_vbar(VECTOR_TABLE_BASE);
}

static void fwk_gic_init_bases(kuint32_t gic_nr, kuint32_t irq_start,
			                void *dest_base, void *cpu_base,
			                kuint32_t percpu_offset, struct fwk_device_node *sprt_node)
{
    srt_ca7_gic_t *sprt_gic;
    srt_fwk_irq_domain_t *sprt_domain;

    sprt_gic = fwk_get_gic_data(gic_nr);
    if (!isValid(sprt_gic))
        return;

    sprt_gic->dest_base = dest_base;
    sprt_gic->cpu_base = cpu_base;

    /*!< Initial GIC */
    fwk_gic_initial(sprt_gic);

    /*!< sprt_gic->gic_irqs will be get on local_irq_initial */
    if (!sprt_gic->gic_irqs)
        print_err("Get IRQ Controller Number failed!\n");

    if (isValid(sprt_node))
    {
        sprt_domain = fwk_irq_domain_add_hierarchy(mrt_nullptr, sprt_node, 
                                sprt_gic->gic_irqs, &sgrt_gic_domain_hierarchy_ops, sprt_gic);
        sprt_gic->sprt_domain = sprt_domain;
    }
}

/*!
 * @brief   initial GIC
 * @param   sprt_node: current interrupt-controller
 * @param   sprt_parent: interrupt-controller parent
 * @retval  none
 * @note    none
 */
ksint32_t fwk_gic_of_init(struct fwk_device_node *sprt_node, struct fwk_device_node *sprt_parent)
{
    kuint32_t destributor = 0, cpu_interface = 0;

    if (isValid(sprt_parent))
        return -NR_IS_UNVALID;

    fwk_of_property_read_u32_index(sprt_node, "reg", 0, &destributor);
    fwk_of_property_read_u32_index(sprt_node, "reg", 2, &cpu_interface);

    if ((!destributor) || (!cpu_interface))
        return -NR_IS_NOTFOUND;

    fwk_gic_init_bases(g_iHal_gic_cnts, -1, (void *)destributor, (void *)cpu_interface, 0, sprt_node);
    g_iHal_gic_cnts++;

    return NR_IS_NORMAL;
}

srt_ca7_gic_t *fwk_get_gic_data(kuint32_t gic_nr)
{
    return (gic_nr >= CA7_MAX_GIC_NR) ? mrt_nullptr : &sgrt_gic_global_data[gic_nr];
}

ksint32_t fwk_gic_to_gpc_irq(ksint32_t hwirq)
{
    hwirq -= 32;
    if (hwirq < 0)
        return hwirq;

    return fwk_irq_get_by_domain_name("gpc", hwirq);
}

ksint32_t fwk_gpc_to_gic_irq(ksint32_t virq)
{
    srt_fwk_irq_data_t *sprt_data;

    sprt_data = fwk_irq_get_data(virq);
    return isValid(sprt_data) ? (sprt_data->hwirq + 32) : -1;
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
static ksint32_t gpc_irq_domain_xlate(struct fwk_irq_domain *sprt_domain, struct fwk_device_node *sprt_intc,
				const kuint32_t *intspec, kuint32_t intsize, kuint32_t *out_hwirq, kuint32_t *out_type)
{
	if (sprt_domain->sprt_node != sprt_intc)
		return -NR_IS_UNVALID;
	if (intsize != 3)
		return -NR_IS_UNVALID;
    if (intspec[0] != 0)
        return -NR_IS_UNVALID;

	/* skip over PPI, which is 0 ~ 15 */
	*out_hwirq = intspec[1];
    *out_type = intspec[2];

	return NR_IS_NORMAL;
}

static const srt_fwk_irq_domain_ops_t sgrt_gpc_domain_hierarchy_ops = 
{
	.xlate = gpc_irq_domain_xlate,
	.alloc = mrt_nullptr,
	.free = mrt_nullptr,
};

/*!
 * @brief   initial GPC
 * @param   sprt_node: current interrupt-controller
 * @param   sprt_parent: interrupt-controller parent
 * @retval  none
 * @note    none
 */
ksint32_t fwk_gpc_of_init(struct fwk_device_node *sprt_node, struct fwk_device_node *sprt_parent)
{
    srt_fwk_irq_domain_t *sprt_domain, *sprt_par;

    if (!isValid(sprt_parent) || !isValid(sprt_node))
        return -NR_IS_UNVALID;

    sprt_par = fwk_of_irq_host(sprt_parent);
    if (!isValid(sprt_par))
        return -NR_IS_UNVALID;

    sprt_domain = fwk_irq_domain_add_hierarchy(sprt_par, sprt_node, CA7_MAX_GPC_NR, &sgrt_gpc_domain_hierarchy_ops, mrt_nullptr);
    if (!isValid(sprt_domain))
        return -NR_IS_FAILD;

    return NR_IS_NORMAL;
}

/* end of file */
