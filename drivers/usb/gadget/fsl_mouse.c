/*
 * USB Gadget Driver
 *
 * File Name:   fsl_gadget.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.01.07
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/hal_basic.h>
#include <platform/hal_cdev.h>
#include <platform/hal_chrdev.h>
#include <platform/hal_inode.h>
#include <platform/hal_fs.h>
#include <platform/of/hal_of.h>
#include <platform/hal_platdrv.h>
#include <platform/hal_uaccess.h>
#include <platform/usb/hal_ch9.h>
#include <platform/usb/hal_urb.h>
#include <platform/usb/hal_hid.h>
#include <platform/usb/hal_usb.h>

/*!< The defines */
struct fsl_mouse_drv
{
	kstring_t *ptrName;
	kuint32_t major;
	kuint32_t minor;

	struct hal_cdev *sprt_cdev;

	void *ptrData;
};

#define FSL_MOUSE_DRIVER_NAME								"fsl-mouse"
#define FSL_MOUSE_DRIVER_MAJOR								108

#define FSL_MOUSE_ENDPOINT_ADDR                            	(1)

#define FSL_HS_HID_MOUSE_INTR_IN_PACKET_SIZE                (8U)
#define FSL_FS_HID_MOUSE_INTR_IN_PACKET_SIZE                (8U)

/*!< 2 ^ (6 - 1) = 1ms */
#define FSL_HS_HID_MOUSE_INTR_IN_INTERVAL                   (0x06U)
#define FSL_FS_HID_MOUSE_INTR_IN_INTERVAL                   (0x04U)

typedef struct fsl_mouse_desc
{
    const srt_hal_usb_endpoint_desc_t *sprt_ep_dt;
    const srt_hal_usb_interface_desc_t *sprt_if_dt;
    const srt_hal_usb_config_desc_t *sprt_config_dt;
    const srt_hal_usb_device_desc_t *sprt_device_dt;
    const srt_hal_usb_hid_desc_t *sprt_hid_dt;

} srt_fsl_mouse_desc_t;

/*!< The globals */
/*!< Hid mouse endpoint descriptor */
static const srt_hal_usb_endpoint_desc_t sgrt_fsl_mouse_ep_dt[] =
{
    {
        .bLength = HAL_USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = HAL_USB_DT_ENDPOINT,

        .bEndpointAddress = HAL_USB_ENDPOINT_ADDRESS(Ert_HAL_USB_EndpointAddrDirIn, FSL_MOUSE_ENDPOINT_ADDR),
        .bmAttributes = Ert_HAL_USB_EndPointDescXferInterrupt,
        .wMaxPacketSize = FSL_FS_HID_MOUSE_INTR_IN_PACKET_SIZE,
        .bInterval = FSL_FS_HID_MOUSE_INTR_IN_INTERVAL,
    }
};

/*!< Hid mouse interface descriptor */
static const srt_hal_usb_interface_desc_t sgrt_fsl_mouse_if_dt[] =
{
    {
        .bLength = HAL_USB_DT_INTERFACE_SIZE,
        .bDescriptorType = HAL_USB_DT_INTERFACE,

        .bInterfaceNumber = 0U,
        .bAlternateSetting = 0U,
        .bNumEndpoints = ARRAY_SIZE(sgrt_fsl_mouse_ep_dt),
        .bInterfaceClass = HAL_USB_HID_CLASS,
        .bInterfaceSubClass = HAL_USB_HID_SUBCLASS_GRUB,
        .bInterfaceProtocol = HAL_USB_HID_PROTOCOL_MOUSE,
        .iInterface = 0U,
    }
};

/*!< Hid mouse configuration descriptor */
static const srt_hal_usb_config_desc_t sgrt_fsl_mouse_config_dt[] =
{
    {
        .bLength = HAL_USB_DT_CONFIG_SIZE,
        .bDescriptorType = HAL_USB_DT_CONFIG,

        .wTotalLength = 1U,
        .bNumInterfaces = ARRAY_SIZE(sgrt_fsl_mouse_if_dt),
        .bConfigurationValue = 0U,
        .iConfiguration = 0U,
        .bmAttributes = Ert_HAL_USB_ConfigDescAttrSelfPower | Ert_HAL_USB_ConfigDescAttrWakeUp,
        .bMaxPower = HAL_USB_DT_CONFIG_MAX_CURRENT(9),
    }
};

/*!< Hid mouse device descriptor */
static const srt_hal_usb_device_desc_t sgrt_fsl_mouse_device_dt =
{
    .bLength = HAL_USB_DT_DEVICE_SIZE,
    .bDescriptorType = HAL_USB_DT_DEVICE,

    .bcdUSB = 0x0200,
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = HAL_USB_INTR_FS_MAX_PACKET_SIZE,
    .idVendor = 0x1234,
    .idProduct = 0x0100,
    .bcdDevice = 65535,
    .iManufacturer = 0,
    .iProduct = 0,
    .iSerialNumber = 0,
    .bNumConfigurations = ARRAY_SIZE(sgrt_fsl_mouse_config_dt),
};

