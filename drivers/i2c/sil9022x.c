/*
 * Template of HDMI Phy Driver : sil9022x (such as sil9022a)
 *  (In general, sii9022x(not sil9022x) is a more common name)
 *
 * File Name:   sil9022x.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.04.05
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/base/fwk_platdrv.h>
#include <platform/base/fwk_uaccess.h>
#include <platform/gpio/fwk_gpiodesc.h>
#include <platform/i2c/fwk_i2c_dev.h>
#include <platform/i2c/fwk_i2c_core.h>
#include <platform/i2c/fwk_i2c_algo.h>
#include <platform/base/fwk_cdev.h>
#include <platform/base/fwk_chrdev.h>
#include <platform/base/fwk_inode.h>
#include <platform/base/fwk_fs.h>
#include <platform/base/fwk_fcntl.h>
#include <platform/notifier/fwk_notifier.h>
#include <platform/gpio/fwk_gpiodesc.h>
#include <platform/video/fwk_fbmem.h>
#include <kernel/sleep.h>
#include <kernel/workqueue.h>

/*!< The defines */
/*!< Register */
/*!< -------------------------------------------------------------- */
/*!< 1. Reset and Initialize */
#define SIL9022X_TMDS                               (0xC7)

/*!
 * 2. Detect Revision
 * When TPI 0x1B can be read correctly, the TPI subsystem is ready 
 */
#define SIL9022X_DEVICE_ID                          (0x1B)  /*!< Expected value: 0xB0 */
#define SIL9022X_PRODUCT_ID                         (0x1C)  /*!< Expected value: 0x02 */
#define SIL9022X_REVISION_ID                        (0x1D)  /*!< Expected value: 0x03 */
#define SIL9022X_HDCP_REVISION                      (0x30)  /*!< Expected value: 0x00 */

/*!< 3. Power up transmitter */
#define SIL9022X_POWER_UP                           (0x1E)  /*!< Write TPI 0x1E[1:0] = 00 to enable active mode */

/*!< 4. Configure Input Bus and Pixel Repetition */
#define SIL9022X_INBUS                              (0x08)  /*!< Select input bus characteristics like pixel size, clock edge */

/*!< 5. Select YC Input Mode */
#define SIL9022X_YC_INPUT_MODE                      (0x0B)  /*!< Select YC input mode and signal timing features */

/*!< 6. TPI AVI Input and Output Format Data */
#define SIL9022X_IN_FMT                             (0x09)
#define SIL9022X_OUT_FMT                            (0x0A)

/*!< 7. Interrupt Enable Register */
#define SIL9022X_INTR_EN                            (0x3C)

/*!< 8. Interrupt Satus Register */
#define SIL9022X_INTR_STATUS                        (0x3D)

/*!< 9. Video Mode */
#define SIL9022X_VM_PIXELCLOCK_LSB                  (0x00)  /*!< Pixel Clock / 10000 */
#define SIL9022X_VM_PIXELCLOCK_MSB                  (0x01)
#define SIL9022X_VM_VFREQ_LSB                       (0x02)  /*!< Vertical Frequency in Hz  */
#define SIL9022X_VM_VFREQ_MSB                       (0x03)
#define SIL9022X_VM_HPIXELS_LSB                     (0x04)  /*!< Total Pixels per line */
#define SIL9022X_VM_HPIXELS_MSB                     (0x05)
#define SIL9022X_VM_LINES_LSB                       (0x06)  /*!< Total Pixels per line */
#define SIL9022X_VM_LINES_MSB                       (0x07)

/*!< 10. System Control */
#define SIL9022X_SYSTEM                             (0x1A)

/*!< 11. Audio */
#define SIL9022X_AUDIO_HANDING		                (0x25)
#define SIL9022X_AUDIO_INTF		                    (0x26)
#define SIL9022X_AUDIO_FREQ		                    (0x27)

/*!< 12. Page */
#define SIL9022X_SET_PAGE		                    (0xBC)
#define SIL9022X_SET_OFFSET		                    (0xBD)
#define SIL9022X_RW_ACCESS		                    (0xBE)

