/*
 * String General Function
 *
 * File Name:   api_string.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.26
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/api_string.h>
#include <common/io_stream.h>

/*!< API function */
/*!
 * @brief   get_integrater_lenth
 * @param   value
 * @retval  none
 * @note    calculate the lenth of integrater
 */
kusize_t get_integrater_lenth(kuint64_t value)
{
	kuint64_t num;
	kusize_t lenth;

	num = value;
	lenth = 0;

	/*!< Dividing by 10 and taking remainder */
	do
	{
		num = num / 10;
		lenth++;

	} while (num);

	return lenth;
}

/*!
 * @brief   get_string_lenth
 * @param   ptr_src
 * @retval  none
 * @note    calculate the lenth of string, does not include '\0'
 */
kusize_t get_string_lenth(const void *ptr_src)
{
	kuint8_t *ptr_ch;

	ptr_ch = (kuint8_t *)ptr_src;

	while ('\0' != *(ptr_ch++));

	return ((kusize_t)(ptr_ch - (kuint8_t *)ptr_src) - 1);
}

/*!
 * @brief   do_string_split
 * @param   ptr_dst, offset, ptr_src
 * @retval  none
 * @note    split two strings
 */
void do_string_split(void *ptr_dst, kuint32_t offset, const void *ptr_src)
{
	kuint8_t *ptr_ch;
	kuint8_t *ptr_buf;

	ptr_ch  = (kuint8_t *)ptr_src;
	ptr_buf	= (kuint8_t *)ptr_dst + offset;

	while ('\0' != *ptr_ch)
		*(ptr_buf++) = *(ptr_ch++);
}

/*!
 * @brief   do_string_copy
 * @param   ptr_dst, ptr_src
 * @retval  none
 * @note    copy string to another string
 */
kint8_t *do_string_copy(void *ptr_dst, const void *ptr_src)
{
	kuint8_t *ptr_ch;
	kuint8_t *ptr_buf;

	ptr_ch  = (kuint8_t *)ptr_src;
	ptr_buf	= (kuint8_t *)ptr_dst;

	do
	{
		*(ptr_buf++) = *(ptr_ch++);

	} while ('\0' != *ptr_ch);

	return (kint8_t *)ptr_dst;
}

/*!
 * @brief   do_string_n_copy
 * @param   ptr_dst, offset, ptr_src
 * @retval  none
 * @note    copy n char to another string
 */
kint8_t *do_string_n_copy(void *ptr_dst, const void *ptr_src, kuint32_t size)
{
	kuint8_t *ptr_ch;
	kuint8_t *ptr_buf;

	ptr_ch  = (kuint8_t *)ptr_src;
	ptr_buf	= (kuint8_t *)ptr_dst;

	while ('\0' != *ptr_ch && (size--))
		*(ptr_buf++) = *(ptr_ch++);

	return (kint8_t *)ptr_dst;
}

/*!
 * @brief   do_string_n_copy_safe
 * @param   ptr_dst, offset, ptr_src
 * @retval  none
 * @note    copy n char to another string
 */
kuint32_t do_string_n_copy_safe(void *ptr_dst, const void *ptr_src, kuint32_t size)
{
	kuint8_t *ptr_ch;
	kuint8_t *ptr_buf;
	kuint32_t lenth;

	ptr_ch  = (kuint8_t *)ptr_src;
	ptr_buf	= (kuint8_t *)ptr_dst;
	lenth = get_string_lenth(ptr_src);

	while (('\0' != *ptr_ch) && size)
	{
		if (--size)
		{
			*(ptr_buf++) = *(ptr_ch++);
			continue;
		}

		*ptr_buf = '\0';
	}

	return lenth;
}

/*!
 * @brief   do_string_compare
 * @param   ptr_dst, ptr_src
 * @retval  none
 * @note    compare string
 */
kbool_t do_string_compare(const void *ptr_dst, const void *ptr_src)
{
	kuint8_t *ptr_ch;
	kuint8_t *ptr_buf;

	ptr_ch  = (kuint8_t *)ptr_src;
	ptr_buf	= (kuint8_t *)ptr_dst;

	do
	{
		if (*(ptr_buf++) != *(ptr_ch++))
			return true;

	} while ('\0' != *ptr_ch);

	return false;	
}

/*!
 * @brief   do_string_n_compare
 * @param   ptr_dst, ptr_src, size
 * @retval  none
 * @note    compare n char
 */
kbool_t do_string_n_compare(const void *ptr_dst, const void *ptr_src, kuint32_t size)
{
	kuint8_t *ptr_ch;
	kuint8_t *ptr_buf;

	ptr_ch  = (kuint8_t *)ptr_src;
	ptr_buf	= (kuint8_t *)ptr_dst;

	if (!size)
		return true;

	do
	{
		if (*(ptr_buf++) != *(ptr_ch++))
			return true;

	} while (('\0' != *ptr_ch) && (--size));

	return false;	
}

