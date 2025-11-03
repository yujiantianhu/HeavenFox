/*
 * General Function
 *
 * File Name:   generic.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.18
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/generic.h>
#include <common/api_string.h>
#include <common/io_stream.h>
#include <common/time.h>
#include <kernel/spinlock.h>
#include <platform/base/fwk_mempool.h>

/*!< The defines */
#define RANDOM_BLOCK_SIZE                       (64)

struct common_random_state
{
    struct spin_lock sgtc_lock;

    union {
        kutype_t value;
        kuint8_t state[RANDOM_BLOCK_SIZE];
    } u;
};

/*!< The globals */
static struct common_random_state sgtc_com_random_state = 
{
    .sgtc_lock = SPIN_LOCK_INIT(),
};

/*!< API function */
/*!
 * @brief   unsigned divied: "divied / div"
 * @param   divied, div
 * @retval  none
 * @note    none
 */
kuint64_t udiv_int64(kuint64_t divied, kuint64_t div)
{
    kuint64_t count = 0;

    while (divied >= div)
    {
        divied -= div;
        count++;
    }

    return count;
}

/*!
 * @brief   signed divied: "divied / div"
 * @param   divied, div
 * @retval  none
 * @note    none
 */
kint64_t sdiv_int64(kint64_t divied, kint64_t div)
{
    kint64_t count = 0;
    kint64_t number1 = mr_abs(divied);
    kint64_t number2 = mr_abs(div);

    while (number1 >= number2)
    {
        number1 -= number2;
        count++;
    }

    return ((divied ^ div) < 0) ? -count : count;
}

/*!
 * @brief   get the remainder: "divied % div"
 * @param   divied, div
 * @retval  none
 * @note    none
 */
kuint64_t urem_int64(kuint64_t divied, kuint64_t div)
{
    while (divied >= div)
        divied -= div;

    return divied;
}

/*!
 * @brief   unsigned divied: "divied / div"， and get the remainder: "divied % div"
 * @param   rem_val: remainder
 * @param   divied, div
 * @retval  none
 * @note    none
 */
kuint64_t udiv_rem_int64(kuint64_t *rem_val, kuint64_t divied, kuint64_t div)
{
    kuint64_t count = 0;

    while (divied >= div)
    {
        divied -= div;
        count++;
    }

    if (rem_val)
        *rem_val = divied;

    return count;
}

/*!
 * @brief   convert number to hex, and save the result to buf
 * @param   buf, number
 * @param   mode: -1, exinlcude '0x'; 0, 0xabc; 1, 0XABC
 * @retval  none
 * @note    none
 */
kutype_t dec_to_hex(kchar_t *buf, kutype_t number, kint32_t mode)
{
    kchar_t temp[(sizeof(kutype_t) << 1) + 4];
    kchar_t result = 0;
    kint16_t count = 0, idx;
    kuint32_t offset = (mode != (-1)) ? 2 : 0;

    do {
        result = number - ((number >> 4) << 4);
        number = number >> 4;

        if (result >= 10)
            temp[count] = result - 10 + ((mode == 1) ? 'A' : 'a');
        else
            temp[count] = result + '0';

        count++;
        
    } while (number);

    if (buf) 
    {
        if (offset == 2) 
        {
            *buf = '0';
            *(buf + 1) = (mode == 1) ? 'X' : 'x';
        }

        for (idx = 0; idx < count; idx++)
            *(buf + idx + offset) = temp[count - idx - 1];
    }

    return (count + offset);
}

/*!
 * @brief   convert number to binary, and save the result to buf
 * @param   buf, number
 * @param   mode: -1, exinlcude '0b'; 0, 0b110; 1, 0B110
 * @retval  none
 * @note    none
 */
kutype_t dec_to_binary(kchar_t *buf, kutype_t number, kint32_t mode)
{
    kchar_t temp[(sizeof(kutype_t) << 3) + 4];
    kchar_t result = 0;
    kint16_t count = 0, idx;
    kuint32_t offset = (mode != (-1)) ? 2 : 0;

    do {
        result = number - ((number >> 1) << 1);
        number = number >> 1;

        temp[count++] = result + '0';
        
    } while (number);

    if (buf) 
    {
        if (offset == 2) 
        {
            *buf = '0';
            *(buf + 1) = 'b';
        }

        for (idx = 0; idx < count; idx++)
            *(buf + idx + offset) = temp[count - idx - 1];
    }

    return (count + offset);
}

/*!
 * @brief   convert string to number
 * @param   str
 * @param   result
 * @retval  errno
 * @note    such as: "160"/"0xA0"/"0xa0"/"0b10100000" ===> 160
 */
