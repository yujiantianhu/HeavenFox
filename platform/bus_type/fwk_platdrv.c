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
#include <platform/fwk_platform.h>
#include <platform/fwk_platdrv.h>

/*!< The functions */
static ksint32_t fwk_driver_attach(struct fwk_driver *sprt_driver, struct fwk_bus_type *sprt_bus_type,
								ksint32_t (*pFunc_Match) (struct fwk_device *, struct fwk_bus_type *, void *));
static ksint32_t fwk_driver_detach(struct fwk_driver *sprt_driver, struct fwk_bus_type *sprt_bus_type);
static ksint32_t fwk_driver_find(struct fwk_driver *sprt_driver, struct fwk_bus_type *sprt_bus_type);
static ksint32_t fwk_driver_to_bus(struct fwk_driver *sprt_driver, struct fwk_bus_type *sprt_bus_type);
static ksint32_t fwk_bus_del_driver(struct fwk_driver *sprt_driver, struct fwk_bus_type *sprt_bus_type);
static ksint32_t fwk_driver_register(struct fwk_driver *sprt_driver);
static ksint32_t fwk_driver_unregister(struct fwk_driver *sprt_driver);

/*!< API function */
/*!
 * @brief   Register platform driver
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t fwk_register_platdriver(struct fwk_platdrv *sprt_platdrv)
{
	sprt_platdrv->sgrt_driver.sprt_bus_type	= &sgrt_fwk_platform_bus_type;

	return fwk_driver_register(&sprt_platdrv->sgrt_driver);
}

/*!
 * @brief   Unegister platform driver
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t fwk_unregister_platdriver(struct fwk_platdrv *sprt_platdrv)
{
	return fwk_driver_unregister(&sprt_platdrv->sgrt_driver);
}

/*!< -------------------------------------------------------------------------- */
/*!<							Device Driver Interface					  	 	*/
/*!< -------------------------------------------------------------------------- */
/*!
 * @brief   Device-driver matching
 * @param   driver, platform-bus, match-callback
 * @retval  errno
 * @note    none
 */
static ksint32_t fwk_driver_attach(struct fwk_driver *sprt_driver, struct fwk_bus_type *sprt_bus_type,
								ksint32_t (*pFunc_Match) (struct fwk_device *, struct fwk_bus_type *, void *))
{
	struct fwk_device *sprt_device;
	ksint32_t matches;
	ksint32_t retval = 0;

	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);

	/*!< First check if the match function is defined */
	if (!sprt_bus_type->match)
		return -NR_isAnyErr;

	matches	= 0;
	FWK_INIT_BUS_DEVICE_LIST(sprt_parent, sprt_list, sprt_bus_type);

	/*!< Take out the devices on the bus in turn */
	while ((sprt_device = FWK_NEXT_DEVICE(sprt_parent, sprt_list)))
	{
		/*!< The device is matched with drivers */
		if (sprt_device->sprt_driver)
			continue;

		/*!< Match one by one */
		/*!< One driver supports matching multiple devices, and does not exit until the linked list is fully traversed */
		/*!< However, try not to mount multiple devices with the same driver at the same time to avoid misoperation */
		retval = (*pFunc_Match)(sprt_device, sprt_bus_type, sprt_driver);
		if (!retval || (retval == -NR_isPermit))
		{
			/*!< Record the number of matching devices */
			matches++;
		}
	}

	sprt_driver->matches += matches;

	/*!<
	 * NR_isNotFound: no device can be matched
	 * NR_isPermit: matched already, but probe failed;
	 * NR_isWell: matched already, and probe successfully;
	 * other (retval < 0): error
	 */
	return ((retval == (-NR_isNotFound)) || matches) ? matches : retval;
}

/*!
 * @brief   Driver seperation
 * @param   driver, platform-bus
 * @retval  errno
 * @note    none
 */
static ksint32_t fwk_driver_detach(struct fwk_driver *sprt_driver, struct fwk_bus_type *sprt_bus_type)
{
	struct fwk_device *sprt_device;

	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);

	FWK_INIT_BUS_DEVICE_LIST(sprt_parent, sprt_list, sprt_bus_type);

	/*!< Take out the devices on the bus in turn */
	while ((sprt_device = FWK_NEXT_DEVICE(sprt_parent, sprt_list)))
	{
		/*!< All devices that match this drive are separated */
		if (sprt_device->sprt_driver != sprt_driver)
			continue;

		/*!< Do the preparation before separation */
		fwk_device_driver_remove(sprt_device);

		/*!< separation */
		sprt_device->sprt_driver = mrt_nullptr;
		sprt_driver->matches--;
	}

	return NR_isWell;
}

/*!
 * @brief   Find driver on bus
 * @param   driver, platform-bus
 * @retval  errno
 * @note    Detects whether the drive to be registered is hooked up on the bus
 */
