/*
 * IO Stream General Function
 *
 * File Name:   io_stream.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.28
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <configs/configs.h>
#include <common/io_stream.h>
#include <common/api_string.h>
#include <common/mem_manage.h>
#include <common/list_types.h>
#include <common/buffer.h>
#include <kernel/kernel.h>
#include <kernel/spinlock.h>
#include <platform/fwk_mempool.h>

/*!< The globals */
static DECLARE_LIST_HEAD(sgtc_io_stream_devices);
static struct spin_lock sgtc_io_stream_lock = SPIN_LOCK_INIT();

/*!< Log msgs: 128KB */
static kuint8_t g_io_stream_buffer[128 * 1024];
static struct pq_buffer sgtc_io_stream_logs = PQ_RING_BUFFER_INIT(g_io_stream_buffer, sizeof(g_io_stream_buffer));
static struct spin_lock sgtc_io_stream_logs_lock = SPIN_LOCK_INIT();

kuint32_t g_io_stream_flags = 0;

/*!< API function */
/*!
 * @brief   get logs handler
 * @param   none
 * @retval  &sgtc_io_stream_logs
 * @note    none
 */
struct pq_buffer *io_stream_logs_ptr(void)
{
    return &sgtc_io_stream_logs;
}

/*!
 * @brief   logs buffer lock (spin lock)
 * @param   none
 * @retval  none
 * @note    spin_lock_irqsave(&sgtc_io_stream_logs_lock)
 */
void io_stream_logs_lock(void)
{
    spin_lock_irqsave(&sgtc_io_stream_logs_lock);
}

/*!
 * @brief   logs buffer unlock (spin lock)
 * @param   none
 * @retval  none
 * @note    spin_unlock_irqrestore(&sgtc_io_stream_logs_lock)
 */
void io_stream_logs_unlock(void)
{
    spin_unlock_irqrestore(&sgtc_io_stream_logs_lock);
}

/*!
 * @brief   io_putc
 * @param   ch
 * @retval  none
 * @note    character output
 */
void io_putc(const kubyte_t ch)
{
    struct io_stream_dev *sptr_stream;

    foreach_list_next_entry(sptr_stream, &sgtc_io_stream_devices, sgtc_link)
    {
        if (sptr_stream->is_opened && 
            sptr_stream->_putc)
            sptr_stream->_putc(ch);
    }
}

/*!
 * @brief   io_putstr
 * @param   string
 * @retval  none
 * @note    string output
 */
void io_putstr(const kubyte_t *msgs, kusize_t size)
{
    struct io_stream_dev *sptr_stream;

    foreach_list_next_entry(sptr_stream, &sgtc_io_stream_devices, sgtc_link)
    {
        g_io_stream_flags |= IO_STREAM_TXING;

        if (sptr_stream->is_opened && 
            sptr_stream->_putstr)
            sptr_stream->_putstr(msgs, size);

        g_io_stream_flags &= ~IO_STREAM_TXING;
    }
}

/*!
 * @brief   io_getc
 * @param   ch
 * @retval  none
 * @note    character read
 */
kubyte_t io_getc(kubyte_t *ch)
{
    struct io_stream_dev *sptr_stream;
    kubyte_t ret;

    foreach_list_next_entry(sptr_stream, &sgtc_io_stream_devices, sgtc_link)
    {
        if (sptr_stream->is_opened && 
            sptr_stream->_getc)
        {
            if ((ret = sptr_stream->_getc(ch)))
                return ret;
        }
    }

    return 0;
}

/*!
 * @brief   io_getstr
 * @param   string
 * @retval  none
 * @note    string read
 */
kssize_t io_getstr(kubyte_t *msgs, kusize_t size)
{
    struct io_stream_dev *sptr_stream;
    kssize_t ret, flag = 0;

    foreach_list_next_entry(sptr_stream, &sgtc_io_stream_devices, sgtc_link)
    {
        if (sptr_stream->is_opened && 
            sptr_stream->_getstr)
        {
            ret = sptr_stream->_getstr(msgs, size);
            if (ret > 0)
                return ret;
            if (ret < 0)
                flag = ret;
        }
    }

    return flag ? (-ER_EMPTY) : 0;
}