/*!
 * @brief   do_string_reverse
 * @param   value
 * @retval  none
 * @note    reverse string
 */
void do_string_reverse(void *ptr_src, kuint32_t size)
{
	kuint8_t *ptr_head, *ptr_tail;
	kuint8_t ch;

	/*!< ptr_head points to string head, ptr_tail points to end symbol '\0' */
	ptr_head = (kuint8_t *)ptr_src;
	ptr_tail = (kuint8_t *)ptr_src + ((size) ? size : get_string_lenth(ptr_src));

	/*!< if the number of characters is lower than 2, it's not necessary to reverse */
	if ((kuint32_t)(ptr_tail - ptr_head) <= 1)
		return;

	ptr_tail--;

	while (ptr_tail > ptr_head)
	{
		ch				= *(ptr_head);
		*(ptr_head++) 	= *(ptr_tail);
		*(ptr_tail--) 	= ch;
	}
}

/*!
 * @brief   convert_number_to_string
 * @param   ptr_dst, value
 * @retval  none
 * @note    integrater convert to string
 */
kusize_t convert_number_to_string(void *ptr_dst, kuint64_t value)
{
	kuint8_t *ptr_buf;
	kuint64_t num;
	kusize_t lenth;

	ptr_buf = (kuint8_t *)ptr_dst;
	num		= value;
	lenth	= 0;

	/*!< Dividing by 10 and taking remainder */
	do
	{
		num = num / 10;
		lenth++;

		if (isValid(ptr_dst))
		{
			*(ptr_buf++) = (value - ((num << 1) + (num << 3))) + '0';
			value = value / 10;
		}

	} while (num);

	/*!< reverse string */
	if (isValid(ptr_dst))
		do_string_reverse(ptr_dst, lenth);

	return lenth;
}

/*!
 * @brief   seek_char_in_string
 * @param   ptr_src, ch
 * @retval  the character position in string
 * @note    find a character position
 */
kuint32_t seek_char_in_string(const void *ptr_src, kuint8_t ch)
{
	kuint8_t *ptr_head, *ptr_tail;
	kuint32_t iChPosition = 0;

	ptr_head = (kuint8_t *)ptr_src;
	ptr_tail = ptr_head + get_string_lenth(ptr_src) - 1;

	while (ptr_head != ptr_tail)
	{
		if (ch == *(ptr_tail--))
			return iChPosition;

		iChPosition++;
	}

	if (ch == *ptr_tail)
		return iChPosition;

	return 0;
}

/*!
 * @brief   do_fmt_convert
 * @param   ptr_buf, ptr_level, ptr_fmt, ptr_list
 * @retval  none
 * @note    string format conversion
 */
kusize_t do_fmt_convert(void *ptr_buf, kubyte_t *ptr_level, const kchar_t *ptr_fmt, va_list ptr_list, kusize_t size)
{
	kchar_t *ptr_data, *ptr_args;
	kubyte_t ch;
	kusize_t lenth, count;
	kuint64_t i, num;

	ptr_data = (kchar_t *)ptr_buf;
	lenth = 0;
	
	if (size < 1)
		return 0;

	size -= 1;

	/*!< calculate numbers first */
	for (i = 0; *(ptr_fmt + i) != '\0'; i++)
	{
		ch = *(ptr_fmt + i);

		if (ch == '\n')
		{
			/*!< jump over */
		}

		else if (ch == *(PRINT_LEVEL_SOH))
		{
			i++;

			if (ptr_level)
			{
				*ptr_level = *(PRINT_LEVEL_SOH);
				*(ptr_level + 1) = *(ptr_fmt + i);
			}

			continue;
		}

		/*!< ------------------------------------------------------------------ */
		/*!< Aim to '%' */
		if (ch != '%')
		{
			if ((lenth + 1) > size)
				break;

			if (isValid(ptr_buf))
				*(ptr_data++) = ch;

			lenth++;

			continue;
		}

		/*!< if (ch == '%') */
		i++;
		ch = *(ptr_fmt + i);

		switch (ch)
		{
			case 'c':
				num = (kuint8_t)va_arg(ptr_list, kuint32_t);

				if ((lenth + 1) > size)
					break;

				if (isValid(ptr_buf))
					*(ptr_data++) = num + ' ';

				lenth += 1;
				break;

			case 'd':
				num = (kuint32_t)va_arg(ptr_list, kuint32_t);
				count = convert_number_to_string(mrt_nullptr, num);

				if ((lenth + count) > size)
					break;

				convert_number_to_string(ptr_data, num);

				if (isValid(ptr_buf))
					ptr_data += count;

				lenth += count;
				break;

			case 'l':
				if (*(ptr_fmt + i + 1) == 'd')
				{
					num = (kuint64_t)va_arg(ptr_list, kuint64_t);
					count = convert_number_to_string(mrt_nullptr, num);

					if ((lenth + count) > size)
						break;

					if (isValid(ptr_buf))
					{
						convert_number_to_string(ptr_data, num);
						ptr_data += count;
					}

					i++;
					lenth += count;
				}
				else
				{
					if ((lenth + 1) > size)
						break;

					if (isValid(ptr_buf))
						*(ptr_data++) = '%';

					i--;
					lenth++;
				}
				break;

			case 's':
				ptr_args = (kchar_t *)va_arg(ptr_list, kchar_t *);
				count	 = get_string_lenth(ptr_args);

				if ((lenth + count) > size)
					break;

				if (isValid(ptr_buf))
				{
					do_string_split(ptr_data, 0, ptr_args);
					ptr_data += count;
				}

				lenth += count;

				break;

			default:
				if ((lenth + 1) > size)
					break;

				if (isValid(ptr_buf))
					*(ptr_data++) = '%';

				i--;
				lenth++;
				break;
		}
	}

	if (isValid(ptr_buf))
		*ptr_data = '\0';

	return lenth;
}