/*!< Hid mouse hid descriptor */
static const srt_hal_usb_hid_desc_t sgrt_fsl_mouse_hid_dt =
{
    .bLenth = HAL_USB_DT_HID_SIZE,
    .bDescriptorType = HAL_USB_DT_HID,

    .bcdHID = 0x0200,
    .bCountryCode = 156,
    .bNumDescriptor = 0U,
    .bSubDescriptorType = HAL_USB_DT_HID_REPORT,
    .wDescriptorLength = 0U,
};

/*!< Gather all */
static srt_fsl_mouse_desc_t sgrt_fsl_mouse_dt =
{
    .sprt_ep_dt = &sgrt_fsl_mouse_ep_dt[0],
    .sprt_if_dt = &sgrt_fsl_mouse_if_dt[0],
    .sprt_config_dt = &sgrt_fsl_mouse_config_dt[0],
    .sprt_device_dt = &sgrt_fsl_mouse_device_dt,
    .sprt_hid_dt = &sgrt_fsl_mouse_hid_dt,
};

/*!< API function */
/*!
 * @brief   usb gadget interrupt handler
 * @param   ptrDev: private parameter
 * @retval  error code
 * @note    none
 */
irq_return_t fsl_mouse_driver_handler(void *ptrDev)
{
    if (!mrt_isValid(ptrDev))
    {
        return mrt_retval(Ert_isNullPtr);
    }

    return mrt_retval(Ert_isWell);
}

/*!
 * @brief   fsl_mouse_driver_open
 * @param   sprt_inode, sprt_file
 * @retval  errno
 * @note    none
 */
static ksint32_t fsl_mouse_driver_open(struct hal_inode *sprt_inode, struct hal_file *sprt_file)
{
	struct fsl_mouse_drv *sprt_privdata;

	sprt_privdata = sprt_inode->sprt_cdev->privData;
	sprt_file->private_data = sprt_privdata;

	return 0;
}

/*!
 * @brief   fsl_mouse_driver_close
 * @param   sprt_inode, sprt_file
 * @retval  errno
 * @note    none
 */
static ksint32_t fsl_mouse_driver_close(struct hal_inode *sprt_inode, struct hal_file *sprt_file)
{
	sprt_file->private_data = mrt_nullptr;

	return 0;
}

/*!
 * @brief   fsl_mouse_driver_write
 * @param   sprt_file, ptrBuffer, size
 * @retval  errno
 * @note    none
 */
static kssize_t fsl_mouse_driver_write(struct hal_file *sprt_file, const ksbuffer_t *ptrBuffer, kssize_t size)
{
    return 0;
}

/*!
 * @brief   fsl_mouse_driver_read
 * @param   sprt_file, ptrBuffer, size
 * @retval  errno
 * @note    none
 */
static kssize_t fsl_mouse_driver_read(struct hal_file *sprt_file, ksbuffer_t *ptrBuffer, kssize_t size)
{
	return 0;
}

/*!< led-template driver operation */
const struct hal_file_oprts sgrt_fsl_mouse_driver_oprts =
{
	.open	= fsl_mouse_driver_open,
	.close	= fsl_mouse_driver_close,
	.write	= fsl_mouse_driver_write,
	.read	= fsl_mouse_driver_read,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   fsl_mouse_driver_probe
 * @param   sprt_dev
 * @retval  errno
 * @note    none
 */
static ksint32_t fsl_mouse_driver_probe(struct hal_platdev *sprt_dev)
{
	ksint32_t retval;

    /*!< Register interrupt handler */
	retval = hal_request_irq(75, fsl_mouse_driver_handler, 0, FSL_MOUSE_DRIVER_NAME, mrt_nullptr);
	if (mrt_isErr(retval))
	{
        return mrt_retval(Ert_isNotSuccess);
    }

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   fsl_mouse_driver_remove
 * @param   sprt_dev
 * @retval  errno
 * @note    none
 */
static ksint32_t fsl_mouse_driver_remove(struct hal_platdev *sprt_dev)
{

	return mrt_retval(Ert_isWell);
}

/*!< platform instance */
static struct hal_platdrv sgrt_fsl_mouse_platdriver =
{
	.probe	= fsl_mouse_driver_probe,
	.remove	= fsl_mouse_driver_remove,
	
	.sgrt_driver =
	{
		.name 	= FSL_MOUSE_DRIVER_NAME,
		.id 	= -1,
	},
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   fsl_mouse_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
ksint32_t __hal_init fsl_mouse_driver_init(void)
{
	return hal_register_platdriver(&sgrt_fsl_mouse_platdriver);
}

/*!
 * @brief   fsl_mouse_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __hal_exit fsl_mouse_driver_exit(void)
{
	hal_unregister_platdriver(&sgrt_fsl_mouse_platdriver);
}

// IMPORT_DRIVER_INIT(fsl_mouse_driver_init);
// IMPORT_DRIVER_EXIT(fsl_mouse_driver_exit);

/*!< end of file */
