/*
 * Hardware Abstraction Layer Device-Tree Property
 *
 * File Name:   hal_of_prop.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.14
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_OF_PROP_H_
#define __HAL_OF_PROP_H_

/*!< The includes */
#include <platform/hal_basic.h>

/*!< The defines */
#define FDT_TO_ARCH_ENDIAN32(x)					mrt_be32_to_cpu(x)
#define FDT_TO_ARCH_ENDIAN16(x)					mrt_be16_to_cpu(x)

struct hal_of_property
{
	/*!< Property name: such as name, compatible, #size-cells */
	kstring_t *name;
	/*!< Property lenth */
	ksint32_t length;
	/*!< Property value, such as compatible = "myBoard", ===> value = "myBoard", lenth = strlen(value) */
	void *value;
	/*!< Next property */
	struct hal_of_property *sprt_next;

};

struct hal_device_node
{
	kstring_t *name;
	kstring_t *type;
	ksint32_t phandle;
	kstring_t *full_name;

	struct hal_of_property *properties;	/*!< Node property */
	struct hal_of_property *deadprops;	/*!< removed properties */

	struct hal_device_node *parent;    	/*!< Parent node */
	struct hal_device_node *child;     	/*!< Child node */
	struct hal_device_node *sibling;   	/*!< Sibling (brother) node*/

	struct hal_device_node *sprt_next;	/*!< Used in the device_node build phase, after which it loses its meaning, the value is NULL  */
	struct hal_device_node *allnext;	/*!< All nodes form a linked list */

	kuint32_t flags;
	void *data;
};

/*!< The functions */
TARGET_EXT struct hal_of_property *hal_of_find_property(struct hal_device_node *sprt_node,
													const kstring_t *ptr_name, kusize_t *ptr_lenth);
TARGET_EXT void *hal_of_get_property(struct hal_device_node *sprt_node, const kstring_t *ptr_name, kusize_t *ptr_lenth);
TARGET_EXT ksint32_t hal_of_property_read_u8_array_index(struct hal_device_node *sprt_node, const kstring_t *ptr_name, 
													kuint8_t *ptr_value, kuint32_t index, kusize_t size);
TARGET_EXT ksint32_t hal_of_property_read_u16_array_index(struct hal_device_node *sprt_node, const kstring_t *ptr_name, 
													kuint16_t *ptr_value, kuint32_t index, kusize_t size);
TARGET_EXT ksint32_t hal_of_property_read_u32_array_index(struct hal_device_node *sprt_node, const kstring_t *ptr_name, 
													kuint32_t *ptr_value, kuint32_t index, kusize_t size);
TARGET_EXT ksint32_t hal_of_property_read_u32_index(struct hal_device_node *sprt_node,
													const kstring_t *ptr_name, kuint32_t index, kuint32_t *ptr_value);
TARGET_EXT ksint32_t hal_of_property_read_u8_array(struct hal_device_node *sprt_node,
													const kstring_t *ptr_name, kuint8_t *ptr_value, kusize_t size);
TARGET_EXT ksint32_t hal_of_property_read_u16_array(struct hal_device_node *sprt_node,
													const kstring_t *ptr_name, kuint16_t *ptr_value, kusize_t size);
TARGET_EXT ksint32_t hal_of_property_read_u32_array(struct hal_device_node *sprt_node,
													const kstring_t *ptr_name, kuint32_t *ptr_value, kusize_t size);
TARGET_EXT ksint32_t hal_of_property_read_u8(struct hal_device_node *sprt_node, const kstring_t *ptr_name, kuint8_t *ptr_value);
TARGET_EXT ksint32_t hal_of_property_read_u16(struct hal_device_node *sprt_node, const kstring_t *ptr_name, kuint16_t *ptr_value);
TARGET_EXT ksint32_t hal_of_property_read_u32(struct hal_device_node *sprt_node, const kstring_t *ptr_name, kuint32_t *ptr_value);
TARGET_EXT ksint32_t hal_of_property_read_string(struct hal_device_node *sprt_node, const kstring_t *ptr_name, kstring_t **ptr_string);
TARGET_EXT ksint32_t hal_of_property_read_string_index(struct hal_device_node *sprt_node,
													const kstring_t *ptr_name, kuint32_t index, kstring_t **ptr_string);
TARGET_EXT kuint32_t hal_of_n_size_cells(struct hal_device_node *sprt_node);
TARGET_EXT kuint32_t hal_of_n_addr_cells(struct hal_device_node *sprt_node);
TARGET_EXT kbool_t   hal_of_device_is_compatible(struct hal_device_node *sprt_node, const kstring_t *ptr_compat);

#endif /*!< __HAL_OF_PROP_H_ */
