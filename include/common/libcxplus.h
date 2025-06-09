/*
 * C++ Standard Lib Reload
 *
 * File Name:   libcxplus.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.01.07
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __LIBCXPLUS_H
#define __LIBCXPLUS_H

/*!< The globals */
#include <common/generic.h>
#include <platform/base/fwk_basic.h>

/*!< The defines */
#define BEGIN_NAMESPACE(name)                       namespace name {
#define END_NAMESPACE(name)                         }

/*!< ------------------------------------------------------------- */
/*!< namespace is "basic" */
BEGIN_NAMESPACE(bsc)
/*!< ------------------------------------------------------------- */

class string {

public:
    string()
        : real_size(0)
        , msgs(mr_nullptr)
        , len(0)
    {}

    string(const kchar_t *str)
        : real_size(0)
        , msgs(mr_nullptr)
        , len(0)
    {
        kssize_t size = this->strlen(str);

        if (!this->alloc_msgs(size))
            return;

        this->strcpy(this->msgs, str);
        *(this->msgs + size) = '\0';
        this->real_size = size;
    }

    string(kssize_t size)
        : real_size(0)
        , msgs(mr_nullptr)
        , len(0)
    {
        this->alloc_msgs(size);
    }

    string(string &cgtc_str)
        : real_size(0)
        , msgs(mr_nullptr)
        , len(0)
    {
        kssize_t size = cgtc_str.size();

        if (!this->alloc_msgs(size))
            return;

        this->strcpy(this->msgs, cgtc_str.c_str());
        *(this->msgs + size) = '\0';
        this->real_size = size;
    }

    ~string() 
    {
        this->free_msgs();
    }

public:
    const kchar_t *c_str(void)
    {
        return (const kchar_t *)this->msgs;
    }

    kssize_t size(void)
    {
        return this->real_size;
    }

    kssize_t length(void)
    {
        return this->len;
    }

    void operator=(const kchar_t *str)
    {
        kssize_t size = this->strlen(str);

        if (!this->msgs)
        {
            if (!this->alloc_msgs(size))
                return;
        }

        if (size >= this->len)
            return;

        this->strcpy(this->msgs, str);
        *(this->msgs + size) = '\0';
        this->real_size = size;
    }

    void operator=(string &cgtc_str)
    {
        kssize_t size = cgtc_str.size();

        if (!this->msgs)
        {
            if (!this->alloc_msgs(size))
                return;
        }

        if (size >= this->len)
            return;

        this->strcpy(this->msgs, cgtc_str.c_str());
        *(this->msgs + size) = '\0';
        this->real_size = size;
    }

    void operator+=(const kchar_t *str)
    {
        kssize_t size = this->strlen(str);

        if (this->msgs)
        {
            if ((this->real_size + size) >= this->len)
                return;

            this->strcpy(this->msgs + this->real_size, str);
            *(this->msgs + size) = '\0';
            this->real_size += size;
        }
        else
        {
            if (!this->alloc_msgs(size))
                return;

            this->strcpy(this->msgs, str);
            *(this->msgs + size) = '\0';
            this->real_size = size;
        }
    }

    void operator+=(string &cgtc_str)
    {
        kssize_t size = cgtc_str.size();

        if (this->msgs)
        {
            if ((this->real_size + size) >= this->len)
                return;

            this->strcpy(this->msgs + this->real_size, cgtc_str.c_str());
            *(this->msgs + size) = '\0';
            this->real_size += size;
        }
        else
        {
            if (!this->alloc_msgs(size))
                return;

            this->strcpy(this->msgs, cgtc_str.c_str());
            *(this->msgs + size) = '\0';
            this->real_size = size;
        }
    }

    kbool_t operator==(const kchar_t *str)
    {
        return (this->msgs && (!this->strcmp(this->msgs, str)));
    }