/*!
 * @brief   register new IO
 * @param   sptr_stream
 * @retval  errno
 * @note    none
 */
kint32_t register_io_stream(struct io_stream_dev *sptr_stream)
{
    if (!sptr_stream)
        return -ER_NODEV;

    if (!(*sptr_stream->name) || 
        !mr_list_empty(&sptr_stream->sgtc_link))
        return -ER_INVALID;

    spin_lock_irqsave(&sgtc_io_stream_lock);
    list_head_add_tail(&sgtc_io_stream_devices, &sptr_stream->sgtc_link);
    spin_unlock_irqrestore(&sgtc_io_stream_lock);

    return ER_NORMAL;
}

/*!
 * @brief   unregister old IO
 * @param   sptr_stream
 * @retval  none
 * @note    none
 */
void unregister_io_stream(struct io_stream_dev *sptr_stream)
{
    if (!sptr_stream)
        return;

    spin_lock_irqsave(&sgtc_io_stream_lock);
    list_head_del(&sptr_stream->sgtc_link);
    spin_unlock_irqrestore(&sgtc_io_stream_lock);
}

/*!
 * @brief   enable io_stream device
 * @param   name
 * @retval  none
 * @note    none
 */
void io_stream_enable(const kchar_t *name)
{
    struct io_stream_dev *sptr_stream;

    foreach_list_next_entry(sptr_stream, &sgtc_io_stream_devices, sgtc_link)
    {
        if (!kstrcmp(sptr_stream->name, name))
        {
            sptr_stream->is_opened = true;
            return;
        }
    }
}

/*!
 * @brief   disable io_stream device
 * @param   name
 * @retval  none
 * @note    none
 */
void io_stream_disable(const kchar_t *name)
{
    struct io_stream_dev *sptr_stream;

    foreach_list_next_entry(sptr_stream, &sgtc_io_stream_devices, sgtc_link)
    {
        if (!kstrcmp(sptr_stream->name, name))
        {
            sptr_stream->is_opened = false;
            return;
        }
    }
}

/*!
 * @brief   find io_stream device
 * @param   name
 * @retval  sptr_stream
 * @note    none
 */
struct io_stream_dev *find_io_stream_dev(const kchar_t *name)
{
    struct io_stream_dev *sptr_stream;

    foreach_list_next_entry(sptr_stream, &sgtc_io_stream_devices, sgtc_link)
    {
        if (!kstrcmp(sptr_stream->name, name))
            return sptr_stream;
    }

    return mr_nullptr;
}

/*!
 * @brief   Save msgs to buffer, not output right away
 * @param   msgs
 * @retval  none
 * @note    kthread task will put string
 */
void io_putstr_async(const kubyte_t *msgs, kusize_t size)
{
    kbool_t is_prohibit;
    kssize_t ret = -1;

    /*!< In exception ? */
    is_prohibit = g_interrupt_flags ? IS_IN_EXCEPTION() : false;

    if ((g_io_stream_flags & IO_STREAM_ASYNC) && (!is_prohibit))
    {
        spin_lock_irqsave(&sgtc_io_stream_logs_lock);
        ret = pq_message_write(&sgtc_io_stream_logs, msgs, size);
        spin_unlock_irqrestore(&sgtc_io_stream_logs_lock);
    }

    /*!< Not async, or message write fail, re-send message by hardware directlly */
    if (ret < 0)
        io_putstr(msgs, size);
}

/*!
 * @brief   Extract message
 * @param   buffer
 * @retval  none
 * @note    size can be larger than sgtc_io_stream_logs.buffer, then extract all
 */
kssize_t io_stream_logs_extract(void *buffer, kusize_t size)
{
    kssize_t ret;

    if (!(g_io_stream_flags & IO_STREAM_ASYNC))
        return -ER_PERMIT;

    spin_lock_irqsave(&sgtc_io_stream_logs_lock);
    ret = pq_message_read(&sgtc_io_stream_logs, buffer, size);
    spin_unlock_irqrestore(&sgtc_io_stream_logs_lock);

    return ret;
}

