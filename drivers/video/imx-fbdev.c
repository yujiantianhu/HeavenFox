/*
 * Video Driver : LCD with framebuffer
 *
 * File Name:   imx_fbdev.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.03.25
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/base/fwk_inode.h>
#include <platform/base/fwk_fs.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/base/fwk_platdrv.h>
#include <platform/clk/fwk_clk.h>
#include <platform/base/fwk_pinctrl.h>
#include <platform/gpio/fwk_gpiodesc.h>
#include <platform/base/fwk_uaccess.h>
#include <platform/video/fwk_fbmem.h>
#include <platform/video/fwk_rgbmap.h>
#include <kernel/sleep.h>
#include <kernel/wait.h>

#include <imx6/imx6ull_periph.h>
#include <imx6/imx6ull_pins.h>

/*!< The defines */
struct imx_fbdev_trigger
{
    kuint32_t hsync_active;
    kuint32_t vsync_active;
    kuint32_t de_active;
    kuint32_t pixelclk_active;
    kuint32_t bus_width;
};

struct imx_fbdev_backlight
{
    struct fwk_device *sptr_par;
    struct fwk_pinctrl *sptr_pctl;
    struct fwk_gpio_desc *sptr_gdesc;

    struct fwk_device *sptr_dev;
};

enum __ERT_IMX_FBDEV_INTERFACE
{
    NR_IMX_FBDEV_ELCDIF = 0,
    NR_IMX_FBDEV_HDMI,
    NR_IMX_FBDEV_MIPI,
    NR_IMX_FBDEV_DSI,
};

struct imx_fbdev_drv
{
    kuint32_t minor;

    void *base;
    struct fwk_fb_info *sptr_fb;
    struct fwk_device *sptr_dev;

    struct imx_fbdev_trigger sgtc_phase;
    struct fwk_clk *sptr_clk[3];

    kuint32_t interface_type;
    struct imx_fbdev_backlight sgtc_blight;

    struct wait_queue_head sgtc_wqh;
};

#define FBDEV_IMX_DRIVER_MINOR					0

/* The globals */

/*!< API function */
/*!
 * @brief   imx_fbdev_init
 * @param   base
 * @retval  errno
 * @note    none
 */