/*!< Register Bit Mask */
/*!< -------------------------------------------------------------- */
/*!< SIL9022X_TMDS */
#define SIL9022X_TMDS_TPIEN                         (0x00)  /*!< write 0x00 to 0xC7 to enable TPI mode */

/*!< SIL9022X_POWER_UP */
#define SIL9022X_PWR_UP_STAT_OFFSET                 (0)
#define SIL9022X_PWR_UP_STAT_MASK                   (0x03)
#define SIL9022X_PWR_UP_STAT_BITS(x)                (((x) << SIL9022X_PWR_UP_STAT_OFFSET) & SIL9022X_PWR_UP_STAT_MASK)
#define SIL9022X_PWR_UP_STAT_D0                     SIL9022X_PWR_UP_STAT_BITS(0x00)

/*!< SIL9022X_INBUS */
/*!< a) Pixel Repetition Factor (bit[3:0]) */
#define SIL9022X_INBUS_PR_OFFSET                    (0)
#define SIL9022X_INBUS_PR_MASK                      (0x0F)
#define SIL9022X_INBUS_PR_BITS(x)                   (((x) << SIL9022X_INBUS_PR_OFFSET) & SIL9022X_INBUS_PR_MASK)              
#define SIL9022X_INBUS_PR_NREPLY                    SIL9022X_INBUS_PR_BITS(0x00)
#define SIL9022X_INBUS_PR_2TIMES                    SIL9022X_INBUS_PR_BITS(0x01)
#define SIL9022X_INBUS_PR_4TIMES                    SIL9022X_INBUS_PR_BITS(0x03)

/*!< b) Edge Select (bit4). Input data is latched on the selected rising or falling clock edge */
#define SIL9022X_INBUS_ES_OFFSET                    (4)
#define SIL9022X_INBUS_ES_MASK                      (0x10)
#define SIL9022X_INBUS_ES_BITS(x)                   (((x) << SIL9022X_INBUS_ES_OFFSET) & SIL9022X_INBUS_ES_MASK)
#define SIL9022X_INBUS_ES_FALLING                   SIL9022X_INBUS_ES_BITS(0x00)
#define SIL9022X_INBUS_ES_RISING                    SIL9022X_INBUS_ES_BITS(0x01)
/*!< c) Input Bus Select (bit5). The input data bus can be either one pixel wide or one-half pixel wide. */
#define SIL9022X_INBUS_SEL_OFFSET                   (5)
#define SIL9022X_INBUS_SEL_MASK                     (0x20)
#define SIL9022X_INBUS_SEL_BITS(x)                  (((x) << SIL9022X_INBUS_SEL_OFFSET) & SIL9022X_INBUS_SEL_MASK)
#define SIL9022X_INBUS_SEL_HALF                     SIL9022X_INBUS_SEL_BITS(0x00)
#define SIL9022X_INBUS_SEL_FULL                     SIL9022X_INBUS_SEL_BITS(0x01)
/*!<
 * d) TClkSel (bit[7:6])
 *  If the video host drives in data using anything other than a 1:1 ratio of input clock speed to TMDS clock speed,
 *  the host must program the clock multiplier logic
 */
#define SIL9022X_INBUS_TCLK_OFFSET                  (6)
#define SIL9022X_INBUS_TCLK_MASK                    (0xC0)
#define SIL9022X_INBUS_TCLK_BITS(x)                 (((x) << SIL9022X_INBUS_TCLK_OFFSET) & SIL9022X_INBUS_TCLK_MASK)
#define SIL9022X_INBUS_TCLK_0_5                     SIL9022X_INBUS_TCLK_BITS(0x00)
#define SIL9022X_INBUS_TCLK_1                       SIL9022X_INBUS_TCLK_BITS(0x01)
#define SIL9022X_INBUS_TCLK_2                       SIL9022X_INBUS_TCLK_BITS(0x02)
#define SIL9022X_INBUS_TCLK_4                       SIL9022X_INBUS_TCLK_BITS(0x03)

