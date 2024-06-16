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
static void fwk_irq_chip_dummy(struct fwk_irq_data *sprt_data) {}

struct fwk_irq_chip sgrt_fwk_irq_dummy_chip =
{
	.irq_enable = fwk_irq_chip_dummy,
	.irq_disable = fwk_irq_chip_dummy,
	.irq_mask = fwk_irq_chip_dummy,
	.irq_unmask = fwk_irq_chip_dummy,
	.irq_ack = mrt_nullptr,
	.irq_set_type = mrt_nullptr,
};

/*!
 * @brief   default enable API
 * @param   sprt_data
 * @retval  none
 * @note    none
 */
static void fwk_irq_chip_gc_enable(struct fwk_irq_data *sprt_data)
{

}

/*!
 * @brief   default disable API
 * @param   sprt_data
 * @retval  none
 * @note    none
 */
static void fwk_irq_chip_gc_disable(struct fwk_irq_data *sprt_data)
{

}

/*!
 * @brief   irq register enable
 * @param   sprt_data
 * @retval  none
 * @note    none
 */
static void fwk_irq_chip_gc_mask(struct fwk_irq_data *sprt_data)
{
	struct fwk_irq_generic *sprt_gc;

	sprt_gc = (struct fwk_irq_generic *)sprt_data->handler_data;
	if (!sprt_gc)
		return;

	/*!< manage_reg: write 1 to enable; status_reg: write 1 to reset */
	mrt_setbitl(sprt_data->mask, sprt_gc->manage_reg);
	mrt_setbitl(sprt_data->mask, sprt_gc->status_reg);
}

/*!
 * @brief   irq register disable
 * @param   sprt_data
 * @retval  none
 * @note    none
 */
static void fwk_irq_chip_gc_unmask(struct fwk_irq_data *sprt_data)
{
	struct fwk_irq_generic *sprt_gc;

	sprt_gc = (struct fwk_irq_generic *)sprt_data->handler_data;
	if (!sprt_gc)
		return;

	mrt_clrbitl(sprt_data->mask, sprt_gc->manage_reg);
}

/*!
 * @brief   get irq status register
 * @param   sprt_data
 * @retval  1: irq occur; 0: no irq
 * @note    none
 */
static kbool_t fwk_irq_chip_gc_ack(struct fwk_irq_data *sprt_data)
{
	struct fwk_irq_generic *sprt_gc;

	sprt_gc = (struct fwk_irq_generic *)sprt_data->handler_data;
	if (!sprt_gc)
		return false;

	return !!mrt_getbitl(sprt_data->mask, sprt_gc->status_reg);
}

/*!
 * @brief   check if irq register is enabled
 * @param   sprt_data
 * @retval  1: enabled; 0: close
 * @note    none
 */
static kbool_t fwk_irq_chip_gc_is_enabled(struct fwk_irq_data *sprt_data)
{
	struct fwk_irq_generic *sprt_gc;

	sprt_gc = (struct fwk_irq_generic *)sprt_data->handler_data;
	if (!sprt_gc)
		return false;

	return !!mrt_getbitl(sprt_data->mask, sprt_gc->manage_reg);
}

struct fwk_irq_chip sgrt_fwk_irq_generic_chip =
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
 * @param   sprt_data, data
 * @retval  none
 * @note    none
 */
void fwk_irq_set_chip_data(struct fwk_irq_data *sprt_data, void *data)
{
	sprt_data->chip_data = data;
}

/*!
 * @brief   save privdata to irq_chip
 * @param   sprt_data, sprt_chip, data
 * @retval  none
 * @note    none
 */
void fwk_irq_set_chip_handler(struct fwk_irq_data *sprt_data, struct fwk_irq_chip *sprt_chip, void *data)
{
	sprt_data->sprt_chip = sprt_chip;
	sprt_data->handler_data = data;
}

/*!
 * @brief   save data to irq_chip
 * @param   irq_base, sprt_gc, chip_data
 * @retval  none
 * @note    none
 */