/*!
 * @brief   kprintf
 * @param   __temp_buffer: suggest 4KB
 * @retval  none
 * @note    string output (from sgtc_io_stream_logs.buffer)
 */
void io_stream_logs_print(void *__temp_buffer, kusize_t __temp_size)
{
    /*!< 4KB */
//  kubyte_t log_buffer[4096];
    kssize_t size;
    struct pq_message *sptr_msg;

    if (!__temp_buffer || !__temp_size)
        return;
    if (!mr_is_aligned((kuaddr_t)__temp_buffer, ARCH_PER_SIZE))
        return;

    sptr_msg = (struct pq_message *)__temp_buffer;
    size = io_stream_logs_extract(__temp_buffer, __temp_size);
    
    while (size > 0) 
    {
        /*!< Put every string one by one */
        io_putstr(sptr_msg->data, sptr_msg->len);

        size -= (sptr_msg->len + sizeof(sptr_msg->len));
        sptr_msg = (void *)sptr_msg + sptr_msg->len + sizeof(sptr_msg->len);
    }
}

/*!
 * @brief   kprintf
 * @param   ptr_fmt
 * @retval  none
 * @note    perhaps sleep, because it calls "kmalloc" to save fmt, don't use in IRQ_handler!!!
 */
void kprintf(const kchar_t *ptr_fmt, ...)
{
#if defined(CONFIG_PRINT_LEVEL)
    va_list ptr_list;
    kubyte_t *ptr_buf, level[2] = {};
    kusize_t size;
//  kuint32_t i;

    va_start(ptr_list, ptr_fmt);
    ptr_buf = (kubyte_t *)lv_vasprintk_safe(ptr_fmt, &size, level, ptr_list);
    va_end(ptr_list);

    if (!isValid(ptr_buf))
        return;

    /*!< if level is not set, default PRINT_LEVEL_WARNING */
    if (*(PRINT_LEVEL_SOH) != *level)
        memcpy(level, PRINT_LEVEL_WARNING, sizeof(level));

    if (*(level + 1) > *((kubyte_t *)(CONFIG_PRINT_LEVEL)))
        return;

//	for (i = 0; i < size; i++)
//		io_putc(*(ptr_buf + i));

    io_putstr_async(ptr_buf, size + 1);
    kfree(ptr_buf);
#endif
}

/*!
 * @brief   printk
 * @param   ptr_fmt
 * @retval  none
 * @note    Not sleep, but fmt must less than 1024 bytes!!!
 */
void printk(const kchar_t *ptr_fmt, ...)
{
#if defined(CONFIG_PRINT_LEVEL)
    va_list ptr_list;
    kubyte_t level[2] = {};
    kusize_t size;
    kubyte_t logs_buf[1024];
    
    va_start(ptr_list, ptr_fmt);
    size = do_fmt_convert(logs_buf, level, ptr_fmt, ptr_list, sizeof(logs_buf));
    va_end(ptr_list);

    if (!size)
        return;

    /*!< if level is not set, default PRINT_LEVEL_WARNING */
    if (*(PRINT_LEVEL_SOH) != *level)
        memcpy(level, PRINT_LEVEL_WARNING, sizeof(level));

    if (*(level + 1) > *((kubyte_t *)(CONFIG_PRINT_LEVEL)))
        return;

    io_putstr_async(logs_buf, size + 1);
#endif
}

/*!
 * @brief   find first bit that equaled to value
 * @param   bitmap: array
 * @param	start: base
 * @param	total_bit: bitmap lenth
 * @param	value: 0 or 1
 * @retval  index (bit position); -1: not found
 * @note    find bit on bitmap
 */
static kint32_t bitmap_find_first_bit(kuint8_t *bitmap, kuint32_t start, kusize_t total_bits, kbool_t value)
{
    kuint32_t index, bit_mask, area_mask;
    kuint32_t bit_per_map = sizeof(*bitmap) << 3;

    /*!< search by per 8 bits */
    for (index = start; index < total_bits; index++)
    {
        area_mask = mr_udiv(index, bit_per_map);
        bit_mask = mr_urem(index, bit_per_map);

        if ((!!(*(bitmap + area_mask) & (1 << bit_mask))) == value)
            break;
    }

    return (index >= total_bits) ? -1 : index;
}