/*!< SIL9022X_IN_FMT */
/*!< a) Input Color Space (bit[1:0]) */
#define SIL9022X_IN_FMT_SPACE_OFFSET                (0)
#define SIL9022X_IN_FMT_SPACE_MASK                  (0x03)
#define SIL9022X_IN_FMT_SPACE_BITS(x)               (((x) << SIL9022X_IN_FMT_SPACE_OFFSET) & SIL9022X_IN_FMT_SPACE_MASK)
#define SIL9022X_IN_FMT_SPACE_RGB                   SIL9022X_IN_FMT_SPACE_BITS(0x00)
#define SIL9022X_IN_FMT_SPACE_YCbCr444              SIL9022X_IN_FMT_SPACE_BITS(0x01)
#define SIL9022X_IN_FMT_SPACE_YCbCr422              SIL9022X_IN_FMT_SPACE_BITS(0x02)
#define SIL9022X_IN_FMT_SPACE_BLACK                 SIL9022X_IN_FMT_SPACE_BITS(0x03)
/*!< b) Video Range Expansion (bit[3:2]) */
#define SIL9022X_IN_FMT_VRE_OFFSET                  (2)
#define SIL9022X_IN_FMT_VRE_MASK                    (0x0C)
#define SIL9022X_IN_FMT_VRE_BITS(x)                 (((x) << SIL9022X_IN_FMT_VRE_OFFSET) & SIL9022X_IN_FMT_VRE_MASK)
#define SIL9022X_IN_FMT_VRE_AUTO                    SIL9022X_IN_FMT_VRE_BITS(0x00)      /*!< selected by [1:0] */
#define SIL9022X_IN_FMT_VRE_ON                      SIL9022X_IN_FMT_VRE_BITS(0x01)
#define SIL9022X_IN_FMT_VRE_OFF                     SIL9022X_IN_FMT_VRE_BITS(0x02)
/*!< c) Input Color Depth (bit[7:6]) */
#define SIL9022X_IN_FMT_DEPTH_OFFSET                (6)
#define SIL9022X_IN_FMT_DEPTH_MASK                  (0xC0)
#define SIL9022X_IN_FMT_DEPTH_BITS(x)               (((x) << SIL9022X_IN_FMT_DEPTH_OFFSET) & SIL9022X_IN_FMT_DEPTH_MASK)
#define SIL9022X_IN_FMT_DEPTH_8BIT                  SIL9022X_IN_FMT_DEPTH_BITS(0x00)
#define SIL9022X_IN_FMT_DEPTH_UN10BIT               SIL9022X_IN_FMT_DEPTH_BITS(0x02)    /*!< 10/12-bit un-dithered for 4:2:2 mode */
#define SIL9022X_IN_FMT_DEPTH_10BIT                 SIL9022X_IN_FMT_DEPTH_BITS(0x02)    /*!< 10/12-bit dithered to for 4:2:2 mode */

