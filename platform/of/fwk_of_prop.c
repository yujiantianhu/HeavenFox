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
struct fwk_of_property *fwk_of_find_property(struct fwk_device_node *sprt_node, const kchar_t *ptr_name, kusize_t *ptr_lenth)
{
	struct fwk_of_property *sprt_head = isValid(sprt_node) ? sprt_node->properties : mrt_nullptr;
	struct fwk_of_property *sprt_list = mrt_nullptr;
	kusize_t iLenth	= 0;

	foreach_list_odd_head(sprt_head, sprt_list)
	{
		if (!strcmp(ptr_name, sprt_list->name))
		{
			iLenth = sprt_list->length;
			break;
		}
	}

	if (ptr_lenth)
		*ptr_lenth = iLenth;

	return sprt_list;
}

/*!
 * @brief   fwk_of_get_property
 * @param   none
 * @retval  none
 * @note    none
 */
void *fwk_of_get_property(struct fwk_device_node *sprt_node, const kchar_t *ptr_name, kusize_t *ptr_lenth)
{
	struct fwk_of_property *sprt_prop = fwk_of_find_property(sprt_node, ptr_name, ptr_lenth);

	return isValid(sprt_prop) ? sprt_prop->value : mrt_nullptr;
}

/*!
 * @brief   Read integer data in ptr_name properties as a single byte
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u8_array_index(struct fwk_device_node *sprt_node, const kchar_t *ptr_name, 
													kuint8_t *ptr_value, kuint32_t index, kusize_t size)
{
	struct fwk_of_property *sprt_prop = mrt_nullptr;
	kuint32_t i, lenth;

	/*!< sprt_prop == mrt_nullptr : The property is not available */
	sprt_prop = fwk_of_find_property(sprt_node, ptr_name, mrt_nullptr);
	if (!isValid(sprt_prop))
		return -ER_FAULT;

	lenth = sprt_prop->length;
	if ((index >= lenth) || (!size))
		return -ER_FAULT;

	/*!< sprt_prop->length: the number of bytes occupied by sprt_prop->value */
	size = ((index + size) >= lenth) ? lenth : (index + size);

	for (i = index; i < size; i++, ptr_value++)
		*ptr_value = *((kuint8_t *)sprt_prop->value + i);

	return ER_NORMAL;
}

/*!
 * @brief   Read integer data in ptr_name properties as a single byte
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u16_array_index(struct fwk_device_node *sprt_node, const kchar_t *ptr_name, 
													kuint16_t *ptr_value, kuint32_t index, kusize_t size)
{
	struct fwk_of_property *sprt_prop = mrt_nullptr;
	kuint32_t i, lenth;

	/*!< sprt_prop == mrt_nullptr : The property is not available */
	sprt_prop = fwk_of_find_property(sprt_node, ptr_name, mrt_nullptr);
	if (!isValid(sprt_prop))
		return -ER_FAULT;

	lenth = sprt_prop->length >> 1;
	if ((index >= lenth) || (!size))
		return -ER_FAULT;

	/*!< sprt_prop->length: the number of bytes occupied by sprt_prop->value */
	size = ((index + size) >= lenth) ? lenth : (index + size);

	for (i = index; i < size; i++, ptr_value++)
		*ptr_value = FDT_TO_ARCH_ENDIAN16(*((kuint16_t *)sprt_prop->value + i));

	return ER_NORMAL;
}

/*!
 * @brief   Read integer data in ptr_name properties as a single byte
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u32_array_index(struct fwk_device_node *sprt_node, const kchar_t *ptr_name, 
													kuint32_t *ptr_value, kuint32_t index, kusize_t size)
{
	struct fwk_of_property *sprt_prop = mrt_nullptr;
	kuint32_t i, lenth;

	/*!< sprt_prop == mrt_nullptr : The property is not available */
	sprt_prop = fwk_of_find_property(sprt_node, ptr_name, mrt_nullptr);
	if (!isValid(sprt_prop))
		return -ER_FAULT;

	lenth = sprt_prop->length >> 2;
	if ((index >= lenth) || (!size))
		return -ER_FAULT;

	/*!< sprt_prop->length: the number of bytes occupied by sprt_prop->value */
	size = ((index + size) >= lenth) ? lenth : (index + size);

	for (i = index; i < size; i++, ptr_value++)
		*ptr_value = FDT_TO_ARCH_ENDIAN32(*((kuint32_t *)sprt_prop->value + i));

	return ER_NORMAL;
}