static void imx_fbdev_init(void *base, struct imx_fbdev_drv *sptr_drv)
{
    srt_imx_lcdif_t *sptr_lcdif = (srt_imx_lcdif_t *)base;
    struct fwk_fb_fix_screen_info *sptr_fix;
    struct fwk_fb_var_screen_info *sptr_var;
    kuaddr_t reg;

    if (!sptr_lcdif || !sptr_drv)
        return;

    sptr_fix = &sptr_drv->sptr_fb->sgtc_fix;
    sptr_var = &sptr_drv->sptr_fb->sgtc_var;

    /*!< 
     * CTRLn: eLCDIF General Control Register
     * bit31: SFTRST. This bit must be set to zero to enable normal operation of the eLCDIF. When set to one, it forces a block level reset.
     * bit30: CLKGATE. This bit must be set to zero for normal operation. When set to one it gates off the clocks to the block
     * bit19: BYPASS_OUNT. This bit must be 0 in MPU and VSYNC modes, and must be 1 in DOTCLK and DVI modes of operation.
     * bit17: Set this bit to 1 to make the hardware go into the DOTCLK mode, i.e. VSYNC/HSYNC/DOTCLK/ENABLE interface mode
     * bit[11:10]: LCD Data bus transfer width. 0x0(16 bits), 0x1(8 bits), 0x2(18bits), 0x3(24bits)
     * bit[9:8]: Input data format. 0x0(16 bits), 0x1(8 bits), 0x2(18bits), 0x3(24bits)
     * bit5: Set this bit to make the eLCDIF act as a bus master.
     * bit0: When this bit is set by software, the eLCDIF will begin transferring data between the SoC and the display.
     *		 This bit must remain set until the operation is complete. 
     */
    /*!< clear bit30 */
    mr_writel(mr_bit(30), &sptr_lcdif->CTRL_CLR);
    while (mr_isBitSetl(mr_bit(30), &sptr_lcdif->CTRL));

    /*!< set bit31 */
    mr_writel(mr_bit(31), &sptr_lcdif->CTRL_SET);
    while (mr_isBitResetl(mr_bit(31), &sptr_lcdif->CTRL));

    msleep(50);

    /*!< clear bit30 & bit31 */
    mr_writel(mr_bit(31), &sptr_lcdif->CTRL_CLR);
    mr_writel(mr_bit(30), &sptr_lcdif->CTRL_CLR);

    mr_writel(mr_bit(19) | mr_bit(17) | mr_bit(5), &reg);

    switch (sptr_var->bits_per_pixel)
    {
        case 16:
//			mr_setbitl(mr_bit_nr(0x0, 8), &reg);
//			break;
        case 24:
        case 32:
            mr_setbitl(mr_bit_nr(0x03, 8), &reg);
            break;
        default:
            break;
    }

    switch (sptr_drv->sgtc_phase.bus_width)
    {
        case 16:
            mr_setbitl(mr_bit_nr(0x0, 10), &reg);
            break;
        case 24:
        case 32:
            mr_setbitl(mr_bit_nr(0x3, 10), &reg);
            break;
        default:
            break;
    }

    mr_writel(reg, &sptr_lcdif->CTRL);

    /*!<
     * CTRL1: eLCDIF General Control Register 1
     * bit[19:16]: BYTE_PACKING_FORMAT. 
     * 		set the bit field value to 0x7 if the display data is arranged in the 24-bit unpacked format 
     * 		(A-R-G-B where A value does not have be transmitted).
     */
    mr_writel(mr_bit_nr(0x7, 16), &sptr_lcdif->CTRL1);

    /*!<
     * TRANSFER_COUNT: eLCDIF Horizontal and Vertical Valid Data Count Register
     * bit[31:16]: Number of horizontal lines per frame which contain valid data.
     * bit[15:0]: Total valid data (pixels) in each horizontal line
     */
    mr_writel(mr_mask(sptr_var->yres << 16, 0xffff0000) | mr_mask(sptr_var->xres, 0xffff), &sptr_lcdif->TRANSFER_COUNT);

    /*!<
     * VDCTRL0: eLCDIF VSYNC Mode and Dotclk Mode Control Register 0
     * bit29: 
     * bit28: Setting this bit to 1 will make the hardware generate the ENABLE signal in the DOTCLK mode
     * bit21: VSYNC_PERIOD_UNIT. Set it to 1 to count in terms of complete horizontal lines (used in the DOTCLK mode)
     * bit20: VSYNC_PULSE_WIDTH_UNIT. Set it to 1 to count in terms of complete horizontal lines.
     * bit[17:0]: VSYNC_PULSE_WIDTH ===> vsync-len
     * 
     * bit27: VSYNC_POL (0: low; 1: high)
     * bit26: HSYNC_POL (0: low; 1: high)
     * bit25: DOTCLK_POL (1: rising edge; 0: falling edge)
     * bit24: ENABLE_POL (0: low; 1: high)
     */
    mr_resetl(&reg);
    mr_setbitl(mr_bit(28) | mr_bit(21) | mr_bit(20), &reg);
    mr_setbitl(mr_mask(sptr_var->vsync_len, 0x3ffff), &reg);
    mr_setbitl(mr_bit_nr(!!sptr_drv->sgtc_phase.vsync_active, 27), &reg);
    mr_setbitl(mr_bit_nr(!!sptr_drv->sgtc_phase.hsync_active, 26), &reg);
    mr_setbitl(mr_bit_nr(!sptr_drv->sgtc_phase.pixelclk_active, 25), &reg);
    mr_setbitl(mr_bit_nr(!!sptr_drv->sgtc_phase.de_active, 24), &reg);
    mr_writel(reg, &sptr_lcdif->VDCTRL0);

    /*!<
     * VDCTRL1: eLCDIF VSYNC Mode and Dotclk Mode Control Register 1
     * bit[31:0]: D Total number of units between two positive or two negative edges of the VSYNC signal
     */
    mr_writel(sptr_var->yres + sptr_var->vsync_len + sptr_var->lower_margin + sptr_var->upper_margin, &sptr_lcdif->VDCTRL1);

    /*!<
     * VDCTRL2: eLCDIF VSYNC Mode and Dotclk Mode Control Register 2
     * bit[31:18]: Number of DISPLAY CLOCK (pix_clk) cycles for which HSYNC signal is active
     * bit[17:0]: Total number of DISPLAY CLOCK (pix_clk) cycles between two positive or two negative edges of the HSYNC signal.
     */
    mr_writel(mr_mask(sptr_var->hsync_len << 18, 0x3ffff), &sptr_lcdif->VDCTRL2);
    mr_setbitl(sptr_var->xres + sptr_var->hsync_len + sptr_var->right_margin + sptr_var->left_margin, &sptr_lcdif->VDCTRL2);
    
    /*!<
     * VDCTRL3: eLCDIF VSYNC Mode and Dotclk Mode Control Register 3
     * bit[27:16]: HORIZONTAL_WAIT_CNT
     * bit[15:0]: VERTICAL_WAIT_CNT
     */
    mr_writel(mr_mask((sptr_var->left_margin + sptr_var->hsync_len) << 16, 0xfff0000), &sptr_lcdif->VDCTRL3);
    mr_setbitl(sptr_var->upper_margin + sptr_var->vsync_len, &sptr_lcdif->VDCTRL3);

    /*!<
     * VDCTRL3: eLCDIF VSYNC Mode and Dotclk Mode Control Register 4
     * bit18: 	Set this field to 1 if the LCD controller requires that the VSYNC or VSYNC/HSYNC/DOTCLK control
     *			signals should be active at least one frame before the data transfers actually start and remain active at
     *			least one frame after the data transfers end
     * bit[17:0]: Total number of DISPLAY CLOCK (pix_clk) cycles on each horizontal line that carry valid data in DOTCLK mode.
     */
    mr_writel(mr_bit(18) | sptr_var->xres, &sptr_lcdif->VDCTRL4);

    /*!< CUR_BUF: Address of the current frame being transmitted by eLCDIF. */
    mr_writel(sptr_fix->smem_start, &sptr_lcdif->CUR_BUF);
    /*!< NEXT_BUF: Address of the next frame that will be transmitted by eLCDIF */
    mr_writel(sptr_fix->smem_start, &sptr_lcdif->NEXT_BUF);

    sleep(1);
}

