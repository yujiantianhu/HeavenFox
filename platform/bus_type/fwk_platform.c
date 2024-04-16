/*
 * Platform Bus Interface of Hardware Abstraction Layer
 *
 * File Name:   fwk_platform.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.25
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_platform.h>
#include <platform/fwk_platdev.h>
#include <platform/fwk_platdrv.h>

/*!<
 * One device can only be matched with one driver
 * One driver can be used for multiple devices
 */

/*!< API function */
/*!
 * @brief   Match device and driver
 * @param   device, driver
 * @retval  errno
 * @note    none
 */
static ksint32_t fwk_platform_match(struct fwk_device *sprt_dev, struct fwk_driver *sprt_driver)
{
	struct fwk_platdev *sprt_platdev;
	struct fwk_platdrv *sprt_platdrv;
	struct fwk_id_table *sprt_idTable;
	struct fwk_device_node *sprt_np;
	kuint32_t idTable_cnt;

	sprt_platdev = mrt_container_of(sprt_dev, struct fwk_platdev, sgrt_dev);
	sprt_platdrv = mrt_container_of(sprt_driver, struct fwk_platdrv, sgrt_driver);

	/*!<
	 * Devices cannot have the same name as each other; Drivers cannot have the same name from one drive to another
	 * Therefore, if you match the device name, it is not possible to match one driver with multiple devices
	 * This function should be done by idTable
	 */

	/*!< Matching priority 1: override, where override is set, all other matching methods will be invalidated */
	if (sprt_platdev->driver_override)
		return ((!strcmp(sprt_platdev->driver_override, sprt_driver->name)) ? NR_isWell : (-NR_isNotSuccess));

	/*!< Match Priority 2: Device Tree */
	sprt_np = fwk_of_node_try_matches(sprt_dev->sprt_node, sprt_driver->sprt_of_match_table, mrt_nullptr);
	if (isValid(sprt_np) && (sprt_dev->sprt_node == sprt_np))
		return NR_isWell;

	/*!< Match Priority 3: idTable */
	for (idTable_cnt = 0; idTable_cnt < sprt_platdrv->num_idTable; idTable_cnt++)
	{
		sprt_idTable = sprt_platdrv->sprt_idTable + idTable_cnt;

		if (!strcmp((char *)sprt_platdev->name, (char *)sprt_idTable->name))
			return NR_isWell;
	}

	/*!< Match Priority 4: Name */
	/*!< 
	 * Each device is not allowed to have the same name;
	 * So you can exit immediately after finding it, and there will be no more devices with the same name in the future */
	if (!strcmp((char *)sprt_platdev->name, (char *)sprt_driver->name))
		return NR_isWell;

	return -NR_isNotFound;
}

/*!
 * @brief   Probe after matching successfully
 * @param   device
 * @retval  errno
 * @note    none
 */
static ksint32_t fwk_platform_probe(struct fwk_device *sprt_dev)
{
	struct fwk_driver  *sprt_driver;
	struct fwk_platdev *sprt_platdev;
	struct fwk_platdrv *sprt_platdrv;

	sprt_driver	= sprt_dev->sprt_driver;
	if (!sprt_driver)
		return -NR_isArgFault;

	sprt_platdev = mrt_container_of(sprt_dev, struct fwk_platdev, sgrt_dev);
	sprt_platdrv = mrt_container_of(sprt_driver, struct fwk_platdrv, sgrt_driver);

	if ((!sprt_platdrv->probe) || (0 > sprt_platdrv->probe(sprt_platdev)))
		return -NR_isPermit;

	return NR_isWell;
}

/*!
 * @brief   Remove on bus
 * @param   device
 * @retval  errno
 * @note    none
 */
static ksint32_t fwk_platform_remove(struct fwk_device *sprt_dev)
{
	struct fwk_driver  *sprt_driver;
	struct fwk_platdev *sprt_platdev;
	struct fwk_platdrv *sprt_platdrv;

	sprt_driver	= sprt_dev->sprt_driver;
	if (!sprt_driver)
		return -NR_isArgFault;

	sprt_platdev = mrt_container_of(sprt_dev, struct fwk_platdev, sgrt_dev);
	sprt_platdrv = mrt_container_of(sprt_driver, struct fwk_platdrv, sgrt_driver);

	if ((!sprt_platdrv->remove) || (0 > sprt_platdrv->remove(sprt_platdev)))
		return -NR_isPermit;

	return NR_isWell;
}

struct fwk_SysPrivate sgrt_fwk_platform_SysPriv =
{
	.sprt_bus_type		= &sgrt_fwk_platform_bus_type,

	.sgrt_list_devices	= LIST_HEAD_INIT(&sgrt_fwk_platform_SysPriv.sgrt_list_devices),
	.sgrt_list_drivers	= LIST_HEAD_INIT(&sgrt_fwk_platform_SysPriv.sgrt_list_drivers),
};

struct fwk_bus_type sgrt_fwk_platform_bus_type =
{
	.name	= "platform",

	.match	= fwk_platform_match,
	.probe	= fwk_platform_probe,
	.remove	= fwk_platform_remove,

	.sprt_SysPriv = &sgrt_fwk_platform_SysPriv,
};

/*!
 * @brief   Probe operation
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t fwk_device_driver_probe(struct fwk_device *sprt_dev)
{
	struct fwk_bus_type *sprt_bus_type = sprt_dev->sprt_bus_type;

	return sprt_bus_type->probe ? sprt_bus_type->probe(sprt_dev) : (-NR_isNotSupport);
}

/*!
 * @brief   Remove operation
 * @param   device
 * @retval  errno
 * @note    none
 */
ksint32_t fwk_device_driver_remove(struct fwk_device *sprt_dev)
{
	struct fwk_bus_type *sprt_bus_type = sprt_dev->sprt_bus_type;

	return sprt_bus_type->remove ? sprt_bus_type->remove(sprt_dev) : (-NR_isNotSupport);
}

/*!
 * @brief   Match device and driver
 * @param   device, platform-bus, driver
 * @retval  errno
 * @note    none
 */
ksint32_t fwk_device_driver_match(struct fwk_device *sprt_dev, struct fwk_bus_type *sprt_bus_type, void *ptr_data)
{
	struct fwk_driver *sprt_driver;
	ksint32_t retval;

	sprt_driver	= (struct fwk_driver *)ptr_data;
	retval = sprt_bus_type->match(sprt_dev, sprt_driver);
	if (retval < 0)
		return retval;

	/*!< 
	 * Save the device to indicate that the driver has been matched;
	 * You only need to operate the device to obtain the driver's information 
	 */
	sprt_dev->sprt_driver = sprt_driver;

	return fwk_device_driver_probe(sprt_dev);
}

