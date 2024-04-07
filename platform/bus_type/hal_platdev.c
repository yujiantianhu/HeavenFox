/*
 * Platform Bus Interface of Hardware Abstraction Layer
 *
 * File Name:   hal_platdev.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.23
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/hal_platform.h>
#include <platform/hal_platdev.h>

/*!< The functions */
static ksint32_t hal_device_attach(struct hal_device *sprt_device, struct hal_bus_type *sprt_bus_type);
static ksint32_t hal_device_detach(struct hal_device *sprt_device);
static ksint32_t hal_device_find(struct hal_device *sprt_device, struct hal_bus_type *sprt_bus_type);
static ksint32_t hal_device_to_bus(struct hal_device *sprt_device, struct hal_bus_type *sprt_bus_type);
static ksint32_t hal_bus_del_device(struct hal_device *sprt_device, struct hal_bus_type *sprt_bus_type);

/*!< API function */
/*!
 * @brief   Register Platform Device
 * @param   sprt_platdev
 * @retval  Register Result
 * @note    Should be used at initcall
 */
ksint32_t hal_register_platdevice(struct hal_platdev *sprt_platdev)
{
	sprt_platdev->sgrt_device.sprt_bus_type	= &sgrt_hal_platform_bus_type;

	return hal_device_register(&sprt_platdev->sgrt_device);
}

/*!
 * @brief   Unregister Platform Device
 * @param   sprt_platdev
 * @retval  Unregister Result
 * @note    Should be used at exitcall
 */
ksint32_t hal_unregister_platdevice(struct hal_platdev *sprt_platdev)
{
	return hal_device_unregister(&sprt_platdev->sgrt_device);
}

/*!< -------------------------------------------------------------------------- */
/*!<							Device Local Interface							*/
/*!< -------------------------------------------------------------------------- */
/*!
 * @brief   Device Match Interface
 * @param   device, platform-bus
 * @retval  errno
 * @note    Device actively matches driver
 */
static ksint32_t hal_device_attach(struct hal_device *sprt_device, struct hal_bus_type *sprt_bus_type)
{
	struct hal_driver *sprt_driver;
	ksint32_t retval;

	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);

	/*!< sprt_driver is not null, maybe this device has been matched to driver */
	if (mrt_isValid(sprt_device->sprt_driver))
	{
		return mrt_retval(Ert_isWell);
	}

	/*!< check if "match" function defines in platform-bus */
	if (!mrt_isValid(sprt_bus_type->match))
	{
		return mrt_retval(Ert_isNotSupport);
	}

	HAL_INIT_BUS_DRIVER_LIST(sprt_parent, sprt_list, sprt_bus_type);

	/*!< get driver from bus one after another */
	while (mrt_isValid(sprt_driver = HAL_NEXT_DRIVER(sprt_parent, sprt_list)))
	{
		/*!< try to attach this driver */
		retval = hal_device_driver_match(sprt_device, sprt_bus_type, sprt_driver);
		if (!mrt_isErr(retval))
		{
			return mrt_retval(Ert_isWell);
		}
	}

	return mrt_retval(Ert_isPermit);
}

/*!
 * @brief   Device dissolve the relationship with driver 
 * @param   device
 * @retval  errno
 * @note    Device actively leaves driver
 */