/*!
 * @brief   step to turn backlight on
 * @param   sptr_drv
 * @retval  none
 * @note    none
 */
static void imx_fbdev_backlight_enable(struct imx_fbdev_drv *sptr_drv)
{
    srt_hal_imx_pin_t sgtc_cfg;

    if (!sptr_drv->sgtc_blight.sptr_gdesc)
        return;

    hal_imx_pin_attribute_init(&sgtc_cfg, IMX6UL_PIN_ADDR_BASE, IMX6UL_MUX_GPIO1_IO08_GPIO1_IO08, 0, 0);

    /*!< Soft Start */
    for (kint32_t i = 0; i < 5; i++)
    {
        mr_clrbitl(IMX6UL_IO_CTL_PAD_DSE_BIT(IMX6UL_IO_CTL_PAD_DSE_MASK), &sgtc_cfg.pad_base);
        mr_setbitl(IMX6UL_IO_CTL_PAD_DSE_BIT(IMX6UL_IO_CTL_PAD_DSE_RDIV(i + 1)), &sgtc_cfg.pad_base);

        fwk_gpio_set_value(sptr_drv->sgtc_blight.sptr_gdesc, 1);
        msleep((i + 1) * 10);
        fwk_gpio_set_value(sptr_drv->sgtc_blight.sptr_gdesc, 0);
        msleep(50 - (i * 10));
    }

    mr_clrbitl(IMX6UL_IO_CTL_PAD_DSE_BIT(IMX6UL_IO_CTL_PAD_DSE_MASK), &sgtc_cfg.pad_base);
    mr_setbitl(IMX6UL_IO_CTL_PAD_DSE_BIT(IMX6UL_IO_CTL_PAD_DSE_RDIV(6)), &sgtc_cfg.pad_base);
    fwk_gpio_set_value(sptr_drv->sgtc_blight.sptr_gdesc, 1);
}

/*!
 * @brief   turn backlight off
 * @param   sptr_drv
 * @retval  none
 * @note    none
 */
#define imx_fbdev_backlight_disable(sptr_drv)   \
    do {    \
        if ((sptr_drv)->sgtc_blight.sptr_gdesc)   \
            fwk_gpio_set_value((sptr_drv)->sgtc_blight.sptr_gdesc, 0);    \
    } while (0)

/*!
 * @brief   call notifier: wake up interface
 * @param   sptr_drv
 * @retval  errno
 * @note    none
 */
