/*
 * Platform Bus Interface of Hardware Abstraction Layer
 *
 * File Name:   hal_platdrv.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.23
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/hal_platform.h>
#include <platform/hal_platdrv.h>

/*!< The functions */
static ksint32_t hal_driver_attach(struct hal_driver *sprt_driver, struct hal_bus_type *sprt_bus_type,
								ksint32_t (*pFunc_Match) (struct hal_device *, struct hal_bus_type *, void *));
static ksint32_t hal_driver_detach(struct hal_driver *sprt_driver, struct hal_bus_type *sprt_bus_type);
static ksint32_t hal_driver_find(struct hal_driver *sprt_driver, struct hal_bus_type *sprt_bus_type);
static ksint32_t hal_driver_to_bus(struct hal_driver *sprt_driver, struct hal_bus_type *sprt_bus_type);
static ksint32_t hal_bus_del_driver(struct hal_driver *sprt_driver, struct hal_bus_type *sprt_bus_type);
static ksint32_t hal_driver_register(struct hal_driver *sprt_driver);
static ksint32_t hal_driver_unregister(struct hal_driver *sprt_driver);

/*!< API function */
/*!
 * @brief   Register platform driver
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t hal_register_platdriver(struct hal_platdrv *sprt_platdrv)
{
	sprt_platdrv->sgrt_driver.sprt_bus_type	= &sgrt_hal_platform_bus_type;

	return hal_driver_register(&sprt_platdrv->sgrt_driver);
}

/*!
 * @brief   Unegister platform driver
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t hal_unregister_platdriver(struct hal_platdrv *sprt_platdrv)
{
	return hal_driver_unregister(&sprt_platdrv->sgrt_driver);
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
static ksint32_t hal_driver_attach(struct hal_driver *sprt_driver, struct hal_bus_type *sprt_bus_type,
								ksint32_t (*pFunc_Match) (struct hal_device *, struct hal_bus_type *, void *))
{
	struct hal_device *sprt_device;
	ksint32_t matches;
	ksint32_t retval = 0;

	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);

	/*!< First check if the match function is defined */
	if (!mrt_isValid(sprt_bus_type->match))
	{
		return mrt_retval(Ert_isAnyErr);
	}

	matches	= 0;
	HAL_INIT_BUS_DEVICE_LIST(sprt_parent, sprt_list, sprt_bus_type);

	/*!< Take out the devices on the bus in turn */
	while (mrt_isValid(sprt_device = HAL_NEXT_DEVICE(sprt_parent, sprt_list)))
	{
		/*!< The device is matched with drivers */
		if (mrt_isValid(sprt_device->sprt_driver))
		{
			continue;
		}

		/*!< Match one by one */
		/*!< One driver supports matching multiple devices, and does not exit until the linked list is fully traversed */
		/*!< However, try not to mount multiple devices with the same driver at the same time to avoid misoperation */
		retval = (*pFunc_Match)(sprt_device, sprt_bus_type, sprt_driver);
		if (!mrt_isErr(retval))
		{
			/*!< Record the number of matching devices */
			matches++;
		}
	}

	sprt_driver->matches += matches;

	return ((retval == (-Ert_isNotFound)) || matches) ? matches : -Ert_isAnyErr;
}

/*!
 * @brief   Driver seperation
 * @param   driver, platform-bus
 * @retval  errno
 * @note    none
 */
static ksint32_t hal_driver_detach(struct hal_driver *sprt_driver, struct hal_bus_type *sprt_bus_type)
{
	struct hal_device *sprt_device;

	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);

	HAL_INIT_BUS_DEVICE_LIST(sprt_parent, sprt_list, sprt_bus_type);

	/*!< Take out the devices on the bus in turn */
	while (mrt_isValid(sprt_device = HAL_NEXT_DEVICE(sprt_parent, sprt_list)))
	{
		/*!< All devices that match this drive are separated */
		if (sprt_device->sprt_driver != sprt_driver)
		{
			continue;
		}

		/*!< Do the preparation before separation */
		hal_device_driver_remove(sprt_device);

		/*!< separation */
		sprt_device->sprt_driver = mrt_nullptr;
		sprt_driver->matches--;
	}

	return mrt_retval(Ert_isAnyErr);
}

/*!
 * @brief   Find driver on bus
 * @param   driver, platform-bus
 * @retval  errno
 * @note    Detects whether the drive to be registered is hooked up on the bus
 */
