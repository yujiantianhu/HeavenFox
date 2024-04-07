/*
 * Hardware Abstraction Layer Device-Tree / Platform Device Convertion
 *
 * File Name:   hal_of_device.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.14
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/of/hal_of_device.h>
#include <platform/irq/hal_irq_domain.h>

/*!< -------------------------------------------------------------------------- */
/*!< The defines */

/*!< -------------------------------------------------------------------------- */
/*!< The globals */

/*!< -------------------------------------------------------------------------- */
/*!< The functions */
static ksint32_t hal_of_platform_default_populate(struct hal_device_node *sprt_node);
static ksint32_t hal_of_platform_populate(struct hal_device_node *sprt_node, const struct hal_of_device_id *sprt_matches);
static ksint32_t hal_of_platform_bus_create(struct hal_device_node *sprt_node, const struct hal_of_device_id *sprt_matches, kbool_t strict);
static struct hal_platdev *hal_of_platform_device_create_pdata(struct hal_device_node *sprt_node);

/*!< -------------------------------------------------------------------------- */
/*!< API function */
/*!
 * @brief   of_platform_default_populate_init
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t __plat_init of_platform_default_populate_init(void)
{
	return hal_of_platform_default_populate(mrt_nullptr);
}
IMPORT_PLATFORM_INIT(of_platform_default_populate_init);

/*!< Specify the following nodes to be converted to platform_device */
const struct hal_of_device_id sgrt_hal_of_default_bus_match_table[] =
{
	{ .compatible = "simple-bus", },
	{ .compatible = "simple-mfd", },
	{ .compatible = "isa", },

	/*!< Empty terminated list */
	{} 
};

/*!
 * @brief   hal_of_platform_default_populate
 * @param   none
 * @retval  none
 * @note    The default matching table mode
 */
static ksint32_t hal_of_platform_default_populate(struct hal_device_node *sprt_node)
{
	return hal_of_platform_populate(sprt_node, sgrt_hal_of_default_bus_match_table);
}

/*!
 * @brief   hal_of_platform_populate
 * @param   none
 * @retval  none
 * @note    Take out each node and convert it to platform_device: You can start the conversion from a specific node
 */
static ksint32_t hal_of_platform_populate(struct hal_device_node *sprt_node, const struct hal_of_device_id *sprt_matches)
{
	struct hal_device_node *sprt_head = mrt_nullptr;
	struct hal_device_node *sprt_list = mrt_nullptr;
	ksint32_t retval;

	sprt_head = mrt_isValid(sprt_node) ? sprt_node : hal_of_find_node_by_path("/");
	retval = mrt_retval(Ert_isAnyErr);

	if (!mrt_isValid(sprt_head))
	{
		return mrt_retval(Ert_isMemErr);
	}

	/*!< Take out each node */
	FOREACH_CHILD_OF_DT_NODE(sprt_head, sprt_list)
	{
		retval = hal_of_platform_bus_create(sprt_list, sprt_matches, Ert_true);
		if (!mrt_isErr(retval))
		{
			/*!< do nothing */
		}
	}

	return retval;
}

/*!
 * @brief   hal_of_platform_bus_create
 * @param   none
 * @retval  none
 * @note    Node properties match
 */
