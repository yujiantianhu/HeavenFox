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
#include <platform/video/fwk_disp.h>
#include <platform/video/fwk_bitmap.h>
#include <platform/video/fwk_rgbmap.h>
#include <kernel/kernel.h>
#include <kernel/mutex.h>

/*!< API function */
/*!
 * @brief   BGR <===> RGB
 * @param   srctype: 16/24/32
 * @retval  none
 * @note    none
 */
kuint32_t fwk_pixel_rgbform_convert(kint8_t srctype, kuint32_t data)
{
    /*!< R, G, B, diaphaneity */
    kuint8_t  r_data, g_data, b_data, d_data;
    kuint32_t result = data;

    switch (srctype)
    {
        case FWK_RGB_PIXEL16:
//          b_data = (uint8_t)data  << 3;
//          g_data = (uint8_t)(data >> 5)  << 2;
//          r_data = (uint8_t)(data >> 11) << 3;
//          result = ((uint16_t)b_data << 8) | ((uint16_t)g_data << 3) | ((uint16_t)r_data >> 3);

            /*!< 
             * The so-called BGR and RGB are actually caused by the influence of the little-end mode, 
             * not that R and B change positions! It's just that for RGB888, that's what it seems 
             */
            b_data = (kuint8_t)data;
            r_data = (kuint8_t)(data >> 8);
            result = ((kuint16_t)b_data << 8) | ((kuint16_t)r_data);
            break;

        case FWK_RGB_PIXEL24:
            b_data = (kuint8_t)data;
            g_data = (kuint8_t)(data >> 8);
            r_data = (kuint8_t)(data >> 16);
            result = ((kuint32_t)b_data << 16) | ((kuint32_t)g_data << 8) | ((kuint32_t)r_data);
            break;

        case FWK_RGB_PIXEL32:
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

/*!
 * @brief   display one image (with dot matrix)
 * @param   x_start: base position in the x-direction
 * @param   y_start: base position in the y-direction
 * @retval  None
 * @note    none
 */
void fwk_display_dot_matrix_image(struct fwk_disp_info *sprt_disp, kuint32_t x_start, kuint32_t y_start, const kuint8_t *image)
{
    struct fwk_dotmat_header *sprt_dmatx = mrt_nullptr;
    kuint8_t *ptr_image = mrt_nullptr;

    kuint32_t offset, p_cnt = 0;
    kuint8_t image_bpp;
    kuint32_t rgb_data, rgb_inc = 0;
    kuint32_t width, height, px_cnt, py_cnt;

    if ((!image) || (!sprt_disp) || (!sprt_disp->sprt_ops))
        return;

    sprt_dmatx = (struct fwk_dotmat_header *)image;
    ptr_image = (kuint8_t *)(image + sizeof(*sprt_dmatx));

    width   = (kuint32_t)sprt_dmatx->width;
    height  = (kuint32_t)sprt_dmatx->height;

    if (((x_start + width) > sprt_disp->width) || ((y_start + height) > sprt_disp->height))
        return;

    /*!< bytes of per pixel color (RGB24: bits = 24, bytes = 3) */
    image_bpp = sprt_dmatx->pixelbit >> 3;

    /*!< horizontal scanning is default used for image dot matrix modeling, without considering vertical scanning */
    for (py_cnt = 0; py_cnt < height; py_cnt++)
    {
        offset = sprt_disp->sprt_ops->get_offset(x_start, y_start + py_cnt, sprt_disp->width);

        for (px_cnt = 0; px_cnt < width; px_cnt++)
        {
            for (p_cnt = 0; p_cnt < image_bpp; p_cnt++)
            {
                rgb_data <<= 8;
                rgb_data  |= *(ptr_image + rgb_inc + p_cnt);
            }

            rgb_inc += image_bpp;

            /*!< reverse littile endian, for RGB24, such as: BGR <===> RGB */
            rgb_data = fwk_pixel_rgbform_convert(image_bpp, rgb_data);

            if (sprt_disp->sprt_ops->convert_rgbbit)
                rgb_data = sprt_disp->sprt_ops->convert_rgbbit(image_bpp, sprt_disp->bpp, rgb_data);

            if (sprt_disp->sprt_ops->write_pixel)
                sprt_disp->sprt_ops->write_pixel(sprt_disp->buffer, offset + px_cnt, sprt_disp->bpp, rgb_data);
        }
    }
}

/*!
 * @brief   display one image (with bmp)
 * @param   x_start: base position in the x-direction
 * @param   y_start: base position in the y-direction
 * @param   image: ram address of bmp
 * @retval  None
 * @note    none
 */
void fwk_display_bitmap(struct fwk_disp_info *sprt_disp, kuint32_t x_start, kuint32_t y_start, const kuint8_t *image)
{
    struct fwk_bmp_file_header *sprt_file;
    struct fwk_bmp_info_header *sprt_info;
    kuint8_t *ptr_bitmap;

    union fwk_bmp_type
    {
        kuint8_t bmpname[2];
        kuint16_t pic_type;
    };
    union fwk_bmp_type ugrt_type = { .bmpname = {'B', 'M'} };

    kuint32_t offset, p_cnt = 0;
    kuint32_t rgb_data, rgb_inc = 0;
    kuint8_t image_bpp;
    kuint32_t width, height, px_cnt, py_cnt, y_offset;

    if ((!image) || (!sprt_disp) || (!sprt_disp->sprt_ops))
        return;

    sprt_file = (struct fwk_bmp_file_header *)image;
    sprt_info = (struct fwk_bmp_info_header *)(image + sizeof(*sprt_file));
    ptr_bitmap = (kuint8_t *)(image + sizeof(*sprt_file) + sizeof(*sprt_info));

    if (sprt_file->picType != ugrt_type.pic_type)
        return;

    /*!< deal with bmp */
    width  = (kuint32_t)sprt_info->width;
    height = (kuint32_t)sprt_info->height;
    
    if (((x_start + width) > sprt_disp->width) || ((y_start + height) > sprt_disp->height))
        return;

    /*!< bytes of per pixel color (RGB24: bits = 24, bytes = 3) */
    image_bpp = sprt_info->pixelbit >> 3;

    /*!< draw rgb pixel */
    for (py_cnt = 0; py_cnt < height; py_cnt++)
    {
        /*!<
         * if height > 0: the image scanning method is from left to right and from bottom to top; 
         * Otherwise, it will be from left to right, from top to bottom 
         */
        y_offset = (0 > sprt_info->height) ? py_cnt : ((height - 1) -py_cnt);
        offset = sprt_disp->sprt_ops->get_offset(x_start, y_start + y_offset, sprt_disp->width);

        for (px_cnt = 0; px_cnt < width; px_cnt++)
        {
            for (p_cnt = 0; p_cnt < image_bpp; p_cnt++)
            {
                rgb_data <<= 8;
                rgb_data  |= *(ptr_bitmap + rgb_inc + p_cnt);
            }

            rgb_inc += image_bpp;

            /*!< reverse littile endian, for RGB24, such as: BGR <===> RGB */
            rgb_data = fwk_pixel_rgbform_convert(image_bpp, rgb_data);

            if (sprt_disp->sprt_ops->convert_rgbbit)
                rgb_data = sprt_disp->sprt_ops->convert_rgbbit(image_bpp, sprt_disp->bpp, rgb_data);

            if (sprt_disp->sprt_ops->write_pixel)
                sprt_disp->sprt_ops->write_pixel(sprt_disp->buffer, offset + px_cnt, sprt_disp->bpp, rgb_data);
        }
    }
}

/*!< end of file */