void fwk_irq_setup_generic_chip(kint32_t irq_base, kuint32_t irq_max, struct fwk_irq_generic *sprt_gc, void *chip_data)
{
	struct fwk_irq_data *sprt_data;
	kuint32_t irq;

	for (irq = irq_base; irq < (irq_base + irq_max); irq++)
	{
		sprt_data = fwk_irq_get_data(irq);
		if (!sprt_data)
			continue;

		sprt_data->mask = mrt_bit(irq - irq_base);

		fwk_irq_set_chip_data(sprt_data, chip_data);
		fwk_irq_set_chip_handler(sprt_data, &sprt_gc->sgrt_chip, sprt_gc);
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
	struct fwk_irq_data *sprt_data;
	kuint32_t irq;

	for (irq = irq_base; irq < (irq_base + irq_max); irq++)
	{
		sprt_data = fwk_irq_get_data(irq);
		if (!sprt_data)
			continue;

		sprt_data->mask = 0;
		fwk_irq_set_chip_data(sprt_data, mrt_nullptr);
		fwk_irq_set_chip_handler(sprt_data, &sgrt_fwk_irq_dummy_chip, mrt_nullptr);
	}
}

/*!
 * @brief   get data
 * @param   sprt_data
 * @retval  generic data
 * @note    none
 */
struct fwk_irq_generic *fwk_irq_get_generic_data(struct fwk_irq_data *sprt_data)
{
	struct fwk_irq_generic *sprt_gc;

	sprt_gc = mrt_container_of(sprt_data->sprt_chip, typeof(*sprt_gc), sgrt_chip);
	return sprt_gc;
}

/*!
 * @brief   irq enable
 * @param   irq
 * @retval  none
 * @note    none
 */
void fwk_enable_irq(kint32_t irq)
{
	struct fwk_irq_data *sprt_data;

	sprt_data = fwk_irq_get_data(irq);
	if (!sprt_data)
		return;

	if (sprt_data->sprt_chip->irq_enable)
		sprt_data->sprt_chip->irq_enable(sprt_data);
	else if (sprt_data->sprt_chip->irq_mask)
		sprt_data->sprt_chip->irq_mask(sprt_data);
}

/*!
 * @brief   irq disable
 * @param   irq
 * @retval  none
 * @note    none
 */
void fwk_disable_irq(kint32_t irq)
{
	struct fwk_irq_data *sprt_data;

	sprt_data = fwk_irq_get_data(irq);
	if (!sprt_data)
		return;

	if (sprt_data->sprt_chip->irq_disable)
		sprt_data->sprt_chip->irq_disable(sprt_data);
	else if (sprt_data->sprt_chip->irq_mask)
		sprt_data->sprt_chip->irq_unmask(sprt_data);
}

/*!
 * @brief   set irq trigger type
 * @param   irq, flags
 * @retval  none
 * @note    none
 */
void fwk_irq_set_type(kint32_t irq, kuint32_t flags)
{
	struct fwk_irq_data *sprt_data;
	struct fwk_irq_desc *sprt_desc;
	kuint32_t type;

	sprt_data = fwk_irq_get_data(irq);
	if (!sprt_data)
		return;

	sprt_desc = fwk_irq_data_to_desc(sprt_data);

	type = flags & IRQ_TYPE_SENSE_MASK;
	if (!type)
		type = sprt_desc->flags & IRQ_TYPE_SENSE_MASK;

	if (sprt_data->sprt_chip->irq_set_type)
		sprt_data->sprt_chip->irq_set_type(sprt_data, type);
}

/*!
 * @brief   check if irq is happend
 * @param   irq
 * @retval  1: irq occur
 * @note    none
 */
kbool_t fwk_irq_is_acked(kint32_t irq)
{
	struct fwk_irq_data *sprt_data;

	sprt_data = fwk_irq_get_data(irq);
	if (!sprt_data)
		return false;

	if (sprt_data->sprt_chip->irq_ack)
		return sprt_data->sprt_chip->irq_ack(sprt_data);

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
	struct fwk_irq_data *sprt_data;

	sprt_data = fwk_irq_get_data(irq);
	if (!sprt_data)
		return false;

	if (sprt_data->sprt_chip->irq_is_enabled)
		return sprt_data->sprt_chip->irq_is_enabled(sprt_data);

	return false;
}

/*!< end of file */