static ksint32_t hal_of_platform_bus_create(struct hal_device_node *sprt_node, const struct hal_of_device_id *sprt_matches, kbool_t strict)
{
	struct hal_device_node *sprt_head;
	struct hal_device_node *sprt_list;
	struct hal_platdev *sprt_platdev;
	ksint32_t retval;

	/*!< Don't have "compatible" property? The current node does not convert */
	if (strict && (!mrt_isValid(hal_of_get_property(sprt_node, "compatible", mrt_nullptr))))
	{
		return mrt_retval(Ert_isArgFault);
	}

	/*!< 
	 * Check: If this node is a grandson of the root node;
	 * Judgment: Converted? Nodes that have already been converted cannot be converted repeatedly 
	 */
	/*!< This flag is only used for recursion */
	if (hal_of_node_check_flag(sprt_node, Ert_OfNodePopulatedBus))
	{
		return mrt_retval(Ert_isArgFault);
	}

	/*!< Convert this node */
	sprt_platdev = hal_of_platform_device_create_pdata(sprt_node);
	if (!mrt_isValid(sprt_platdev))
	{
		return mrt_retval(Ert_isMemErr);
	}

	/*!< 
	 * Check: Whether you need to convert the child nodes under this node
	 * If the value of the compatible attribute is the same as that of one of the matches, 
	 * all the children under this node must also be converted to platform device
	 */
	if (!hal_of_match_node(sprt_matches, sprt_node))
	{
		return mrt_retval(Ert_isWell);
	}

	/*!< All child nodes are converted to platform_device */
	sprt_head = sprt_node;

	/*!< Recursion: Handles child nodes */
	FOREACH_CHILD_OF_DT_NODE(sprt_head, sprt_list)
	{
		retval = hal_of_platform_bus_create(sprt_list, sprt_matches, strict);
		if (!mrt_isErr(retval))
		{
			/*!< do nothing */
		}
	}

	/*!< This will only be executed if the child nodes under this node also need to be converted */
	hal_of_node_set_flag(sprt_node, Ert_OfNodePopulatedBus);

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   hal_of_platform_device_create_pdata
 * @param   none
 * @retval  none
 * @note    Convert the specified node to platform_device and hook it to the bus
 */
static struct hal_platdev *hal_of_platform_device_create_pdata(struct hal_device_node *sprt_node)
{
	struct hal_platdev *sprt_platdev;
	struct hal_resources sgrt_resources;
	struct hal_resources *sprt_resources = mrt_nullptr;
	ksint32_t retval;
	kuint32_t i, num_res = 0, num_irq = 0;

	if (!mrt_isValid(sprt_node))
	{
		return mrt_nullptr;
	}

	/*!< Check the status attribute of this node, and do not convert the attribute if the attribute value is disabled */
	/*!< 
	 * Check: If this node is a child of the root node;
	 * Judgment: Converted? Nodes that have already been converted cannot be converted repeatedly 
	 */
	if (!hal_of_device_is_avaliable(sprt_node) || hal_of_node_check_flag(sprt_node, Ert_OfNodePopulated))
	{
		return mrt_nullptr;
	}

	sprt_platdev = (struct hal_platdev *)kzalloc(sizeof(struct hal_platdev), GFP_KERNEL);
	if (!mrt_isValid(sprt_platdev))
	{
		return mrt_nullptr;
	}

	/*!< Determine the size of sprt_resources to be allocated */
	while (!mrt_isErr(hal_of_address_to_resource(sprt_node, num_res, &sgrt_resources)))
	{
		num_res++;
	}

	num_irq = hal_of_irq_count(sprt_node);

	if ((num_res + num_irq) > 0)
	{
		sprt_resources = (struct hal_resources *)kzalloc((num_res + num_irq) * sizeof(struct hal_resources), GFP_KERNEL);
		sprt_platdev->num_resources	= num_res + num_irq;
		sprt_platdev->sprt_resources = sprt_resources;

		/*!< Fill sprt_resources */
		for (i = 0; i < num_res; i++)
		{
			hal_of_address_to_resource(sprt_node, i, (sprt_resources + i));
		}

		hal_of_irq_to_resource_table(sprt_node, sprt_resources + num_res, num_irq);
	}

	/*!< Register to the device bus */
	retval = hal_of_register_platdevice(sprt_node, sprt_platdev);
	if (mrt_isErr(retval))
	{
		/*!< Hook failed */
		if (mrt_isValid(sprt_resources))
		{
			kfree(sprt_resources);
		}

		kfree(sprt_platdev);

		return mrt_nullptr;
	}

	/*!< Stored as private data */
	sprt_node->data	= sprt_platdev;
	/*!< Set the conversion flag */
	hal_of_node_set_flag(sprt_node, Ert_OfNodePopulated);

	return sprt_platdev;
}

/*!
 * @brief   hal_of_register_platdevice
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t hal_of_register_platdevice(struct hal_device_node *sprt_node, struct hal_platdev *sprt_platdev)
{
	if (!mrt_isValid(sprt_platdev))
	{
		return mrt_retval(Ert_isArgFault);
	}

	/*!< Fill platform_device */
	sprt_platdev->name = sprt_node->full_name;
	sprt_platdev->id = -1;
	sprt_platdev->sgrt_device.sprt_node = sprt_node;
	sprt_platdev->sgrt_device.release = mrt_nullptr;
	sprt_platdev->sgrt_device.sprt_parent = mrt_nullptr;

	/*!< Bus hook-up */
	sprt_platdev->sgrt_device.sprt_bus_type	= &sgrt_hal_platform_bus_type;

	return hal_device_register(&sprt_platdev->sgrt_device);
}

/*!
 * @brief   hal_of_unregister_platdevice
 * @param   none
 * @retval  none
 * @note    none
 */
void hal_of_unregister_platdevice(struct hal_device_node *sprt_node)
{
	struct hal_platdev *sprt_platdev = (struct hal_platdev *)sprt_node->data;

	if (!mrt_isValid(sprt_platdev))
	{
		return;
	}

	/*!< Log out the device from the bus */
	hal_device_unregister(&sprt_platdev->sgrt_device);

	if (mrt_isValid(sprt_platdev->sprt_resources))
	{
		kfree(sprt_platdev->sprt_resources);
	}

	kfree(sprt_platdev);
	sprt_node->data	= mrt_nullptr;
}

