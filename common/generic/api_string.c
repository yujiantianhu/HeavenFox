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
#include <platform/base/fwk_mempool.h>

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
kchar_t *do_string_copy(void *ptr_dst, const void *ptr_src)
{
    kuint8_t *ptr_ch;
    kuint8_t *ptr_buf;

    ptr_ch  = (kuint8_t *)ptr_src;
    ptr_buf	= (kuint8_t *)ptr_dst;

    *ptr_buf = *ptr_ch;
    while ('\0' != *ptr_buf)
        *(++ptr_buf) = *(++ptr_ch);

    return (kchar_t *)ptr_dst;
}

/*!
 * @brief   do_string_n_copy
 * @param   ptr_dst, offset, ptr_src
 * @retval  none
 * @note    copy n char to another string
 */
kchar_t *do_string_n_copy(void *ptr_dst, const void *ptr_src, kuint32_t size)
{
    kuint8_t *ptr_ch;
    kuint8_t *ptr_buf;

    ptr_ch  = (kuint8_t *)ptr_src;
    ptr_buf	= (kuint8_t *)ptr_dst;

    if (!size)
        return mr_nullptr;

    *ptr_buf = *ptr_ch;
    while (('\0' != *ptr_buf) && (--size))
        *(++ptr_buf) = *(++ptr_ch);

    return (kchar_t *)ptr_dst;
}

/*!
 * @brief   do_string_n_copy_rev
 * @param   ptr_dst, offset, ptr_src
 * @retval  none
 * @note    reverse copy
 */
kchar_t *do_string_n_copy_rev(void *ptr_dst, const void *ptr_src, kuint32_t size)
{
    kuint8_t *ptr_ch;
    kuint8_t *ptr_buf;

    ptr_ch  = (kuint8_t *)ptr_src + size - 1;
    ptr_buf	= (kuint8_t *)ptr_dst + size - 1;

    while (size--)
        *(ptr_buf--) = *(ptr_ch--);

    return (kchar_t *)ptr_dst;
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

    if (!size)
        return lenth;

    *ptr_buf = *ptr_ch;
    while (('\0' != *ptr_buf) && (--size))
        *(++ptr_buf) = *(++ptr_ch);

    if (!size)
        *ptr_buf = '\0';

    return lenth;
}

/*!
 * @brief   do_string_compare
 * @param   ptr_dst, ptr_src
 * @retval  none
 * @note    compare string
 */
kint32_t do_string_compare(const void *s1, const void *s2)
{
    kuint8_t *str1 = (kuint8_t *)s1;
    kuint8_t *str2 = (kuint8_t *)s2;

    do {
        if (*str1 < *str2)
            return -1;
        else if (*str1 > *str2)
            return 1;
    } while (*(str1++) && *(str2++));

    return 0;	
}

/*!
 * @brief   do_string_n_compare
 * @param   ptr_dst, ptr_src, size
 * @retval  none
 * @note    compare n char
 */
