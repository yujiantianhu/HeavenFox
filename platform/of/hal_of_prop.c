/*
 * Hardware Abstraction Layer Device-Tree Property
 *
 * File Name:   hal_of_prop.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.14
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/of/hal_of_prop.h>

/*!< API function */
/*!
 * @brief   Locate the property based on its name
 * @param   none
 * @retval  none
 * @note    none
 */
struct hal_of_property *hal_of_find_property(struct hal_device_node *sprt_node, const kstring_t *ptr_name, kusize_t *ptr_lenth)
{
	struct hal_of_property *sprt_head = mrt_isValid(sprt_node) ? sprt_node->properties : mrt_nullptr;
	struct hal_of_property *sprt_list = mrt_nullptr;
	kusize_t iLenth	= 0;

	foreach_list_odd_head(sprt_head, sprt_list)
	{
		if (!strcmp(ptr_name, sprt_list->name))
		{
			iLenth = sprt_list->length;
			break;
		}
	}

	if (mrt_isValid(ptr_lenth))
	{
		*ptr_lenth = iLenth;
	}

	return sprt_list;
}

/*!
 * @brief   hal_of_get_property
 * @param   none
 * @retval  none
 * @note    none
 */
void *hal_of_get_property(struct hal_device_node *sprt_node, const kstring_t *ptr_name, kusize_t *ptr_lenth)
{
	struct hal_of_property *sprt_prop = hal_of_find_property(sprt_node, ptr_name, ptr_lenth);

	return (mrt_isValid(sprt_prop) ? sprt_prop->value : mrt_nullptr);
}

