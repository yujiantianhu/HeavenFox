/*
 * Hardware Abstraction Layer USB HID Interface
 *
 * File Name:   hal_hid.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.12.31
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_HID_H_
#define __HAL_HID_H_

/*!< The includes */
#include <platform/hal_basic.h>
#include "hal_ch9.h"
#include "hal_urb.h"

/*!< The defines */
#define HAL_USB_DT_HID                                          (Ert_HAL_USB_DescTypeHID)
#define HAL_USB_DT_HID_REPORT                                   (Ert_HAL_USB_DescTypeHIDReport)
#define HAL_USB_HID_CLASS                                       (Ert_HAL_USB_DevDescClassHID)

#define HAL_USB_HID_SUBCLASS_NONE                               (0x00)
#define HAL_USB_HID_SUBCLASS_GRUB                               (0x01)

/*!< if SubClass is not "HAL_USB_HID_SUBCLASS_GRUB", protocol value is invalid */
#define HAL_USB_HID_PROTOCOL_KEYBOARD                           (0x01)
#define HAL_USB_HID_PROTOCOL_MOUSE                              (0x02)

typedef struct hal_usb_hid_desc
{
    kuint8_t  bLenth;                                           /*!< 该描述符字节数长度（由下级描述符多少决定） */
    kuint8_t  bDescriptorType;                                  /*!< 描述符类型: HAL_USB_DT_HID */
    kuint16_t bcdHID;                                           /*!< HID协议版本 */
    kuint8_t  bCountryCode;                                     /*!< 国家代码 */
    kuint8_t  bNumDescriptor;                                   /*!< HID设备支持的下级描述符的数量(也就是HID设备特有的报告描述符和物理描述符的综合) */
    kuint8_t  bSubDescriptorType;                               /*!< 下级描述符类型(第一个必须是报告描述符: HAL_USB_DT_HID_REPORT) */
    kuint16_t wDescriptorLength;                                /*!< 下级描述符的长度 */
} srt_hal_usb_hid_desc_t;

#define HAL_USB_DT_HID_SIZE                                     (9)

/*!< HID Packet */
/*!< for bRequest */
enum __ERT_HAL_URB_HID_CLASS_REQ
{
    Ert_HAL_URB_HidClassReqGetReport = 0x01,
    Ert_HAL_URB_HidClassReqGetIdle = 0x02,
    Ert_HAL_URB_HidClassReqGetProtocol = 0x03,
    Ert_HAL_URB_HidClassReqSetReport = 0x09,
    Ert_HAL_URB_HidClassReqSetIdle = 0x0a,
    Ert_HAL_URB_HidClassReqSetProtocol = 0x0b,
};



#endif /*!< __HAL_HID_H_ */
