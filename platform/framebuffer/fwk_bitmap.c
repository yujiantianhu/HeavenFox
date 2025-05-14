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
//  kuint8_t  r_data, g_data, b_data, d_data;
    volatile kuint32_t result = data;

    switch (srctype)
    {
        case FWK_RGB_PIXEL16:
            /*!< 
             * The so-called BGR and RGB are actually caused by the influence of the little-end mode, 
             * not that R and B change positions! It's just that for RGB888, that's what it seems 
             */
//          b_data = (kuint8_t)data;
//          r_data = (kuint8_t)(data >> 8);
//          result = ((kuint16_t)b_data << 8) | ((kuint16_t)r_data);
            result = be16_to_cpu(data);
            break;

        case FWK_RGB_PIXEL24:
//          b_data = (kuint8_t)data;
//          g_data = (kuint8_t)(data >> 8);
//          r_data = (kuint8_t)(data >> 16);
//          result = ((kuint32_t)b_data << 16) | ((kuint32_t)g_data << 8) | ((kuint32_t)r_data);
            result = be32_to_cpu(data) >> 8;
            break;

        case FWK_RGB_PIXEL32:
//          d_data = (kuint8_t)data;
//          b_data = (kuint8_t)(data >> 8);
//          g_data = (kuint8_t)(data >> 16);
//          r_data = (kuint8_t)(data >> 24);
//          result = (((kuint32_t)d_data << 24) | (kuint32_t)b_data << 16) | ((kuint32_t)g_data << 8) | ((kuint32_t)r_data);
            result = be32_to_cpu(data);
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
 * @retval  none
 * @note    none
 */
void fwk_display_dot_matrix_image(struct fwk_disp_info *sptr_disp, kuint32_t x_start, kuint32_t y_start, const kuint8_t *image)
{
    struct fwk_dotmat_header *sptr_dmatx = mr_nullptr;
    kuint8_t *ptr_image = mr_nullptr;

    kuint32_t offset, p_cnt = 0;
    kuint8_t image_bpp;
    kuint32_t rgb_data, rgb_inc = 0;
    kuint32_t width, height, px_cnt, py_cnt;

    if ((!image) || (!sptr_disp))
        return;

    sptr_dmatx = (struct fwk_dotmat_header *)image;
    ptr_image = (kuint8_t *)(image + sizeof(*sptr_dmatx));

    width   = (kuint32_t)sptr_dmatx->width;
    height  = (kuint32_t)sptr_dmatx->height;

    if (((x_start + width) > sptr_disp->width) || ((y_start + height) > sptr_disp->height))
        return;

    /*!< bytes of per pixel color (RGB24: bits = 24, bytes = 3) */
    image_bpp = sptr_dmatx->pixelbit >> 3;

    /*!< horizontal scanning is default used for image dot matrix modeling, without considering vertical scanning */
    for (py_cnt = 0; py_cnt < height; py_cnt++)
    {
        offset = fwk_display_advance_position(x_start, y_start + py_cnt, sptr_disp->width);

        for (px_cnt = 0; px_cnt < width; px_cnt++)
        {
            rgb_data = 0;
            for (p_cnt = 0; p_cnt < image_bpp; p_cnt++)
            {
                rgb_data <<= 8;
                rgb_data  |= *(ptr_image + rgb_inc + p_cnt);
            }

            rgb_inc += image_bpp;

            /*!< reverse littile endian, for RGB24, such as: BGR <===> RGB */
            rgb_data = fwk_pixel_rgbform_convert(sptr_dmatx->pixelbit, rgb_data);
            rgb_data = fwk_display_convert_rgbbit(sptr_dmatx->pixelbit, sptr_disp->bpp, rgb_data);
            fwk_display_write_pixel(sptr_disp->buffer, offset + px_cnt, sptr_disp->bpp, rgb_data);
        }
    }
}

/*!
 * @brief   check if image is bmp, and return offset
 * @param   sptr_bctl: bmp ctrl struct
 * @param   image: bmp data
 * @retval  offset of bmp pixel data
 * @note    none
 */
kint32_t fwk_bitmap_get_and_check(struct fwk_bmp_ctrl *sptr_bctl, const kuint8_t *image)
{
    struct fwk_bmp_file_header sgtc_file;
    struct fwk_bmp_info_header *sptr_bi;
    kuint8_t *ptr_bitmap;

    union fwk_bmp_type
    {
        kuint8_t bmpname[2];
        kuint16_t pic_type;
    };
    union fwk_bmp_type ugtr_type = { .bmpname = {'B', 'M'} };

    if ((!image) || (!sptr_bctl))
        return -ER_NOMEM;

    sptr_bi = &sptr_bctl->sgtc_bi;
    ptr_bitmap = (kuint8_t *)image;

    kmemcpy(&sgtc_file, ptr_bitmap, FWK_BMP_FILE_HDR_LEN);
    ptr_bitmap += FWK_BMP_FILE_HDR_LEN;
    kmemcpy(sptr_bi, ptr_bitmap, FWK_BMP_INFO_HDR_LEN);
    ptr_bitmap += FWK_BMP_INFO_HDR_LEN;

    if (sgtc_file.picType != ugtr_type.pic_type)
        return -ER_FAULT;

    return (kint32_t)(ptr_bitmap - image);
}

/*!
 * @brief   display one image (with bmp)
 * @param   sptr_bctl: bmp ctrl struct
 * @param   image: bmp data
 * @param   size: image size (bytes)
 * @retval  error code
 * @note    none
 */
kint32_t fwk_display_bitmap(struct fwk_bmp_ctrl *sptr_bctl, const kuint8_t *image, kusize_t size)
{
    struct fwk_bmp_info_header *sptr_bi;
    struct fwk_disp_info *sptr_disp;
    kuint32_t offset;
    kuint32_t rgb_data, rgb_inc = 0;
    kuint8_t image_bpp;
    kuint32_t width, height, x_pos, y_pos, y_offset;
    kuint8_t *ptr_bitmap;

    if ((!image) || 
        (!sptr_bctl) || 
        (!sptr_bctl->sptr_disp) ||
        (!size))
        return -ER_NOMEM;

    ptr_bitmap = (kuint8_t *)image;
    sptr_bi = &sptr_bctl->sgtc_bi;
    sptr_disp = sptr_bctl->sptr_disp;

    /*!< bytes of per pixel color (RGB24: bits = 24, bytes = 3) */
    image_bpp = sptr_bi->pixelbit >> 3;

    width   = (kuint32_t)mr_abs(sptr_bi->width);
    height  = (kuint32_t)mr_abs(sptr_bi->height);

    if ((!sptr_bctl->x_next) && (!sptr_bctl->y_next))
    {
        if (((sptr_bctl->x_start + width)  > sptr_disp->width) || 
            ((sptr_bctl->y_start + height) > sptr_disp->height))
            return -ER_CHECKERR;

        sptr_bctl->x_next = sptr_bctl->x_start;
        sptr_bctl->y_next = sptr_bctl->y_start;
    }
    else
    {
        if (sptr_bctl->y_next >= (sptr_bctl->y_start + height))
            return -ER_FORBID;
    }

    x_pos = sptr_bctl->x_next;
    y_pos = sptr_bctl->y_next;

    /*!< draw rgb pixel */
    for (rgb_inc = 0; rgb_inc < size; x_pos++, rgb_inc += image_bpp)
    {
        if (x_pos >= (sptr_bctl->x_start + width))
        {
            x_pos = sptr_bctl->x_start;
            y_pos++;
        }

        if (y_pos >= (sptr_bctl->y_start + height))
            break;

        /*!<
         * if height > 0: the image scanning method is from left to right and from bottom to top; 
         *  y_offset = sptr_bi->height - ((y_pos - y_start) + (sptr_bi->height - (y_start + height)))
         * Otherwise, it will be from left to right, from top to bottom 
         */
        y_offset = (sptr_bi->height < 0) ? y_pos : ((sptr_bctl->y_start >> 1) + height - y_pos - 1);

        /*!< offset = x + y * width */
        offset = mr_fwk_disp_advance_pos(x_pos, y_offset, sptr_disp->width);

        rgb_data = 0;
#if 0
        for (kuint32_t p_cnt = 0; p_cnt < image_bpp; p_cnt++)
        {
            rgb_data <<= 8;
            rgb_data  |= *(ptr_bitmap + rgb_inc + p_cnt);
        }

        /*!< reverse littile endian, for RGB24, such as: BGR <===> RGB */
        rgb_data = fwk_pixel_rgbform_convert(sptr_bi->pixelbit, rgb_data);
#else
        kmemcpy(&rgb_data, ptr_bitmap + rgb_inc, image_bpp);
#endif
        rgb_data = mr_fwk_disp_convert_rgb(sptr_bi->pixelbit, sptr_disp->bpp, rgb_data);
        mr_fwk_disp_write_pixel(sptr_disp->buffer, offset, sptr_disp->bpp, rgb_data); 
    }

    sptr_bctl->x_next = x_pos;
    sptr_bctl->y_next = y_pos;

    return ER_NORMAL;
}

/*!
 * @brief   display one image (with bmp)
 * @param   x_start: base position in the x-direction
 * @param   y_start: base position in the y-direction
 * @param   image: ram address of bmp
 * @retval  none
 * @note    none
 */
kssize_t fwk_display_whole_bitmap(struct fwk_bmp_ctrl *sptr_bctl, const kuint8_t *image)
{
    struct fwk_disp_info *sptr_disp;
    struct fwk_bmp_info_header *sptr_bi;
    kuint32_t x_start, y_start;
    kint32_t image_offset;
    kuint8_t *ptr_bitmap;

    kuint32_t offset;
    kuint32_t rgb_data, rgb_inc = 0;
    kuint8_t image_bpp;
    kuint32_t width, height, px_cnt, py_cnt, y_offset;

    if ((!image) || 
        (!sptr_bctl) || 
        (!sptr_bctl->sptr_disp))
        return -ER_NULLPTR;

    image_offset = fwk_bitmap_get_and_check(sptr_bctl, image);
    if (image_offset < 0)
        return -ER_CHECKERR;

    ptr_bitmap = (kuint8_t *)(image + image_offset);
    sptr_bi = &sptr_bctl->sgtc_bi;
    sptr_disp = sptr_bctl->sptr_disp;

    /*!< deal with bmp */
    width  = (kuint32_t)mr_abs(sptr_bi->width);
    height = (kuint32_t)mr_abs(sptr_bi->height);

    x_start = sptr_bctl->x_start;
    y_start = sptr_bctl->y_start;
    
    if (((x_start + width) > sptr_disp->width) || 
        ((y_start + height) > sptr_disp->height))
        return -ER_MORE;

    /*!< bytes of per pixel color (RGB24: bits = 24, bytes = 3) */
    image_bpp = sptr_bi->pixelbit >> 3;

    /*!< draw rgb pixel */
    for (py_cnt = 0; py_cnt < height; py_cnt++)
    {
        /*!<
         * if height > 0: the image scanning method is from left to right and from bottom to top; 
         * Otherwise, it will be from left to right, from top to bottom 
         */
        y_offset = (sptr_bi->height < 0) ? py_cnt : ((height - 1) - py_cnt);
        offset = mr_fwk_disp_advance_pos(x_start, y_start + y_offset, sptr_disp->width);

        for (px_cnt = 0; px_cnt < width; px_cnt++)
        {
            rgb_data = 0;
#if 0
            for (kuint32_t p_cnt = 0; p_cnt < image_bpp; p_cnt++)
            {
                rgb_data <<= 8;
                rgb_data  |= *(ptr_bitmap + rgb_inc + p_cnt);
            }

            /*!< reverse littile endian, for RGB24, such as: BGR <===> RGB */
            rgb_data = fwk_pixel_rgbform_convert(sptr_bi->pixelbit, rgb_data);
#else
            kmemcpy(&rgb_data, ptr_bitmap + rgb_inc, image_bpp);
#endif
            rgb_data = mr_fwk_disp_convert_rgb(sptr_bi->pixelbit, sptr_disp->bpp, rgb_data);
            mr_fwk_disp_write_pixel(sptr_disp->buffer, offset + px_cnt, sptr_disp->bpp, rgb_data);

            rgb_inc += image_bpp;
        }
    }

    return (width * height * (sptr_disp->bpp >> 3));
}

/*!< end of file */