    kbool_t operator==(string &cgtc_str)
    {
        return (this->msgs && (this->real_size == cgtc_str.real_size) && 
                (!this->strcmp((const kchar_t *)this->msgs, cgtc_str.c_str())));
    }

    kbool_t operator!=(const kchar_t *str)
    {
        return (!this->msgs || (this->strcmp((const kchar_t *)this->msgs, str)));
    }

    kbool_t operator!=(string &cgtc_str)
    {
        return (!this->msgs || (this->real_size != cgtc_str.real_size) ||
                (this->strcmp((const kchar_t *)this->msgs, cgtc_str.c_str())));
    }

    kbool_t operator<(const kchar_t *str)
    {
        if (!this->msgs)
            return true;
        if (!str)
            return false;

        return ((-1) == this->strcmp((const kchar_t *)this->msgs, str));
    }

    kbool_t operator<(string &cgtc_str)
    {
        if (!this->msgs)
            return true;
        if (!cgtc_str.c_str())
            return false;

        return ((-1) == this->strcmp((const kchar_t *)this->msgs, cgtc_str.c_str()));
    }

    kbool_t operator<=(const kchar_t *str)
    {
        kint32_t retval;

        if (!this->msgs)
            return true;
        if (!str)
            return false;

        retval = this->strcmp((const kchar_t *)this->msgs, str);
        return !!(retval <= 0);
    }

    kbool_t operator<=(string &cgtc_str)
    {
        kint32_t retval;

        if (!this->msgs)
            return true;
        if (!cgtc_str.c_str())
            return false;

        retval = this->strcmp((const kchar_t *)this->msgs, cgtc_str.c_str());
        return !!(retval <= 0);
    }

    kbool_t operator>(const kchar_t *str)
    {
        if (!this->msgs)
            return false;
        if (!str)
            return true;

        return (1 == this->strcmp((const kchar_t *)this->msgs, str));
    }

    kbool_t operator>(string &cgtc_str)
    {
        if (!this->msgs)
            return false;
        if (!cgtc_str.c_str())
            return true;

        return (1 == this->strcmp((const kchar_t *)this->msgs, cgtc_str.c_str()));
    }

    kbool_t operator>=(const kchar_t *str)
    {
        kint32_t retval;

        if (!this->msgs)
            return false;
        if (!str)
            return true;

        retval = this->strcmp((const kchar_t *)this->msgs, str);
        return !!(retval >= 0);
    }

    kbool_t operator>=(string &cgtc_str)
    {
        kint32_t retval;

        if (!this->msgs)
            return false;
        if (!cgtc_str.c_str())
            return true;

        retval = this->strcmp((const kchar_t *)this->msgs, cgtc_str.c_str());
        return !!(retval >= 0);
    }

public:
    /*!
     * @brief   strlen
     * @param   none
     * @retval  none
     * @note    return string lenth
     */
    static kuint32_t strlen(const kchar_t *__s)
    {
        return kstrlen(__s);
    }

    /*!
     * @brief   strcpy
     * @param   none
     * @retval  none
     * @note    copy src to dest
     */
    static kchar_t *strcpy(kchar_t *__dest, const kchar_t *__src)
    {
        return kstrcpy(__dest, __src);
    }

    /*!
     * @brief   strncpy
     * @param   none
     * @retval  none
     * @note    copy src to dest
     */
    static kchar_t *strncpy(kchar_t *__dest, const kchar_t *__src, kusize_t __n)
    {
        return kstrncpy(__dest, __src, __n);
    }

    /*!
     * @brief   strlcpy
     * @param   none
     * @retval  none
     * @note    copy src to dest
     */
    static kusize_t strlcpy(kchar_t *__dest, const kchar_t *__src, kusize_t __n)
    {
        return kstrlcpy(__dest, __src, __n);
    }

    /*!
     * @brief   strncpyr
     * @param   none
     * @retval  none
     * @note    copy src to dest (reverse)
     */
    static kchar_t *strncpyr(kchar_t *__dest, const kchar_t *__src, kusize_t __n)
    {
        return kstrncpyr(__dest, __src, __n);
    }

