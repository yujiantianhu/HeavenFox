/*
 * Gloabl Definition For R/W
 *
 * File Name:   io_stream.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.28
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __IO_STREAM_H
#define __IO_STREAM_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/generic.h>
#include <common/list_types.h>
#include <configs/configs.h>

/*!< The defines */
#define __IO                                            volatile
#define __IOW                                           volatile
#define __IOR                                           volatile const
#define __I                                             volatile const

/*!< defines a general union format */
#define UNION_SRT_FORMAT_DECLARE(urt_name, srt_name)  \
typedef union urt_name  \
{   \
    kuint32_t all;  \
    srt_name  bit;  \
} urt_name##_t;

#define mr_trans_urt_data(urt)                     	    ((urt)->all)
#define mr_reset_urt_data(urt)                			((urt)->all = 0U)
#define mr_write_urt_data(urt, data)          			((urt)->all = (data))
#define mr_trans_urt_bits(urt, member)  			    ((urt)->bit.member)
#define mr_write_urt_bits(urt, member, data)  			((urt)->bit.member = (data))

/*!< for a word */
#define mr_writel(data, addr)                           do { *((volatile kuint32_t *)(addr)) = (kuint32_t)(data); mr_dmb(); } while (0)
#define mr_readl(addr)                                  ( (*((volatile kuint32_t *)(addr))))
#define mr_resetl(addr)                                 do { *((volatile kuint32_t *)(addr)) = 0U; mr_dmb(); } while (0)
#define mr_setbitl(bit, addr)                           do { *((volatile kuint32_t *)(addr)) |= (kuint32_t)(bit); mr_dmb(); } while (0)
#define mr_clrbitl(bit, addr)                           do { *((volatile kuint32_t *)(addr)) &= ~((kuint32_t)(bit)); mr_dmb(); } while (0)
#define mr_getbitl(bit, addr)                           ( (*((volatile kuint32_t *)(addr)) & ((kuint32_t)(bit))))
#define mr_getbit_u32(mask, nr, addr)                   ( (mr_getbitl(mask, addr) >> (nr)))
#define mr_setfieldl(bit, mask, addr)  \
do {   \
    mr_clrbitl(mask, addr);    \
    mr_setbitl(bit, addr); \
} while (0)

/*!< for a half word */
#define mr_writew(data, addr)                           do { *((volatile kuint16_t *)(addr)) = (kuint16_t)(data); mr_dmb(); } while (0)
#define mr_readw(addr)                                  ( (*((volatile kuint16_t *)(addr))))
#define mr_resetw(addr)                                 do { *((volatile kuint16_t *)(addr)) = 0U; mr_dmb(); } while (0)
#define mr_setbitw(bit, addr)                           do { *((volatile kuint16_t *)(addr)) |= (kuint16_t)(bit); mr_dmb(); } while (0)
#define mr_clrbitw(bit, addr)                           do { *((volatile kuint16_t *)(addr)) &= ~((kuint16_t)(bit)); mr_dmb(); } while (0)
#define mr_getbitw(bit, addr)                           ( (*((volatile kuint16_t *)(addr)) & ((kuint16_t)(bit))))
#define mr_getbit_u16(mask, nr, addr)                   ( (mr_getbitw(mask, addr) >> (nr)))
#define mr_setfieldw(bit, mask, addr)  \
{   \
    mr_clrbitw(mask, addr);    \
    mr_setbitw(bit, addr); \
}

