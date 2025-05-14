/*
 * Interrupt Interface Defines
 *
 * File Name:   fwk_irq_chip.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.13
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/irq/fwk_irq_types.h>
#include <platform/irq/fwk_irq_chip.h>

/*!< API function */
static void fwk_irq_chip_dummy(struct fwk_irq_data *sptr_data) {}

struct fwk_irq_chip sgtc_fwk_irq_dummy_chip =
{
	.irq_enable = fwk_irq_chip_dummy,
	.irq_disable = fwk_irq_chip_dummy,
	.irq_mask = fwk_irq_chip_dummy,
	.irq_unmask = fwk_irq_chip_dummy,
	.irq_ack = mr_nullptr,
	.irq_set_type = mr_nullptr,
};

/*!
 * @brief   default enable API
 * @param   sptr_data
 * @retval  none
 * @note    none
 */
static void fwk_irq_chip_gc_enable(struct fwk_irq_data *sptr_data)
{

}

/*!
 * @brief   default disable API
 * @param   sptr_data
 * @retval  none
 * @note    none
 */
static void fwk_irq_chip_gc_disable(struct fwk_irq_data *sptr_data)
{

}

/*!
 * @brief   irq register enable
 * @param   sptr_data
 * @retval  none
 * @note    none
 */
static void fwk_irq_chip_gc_mask(struct fwk_irq_data *sptr_data)
{
	struct fwk_irq_generic *sptr_gc;

	sptr_gc = (struct fwk_irq_generic *)sptr_data->handler_data;
	if (!sptr_gc)
		return;

	/*!< manage_reg: write 1 to enable; status_reg: write 1 to reset */
	mr_setbitl(sptr_data->mask, sptr_gc->manage_reg);
	mr_setbitl(sptr_data->mask, sptr_gc->status_reg);
}

/*!
 * @brief   irq register disable
 * @param   sptr_data
 * @retval  none
 * @note    none
 */
static void fwk_irq_chip_gc_unmask(struct fwk_irq_data *sptr_data)
{
	struct fwk_irq_generic *sptr_gc;

	sptr_gc = (struct fwk_irq_generic *)sptr_data->handler_data;
	if (!sptr_gc)
		return;

	mr_clrbitl(sptr_data->mask, sptr_gc->manage_reg);
}

/*!
 * @brief   get irq status register
 * @param   sptr_data
 * @retval  1: irq occur; 0: no irq
 * @note    none
 */
static kbool_t fwk_irq_chip_gc_ack(struct fwk_irq_data *sptr_data)
{
	struct fwk_irq_generic *sptr_gc;

	sptr_gc = (struct fwk_irq_generic *)sptr_data->handler_data;
	if (!sptr_gc)
		return false;

	return !!mr_getbitl(sptr_data->mask, sptr_gc->status_reg);
}

/*!
 * @brief   check if irq register is enabled
 * @param   sptr_data
 * @retval  1: enabled; 0: close
 * @note    none
 */
static kbool_t fwk_irq_chip_gc_is_enabled(struct fwk_irq_data *sptr_data)
{
	struct fwk_irq_generic *sptr_gc;

	sptr_gc = (struct fwk_irq_generic *)sptr_data->handler_data;
	if (!sptr_gc)
		return false;

	return !!mr_getbitl(sptr_data->mask, sptr_gc->manage_reg);
}

struct fwk_irq_chip sgtc_fwk_irq_generic_chip =
{
	.irq_enable = fwk_irq_chip_gc_enable,
	.irq_disable = fwk_irq_chip_gc_disable,
	.irq_mask = fwk_irq_chip_gc_mask,
	.irq_unmask = fwk_irq_chip_gc_unmask,
	.irq_ack = fwk_irq_chip_gc_ack,
	.irq_is_enabled = fwk_irq_chip_gc_is_enabled,
};

/*!
 * @brief   save privdata to irq_chip
 * @param   sptr_data, data
 * @retval  none
 * @note    none
 */
void fwk_irq_set_chip_data(struct fwk_irq_data *sptr_data, void *data)
{
	sptr_data->chip_data = data;
}

/*!
 * @brief   save privdata to irq_chip
 * @param   sptr_data, sptr_chip, data
 * @retval  none
 * @note    none
 */
void fwk_irq_set_chip_handler(struct fwk_irq_data *sptr_data, struct fwk_irq_chip *sptr_chip, void *data)
{
	sptr_data->sptr_chip = sptr_chip;
	sptr_data->handler_data = data;
}

/*!
 * @brief   save data to irq_chip
 * @param   irq_base, sptr_gc, chip_data
 * @retval  none
 * @note    none
 */
