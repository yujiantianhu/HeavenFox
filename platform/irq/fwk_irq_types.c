/*
 * Interrupt Interface Defines
 *
 * File Name:   fwk_irq_types.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.11.14
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/irq/fwk_irq_types.h>
#include <platform/irq/fwk_irq_domain.h>
#include <platform/of/fwk_of.h>

/*!< The globals */

/*!< API function */
/*!
 * @brief  fwk_default_irqhandler
 * @param  none
 * @retval none
 * @note   default irq handler
 */
static irq_return_t fwk_default_irqhandler(void *ptrDev)
{
	return NR_IS_NORMAL;
}

/*!
 * @brief  fwk_request_irq
 * @param  none
 * @retval none
 * @note   irq register
 */
void *fwk_find_irq_action(ksint32_t irq, const kstring_t *name, void *ptrDev)
{
	srt_fwk_irq_desc_t *sprt_desc;
	srt_fwk_irq_action_t *sprt_action;
	ksint32_t retval = 0;

	sprt_desc = fwk_irq_to_desc(irq);
	if (!isValid(sprt_desc))
		return mrt_nullptr;

	foreach_list_next_entry(sprt_action, &sprt_desc->sgrt_action, sgrt_link)
	{
		if (name)
			retval = strncmp(sprt_action->name, name, FWK_IRQ_DESC_NAME_LENTH);

		if (!retval && (sprt_action->ptrArgs == ptrDev))
			return sprt_action;
	}

	return mrt_nullptr;
}

/*!
 * @brief  fwk_request_irq
 * @param  none
 * @retval none
 * @note   irq register
 */
ksint32_t fwk_request_irq(ksint32_t irq, irq_handler_t handler, kuint32_t flags, const kstring_t *name, void *ptrDev)
{
	srt_fwk_irq_desc_t *sprt_desc;
	srt_fwk_irq_action_t *sprt_action;

	if ((!name) || (!ptrDev))
		return -NR_IS_FAULT;

	if (fwk_find_irq_action(irq, name, ptrDev))
		return -NR_IS_EXISTED;

	sprt_desc = fwk_irq_to_desc(irq);
	if (!isValid(sprt_desc))
		return -NR_IS_NOMEM;

	sprt_action = (srt_fwk_irq_action_t *)kzalloc(sizeof(*sprt_action), GFP_KERNEL);
	if (!isValid(sprt_action))
		return -NR_IS_NOMEM;

	sprt_action->handler = handler ? handler : fwk_default_irqhandler;
	sprt_action->flags = flags;
	sprt_action->ptrArgs = ptrDev;
	strcpy(sprt_action->name, name);
	
	list_head_add_tail(&sprt_desc->sgrt_action, &sprt_action->sgrt_link);

	return NR_IS_NORMAL;
}

/*!
 * @brief  fwk_free_irq
 * @param  none
 * @retval none
 * @note   irq unregister
 */
void fwk_free_irq(ksint32_t irq, void *ptrDev)
{
	srt_fwk_irq_action_t *sprt_action;

	if ((irq < 0) || (!ptrDev))
		return;

	sprt_action = fwk_find_irq_action(irq, mrt_nullptr, ptrDev);
	if (isValid(sprt_action))
	{
		list_head_del(&sprt_action->sgrt_link);
		kfree(sprt_action);
	}
}

/*!
 * @brief   destroy the content of irq_desc
 * @param   irq
 * @retval  none
 * @note    none
 */
void fwk_destroy_irq_desc(ksint32_t irq)
{
	srt_fwk_irq_desc_t *sprt_desc;
	srt_fwk_irq_action_t *sprt_action, *sprt_temp;

	sprt_desc = fwk_irq_to_desc(irq);
	if (!isValid(sprt_desc))
		return;

	foreach_list_next_entry_safe(sprt_action, sprt_temp, &sprt_desc->sgrt_action, sgrt_link)
	{
		list_head_del(&sprt_action->sgrt_link);
		kfree(sprt_action);
	}
}

/*!
 * @brief   fwk_do_irq_handler
 * @param   none
 * @retval  none
 * @note    excute irq handler
 */
void fwk_do_irq_handler(ksint32_t softIrq)
{
	srt_fwk_irq_desc_t *sprt_desc;
	srt_fwk_irq_action_t *sprt_action;
	ksint32_t retval;

	if (softIrq < 0)
		return;

	sprt_desc = fwk_irq_to_desc(softIrq);
	if (!isValid(sprt_desc))
		return;
		
	foreach_list_next_entry(sprt_action, &sprt_desc->sgrt_action, sgrt_link)
	{
		retval = sprt_action->handler ? sprt_action->handler(sprt_action->ptrArgs) : -1;
		switch (retval)
		{
			case 0:
				break;

			case 1:
				break;

			default:
				break;
		}
	}
}

/*!
 * @brief   fwk_handle_softirq
 * @param   none
 * @retval  none
 * @note    excute irq handler
 */
void fwk_handle_softirq(ksint32_t softIrq, kuint32_t event)
{
	switch (event)
	{
		case SWI_EVENT_SCHEDULED:
			print_info("trigger NR_EVENT_SCHEDULED \n");
			break;
		
		case SWI_EVENT_SYSCALL:
			print_info("trigger NR_EVENT_SYSCALL \n");
			break;

		default: break;
	}
}

/* end of file */
