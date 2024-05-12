/*
 * Hardware Abstraction Layer Device-Tree Property
 *
 * File Name:   fwk_of_prop.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.14
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_OF_PROP_H_
#define __FWK_OF_PROP_H_

/*!< The includes */
#include <platform/fwk_basic.h>

/*!< The defines */
struct fwk_of_phandle_args;

#define FDT_TO_ARCH_ENDIAN32(x)					mrt_be32_to_cpu(x)
#define FDT_TO_ARCH_ENDIAN16(x)					mrt_be16_to_cpu(x)
#define FDT_TO_ARCH_PTR32(x)					mrt_be32_to_cpup(x)

typedef struct fwk_of_property
{
	/*!< Property name: such as name, compatible, #size-cells */
	kstring_t *name;
	/*!< Property lenth */
	ksint32_t length;
	/*!< Property value, such as compatible = "myBoard", ===> value = "myBoard", lenth = strlen(value) */
	void *value;
	/*!< Next property */
	struct fwk_of_property *sprt_next;

} srt_fwk_of_property_t;

typedef struct fwk_device_node
{
	kstring_t *name;
	kstring_t *type;
	ksint32_t phandle;
	kstring_t *full_name;

	struct fwk_of_property *properties;	/*!< Node property */
	struct fwk_of_property *deadprops;	/*!< removed properties */

	struct fwk_device_node *parent;    	/*!< Parent node */
	struct fwk_device_node *child;     	/*!< Child node */
	struct fwk_device_node *sibling;   	/*!< Sibling (brother) node*/

	struct fwk_device_node *sprt_next;	/*!< Used in the device_node build phase, after which it loses its meaning, the value is NULL  */
	struct fwk_device_node *allnext;	/*!< All nodes form a linked list */

	kuint32_t flags;
	void *data;

} srt_fwk_device_node_t;

/*!< The functions */
TARGET_EXT struct fwk_of_property *fwk_of_find_property(struct fwk_device_node *sprt_node,
													const kstring_t *ptr_name, kusize_t *ptr_lenth);
TARGET_EXT void *fwk_of_get_property(struct fwk_device_node *sprt_node, const kstring_t *ptr_name, kusize_t *ptr_lenth);
TARGET_EXT ksint32_t fwk_of_property_read_u8_array_index(struct fwk_device_node *sprt_node, const kstring_t *ptr_name, 
													kuint8_t *ptr_value, kuint32_t index, kusize_t size);
TARGET_EXT ksint32_t fwk_of_property_read_u16_array_index(struct fwk_device_node *sprt_node, const kstring_t *ptr_name, 
													kuint16_t *ptr_value, kuint32_t index, kusize_t size);
TARGET_EXT ksint32_t fwk_of_property_read_u32_array_index(struct fwk_device_node *sprt_node, const kstring_t *ptr_name, 
													kuint32_t *ptr_value, kuint32_t index, kusize_t size);
TARGET_EXT ksint32_t fwk_of_property_read_u32_index(struct fwk_device_node *sprt_node,
													const kstring_t *ptr_name, kuint32_t index, kuint32_t *ptr_value);
TARGET_EXT ksint32_t fwk_of_property_read_u8_array(struct fwk_device_node *sprt_node,
													const kstring_t *ptr_name, kuint8_t *ptr_value, kusize_t size);
TARGET_EXT ksint32_t fwk_of_property_read_u16_array(struct fwk_device_node *sprt_node,
													const kstring_t *ptr_name, kuint16_t *ptr_value, kusize_t size);
TARGET_EXT ksint32_t fwk_of_property_read_u32_array(struct fwk_device_node *sprt_node,
													const kstring_t *ptr_name, kuint32_t *ptr_value, kusize_t size);
TARGET_EXT ksint32_t fwk_of_property_read_u8(struct fwk_device_node *sprt_node, const kstring_t *ptr_name, kuint8_t *ptr_value);
TARGET_EXT ksint32_t fwk_of_property_read_u16(struct fwk_device_node *sprt_node, const kstring_t *ptr_name, kuint16_t *ptr_value);
TARGET_EXT ksint32_t fwk_of_property_read_u32(struct fwk_device_node *sprt_node, const kstring_t *ptr_name, kuint32_t *ptr_value);
TARGET_EXT ksint32_t fwk_of_property_read_string(struct fwk_device_node *sprt_node, const kstring_t *ptr_name, kstring_t **ptr_string);
TARGET_EXT ksint32_t fwk_of_property_read_string_index(struct fwk_device_node *sprt_node,
													const kstring_t *ptr_name, kuint32_t index, kstring_t **ptr_string);
TARGET_EXT kuint32_t fwk_of_n_size_cells(struct fwk_device_node *sprt_node);
TARGET_EXT kuint32_t fwk_of_n_addr_cells(struct fwk_device_node *sprt_node);
TARGET_EXT kbool_t   fwk_of_device_is_compatible(struct fwk_device_node *sprt_node, const kstring_t *ptr_compat);
TARGET_EXT ksint32_t fwk_of_parse_phandle_with_args(struct fwk_device_node *sprt_node, const kstring_t *list_name,
										const kstring_t *cells_name, kuint32_t cell_count, ksint32_t index, struct fwk_of_phandle_args *sprt_args);
TARGET_EXT ksint32_t fwk_of_property_match_string(struct fwk_device_node *sprt_node, const kstring_t *list_name, const kstring_t *match_name);

#endif /*!< __FWK_OF_PROP_H_ */