/*!< SIL9022X_OUT_FMT */
/*!< a) Output Color Space (bit[1:0]) */
#define SIL9022X_OUT_FMT_SPACE_OFFSET               (0)
#define SIL9022X_OUT_FMT_SPACE_MASK                 (0x03)
#define SIL9022X_OUT_FMT_SPACE_BITS(x)              (((x) << SIL9022X_OUT_FMT_SPACE_OFFSET) & SIL9022X_OUT_FMT_SPACE_MASK)
#define SIL9022X_OUT_FMT_SPACE_RGB                  SIL9022X_OUT_FMT_SPACE_BITS(0x00)
#define SIL9022X_OUT_FMT_SPACE_YCbCr444             SIL9022X_OUT_FMT_SPACE_BITS(0x01)
#define SIL9022X_OUT_FMT_SPACE_YCbCr422             SIL9022X_OUT_FMT_SPACE_BITS(0x02)
#define SIL9022X_OUT_FMT_SPACE_RGB2                 SIL9022X_OUT_FMT_SPACE_BITS(0x03)   /*!< same as SIL9022X_OUT_FMT_SPACE_RGB */
/*!< b) Video Range Compression (bit[3:2]) */
#define SIL9022X_OUT_FMT_VRC_OFFSET                 (2)
#define SIL9022X_OUT_FMT_VRC_MASK                   (0x0C)
#define SIL9022X_OUT_FMT_VRC_BITS(x)                (((x) << SIL9022X_OUT_FMT_VRC_OFFSET) & SIL9022X_OUT_FMT_VRC_MASK)
#define SIL9022X_OUT_FMT_VRC_AUTO                   SIL9022X_OUT_FMT_VRC_BITS(0x00)     /*!< selected by [1:0] */
#define SIL9022X_OUT_FMT_VRC_OFF                    SIL9022X_OUT_FMT_VRC_BITS(0x01)
#define SIL9022X_OUT_FMT_VRC_ON                     SIL9022X_OUT_FMT_VRC_BITS(0x02)
/*!< c) Color Space Standard (bit4) */
#define SIL9022X_OUT_FMT_CSS_OFFSET                 (4)  
#define SIL9022X_OUT_FMT_CSS_MASK                   (0x10)
#define SIL9022X_OUT_FMT_CSS_BITS(x)                (((x) << SIL9022X_OUT_FMT_CSS_OFFSET) & SIL9022X_OUT_FMT_CSS_MASK)     
#define SIL9022X_OUT_FMT_CSS_BT601                  SIL9022X_OUT_FMT_CSS_BITS(0x00)
#define SIL9022X_OUT_FMT_CSS_BT709                  SIL9022X_OUT_FMT_CSS_BITS(0x01)
/*!< c) Output Color Depth (bit[7:6]) */
#define SIL9022X_OUT_FMT_DEPTH_OFFSET               (6)
#define SIL9022X_OUT_FMT_DEPTH_MASK                 (0xC0)
#define SIL9022X_OUT_FMT_DEPTH_BITS(x)              (((x) << SIL9022X_OUT_FMT_DEPTH_OFFSET) & SIL9022X_OUT_FMT_DEPTH_MASK)
#define SIL9022X_OUT_FMT_DEPTH_8BIT                 SIL9022X_OUT_FMT_DEPTH_BITS(0x00)

/*!< SIL9022X_INTR_EN / SIL9022X_INTR_STATUS */
#define SIL9022X_INTR_HP_CONN                       (0x01)  /*!< Hot Plug / Connection Event */
#define SIL9022X_INTR_RxS_EVENT                     (0x02)  /*!< Receiver Sense Event */
/*!< 
 * SIL9022X_INTR_EN: rsvd
 * SIL9022X_INTR_STATUS:
 *      0 – HP Low or no event pending
 *      1 – HP High or CTRL event pending 
 */
#define SIL9022X_INTR_HP_STATE                      (0x04)
#define SIL9022X_INTR_RxS_CPI                       (0x08)
#define SIL9022X_INTR_AUDIO_ERR                     (0x10)
#define SIL9022X_INTR_SECU_CHANGE                   (0x20)
#define SIL9022X_INTR_HDCP_READY                    (0x40)
#define SIL9022X_INTR_HDCP_AUTH                     (0x80)

/*!< SIL9022X_SYSTEM */
/*!< 1) Output Mode Select (bit0): Write in Hot Plug Service Loop*/
#define SIL9022X_SYSTEM_OUTHDMI                     (0x01)
#define SIL9022X_SYSTEM_OUTDVI                      (!SIL9022X_SYSTEM_OUTHDMI)
/*!< 2) DDC Bus Grant by TPI Read (bit3) */
#define SIL9022X_SYSTEM_AVMUTE_HDMI                 (0x08)
#define SIL9022X_SYSTEM_AVMUTE_NORMAL               (!SIL9022X_SYSTEM_AVMUTE_HDMI)
/*!< 3) TMDS Output Control (bit4) */
#define SIL9022X_SYSTEM_TMS_POWER                   (0x10)
#define SIL9022X_SYSTEM_TMS_ACTIVE                  (!SIL9022X_SYSTEM_TMS_POWER)

/*!< SIL9022X_AUDIO */
#define SIL9022X_AUDIO_PASS_BASIC                   (0x00 << 0)
#define SIL9022X_AUDIO_INTF_I2S                     (0x02 << 6)
#define SIL9022X_AUDIO_INTF_NORMAL                  (0x00 << 4)
#define SIL9022X_AUDIO_TYPE_PCM                     (0x01 << 0)
#define SIL9022X_AUDIO_SAMP_SIZE_16BIT              (0x01 << 6)
#define SIL9022X_AUDIO_SAMP_FREQ_44K                (0x02 << 3)

