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
#include <asm/armv7/gic_basic.h>

#include <common/io_stream.h>
#include <boot/boot_text.h>
#include <platform/base/fwk_basic.h>
#include <platform/irq/fwk_irq_domain.h>
#include <platform/irq/fwk_irq.h>
#include <platform/irq/fwk_irq_types.h>
#include <platform/irq/fwk_irq_chip.h>
#include <platform/of/fwk_of.h>

/*!< The defines */
#define CA7_MAX_GPC_NR                  (128)

/*!< The functions */
static kint32_t fwk_gic_of_init(struct fwk_device_node *sptr_node, struct fwk_device_node *sptr_parent);
static kint32_t fwk_gpc_of_init(struct fwk_device_node *sptr_node, struct fwk_device_node *sptr_parent);

/*!< The globals */
static kuint32_t g_iHal_gic_cnts = 0;
static srt_gic_t sgtc_gic_global_data[GIC_MAX_NR] = {0};

const struct fwk_of_device_id sgtc_fwk_irq_intcs_table[] =
{
    { .compatible = "arm,cortex-a7-gic", .data = fwk_gic_of_init },
    { .compatible = "arm,cortex-a9-gic", .data = fwk_gic_of_init },

    { .compatible = "fsl,imx6ul-gpc", .data = fwk_gpc_of_init },
    { .compatible = mr_nullptr, .data = mr_nullptr },
};

static kbool_t g_isIntcGicDirect = true;

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
    fwk_of_irq_init(sgtc_fwk_irq_intcs_table);

#else
    local_irq_initial(GIC_NULL);

#endif

    print_info("initial irq finished\r\n");
}

/*!
 * @brief   gic enbale irq
 * @param   none
 * @retval  none
 * @note    enable IRQ: group0
 */
void hw_enable_irq(kint32_t hwirq)
{
    srt_gic_t *sptr_gic = fwk_get_gic_data(0);
    srt_gic_dist_t *sptr_dist;

    sptr_dist = mr_get_gic_distributor(sptr_gic);
    mr_setbit_towords(hwirq, &sptr_dist->D_ISENABLER);
}

/*!
 * @brief   gic disable irq
 * @param   none
 * @retval  none
 * @note    disable IRQ: group0
 */
void hw_disable_irq(kint32_t hwirq)
{
    srt_gic_t *sptr_gic = fwk_get_gic_data(0);
    srt_gic_dist_t *sptr_dist;

    sptr_dist = mr_get_gic_distributor(sptr_gic);
    mr_setbit_towords(hwirq, &sptr_dist->D_ICENABLER);
}

/*!
 * @brief   gic acknowledge irq
 * @param   none
 * @retval  none
 * @note    return IRQ number (and CPU source in SGI case)
 */
kint32_t hw_irq_acknowledge(void)
{
    srt_gic_t *sptr_gic = fwk_get_gic_data(0);
    srt_gic_cpu_t *sptr_cpu;

    sptr_cpu = mr_get_gic_interface(sptr_gic);
    return mr_mask(sptr_cpu->C_IAR, 0x1fffU);
}

/*!
 * @brief   gic deactivate irq
 * @param   none
 * @retval  none
 * @note    value should be got from gic_acknowledge_irq()
 */
void hw_irq_deactivate(kuint32_t value)
{
    srt_gic_t *sptr_gic = fwk_get_gic_data(0);
    srt_gic_cpu_t *sptr_cpu;

    sptr_cpu = mr_get_gic_interface(sptr_gic);
    mr_writel(value, &sptr_cpu->C_EOIR);
}

/*!
 * @brief   gic get running priority
 * @param   none
 * @retval  none
 * @note    get current interrupt priority
 */
kuint32_t hw_irq_get_running_priority(void)
{
    srt_gic_t *sptr_gic = fwk_get_gic_data(0);
    srt_gic_cpu_t *sptr_cpu;

    sptr_cpu = mr_get_gic_interface(sptr_gic);
    return mr_mask(sptr_cpu->C_RPR, 0xffU);
}

/*!
 * @brief   gic set priority grouping
 * @param   none
 * @retval  none
 * @note    configure priority group
 */
void hw_irq_set_priority_grouping(kuint32_t priorityGroup)
{
    srt_gic_t *sptr_gic = fwk_get_gic_data(0);
    srt_gic_cpu_t *sptr_cpu;

    sptr_cpu = mr_get_gic_interface(sptr_gic);
    mr_writel(mr_mask(priorityGroup, 0x7U), &sptr_cpu->C_BPR);
}

