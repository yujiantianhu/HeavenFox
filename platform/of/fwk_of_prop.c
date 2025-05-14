/*
 * Hardware Abstraction Layer Device-Tree Property
 *
 * File Name:   fwk_of_prop.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.14
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_prop.h>

/*!< API function */
/*!
 * @brief   Locate the property based on its name
 * @param   none
 * @retval  none
 * @note    none
 */
struct fwk_of_property *fwk_of_find_property(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, kusize_t *ptr_lenth)
{
	struct fwk_of_property *sptr_head = isValid(sptr_node) ? sptr_node->properties : mr_nullptr;
	struct fwk_of_property *sptr_list = mr_nullptr;
	kusize_t iLenth	= 0;

	foreach_list_odd_head(sptr_head, sptr_list)
	{
		if (!strcmp(ptr_name, sptr_list->name))
		{
			iLenth = sptr_list->length;
			break;
		}
	}

	if (ptr_lenth)
		*ptr_lenth = iLenth;

	return sptr_list;
}

/*!
 * @brief   fwk_of_get_property
 * @param   none
 * @retval  none
 * @note    none
 */
void *fwk_of_get_property(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, kusize_t *ptr_lenth)
{
	struct fwk_of_property *sptr_prop = fwk_of_find_property(sptr_node, ptr_name, ptr_lenth);

	return isValid(sptr_prop) ? sptr_prop->value : mr_nullptr;
}

/*!
 * @brief   Read integer data in ptr_name properties as a single byte
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u8_array_index(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, 
													kuint8_t *ptr_value, kuint32_t index, kusize_t size)
{
	struct fwk_of_property *sptr_prop = mr_nullptr;
	kuint32_t i, lenth;

	/*!< sptr_prop == mr_nullptr : The property is not available */
	sptr_prop = fwk_of_find_property(sptr_node, ptr_name, mr_nullptr);
	if (!isValid(sptr_prop))
		return -ER_FAULT;

	lenth = sptr_prop->length;
	if ((index >= lenth) || (!size))
		return -ER_FAULT;

	/*!< sptr_prop->length: the number of bytes occupied by sptr_prop->value */
	size = ((index + size) >= lenth) ? lenth : (index + size);

	for (i = index; i < size; i++, ptr_value++)
		*ptr_value = *((kuint8_t *)sptr_prop->value + i);

	return ER_NORMAL;
}

/*!
 * @brief   Read integer data in ptr_name properties as a single byte
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u16_array_index(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, 
													kuint16_t *ptr_value, kuint32_t index, kusize_t size)
{
	struct fwk_of_property *sptr_prop = mr_nullptr;
	kuint32_t i, lenth;

	/*!< sptr_prop == mr_nullptr : The property is not available */
	sptr_prop = fwk_of_find_property(sptr_node, ptr_name, mr_nullptr);
	if (!isValid(sptr_prop))
		return -ER_FAULT;

	lenth = sptr_prop->length >> 1;
	if ((index >= lenth) || (!size))
		return -ER_FAULT;

	/*!< sptr_prop->length: the number of bytes occupied by sptr_prop->value */
	size = ((index + size) >= lenth) ? lenth : (index + size);

	for (i = index; i < size; i++, ptr_value++)
		*ptr_value = FDT_TO_ARCH_ENDIAN16(*((kuint16_t *)sptr_prop->value + i));

	return ER_NORMAL;
}

/*!
 * @brief   Read integer data in ptr_name properties as a single byte
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u32_array_index(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, 
													kuint32_t *ptr_value, kuint32_t index, kusize_t size)
{
	struct fwk_of_property *sptr_prop = mr_nullptr;
	kuint32_t i, lenth;

	/*!< sptr_prop == mr_nullptr : The property is not available */
	sptr_prop = fwk_of_find_property(sptr_node, ptr_name, mr_nullptr);
	if (!isValid(sptr_prop))
		return -ER_FAULT;

	lenth = sptr_prop->length >> 2;
	if ((index >= lenth) || (!size))
		return -ER_FAULT;

	/*!< sptr_prop->length: the number of bytes occupied by sptr_prop->value */
	size = ((index + size) >= lenth) ? lenth : (index + size);

	for (i = index; i < size; i++, ptr_value++)
		*ptr_value = FDT_TO_ARCH_ENDIAN32(*((kuint32_t *)sptr_prop->value + i));

	return ER_NORMAL;
}

