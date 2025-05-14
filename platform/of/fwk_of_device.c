/*
 * Hardware Abstraction Layer Device-Tree / Platform Device Convertion
 *
 * File Name:   fwk_of_device.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.14
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/of/fwk_of_device.h>
#include <platform/irq/fwk_irq_domain.h>

/*!< -------------------------------------------------------------------------- */
/*!< The defines */

/*!< -------------------------------------------------------------------------- */
/*!< The globals */

/*!< -------------------------------------------------------------------------- */
/*!< The functions */
static kint32_t fwk_of_platform_default_populate(struct fwk_device_node *sptr_node);
static kint32_t fwk_of_platform_populate(struct fwk_device_node *sptr_node, const struct fwk_of_device_id *sptr_matches);
static kint32_t fwk_of_platform_bus_create(struct fwk_device_node *sptr_node, const struct fwk_of_device_id *sptr_matches, kbool_t strict);
static struct fwk_platdev *fwk_of_platform_device_create_pdata(struct fwk_device_node *sptr_node);

/*!< -------------------------------------------------------------------------- */
/*!< API function */
/*!
 * @brief   fwk_of_platform_populate_init
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t __plat_init fwk_of_platform_populate_init(void)
{
    return fwk_of_platform_default_populate(mr_nullptr);
}

/*!< Specify the following nodes to be converted to platform_device */
const struct fwk_of_device_id sgtc_fwk_of_default_bus_match_table[] =
{
    { .compatible = "simple-bus", },
    { .compatible = "simple-mfd", },
    { .compatible = "isa", },

    /*!< Empty terminated list */
    {} 
};

/*!
 * @brief   fwk_of_platform_default_populate
 * @param   none
 * @retval  none
 * @note    The default matching table mode
 */
static kint32_t fwk_of_platform_default_populate(struct fwk_device_node *sptr_node)
{
    return fwk_of_platform_populate(sptr_node, sgtc_fwk_of_default_bus_match_table);
}

/*!
 * @brief   fwk_of_platform_populate
 * @param   none
 * @retval  none
 * @note    Take out each node and convert it to platform_device: You can start the conversion from a specific node
 */
static kint32_t fwk_of_platform_populate(struct fwk_device_node *sptr_node, const struct fwk_of_device_id *sptr_matches)
{
    struct fwk_device_node *sptr_head = mr_nullptr;
    struct fwk_device_node *sptr_list = mr_nullptr;
    kint32_t retval;

    sptr_head = isValid(sptr_node) ? sptr_node : fwk_of_find_node_by_path("/");
    retval = -ER_ERROR;

    if (!isValid(sptr_head))
        return -ER_NOMEM;

    /*!< Take out each node */
    FOREACH_CHILD_OF_DT_NODE(sptr_head, sptr_list)
    {
        retval = fwk_of_platform_bus_create(sptr_list, sptr_matches, true);
        if (!retval)
        {
            /*!< do nothing */
        }
    }

    return retval;
}

/*!
 * @brief   fwk_of_platform_bus_create
 * @param   none
 * @retval  none
 * @note    Node properties match
 */
static kint32_t fwk_of_platform_bus_create(struct fwk_device_node *sptr_node, const struct fwk_of_device_id *sptr_matches, kbool_t strict)
{
    struct fwk_device_node *sptr_head;
    struct fwk_device_node *sptr_list;
    struct fwk_platdev *sptr_platdev;
    kint32_t retval;

    /*!< Don't have "compatible" property? The current node does not convert */
    if (strict && (!isValid(fwk_of_get_property(sptr_node, "compatible", mr_nullptr))))
        return -ER_FAULT;

    /*!< 
     * Check: If this node is a grandson of the root node;
     * Judgment: Converted? Nodes that have already been converted cannot be converted repeatedly 
     */
    /*!< This flag is only used for recursion */
    if (fwk_of_node_check_flag(sptr_node, NR_OfNodePopulatedBus))
        return -ER_FAULT;

    /*!< Convert this node */
    sptr_platdev = fwk_of_platform_device_create_pdata(sptr_node);
    if (!isValid(sptr_platdev))
        return -ER_NOMEM;

    /*!< 
     * Check: Whether you need to convert the child nodes under this node
     * If the value of the compatible attribute is the same as that of one of the matches, 
     * all the children under this node must also be converted to platform device
     */
    if (!fwk_of_match_node(sptr_matches, sptr_node))
        return ER_NORMAL;

    /*!< All child nodes are converted to platform_device */
    sptr_head = sptr_node;

    /*!< Recursion: Handles child nodes */
    FOREACH_CHILD_OF_DT_NODE(sptr_head, sptr_list)
    {
        retval = fwk_of_platform_bus_create(sptr_list, sptr_matches, strict);
        if (!retval)
        {
            /*!< do nothing */
        }
    }

    /*!< This will only be executed if the child nodes under this node also need to be converted */
    fwk_of_node_set_flag(sptr_node, NR_OfNodePopulatedBus);

    return ER_NORMAL;
}