/*!< for a byte */
#define mr_writeb(data, addr)                           do { *((volatile kuint8_t *)(addr)) = (kuint8_t)(data); mr_dmb(); } while (0)
#define mr_readb(addr)                                  ( (*((volatile kuint8_t *)(addr))))
#define mr_resetb(addr)                                 do { *((volatile kuint8_t *)(addr)) = 0U; mr_dmb(); } while (0)
#define mr_setbitb(bit, addr)                           do { *((volatile kuint8_t *)(addr)) |= (kuint8_t)(bit); mr_dmb(); } while (0)
#define mr_clrbitb(bit, addr)                           do { *((volatile kuint8_t *)(addr)) &= ~((kuint8_t)(bit)); mr_dmb(); } while (0)
#define mr_getbitb(bit, addr)                           ( (*((volatile kuint8_t *)(addr)) & ((kuint8_t)(bit))))
#define mr_getbit_u8(mask, nr, addr)                    ( (mr_getbitb(mask, addr) >> (nr)))
#define mr_setfieldb(bit, mask, addr)  \
{   \
    mr_clrbitb(mask, addr);    \
    mr_setbitb(bit, addr); \
}

/*!
 * @note mr_isBitSetl: like mr_getbitl(0x01 | 0x02, addr) = 0x02, it is not equaling to (0x01 | 0x02), and (!=0)
 * @note mr_isBitResetl: mr_getbitl(0x01 | 0x02, addr) must equal to zero.
 */
#define mr_isBitSetl(bit, addr)                         ( (bit) == mr_getbitl(bit, addr))
#define mr_isBitResetl(bit, addr)                       ( 0U == mr_getbitl(bit, addr))
#define mr_isBitSetw(bit, addr)                         ( (bit) == mr_getbitw(bit, addr))
#define mr_isBitResetw(bit, addr)                       ( 0U == mr_getbitw(bit, addr))
#define mr_isBitSetb(bit, addr)                         ( (bit) == mr_getbitb(bit, addr))
#define mr_isBitResetb(bit, addr)                       ( 0U == mr_getbitb(bit, addr))

/*!< set bit to 32-bit array */
#define mr_setbit_towords(integer, addr)	\
do {   \
    *((volatile kuint32_t *)(addr) + mr_word_offset(integer)) |= mr_bit(mr_bit_offset(integer)); \
    mr_dmb();   \
} while (0)

/*!< clear bit to 32-bit array */
#define mr_clrbit_towords(integer, addr)	\
do {   \
    *((volatile kuint32_t *)(addr) + mr_word_offset(integer)) &= ~mr_bit(mr_bit_offset(integer));    \
    mr_dmb();   \
} while (0)

/*!< git bit from 32-bit array */
#define mr_getbit_fromwords(integer, addr)	\
    (*((volatile kuint32_t *)(addr) + mr_word_offset(integer)) & mr_bit(mr_bit_offset(integer)))

#define mr_isBitSetWords(integer, addr)                 ( mr_bit(mr_bit_offset(integer)) == mr_getbit_fromwords(bit, addr))
#define mr_isBitResetWords(integer, addr)               ( 0U == mr_getbit_fromwords(bit, addr))

/*!< set mask to 32-bit array */
#define mr_setmask_towords(integer, mask, addr)	\
do {   \
    const kuint32_t _integer = (integer);   \
    *((volatile kuint32_t *)(addr) + mr_word_offset(_integer)) |= mr_bit_nr(mask, mr_bit_offset(_integer)); \
    mr_dmb();   \
} while (0)

/*!< clear mask to 32-bit array */
#define mr_clrmask_towords(integer, mask, addr)	\
do {   \
    const kuint32_t _integer = (integer);   \
    *((volatile kuint32_t *)(addr) + mr_word_offset(_integer)) &= ~mr_bit_nr(mask, mr_bit_offset(_integer));    \
    mr_dmb();   \
} while (0)

/*!< git mask from 32-bit array (shift to lowest bit) */
#define mr_getmask_fromwords(integer, mask, addr)	\
({  \
    const kuint32_t _integer = (integer);   \
    ((*((volatile kuint32_t *)(addr) + mr_word_offset(_integer)) >> mr_bit_offset(_integer)) & (mask));   \
})

