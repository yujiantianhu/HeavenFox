/*
 * Platform Bus Interface of Hardware Abstraction Layer
 *
 * File Name:   fwk_platdev.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.23
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_platform.h>
#include <platform/fwk_platdev.h>

/*!< The globals */
static struct fwk_device_type sgrt_fwk_platform_dev_type =
{
	.name = "platform-type",
};

static DECLARE_LIST_HEAD(sgrt_fwk_devices);

/*!< The functions */
static kint32_t fwk_device_attach(struct fwk_device *sprt_dev, struct fwk_bus_type *sprt_bus_type);
static kint32_t fwk_device_detach(struct fwk_device *sprt_dev);
static kint32_t fwk_device_find_in_bus(struct fwk_device *sprt_dev, struct fwk_bus_type *sprt_bus_type);
static kint32_t fwk_device_to_bus(struct fwk_device *sprt_dev, struct fwk_bus_type *sprt_bus_type);
static kint32_t fwk_bus_del_device(struct fwk_device *sprt_dev, struct fwk_bus_type *sprt_bus_type);

/*!< API function */
/*!
 * @brief   Register Platform Device
 * @param   sprt_platdev
 * @retval  Register Result
 * @note    Should be used at initcall
 */
kint32_t fwk_register_platdevice(struct fwk_platdev *sprt_platdev)
{
	sprt_platdev->sgrt_dev.sprt_bus = &sgrt_fwk_platform_bus_type;
	sprt_platdev->sgrt_dev.sprt_type = &sgrt_fwk_platform_dev_type;

	return fwk_device_add(&sprt_platdev->sgrt_dev);
}

/*!
 * @brief   Unregister Platform Device
 * @param   sprt_platdev
 * @retval  Unregister Result
 * @note    Should be used at exitcall
 */
kint32_t fwk_unregister_platdevice(struct fwk_platdev *sprt_platdev)
{
	return fwk_device_del(&sprt_platdev->sgrt_dev);
}

/*!< -------------------------------------------------------------------------- */
/*!<							Device Local Interface							*/
/*!< -------------------------------------------------------------------------- */
static kint32_t fwk_device_find(struct fwk_device *sprt_dev)
{
	struct fwk_device *sprt_leaf;

	foreach_list_next_entry(sprt_leaf, &sgrt_fwk_devices, sgrt_leaf)
	{
		if (sprt_leaf == sprt_dev)
			return NR_IS_NORMAL;

		if (!strcmp(mrt_dev_get_name(sprt_leaf), mrt_dev_get_name(sprt_dev)))
			return NR_IS_NORMAL;
	}

	return -NR_IS_NOTFOUND;
}

/*!
 * @brief   Device Match Interface
 * @param   device, platform-bus
 * @retval  errno
 * @note    Device actively matches driver
 */
static kint32_t fwk_device_attach(struct fwk_device *sprt_dev, struct fwk_bus_type *sprt_bus_type)
{
	struct fwk_driver *sprt_driver;
	kint32_t retval;

	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);

	/*!< sprt_driver is not null, maybe this device has been matched to driver */
	if (sprt_dev->sprt_driver)
		return NR_IS_NORMAL;

	/*!< check if "match" function defines in platform-bus */
	if (!sprt_bus_type->match)
		return -NR_IS_NSUPPORT;

	FWK_INIT_BUS_DRIVER_LIST(sprt_parent, sprt_list, sprt_bus_type);

	/*!< get driver from bus one after another */
	while ((sprt_driver = FWK_NEXT_DRIVER(sprt_parent, sprt_list)))
	{
		/*!< try to attach this driver */
		retval = fwk_device_driver_match(sprt_dev, sprt_bus_type, sprt_driver);
		if (!retval || (retval == -NR_IS_PERMIT))
			return NR_IS_NORMAL;
	}

	return -NR_IS_PERMIT;
}

/*!
 * @brief   Device dissolve the relationship with driver 
 * @param   device
 * @retval  errno
 * @note    Device actively leaves driver
 */
static kint32_t fwk_device_detach(struct fwk_device *sprt_dev)
{
	struct fwk_driver *sprt_driver;

	/*!< sprt_driver is null, no driver has been mathced */
	if (!sprt_dev->sprt_driver)
		return NR_IS_NORMAL;

	sprt_driver	= sprt_dev->sprt_driver;

	/*!< prepare to separate */
	fwk_device_driver_remove(sprt_dev);

	/*!< do separattion */
	sprt_dev->sprt_driver = mrt_nullptr;
	sprt_driver->matches--;

	return NR_IS_NORMAL;
}

/*!
 * @brief   Find device on the bus
 * @param   device, platform-bus
 * @retval  errno
 * @note    Detect if the device that will be registered is already attached to bus: yes(return well), no(return err)
 */
