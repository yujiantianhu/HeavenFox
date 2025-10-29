/*
 * Platform Bus Interface of Hardware Abstraction Layer
 *
 * File Name:   fwk_platdrv.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.23
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_platform.h>
#include <platform/base/fwk_platdrv.h>

/*!< The functions */
static kint32_t fwk_driver_attach(struct fwk_driver *sptr_driver, struct fwk_bus_type *sptr_bus_type,
								kint32_t (*pFunc_Match) (struct fwk_device *, struct fwk_bus_type *, void *));
static kint32_t fwk_driver_detach(struct fwk_driver *sptr_driver, struct fwk_bus_type *sptr_bus_type);
static kint32_t fwk_driver_find(struct fwk_driver *sptr_driver, struct fwk_bus_type *sptr_bus_type);
static kint32_t fwk_driver_to_bus(struct fwk_driver *sptr_driver, struct fwk_bus_type *sptr_bus_type);
static kint32_t fwk_bus_del_driver(struct fwk_driver *sptr_driver, struct fwk_bus_type *sptr_bus_type);

/*!< API function */
/*!
 * @brief   Register platform driver
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_register_platdriver(struct fwk_platdrv *sptr_platdrv)
{
	sptr_platdrv->sgtc_driver.sptr_bus	= &sgtc_fwk_platform_bus_type;

	return fwk_driver_register(&sptr_platdrv->sgtc_driver);
}

/*!
 * @brief   Unegister platform driver
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_unregister_platdriver(struct fwk_platdrv *sptr_platdrv)
{
	return fwk_driver_unregister(&sptr_platdrv->sgtc_driver);
}

/*!< --------------------------------------------------------------------------
							Device Driver Interface					  	 
 --------------------------------------------------------------------------- */
/*!
 * @brief   Device-driver matching
 * @param   driver, platform-bus, match-callback
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_driver_attach(struct fwk_driver *sptr_driver, struct fwk_bus_type *sptr_bus_type,
								kint32_t (*pFunc_Match) (struct fwk_device *, struct fwk_bus_type *, void *))
{
	struct fwk_device *sptr_dev;
	kint32_t matches;
	kint32_t retval = 0;

	DECLARE_LIST_HEAD_PTR(sptr_list);
	DECLARE_LIST_HEAD_PTR(sptr_parent);

	/*!< First check if the match function is defined */
	if (!sptr_bus_type->match)
		return -ER_ERROR;

	matches	= 0;
	FWK_INIT_BUS_DEVICE_LIST(sptr_parent, sptr_list, sptr_bus_type);
	__BUS_DEVICE_RD_LOCK(sptr_bus_type);

	/*!< Take out the devices on the bus in turn */
	while ((sptr_dev = FWK_NEXT_DEVICE(sptr_parent, sptr_list)))
	{
		/*!< The device is matched with drivers */
		if (sptr_dev->sptr_driver)
			continue;

		/*!< Match one by one */
		/*!< One driver supports matching multiple devices, and does not exit until the linked list is fully traversed */
		/*!< However, try not to mount multiple devices with the same driver at the same time to avoid misoperation */
		retval = (*pFunc_Match)(sptr_dev, sptr_bus_type, sptr_driver);
		if (!retval || (retval == -ER_PERMIT))
		{
			/*!< Record the number of matching devices */
			matches++;
		}
	}

	__BUS_DEVICE_RD_UNLOCK(sptr_bus_type);
	sptr_driver->matches += matches;

	/*!<
	 * ER_NOTFOUND: no device can be matched
	 * ER_PERMIT: matched already, but probe failed;
	 * ER_NORMAL: matched already, and probe successfully;
	 * other (retval < 0): error
	 */
	return ((retval == (-ER_NOTFOUND)) || matches) ? matches : retval;
}

/*!
 * @brief   Driver seperation
 * @param   driver, platform-bus
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_driver_detach(struct fwk_driver *sptr_driver, struct fwk_bus_type *sptr_bus_type)
{
	struct fwk_device *sptr_dev;

	DECLARE_LIST_HEAD_PTR(sptr_list);
	DECLARE_LIST_HEAD_PTR(sptr_parent);

	FWK_INIT_BUS_DEVICE_LIST(sptr_parent, sptr_list, sptr_bus_type);
	__BUS_DEVICE_RD_LOCK(sptr_bus_type);

	/*!< Take out the devices on the bus in turn */
	while ((sptr_dev = FWK_NEXT_DEVICE(sptr_parent, sptr_list)))
	{
		/*!< All devices that match this drive are separated */
		if (sptr_dev->sptr_driver != sptr_driver)
			continue;

		/*!< Do the preparation before separation */
		fwk_device_driver_remove(sptr_dev);

		/*!< separation */
		sptr_dev->sptr_driver = mr_nullptr;
		sptr_driver->matches--;
	}

	__BUS_DEVICE_RD_UNLOCK(sptr_bus_type);
	return ER_NORMAL;
}