static ksint32_t hal_driver_find(struct hal_driver *sprt_driver, struct hal_bus_type *sprt_bus_type)
{
	struct hal_driver *sprt_drv;

	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);
	DECLARE_LIST_HEAD_PTR(ptr_left);
	DECLARE_LIST_HEAD_PTR(ptr_right);

	sprt_list 	= &sprt_driver->sgrt_list;
	sprt_parent	= HAL_GET_BUS_DRIVER(sprt_bus_type);

	/*!< Two-way search */
	foreach_list_head(ptr_left, ptr_right, sprt_parent)
	{
		/*!< 1. Matching address */
		if ((ptr_left == sprt_list) || (ptr_right == sprt_list))
		{
			return mrt_retval(Ert_isWell);
		}

		/*!< 2. Match the device name */
		sprt_drv = mrt_container_of(ptr_left, struct hal_driver, sgrt_list);
		if (!strcmp((char *)sprt_drv->name, (char *)sprt_driver->name))
		{
			return mrt_retval(Ert_isWell);
		}

		if (ptr_left != ptr_right)
		{
			sprt_drv = mrt_container_of(ptr_right, struct hal_driver, sgrt_list);
			if (!strcmp((char *)sprt_drv->name, (char *)sprt_driver->name))
			{
				return mrt_retval(Ert_isWell);
			}
		}
		/*!< The search is complete, and there is no same driver */
		else
		{
			break;
		}
	}

	return mrt_retval(Ert_isAnyErr);
}

/*!
 * @brief   Add driver to platform-bus
 * @param   driver, platform-bus
 * @retval  errno
 * @note    none
 */
static ksint32_t hal_driver_to_bus(struct hal_driver *sprt_driver, struct hal_bus_type *sprt_bus_type)
{
	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);

	sprt_list 	= &sprt_driver->sgrt_list;
	sprt_parent	= HAL_GET_BUS_DRIVER(sprt_bus_type);

	/*!< Insert from the tail of the bus */
	list_head_add_tail(sprt_parent, sprt_list);

	/*!< Execute the device driver matching logic */
	return hal_driver_attach(sprt_driver, sprt_bus_type, hal_device_driver_match);
}

/*!
 * @brief   Detach driver on bus
 * @param   driver, platform-bus
 * @retval  errno
 * @note    none
 */
static ksint32_t hal_bus_del_driver(struct hal_driver *sprt_driver, struct hal_bus_type *sprt_bus_type)
{
	ksint32_t retval;

	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);

	sprt_list 	= &sprt_driver->sgrt_list;
	sprt_parent	= HAL_GET_BUS_DRIVER(sprt_bus_type);

	/*!< Perform the device drive separation procedure */
	retval = hal_driver_detach(sprt_driver, sprt_bus_type);

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
static ksint32_t hal_driver_register(struct hal_driver *sprt_driver)
{
	struct hal_bus_type *sprt_bus_type;
	ksint32_t retval;

	sprt_bus_type = sprt_driver->sprt_bus_type;

	/*!< The bus does not exist and cannot be registered */
	if (!mrt_isValid(sprt_bus_type))
	{
		goto fail;
	}

	/*!< The bus does not exist and cannot be registered */
	if (!mrt_isValid(sprt_bus_type->sprt_SysPriv))
	{
		goto fail;
	}

	/*!< Is the driver registered? No more duplicate registrations */
	retval = hal_driver_find(sprt_driver, sprt_bus_type);
	if (!mrt_isErr(retval))
	{
		goto fail;
	}

	/*!< For the first registration, the number of device matches is initialized to 0 */
	sprt_driver->matches = 0;

	/*!< Add the driver to the bus, and if the match is successful, return Well */
	return hal_driver_to_bus(sprt_driver, sprt_bus_type);

fail:
	return mrt_retval(Ert_isAnyErr);
}

/*!
 * @brief   Unregister driver
 * @param   driver
 * @retval  errno
 * @note    none
 */
static ksint32_t hal_driver_unregister(struct hal_driver *sprt_driver)
{
	struct hal_bus_type *sprt_bus_type;
	ksint32_t retval;

	sprt_bus_type = sprt_driver->sprt_bus_type;

	/*!< The bus does not exist and cannot be logged out */
	if (!mrt_isValid(sprt_bus_type))
	{
		goto fail;
	}

	/*!< The bus does not exist and cannot be logged out */
	if (!mrt_isValid(sprt_bus_type->sprt_SysPriv))
	{
		goto fail;
	}

	/*!< The driver is not registered? Go straight back */
	retval = hal_driver_find(sprt_driver, sprt_bus_type);
	if (mrt_isErr(retval))
	{
		goto fail;
	}

	/*!< Remove the driver from the bus */
	return hal_bus_del_driver(sprt_driver, sprt_bus_type);

fail:
	return mrt_retval(Ert_isAnyErr);
}
