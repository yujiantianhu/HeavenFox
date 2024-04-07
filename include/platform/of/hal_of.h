/*
 * Hardware Abstraction Layer Device-Tree
 *
 * File Name:   hal_of.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.03
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_OF_H_
#define __HAL_OF_H_

/*!< The includes */
#include <platform/hal_basic.h>
#include <platform/of/hal_of_prop.h>

/*!< The defines */
#define mrt_hal_fdt_node_header()						(sprt_hal_of_allNodes)

/*!< Traverse all child nodes under the head (starting with child, get the sibling nodes of child) */
#define FOREACH_CHILD_OF_DT_NODE(head, list)	\
	foreach_list_odd(head->child, list, sibling)

/*!< The following node identities are all represented in processor architecture patterns (big-endian/little-endian) */
/*!< magic number */
#define FDT_MAGIC_VERIFY						(0xd00dfeed)
/*!< child node start */
#define FDT_NODE_START							(0x00000001)
/*!< child node end */
#define FDT_NODE_END							(0x00000002)
/*!< node property */
#define FDT_NODE_PROP							(0x00000003)
/*!< empty node */
#define FDT_NODE_NOP							(0x00000004)
/*!< start of all nodes: That is, the start of the block */
#define FDT_ALL_NODE_START						(FDT_NODE_START)
/*!< end of all nodes: That is, the end of the block */
#define FDT_ALL_NODE_END						(0x00000009)

enum __ERT_DT_NODE_FLGAS
{
	/*!< The node has been converted */
	Ert_OfNodePopulated	= mrt_bit(0),
	/*!< The child nodes have also been converted */
	Ert_OfNodePopulatedBus = mrt_bit(1),
};

/*!< This file structure object is derived from the Linux kernel definition */
struct hal_fdt_header
{
    kuint32_t magic;              				/*!< The magic number of the device tree, it is fixed at 0xd00dfeed */
    kuint32_t totalsize;          				/*!< The size of the entire device tree */
    kuint32_t off_dt_struct;      				/*!< Saves the offset of the block in the entire device tree */
    kuint32_t off_dt_strings;     				/*!< The offset of the saved string block in the device tree */
    kuint32_t off_mem_rsvmap;     				/*!< Reserved memory area, which reserves memory space that cannot be dynamically allocated by the kernel */
    kuint32_t version;            				/*!< Device tree version */
    kuint32_t last_comp_version;  				/*!< Backwards compatible version number */
    kuint32_t boot_cpuid_phys;    				/*!< Is the physical ID of the primary CPU used to boot in the multi-core processor */
    kuint32_t size_dt_strings;   				/*!< String block size */
    kuint32_t size_dt_struct;    				/*!< Block size */
};

/*!< The device tree matches the data structure */
struct hal_of_device_id
{
    kstring_t   *name;
    kstring_t   *type;
    kstring_t	*compatible;

    const void 	*data;
};

#define HAL_OF_MAX_PHANDLE_ARGS 				(16)
struct hal_of_phandle_args 
{
	struct hal_device_node *sprt_node;
	kuint32_t args_count;
	kuint32_t args[HAL_OF_MAX_PHANDLE_ARGS];
};

/*!< The gloabls */
TARGET_EXT struct hal_device_node *sprt_hal_of_allNodes;

/*!< The functions */
TARGET_EXT void setup_machine_fdt(void *ptr);
TARGET_EXT void destroy_machine_fdt(void);
TARGET_EXT ksint32_t of_platform_default_populate_init(void);

/*!< Device tree operation API */
TARGET_EXT struct hal_device_node *hal_of_find_node_by_path(const kstring_t *ptr_path);
TARGET_EXT struct hal_device_node *hal_of_find_node_by_name(struct hal_device_node *sprt_from, const kstring_t *ptr_name);
TARGET_EXT struct hal_device_node *hal_of_find_node_by_type(struct hal_device_node *sprt_from, const kstring_t *ptr_type);
TARGET_EXT struct hal_device_node *hal_of_find_node_by_phandle(struct hal_device_node *sprt_from, kuint32_t phandle);
TARGET_EXT struct hal_device_node *hal_of_find_compatible_node(struct hal_device_node *sprt_from,
											const kstring_t *ptr_type, const kstring_t *ptr_compat);
TARGET_EXT struct hal_device_node *hal_of_node_try_matches(struct hal_device_node *sprt_node,
							const struct hal_of_device_id *sprt_matches, struct hal_of_device_id **sprt_match);
TARGET_EXT struct hal_device_node *hal_of_find_matching_node_and_match(struct hal_device_node *sprt_from,
											const struct hal_of_device_id *sprt_matches, struct hal_of_device_id **sprt_match);
TARGET_EXT struct hal_device_node *hal_of_get_parent(struct hal_device_node *sprt_node);
TARGET_EXT struct hal_device_node *hal_of_get_next_child(struct hal_device_node *sprt_node, struct hal_device_node *ptr_prev);
TARGET_EXT kbool_t hal_of_device_is_avaliable(struct hal_device_node *sprt_node);
TARGET_EXT struct hal_of_device_id *hal_of_match_node(const struct hal_of_device_id *sprt_matches, struct hal_device_node *sprt_node);
TARGET_EXT struct hal_device_node *hal_of_irq_parent(struct hal_device_node *sprt_node);
TARGET_EXT kuint32_t hal_of_n_irq_cells(struct hal_device_node *sprt_node);
TARGET_EXT kuint32_t hal_of_irq_count(struct hal_device_node *sprt_node);

/*!< API function */
/*!
 * @brief   hal_early_init_dt_verify
 * @param   none
 * @retval  none
 * @note    In order to improve efficiency, this function does not check whether the input parameter is null, 
 *          so you need to complete the null check before calling this function
 */
static inline kbool_t hal_early_init_dt_verify(void *ptr_dt)
{
	struct hal_fdt_header *sprt_fdt_header = (struct hal_fdt_header *)ptr_dt;
	return ((FDT_MAGIC_VERIFY == FDT_TO_ARCH_ENDIAN32(sprt_fdt_header->magic)) ? Ert_true : Ert_false);
}

/*!
 * @brief   hal_of_node_set_flag
 * @param   none
 * @retval  none
 * @note    none
 */
static inline void hal_of_node_set_flag(struct hal_device_node *sprt_node, kuint32_t flags)
{
	sprt_node->flags |= flags;
}

/*!
 * @brief   hal_of_node_check_flag
 * @param   none
 * @retval  none
 * @note    none
 */
static inline kbool_t hal_of_node_check_flag(struct hal_device_node *sprt_node, kuint32_t flags)
{
	return (((sprt_node->flags & flags) == flags) ? Ert_true : Ert_false);
}

/*!< get each node */
#define FOREACH_OF_DT_NODE(np, head)	\
	for (np = (mrt_isValid(head) ? head : mrt_hal_fdt_node_header()); mrt_isValid(np); np = np->allnext)

#endif /*!< __HAL_OF_H_ */
