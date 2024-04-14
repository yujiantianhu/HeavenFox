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

#ifndef __FWK_FONT_H_
#define __FWK_FONT_H_

/*!< The includes */
#include <platform/fwk_basic.h>

/*!< The defines */
#define FWK_FONT_HZ_3BYTES                  (0)

#define FWK_FONT_8                          (8)
#define FWK_FONT_16                         (16)
#define FWK_FONT_24                         (24)
#define FWK_FONT_32                         (32)

#define ASCII_SPACE					        32              /*!< space ' ' */
#define ASCII_MAX					        127

/*!< area code */
#define AREA_FIRST_CODE				        0xA1
#define BITS_FIRST_CODE				        0xA1

enum __ERT_FWK_FONT_TYPE
{
    NR_FWK_FONT_SONG        = 0,
    NR_FWK_FONT_XINGKAI,
};

/*!< The globals */
typedef struct fwk_font_setting
{
    void *ptr_ascii;
    void *ptr_hz;

    kuint32_t font;                                         /*!< refer to "__ERT_FWK_FONT_TYPE" */
    kuint32_t color;
    kuint32_t background;
    kuint32_t size;
    kuint32_t line_spacing;

} srt_fwk_font_setting_t;

/*!< 8 * 12 ASCII character dot matrix */
TARGET_EXT const kuint8_t g_font_ascii_song12[];

/*!< 16 * 16 ASCII character dot matrix */
TARGET_EXT const kuint8_t g_font_ascii_song16[];

/*!< 24 * 12 ASICII character dot matrix */
TARGET_EXT const kuint8_t g_font_ascii_song24[];

#endif /*!< __FWK_FONT_H_ */