void fwk_irq_setup_generic_chip(kint32_t irq_base, kuint32_t irq_max, struct fwk_irq_generic *sptr_gc, void *chip_data)
{
	struct fwk_irq_data *sptr_data;
	kuint32_t irq;

	for (irq = irq_base; irq < (irq_base + irq_max); irq++)
	{
		sptr_data = fwk_irq_get_data(irq);
		if (!sptr_data)
			continue;

		sptr_data->mask = mr_bit(irq - irq_base);

		fwk_irq_set_chip_data(sptr_data, chip_data);
		fwk_irq_set_chip_handler(sptr_data, &sptr_gc->sgtc_chip, sptr_gc);
	}
}

/*!
 * @brief   delete data in irq_chip
 * @param   irq_base, irq_max
 * @retval  none
 * @note    none
 */
void fwk_irq_shutdown_generic_chip(kint32_t irq_base, kuint32_t irq_max)
{
	struct fwk_irq_data *sptr_data;
	kuint32_t irq;

	for (irq = irq_base; irq < (irq_base + irq_max); irq++)
	{
		sptr_data = fwk_irq_get_data(irq);
		if (!sptr_data)
			continue;

		sptr_data->mask = 0;
		fwk_irq_set_chip_data(sptr_data, mr_nullptr);
		fwk_irq_set_chip_handler(sptr_data, &sgtc_fwk_irq_dummy_chip, mr_nullptr);
	}
}

/*!
 * @brief   get data
 * @param   sptr_data
 * @retval  generic data
 * @note    none
 */
struct fwk_irq_generic *fwk_irq_get_generic_data(struct fwk_irq_data *sptr_data)
{
	struct fwk_irq_generic *sptr_gc;

	sptr_gc = mr_container_of(sptr_data->sptr_chip, typeof(*sptr_gc), sgtc_chip);
	return sptr_gc;
}

/*!
 * @brief   irq enable
 * @param   irq
 * @retval  none
 * @note    none
 */
void fwk_enable_irq(kint32_t irq)
{
	struct fwk_irq_data *sptr_data;

	sptr_data = fwk_irq_get_data(irq);
	if (!sptr_data)
		return;

	if (sptr_data->sptr_chip->irq_enable)
		sptr_data->sptr_chip->irq_enable(sptr_data);
	else if (sptr_data->sptr_chip->irq_mask)
		sptr_data->sptr_chip->irq_mask(sptr_data);
}

/*!
 * @brief   irq disable
 * @param   irq
 * @retval  none
 * @note    none
 */
void fwk_disable_irq(kint32_t irq)
{
	struct fwk_irq_data *sptr_data;

	sptr_data = fwk_irq_get_data(irq);
	if (!sptr_data)
		return;

	if (sptr_data->sptr_chip->irq_disable)
		sptr_data->sptr_chip->irq_disable(sptr_data);
	else if (sptr_data->sptr_chip->irq_mask)
		sptr_data->sptr_chip->irq_unmask(sptr_data);
}

/*!
 * @brief   set irq trigger type
 * @param   irq, flags
 * @retval  none
 * @note    none
 */
void fwk_irq_set_type(kint32_t irq, kuint32_t flags)
{
	struct fwk_irq_data *sptr_data;
	struct fwk_irq_desc *sptr_desc;
	kuint32_t type;

	sptr_data = fwk_irq_get_data(irq);
	if (!sptr_data)
		return;

	sptr_desc = fwk_irq_data_to_desc(sptr_data);

	type = flags & IRQ_TYPE_SENSE_MASK;
	if (!type)
		type = sptr_desc->flags & IRQ_TYPE_SENSE_MASK;

	if (sptr_data->sptr_chip->irq_set_type)
		sptr_data->sptr_chip->irq_set_type(sptr_data, type);
}

/*!
 * @brief   check if irq is happend
 * @param   irq
 * @retval  1: irq occur
 * @note    none
 */
kbool_t fwk_irq_is_acked(kint32_t irq)
{
	struct fwk_irq_data *sptr_data;

	sptr_data = fwk_irq_get_data(irq);
	if (!sptr_data)
		return false;

	if (sptr_data->sptr_chip->irq_ack)
		return sptr_data->sptr_chip->irq_ack(sptr_data);

	return false;
}

/*!
 * @brief   check if irq is enabled
 * @param   irq
 * @retval  1: irq enabled
 * @note    none
 */
kbool_t fwk_irq_is_enabled(kint32_t irq)
{
	struct fwk_irq_data *sptr_data;

	sptr_data = fwk_irq_get_data(irq);
	if (!sptr_data)
		return false;

	if (sptr_data->sptr_chip->irq_is_enabled)
		return sptr_data->sptr_chip->irq_is_enabled(sptr_data);

	return false;
}

/*!< end of file */