/*!
 * @brief   fwk_of_platform_device_create_pdata
 * @param   none
 * @retval  none
 * @note    Convert the specified node to platform_device and hook it to the bus
 */
static struct fwk_platdev *fwk_of_platform_device_create_pdata(struct fwk_device_node *sptr_node)
{
    struct fwk_platdev *sptr_platdev;
    struct fwk_resources sgtc_resources;
    struct fwk_resources *sptr_resources = mr_nullptr;
    kint32_t retval;
    kuint32_t i, num_res = 0, num_irq = 0;

    if (!isValid(sptr_node))
        return mr_nullptr;

    /*!< Check the status attribute of this node, and do not convert the attribute if the attribute value is disabled */
    /*!< 
     * Check: If this node is a child of the root node;
     * Judgment: Converted? Nodes that have already been converted cannot be converted repeatedly 
     */
    if (!fwk_of_device_is_avaliable(sptr_node) || fwk_of_node_check_flag(sptr_node, NR_OfNodePopulated))
        return mr_nullptr;

    sptr_platdev = (struct fwk_platdev *)kzalloc(sizeof(struct fwk_platdev), GFP_KERNEL);
    if (!isValid(sptr_platdev))
        return mr_nullptr;

    /*!< Determine the size of sptr_resources to be allocated */
    while (!fwk_of_address_to_resource(sptr_node, num_res, &sgtc_resources))
        num_res++;

    num_irq = fwk_of_irq_count(sptr_node);

    if ((num_res + num_irq) > 0)
    {
        sptr_resources = (struct fwk_resources *)kzalloc((num_res + num_irq) * sizeof(struct fwk_resources), GFP_KERNEL);
        sptr_platdev->num_resources	= num_res + num_irq;
        sptr_platdev->sptr_resources = sptr_resources;

        /*!< Fill sptr_resources */
        for (i = 0; i < num_res; i++)
            fwk_of_address_to_resource(sptr_node, i, (sptr_resources + i));

        /*!< error can be permitted */
        fwk_of_irq_to_resource_table(sptr_node, sptr_resources + num_res, num_irq);
    }

    /*!< Register to the device bus */
    retval = fwk_of_register_platdevice(sptr_node, sptr_platdev);
    if (retval)
    {
        /*!< Hook failed */
        if (isValid(sptr_resources))
            kfree(sptr_resources);

        kfree(sptr_platdev);
        return mr_nullptr;
    }

    /*!< Stored as private data */
    sptr_node->data	= sptr_platdev;
    /*!< Set the conversion flag */
    fwk_of_node_set_flag(sptr_node, NR_OfNodePopulated);

    return sptr_platdev;
}

/*!
 * @brief   fwk_of_register_platdevice
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_register_platdevice(struct fwk_device_node *sptr_node, struct fwk_platdev *sptr_platdev)
{
    if (!isValid(sptr_platdev))
        return -ER_FAULT;

    if (fwk_device_initial(&sptr_platdev->sgtc_dev))
        return -ER_FAILD;

    /*!< Fill platform_device */
    sptr_platdev->name = sptr_node->full_name;
    sptr_platdev->id = fwk_of_get_alias_id(sptr_node);

    sptr_platdev->sgtc_dev.init_name = sptr_node->full_name;
    sptr_platdev->sgtc_dev.sptr_node = sptr_node;
    sptr_platdev->sgtc_dev.release = mr_nullptr;
    sptr_platdev->sgtc_dev.sptr_parent = mr_nullptr;

    /*!< Bus hook-up */
    sptr_platdev->sgtc_dev.sptr_bus = &sgtc_fwk_platform_bus_type;

    return fwk_device_add(&sptr_platdev->sgtc_dev);
}

/*!
 * @brief   fwk_of_unregister_platdevice
 * @param   none
 * @retval  none
 * @note    none
 */
void fwk_of_unregister_platdevice(struct fwk_device_node *sptr_node)
{
    struct fwk_platdev *sptr_platdev = (struct fwk_platdev *)sptr_node->data;

    if (!isValid(sptr_platdev))
        return;

    /*!< Log out the device from the bus */
    fwk_device_del(&sptr_platdev->sgtc_dev);

    if (isValid(sptr_platdev->sptr_resources))
        kfree(sptr_platdev->sptr_resources);

    kfree(sptr_platdev);
    sptr_node->data	= mr_nullptr;
}

/*!< -------------------------------------------------------------------------- */
/*!
 * @brief   fwk_of_address_to_resource
 * @param   none
 * @retval  none
 * @note    reg property data conversion
 */