/*!
 * @brief   Obtain the 32-digit [index] value from the property name ptr_name
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u32_index(struct fwk_device_node *sprt_node, const kchar_t *ptr_name, kuint32_t index, kuint32_t *ptr_value)
{
	return fwk_of_property_read_u32_array_index(sprt_node, ptr_name, ptr_value, index, 1);
}

/*!
 * @brief   Read integer data in ptr_name properties as a single byte
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u8_array(struct fwk_device_node *sprt_node, const kchar_t *ptr_name, kuint8_t *ptr_value, kusize_t size)
{
	return fwk_of_property_read_u8_array_index(sprt_node, ptr_name, ptr_value, 0, size);
}

/*!
 * @brief   Read integer data in ptr_name properties in double-byte fashion
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u16_array(struct fwk_device_node *sprt_node, const kchar_t *ptr_name, kuint16_t *ptr_value, kusize_t size)
{
	return fwk_of_property_read_u16_array_index(sprt_node, ptr_name, ptr_value, 0, size);
}

/*!
 * @brief   Read integer data in ptr_name properties in 4-byte fashion
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u32_array(struct fwk_device_node *sprt_node, const kchar_t *ptr_name, kuint32_t *ptr_value, kusize_t size)
{
	return fwk_of_property_read_u32_array_index(sprt_node, ptr_name, ptr_value, 0, size);
}

/*!
 * @brief   Read integer data in ptr_name properties as a single byte
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u8(struct fwk_device_node *sprt_node, const kchar_t *ptr_name, kuint8_t *ptr_value)
{
	return fwk_of_property_read_u8_array_index(sprt_node, ptr_name, ptr_value, 0, 1);
}

/*!
 * @brief   Read integer data in ptr_name properties in double-byte fashion
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u16(struct fwk_device_node *sprt_node, const kchar_t *ptr_name, kuint16_t *ptr_value)
{
	return fwk_of_property_read_u16_array_index(sprt_node, ptr_name, ptr_value, 0, 1);
}

/*!
 * @brief   Reads integer data in ptr_name properties in 4-byte mode
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_u32(struct fwk_device_node *sprt_node, const kchar_t *ptr_name, kuint32_t *ptr_value)
{
	return fwk_of_property_read_u32_array_index(sprt_node, ptr_name, ptr_value, 0, 1);
}

/*!
 * @brief   Read integer data in ptr_name properties as strings
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_string(struct fwk_device_node *sprt_node, const kchar_t *ptr_name, kchar_t **ptr_string)
{
	struct fwk_of_property *sprt_prop = mrt_nullptr;

	/*!< sprt_prop == mrt_nullptr : The property is not available */
	sprt_prop = fwk_of_find_property(sprt_node, ptr_name, mrt_nullptr);
	if (!isValid(sprt_prop))
	{
		*ptr_string = mrt_nullptr;
		return -ER_FAULT;
	}

	*ptr_string	= (kchar_t *)sprt_prop->value;

	return ER_NORMAL;
}

/*!
 * @brief   Get the index-string from the property name ptr_name
 * @param   none
 * @retval  none
 * @note    none
 */
