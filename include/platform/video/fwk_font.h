/*
 * Display Font Libraries of ASCII
 *
 * File Name:   fwk_font.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.29
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_ASCII_H_
#define __FWK_ASCII_H_

/*!< The includes */
#include <platform/fwk_basic.h>

/*!< The globals */
/*!< 8 * 12 ASCII character dot matrix */
TARGET_EXT const kuint8_t g_font_ascii_song12[];

/*!< 16 * 16 ASCII character dot matrix */
TARGET_EXT const kuint8_t g_font_ascii_song16[];

/*!< 24 * 12 ASICII character dot matrix */
TARGET_EXT const kuint8_t g_font_ascii_song24[];

#endif /*!< __FWK_ASCII_H_ */
