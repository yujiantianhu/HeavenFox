/*
 * Baisc Types Defines
 *
 * File Name:   basic_types.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __BASIC_TYPES_H
#define __BASIC_TYPES_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The defines */
#if !defined(false)
#define false                   (0)
#elif (0 != false)
#undef false
#define false                   (0)
#endif

#if !defined(true)
#define true                    (1)
#elif (1 != true)
#undef true
#define true                    (1)
#endif

#define mr_false                (false)
#define mr_true                 (true)
#define mr_boolen(x)            (!!(x))

#ifndef NULL
#define NULL                    ((void *)0)
#endif

#ifdef __cplusplus
#define mr_nullptr              nullptr
#else
#define mr_nullptr              NULL
#endif

#define IT_FALSE                (mr_false)
#define IT_TRUE                 (mr_true)
#define IT_BOOLEN(x)            (mr_boolen(x))
#define IT_NULL                 (mr_nullptr)

typedef enum nrt_bool
{
    NR_FALSE = false,
    NR_TRUE  = true

} nrt_bool_t;

#if !defined(__cplusplus) && !defined(bool)
#define bool                    _Bool
#endif

typedef bool                    kbool_t;

#define mr_to_kbool(x)          ((x) ? NR_TRUE : NR_FALSE)

/*!< basic types */
typedef signed          char    kint8_t;
typedef unsigned        char    kuint8_t;
typedef                 short   kint16_t;
typedef unsigned        short   kuint16_t;
typedef                 int     kint32_t;
typedef unsigned        int     kuint32_t;
typedef          long   long    kint64_t;
typedef unsigned long   long    kuint64_t;
typedef                 float   kfloat_t;
typedef                 double  kdouble_t;

/*!< super types */
typedef                 char    kchar_t;
typedef                 char    kbyte_t;
typedef unsigned        char    kubyte_t;
typedef				    char	kbuffer_t;
typedef	unsigned 	    char	kubuffer_t;
typedef				    int		kssize_t;
typedef	unsigned 	    int		kusize_t;
typedef unsigned        long    kuaddr_t;
typedef unsigned        long    kutype_t;
typedef                 long    kstype_t;

typedef kint8_t                 s8;
typedef kuint8_t                u8;
typedef kint16_t                s16;
typedef kuint16_t               u16;
typedef kint32_t                s32;
typedef kuint32_t               u32;

typedef kusize_t                size_t;

#define ARCH_PER_SIZE           sizeof(kutype_t)
#define __RESERVED(x)           (void)(x)

#define __used		            __attribute__((used))
#define __weak                  __attribute__((weak))
#define __packed                __attribute__((packed))
#define __align(x)              __attribute__((__aligned__(x)))
#define __section(x)            __attribute__((section(x)))
#define __force                 __attribute__((force))
#define __unused                __attribute__((unused))
#define __noreturn              __attribute__((noreturn))

#if defined(__GNUC__)
#define __force_inline          __attribute__((always_inline))
#else 
#define __force_inline
#endif

#define mr_likely(x)            __builtin_expect(mr_boolen(x), true)
#define mr_unlikely(x)          __builtin_expect(mr_boolen(x), false)

#ifndef mr_likely
#define mr_likely(x)            (__builtin_constant_p(x) ? mr_boolen(x) : __branch_check__(x, true))
#endif

#ifndef mr_unlikely
#define mr_check_unlikely(x)    (__builtin_constant_p(x) ? mr_boolen(x) : __branch_check__(x, false))
#endif

#define __compiler_offsetof(a, b)   \
                                __builtin_offsetof(a, b)

#define __read_prefetch(x)      __builtin_prefetch(x, 0, 3)
#define __write_prefetch(x)     __builtin_prefetch(x, 1, 3)

#define __visible               __attribute__((externally_visible))

#ifdef __cplusplus
#define CPP_ASMLINKAGE          extern "C"
#else
#define CPP_ASMLINKAGE
#endif

#ifndef __asmlinkage
#define __asmlinkage            CPP_ASMLINKAGE
#endif

#ifndef _STDARG_H
#ifndef __va_list__
typedef __builtin_va_list       va_list;
#endif

#define va_start(v,l)	        __builtin_va_start(v,l)
#define va_end(v)	            __builtin_va_end(v)
#define va_arg(v,l)	            __builtin_va_arg(v,l)
//#if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L || defined(__GXX_EXPERIMENTAL_CXX0X__)
#define va_copy(d,s)	        __builtin_va_copy(d,s)
//#endif
#define __va_copy(d,s)	        __builtin_va_copy(d,s)

#endif /* _STDARG_H */

#ifdef __cplusplus
    }
#endif

#endif  /* __BASIC_TYPES_H */
