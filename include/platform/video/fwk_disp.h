/*
 * Display Generic API
 *
 * File Name:   fwk_disp.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.13
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_DISP_H_
#define __FWK_DISP_H_

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <kernel/mutex.h>
#include <kernel/spinlock.h>

#include "fwk_font.h"
#include "fwk_rgbmap.h"

/*!< The globals */

/*!< The defines */
typedef struct fwk_disp_info
{
    void *buffer;
    void *buffer_bak;
    kusize_t buf_size;                      /*!< unit: byte */
    kuint32_t width;
    kuint32_t height;
    kuint8_t bpp;

    struct mutex_lock sgtc_lock;

} srt_fwk_disp_info_t;

typedef struct fwk_disp_ctrl
{
    struct fwk_disp_info *sptr_di;
    struct fwk_font_setting sgtc_set;

    kuint32_t x_start;
    kuint32_t y_start;
    kuint32_t x_end;
    kuint32_t y_end;

    kuint32_t x_next;
    kuint32_t y_next;

} srt_fwk_disp_ctrl_t;

#define IS_DISP_FRAME_FULL(sptr_dctrl)  \
        (((sptr_dctrl)->x_next == (sptr_dctrl)->x_start) && \
         ((sptr_dctrl)->y_next >= (sptr_dctrl)->y_end))

/*!< buffer must be 2 bytes alignment */
#define mr_fwk_disp_write_fb16(buffer, offset, data) \
    do {    \
        *(((kuint16_t *)(buffer)) + (offset)) = data;   \
    } while (0)

/*!< buffer must be 4 bytes alignment */
#define mr_fwk_disp_write_fb32(buffer, offset, data) \
    do {    \
        *(((kuint32_t *)(buffer)) + (offset)) = data;   \
    } while (0)

/*!< buffer must be 4 bytes alignment */
#define mr_fwk_disp_write_fb24(buffer, offset, data)               mr_fwk_disp_write_fb32(buffer, offset, data)

#define mr_fwk_disp_write_pixel(buffer, offset, bpp, data)   \
    do {    \
        if ((bpp) == FWK_RGB_PIXEL32) \
            mr_fwk_disp_write_fb32(buffer, offset, data);  \
        else if ((bpp) == FWK_RGB_PIXEL24) \
            mr_fwk_disp_write_fb24(buffer, offset, data);  \
        else if ((bpp) == FWK_RGB_PIXEL16)    \
            mr_fwk_disp_write_fb16(buffer, offset, data);  \
    } while (0)

#define mr_fwk_disp_bpp_get(bpp)                                   (((bpp) == FWK_RGB_PIXEL24) ? FWK_RGB_PIXEL32 : (bpp))
#define mr_fwk_disp_advance_pos(x, y, x_max)                       ((y) * (x_max) + (x))
#define mr_fwk_disp_convert_rgb(src, dest, data)   \
            (((src) == (dest)) ? (data) : fwk_display_convert_rgbbit(src, dest, data))

/*!< The functions */
extern kuint32_t fwk_display_convert_rgbbit(kuint8_t srctype, kuint8_t destype, kuint32_t data);
extern void fwk_display_set_cursor(struct fwk_disp_ctrl *sptr_dctrl, 
                            kuint32_t x_start, kuint32_t y_start, kuint32_t x_end, kuint32_t y_end);
extern void fwk_display_write_straight_line(struct fwk_disp_info *sptr_disp, kuint32_t x_start, kuint32_t y_start, 
                            kuint32_t x_end, kuint32_t y_end, kuint32_t data);
extern void fwk_display_write_rectangle(struct fwk_disp_info *sptr_disp, kuint32_t x_start, kuint32_t y_start, 
                            kuint32_t x_end, kuint32_t y_end, kuint32_t data);
extern void fwk_display_fill_rectangle(struct fwk_disp_info *sptr_disp, kuint32_t x_start, kuint32_t y_start, 
                            kuint32_t x_end, kuint32_t y_end, kuint32_t data);