    /*!
     * @brief   strcmp
     * @param   none
     * @retval  none
     * @note    compare s1 and s2
     */
    static kint32_t strcmp(const kchar_t *__s1, const kchar_t *__s2)
    {
        return kstrcmp(__s1, __s2);
    }

    /*!
     * @brief   strncmp
     * @param   none
     * @retval  none
     * @note    compare s1 and s2
     */
    static kint32_t strncmp(const kchar_t *__s1, const kchar_t *__s2, kusize_t __n)
    {
        return kstrncmp(__s1, __s2, __n);
    }

    /*!
     * @brief   strchr
     * @param   none
     * @retval  none
     * @note    locate where the first "ch" appears
     */
    static kchar_t *strchr(const kchar_t *__s1, kint32_t ch)
    {
        return kstrchr(__s1, (kchar_t)ch);
    }

    /*!
     * @brief   strnchr
     * @param   none
     * @retval  none
     * @note    locate where the n "ch" appears
     */
    static kchar_t *strnchr(const kchar_t *__s1, kchar_t ch, kint32_t n)
    {
        return kstrnchr(__s1, ch, n);
    }

    /*!
     * @brief   strcut
     * @param   none
     * @retval  none
     * @note    locate where the index "ch" appears
     */
    static kchar_t *strcut(const kchar_t *__s1, kuint32_t index)
    {       
        return kstrcut(__s1, index);
    }

    kssize_t real_size;

private:
    kchar_t *msgs;
    kssize_t len;

    kbool_t alloc_msgs(kssize_t size)
    {
        if (size <= 0)
            return false;

        this->len = CMP_MAX2(1024, size + 1);
        this->msgs = new kchar_t[this->len];
        if (!isValid(this->msgs))
            return false;

        this->msgs[0] = '\0';
        return true;
    }

    void free_msgs(void)
    {
        if (this->msgs)
            delete[] this->msgs;
    }
};

static inline void endl(void)
{
    io_putstr_async((const kubyte_t *)"\r\n", 3);
}

class ostream {
public:
    ostream() {}
    ~ostream() {}

    ostream &operator<<(const kchar_t ch)
    {
        io_putc((const kubyte_t)ch);        
        return *this;
    }

    ostream &operator<<(const kchar_t *str)
    {
        io_putstr_async((const kubyte_t *)str, kstrlen(str) + 1);
        return *this;
    }

    ostream &operator<<(const kubyte_t *str)
    {
        io_putstr_async(str, kstrlen((const kchar_t *)str) + 1);
        return *this;
    }

    ostream &operator<<(string &str)
    {
        if (str.c_str())
            io_putstr_async((const kubyte_t *)str.c_str(), str.size() + 1);
        
        return *this;
    }

    ostream &operator<<(kint32_t i)
    {
        printk("%d", i);
        return *this;
    }

    ostream &operator<<(kuint32_t i)
    {
        printk("%d", i);
        return *this;
    }

    ostream &operator<<(void (*fn)(void))
    {
        fn();
        return *this;
    }
};

class istream {
public:
    istream() {}
    ~istream() {}

    istream &operator>>(kchar_t &ch)
    {
        io_getc((kubyte_t *)&ch);        
        return *this;
    }

    istream &operator>>(kubyte_t &ch)
    {
        io_getc(&ch);        
        return *this;
    }

    istream &operator>>(string &str)
    {
        if (str.c_str())
            str.real_size = io_getstr((kubyte_t *)str.c_str(), str.length());
        
        return *this;
    }

    istream &operator>>(void (*fn)(void))
    {
        fn();
        return *this;
    }
};

extern istream cin;
extern ostream cout;
extern ostream cerr;

/*!< ------------------------------------------------------------- */
END_NAMESPACE(bsc)
/*!< ------------------------------------------------------------- */

#endif

/*!< end of file */
