/*
 * Assembly Defines
 *
 * File Name:   linkage.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.17
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __LINKAGE_H
#define __LINKAGE_H

#define ENTRY(name) \
    .global name

#define ENDPROC(name)   \
    .type name, %function;  \
    .size name, .-name

#endif /* __LINKAGE_H */