/*!
 * @brief   Read integer data in ptr_name properties as a single byte
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
ksint32_t hal_of_property_read_u8_array_index(struct hal_device_node *sprt_node, const kstring_t *ptr_name, 
													kuint8_t *ptr_value, kuint32_t index, kusize_t size)
{
	struct hal_of_property *sprt_prop = mrt_nullptr;
	kuint32_t i, lenth;

	/*!< sprt_prop == mrt_nullptr : The property is not available */
	sprt_prop = hal_of_find_property(sprt_node, ptr_name, mrt_nullptr);
	if (!mrt_isValid(sprt_prop))
	{
		return mrt_retval(Ert_isArgFault);
	}

	lenth = sprt_prop->length;
	if ((index >= lenth) || (!size))
	{
		return mrt_retval(Ert_isArgFault);
	}

	/*!< sprt_prop->length: the number of bytes occupied by sprt_prop->value */
	size = ((index + size) >= lenth) ? lenth : (index + size);

	for (i = index; i < size; i++, ptr_value++)
	{
		*ptr_value = *((kuint8_t *)sprt_prop->value + i);
	}

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   Read integer data in ptr_name properties as a single byte
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
ksint32_t hal_of_property_read_u16_array_index(struct hal_device_node *sprt_node, const kstring_t *ptr_name, 
													kuint16_t *ptr_value, kuint32_t index, kusize_t size)
{
	struct hal_of_property *sprt_prop = mrt_nullptr;
	kuint32_t i, lenth;

	/*!< sprt_prop == mrt_nullptr : The property is not available */
	sprt_prop = hal_of_find_property(sprt_node, ptr_name, mrt_nullptr);
	if (!mrt_isValid(sprt_prop))
	{
		return mrt_retval(Ert_isArgFault);
	}

	lenth = sprt_prop->length >> 1;
	if ((index >= lenth) || (!size))
	{
		return mrt_retval(Ert_isArgFault);
	}

	/*!< sprt_prop->length: the number of bytes occupied by sprt_prop->value */
	size = ((index + size) >= lenth) ? lenth : (index + size);

	for (i = index; i < size; i++, ptr_value++)
	{
		*ptr_value = FDT_TO_ARCH_ENDIAN16(*((kuint16_t *)sprt_prop->value + i));
	}

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   Read integer data in ptr_name properties as a single byte
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
ksint32_t hal_of_property_read_u32_array_index(struct hal_device_node *sprt_node, const kstring_t *ptr_name, 
													kuint32_t *ptr_value, kuint32_t index, kusize_t size)
{
	struct hal_of_property *sprt_prop = mrt_nullptr;
	kuint32_t i, lenth;

	/*!< sprt_prop == mrt_nullptr : The property is not available */
	sprt_prop = hal_of_find_property(sprt_node, ptr_name, mrt_nullptr);
	if (!mrt_isValid(sprt_prop))
	{
		return mrt_retval(Ert_isArgFault);
	}

	lenth = sprt_prop->length >> 2;
	if ((index >= lenth) || (!size))
	{
		return mrt_retval(Ert_isArgFault);
	}

	/*!< sprt_prop->length: the number of bytes occupied by sprt_prop->value */
	size = ((index + size) >= lenth) ? lenth : (index + size);

	for (i = index; i < size; i++, ptr_value++)
	{
		*ptr_value = FDT_TO_ARCH_ENDIAN32(*((kuint32_t *)sprt_prop->value + i));
	}

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   Obtain the 32-digit [index] value from the property name ptr_name
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t hal_of_property_read_u32_index(struct hal_device_node *sprt_node, const kstring_t *ptr_name, kuint32_t index, kuint32_t *ptr_value)
{
	return hal_of_property_read_u32_array_index(sprt_node, ptr_name, ptr_value, index, 1);
}

/*!
 * @brief   Read integer data in ptr_name properties as a single byte
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
ksint32_t hal_of_property_read_u8_array(struct hal_device_node *sprt_node, const kstring_t *ptr_name, kuint8_t *ptr_value, kusize_t size)
{
	return hal_of_property_read_u8_array_index(sprt_node, ptr_name, ptr_value, 0, size);
}

/*!
 * @brief   Read integer data in ptr_name properties in double-byte fashion
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
ksint32_t hal_of_property_read_u16_array(struct hal_device_node *sprt_node, const kstring_t *ptr_name, kuint16_t *ptr_value, kusize_t size)
{
	return hal_of_property_read_u16_array_index(sprt_node, ptr_name, ptr_value, 0, size);
}

/*!
 * @brief   Read integer data in ptr_name properties in 4-byte fashion
 * @param   size: the number of array, it can be given by ARRAY_SIZE(arr)
 * @retval  none
 * @note    none
 */
ksint32_t hal_of_property_read_u32_array(struct hal_device_node *sprt_node, const kstring_t *ptr_name, kuint32_t *ptr_value, kusize_t size)
{
	return hal_of_property_read_u32_array_index(sprt_node, ptr_name, ptr_value, 0, size);
}

/*!
 * @brief   Read integer data in ptr_name properties as a single byte
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t hal_of_property_read_u8(struct hal_device_node *sprt_node, const kstring_t *ptr_name, kuint8_t *ptr_value)
{
	return hal_of_property_read_u8_array_index(sprt_node, ptr_name, ptr_value, 0, 1);
}

/*!
 * @brief   Read integer data in ptr_name properties in double-byte fashion
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t hal_of_property_read_u16(struct hal_device_node *sprt_node, const kstring_t *ptr_name, kuint16_t *ptr_value)
{
	return hal_of_property_read_u16_array_index(sprt_node, ptr_name, ptr_value, 0, 1);
}

/*!
 * @brief   Reads integer data in ptr_name properties in 4-byte mode
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t hal_of_property_read_u32(struct hal_device_node *sprt_node, const kstring_t *ptr_name, kuint32_t *ptr_value)
{
	return hal_of_property_read_u32_array_index(sprt_node, ptr_name, ptr_value, 0, 1);
}

/*!
 * @brief   Read integer data in ptr_name properties as strings
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t hal_of_property_read_string(struct hal_device_node *sprt_node, const kstring_t *ptr_name, kstring_t **ptr_string)
{
	struct hal_of_property *sprt_prop = mrt_nullptr;

	/*!< sprt_prop == mrt_nullptr : The property is not available */
	sprt_prop = hal_of_find_property(sprt_node, ptr_name, mrt_nullptr);
	if (!mrt_isValid(sprt_prop))
	{
		*ptr_string = mrt_nullptr;
		return mrt_retval(Ert_isArgFault);
	}

	*ptr_string	= (kstring_t *)sprt_prop->value;

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   Get the index-string from the property name ptr_name
 * @param   none
 * @retval  none
 * @note    none
 */
ksint32_t hal_of_property_read_string_index(struct hal_device_node *sprt_node,
											const kstring_t *ptr_name, kuint32_t index, kstring_t **ptr_string)
{
	struct hal_of_property *sprt_prop = mrt_nullptr;
	kstring_t *ptr_value = mrt_nullptr;
	kusize_t iStrLenth = 0, iTotalLenth;
	kuint32_t i;

	/*!< sprt_prop == mrt_nullptr : The property is not available */
	sprt_prop = hal_of_find_property(sprt_node, ptr_name, mrt_nullptr);
	if (!mrt_isValid(sprt_prop))
	{
		*ptr_string = mrt_nullptr;
		return mrt_retval(Ert_isArgFault);
	}

	for (i = 0, iTotalLenth = 0; ((i <= index) && (iTotalLenth < sprt_prop->length)); i++)
	{
		iStrLenth = mrt_isValid(ptr_value) ? (strlen(ptr_value) + 1) : 0;
		ptr_value = mrt_isValid(ptr_value) ? (ptr_value + iStrLenth) : (kstring_t *)sprt_prop->value;
		iTotalLenth += iStrLenth;
	}

	/*!< sprt_prop->length: the number of bytes occupied by sprt_prop->value */
	if (iTotalLenth >= sprt_prop->length)
	{
		return mrt_retval(Ert_isArrayOver);
	}

	*ptr_string	= ptr_value;

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   Get the value of #size-cells
 * @param   none
 * @retval  none
 * @note    none
 */
kuint32_t hal_of_n_size_cells(struct hal_device_node *sprt_node)
{
	struct hal_device_node *sprt_np;
	kuint32_t value;
	ksint32_t retval;

	for (sprt_np = sprt_node; mrt_isValid(sprt_np); sprt_np = sprt_np->parent)
	{
		retval = hal_of_property_read_u32(sprt_np, "#size-cells", &value);
		if (!mrt_isErr(retval))
		{
			return value;
		}
	}

	return 0;
}

/*!
 * @brief   Get the value of #address-cells
 * @param   none
 * @retval  none
 * @note    none
 */
kuint32_t hal_of_n_addr_cells(struct hal_device_node *sprt_node)
{
	struct hal_device_node *sprt_np;
	kuint32_t value;
	ksint32_t retval;

	/*!< Search "#address-cells", from child to parent */
	for (sprt_np = sprt_node; mrt_isValid(sprt_np); sprt_np = sprt_np->parent)
	{
		retval = hal_of_property_read_u32(sprt_np, "#address-cells", &value);
		if (!mrt_isErr(retval))
		{
			return value;
		}
	}

	return 0;
}

/*!
 * @brief   Compare if the property of compatible are the same
 * @param   none
 * @retval  none
 * @note    Compatible property may be a string or an array of strings
 */
kbool_t hal_of_device_is_compatible(struct hal_device_node *sprt_node, const kstring_t *ptr_compat)
{
	kstring_t *ptr_info;
	kuint32_t index	= 0;

	while (!mrt_isErr(hal_of_property_read_string_index(sprt_node, "compatible", index, &ptr_info)))
	{
		index++;

		if (!strcmp(ptr_info, ptr_compat))
		{
			return Ert_true;
		}
	}

	return Ert_false;
}