/*!
 * @brief   gic get priority grouping
 * @param   none
 * @retval  none
 * @note    get priority group
 */
kuint32_t hw_irq_get_priority_grouping(void)
{
    srt_gic_t *sptr_gic = fwk_get_gic_data(0);
    srt_gic_cpu_t *sptr_cpu;

    sptr_cpu = mr_get_gic_interface(sptr_gic);
    return mr_mask(sptr_cpu->C_BPR, 0x7U);
}

/*!
 * @brief   gic set priority
 * @param   none
 * @retval  none
 * @note    set "irq_number" interrupt priority
 */
void hw_irq_set_priority(kuint32_t irq_number, kuint32_t priority)
{
    srt_gic_t *sptr_gic = fwk_get_gic_data(0);
    srt_gic_dist_t *sptr_dist;
    kint32_t hwirq;

    hwirq = fwk_gpc_to_gic_irq(irq_number);
    if (hwirq < 0)
        return;

    sptr_dist = mr_get_gic_distributor(sptr_gic);
    mr_writeb(mr_bit_mask(priority, 0xffU, 8U - __GIC_PRIO_BITS), &sptr_dist->D_IPRIORITYR[hwirq]);
}

/*!
 * @brief   gic get priority
 * @param   none
 * @retval  none
 * @note    get "irq_number" interrupt priority
 */
kuint32_t hw_irq_get_priority(kuint32_t irq_number)
{
    srt_gic_t *sptr_gic = fwk_get_gic_data(0);
    srt_gic_dist_t *sptr_dist;
    kint32_t hwirq;

    hwirq = fwk_gpc_to_gic_irq(irq_number);
    if (hwirq < 0)
        return 0;

    sptr_dist = mr_get_gic_distributor(sptr_gic);
    return (kuint32_t)mr_getbit_u8(0xffU, 8U - __GIC_PRIO_BITS, &sptr_dist->D_IPRIORITYR[hwirq]);
}

/*!< ------------------------------------------------------------------------- */
/*!
 * @brief   return gic data
 * @param   gic_nr
 * @retval  none
 * @note    none
 */