static kint32_t imx_fbdev_wake_interface(struct imx_fbdev_drv *sptr_drv)
{
    struct fwk_fb_notifier_param *sptr_param;
    kint32_t dev_cnt = 0;

    switch (sptr_drv->interface_type)
    {
        case NR_IMX_FBDEV_ELCDIF:
            /*!< open backlight */
            imx_fbdev_backlight_enable(sptr_drv);
            dev_cnt++;
            break;
        
        case NR_IMX_FBDEV_HDMI:
            sptr_param = kmalloc(sizeof(*sptr_param), GFP_KERNEL);
            if (!isValid(sptr_param))
                return -ER_NOMEM;

            sptr_param->sptr_fix = &sptr_drv->sptr_fb->sgtc_fix;
            sptr_param->sptr_var = &sptr_drv->sptr_fb->sgtc_var;
            sptr_param->bus_width = sptr_drv->sgtc_phase.bus_width;

            dev_cnt = fwk_blocking_notifier_call_chain(
                            &sgtc_fbmem_notifier_chain, FB_NOTIFIER_HDMI_OPEN, sptr_param);
            
            kfree(sptr_param);
            break;

        default: break;
    }

    return (dev_cnt >= 0) ? dev_cnt : (-ER_NODEV);
}

/*!
 * @brief   call notifier: stop interface
 * @param   sptr_drv
 * @retval  errno
 * @note    none
 */
static void imx_fbdev_quit_interface(struct imx_fbdev_drv *sptr_drv)
{
    switch (sptr_drv->interface_type)
    {
        case NR_IMX_FBDEV_ELCDIF:
            /*!< open backlight */
            imx_fbdev_backlight_disable(sptr_drv);
            break;
        
        case NR_IMX_FBDEV_HDMI:
            fwk_blocking_notifier_call_chain(
                &sgtc_fbmem_notifier_chain, FB_NOTIFIER_HDMI_CLOSE, mr_nullptr);
            break;

        default: break;
    }
}

/*!
 * @brief   call notifier: waitting for interface
 * @param   sptr_drv
 * @retval  errno
 * @note    none
 */
static kbool_t imx_fbdev_wait_interface(struct imx_fbdev_drv *sptr_drv)
{
    kint32_t dev_cnt = 0;

    switch (sptr_drv->interface_type)
    {
        case NR_IMX_FBDEV_ELCDIF:
            dev_cnt++;
            break;
        
        case NR_IMX_FBDEV_HDMI:
            dev_cnt = fwk_blocking_pengding_call_chain(
                            &sgtc_fbmem_notifier_chain, FB_NOTIFIER_HDMI_OPEN, mr_nullptr);
            break;

        default: break;
    }

    return !!(dev_cnt > 0);
}

/*!
 * @brief   driver open
 * @param   sptr_inode, user
 * @retval  errno
 * @note    none
 */
static kint32_t imx_fbdev_open(struct fwk_fb_info *sptr_info, kint32_t user)
{
    struct imx_fbdev_drv *sptr_drv;
    srt_imx_lcdif_t *sptr_lcdif;
    kint32_t retval;

    sptr_drv = fwk_fb_get_drvdata(sptr_info);
    sptr_lcdif = (srt_imx_lcdif_t *)sptr_drv->base;

    /*!< Enable interface */
    retval = imx_fbdev_wake_interface(sptr_drv);
    if (retval < 0)
        return retval;
    else if (retval == 0)
        wait_event_interruptible_timeout(&sptr_drv->sgtc_wqh, 
                            imx_fbdev_wait_interface(sptr_drv), msecs_to_jiffies(100));

    /*!< Enable LCD */
    mr_writel(mr_bit(17) | mr_bit(0), &sptr_lcdif->CTRL_SET);
    print_info("fbdev is opened\r\n");

    memset_ex(sptr_info->screen_base, RGB_WHITE, sptr_info->screen_size);
    print_info("clear full screen with black color\r\n");

    return ER_NORMAL;
}

/*!
 * @brief   driver close
 * @param   sptr_inode, user
 * @retval  errno
 * @note    none
 */
static kint32_t imx_fbdev_close(struct fwk_fb_info *sptr_info, kint32_t user)
{
    struct imx_fbdev_drv *sptr_drv;
    srt_imx_lcdif_t *sptr_lcdif;

    sptr_drv = fwk_fb_get_drvdata(sptr_info);
    sptr_lcdif = (srt_imx_lcdif_t *)sptr_drv->base;

    /*!< Close Interface */
    imx_fbdev_quit_interface(sptr_drv);

    /*!< Disable LCD */
    mr_writel(0U, &sptr_lcdif->CTRL_SET);
    print_info("fbdev is closed\r\n");

    return ER_NORMAL;
}