extern void fwk_display_clear(struct fwk_disp_info *sptr_disp, kuint32_t data);
extern kusize_t fwk_display_word(struct fwk_disp_ctrl *sptr_dctrl, const kchar_t *fmt, ...);

extern void fwk_display_ctrl_init(struct fwk_disp_info *sptr_disp, void *fbuffer, 
                          void *fbuffer2, kusize_t size, kuint32_t width, kuint32_t height, kuint32_t bpp);

/*!< API functions */
/*!
 * @brief   get position real-time
 * @param   xpos, ypos
 * @retval  none
 * @note    none
 */
static inline __force_inline 
kuint32_t fwk_display_advance_position(kuint32_t xpos, kuint32_t ypos, kuint32_t x_max)
{
    return mr_fwk_disp_advance_pos(xpos, ypos, x_max);
}

/*!
 * @brief   write a pixel
 * @param   buffer: framebuffer
 * @param   offset: buffer offset
 * @param   data: rgb color
 * @retval  none
 * @note    none
 */
static inline __force_inline 
void fwk_display_write_pixel(void *buffer, kuint32_t offset, kuint8_t bpp, kuint32_t data)
{
    mr_fwk_disp_write_pixel(buffer, offset, bpp, data);
}

/*!
 * @brief   write one data
 * @param   buffer: framebuffer
 * @param   offset: buffer offset
 * @param   data: rgb color
 * @retval  none
 * @note    combine format-converter and pixel writting
 */
static inline 
void fwk_display_write_frame_data(void *buffer, kuint32_t offset, kuint8_t bpp, kuint32_t data)
{
    kuint32_t rgb_data;
    kuint8_t  pixelbits;

    pixelbits = mr_fwk_disp_bpp_get(bpp);
    rgb_data  = mr_fwk_disp_convert_rgb(FWK_RGB_PIXELBIT, pixelbits, data);

    mr_fwk_disp_write_pixel(buffer, offset, pixelbits, rgb_data);
}

/*!
 * @brief   write a pixel point
 * @param   sptr_disp: screen information
 * @param   xpos: x-direction position
 * @param   ypos: y-direction position
 * @param   data: rgb color
 * @retval  none
 * @note    none
 */
static inline 
void fwk_display_write_point(struct fwk_disp_info *sptr_disp, kuint32_t xpos, kuint32_t ypos, kuint32_t data)
{
    kuint32_t offset, rgb_data;
    kuint8_t  pixelbits;

    offset = mr_fwk_disp_advance_pos(xpos, ypos, sptr_disp->width);
    pixelbits = mr_fwk_disp_bpp_get(sptr_disp->bpp);
    rgb_data  = mr_fwk_disp_convert_rgb(FWK_RGB_PIXELBIT, pixelbits, data);

    mr_fwk_disp_write_pixel(sptr_disp->buffer, offset, pixelbits, rgb_data);
}

/*!
 * @brief  exchange framebuffer
 * @param  sptr_disp
 * @retval none
 * @note   none
 */
static inline 
void fwk_display_frame_exchange(struct fwk_disp_info *sptr_disp)
{
    void *buffer = sptr_disp->buffer_bak;

    if (!buffer)
        return;

    sptr_disp->buffer_bak = sptr_disp->buffer;
    mr_barrier();
    sptr_disp->buffer = buffer;
}

/*!
 * @brief  copy backup to active screen
 * @param  sptr_disp
 * @retval none
 * @note   none
 */
static inline
void fwk_display_frame_flush(struct fwk_disp_info *sptr_disp, kusize_t size)
{
    if (sptr_disp->buffer_bak)
        memcpy(sptr_disp->buffer, sptr_disp->buffer_bak, size);
}

/*!
 * @brief  copy backup to deactive screen
 * @param  sptr_disp
 * @retval none
 * @note   none
 */
static inline
void fwk_display_frame_sync(struct fwk_disp_info *sptr_disp, kusize_t size)
{
    if (sptr_disp->buffer_bak)
        memcpy(sptr_disp->buffer_bak, sptr_disp->buffer, size);
}

#ifdef __cplusplus
    }
#endif

#endif /*!< __FWK_DISP_H_ */