srt_gic_t *fwk_get_gic_data(kuint32_t gic_nr)
{
    return (gic_nr >= GIC_MAX_NR) ? mr_nullptr : &sgtc_gic_global_data[gic_nr];
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
static kint32_t gic_irq_domain_xlate(struct fwk_irq_domain *sptr_domain, struct fwk_device_node *sptr_intc,
				const kuint32_t *intspec, kuint32_t intsize, kuint32_t *out_hwirq, kuint32_t *out_type)
{
	if (sptr_domain->sptr_node != sptr_intc)
		return -ER_INVALID;
	if (intsize < 3)
		return -ER_INVALID;

	*out_hwirq = intspec[1];

	/* skip over PPI, which is 0 ~ 15 */
//	*out_hwirq += 16;

	/* GIC_SGI: 1; GIC_SPI: 0 ===> skip over SGI */
	if (!intspec[0])
		*out_hwirq += 16;

    /* interrupt trigger type */
	*out_type = intspec[2] & IRQ_TYPE_SENSE_MASK;

	return ER_NORMAL;
}

/*!
 * @brief   enable IRQ
 * @param   sptr_data
 * @retval  none
 * @note    none
 */
static void gic_irq_chip_enable(struct fwk_irq_data *sptr_data)
{
    srt_gic_t *sptr_gic = fwk_get_gic_data(0);
    srt_gic_dist_t *sptr_dest;

    if (!sptr_data || (sptr_data->hwirq < 0))
        return;

    sptr_dest = mr_get_gic_distributor(sptr_gic);
    mr_setbit_towords(sptr_data->hwirq + sptr_data->sptr_domain->hwirq, 
                    &sptr_dest->D_ISENABLER);
}

/*!
 * @brief   disable IRQ
 * @param   sptr_data
 * @retval  none
 * @note    none
 */
static void gic_irq_chip_disable(struct fwk_irq_data *sptr_data)
{
    srt_gic_t *sptr_gic = fwk_get_gic_data(0);
    srt_gic_dist_t *sptr_dest;

    if (!sptr_data || (sptr_data->hwirq < 0))
        return;

    sptr_dest = mr_get_gic_distributor(sptr_gic);
    mr_setbit_towords(sptr_data->hwirq + sptr_data->sptr_domain->hwirq, 
                    &sptr_dest->D_ICENABLER);
}

/*!
 * @brief   get IRQ status
 * @param   sptr_data
 * @retval  none
 * @note    none
 */
static kbool_t gic_irq_chip_ack(struct fwk_irq_data *sptr_data)
{
    srt_gic_t *sptr_gic = fwk_get_gic_data(0);
    srt_gic_cpu_t *sptr_cpu;
    kuint32_t hwirq;

    if (!sptr_data || (sptr_data->hwirq < 0))
        return false;

    sptr_cpu = mr_get_gic_interface(sptr_gic);
    hwirq = mr_mask(sptr_cpu->C_IAR, 0x1fffU);

    return !!(hwirq & mr_bit(sptr_data->hwirq + sptr_data->sptr_domain->hwirq));
}

/*!
 * @brief   check if IRQ is enabled
 * @param   sptr_data
 * @retval  none
 * @note    none
 */
static kbool_t gic_irq_chip_is_enabled(struct fwk_irq_data *sptr_data)
{
    srt_gic_t *sptr_gic = fwk_get_gic_data(0);
    srt_gic_dist_t *sptr_dest;

    if (!sptr_data || (sptr_data->hwirq < 0))
        return false;

    sptr_dest = mr_get_gic_distributor(sptr_gic);
    return !!mr_getbit_fromwords(sptr_data->hwirq + sptr_data->sptr_domain->hwirq, 
                                &sptr_dest->D_ISENABLER);
}

/*!
 * @brief   allocate irq_domain for gic
 * @param   sptr_domain
 * @retval  error code
 * @note    none
 */
kint32_t gic_irq_domain_alloc(struct fwk_irq_domain *sptr_domain, kuint32_t virq, kuint32_t nr_irqs, void *arg)
{
    struct fwk_irq_generic *sptr_gc;

    if (!sptr_domain)
        return -ER_NODEV;

	sptr_gc = kzalloc(sizeof(*sptr_gc), GFP_KERNEL);
	if (!isValid(sptr_gc))
        return -ER_NOMEM;

    sptr_gc->sgtc_chip.irq_enable = gic_irq_chip_enable;
    sptr_gc->sgtc_chip.irq_disable = gic_irq_chip_disable;
    sptr_gc->sgtc_chip.irq_ack = gic_irq_chip_ack;
    sptr_gc->sgtc_chip.irq_is_enabled = gic_irq_chip_is_enabled;

    fwk_irq_setup_generic_chip(virq, nr_irqs, sptr_gc, mr_nullptr);

    return ER_NORMAL;
}

/*!
 * @brief   free irq_domain of gpc
 * @param   sptr_domain
 * @retval  none
 * @note    none
 */
void gic_irq_domain_free(struct fwk_irq_domain *sptr_domain, kuint32_t virq, kuint32_t nr_irqs)
{
    struct fwk_irq_generic *sptr_gc;
    struct fwk_irq_data *sptr_data;

    sptr_data = fwk_irq_get_data(virq);
    if (!isValid(sptr_data))
        return;
    
    sptr_gc = fwk_irq_get_generic_data(sptr_data);
    fwk_irq_shutdown_generic_chip(virq, nr_irqs);

    kfree(sptr_gc);
}

static const struct fwk_irq_domain_ops sgtc_gic_domain_hierarchy_ops = 
{
	.xlate = gic_irq_domain_xlate,
	.alloc = gic_irq_domain_alloc,
	.free = gic_irq_domain_free,
};

/*!
 * @brief   initial GIC interrupt
 * @param   none
 * @retval  none
 * @note    For simplicity, we only use group0 of GIC
 */
static void fwk_gic_initial(srt_gic_t *sptr_gic)
{
    srt_gic_dist_t *sptr_dest;
    srt_gic_cpu_t *sptr_cpu;
    kuint32_t i;
    kuint32_t irqRegs;

    sptr_gic->dest_base = fwk_io_remap(sptr_gic->dest_base, ARCH_PER_SIZE);
    if (!isValid(sptr_gic->dest_base))
        return;

    sptr_gic->cpu_base = fwk_io_remap(sptr_gic->cpu_base, ARCH_PER_SIZE);
    if (!isValid(sptr_gic->cpu_base))
        return;

    sptr_dest = mr_get_gic_distributor(sptr_gic);
    sptr_cpu = mr_get_gic_interface(sptr_gic);

    /*!< Disable group0 distribution */
    mr_writel(0U, &sptr_dest->D_CTLR);

    irqRegs = mr_mask(sptr_dest->D_TYPER, 0x1fU) + 1;

    if (isValid(sptr_gic))
    {
        /*!< irq number = ((sptr_dest->D_TYPER & 0x1fU) + 1) * 32 */
        sptr_gic->gic_irqs = irqRegs << 5;
        if (sptr_gic->gic_irqs > __GIC_MAX_IRQS)
            sptr_gic->gic_irqs = __GIC_MAX_IRQS;
    }

    /*!< On POR, all SPI is in group 0, level-sensitive and using 1-N model */

    /*!< Disable all PPI, SGI and SPI */
    for (i = 0; i < irqRegs; i++)
        mr_writel(0xffffffffU, &sptr_dest->D_ICENABLER[i]);

	/*!< The trigger mode in the int_config register, only write to the SPI interrupts, so start at 32 */
    for (i = 32U; i < __GIC_MAX_SPI_IRQS; i += 16U)
    {
		/*!<
		 * Each INT_ID uses two bits, or 16 INT_ID per register
		 * Set them all to be level sensitive, active HIGH.
         * 
         * 00: Reserved
         * 01: Edge triggered
         * 10: Reserved
         * 11: Level triggered
         * 
         * D_ICFGR[0]: irq0 ~ irq15
         * ...
         * D_ICFGR[2]: irq32 ~ irq47
		 */
        mr_writel(0U, &sptr_dest->D_ICFGR[i >> 4]);
    }

    for (i = 0; i < 512; i++)
    {
		/*!<
		 * The priority using int the priority_level register
		 * The priority_level and spi_target registers use one byte per
		 * INT_ID.
		 * Write a default value that can be changed elsewhere.
		 */
        mr_writeb(0xa0, &sptr_dest->D_IPRIORITYR[i]);
    }

    for (i = 32U; i < 512; i++)
    {
		/*!<
		 * The CPU interface in the spi_target register
		 * Only write to the SPI interrupts, so start at 32
		 */
        mr_writeb(0x01, &sptr_dest->D_ITARGETSR[i]);
    }

    /*!< Make all interrupts have higher priority */
    mr_writel(mr_bit_mask(0xffU, 0xffU, 8 - __GIC_PRIO_BITS), &sptr_cpu->C_PMR);

    /*!< No subpriority, all priority level allows preemption */
    mr_writel(7 - __GIC_PRIO_BITS, &sptr_cpu->C_BPR);

    /*!< Enable group0 distribution */
    mr_writel(1U, &sptr_dest->D_CTLR);

    /*!< Enable group0 signaling */
    mr_writel(1U, &sptr_cpu->C_CTLR);

    /*!< if program has run to main, do not set VBAR again */
//  __set_cp15_vbar(VECTOR_TABLE_BASE);
}

/*!
 * @brief   initial GIC
 * @param   sptr_node: current interrupt-controller
 * @retval  none
 * @note    none
 */
static void fwk_gic_init_bases(kuint32_t gic_nr, kuint32_t irq_start,
			                void *dest_base, void *cpu_base,
			                kuint32_t percpu_offset, struct fwk_device_node *sptr_node)
{
    srt_gic_t *sptr_gic;
    struct fwk_irq_domain *sptr_domain;

    sptr_gic = fwk_get_gic_data(gic_nr);
    if (!isValid(sptr_gic))
        return;

    sptr_gic->dest_base = dest_base;
    sptr_gic->cpu_base = cpu_base;

    /*!< Initial GIC */
    fwk_gic_initial(sptr_gic);

    /*!< sptr_gic->gic_irqs will be get on local_irq_initial */
    if (!sptr_gic->gic_irqs)
        print_err("Get IRQ Controller Number failed\r\n");

    if (!isValid(sptr_node))
        return;
    
    sptr_domain = fwk_irq_domain_add_hierarchy(mr_nullptr, sptr_node, 
                            sptr_gic->gic_irqs, &sgtc_gic_domain_hierarchy_ops, sptr_gic);
    sptr_domain->hwirq = 16;
    sptr_gic->sptr_domain = sptr_domain;
}

/*!
 * @brief   initial GIC
 * @param   sptr_node: current interrupt-controller
 * @param   sptr_parent: interrupt-controller parent
 * @retval  none
 * @note    none
 */
kint32_t fwk_gic_of_init(struct fwk_device_node *sptr_node, struct fwk_device_node *sptr_parent)
{
    kuint32_t destributor = 0, cpu_interface = 0;

    if (isValid(sptr_parent))
        return -ER_INVALID;

    fwk_of_property_read_u32_index(sptr_node, "reg", 0, &destributor);
    fwk_of_property_read_u32_index(sptr_node, "reg", 2, &cpu_interface);

    if ((!destributor) || (!cpu_interface))
        return -ER_NOTFOUND;

    fwk_gic_init_bases(g_iHal_gic_cnts, -1, (void *)destributor, (void *)cpu_interface, 0, sptr_node);
    g_iHal_gic_cnts++;

    return ER_NORMAL;
}

/*!
 * @brief   convert GIC to GPC
 * @param   hwirq
 * @retval  none
 * @note    none
 */
kint32_t fwk_gic_to_actual_irq(kint32_t hwirq)
{
    kchar_t *intc_name = "interrupt-controller";
    kuint32_t irq_base = 16;

    if (!g_isIntcGicDirect)
    {
        irq_base = 32;
        intc_name = "gpc";
    }

    hwirq -= irq_base;
    if (hwirq < 0)
        return hwirq;

    return fwk_irq_get_by_domain_name(intc_name, hwirq);
}

/*!
 * @brief   convert GPC to GIC
 * @param   virq
 * @retval  none
 * @note    none
 */
kint32_t fwk_gpc_to_gic_irq(kint32_t virq)
{
    struct fwk_irq_data *sptr_data;

    sptr_data = fwk_irq_get_data(virq);
    return isValid(sptr_data) ? (sptr_data->hwirq + 32) : -1;
}

/*!< --------------------------------------------------------------------------- */
/*!
 * @brief   hardware irqnumber translate
 * @param   sptr_domain: parent interrupt controller
 * @param   sptr_intc: parent interrupt controller's device-node
 * @param   intspec: of_handle_args::args[]
 * @param   intsize: property "interrupts" cell number
 * @retval  none
 * @note    none
 */
static kint32_t gpc_irq_domain_xlate(struct fwk_irq_domain *sptr_domain, struct fwk_device_node *sptr_intc,
				const kuint32_t *intspec, kuint32_t intsize, kuint32_t *out_hwirq, kuint32_t *out_type)
{
	if (sptr_domain->sptr_node != sptr_intc)
		return -ER_INVALID;
	if (intsize != 3)
		return -ER_INVALID;

    /*!< not allow GIC_SGI */
    if (intspec[0] != 0)
        return -ER_INVALID;

	*out_hwirq = intspec[1];
    *out_type = intspec[2];

	return ER_NORMAL;
}

static const struct fwk_irq_domain_ops sgtc_gpc_domain_hierarchy_ops = 
{
	.xlate = gpc_irq_domain_xlate,
	.alloc = gic_irq_domain_alloc,
	.free = gic_irq_domain_free,
};

/*!
 * @brief   initial GPC
 * @param   sptr_node: current interrupt-controller
 * @param   sptr_parent: interrupt-controller parent
 * @retval  none
 * @note    none
 */
kint32_t fwk_gpc_of_init(struct fwk_device_node *sptr_node, struct fwk_device_node *sptr_parent)
{
    struct fwk_irq_domain *sptr_domain, *sptr_par;

    if (!isValid(sptr_parent) || !isValid(sptr_node))
        return -ER_INVALID;

    /*! get gic */
    sptr_par = fwk_of_irq_host(sptr_parent);
    if (!isValid(sptr_par))
        return -ER_INVALID;

    sptr_domain = fwk_irq_domain_add_hierarchy(sptr_par, sptr_node, CA7_MAX_GPC_NR, &sgtc_gpc_domain_hierarchy_ops, mr_nullptr);
    if (!isValid(sptr_domain))
        return -ER_FAILD;

    sptr_domain->hwirq = 32;
    g_isIntcGicDirect = false;

    return ER_NORMAL;
}

/* end of file */