/*!
 * @brief   Find driver on bus
 * @param   driver, platform-bus
 * @retval  errno
 * @note    Detects whether the drive to be registered is hooked up on the bus
 */
static kint32_t fwk_driver_find(struct fwk_driver *sptr_driver, struct fwk_bus_type *sptr_bus_type)
{
	struct fwk_driver *sptr_drv;

	__BUS_DRIVER_RD_LOCK(sptr_bus_type);

	foreach_list_next_entry(sptr_drv, FWK_GET_BUS_DRIVER(sptr_bus_type), sgtc_link)
	{
		if (!kstrcmp((char *)sptr_drv->name, (char *)sptr_driver->name))
		{
			__BUS_DRIVER_RD_UNLOCK(sptr_bus_type);
			return ER_NORMAL;
		}
	}

	__BUS_DRIVER_RD_UNLOCK(sptr_bus_type);
	return -ER_ERROR;
}

/*!
 * @brief   Add driver to platform-bus
 * @param   driver, platform-bus
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_driver_to_bus(struct fwk_driver *sptr_driver, struct fwk_bus_type *sptr_bus_type)
{
	/*!< Insert from the tail of the bus */
	__BUS_DRIVER_WR_LOCK(sptr_bus_type);
	list_head_add_tail(FWK_GET_BUS_DRIVER(sptr_bus_type), &sptr_driver->sgtc_link);
	__BUS_DRIVER_WR_UNLOCK(sptr_bus_type);

	/*!< Execute the device driver matching logic */
	return fwk_driver_attach(sptr_driver, sptr_bus_type, fwk_device_driver_match);
}

/*!
 * @brief   Detach driver on bus
 * @param   driver, platform-bus
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_bus_del_driver(struct fwk_driver *sptr_driver, struct fwk_bus_type *sptr_bus_type)
{
	kint32_t retval;

	/*!< Perform the device drive separation procedure */
	retval = fwk_driver_detach(sptr_driver, sptr_bus_type);

	/*!< Remove the driver from the bus */
	__BUS_DRIVER_WR_LOCK(sptr_bus_type);
	list_head_del_safe(FWK_GET_BUS_DRIVER(sptr_bus_type), &sptr_driver->sgtc_link);
	__BUS_DRIVER_WR_UNLOCK(sptr_bus_type);

	return retval;
}

/*!
 * @brief   Register Driver
 * @param   driver
 * @retval  errno
 * @note    none
 */
kint32_t fwk_driver_register(struct fwk_driver *sptr_driver)
{
	struct fwk_bus_type *sptr_bus_type;
	kint32_t retval;

	sptr_bus_type = sptr_driver->sptr_bus;

	/*!< The bus does not exist and cannot be registered */
	if (!sptr_bus_type)
		goto fail;

	/*!< The bus does not exist and cannot be registered */
	if (!sptr_bus_type->sptr_buspriv)
		goto fail;

	/*!< Is the driver registered? No more duplicate registrations */
	retval = fwk_driver_find(sptr_driver, sptr_bus_type);
	if (!retval)
		goto fail;

	/*!< For the first registration, the number of device matches is initialized to 0 */
	sptr_driver->matches = 0;

	/*!< Add the driver to the bus, and if the match is successful, return Well */
	return fwk_driver_to_bus(sptr_driver, sptr_bus_type);

fail:
	return -ER_ERROR;
}

/*!
 * @brief   Unregister driver
 * @param   driver
 * @retval  errno
 * @note    none
 */
kint32_t fwk_driver_unregister(struct fwk_driver *sptr_driver)
{
	struct fwk_bus_type *sptr_bus_type;
	kint32_t retval;

	sptr_bus_type = sptr_driver->sptr_bus;

	/*!< The bus does not exist and cannot be logged out */
	if (!sptr_bus_type)
		goto fail;

	/*!< The bus does not exist and cannot be logged out */
	if (!sptr_bus_type->sptr_buspriv)
		goto fail;

	/*!< The driver is not registered? Go straight back */
	retval = fwk_driver_find(sptr_driver, sptr_bus_type);
	if (retval < 0)
		goto fail;

	/*!< Remove the driver from the bus */
	return fwk_bus_del_driver(sptr_driver, sptr_bus_type);

fail:
	return -ER_ERROR;
}