/*!
 * @brief   Obtain the 32-digit [index] value from the property name ptr_name
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u32_index(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, kuint32_t index, kuint32_t *ptr_value)
{
	return fwk_of_property_read_u32_array_index(sptr_node, ptr_name, ptr_value, index, 1);
}

/*!
 * @brief   Read integer data in ptr_name properties as a single byte
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u8_array(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, kuint8_t *ptr_value, kusize_t size)
{
	return fwk_of_property_read_u8_array_index(sptr_node, ptr_name, ptr_value, 0, size);
}

/*!
 * @brief   Read integer data in ptr_name properties in double-byte fashion
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u16_array(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, kuint16_t *ptr_value, kusize_t size)
{
	return fwk_of_property_read_u16_array_index(sptr_node, ptr_name, ptr_value, 0, size);
}

/*!
 * @brief   Read integer data in ptr_name properties in 4-byte fashion
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u32_array(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, kuint32_t *ptr_value, kusize_t size)
{
	return fwk_of_property_read_u32_array_index(sptr_node, ptr_name, ptr_value, 0, size);
}

/*!
 * @brief   Read integer data in ptr_name properties as a single byte
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u8(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, kuint8_t *ptr_value)
{
	return fwk_of_property_read_u8_array_index(sptr_node, ptr_name, ptr_value, 0, 1);
}

/*!
 * @brief   Read integer data in ptr_name properties in double-byte fashion
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u16(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, kuint16_t *ptr_value)
{
	return fwk_of_property_read_u16_array_index(sptr_node, ptr_name, ptr_value, 0, 1);
}

/*!
 * @brief   Reads integer data in ptr_name properties in 4-byte mode
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u32(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, kuint32_t *ptr_value)
{
	return fwk_of_property_read_u32_array_index(sptr_node, ptr_name, ptr_value, 0, 1);
}

/*!
 * @brief   Read integer data in ptr_name properties as strings
 * @param   ptr_string: empty pointer, not an array
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_string(struct fwk_device_node *sptr_node, const kchar_t *ptr_name, kchar_t **ptr_string)
{
	struct fwk_of_property *sptr_prop = mr_nullptr;

	/*!< sptr_prop == mr_nullptr : The property is not available */
	sptr_prop = fwk_of_find_property(sptr_node, ptr_name, mr_nullptr);
	if (!isValid(sptr_prop))
	{
		*ptr_string = mr_nullptr;
		return -ER_FAULT;
	}

	*ptr_string	= (kchar_t *)sptr_prop->value;

	return ER_NORMAL;
}

/*!
 * @brief   Get the index-string from the property name ptr_name
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_string_index(struct fwk_device_node *sptr_node,
											const kchar_t *ptr_name, kuint32_t index, kchar_t **ptr_string)
{
	struct fwk_of_property *sptr_prop = mr_nullptr;
	kchar_t *ptr_value = mr_nullptr;
	kusize_t iStrLenth = 0, iTotalLenth;
	kuint32_t i;

	/*!< sptr_prop == mr_nullptr : The property is not available */
	sptr_prop = fwk_of_find_property(sptr_node, ptr_name, mr_nullptr);
	if (!isValid(sptr_prop))
	{
		*ptr_string = mr_nullptr;
		return -ER_FAULT;
	}

	for (i = 0, iTotalLenth = 0; ((i <= index) && (iTotalLenth < sptr_prop->length)); i++)
	{
		iStrLenth = ptr_value ? (strlen(ptr_value) + 1) : 0;
		ptr_value = ptr_value ? (ptr_value + iStrLenth) : (kchar_t *)sptr_prop->value;
		iTotalLenth += iStrLenth;
	}

	/*!< sptr_prop->length: the number of bytes occupied by sptr_prop->value */
	if (iTotalLenth >= sptr_prop->length)
		return -ER_MORE;

	*ptr_string	= ptr_value;

	return ER_NORMAL;
}

/*!
 * @brief   Get the value of #size-cells
 * @param   none
 * @retval  none
 * @note    none
 */
kuint32_t fwk_of_n_size_cells(struct fwk_device_node *sptr_node)
{
	struct fwk_device_node *sptr_np;
	kuint32_t value;
	kint32_t retval;

	for (sptr_np = sptr_node; isValid(sptr_np); sptr_np = sptr_np->parent)
	{
		retval = fwk_of_property_read_u32(sptr_np, "#size-cells", &value);
		if (!retval)
			return value;
	}

	return 0;
}