/*!
 * @brief   request multiple bits that equaled to value
 * @param   bitmap: array
 * @param	start: base
 * @param	total_bit: bitmap lenth
 * @param	nr: count, index ~ (index + nr)
 * @param	value: 0 or 1
 * @retval  0: request successfully; > 0: the first bit failed
 * @note    request multiple consecutive bits on bitmap
 */
static kint32_t bitmap_find_nr_bit(kuint8_t *bitmap, kuint32_t start, kusize_t total_bits, kuint32_t nr, kbool_t value)
{
    kuint32_t end = start + nr;
    kint32_t index;

    if (end >= total_bits)
        return -1;
    
    /*!<
     * found (!value), return current index (the first bit that does not meet the condition)
     * indicate that no consecutive bits are avaliable, i.e request nr bits failed
     */
    index = bitmap_find_first_bit((kuint8_t *)bitmap, start, end, !value);
    return (index < 0) ? 0 : index;
}

/*!
 * @brief   set start ~ (start + nr) to value (0 or 1)
 * @param   ...
 * @retval  none
 * @note    (start + nr) < total_bits
 */
static void bitmap_set_nr_bit(kuint8_t *bitmap, kuint32_t start, kusize_t total_bits, kuint32_t nr, kbool_t value)
{
    kuint32_t end = start + nr;
    kuint32_t index, bit_mask, area_mask;
    kuint32_t bit_per_map = sizeof(*bitmap) << 3;

    if (end >= total_bits)
        return;

    for (index = start; index < end; index++)
    {
        area_mask = mr_udiv(index, bit_per_map);
        bit_mask = mr_urem(index, bit_per_map);

        if (value)
            *(bitmap + area_mask) |= (1 << bit_mask);
        else
            *(bitmap + area_mask) &= ~(1 << bit_mask);
    }
}

/*!
 * @brief   find first bit that equaled to 0
 * @param   ...
 * @retval  none
 * @note    none
 */
kint32_t bitmap_find_first_zero_bit(void *bitmap, kuint32_t start, kusize_t total_bits)
{
    return bitmap_find_first_bit((kuint8_t *)bitmap, start, total_bits, false);
}

/*!
 * @brief   find first bit that equaled to 1
 * @param   ...
 * @retval  none
 * @note    none
 */
kint32_t bitmap_find_first_valid_bit(void *bitmap, kuint32_t start, kusize_t total_bits)
{
    return bitmap_find_first_bit((kuint8_t *)bitmap, start, total_bits, true);
}

/*!
 * @brief   request multiple consecutive bits that equaled to 0
 * @param   ...
 * @retval  none
 * @note    none
 */
kint32_t bitmap_find_nr_zero_bit(void *bitmap, kuint32_t start, kusize_t total_bits, kuint32_t nr)
{
    return bitmap_find_nr_bit((kuint8_t *)bitmap, start, total_bits, nr, false);
}

/*!
 * @brief   request multiple consecutive bits that equaled to 1
 * @param   ...
 * @retval  none
 * @note    none
 */
kint32_t bitmap_find_nr_valid_bit(void *bitmap, kuint32_t start, kusize_t total_bits, kuint32_t nr)
{
    return bitmap_find_nr_bit((kuint8_t *)bitmap, start, total_bits, nr, true);
}

/*!
 * @brief   set start ~ (start + nr) to value 0
 * @param   ...
 * @retval  none
 * @note    (start + nr) < total_bits
 */
void bitmap_set_nr_bit_zero(void *bitmap, kuint32_t start, kusize_t total_bits, kuint32_t nr)
{
    bitmap_set_nr_bit((kuint8_t *)bitmap, start, total_bits, nr, false);
}

/*!
 * @brief   set start ~ (start + nr) to value 1
 * @param   ...
 * @retval  none
 * @note    (start + nr) < total_bits
 */
void bitmap_set_nr_bit_valid(void *bitmap, kuint32_t start, kusize_t total_bits, kuint32_t nr)
{
    bitmap_set_nr_bit((kuint8_t *)bitmap, start, total_bits, nr, true);
}

/* end of file */
