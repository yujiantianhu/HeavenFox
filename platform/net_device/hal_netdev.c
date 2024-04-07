/*
 * Hardware Abstraction Layer Net Interface
 *
 * File Name:   hal_netdev.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.07.06
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/net/hal_netdev.h>

/*!< API function */
/*!
 * @brief   Allocate network device
 * @param   none
 * @retval  none
 * @note    none
 */
struct hal_net_device *hal_alloc_netdev_mq(ksint32_t sizeof_priv, const kstring_t *name,
													void (*setup) (struct hal_net_device *sprt_dev), kuint32_t queue_count)
{
	struct hal_net_device *sprt_netdev;
	struct hal_netdev_queue *sprt_tx;

	sprt_netdev = (struct hal_net_device *)kzalloc(sizeof(struct hal_net_device), GFP_KERNEL);
	if (!mrt_isValid(sprt_netdev))
	{
		return mrt_nullptr;
	}

	sprt_tx = (struct hal_netdev_queue *)kzalloc(queue_count * sizeof(struct hal_netdev_queue), GFP_KERNEL);
	if (!mrt_isValid(sprt_tx))
	{
		kfree(sprt_netdev);
		return mrt_nullptr;
	}

	/*!< register send queue */
	sprt_netdev->sprt_tx = sprt_tx;
	sprt_netdev->num_tx_queues = queue_count;
	sprt_netdev->real_num_tx_queues = queue_count;

	/*!< initial recieve queue */
	memset(&sprt_netdev->sgrt_rcu, 0, sizeof(struct hal_netdev_queue));

	if (mrt_isValid(setup))
	{
		setup(sprt_netdev);
	}

	strcpy(sprt_netdev->name, name);

	return sprt_netdev;
}

/*!
 * @brief   relese network device
 * @param   none
 * @retval  none
 * @note    none
 */
void hal_free_netdev(struct hal_net_device *sprt_dev)
{
	if (!mrt_isValid(sprt_dev))
	{
		return;
	}

	/*!< release send queue first */
	if (mrt_isValid(sprt_dev->sprt_tx))
	{
		kfree(sprt_dev->sprt_tx);
	}

	kfree(sprt_dev);
}