/*!
 * @brief   Get the value of #address-cells
 * @param   none
 * @retval  none
 * @note    none
 */
kuint32_t fwk_of_n_addr_cells(struct fwk_device_node *sptr_node)
{
	struct fwk_device_node *sptr_np;
	kuint32_t value;
	kint32_t retval;

	/*!< Search "#address-cells", from child to parent */
	for (sptr_np = sptr_node; isValid(sptr_np); sptr_np = sptr_np->parent)
	{
		retval = fwk_of_property_read_u32(sptr_np, "#address-cells", &value);
		if (!retval)
			return value;
	}

	return 0;
}

/*!
 * @brief   Compare if the property of compatible are the same
 * @param   none
 * @retval  none
 * @note    Compatible property may be a string or an array of strings
 */
kbool_t fwk_of_device_is_compatible(struct fwk_device_node *sptr_node, const kchar_t *ptr_compat)
{
	kchar_t *ptr_info;
	kuint32_t index	= 0;

	while (!fwk_of_property_read_string_index(sptr_node, "compatible", index, &ptr_info))
	{
		index++;

		if (!strcmp(ptr_info, ptr_compat))
			return true;
	}

	return false;
}

/*!
 * @brief   parse arguments
 * @param   sptr_node: current node
 * @param	list_name: property name
 * @param	cells_name: the property named xxx-cells in parent node
 * @param	index: position
 * @param	sptr_args: save to this
 * @retval  errno
 * @note    none
 */
kint32_t fwk_of_parse_phandle_with_args(struct fwk_device_node *sptr_node, const kchar_t *list_name,
							const kchar_t *cells_name, kuint32_t cell_count, kint32_t index, struct fwk_of_phandle_args *sptr_args)
{
	struct fwk_device_node *sptr_np;
	kuint32_t *ptr_value, *ptr_end;
	kuint32_t phandle, cells = 0, cur_index = 0;
	kusize_t size = 0, count, i;

	if (!sptr_node || (index < 0) || !sptr_args)
		return -ER_NODEV;

	ptr_value = fwk_of_get_property(sptr_node, list_name, &size);
	if (!ptr_value || !size)
		return -ER_NOTFOUND;

	count = size / sizeof(*ptr_value);
	ptr_end = ptr_value + count;

	while (ptr_value < ptr_end)
	{
		phandle = FDT_TO_ARCH_PTR32(ptr_value++);

		sptr_np = fwk_of_find_node_by_phandle(mr_nullptr, phandle);
		if (!sptr_np)
			return -ER_NOTFOUND;

		if (cells_name)
		{
			if (fwk_of_property_read_u32(sptr_np, cells_name, &cells))
				return -ER_EMPTY;
		}
		else
			cells = cell_count;

		if (!cells)
			return -ER_CHECKERR;

		if ((ptr_value + cells) > ptr_end)
			return -ER_MORE;

		if ((cur_index++) == index)
		{
			sptr_args->sptr_node = sptr_np;
			sptr_args->args_count = cells;

			for (i = 0; i < cells; i++)
				sptr_args->args[i] = FDT_TO_ARCH_PTR32(ptr_value++);

			goto END;
		}

		ptr_value += cells;
	}

	return -ER_ERROR;

END:
	return ER_NORMAL;
}

/*!
 * @brief   get the order of match_name in sptr_prop which named list_name
 * @param   sptr_node: device node
 * @param	list_name: property name
 * @param	match_name: property's value
 * @retval  idx
 * @note    none
 */
kint32_t fwk_of_property_match_string(struct fwk_device_node *sptr_node, const kchar_t *list_name, const kchar_t *match_name)
{
	struct fwk_of_property *sptr_prop;
	kchar_t *str, *end;
	kusize_t size = 0;
	kuint32_t idx, lenth;

	sptr_prop = fwk_of_find_property(sptr_node, list_name, &size);
	if (!isValid(sptr_prop) || !size)
		return -ER_NOTFOUND;

	str = (kchar_t *)sptr_prop->value;
	end = str + size;

	for (idx = 0; str < end; str += lenth)
	{
		lenth = strlen(str) + 1;
		if ((str + lenth) > end)
			return -ER_MORE;

		if (!strcmp(str, match_name))
			return idx;

		idx++;
	}

	return -ER_CHECKERR;
}

/*!< end of file */
