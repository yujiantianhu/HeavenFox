/*
 * String Function Declare
 *
 * File Name:   api_string.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.26
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __API_STRING_H
#define __API_STRING_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>

/*!< The defines */
#define mr_to_str(x)                                           #x
#define mr_to_split_str(x, y, z)                               x##y##z

typedef struct
{
    const kchar_t *str;
    kusize_t size;

} srt_string_t;

/*!< The functions */
extern kusize_t get_integrater_lenth(kuint64_t value);
extern kusize_t get_string_lenth(const void *ptr_src);
extern void do_string_split(void *ptr_dst, kuint32_t offset, const void *ptr_src);
extern kchar_t *do_string_copy(void *ptr_dst, const void *ptr_src);
extern kchar_t *do_string_n_copy(void *ptr_dst, const void *ptr_src, kuint32_t size);
extern kchar_t *do_string_n_copy_rev(void *ptr_dst, const void *ptr_src, kuint32_t size);
extern kuint32_t do_string_n_copy_safe(void *ptr_dst, const void *ptr_src, kuint32_t size);
extern kbool_t do_string_compare(const void *ptr_dst, const void *ptr_src);
extern kbool_t do_string_n_compare(const void *ptr_dst, const void *ptr_src, kuint32_t size);
extern void do_string_reverse(void *ptr_src, kuint32_t size);
extern kusize_t uint_to_str(void *ptr_dst, kuint64_t value);
extern kusize_t int_to_str(void *ptr_dst, kint64_t value);
extern kchar_t *seek_char_in_string(const void *ptr_src, kchar_t ch);
extern kchar_t *seek_char_by_pos(const void *ptr_src, kuint32_t offset);
extern kchar_t *seek_n_char_in_string(const void *ptr_src, kchar_t ch, kint32_t n);
extern kusize_t do_fmt_convert(void *ptr_buf, kubyte_t *ptr_level, const kchar_t *ptr_fmt, va_list ptr_list, kusize_t size);

extern kchar_t *vasprintk_safe(const kchar_t *ptr_fmt, kusize_t *size, va_list sptr_list);
extern kchar_t *lv_vasprintk_safe(const kchar_t *ptr_fmt, kusize_t *size, kubyte_t *ptr_lv, va_list sptr_list);
extern kint32_t vasprintk(void *ptr_buf, const kchar_t *ptr_fmt, va_list sptr_list);
extern kint32_t sprintk(void *ptr_buf, const kchar_t *ptr_fmt, ...);
extern kchar_t *sprintk_safe(const kchar_t *ptr_fmt, ...);
extern void fmt_free(kchar_t *ptr);
extern kuint32_t kstrlen(const kchar_t *__s);
extern kchar_t *kstrcpy(kchar_t *__dest, const kchar_t *__src);
extern kchar_t *kstrncpy(kchar_t *__dest, const kchar_t *__src, kusize_t __n);
extern kusize_t kstrlcpy(kchar_t *__dest, const kchar_t *__src, kusize_t __n);
extern kchar_t *kstrncpyr(kchar_t *__dest, const kchar_t *__src, kusize_t __n);
extern kint32_t kstrcmp(const kchar_t *__s1, const kchar_t *__s2);
extern kint32_t kstrncmp(const kchar_t *__s1, const kchar_t *__s2, kusize_t __n);
extern kchar_t *kstrchr(const kchar_t *__s1, kchar_t ch);
extern kchar_t *kstrnchr(const kchar_t *__s1, kchar_t ch, kint32_t n);
extern kchar_t *kstrcat(const kchar_t *__s1, kuint32_t index);

#ifdef __cplusplus
    }
#endif

#endif /* __API_STRING_H */