/*!
 * @brief   driver ioctl
 * @param   sptr_inode, user
 * @retval  errno
 * @note    none
 */
kint32_t imx_fbdev_ioctl(struct fwk_fb_info *sptr_info, kuint32_t cmd, kuaddr_t arg)
{
    struct fwk_fb_var_screen_info *sptr_var;
    struct imx_fbdev_drv *sptr_drv;
    srt_imx_lcdif_t *sptr_lcdif;
    kuaddr_t new_smem;

    sptr_drv = fwk_fb_get_drvdata(sptr_info);
    sptr_lcdif = (srt_imx_lcdif_t *)sptr_drv->base;

    switch (cmd)
    {
        case NR_FB_IOSET_VARINFO:
            new_smem = sptr_info->sgtc_fix.smem_start;
            sptr_var = (struct fwk_fb_var_screen_info *)arg;

            if ((sptr_var->xoffset >= sptr_info->sgtc_var.xres_virtual) ||
                (sptr_var->yoffset >= sptr_info->sgtc_var.yres_virtual))
                return -ER_MORE;

            if (sptr_var->yoffset >= sptr_info->sgtc_var.yres)
            {
                new_smem += sptr_info->sgtc_fix.smem_len;
                new_smem  = mr_align(new_smem, 8);
            }

            sptr_info->screen_base = (kuint8_t *)new_smem;

            /*!< Disable LCD */
            mr_writel(0U, &sptr_lcdif->CTRL_SET);

            /*!< CUR_BUF: Address of the current frame being transmitted by eLCDIF. */
            mr_writel(new_smem, &sptr_lcdif->CUR_BUF);
            /*!< NEXT_BUF: Address of the next frame that will be transmitted by eLCDIF */
            mr_writel(new_smem, &sptr_lcdif->NEXT_BUF);

            /*!< Enable LCD */
            mr_writel(mr_bit(17) | mr_bit(0), &sptr_lcdif->CTRL_SET);

            break;

        default:
            break;
    }

    return ER_NORMAL;
}