/*!< print levels */
#define PRINT_LEVEL_SOH                                 "\001"
#define PRINT_LEVEL_EMERG                               PRINT_LEVEL_SOH "0"
#define PRINT_LEVEL_ALERT                               PRINT_LEVEL_SOH "1"
#define PRINT_LEVEL_CRIT                                PRINT_LEVEL_SOH "2"
#define PRINT_LEVEL_ERR                                 PRINT_LEVEL_SOH "3"
#define PRINT_LEVEL_WARNING                             PRINT_LEVEL_SOH "4"
#define PRINT_LEVEL_NOTICE                              PRINT_LEVEL_SOH "5"
#define PRINT_LEVEL_INFO                                PRINT_LEVEL_SOH "6"
#define PRINT_LEVEL_DEBUG                               PRINT_LEVEL_SOH "7"

/*!< Input/Output Stream Interface */
#define IO_STREAM_ASYNC                                 (0x00000001U)
#define IO_STREAM_TXING                                 (0x00000002U)
#define IO_STREAM_KERNEL                                (0x00000004U)

struct io_stream_dev
{
    kchar_t name[32];
    kbool_t is_opened;
    struct list_head sgtc_link;

    void (*_putc)(const kubyte_t ch);
    void (*_putstr)(const kubyte_t *msgs, kusize_t size);
    kubyte_t (*_getc)(kubyte_t *ch);
    kssize_t (*_getstr)(kubyte_t *msgs, kusize_t size);
};

/*!< The globals */
extern kuint32_t g_io_stream_flags;

#define IO_STREAM_INTR_ENTER(flags) \
    do {    \
        (flags) = g_io_stream_flags;  \
        g_io_stream_flags &= ~IO_STREAM_ASYNC;  \
    } while (0)

#define IO_STREAM_INTR_LEAVE(flags) \
    do {    \
        g_io_stream_flags = (flags);  \
    } while (0)

/*!< The functions */
extern void iostream_init(void);
extern kint32_t register_io_stream(struct io_stream_dev *sptr_stream);
extern void unregister_io_stream(struct io_stream_dev *sptr_stream);
extern void io_stream_enable(const kchar_t *name);
extern void io_stream_disable(const kchar_t *name);
extern struct io_stream_dev *find_io_stream_dev(const kchar_t *name);

/*!< Logs buffer */
extern struct pq_buffer *io_stream_logs_ptr(void);
extern void io_stream_logs_lock(void);
extern void io_stream_logs_unlock(void);

/*!< Hardware transfer */
extern void io_putc(const kubyte_t ch);
extern void io_putstr(const kubyte_t *msgs, kusize_t size);
extern kubyte_t io_getc(kubyte_t *ch);
extern kssize_t io_getstr(kubyte_t *msgs, kusize_t size);
extern void io_putstr_async(const kubyte_t *msgs, kusize_t size);
extern kssize_t io_stream_logs_extract(void *buffer, kusize_t size);
extern void io_stream_logs_print(void *__temp_buffer, kusize_t __temp_size);

extern void printk(const kchar_t *ptr_fmt, ...);
extern void kprintf(const kchar_t *ptr_fmt, ...);
extern void print_sync(const kchar_t *ptr_fmt, ...);

#define print_err(fmt, ...)                             printk(PRINT_LEVEL_ERR fmt, ##__VA_ARGS__)
#define print_warn(fmt, ...)                            printk(PRINT_LEVEL_WARNING fmt, ##__VA_ARGS__)
#define print_info(fmt, ...)                            printk(PRINT_LEVEL_INFO fmt, ##__VA_ARGS__)
#define print_debug(fmt, ...)                           printk(PRINT_LEVEL_DEBUG fmt, ##__VA_ARGS__)