kint32_t ascii_to_dec(const kchar_t *str, kint32_t *result)
{
    kchar_t *p;
    kint32_t val = 0, count = 0, bak;
    kchar_t type = -1, dir = 0;

    p = (kchar_t *)str;
    if (p)
    {
        if (*p == '-')
        {
            p++;
            dir = 1;
        }

        if ((*(p) == '0') && 
            ((*(p + 1) == 'X') || (*(p + 1) == 'x')))
            type = 16;
        else if ((*(p) == '0') && 
            ((*(p + 1) == 'B') || (*(p + 1) == 'b')))
            type = 2;
        else
            type = 10;

        if (type != 10)
        {
            p += 2;
            if (!(*p))
                goto fail;
        }
    }

    for (; p && (*p != '\0'); p++) 
    {
        switch (type) 
        {
            case 2:
                if ((*p != '0') && (*p != '1'))
                    goto fail;

                val = (val << 1) + (*p) - '0';

                /*!< Over */
                if ((count++) >= 32)
                    goto fail;
                break;

            case 10:
                if ((*p < '0') || (*p > '9'))
                    goto fail;

                bak = val;
                val = (val << 1) + (val << 3) + (*p) - '0';

                /*!< Over */
                if (((count++) >= 10) || (bak > val))
                    goto fail;
                
                break;

            case 16:
                if ((*p >= '0') && (*p <= '9'))
                    val = (val << 4) + (*p) - '0';
                else if ((*p >= 'a') && (*p <= 'z'))
                    val = (val << 4) + (*p) - 'a' + 10;
                else if ((*p >= 'A') && (*p <= 'Z'))
                    val = (val << 4) + (*p) - 'A' + 10;
                else
                    goto fail;

                /*!< Over */
                if ((count++) >= 8)
                    goto fail;

                break;

            default: goto fail;
        }
    }

    *result = dir ? (-val) : val;
    return ER_NORMAL;

fail:
    *result = -1;
    print_err("%s: input argument error!\r\n", __FUNCTION__);
    
    return -ER_INVALID;
}

/*!
 * @brief   build random value
 * @param   none
 * @retval  value
 * @note    none
 */
kutype_t random_val(void)
{
    struct common_random_state *sptr_rand;
    kutype_t value;

    sptr_rand = &sgtc_com_random_state;

    spin_lock_irqsave(&sptr_rand->sgtc_lock);
    value = (kutype_t)(((jiffies % 32767) * (sptr_rand->u.value + 345977126UL)) ^ 4298547119UL);
    sptr_rand->u.value = value;
    spin_unlock_irqrestore(&sptr_rand->sgtc_lock);

    return value;
}

/*!
 * @brief   get the number of valid bits (bit = 1)
 * @param   number
 * @param   prompt_valids
 * @retval  none
 * @note    for example: number = 0x1010 (16bits), valid bits = 2; zero bits = 30
 */
kuint16_t parse_valid_u32_bits(kuint32_t number)
{
    kint16_t i;
    kuint16_t count = 0;

    /*!< get the numer of zero-bit from high to low */
    for (i = 31; i >= 0; i--) 
    {
        /*!< The first bit that is '1' */
        if (number & (1UL << i))
            break;

        count++;
    }

    return (32 - count);
}

/*!
 * @brief   add '/' to the end of path, for example, path is "/dev", then fix to "/dev/"
 * @param   path, it can be "xxx/" or "xxx", for "xxx/", nothing will be done; for "xxx", add '/' to the end of it
 * @param   len: length of full_path
 * @param   flag: mark if full_path is from memory pool (true or false)
 * @retval  full path, will contain '/' at the end of full path
 * @note    none
 */
kchar_t *path_symbol_add(const kchar_t *path, kusize_t *len, kbool_t *flag)
{
    kchar_t *full_path = (kchar_t *)path;
    kusize_t length = kstrlen(path);

    if (length && (*(path + length - 1) != '/'))
    {
        full_path = kmalloc(length + 2, GFP_KERNEL);
        if (!isValid(full_path))
            return mr_nullptr;

        kstrncpy(full_path, path, length);
        *(full_path + length) = '/';
        *(full_path + length + 1) = '\0';

        *flag = true;
    }
    else
    {
        *flag = false;
    }

    if (len)
        *len = length + 1;

    return full_path;
}

/*!
 * @brief   free full_path if it is from memory pool
 * @param   full_path
 * @param   flag: guarantee to be from memory pool (true or false)
 * @retval  none
 * @note    none
 */
void path_symbol_release(kchar_t *full_path, kbool_t flag)
{
    if (full_path && flag)
        kfree(full_path);
}

/* end of file */