static const struct fwk_fb_oprts sgtc_fwk_fb_ops =
{
    .fb_open = imx_fbdev_open,
    .fb_release = imx_fbdev_close,
    .fb_ioctl = imx_fbdev_ioctl,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   probe backlight
 * @param   sptr_blight, sptr_node
 * @retval  errno
 * @note    none
 */
static kint32_t imx_fbdev_probe_backlight(struct imx_fbdev_backlight *sptr_blight, struct fwk_device_node *sptr_node)
{
    struct fwk_device *sptr_dev;
    struct fwk_pinctrl *sptr_pctl;
    struct fwk_gpio_desc *sptr_gdesc;
    struct fwk_pinctrl_state *sptr_state;
    kint32_t init_value = 0;

    sptr_dev = kzalloc(sizeof(*sptr_dev), GFP_KERNEL);
    if (!isValid(sptr_dev))
        return -ER_NOMEM;

    sptr_dev->init_name = sptr_node->full_name;
    sptr_dev->sptr_node = sptr_node;
    sptr_dev->sptr_parent = sptr_blight->sptr_par;
    mr_dev_set_name(sptr_dev, "backlight-gpios-%d", 0);

    if (fwk_device_add(sptr_dev))
        goto fail1;

    sptr_pctl = fwk_pinctrl_get(sptr_dev);
    if (!isValid(sptr_pctl))
        goto fail2;

    sptr_state = fwk_pinctrl_lookup_state(sptr_pctl, "default");
    if (isValid(sptr_state))
        fwk_pinctrl_select_state(sptr_pctl, sptr_state);

    sptr_gdesc = fwk_of_get_named_gpiodesc_flags(sptr_node, "backlight-gpios", 0, mr_nullptr);
    if (!isValid(sptr_gdesc))
        goto fail3;

    if (fwk_gpio_request(sptr_gdesc, "backlight-gpios"))
        goto fail3;
    
    fwk_gpio_set_direction_output(sptr_gdesc, init_value);

    sptr_blight->sptr_dev = sptr_dev;
    sptr_blight->sptr_pctl = sptr_pctl;
    sptr_blight->sptr_gdesc = sptr_gdesc;

    return ER_NORMAL;

fail3:
    fwk_pinctrl_put(sptr_pctl);
fail2:
    fwk_device_del(sptr_dev);
fail1:
    mr_dev_del_name(sptr_dev);
    kfree(sptr_dev);

    return -ER_ERROR;
}

/*!
 * @brief   remove backlight
 * @param   sptr_blight
 * @retval  none
 * @note    none
 */
static void imx_fbdev_remove_backlight(struct imx_fbdev_backlight *sptr_blight)
{
    if (sptr_blight->sptr_gdesc)
    {
        fwk_gpio_set_value(sptr_blight->sptr_gdesc, 0);
        fwk_gpio_free(sptr_blight->sptr_gdesc);
    }

    if (sptr_blight->sptr_pctl)
        fwk_pinctrl_put(sptr_blight->sptr_pctl);
    
    fwk_device_del(sptr_blight->sptr_dev);
    kfree(sptr_blight->sptr_dev);
    sptr_blight->sptr_dev = mr_nullptr;
    sptr_blight->sptr_gdesc = mr_nullptr;
    sptr_blight->sptr_pctl = mr_nullptr;
}

/*!
 * @brief   probe display-timings
 * @param   sptr_pdev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_fbdev_driver_probe_timings(struct fwk_platdev *sptr_pdev)
{
    struct imx_fbdev_drv *sptr_drv;
    struct fwk_device_node *sptr_node, *sptr_tim, *sptr_disp;
    struct fwk_device_node *sptr_blnode;
    struct fwk_fb_info *sptr_fb;
    struct fwk_fb_var_screen_info *sptr_var;
    kuint32_t phandle;
    kchar_t *blight_ways = mr_nullptr;
    kint32_t retval = 0;

    sptr_node = sptr_pdev->sgtc_dev.sptr_node;
    sptr_drv = (struct imx_fbdev_drv *)fwk_platform_get_drvdata(sptr_pdev);
    if (!isValid(sptr_node) || !isValid(sptr_drv))
        goto fail;

    sptr_fb = sptr_drv->sptr_fb;
    sptr_var = &sptr_fb->sgtc_var;
    
    retval = fwk_of_property_read_u32(sptr_node, "display", &phandle);
    sptr_disp = (retval < 0) ? mr_nullptr : fwk_of_find_node_by_phandle(sptr_node, phandle);
    if (!isValid(sptr_disp))
        goto fail;

    sptr_tim = fwk_of_find_node_by_name(sptr_disp, "display-timings");
    if (!isValid(sptr_tim))
        goto fail;

    retval = fwk_of_property_read_u32(sptr_tim, "native-mode", &phandle);
    sptr_tim = (retval < 0) ? mr_nullptr : fwk_of_find_node_by_phandle(sptr_tim, phandle);
    if (!isValid(sptr_tim))
        goto fail;

    /*!< get var info */
    retval = 0;
    retval |= fwk_of_property_read_u32(sptr_tim, "clock-frequency", &sptr_var->pixclock);
    retval |= fwk_of_property_read_u32(sptr_tim, "vactive", &sptr_var->yres);
    retval |= fwk_of_property_read_u32(sptr_tim, "hactive", &sptr_var->xres);
    retval |= fwk_of_property_read_u32(sptr_tim, "hback-porch", &sptr_var->left_margin);
    retval |= fwk_of_property_read_u32(sptr_tim, "hsync-len", &sptr_var->hsync_len);
    retval |= fwk_of_property_read_u32(sptr_tim, "hfront-porch", &sptr_var->right_margin);
    retval |= fwk_of_property_read_u32(sptr_tim, "vback-porch", &sptr_var->upper_margin);
    retval |= fwk_of_property_read_u32(sptr_tim, "vsync-len", &sptr_var->vsync_len);
    retval |= fwk_of_property_read_u32(sptr_tim, "vfront-porch", &sptr_var->lower_margin);

    retval |= fwk_of_property_read_u32(sptr_tim, "hsync-active", &sptr_drv->sgtc_phase.hsync_active);
    retval |= fwk_of_property_read_u32(sptr_tim, "vsync-active", &sptr_drv->sgtc_phase.vsync_active);
    retval |= fwk_of_property_read_u32(sptr_tim, "de-active", &sptr_drv->sgtc_phase.de_active);
    retval |= fwk_of_property_read_u32(sptr_tim, "pixelclk-active", &sptr_drv->sgtc_phase.pixelclk_active);

    retval |= fwk_of_property_read_u32(sptr_disp, "bits-per-pixel", &sptr_var->bits_per_pixel);
    retval |= fwk_of_property_read_u32(sptr_disp, "bus-width", &sptr_drv->sgtc_phase.bus_width);

    if (retval < 0)
        goto fail;

    /*!< Convert to ps */
    sptr_var->pixclock = FB_KHZ_2_PICOS(sptr_var->pixclock / 1000);

    if (sptr_var->bits_per_pixel == 24)
        sptr_var->bits_per_pixel = 32;

    if (fwk_of_find_property(sptr_disp, "hdmi-endpoint", mr_nullptr))
        sptr_drv->interface_type = NR_IMX_FBDEV_HDMI;
    else
    {
        /*!< find device-node of backlight */
        retval = fwk_of_property_read_u32(sptr_disp, "remote-endpoint", &phandle);
        sptr_blnode = (retval < 0) ? mr_nullptr : fwk_of_find_node_by_phandle(mr_nullptr, phandle);
        if (!isValid(sptr_blnode))
            goto fail;

        if (fwk_of_property_read_string(sptr_disp, "backlight-ways", &blight_ways))
            goto fail;

        if (blight_ways && (!kstrcmp(blight_ways, "gpio")))
        {
            sptr_drv->sgtc_blight.sptr_par = &sptr_pdev->sgtc_dev;
            imx_fbdev_probe_backlight(&sptr_drv->sgtc_blight, sptr_blnode);
        }
    }

    return ER_NORMAL;

fail:
    return -ER_FAILD;
}

