/*
 * Display Bitmap Interface
 *
 * File Name:   fwk_bitmap.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.29
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/video/fwk_bitmap.h>
#include <platform/video/fwk_rgbmap.h>

/*!< API function */
/*!
 * @brief   pixel_rgbform_convert
 * @param   none
 * @retval  none
 * @note    none
 */
kuint32_t pixel_rgbform_convert(ksint8_t srctype, kuint32_t data)
{
    /*!< R, G, B, diaphaneity */
    kuint8_t  r_data, g_data, b_data, d_data;
    kuint32_t result = data;

    switch (srctype)
    {
        case RGBPIXELBIT16:
            // b_data = (uint8_t)data  << 3;
            // g_data = (uint8_t)(data >> 5)  << 2;
            // r_data = (uint8_t)(data >> 11) << 3;
            // result = ((uint16_t)b_data << 8) | ((uint16_t)g_data << 3) | ((uint16_t)r_data >> 3);

            /*!< 
             * The so-called BGR and RGB are actually caused by the influence of the little-end mode, 
             * not that R and B change positions! It's just that for RGB888, that's what it seems 
             */
            b_data = (kuint8_t)data;
            r_data = (kuint8_t)(data >> 8);
            result = ((kuint16_t)b_data << 8) | ((kuint16_t)r_data);
            break;

        case RGBPIXELBIT24:
            b_data = (kuint8_t)data;
            g_data = (kuint8_t)(data >> 8);
            r_data = (kuint8_t)(data >> 16);
            result = ((kuint32_t)b_data << 16) | ((kuint32_t)g_data << 8) | ((kuint32_t)r_data);
            break;

        case RGBPIXELBIT32:
            d_data = (kuint8_t)data;
            b_data = (kuint8_t)(data >> 8);
            g_data = (kuint8_t)(data >> 16);
            r_data = (kuint8_t)(data >> 24);
            result = (((kuint32_t)d_data << 24) | (kuint32_t)b_data << 16) | ((kuint32_t)g_data << 8) | ((kuint32_t)r_data);
            break;

        default:
            break;
    }

    return result;
}