kint32_t fwk_of_property_read_string_index(struct fwk_device_node *sprt_node,
											const kchar_t *ptr_name, kuint32_t index, kchar_t **ptr_string)
{
	struct fwk_of_property *sprt_prop = mrt_nullptr;
	kchar_t *ptr_value = mrt_nullptr;
	kusize_t iStrLenth = 0, iTotalLenth;
	kuint32_t i;

	/*!< sprt_prop == mrt_nullptr : The property is not available */
	sprt_prop = fwk_of_find_property(sprt_node, ptr_name, mrt_nullptr);
	if (!isValid(sprt_prop))
	{
		*ptr_string = mrt_nullptr;
		return -ER_FAULT;
	}

	for (i = 0, iTotalLenth = 0; ((i <= index) && (iTotalLenth < sprt_prop->length)); i++)
	{
		iStrLenth = ptr_value ? (strlen(ptr_value) + 1) : 0;
		ptr_value = ptr_value ? (ptr_value + iStrLenth) : (kchar_t *)sprt_prop->value;
		iTotalLenth += iStrLenth;
	}

	/*!< sprt_prop->length: the number of bytes occupied by sprt_prop->value */
	if (iTotalLenth >= sprt_prop->length)
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
kuint32_t fwk_of_n_size_cells(struct fwk_device_node *sprt_node)
{
	struct fwk_device_node *sprt_np;
	kuint32_t value;
	kint32_t retval;

	for (sprt_np = sprt_node; isValid(sprt_np); sprt_np = sprt_np->parent)
	{
		retval = fwk_of_property_read_u32(sprt_np, "#size-cells", &value);
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
kuint32_t fwk_of_n_addr_cells(struct fwk_device_node *sprt_node)
{
	struct fwk_device_node *sprt_np;
	kuint32_t value;
	kint32_t retval;

	/*!< Search "#address-cells", from child to parent */
	for (sprt_np = sprt_node; isValid(sprt_np); sprt_np = sprt_np->parent)
	{
		retval = fwk_of_property_read_u32(sprt_np, "#address-cells", &value);
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
kbool_t fwk_of_device_is_compatible(struct fwk_device_node *sprt_node, const kchar_t *ptr_compat)
{
	kchar_t *ptr_info;
	kuint32_t index	= 0;

	while (!fwk_of_property_read_string_index(sprt_node, "compatible", index, &ptr_info))
	{
		index++;

		if (!strcmp(ptr_info, ptr_compat))
			return true;
	}

	return false;
}

/*!
 * @brief   parse arguments
 * @param   sprt_node: current node
 * @param	list_name: property name
 * @param	cells_name: the property named xxx-cells in parent node
 * @param	index: position
 * @param	sprt_args: save to this
 * @retval  errno
 * @note    none
 */
kint32_t fwk_of_parse_phandle_with_args(struct fwk_device_node *sprt_node, const kchar_t *list_name,
							const kchar_t *cells_name, kuint32_t cell_count, kint32_t index, struct fwk_of_phandle_args *sprt_args)
{
	struct fwk_device_node *sprt_np;
	kuint32_t *ptr_value, *ptr_end;
	kuint32_t phandle, cells = 0, cur_index = 0;
	kusize_t size = 0, count, i;

	if (!sprt_node || (index < 0) || !sprt_args)
		return -ER_NODEV;

	ptr_value = fwk_of_get_property(sprt_node, list_name, &size);
	if (!ptr_value || !size)
		return -ER_NOTFOUND;

	count = size / sizeof(*ptr_value);
	ptr_end = ptr_value + count;

	while (ptr_value < ptr_end)
	{
		phandle = FDT_TO_ARCH_PTR32(ptr_value++);

		sprt_np = fwk_of_find_node_by_phandle(mrt_nullptr, phandle);
		if (!sprt_np)
			return -ER_NOTFOUND;

		if (cells_name)
		{
			if (fwk_of_property_read_u32(sprt_np, cells_name, &cells))
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
			sprt_args->sprt_node = sprt_np;
			sprt_args->args_count = cells;

			for (i = 0; i < cells; i++)
				sprt_args->args[i] = FDT_TO_ARCH_PTR32(ptr_value++);

			goto END;
		}

		ptr_value += cells;
	}

	return -ER_ERROR;

END:
	return ER_NORMAL;
}

/*!
 * @brief   get the order of match_name in sprt_prop which named list_name
 * @param   sprt_node, list_name, match_name
 * @retval  idx
 * @note    none
 */
kint32_t fwk_of_property_match_string(struct fwk_device_node *sprt_node, const kchar_t *list_name, const kchar_t *match_name)
{
	struct fwk_of_property *sprt_prop;
	kchar_t *str, *end;
	kusize_t size = 0;
	kuint32_t idx, lenth;

	sprt_prop = fwk_of_find_property(sprt_node, list_name, &size);
	if (!isValid(sprt_prop) || !size)
		return -ER_NOTFOUND;

	str = (kchar_t *)sprt_prop->value;
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
