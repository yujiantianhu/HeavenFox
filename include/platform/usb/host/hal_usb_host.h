/*
 * Hardware Abstraction Layer USB HOST Interface
 *
 * File Name:   hal_usb_host.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.12.17
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __HAL_USB_HOST_H_
#define __HAL_USB_HOST_H_

/*!< The includes */
#include <platform/hal_basic.h>

/*!< The defines */
enum __ERT_HAL_USB_HOST_INDEX
{
    /*!< KHCI 0U */
    Ert_HAL_USB_HostIndexKhci0 = 0U,       
    /*!< KHCI 1U, Currently, there are no platforms which have two KHCI IPs, this is reserved to be used in the future. */                        
    Ert_HAL_USB_HostIndexKhci1 = 1U,                               
    /*!< EHCI 0U */
    Ert_HAL_USB_HostIndexEhci0 = 2U,       
    /*!< EHCI 1U, Currently, there are no platforms which have two EHCI IPs, this is reserved to be used in the future. */                        
    Ert_HAL_USB_HostIndexEhci1 = 3U,                               

    /*!< LPC USB IP3511 FS controller 0 */
    Ert_HAL_USB_HostIndexLpcIp3511Fs0 = 4U,           
    /*!< LPC USB IP3511 FS controller 1, there are no platforms which have two IP3511 IPs, this is reserved to be used in the future. */             
    Ert_HAL_USB_HostIndexLpcIp3511Fs1 = 5U,                        

    /*!< LPC USB IP3511 HS controller 0 */
    Ert_HAL_USB_HostIndexLpcIp3511Hs0 = 6U,            
    /*!< LPC USB IP3511 HS controller 1, there are no platforms which have two IP3511 IPs, this is reserved to be used in the future. */            
    Ert_HAL_USB_HostIndexLpcIp3511Hs1 = 7U,                        

    /*!< OHCI 0U */
    Ert_HAL_USB_HostIndexOhci0 = 8U,        
    /*!< OHCI 1U, Currently, there are no platforms which have two OHCI IPs, this is reserved to be used in the future. */                       
    Ert_HAL_USB_HostIndexOhci1 = 9U,                               

    /*!< IP3516HS 0U */
    Ert_HAL_USB_HostIndexIp3516Hs0 = 10U,         
    /*!< IP3516HS 1U, Currently, there are no platforms which have two IP3516HS IPs, this is reserved to be used in the future. */                 
    Ert_HAL_USB_HostIndexIp3516Hs1 = 11U,                          

};

typedef struct hal_usb_host_if
{
    ksint32_t (*create) (kuint32_t index);
    ksint32_t (*destroy) (kuint32_t index);
    ksint32_t (*open) (kuint32_t index);
    ksint32_t (*close) (kuint32_t index);
    ksint32_t (*send) (kuint32_t index);
    ksint32_t (*recv) (kuint32_t index);

} srt_hal_usb_host_if_t;


#endif /*!< __HAL_USB_HOST_H_ */