kint32_t do_string_n_compare(const void *s1, const void *s2, kuint32_t size)
{
    kuint8_t *str1 = (kuint8_t *)s1;
    kuint8_t *str2 = (kuint8_t *)s2;

    if (!size)
        return 0;

    do {
        if (*str1 < *str2)
            return -1;
        else if (*str1 > *str2)
            return 1;
    } while (*(str1++) && *(str2++) && (--size));

    return 0;	
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
 * @brief   uint_to_str
 * @param   ptr_dst, value
 * @retval  none
 * @note    integrater convert to string
 */
kusize_t uint_to_str(void *ptr_dst, kuint64_t value)
{
    kuint8_t *ptr_buf;
    kuint64_t num;
    kusize_t lenth;

    ptr_buf = (kuint8_t *)ptr_dst;
    num		= value;
    lenth	= 0;

    /*!< Dividing by 10 and taking remainder */
    do {
        num = num / 10;
        lenth++;

        if (isValid(ptr_dst))
        {
            *(ptr_buf++) = (value - ((num << 1) + (num << 3))) + '0';
            value = value / 10;
        }

    } while (num);

    /*!< reverse string */
    if (ptr_dst)
        do_string_reverse(ptr_dst, lenth);

    return lenth;
}

/*!
 * @brief   uint_to_str
 * @param   ptr_dst, value
 * @retval  none
 * @note    integrater convert to string
 */
kusize_t int_to_str(void *ptr_dst, kint64_t value)
{
    kusize_t lenth = 0;

    if (value < 0) 
    {
        kchar_t *ptr_buf = (kchar_t *)ptr_dst;

        if (ptr_buf)
            *(ptr_buf++) = '-';

        lenth = uint_to_str(ptr_buf, (kuint64_t)(-value));
        lenth++;
    } 
    else 
    {
        lenth = uint_to_str(ptr_dst, (kuint64_t)(value));
    }

    return lenth;
}

/*!
 * @brief   seek_char_in_string
 * @param   ptr_src, ch
 * @retval  the character position in string
 * @note    find a character position
 */
kchar_t *seek_char_in_string(const void *ptr_src, kchar_t ch)
{
    kchar_t *ptr_ch;

    ptr_ch = (kchar_t *)ptr_src;

    while (*ptr_ch != '\0')
    {
        if (ch == *ptr_ch)
            return ptr_ch;

        ptr_ch++;
    }

    return mr_nullptr;
}

/*!
 * @brief   seek_char_by_pos
 * @param   ptr_src, offset
 * @retval  the character position in string
 * @note    find a character position
 */
kchar_t *seek_char_by_pos(const void *ptr_src, kuint32_t offset)
{
    kchar_t *ptr_ch, *ptr_end;

    ptr_ch  = (kchar_t *)ptr_src;
    ptr_end = ptr_ch + offset;

    while (*ptr_ch != '\0')
    {
        if (ptr_ch == ptr_end)
            return ptr_ch;

        ptr_ch++;
    }

    return mr_nullptr;
}

/*!
 * @brief   seek_n_char_in_string
 * @param   ptr_src, ch, n
 * @retval  the character position in string
 * @note    find the index character position; if n < 0, it is no limit
 */
kchar_t *seek_n_char_in_string(const void *ptr_src, kchar_t ch, kint32_t n)
{
    kchar_t *str, *str_found = mr_nullptr;
    kuint32_t count = (n < 0) ? (kuint32_t)(~0) : n;

    str = (kchar_t *)ptr_src;
    if (!str)
        return mr_nullptr;

    while (*str != '\0') 
    {
        str = seek_char_in_string(str, ch);
        if (!str || !(count--))
            return str_found;

        str_found = str++;
    }

    /*!< for example, ch = 'a', and string is "a\0" */
    return (n < 0) ? str_found : mr_nullptr;
}

static const kchar_t g_fmt_repeat_chars[] = 
{
    'l', 'c', 'd', 'u', 's', 'B', 'P', 'X', 
    'b', 'p', 'x', '0', '+', '-', '#'
};
static const kusize_t g_fmt_repeat_chars_num = ARRAY_SIZE(g_fmt_repeat_chars);

/*!
 * @brief   do_fmt_convert
 * @param   ptr_buf, ptr_level, ptr_fmt, ptr_list
 * @retval  none
 * @note    string format conversion
 */
kusize_t do_fmt_convert(void *ptr_buf, kubyte_t *ptr_level, const kchar_t *ptr_fmt, va_list ptr_list, kusize_t size)
{
    kchar_t *ptr_data = (kchar_t *)ptr_buf;
    kubyte_t ch, pad_ch = ' ';
    kusize_t lenth = 0, count;
    kuint32_t super_cnt = 0;
	kbool_t is_super = false;
    kbool_t is_longint = false;
    kbool_t is_hexprex = false;

    /*!< false: right align; true: left align */
    kbool_t left_align = false;
    kusize_t width = 0, align_pad = 0;
    
    if (size < 1)
        return 0;

    /*!< Ignore '\0' */
    size -= 1;

    /*!< calculate numbers first */
    for (kuint64_t i = 0; *(ptr_fmt + i) != '\0'; i++) 
    {
        /*!< read current character */
        ch = *(ptr_fmt + i);

        /*!< jump over PRINT_LEVEL_SOH */
        if ((0 == i) && (ch == *(PRINT_LEVEL_SOH)))
        {
            i++;
            if (ptr_level) 
            {
                *ptr_level = *(PRINT_LEVEL_SOH);
                *(ptr_level + 1) = *(ptr_fmt + i);
            }

            continue;
        }

        if (ch == '%') 
        {
            if (!is_super) 
            {
                super_cnt = 0;

                /*!< next character */
                i++;
                ch = *(ptr_fmt + i);
            }

            /*!< mark to start format */
            is_super = true;

            /*!< initial all flags */
            pad_ch = ' ';
            width = 0;
            left_align = false;
            is_longint = false;
            is_hexprex = false;
        }

        if (is_super) 
        {
            super_cnt++;

            switch (ch) 
            {
                case 'c': {
                    kchar_t chr = (kchar_t)va_arg(ptr_list, kuint32_t);

                    align_pad = (width > 1) ? (width - 1) : 0;
                    if ((lenth + 1 + align_pad) > size)
                        goto out;

                    if (ptr_buf) 
                    {
                        if (left_align) 
                        {
                            /*!< save character */
                            *ptr_data = chr;
                            /*!< character's length is 1, fill pad_ch to align */
                            memset(ptr_data + 1, pad_ch, align_pad);
                        } 
                        else 
                        {
                            /*!< character's length is 1, fill pad_ch to align */
                            memset(ptr_data, pad_ch, align_pad);
                            /*!< save character */
                            *(ptr_data + align_pad) = chr;
                        }

                        ptr_data += (1 + align_pad);
                    }

                    is_super = false;
                    lenth += (1 + align_pad);
                    break;
                }
                case 'd': {
                    kint64_t i_num = 0;

                    if (is_longint)
                        i_num = (kint64_t)va_arg(ptr_list, kint64_t);
                    else
                        i_num = (kint64_t)va_arg(ptr_list, kint32_t);

                    /*!< calculate length */
                    count = int_to_str(mr_nullptr, i_num);
                    align_pad = (width > count) ? (width - count) : 0;
                    if ((lenth + count + align_pad) > size)
                        goto out;

                    if (ptr_buf) 
                    {
                        if (left_align) 
                        {
                            /*!< convert digit number to string */
                            int_to_str(ptr_data, i_num);
                            /*!< if string is too short (less than width), fill pad_ch to align */
                            memset(ptr_data + count, pad_ch, align_pad);
                        } 
                        else 
                        {
                            /*!< if string is too short (less than width), fill pad_ch to align */
                            memset(ptr_data, pad_ch, align_pad);
                            /*!< convert digit number to string */
                            int_to_str(ptr_data + align_pad, i_num);
                        }
                    
                        ptr_data += (count + align_pad);
                    }

                    is_super = false;
                    lenth += (count + align_pad);
                    break;
                }
                case 'u': {
                    kuint64_t u_num = 0;

                    if (is_longint)
                        u_num = (kuint64_t)va_arg(ptr_list, kuint64_t);
                    else
                        u_num = (kuint64_t)va_arg(ptr_list, kuint32_t);

                    /*!< calculate length */
                    count = uint_to_str(mr_nullptr, u_num);
                    align_pad = (width > count) ? (width - count) : 0;
                    if ((lenth + count + align_pad) > size)
                        goto out;

                    if (ptr_buf) 
                    {
                        if (left_align) 
                        {
                            /*!< convert digit number to string */
                            uint_to_str(ptr_data, u_num);
                            /*!< if string is too short (less than width), fill pad_ch to align */
                            memset(ptr_data + count, pad_ch, align_pad);
                        }
                        else
                        {
                            /*!< if string is too short (less than width), fill pad_ch to align */
                            memset(ptr_data, pad_ch, align_pad);
                            /*!< convert digit number to string */
                            uint_to_str(ptr_data + align_pad, u_num);
                        }
                    
                        ptr_data += (count + align_pad);
                    }

                    is_super = false;
                    lenth += (count + align_pad);
                    break;
                }
                case 's': {
                    kchar_t *ptr_str = (kchar_t *)va_arg(ptr_list, kchar_t *);

                    /*!< calculate length */
                    count = get_string_lenth(ptr_str);

                    align_pad = (width > count) ? (width - count) : 0;
                    if ((lenth + count + align_pad) > size)
                        goto out;
    
                    if (ptr_buf)
                    {
                        if (left_align) 
                        {
                            /*!< split new string to ptr_data */
                            do_string_split(ptr_data, 0, ptr_str);
                            /*!< if string is too short (less than width), fill pad_ch to align */
                            memset(ptr_data + count, pad_ch, align_pad);
                        } 
                        else 
                        {
                            /*!< if string is too short (less than width), fill pad_ch to align */
                            memset(ptr_data, pad_ch, align_pad);
                            /*!< split new string to ptr_data + align_pad */
                            do_string_split(ptr_data, align_pad, ptr_str);
                        }

                        ptr_data += (count + align_pad);
                    }

                    is_super = false;
                    lenth += (count + align_pad);
                    break;
                }
                case 'x':
                case 'X':
                case 'p': 
                case 'P': {
                    kutype_t hex_num = 0;
                    kuint32_t mode = -1;

                    if (is_hexprex)
                        mode = (ch > 'a') ? 0 : 1;

                    if ((ch == 'p') || (ch == 'P'))
                        hex_num = (kutype_t)va_arg(ptr_list, void *);
                    else
                        hex_num = (kutype_t)va_arg(ptr_list, kutype_t);

                    /*!< calculate length */
                    count = dec_to_hex(mr_nullptr, hex_num, mode);
    
                    align_pad = (width > count) ? (width - count) : 0;
                    if ((lenth + count + align_pad) > size)
                        goto out;
    
                    if (ptr_buf) 
                    {
                        if (left_align) 
                        {
                            /*!< convert hex to string */
                            dec_to_hex(ptr_data, hex_num, mode);
                            /*!< if string is too short (less than width), fill pad_ch to align */
                            memset(ptr_data + count, pad_ch, align_pad);
                        } 
                        else 
                        {
                            /*!< if string is too short (less than width), fill pad_ch to align */
                            memset(ptr_data, pad_ch, align_pad);
                            /*!< convert hex to string */
                            dec_to_hex(ptr_data + align_pad, hex_num, mode);
                        }
                    
                        ptr_data += (count + align_pad);
                    }
    
                    is_super = false;
                    lenth += (count + align_pad);
                    break;
                }
                case 'b':
                case 'B': {
                    kutype_t bin_num = (kutype_t)va_arg(ptr_list, kutype_t);
                    kuint32_t mode = -1;

                    if (is_hexprex)
                        mode = (ch > 'a') ? 0 : 1;

                    /*!< calculate length */
                    count = dec_to_binary(mr_nullptr, bin_num, mode);

                    align_pad = (width > count) ? (width - count) : 0;
                    if ((lenth + count + align_pad) > size)
                        goto out;
    
                    if (ptr_buf) 
                    {
                        if (left_align) 
                        {
                            /*!< convert binary to string */
                            dec_to_binary(ptr_data, bin_num, mode);
                            /*!< if string is too short (less than width), fill pad_ch to align */
                            memset(ptr_data + count, pad_ch, align_pad);
                        } 
                        else 
                        {
                            /*!< if string is too short (less than width), fill pad_ch to align */
                            memset(ptr_data, pad_ch, align_pad);
                            /*!< convert binary to string */
                            dec_to_binary(ptr_data + align_pad, bin_num, mode);
                        }
                    
                        ptr_data += (count + align_pad);
                    }
    
                    is_super = false;
                    lenth += (count + align_pad);
                    break;
                }
                case 'l': 
                {
                    /*!< only permit "%ld", "%lu", "%lld", "%llu" */
                    kchar_t next_ch = *(ptr_fmt + i + 1);

                    if (((next_ch == 'd') || (next_ch == 'u')) ||
                        ((next_ch == 'l') && (!is_longint)))
                        is_longint = true;
                    else 
                    {
                        is_longint = false;
                        goto esac;
                    }
                    break;
                }
                case '0' ... '9': 
                {
                    /*!< do not set is_super to false */
                    /*!< width = width * 10 + ch - '0' */
                    width = (width << 1) + (width << 3) + ch - '0';

                    /*!< first enter */
                    if (!width)
                        pad_ch = (ch == '0') ? '0' : ' ';
                    break;
                }
                case '#':
                case '+':
                case '-': {
                    /*!< left margin align? */
                    const kchar_t *ptr_rpt = g_fmt_repeat_chars;
                    kusize_t rpt_num = g_fmt_repeat_chars_num;

                    kchar_t next_ch = *(ptr_fmt + i + 1);
                    kchar_t last_ch = *(ptr_fmt + i - 1);
                    kbool_t last_matched = false;
                    kbool_t next_matched = false;

                    /*!< if string is "%#3d"/"%#345d", it's correct; but "%3#d"/"%#3#d" is not allowed */
                    width = 0;

                    /*!
                     * @note '#', '+', '-' can be repeated, and no order requirement; 
                     *      but next character must be valid (such as 'x'/'X', '0' ~ '9', ...);
                     *      if next character is 'd'/'c'/'s'/'l', ignore '#'/'+'/'-', does not ouput
                     */
                    while (rpt_num--) 
                    {
                        if (next_ch == ptr_rpt[rpt_num])
                            next_matched = true;
                        if ((super_cnt == 1) || (last_ch == ptr_rpt[rpt_num]))
                            last_matched = true;
                    }

                    /*!< ptr_rpt not include '0' ~ '9' */
                    if (!last_matched || (!next_matched && ((next_ch < '0') || (next_ch > '9'))))
                        goto esac;

                    if (ch == '#')
                        is_hexprex = true;
                    else if (ch == '-')
                        left_align = true;
                    break;
                }
                default:
                    goto esac;
            }

            continue;

        esac:
            /*!< if string is invalid (such as "%3h"), clear width for next formating */
            width = 0;
            is_super = false;

            /*!< back to last character */
            i -= super_cnt;
            ch = '%';
        }
        
        if ((lenth + 1) > size)
            break;

        if (ptr_buf)
            *(ptr_data++) = ch;

        lenth++;
    }

out:
    if (ptr_buf)
        *ptr_data = '\0';

    return lenth;
}

/*!
 * @brief   vasprintk_safe
 * @param   ptr_buf, ptr_fmt
 * @retval  none
 * @note    String format conversion
 */
kchar_t *vasprintk_safe(const kchar_t *ptr_fmt, kusize_t *size, va_list sptr_list)
{
    va_list sptr_copy;
    kchar_t *ptr;
    kusize_t lenth;

    if (!ptr_fmt)
        return mr_nullptr;

    va_copy(sptr_copy, sptr_list);
    lenth = do_fmt_convert(mr_nullptr, mr_nullptr, ptr_fmt, sptr_copy, (kusize_t)(~0));
    va_end(sptr_copy);

    ptr = kmalloc(lenth + 1, GFP_KERNEL);
    if (!isValid(ptr))
        return mr_nullptr;

    do_fmt_convert(ptr, mr_nullptr, ptr_fmt, sptr_list, lenth + 1);
    if (size)
        *size = lenth;

    return ptr;
}

/*!
 * @brief   lv_vasprintk_safe
 * @param   ptr_fmt, size
 * @retval  none
 * @note    String format conversion
 */
kchar_t *lv_vasprintk_safe(const kchar_t *ptr_fmt, kusize_t *size, kubyte_t *ptr_lv, va_list sptr_list)
{
    va_list sptr_copy;
    kchar_t *ptr;
    kusize_t lenth;

    if (!ptr_fmt)
        return mr_nullptr;

    va_copy(sptr_copy, sptr_list);
    lenth = do_fmt_convert(mr_nullptr, mr_nullptr, ptr_fmt, sptr_copy, (kusize_t)(~0));
    va_end(sptr_copy);

    ptr = kmalloc(lenth + 1, GFP_KERNEL);
    if (!isValid(ptr))
        return mr_nullptr;

    do_fmt_convert(ptr, ptr_lv, ptr_fmt, sptr_list, lenth + 1);
    if (size)
        *size = lenth;

    return ptr;
}

/*!
 * @brief   vasprintk
 * @param   ptr_buf, ptr_fmt
 * @retval  none
 * @note    String format conversion
 */
kint32_t vasprintk(void *ptr_buf, const kchar_t *ptr_fmt, va_list sptr_list)
{
    va_list sptr_copy;
    kusize_t size;

    va_copy(sptr_copy, sptr_list);
    size = do_fmt_convert(ptr_buf, mr_nullptr, ptr_fmt, sptr_copy, (kusize_t)(~0));
    va_end(sptr_copy);

    return size;
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
    size = do_fmt_convert(ptr_buf, mr_nullptr, ptr_fmt, ptr_list, (kusize_t)(~0));
    va_end(ptr_list);

    return size;
}

/*!
 * @brief   sprintk
 * @param   ptr_buf, ptr_fmt
 * @retval  none
 * @note    String format conversion
 */
kint32_t sprintk_limit(void *ptr_buf, kusize_t limit_len, const kchar_t *ptr_fmt, ...)
{
    va_list ptr_list;
    kusize_t size;

    if (limit_len == 0)
        return -ER_LACK;

    va_start(ptr_list, ptr_fmt);
    size = do_fmt_convert(ptr_buf, mr_nullptr, ptr_fmt, ptr_list, limit_len);
    va_end(ptr_list);

    return size;
}

/*!
 * @brief   sprintk_safe
 * @param   ptr_buf, ptr_fmt
 * @retval  none
 * @note    String format conversion
 */
kchar_t *sprintk_safe(const kchar_t *ptr_fmt, ...)
{
    va_list sptr_list, sptr_copy;
    kchar_t *ptr;
    kusize_t lenth;

    if (!ptr_fmt)
        return mr_nullptr;

    va_start(sptr_list, ptr_fmt);

    va_copy(sptr_copy, sptr_list);
    lenth = do_fmt_convert(mr_nullptr, mr_nullptr, ptr_fmt, sptr_copy, (kusize_t)(~0));
    va_end(sptr_copy);

    ptr = kmalloc(lenth + 1, GFP_KERNEL);
    if (!isValid(ptr)) 
    {
        va_end(sptr_list);
        return mr_nullptr;
    }

    do_fmt_convert(ptr, mr_nullptr, ptr_fmt, sptr_list, lenth + 1);
    va_end(sptr_list);

    return ptr;
}

/*!
 * @brief   free memory from "sprintk_safe"
 * @param   ptr
 * @retval  none
 * @note    none
 */
void fmt_free(kchar_t *ptr)
{
    if (ptr)
        kfree(ptr);
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

#endif

/*!
 * @brief   kstrlen
 * @param   none
 * @retval  none
 * @note    return string lenth
 */
kuint32_t kstrlen(const kchar_t *__s)
{
    if (!__s)
        return 0;

    return (kuint32_t)get_string_lenth(__s);
}

/*!
 * @brief   kstrcpy
 * @param   none
 * @retval  none
 * @note    copy src to dest
 */
kchar_t *kstrcpy(kchar_t *__dest, const kchar_t *__src)
{
    if (!__dest || !__src)
        return mr_nullptr;

    return do_string_copy(__dest, __src);
}

/*!
 * @brief   kstrncpy
 * @param   none
 * @retval  none
 * @note    copy src to dest
 */
kchar_t *kstrncpy(kchar_t *__dest, const kchar_t *__src, kusize_t __n)
{
    if (!__dest || !__src)
        return mr_nullptr;

    return do_string_n_copy(__dest, __src, __n);
}

/*!
 * @brief   strlcpy
 * @param   none
 * @retval  none
 * @note    copy src to dest
 */
kusize_t kstrlcpy(kchar_t *__dest, const kchar_t *__src, kusize_t __n)
{
    if (!__dest || !__src)
        return 0;
    
    return do_string_n_copy_safe(__dest, __src, __n);
}

/*!
 * @brief   kstrncpyr
 * @param   none
 * @retval  none
 * @note    copy src to dest (reverse)
 */
kchar_t *kstrncpyr(kchar_t *__dest, const kchar_t *__src, kusize_t __n)
{
    if (!__dest || !__src)
        return mr_nullptr;
    
    return do_string_n_copy_rev(__dest, __src, __n);
}

/*!
 * @brief   kstrcmp
 * @param   none
 * @retval  none
 * @note    compare s1 and s2
 */
kint32_t kstrcmp(const kchar_t *__s1, const kchar_t *__s2)
{
    if (!__s1 || !__s2)
        return -2;
    
    return do_string_compare(__s1, __s2);
}

/*!
 * @brief   kstrncmp
 * @param   none
 * @retval  none
 * @note    compare s1 and s2
 */
kint32_t kstrncmp(const kchar_t *__s1, const kchar_t *__s2, kusize_t __n)
{
    if (!__s1 || !__s2)
        return -2;
    
    return do_string_n_compare(__s1, __s2, __n);
}

/*!
 * @brief   kstrchr
 * @param   none
 * @retval  none
 * @note    locate where the first "ch" appears
 */
kchar_t *kstrchr(const kchar_t *__s1, kchar_t ch)
{
    if (!__s1)
        return mr_nullptr;

    return seek_char_in_string(__s1, ch);
}

/*!
 * @brief   kstrnchr
 * @param   none
 * @retval  none
 * @note    locate where the n "ch" appears
 */
kchar_t *kstrnchr(const kchar_t *__s1, kchar_t ch, kint32_t n)
{
    if (!__s1)
        return mr_nullptr;
    
    return seek_n_char_in_string(__s1, ch, n);
}

/*!
 * @brief   kstrcut
 * @param   none
 * @retval  none
 * @note    locate where the index "ch" appears
 */
kchar_t *kstrcut(const kchar_t *__s1, kuint32_t index)
{
    if (!__s1)
        return mr_nullptr;
    
    return seek_char_by_pos(__s1, index);
}

/* end of file */