static ksint32_t hal_device_detach(struct hal_device *sprt_device)
{
	struct hal_driver *sprt_driver;

	/*!< sprt_driver is null, no driver has been mathced */
	if (!mrt_isValid(sprt_device->sprt_driver))
	{
		return mrt_retval(Ert_isWell);
	}

	sprt_driver	= sprt_device->sprt_driver;

	/*!< prepare to separate */
	hal_device_driver_remove(sprt_device);

	/*!< do separattion */
	sprt_device->sprt_driver = mrt_nullptr;
	sprt_driver->matches--;

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   Find device on the bus
 * @param   device, platform-bus
 * @retval  errno
 * @note    Detect if the device that will be registered is already attached to bus: yes(return well), no(return err)
 */
static ksint32_t hal_device_find(struct hal_device *sprt_device, struct hal_bus_type *sprt_bus_type)
{
	struct hal_device  *sprt_dev;
	struct hal_platdev *sprt_platDevAny;
	struct hal_platdev *sprt_platDevDst;

	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);
	DECLARE_LIST_HEAD_PTR(ptr_left);
	DECLARE_LIST_HEAD_PTR(ptr_right);

	sprt_list 		= &sprt_device->sgrt_list;
	sprt_parent		= HAL_GET_BUS_DEVICE(sprt_bus_type);
	sprt_platDevDst	= mrt_container_of(sprt_device, struct hal_platdev, sgrt_device);

	/*!< two-way search */
	foreach_list_head(ptr_left, ptr_right, sprt_parent)
	{
		/*!< 1. matching with address(list pointer) */
		if ((ptr_left == sprt_list) || (ptr_right == sprt_list))
		{
			return mrt_retval(Ert_isWell);
		}

		/*!< 2. matching with device name: left */
		sprt_dev 		= mrt_container_of(ptr_left, struct hal_device,  sgrt_list);
		sprt_platDevAny	= mrt_container_of(sprt_dev, struct hal_platdev, sgrt_device);
		if (!strcmp((char *)sprt_platDevAny->name, (char *)sprt_platDevDst->name))
		{
			return mrt_retval(Ert_isWell);
		}

		/*!< 3. matching with device name: right */
		if (ptr_left != ptr_right)
		{
			sprt_dev 		= mrt_container_of(ptr_right, struct hal_device, sgrt_list);
			sprt_platDevAny	= mrt_container_of(sprt_dev, struct hal_platdev, sgrt_device);
			if (!strcmp((char *)sprt_platDevAny->name, (char *)sprt_platDevDst->name))
			{
				return mrt_retval(Ert_isWell);
			}
		}
		/*!< search finished, no device can be found */
		else
		{
			break;
		}
	}

	return mrt_retval(Ert_isAnyErr);
}

/*!
 * @brief   Add new device to bus
 * @param   device, platform-bus
 * @retval  errno
 * @note    After adding finished, excute device-driver matching
 */
static ksint32_t hal_device_to_bus(struct hal_device *sprt_device, struct hal_bus_type *sprt_bus_type)
{
	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);

	sprt_list 	= &sprt_device->sgrt_list;
	sprt_parent	= HAL_GET_BUS_DEVICE(sprt_bus_type);

	/*!< add to list tail */
	list_head_add_tail(sprt_parent, sprt_list);

	/*!< do device-driver matching */
	return hal_device_attach(sprt_device, sprt_bus_type);
}

/*!
 * @brief   delete device on bus
 * @param   device, platform-bus
 * @retval  errno
 * @note    none
 */
static ksint32_t hal_bus_del_device(struct hal_device *sprt_device, struct hal_bus_type *sprt_bus_type)
{
	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);

	sprt_list 	= &sprt_device->sgrt_list;
	sprt_parent	= HAL_GET_BUS_DEVICE(sprt_bus_type);

	/*!< do detaching before deleting */
	hal_device_detach(sprt_device);

	/*!< delete device */
	list_head_del_anyone(sprt_parent, sprt_list);

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   Register device
 * @param   device
 * @retval  errno
 * @note    none
 */
ksint32_t hal_device_register(struct hal_device *sprt_device)
{
	struct hal_bus_type *sprt_bus_type;
	ksint32_t retval;

	sprt_bus_type = sprt_device->sprt_bus_type;

	/*!< platform-bus is not exsisted */
	if (!mrt_isValid(sprt_bus_type))
	{
		goto fail;
	}

	/*!< device list is not exsisted */
	if (!mrt_isValid(sprt_bus_type->sprt_SysPriv))
	{
		goto fail;
	}

	/*!< device is already registered */
	retval = hal_device_find(sprt_device, sprt_bus_type);
	if (!mrt_isErr(retval))
	{
		goto fail;
	}

	/*!< fisrt register */
	sprt_device->sprt_driver = mrt_nullptr;

	/*!< add device to device list (platform-bus) */
	return hal_device_to_bus(sprt_device, sprt_bus_type);

fail:
	return mrt_retval(Ert_isAnyErr);
}

/*!
 * @brief   Unregister device
 * @param   device
 * @retval  errno
 * @note    none
 */
ksint32_t hal_device_unregister(struct hal_device *sprt_device)
{
	struct hal_bus_type *sprt_bus_type;
	ksint32_t retval;

	sprt_bus_type = sprt_device->sprt_bus_type;

	/*!< platform-bus is not exsisted */
	if (!mrt_isValid(sprt_bus_type))
	{
		goto fail;
	}

	/*!< device list is not exsisted */
	if (!mrt_isValid(sprt_bus_type->sprt_SysPriv))
	{
		goto fail;
	}

	/*!< device is not registered */
	retval = hal_device_find(sprt_device, sprt_bus_type);
	if (mrt_isErr(retval))
	{
		goto fail;
	}

	/*!< delete device on the bus */
	return hal_bus_del_device(sprt_device, sprt_bus_type);

fail:
	return mrt_retval(Ert_isAnyErr);
}

/* end of file */