static kint32_t fwk_device_find_in_bus(struct fwk_device *sprt_dev, struct fwk_bus_type *sprt_bus_type)
{
	struct fwk_device  *sprt_devTemp;
	struct fwk_platdev *sprt_platDevAny;
	struct fwk_platdev *sprt_platDevDst;

	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);
	DECLARE_LIST_HEAD_PTR(ptr_left);
	DECLARE_LIST_HEAD_PTR(ptr_right);

	sprt_list 		= &sprt_dev->sgrt_list;
	sprt_parent		= FWK_GET_BUS_DEVICE(sprt_bus_type);
	sprt_platDevDst	= mrt_container_of(sprt_dev, struct fwk_platdev, sgrt_dev);

	/*!< two-way search */
	foreach_list_head(ptr_left, ptr_right, sprt_parent)
	{
		/*!< 1. matching with address(list pointer) */
		if ((ptr_left == sprt_list) || (ptr_right == sprt_list))
			return NR_IS_NORMAL;

		/*!< 2. matching with device name: left */
		sprt_devTemp 	= mrt_container_of(ptr_left, struct fwk_device,  sgrt_list);
		sprt_platDevAny	= mrt_container_of(sprt_devTemp, struct fwk_platdev, sgrt_dev);
		if (!strcmp((char *)sprt_platDevAny->name, (char *)sprt_platDevDst->name))
			return NR_IS_NORMAL;

		/*!< 3. matching with device name: right */
		if (ptr_left != ptr_right)
		{
			sprt_devTemp 	= mrt_container_of(ptr_right, struct fwk_device, sgrt_list);
			sprt_platDevAny	= mrt_container_of(sprt_devTemp, struct fwk_platdev, sgrt_dev);
			if (!strcmp((char *)sprt_platDevAny->name, (char *)sprt_platDevDst->name))
				return NR_IS_NORMAL;
		}
		/*!< search finished, no device can be found */
		else
			break;
	}

	return -NR_IS_ERROR;
}

/*!
 * @brief   Add new device to bus
 * @param   device, platform-bus
 * @retval  errno
 * @note    After adding finished, excute device-driver matching
 */
static kint32_t fwk_device_to_bus(struct fwk_device *sprt_dev, struct fwk_bus_type *sprt_bus_type)
{
	kint32_t retval;

	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);

	sprt_list 	= &sprt_dev->sgrt_list;
	sprt_parent	= FWK_GET_BUS_DEVICE(sprt_bus_type);

	/*!< add to list tail */
	list_head_add_tail(sprt_parent, sprt_list);

	/*!< do device-driver matching */
	retval = fwk_device_attach(sprt_dev, sprt_bus_type);
	return (!retval || (retval == -NR_IS_PERMIT)) ? NR_IS_NORMAL : retval;
}

/*!
 * @brief   delete device on bus
 * @param   device, platform-bus
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_bus_del_device(struct fwk_device *sprt_dev, struct fwk_bus_type *sprt_bus_type)
{
	DECLARE_LIST_HEAD_PTR(sprt_list);
	DECLARE_LIST_HEAD_PTR(sprt_parent);

	sprt_list 	= &sprt_dev->sgrt_list;
	sprt_parent	= FWK_GET_BUS_DEVICE(sprt_bus_type);

	/*!< do detaching before deleting */
	fwk_device_detach(sprt_dev);

	/*!< delete device */
	list_head_del_safe(sprt_parent, sprt_list);

	return NR_IS_NORMAL;
}

/*!
 * @brief   Register device
 * @param   device
 * @retval  errno
 * @note    none
 */
kint32_t fwk_device_add(struct fwk_device *sprt_dev)
{
	struct fwk_bus_type *sprt_bus_type;
	kint32_t retval;

	sprt_bus_type = sprt_dev->sprt_bus;

	if (!fwk_device_find(sprt_dev))
		return -NR_IS_EXISTED;

	/*!< platform-bus is not exsisted */
	if (sprt_bus_type)
	{
		/*!< device list is not exsisted */
		if (!sprt_bus_type->sprt_SysPriv)
			goto fail;

		/*!< device is already registered */
		retval = fwk_device_find_in_bus(sprt_dev, sprt_bus_type);
		if (!(retval < 0))
			goto fail;

		/*!< fisrt register */
		sprt_dev->sprt_driver = mrt_nullptr;

		/*!< add device to device list (platform-bus) */
		retval = fwk_device_to_bus(sprt_dev, sprt_bus_type);
		if (retval)
			return retval;
	}

	list_head_add_tail(&sgrt_fwk_devices, &sprt_dev->sgrt_leaf);
	return NR_IS_NORMAL;

fail:
	return -NR_IS_ERROR;
}

/*!
 * @brief   Unregister device
 * @param   device
 * @retval  errno
 * @note    none
 */
kint32_t fwk_device_del(struct fwk_device *sprt_dev)
{
	struct fwk_bus_type *sprt_bus_type;
	kint32_t retval;

	sprt_bus_type = sprt_dev->sprt_bus;

	/*!< platform-bus is not exsisted */
	if (sprt_bus_type)
	{
		/*!< device list is not exsisted */
		if (!sprt_bus_type->sprt_SysPriv)
			goto fail;

		/*!< device is not registered */
		retval = fwk_device_find_in_bus(sprt_dev, sprt_bus_type);
		if (retval < 0)
			goto fail;

		/*!< delete device on the bus */
		retval = fwk_bus_del_device(sprt_dev, sprt_bus_type);
		if (retval)
			return retval;
	}

	list_head_del(&sprt_dev->sgrt_leaf);
	return NR_IS_NORMAL;

fail:
	return -NR_IS_ERROR;
}

/* end of file */