kint32_t fwk_of_address_to_resource(struct fwk_device_node *sptr_node, kuint32_t index, struct fwk_resources *sptr_res)
{
    kuint32_t cells_of_addr, cells_of_size;
    kuint32_t address, size;
    kchar_t *regName;
    kint32_t retval;

    /*!< Get value of "#address-cells" */
    cells_of_addr = fwk_of_n_addr_cells(sptr_node);
    /*!< Get value of "#size-cells" */
    cells_of_size = fwk_of_n_size_cells(sptr_node);

    if ((1 < cells_of_addr) || (1 < cells_of_size))
        return -ER_INVALID;

    /*!< Search for the reg address */
    retval = fwk_of_property_read_u32_index(sptr_node, "reg", index << 1, &address);
    if (retval)
        return -ER_FAULT;

    /*!< Search for the reg length */
    retval = fwk_of_property_read_u32_index(sptr_node, "reg", (index << 1) + 1, &size);
    if (retval)
        return -ER_FAULT;

    /*!< reg-names are not necessarily defined by the device tree */
    retval = fwk_of_property_read_string_index(sptr_node, "reg-names", index, &regName);
    if (retval)
        regName	= sptr_node->full_name;

    sptr_res->name	 = regName;
    sptr_res->start	 = address;
    sptr_res->end	 = address + size - 1;
    sptr_res->type	|= NR_DEVICE_RESOURCE_MEM;

    return ER_NORMAL;
}

/*!
 * @brief   fwk_of_irq_to_resource_table
 * @param   none
 * @retval  none
 * @note    interrupt property data conversion
 */
kint32_t fwk_of_irq_to_resource_table(struct fwk_device_node *sptr_node, struct fwk_resources *sptr_res, kuint32_t nr_irqs)
{
    kint32_t virq;
    kuint32_t index;
    kchar_t *regName;

    for (index = 0; index < nr_irqs; index++, sptr_res++)
    {
        virq = fwk_irq_of_parse_and_map(sptr_node, index);
        if (virq < 0)
            return -ER_FAILD;

        /*!< reg-names are not necessarily defined by the device tree */
        if (fwk_of_property_read_string_index(sptr_node, "interrupt-names", index, &regName))
            regName	= sptr_node->full_name;

        sptr_res->name	 = regName;
        sptr_res->start	 = virq;
        sptr_res->end	 = virq;
        sptr_res->type	|= NR_DEVICE_RESOURCE_IRQ;
    }

    return ER_NORMAL;
}

/*!
 * @brief   get resources from device_node
 * @param   none
 * @retval  none
 * @note    none
 */
struct fwk_resources *fwk_platform_get_resources(struct fwk_platdev *sptr_pdev, kuint32_t index, kuint32_t type)
{
    struct fwk_resources *sptr_res, *sptr_max;
    kuint32_t cnt = 0;

    if (!isValid(sptr_pdev) || (index > sptr_pdev->num_resources))
        return mr_nullptr;

    sptr_max = sptr_pdev->sptr_resources + sptr_pdev->num_resources;

    for (sptr_res = sptr_pdev->sptr_resources; sptr_res < sptr_max; sptr_res++)
    {
        cnt += (!!(type & sptr_res->type)) ? 1 : 0;

        /*!< found */
        if (cnt > index)
            return sptr_res;
    }

    return mr_nullptr;
}

/*!
 * @brief   get reg address resources from device_node
 * @param   none
 * @retval  none
 * @note    none
 */
kuaddr_t fwk_platform_get_address(struct fwk_platdev *sptr_pdev, kuint32_t index)
{
    struct fwk_resources *sptr_res;

    sptr_res = fwk_platform_get_resources(sptr_pdev, index, NR_DEVICE_RESOURCE_MEM);
    return sptr_res ? sptr_res->start : 0;
}

/*!
 * @brief   get reg address resources from device_node
 * @param   none
 * @retval  none
 * @note    none
 */
kusize_t fwk_platform_get_address_size(struct fwk_platdev *sptr_pdev, kuint32_t index)
{
    struct fwk_resources *sptr_res;

    sptr_res = fwk_platform_get_resources(sptr_pdev, index, NR_DEVICE_RESOURCE_MEM);
    return sptr_res ? (sptr_res->end - sptr_res->start + 1) : 0;
}

/*!
 * @brief   get irq resources from device_node
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_platform_get_irq(struct fwk_platdev *sptr_pdev, kuint32_t index)
{
    struct fwk_resources *sptr_res;

    sptr_res = fwk_platform_get_resources(sptr_pdev, index, NR_DEVICE_RESOURCE_IRQ);
    return sptr_res ? sptr_res->start : -1;
}

/*!
 * @brief   get address from device_node
 * @param   none
 * @retval  none
 * @note    if platform_device doesn't built, you should use it to get address
 */
void *fwk_of_iomap(struct fwk_device_node *sptr_node, kuint32_t index)
{
    struct fwk_resources sgtc_res;

    if (fwk_of_address_to_resource(sptr_node, index, &sgtc_res))
        return mr_nullptr;

    return (void *)fwk_address_map(&sgtc_res);
}

/*!< end of file */