/*!< SIL9022X_PAGE/OFFSET/RW */
#define SIL9022X_SET_PAGE_SII9022A                  (0x01)
#define SIL9022X_SET_OFFSET_SII9022A                (0x82)
#define SIL9022X_RW_EN_SRC_TERMIN                   (0x01)

/*!< Sil9022x device data */
typedef struct sil9022x_drv_info
{
    kchar_t *name;
    struct fwk_i2c_client *sptr_client;

    struct fwk_gpio_desc *sptr_rstgpio;
    kint32_t irq;

    struct fwk_device sgtc_dev;
    struct fwk_notifier_block sgtc_nb;

    struct workqueue sgtc_wq;
    kbool_t is_connected;

} sil9022x_drv_info_t;

/*!< API function */
/*!
 * @brief  sil9022a interrupt handler
 * @param  args
 * @retval none
 * @note   none
 */
static irq_return_t sil9022x_isr(kint32_t irq, void *args)
{
    struct sil9022x_drv_info *sptr_drv;

    sptr_drv = (struct sil9022x_drv_info *)args;
    schedule_work(&sptr_drv->sgtc_wq);

    return NR_IRQ_HANDLED;
}

/*!
 * @brief  write sil9022x
 * @param  sptr_client
 * @retval none
 * @note   delay 1ms before writting to wait for stable
 */
kint32_t sil9022x_write_data(struct fwk_i2c_client *sptr_client, kuint8_t reg, kuint8_t data)
{
    kint32_t value;

    do {
        value = fwk_i2c_write_byte_data(sptr_client, reg, data);
        mdelay(1);

    } while (value < 0);

    return value;
}

/*!
 * @brief  read sil9022x
 * @param  sptr_client
 * @retval none
 * @note   delay 1ms before reading to wait for stable
 */
kint32_t sil9022x_read_data(struct fwk_i2c_client *sptr_client, kuint8_t reg)
{
    kint32_t value;

    do {
        value = fwk_i2c_read_byte_data(sptr_client, reg);
        mdelay(1);

    } while (value < 0);

    return value;
}

/*!
 * @brief  sil9022a interrupt bottom handler
 * @param  sptr_wq
 * @retval none
 * @note   Hot plug event dectecting
 */
static void sil9022x_detect_work(struct workqueue *sptr_wq)
{
    struct sil9022x_drv_info *sptr_drv;
    kubyte_t value;

    sptr_drv = mr_container_of(sptr_wq, struct sil9022x_drv_info, sgtc_wq);
    value = sil9022x_read_data(sptr_drv->sptr_client, SIL9022X_INTR_STATUS);

    /*!< HDMI plug in */
    if (!sptr_drv->is_connected && (value & SIL9022X_INTR_HP_STATE))
    {
        /*!< Power on: set TPI system control (DVI, not HDMI) */
        sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_SYSTEM, 
                        SIL9022X_SYSTEM_OUTDVI | SIL9022X_SYSTEM_TMS_ACTIVE | SIL9022X_SYSTEM_AVMUTE_NORMAL);

        print_info("HDMI device plug in\r\n");
        sptr_drv->is_connected = true;
    }
    /*!< HDMI plug out */
    else if (sptr_drv->is_connected)
    {
        /*!< Power off */
        sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_SYSTEM, 
                        SIL9022X_SYSTEM_OUTDVI | SIL9022X_SYSTEM_TMS_POWER | SIL9022X_SYSTEM_AVMUTE_NORMAL);
        sptr_drv->is_connected = false;

        print_info("HDMI device plug out\r\n");
    }

    /*!< Clear interrupt status */
    sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_INTR_STATUS, value);
}

/*!
 * @brief  initialize sil9022a
 * @param  sptr_drv
 * @retval errno
 * @note   none
 */