#define ERR_OUT(flag, fmt, ...)   \
    do {    \
        if (!!(flag))    \
            print_err("%s: %d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);   \
    } while (0)

#define WARN_OUT(flag, fmt, ...)    \
    do {    \
        if (!!(flag))    \
            print_warn("%s: %d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);   \
    } while (0)

#define INFO_OUT(flag, fmt, ...)    \
    do {    \
        if (!!(flag))    \
            print_info("%s: %d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);   \
    } while (0)

#define DEBUG_OUT(flag, fmt, ...)   \
    do {    \
        if (!!(flag))    \
            print_debug("%s: %d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);   \
    } while (0)

/*!< Bitmap */
extern kint32_t bitmap_find_first_zero_bit(void *bitmap, kuint32_t start, kusize_t total_bits);
extern kint32_t bitmap_find_first_valid_bit(void *bitmap, kuint32_t start, kusize_t total_bits);
extern kint32_t bitmap_find_nr_zero_bit(void *bitmap, kuint32_t start, kusize_t total_bits, kuint32_t nr);
extern kint32_t bitmap_find_nr_valid_bit(void *bitmap, kuint32_t start, kusize_t total_bits, kuint32_t nr);
extern void bitmap_set_nr_bit_zero(void *bitmap, kuint32_t start, kusize_t total_bits, kuint32_t nr);
extern void bitmap_set_nr_bit_valid(void *bitmap, kuint32_t start, kusize_t total_bits, kuint32_t nr);

/*!< API function */
/*!
 * @brief   api_bit_mask
 * @param   index
 * @retval  bit
 * @note    tranform: val to bit; for example, number 4 is equaled to bit 4
 */
__force_inline static inline kuint32_t api_bit_mask(kuint8_t index)
{
	kuint32_t result;

	__asm__ __volatile__ (
		" mov %0, #1		\n\t"
		" and %1, #0x1f		\n\t"
		" lsl %0, %0, %1	\n\t"
		: "=&r"(result)
		: "r"(index)
		: "cc"
	);

    return result;
}

/*!
 * @brief   set bitmap bit
 * @param   nr, addr
 * @retval  none
 * @note    set bit to array
 */
__force_inline static inline void bitmap_setl(kuint32_t nr, kuint32_t mask, kuint32_t *addr)
{
    kuint32_t *p_addr = addr + (nr >> 5);
    (*p_addr) |= (mask << (nr & 0x1f));
}

/*!
 * @brief   clrbit_to_words
 * @param   nr, addr
 * @retval  none
 * @note    reset bit to array
 */
__force_inline static inline void bitmap_clrl(kuint32_t nr, kuint32_t mask, kuint32_t *addr)
{
    kuint32_t *p_addr = addr + (nr >> 5);
    (*p_addr) &= ~(mask << (nr & 0x1f));
}

/*!
 * @brief   getbit_from_words
 * @param   lenth: the number of bits
 * @retval  none
 * @note    get bit from array
 */
__force_inline static inline kbool_t bitmap_getl(kuint32_t nr, kuint32_t *addr)
{
    kuint32_t *p_addr = addr + (nr >> 5);
    return !!((*p_addr) & (1 << (nr & 0x1f)));
}

/*!
 * @brief   io_writel
 * @param   none
 * @retval  none
 * @note    write SoC register
 */
__force_inline static inline void io_writel(kuaddr_t addr, kuint32_t val)
{
    *((volatile kuaddr_t *)addr) = val;
}

/*!
 * @brief   io_readl
 * @param   none
 * @retval  none
 * @note    read SoC register
 */
__force_inline static inline kuint32_t io_readl(kuaddr_t addr, kuint32_t *val)
{
    if (val)
        *val = *((kuaddr_t *)addr);

    return *((kuaddr_t *)addr);
}

/*!
 * @brief   io_setbit
 * @param   none
 * @retval  none
 * @note    write SoC register
 */
__force_inline static inline void io_setbit(kuaddr_t addr, kuint32_t val)
{
    *((volatile kuaddr_t *)addr) |= val;
}

/*!
 * @brief   io_clrbit
 * @param   none
 * @retval  none
 * @note    write SoC register
 */
__force_inline static inline void io_clrbit(kuaddr_t addr, kuint32_t val)
{
    *((volatile kuaddr_t *)addr) &= ~val;
}

#ifdef __cplusplus
    }
#endif

#endif /* __IO_STREAM_H */