/*!< -------------------------------------------------------------------------- */
/*!
 * @brief   hal_of_address_to_resource
 * @param   none
 * @retval  none
 * @note    reg property data conversion
 */
ksint32_t hal_of_address_to_resource(struct hal_device_node *sprt_node, kuint32_t index, struct hal_resources *sprt_res)
{
	kuint32_t cells_of_addr, cells_of_size;
	kuint32_t address, size;
	kstring_t *regName;
	ksint32_t retval;

	/*!< Get value of "#address-cells" */
	cells_of_addr = hal_of_n_addr_cells(sprt_node);
	/*!< Get value of "#size-cells" */
	cells_of_size = hal_of_n_size_cells(sprt_node);

	if ((1 < cells_of_addr) || (1 < cells_of_size))
	{
		return mrt_retval(Ert_isUnvalid);
	}

	/*!< Search for the reg address */
	retval = hal_of_property_read_u32_index(sprt_node, "reg", index << 1, &address);
	if (mrt_isErr(retval))
	{
		return mrt_retval(Ert_isArgFault);
	}

	/*!< Search for the reg length */
	retval = hal_of_property_read_u32_index(sprt_node, "reg", (index << 1) + 1, &size);
	if (mrt_isErr(retval))
	{
		return mrt_retval(Ert_isArgFault);
	}

	/*!< reg-names are not necessarily defined by the device tree */
	retval = hal_of_property_read_string_index(sprt_node, "reg-names", index, &regName);
	if (mrt_isErr(retval))
	{
		regName	= sprt_node->full_name;
	}

	sprt_res->name	 = regName;
	sprt_res->start	 = address;
	sprt_res->end	 = address + size - 1;
	sprt_res->type	|= Ert_DEVICE_RESOURCE_MEM;

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   hal_of_irq_to_resource_table
 * @param   none
 * @retval  none
 * @note    interrupt property data conversion
 */
ksint32_t hal_of_irq_to_resource_table(struct hal_device_node *sprt_node, struct hal_resources *sprt_res, kuint32_t nr_irqs)
{
	ksint32_t virq;
	kuint32_t index;
	kstring_t *regName;

	for (index = 0; index < nr_irqs; index++, sprt_res++)
	{
		virq = hal_irq_of_parse_and_map(sprt_node, index);
		if (virq < 0)
		{
			return -Ert_isNotSuccess;
		}

		/*!< reg-names are not necessarily defined by the device tree */
		if (mrt_isErr(hal_of_property_read_string_index(sprt_node, "interrupt-names", index, &regName)))
		{
			regName	= sprt_node->full_name;
		}

		sprt_res->name	 = regName;
		sprt_res->start	 = virq;
		sprt_res->end	 = virq;
		sprt_res->type	|= Ert_DEVICE_RESOURCE_IRQ;
	}

	return mrt_retval(Ert_isWell);
}

struct hal_resources *hal_platform_get_resources(struct hal_platdev *sprt_pdev, kuint32_t index, kuint32_t type)
{
	struct hal_resources *sprt_res, *sprt_max;
	kuint32_t cnt = 0;

	if (!mrt_isValid(sprt_pdev) || (index > sprt_pdev->num_resources))
	{
		return mrt_nullptr;
	}

	sprt_max = sprt_pdev->sprt_resources + sprt_pdev->num_resources;

	for (sprt_res = sprt_pdev->sprt_resources; sprt_res < sprt_max; sprt_res++)
	{
		cnt += (!!(type & sprt_res->type)) ? 1 : 0;

		if (cnt > index)
		{
			return sprt_res;
		}
	}

	return mrt_nullptr;
}

kuaddr_t hal_platform_get_address(struct hal_platdev *sprt_pdev, kuint32_t index)
{
	struct hal_resources *sprt_res;

	sprt_res = hal_platform_get_resources(sprt_pdev, index, Ert_DEVICE_RESOURCE_MEM);
	return sprt_res ? sprt_res->start : 0;
}

ksint32_t hal_platform_get_irq(struct hal_platdev *sprt_pdev, kuint32_t index)
{
	struct hal_resources *sprt_res;

	sprt_res = hal_platform_get_resources(sprt_pdev, index, Ert_DEVICE_RESOURCE_IRQ);
	return sprt_res ? sprt_res->start : -1;
}

kuaddr_t hal_of_iomap(struct hal_device_node *sprt_node, kuint32_t index)
{
	struct hal_resources sgrt_res;

	if (mrt_isErr(hal_of_address_to_resource(sprt_node, index, &sgrt_res)))
	{
		return 0;
	}

	return hal_address_map(&sgrt_res);
}

/*!< end of file */
