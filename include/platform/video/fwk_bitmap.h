/*
 * Display Bitmap Interface
 *
 * File Name:   fwk_bitmap.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.29
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_BITMAP_H_
#define __FWK_BITMAP_H_

/*!< The includes */
#include <platform/fwk_basic.h>

/*!< The defines */
#pragma pack(push)
#pragma pack(1)

/*!< image dot matrix */
typedef struct fwk_dotmat_header
{
	kuint8_t  scan_type;					/*!< image scaning type, 0b0000: 水平扫描; 0b1111, 垂直扫描 */
	kuint8_t  pixelbit;						/*!< bits of per pixel color: 4、8、16、24 */
	kuint16_t width;						/*!< image width, unit: pixel */
	kuint16_t height;						/*!< image height, unit: pixel */

} srt_fwk_dotmat_header_t;

/*!< BitMap */
/*!< The first 54 bytes of the BMP image are the file information headers, and the BmpFileHeader of the bitmap file is the first 14 bytes */
typedef struct fwk_bmp_file_header
{
	kuint16_t picType;               		/*!< the content is fixed to 0x42, 0x4D, i.e., ASCII character 'B', 'M', i.e., BMP format */
	kuint32_t bmpSize;               		/*!< BMP size (unit: byte) */

	kuint16_t reserved1;             		/*!< reserved */
	kuint16_t reserved2;             		/*!< reserved */
	kuint32_t offsetbit;             		/*!< BMP file header + BMP info header + color palette */

} srt_fwk_bmp_file_header_t;

/*!< Bitmap information header (BmpInfoHeader) is the first 15~54 bytes */
typedef struct fwk_bmp_info_header
{
	kuint32_t infoSize;              		/*!< the size of the entire bitmap info header in bytes, i.e. the size of the BmpInfoHeader struct */
	kint32_t width;                 		/*!< image width (unit: pixel) */

	/*!< 
	 * image height (unit: pixel). Positive number indicates a positive direction (starting from the bottom left corner), 
	 * and a negative number indicates a reverse direction (starting from the upper left corner) 
	 */
	kint32_t height;                		
	kuint16_t plane;                 		/*!< color flat book, generally fixed at 1 */
	kuint16_t pixelbit;              		/*!< number of pixel color bits: 1、4、8、16、24. (From the header of the file, this entry is at the 28th byte) */
	kuint32_t compression;           		/*!< compression properties of the data */
	kuint32_t imageSize;             		/*!< the size of the image data (unit: byte) */
	kuint32_t X_pixelPerM;           		/*!< horizontal resolution (unit: pixels/meter) */
	kuint32_t Y_pixelPerM;           		/*!< vertical resolution (unit: pixels/meter) */
	kuint32_t colorIndex;            		/*!< the number of color indexes in the palette. 16-bit and 24-bit true color without color palette. This is 0 */
	kuint32_t importanIndex;         		/*!< the number of indexes that have an important impact on the image, 0 indicates that all are important */

} srt_fwk_bmp_info_header_t;

#pragma pack(pop)

/*!< The functions */


#endif /*!< __FWK_BITMAP_H_ */