static ksint32_t fwk_driver_find(struct fwk_driver *sprt_driver, struct fwk_bus_type *sprt_bus_type)
{
	struct fwk_driver *sprt_drv;

	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);
	DECLARE_LIST_HEAD_PTR(ptr_left);
	DECLARE_LIST_HEAD_PTR(ptr_right);

	sprt_list 	= &sprt_driver->sgrt_list;
	sprt_parent	= FWK_GET_BUS_DRIVER(sprt_bus_type);

	/*!< Two-way search */
	foreach_list_head(ptr_left, ptr_right, sprt_parent)
	{
		/*!< 1. Matching address */
		if ((ptr_left == sprt_list) || (ptr_right == sprt_list))
			return NR_isWell;

		/*!< 2. Match the device name */
		sprt_drv = mrt_container_of(ptr_left, struct fwk_driver, sgrt_list);
		if (!strcmp((char *)sprt_drv->name, (char *)sprt_driver->name))
			return NR_isWell;

		if (ptr_left != ptr_right)
		{
			sprt_drv = mrt_container_of(ptr_right, struct fwk_driver, sgrt_list);
			if (!strcmp((char *)sprt_drv->name, (char *)sprt_driver->name))
				return NR_isWell;
		}
		/*!< The search is complete, and there is no same driver */
		else
			break;
	}

	return -NR_isAnyErr;
}

/*!
 * @brief   Add driver to platform-bus
 * @param   driver, platform-bus
 * @retval  errno
 * @note    none
 */
static ksint32_t fwk_driver_to_bus(struct fwk_driver *sprt_driver, struct fwk_bus_type *sprt_bus_type)
{
	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);

	sprt_list 	= &sprt_driver->sgrt_list;
	sprt_parent	= FWK_GET_BUS_DRIVER(sprt_bus_type);

	/*!< Insert from the tail of the bus */
	list_head_add_tail(sprt_parent, sprt_list);

	/*!< Execute the device driver matching logic */
	return fwk_driver_attach(sprt_driver, sprt_bus_type, fwk_device_driver_match);
}

/*!
 * @brief   Detach driver on bus
 * @param   driver, platform-bus
 * @retval  errno
 * @note    none
 */
static ksint32_t fwk_bus_del_driver(struct fwk_driver *sprt_driver, struct fwk_bus_type *sprt_bus_type)
{
	ksint32_t retval;

	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);

	sprt_list 	= &sprt_driver->sgrt_list;
	sprt_parent	= FWK_GET_BUS_DRIVER(sprt_bus_type);

	/*!< Perform the device drive separation procedure */
	retval = fwk_driver_detach(sprt_driver, sprt_bus_type);

	/*!< Remove the driver from the bus */
	list_head_del_anyone(sprt_parent, sprt_list);

	return retval;
}

/*!
 * @brief   Register Driver
 * @param   driver
 * @retval  errno
 * @note    none
 */
static ksint32_t fwk_driver_register(struct fwk_driver *sprt_driver)
{
	struct fwk_bus_type *sprt_bus_type;
	ksint32_t retval;

	sprt_bus_type = sprt_driver->sprt_bus_type;

	/*!< The bus does not exist and cannot be registered */
	if (!sprt_bus_type)
		goto fail;

	/*!< The bus does not exist and cannot be registered */
	if (!sprt_bus_type->sprt_SysPriv)
		goto fail;

	/*!< Is the driver registered? No more duplicate registrations */
	retval = fwk_driver_find(sprt_driver, sprt_bus_type);
	if (!retval)
		goto fail;

	/*!< For the first registration, the number of device matches is initialized to 0 */
	sprt_driver->matches = 0;

	/*!< Add the driver to the bus, and if the match is successful, return Well */
	return fwk_driver_to_bus(sprt_driver, sprt_bus_type);

fail:
	return -NR_isAnyErr;
}

/*!
 * @brief   Unregister driver
 * @param   driver
 * @retval  errno
 * @note    none
 */
static ksint32_t fwk_driver_unregister(struct fwk_driver *sprt_driver)
{
	struct fwk_bus_type *sprt_bus_type;
	ksint32_t retval;

	sprt_bus_type = sprt_driver->sprt_bus_type;

	/*!< The bus does not exist and cannot be logged out */
	if (!sprt_bus_type)
		goto fail;

	/*!< The bus does not exist and cannot be logged out */
	if (!sprt_bus_type->sprt_SysPriv)
		goto fail;

	/*!< The driver is not registered? Go straight back */
	retval = fwk_driver_find(sprt_driver, sprt_bus_type);
	if (retval < 0)
		goto fail;

	/*!< Remove the driver from the bus */
	return fwk_bus_del_driver(sprt_driver, sprt_bus_type);

fail:
	return -NR_isAnyErr;
}