static kint32_t sil9022x_init(struct sil9022x_drv_info *sptr_drv, struct fwk_fb_notifier_param *sptr_param)
{
    struct fwk_fb_var_screen_info *sptr_var;
    kint32_t loop_cnt = 10, value[4];
    kuint16_t pixel_hz_10KHz, line_pixels, lines;
    kuint32_t refresh;
    kuint8_t reg;
    kint32_t retval;

    /*!< Reset sil9022a, write 0 to reset */
    fwk_gpio_set_value(sptr_drv->sptr_rstgpio, 0);
    msleep(100);
    fwk_gpio_set_value(sptr_drv->sptr_rstgpio, 1);
    msleep(200);

    /*!< Try to connect TMDS (test slave address for the first time) */
    retval = sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_TMDS, SIL9022X_TMDS_TPIEN);
    if (retval)
    {
        print_err("Reset and init sil9022a failed!\r\n");
        return -ER_NODEV;
    }

    while (--loop_cnt)
    {
        /*!< Read ID */
        value[0] = sil9022x_read_data(sptr_drv->sptr_client, SIL9022X_DEVICE_ID);
        if (value[0] != 0xB0)
            continue;

        value[1] = sil9022x_read_data(sptr_drv->sptr_client, SIL9022X_PRODUCT_ID);
        value[2] = sil9022x_read_data(sptr_drv->sptr_client, SIL9022X_REVISION_ID);
        value[3] = sil9022x_read_data(sptr_drv->sptr_client, SIL9022X_HDCP_REVISION);

        print_info("Detect sil9022a device:\r\n");
        print_info("    id: 0x%02x, product id: 0x%02x, revision id: 0x%02x, hdcp revision: 0x%02x\r\n", 
                value[0], value[1], value[2], value[3]);
        
        break;
    }

    if (!loop_cnt)
    {
        print_err("Cound not get sil9022a device id, acknowlege failed!\r\n");
        return -ER_NODEV;
    }

    /*!< Power up */
    reg = (kuint8_t)sil9022x_read_data(sptr_drv->sptr_client, SIL9022X_POWER_UP);
    reg = (reg & (~SIL9022X_PWR_UP_STAT_MASK)) | SIL9022X_PWR_UP_STAT_D0;
    retval = sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_POWER_UP, reg);
    if (retval < 0)
    {
        print_err("Cound not power up sil9022a device!\r\n");
        return -ER_FAILD;
    }

    /*!< Power off */
    sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_SYSTEM, 
                    SIL9022X_SYSTEM_OUTDVI | SIL9022X_SYSTEM_TMS_POWER | SIL9022X_SYSTEM_AVMUTE_NORMAL);

    /*!< Enable source termination */
    retval |= sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_SET_PAGE, SIL9022X_SET_PAGE_SII9022A);
    retval |= sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_SET_OFFSET, SIL9022X_SET_OFFSET_SII9022A);
    if (retval < 0)
    {
        print_err("Cound not enable source termination for sil9022a device!\r\n");
        return -ER_FAILD;
    }

    reg = (kuint8_t)sil9022x_read_data(sptr_drv->sptr_client, SIL9022X_RW_ACCESS);
    reg |= SIL9022X_RW_EN_SRC_TERMIN;
    sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_RW_ACCESS, reg);

    /*!< TPI video mode */
    sptr_var = sptr_param->sptr_var;

    /*!< sptr_var->pixclock: picoseconds */
    pixel_hz_10KHz = FB_PICOS_2_KHZ(sptr_var->pixclock) / 10;
    line_pixels = sptr_var->hsync_len + sptr_var->left_margin + sptr_var->right_margin + sptr_var->xres;
    lines = sptr_var->vsync_len + sptr_var->upper_margin + sptr_var->lower_margin + sptr_var->yres;
    /*!< For 720p@60Hz, refresh = 60 * 100 = 6000 */
    refresh = ((FB_PICOS_2_KHZ(sptr_var->pixclock) * 1000) / (line_pixels * lines)) * 100;

    retval |= sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_VM_PIXELCLOCK_LSB, (kuint8_t)pixel_hz_10KHz);
    retval |= sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_VM_PIXELCLOCK_MSB, (kuint8_t)(pixel_hz_10KHz >> 8));
    retval |= sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_VM_VFREQ_LSB, (kuint8_t)refresh);
    retval |= sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_VM_VFREQ_MSB, (kuint8_t)(refresh >> 8));
    retval |= sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_VM_HPIXELS_LSB, (kuint8_t)line_pixels);
    retval |= sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_VM_HPIXELS_MSB, (kuint8_t)(line_pixels >> 8));
    retval |= sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_VM_LINES_LSB, (kuint8_t)lines);
    retval |= sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_VM_LINES_MSB, (kuint8_t)(lines >> 8));
    if (retval < 0)
    {
        print_err("Configure pixel clock and resolution to sil9022a failed!\r\n");
        return -ER_FAILD;
    }

    /*!< Configure Input Bus and Pixel Repetition */
    retval |= sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_INBUS, 
                        SIL9022X_INBUS_PR_NREPLY | SIL9022X_INBUS_TCLK_1 | SIL9022X_INBUS_ES_RISING | SIL9022X_INBUS_SEL_FULL);

    /*!< Input Format: RGB */
    retval |= sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_IN_FMT, 
                        SIL9022X_IN_FMT_SPACE_RGB | SIL9022X_IN_FMT_VRE_AUTO | SIL9022X_IN_FMT_DEPTH_8BIT);

    /*!< Output Format: RGB */
    retval |= sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_OUT_FMT, 
                        SIL9022X_OUT_FMT_SPACE_RGB | SIL9022X_OUT_FMT_VRC_AUTO | SIL9022X_OUT_FMT_DEPTH_8BIT);
    if (retval < 0)
    {
        print_err("Configure sil9022a input-bus and I/O format failed!\r\n");
        return -ER_FAILD;
    }

    /*!< Interrupt Enable */
    if (sptr_drv->irq >= 0)
    {
        sil9022x_write_data(sptr_drv->sptr_client, SIL9022X_INTR_EN, SIL9022X_INTR_HP_CONN);
        fwk_enable_irq(sptr_drv->irq);
    }

    /*!< Perhaps HDMI has been plug, but irq was closed. Call work_func after initilizing imediately */
    schedule_work(&sptr_drv->sgtc_wq);
    return ER_NORMAL;
}