/*!
 * @brief   imx_fbdev_driver_probe
 * @param   sptr_pdev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_fbdev_driver_probe(struct fwk_platdev *sptr_pdev)
{
    struct imx_fbdev_drv *sptr_drv;
    struct fwk_fb_info *sptr_fb;
    void *base, *buffer;
    kusize_t buffer_len, mem_size;
    kint32_t retval;

    sptr_fb = fwk_framebuffer_alloc(sizeof(*sptr_drv), &sptr_pdev->sgtc_dev);
    if (!isValid(sptr_fb))
        return -ER_NOMEM;

    base = (void *)fwk_platform_get_address(sptr_pdev, 0);
    base = fwk_io_remap(base, ARCH_PER_SIZE);
    if (!isValid(base))
        goto fail1;

    sptr_drv = (struct imx_fbdev_drv *)fwk_fb_get_drvdata(sptr_fb);
    sptr_drv->minor = FBDEV_IMX_DRIVER_MINOR;
    sptr_drv->base = base;
    sptr_drv->sptr_fb = sptr_fb;
    sptr_drv->sptr_dev = &sptr_pdev->sgtc_dev;
    sptr_drv->interface_type = NR_IMX_FBDEV_ELCDIF;
    init_waitqueue_head(&sptr_drv->sgtc_wqh);

    sptr_drv->sptr_clk[0] = fwk_clk_get(&sptr_pdev->sgtc_dev, "pix");
    if (!isValid(sptr_drv->sptr_clk[0]))
        goto fail2;

    sptr_drv->sptr_clk[1] = fwk_clk_get(&sptr_pdev->sgtc_dev, "axi");
    if (!isValid(sptr_drv->sptr_clk[1]))
        goto fail3;

//	sptr_drv->sptr_clk[2] = fwk_clk_get(&sptr_pdev->sgtc_dev, "disp_axi");
//	if (!isValid(sptr_drv->sptr_clk[2]))
//		goto fail4;

    fwk_clk_prepare_enable(sptr_drv->sptr_clk[0]);
    fwk_clk_prepare_enable(sptr_drv->sptr_clk[1]);

    fwk_platform_set_drvdata(sptr_pdev, sptr_drv);
    retval = imx_fbdev_driver_probe_timings(sptr_pdev);
    if (retval < 0)
        goto fail5;

    sptr_fb->sptr_fbops = &sgtc_fwk_fb_ops;
    sptr_fb->node = sptr_drv->minor;
    sptr_fb->sgtc_fix.smem_len = sptr_fb->sgtc_var.xres * sptr_fb->sgtc_var.yres * (sptr_fb->sgtc_var.bits_per_pixel >> 3);
    buffer_len = sptr_fb->sgtc_fix.smem_len;

    mem_size = kmget_size(GFP_DRAM);
    if (mem_size < buffer_len)
        goto fail6;

#if (defined(CONFIG_FBUFFER_NUM) && (CONFIG_FBUFFER_NUM >= 2))
    if (mem_size >= (buffer_len << 1))
        buffer_len <<= 1;
#endif

    buffer = kmalloc(buffer_len, GFP_DRAM);
    if (!isValid(buffer))
        goto fail6;

    sptr_fb->sgtc_fix.smem_start = (kuaddr_t)buffer;
    sptr_fb->sgtc_var.xoffset = sptr_fb->sgtc_var.yoffset = 0;
    sptr_fb->sgtc_var.xres_virtual = sptr_fb->sgtc_var.xres;
    sptr_fb->sgtc_var.yres_virtual = sptr_fb->sgtc_var.yres;

    sptr_fb->screen_base = (void *)sptr_fb->sgtc_fix.smem_start;
    sptr_fb->screen_size = sptr_fb->sgtc_fix.smem_len;

    /*!< choose to enable double buffer */
    if (buffer_len >= (2 * sptr_fb->sgtc_fix.smem_len))
    {
        sptr_fb->sgtc_var.yres_virtual <<= 1;
        sptr_fb->screen_size <<= 1;
    }
    
    retval = fwk_register_framebuffer(sptr_fb);
    if (retval < 0)
        goto fail7;

    imx_fbdev_init(base, sptr_drv);

    return ER_NORMAL;