/*!
 * @brief   sprintk
 * @param   ptr_buf, ptr_fmt
 * @retval  none
 * @note    String format conversion
 */
kint32_t sprintk(void *ptr_buf, const kchar_t *ptr_fmt, ...)
{
	va_list ptr_list;
	kusize_t size;

	va_start(ptr_list, ptr_fmt);
	size = do_fmt_convert(ptr_buf, mrt_nullptr, ptr_fmt, ptr_list, (~0) - 1);
	va_end(ptr_list);

	return size;
}

/*!< -------------------------------------------------------------------- */
#if (0)
/*!
 * @brief   strlen
 * @param   none
 * @retval  none
 * @note    return string lenth
 */
__weak size_t strlen(const char *__s)
{
	return (size_t)get_string_lenth(__s);
}

/*!
 * @brief   strcpy
 * @param   none
 * @retval  none
 * @note    copy src to dest
 */
__weak char *strcpy(char *__dest, const char *__src)
{
	return do_string_copy(__dest, __src);
}

/*!
 * @brief   strncpy
 * @param   none
 * @retval  none
 * @note    copy src to dest
 */
__weak char *strncpy(char *__dest, const char *__src, size_t __n)
{
	return do_string_n_copy(__dest, __src, __n);
}

/*!
 * @brief   strlcpy
 * @param   none
 * @retval  none
 * @note    copy src to dest
 */
__weak unsigned int strlcpy(char *__dest, const char *__src, size_t __n)
{
	return do_string_n_copy_safe(__dest, __src, __n);
}

/*!
 * @brief   strcmp
 * @param   none
 * @retval  none
 * @note    compare s1 and s2
 */
__weak int strcmp(const char *__s1, const char *__s2)
{
	return do_string_compare(__s1, __s2);
}

/*!
 * @brief   strncmp
 * @param   none
 * @retval  none
 * @note    compare s1 and s2
 */
__weak int strncmp(const char *__s1, const char *__s2, size_t __n)
{
	return do_string_n_compare(__s1, __s2, __n);
}

#else
/*!
 * @brief   strlen
 * @param   none
 * @retval  none
 * @note    return string lenth
 */
__weak kuint32_t kstrlen(const kchar_t *__s)
{
	return (kuint32_t)get_string_lenth(__s);
}

/*!
 * @brief   strcpy
 * @param   none
 * @retval  none
 * @note    copy src to dest
 */
__weak kchar_t *kstrcpy(kchar_t *__dest, const kchar_t *__src)
{
	return do_string_copy(__dest, __src);
}

/*!
 * @brief   strncpy
 * @param   none
 * @retval  none
 * @note    copy src to dest
 */
__weak kchar_t *kstrncpy(kchar_t *__dest, const kchar_t *__src, kusize_t __n)
{
	return do_string_n_copy(__dest, __src, __n);
}

/*!
 * @brief   strlcpy
 * @param   none
 * @retval  none
 * @note    copy src to dest
 */
__weak kusize_t kstrlcpy(kchar_t *__dest, const kchar_t *__src, kusize_t __n)
{
	return do_string_n_copy_safe(__dest, __src, __n);
}

/*!
 * @brief   strcmp
 * @param   none
 * @retval  none
 * @note    compare s1 and s2
 */
__weak kint32_t kstrcmp(const kchar_t *__s1, const kchar_t *__s2)
{
	return do_string_compare(__s1, __s2);
}

/*!
 * @brief   strncmp
 * @param   none
 * @retval  none
 * @note    compare s1 and s2
 */
__weak kint32_t kstrncmp(const kchar_t *__s1, const kchar_t *__s2, kusize_t __n)
{
	return do_string_n_compare(__s1, __s2, __n);
}

#endif


/* end of file */
