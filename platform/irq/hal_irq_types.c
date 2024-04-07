/*
 * Interrupt Interface Defines
 *
 * File Name:   hal_irq_types.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.11.14
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/irq/hal_irq_types.h>
#include <platform/irq/hal_irq_domain.h>
#include <platform/of/hal_of.h>

/*!< The globals */

/*!< API function */
/*!
 * @brief  hal_default_irqhandler
 * @param  none
 * @retval none
 * @note   default irq handler
 */
static irq_return_t hal_default_irqhandler(void *ptrDev)
{
	return mrt_retval(Ert_isWell);
}

/*!
 * @brief  hal_request_irq
 * @param  none
 * @retval none
 * @note   irq register
 */
void *hal_find_irq_action(ksint32_t irq, const kstring_t *name, void *ptrDev)
{
	srt_hal_irq_desc_t *sprt_desc;
	srt_hal_irq_action_t *sprt_action;
	ksint32_t retval = 0;

	sprt_desc = hal_irq_to_desc(irq);
	if (!mrt_isValid(sprt_desc))
	{
		return mrt_nullptr;
	}

	foreach_list_next_entry(sprt_action, &sprt_desc->sgrt_action, sgrt_link)
	{
		if (mrt_isValid(name))
		{
			retval = strncmp(sprt_action->name, name, HAL_IRQ_DESC_NAME_LENTH);
		}

		if (!retval && (sprt_action->ptrArgs == ptrDev))
		{
			return sprt_action;
		}
	}

	return mrt_nullptr;
}

/*!
 * @brief  hal_request_irq
 * @param  none
 * @retval none
 * @note   irq register
 */
ksint32_t hal_request_irq(ksint32_t irq, irq_handler_t handler, kuint32_t flags, const kstring_t *name, void *ptrDev)
{
	srt_hal_irq_desc_t *sprt_desc;
	srt_hal_irq_action_t *sprt_action;

	if (!mrt_isValid(name) || !mrt_isValid(ptrDev))
	{
		return mrt_retval(Ert_isArgFault);
	}

	if (hal_find_irq_action(irq, name, ptrDev))
	{
		return mrt_retval(Ert_isExisted);
	}

	sprt_desc = hal_irq_to_desc(irq);
	if (!mrt_isValid(sprt_desc))
	{
		return mrt_retval(Ert_isMemErr);
	}

	sprt_action = (srt_hal_irq_action_t *)kzalloc(sizeof(*sprt_action), GFP_KERNEL);
	if (!mrt_isValid(sprt_desc))
	{
		return mrt_retval(Ert_isMemErr);
	}

	sprt_action->handler = mrt_isValid(handler) ? handler : hal_default_irqhandler;
	sprt_action->flags = flags;
	sprt_action->ptrArgs = ptrDev;
	strcpy(sprt_action->name, name);
	init_list_head(&sprt_action->sgrt_link);
	
	list_head_add_tail(&sprt_desc->sgrt_action, &sprt_action->sgrt_link);

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief  hal_free_irq
 * @param  none
 * @retval none
 * @note   irq unregister
 */
void hal_free_irq(ksint32_t irq, void *ptrDev)
{
	srt_hal_irq_action_t *sprt_action;

	if ((irq < 0) || !mrt_isValid(ptrDev))
	{
		return;
	}

	sprt_action = hal_find_irq_action(irq, mrt_nullptr, ptrDev);
	if (mrt_isValid(sprt_action))
	{
		list_head_del(&sprt_action->sgrt_link);
		kfree(sprt_action);
	}
}

/*!
 * @brief   hal_do_irq_handler
 * @param   none
 * @retval  none
 * @note    excute irq handler
 */
void hal_do_irq_handler(ksint32_t softIrq)
{
	srt_hal_irq_desc_t *sprt_desc;
	srt_hal_irq_action_t *sprt_action;
	ksint32_t retval;

	if (softIrq < 0)
	{
		return;
	}

	sprt_desc = hal_irq_to_desc(softIrq);
	if (!mrt_isValid(sprt_desc))
	{
		return;
	}

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
 * @brief   hal_handle_softirq
 * @param   none
 * @retval  none
 * @note    excute irq handler
 */
void hal_handle_softirq(ksint32_t softIrq, kuint32_t event)
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