fail7:
    kfree(buffer);
fail6:
    if (sptr_drv->interface_type == NR_IMX_FBDEV_ELCDIF)
        imx_fbdev_remove_backlight(&sptr_drv->sgtc_blight);
fail5:
    fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);

    fwk_clk_disable_unprepare(sptr_drv->sptr_clk[0]);
    fwk_clk_disable_unprepare(sptr_drv->sptr_clk[1]);

//	fwk_clk_put(sptr_drv->sptr_clk[2]);
// fail4:
    fwk_clk_put(sptr_drv->sptr_clk[1]);
fail3:
    fwk_clk_put(sptr_drv->sptr_clk[0]);
fail2:
    fwk_io_unmap(base);
fail1:
    kfree(sptr_fb);
    return -ER_FAILD;
}

/*!
 * @brief   imx_fbdev_driver_remove
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_fbdev_driver_remove(struct fwk_platdev *sptr_pdev)
{
    struct imx_fbdev_drv *sptr_drv;
    struct fwk_fb_info *sptr_fb;

    sptr_drv = (struct imx_fbdev_drv *)fwk_platform_get_drvdata(sptr_pdev);
    sptr_fb = sptr_drv->sptr_fb;

    fwk_unregister_framebuffer(sptr_fb);
    fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);

    fwk_clk_disable_unprepare(sptr_drv->sptr_clk[0]);
    fwk_clk_disable_unprepare(sptr_drv->sptr_clk[1]);
    fwk_clk_put(sptr_drv->sptr_clk[1]);
    fwk_clk_put(sptr_drv->sptr_clk[0]);

    if (sptr_drv->interface_type == NR_IMX_FBDEV_ELCDIF)
        imx_fbdev_remove_backlight(&sptr_drv->sgtc_blight);
    
    kfree((void *)sptr_fb->sgtc_fix.smem_start);
    fwk_io_unmap(sptr_drv->base);
    kfree(sptr_fb);
    
    return ER_NORMAL;
}

/*!< device id for device-tree */
static const struct fwk_of_device_id sgtc_imx_fbdev_driver_ids[] =
{
    { .compatible = "fsl,imx6ul-lcdif", },
    {},
};

/*!< platform instance */
static struct fwk_platdrv sgtc_imx_fbdev_platdriver =
{
    .probe	= imx_fbdev_driver_probe,
    .remove	= imx_fbdev_driver_remove,
    
    .sgtc_driver =
    {
        .name 	= "fbdev-imx",
        .id 	= -1,
        .sptr_of_match_table = sgtc_imx_fbdev_driver_ids,
    },
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   imx_fbdev_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init imx_fbdev_driver_init(void)
{
    return fwk_register_platdriver(&sgtc_imx_fbdev_platdriver);
}

/*!
 * @brief   imx_fbdev_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit imx_fbdev_driver_exit(void)
{
    fwk_unregister_platdriver(&sgtc_imx_fbdev_platdriver);
}

IMPORT_DRIVER_INIT(imx_fbdev_driver_init);
IMPORT_DRIVER_EXIT(imx_fbdev_driver_exit);

/*!< end of file */