/*!
 * @brief   notifier callback
 * @param   sptr_nb
 * @retval  errno
 * @note    none
 */
kint32_t sil9022x_hdmi_action(struct fwk_notifier_block *sptr_nb, kuint32_t event, void *args)
{
    struct sil9022x_drv_info *sptr_drv;
    struct fwk_fb_notifier_param *sptr_param;

    sptr_drv = (struct sil9022x_drv_info *)sptr_nb->data;
    sptr_param = (struct fwk_fb_notifier_param *)args;

    switch (event)
    {
        case FB_NOTIFIER_HDMI_OPEN:
            sil9022x_init(sptr_drv, sptr_param);
            break;

        case FB_NOTIFIER_HDMI_CLOSE:
            break;

        default: break;
    }

    return ER_NORMAL;
}

/*!
 * @brief   notifier callback
 * @param   sptr_nb
 * @retval  errno
 * @note    none
 */
kint32_t sil9022x_hdmi_wakeup(struct fwk_notifier_block *sptr_nb, kuint32_t event, void *args)
{
    struct sil9022x_drv_info *sptr_drv;

    sptr_drv = (struct sil9022x_drv_info *)sptr_nb->data;

    if ((event == FB_NOTIFIER_HDMI_OPEN) ||
        (event == FB_NOTIFIER_HDMI_CLOSE))
        return sptr_drv->is_connected ? event : -ER_CHECKERR;

    return -ER_CHECKERR;
}

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   sil9022x_driver_probe
 * @param   sptr_client
 * @retval  errno
 * @note    none
 */
static kint32_t sil9022x_driver_probe(struct fwk_i2c_client *sptr_client, const struct fwk_i2c_device_id *sptr_id)
{
    struct sil9022x_drv_info *sptr_drv;

    sptr_drv = kzalloc(sizeof(*sptr_drv), GFP_KERNEL);
    if (!isValid(sptr_drv))
        return -ER_NOMEM;

    sptr_drv->sptr_rstgpio = fwk_gpio_desc_get(&sptr_client->sgtc_dev, "reset", 0);
    if (!isValid(sptr_drv->sptr_rstgpio))
        goto fail;

    fwk_gpio_set_direction_output(sptr_drv->sptr_rstgpio, 1);

    sptr_drv->name = "sil9022a";
    sptr_drv->sptr_client = sptr_client;
    sptr_drv->irq = -1;
    INIT_WORK(&sptr_drv->sgtc_wq, sil9022x_detect_work);

    /*!< Interrupt Enable */
    if (sptr_drv->sptr_client->irq >= 0)
    {
        if (!fwk_request_irq(sptr_drv->sptr_client->irq, sil9022x_isr, 
                        IRQ_TYPE_EDGE_FALLING, "sil9022a", sptr_drv))
        {
            sptr_drv->irq = sptr_drv->sptr_client->irq;
            fwk_disable_irq(sptr_drv->irq);
        }
    }

    fwk_device_initial(&sptr_drv->sgtc_dev);
    sptr_drv->sgtc_dev.sptr_parent = &sptr_client->sgtc_dev;
    mr_dev_set_name(&sptr_drv->sgtc_dev, sptr_drv->name);
    fwk_device_add(&sptr_drv->sgtc_dev);

    fwk_i2c_set_client_data(sptr_client, sptr_drv);

    /*!< Configure Notifier Chain */
    sptr_drv->sgtc_nb.data = sptr_drv;
    sptr_drv->sgtc_nb.notifier_call = sil9022x_hdmi_action;
    sptr_drv->sgtc_nb.pengding_call = sil9022x_hdmi_wakeup;
    sptr_drv->sgtc_nb.expect_event = FB_NOTIFIER_HDMI_OPEN | FB_NOTIFIER_HDMI_CLOSE;
    init_list_head(&sptr_drv->sgtc_nb.sgtc_link);
    fwk_blocking_notifier_chain_register(&sgtc_fbmem_notifier_chain, &sptr_drv->sgtc_nb);

	return ER_NORMAL;

fail:
    kfree(sptr_drv);
    return -ER_FAILD;
}

/*!
 * @brief   sil9022x_driver_remove
 * @param   sptr_client
 * @retval  errno
 * @note    none
 */
static kint32_t sil9022x_driver_remove(struct fwk_i2c_client *sptr_client)
{
    struct sil9022x_drv_info *sptr_drv;

    sptr_drv = fwk_i2c_get_client_data(sptr_client);

    fwk_blocking_notifier_chain_unregister(&sgtc_fbmem_notifier_chain, &sptr_drv->sgtc_nb);

    if (sptr_drv->irq >= 0)
    {
        fwk_disable_irq(sptr_drv->irq);
        fwk_free_irq(sptr_drv->irq, sptr_drv);
    }

    fwk_gpio_desc_put(sptr_drv->sptr_rstgpio);
    kfree(sptr_drv);
    fwk_i2c_set_client_data(sptr_client, mr_nullptr);

    return ER_NORMAL;
}

static const struct fwk_i2c_device_id sgtc_sil9022x_driver_ids[] =
{
    { .name = "sil9022a", .driver_data = -1 },
    {},
};

/*!< device id for device-tree */
static const struct fwk_of_device_id sgtc_sil9022x_driver_dts[] =
{
	{ .compatible = "silicon,sil9022a", },
	{},
};

/*!< platform instance */
static struct fwk_i2c_driver sgtc_sil9022x_driver =
{
	.probe	= sil9022x_driver_probe,
	.remove	= sil9022x_driver_remove,

    .sptr_id_table = sgtc_sil9022x_driver_ids,
	
	.sgtc_driver =
	{
		.name 	= "sil9022a,hdmi-phy",
		.id 	= -1,
		.sptr_of_match_table = sgtc_sil9022x_driver_dts,
	},
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   sil9022x_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init sil9022x_driver_init(void)
{
	return fwk_i2c_add_driver(&sgtc_sil9022x_driver);
}

/*!
 * @brief   sil9022x_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit sil9022x_driver_exit(void)
{
	fwk_i2c_del_driver(&sgtc_sil9022x_driver);
}

IMPORT_DRIVER_INIT(sil9022x_driver_init);
IMPORT_DRIVER_EXIT(sil9022x_driver_exit);

/*!< end of file */
