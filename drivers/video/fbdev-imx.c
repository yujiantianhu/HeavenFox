/*
 * Video Driver : LCD with framebuffer
 *
 * File Name:   fbdev-imx.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.03.25
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_inode.h>
#include <platform/fwk_fs.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/fwk_platdrv.h>
#include <platform/fwk_uaccess.h>
#include <platform/video/fwk_fbmem.h>

#include <asm/imx6/imx6ull_clocks.h>
#include <asm/imx6/imx6ull_pins.h>
#include <asm/imx6/imx6ull_periph.h>

/*!< The defines */
struct fbdev_imx_trigger
{
	kuint32_t hsync_active;
	kuint32_t vsync_active;
	kuint32_t de_active;
	kuint32_t pixelclk_active;
	kuint32_t bus_width;
};

struct fbdev_imx_drv
{
	kuint32_t minor;
	kuint32_t value[3];

	void *base;
	struct fwk_fb_info *sprt_fb;
	struct fwk_device *sprt_dev;

	srt_imx_pin_t sgrt_data[24];
	srt_imx_pin_t sgrt_ctrl[4];
	srt_imx_pin_t sgrt_reset;

	struct fbdev_imx_trigger sgrt_phase;
};

#define FBDEV_IMX_DRIVER_MINOR					0

/* The globals */
static kuint8_t g_fbdev_imx_buffer[480 * 272 * 4];

/*!< API function */
/*!
 * @brief   fbdev_imx_init
 * @param   base
 * @retval  errno
 * @note    none
 */
static void fbdev_imx_init(void *base, struct fbdev_imx_drv *sprt_drv)
{
	srt_imx_lcdif_t *sprt_lcdif = (srt_imx_lcdif_t *)base;
	struct fwk_fb_fix_screen_info *sprt_fix;
	struct fwk_fb_var_screen_info *sprt_var;
	kuaddr_t reg;

	if (!isValid(sprt_lcdif) || !isValid(sprt_drv))
		return;

	sprt_fix = &sprt_drv->sprt_fb->sgrt_fix;
	sprt_var = &sprt_drv->sprt_fb->sgrt_var;

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
	mrt_writel(mrt_bit(30), &sprt_lcdif->CTRL_CLR);
	while (mrt_isBitSetl(mrt_bit(30), &sprt_lcdif->CTRL));

	/*!< set bit31 */
	mrt_writel(mrt_bit(31), &sprt_lcdif->CTRL_SET);
	while (mrt_isBitResetl(mrt_bit(31), &sprt_lcdif->CTRL));

	delay_ms(100);

	/*!< clear bit30 & bit31 */
	mrt_writel(mrt_bit(31), &sprt_lcdif->CTRL_CLR);
	mrt_writel(mrt_bit(30), &sprt_lcdif->CTRL_CLR);

	mrt_writel(mrt_bit(19) | mrt_bit(17) | mrt_bit(5), &reg);

	switch (sprt_var->bits_per_pixel)
	{
		case 16:
//			mrt_setbitl(mrt_bit_nr(0x0, 8), &reg);
//			break;
		case 24:
		case 32:
			mrt_setbitl(mrt_bit_nr(0x03, 8), &reg);
			break;
		default:
			break;
	}

	switch (sprt_drv->sgrt_phase.bus_width)
	{
		case 16:
			mrt_setbitl(mrt_bit_nr(0x0, 10), &reg);
			break;
		case 24:
		case 32:
			mrt_setbitl(mrt_bit_nr(0x3, 10), &reg);
			break;
		default:
			break;
	}

	mrt_writel(reg, &sprt_lcdif->CTRL);

	/*!<
	 * CTRL1: eLCDIF General Control Register 1
	 * bit[19:16]: BYTE_PACKING_FORMAT. 
	 * 		set the bit field value to 0x7 if the display data is arranged in the 24-bit unpacked format 
	 * 		(A-R-G-B where A value does not have be transmitted).
	 */
	mrt_writel(mrt_bit_nr(0x7, 16), &sprt_lcdif->CTRL1);

	/*!<
	 * TRANSFER_COUNT: eLCDIF Horizontal and Vertical Valid Data Count Register
	 * bit[31:16]: Number of horizontal lines per frame which contain valid data.
	 * bit[15:0]: Total valid data (pixels) in each horizontal line
	 */
	mrt_writel(mrt_mask(sprt_var->yres << 16, 0xffff0000) | mrt_mask(sprt_var->xres, 0xffff), &sprt_lcdif->TRANSFER_COUNT);

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
	mrt_resetl(&reg);
	mrt_setbitl(mrt_bit(28) | mrt_bit(21) | mrt_bit(20), &reg);
	mrt_setbitl(mrt_mask(sprt_var->vsync_len, 0x3ffff), &reg);
	mrt_setbitl(mrt_bit_nr(!!sprt_drv->sgrt_phase.vsync_active, 27), &reg);
	mrt_setbitl(mrt_bit_nr(!!sprt_drv->sgrt_phase.hsync_active, 26), &reg);
	mrt_setbitl(mrt_bit_nr(!sprt_drv->sgrt_phase.pixelclk_active, 25), &reg);
	mrt_setbitl(mrt_bit_nr(!!sprt_drv->sgrt_phase.de_active, 24), &reg);
	mrt_writel(reg, &sprt_lcdif->VDCTRL0);

	/*!<
	 * VDCTRL1: eLCDIF VSYNC Mode and Dotclk Mode Control Register 1
	 * bit[31:0]: D Total number of units between two positive or two negative edges of the VSYNC signal
	 */
	mrt_writel(sprt_var->yres + sprt_var->vsync_len + sprt_var->lower_margin + sprt_var->upper_margin, &sprt_lcdif->VDCTRL1);

	/*!<
	 * VDCTRL2: eLCDIF VSYNC Mode and Dotclk Mode Control Register 2
	 * bit[31:18]: Number of DISPLAY CLOCK (pix_clk) cycles for which HSYNC signal is active
	 * bit[17:0]: Total number of DISPLAY CLOCK (pix_clk) cycles between two positive or two negative edges of the HSYNC signal.
	 */
	mrt_writel(mrt_mask(sprt_var->hsync_len << 18, 0x3ffff), &sprt_lcdif->VDCTRL2);
	mrt_setbitl(sprt_var->xres + sprt_var->hsync_len + sprt_var->right_margin + sprt_var->left_margin, &sprt_lcdif->VDCTRL2);
	
	/*!<
	 * VDCTRL3: eLCDIF VSYNC Mode and Dotclk Mode Control Register 3
	 * bit[27:16]: HORIZONTAL_WAIT_CNT
	 * bit[15:0]: VERTICAL_WAIT_CNT
	 */
	mrt_writel(mrt_mask((sprt_var->left_margin + sprt_var->hsync_len) << 16, 0xfff0000), &sprt_lcdif->VDCTRL3);
	mrt_setbitl(sprt_var->upper_margin + sprt_var->vsync_len, &sprt_lcdif->VDCTRL3);

	/*!<
	 * VDCTRL3: eLCDIF VSYNC Mode and Dotclk Mode Control Register 4
	 * bit18: 	Set this field to 1 if the LCD controller requires that the VSYNC or VSYNC/HSYNC/DOTCLK control
	 *			signals should be active at least one frame before the data transfers actually start and remain active at
	 *			least one frame after the data transfers end
	 * bit[17:0]: Total number of DISPLAY CLOCK (pix_clk) cycles on each horizontal line that carry valid data in DOTCLK mode.
	 */
	mrt_writel(mrt_bit(18) | sprt_var->xres, &sprt_lcdif->VDCTRL4);

	/*!< CUR_BUF: Address of the current frame being transmitted by eLCDIF. */
	mrt_writel(sprt_fix->smem_start, &sprt_lcdif->CUR_BUF);
	/*!< NEXT_BUF: Address of the next frame that will be transmitted by eLCDIF */
	mrt_writel(sprt_fix->smem_start, &sprt_lcdif->NEXT_BUF);

	/*!< Enable LCD */
	mrt_writel(mrt_bit(17) | mrt_bit(0), &sprt_lcdif->CTRL_SET);
}

/*!
 * @brief   fbdev_imx_driver_open
 * @param   sprt_inode, sprt_file
 * @retval  errno
 * @note    none
 */
static ksint32_t fbdev_imx_open(struct fwk_fb_info *sprt_info, ksint32_t user)
{
	return NR_isWell;
}

static ksint32_t fbdev_imx_close(struct fwk_fb_info *sprt_info, ksint32_t user)
{
	return NR_isWell;
}

static const struct fwk_fb_oprts sgrt_fwk_fb_ops =
{
	.fb_open = fbdev_imx_open,
	.fb_release = fbdev_imx_close,
};

/*!< --------------------------------------------------------------------- */
static void fbdev_imx_configure_backlight(void *base)
{
	srt_imx_pin_t sgrt_conf;
	srt_imx_gpio_t *sprt_gpio;

	/*!< set backlight */
	hal_imx_pin_attribute_init(&sgrt_conf, (kuaddr_t)base, IMX6UL_MUX_GPIO1_IO08_GPIO1_IO08, 0xb9);
	hal_imx_pin_configure(&sgrt_conf);

	/*!< output, high level */
	sprt_gpio = (srt_imx_gpio_t *)IMX6UL_GPIO1_ADDR_BASE;
	mrt_setbitl(mrt_bit(8), &sprt_gpio->GDIR);
	mrt_setbitl(mrt_bit(8), &sprt_gpio->DR);
}

static ksint32_t fbdev_imx_pinctrl_read_and_set(struct fwk_device_node *sprt_node, kuint32_t count, void *base)
{
	srt_imx_pin_t sgrt_conf;
	kuint32_t index, value[IMX6UL_MUX_CONF_SIZE];
	ksint32_t retval = NR_isWell;

	for (index = 0; index < count; index++)
	{
		retval = fwk_of_property_read_u32_array_index(sprt_node, "fsl,pins", value, index * ARRAY_SIZE(value), ARRAY_SIZE(value));
		if (retval < 0)
			goto END;

		hal_imx_pin_auto_init(&sgrt_conf, (kuaddr_t)base, value, ARRAY_SIZE(value));
		hal_imx_pin_configure(&sgrt_conf);
	}

END:
	return retval;
}

static ksint32_t fbdev_imx_driver_probe_mux(struct fwk_platdev *sprt_dev)
{
	struct fbdev_imx_drv *sprt_drv;
	struct fwk_device_node *sprt_node, *sprt_pindat, *sprt_pinctl, *sprt_reset;
	kuint32_t phandle[3];
	ksint32_t retval = 0;

	sprt_node = sprt_dev->sgrt_device.sprt_node;
	sprt_drv = (struct fbdev_imx_drv *)fwk_platform_get_drvdata(sprt_dev);
	if (!isValid(sprt_node) || !isValid(sprt_drv))
		goto fail;

	retval = fwk_of_property_read_u32_array(sprt_node, "pinctrl-0", phandle, ARRAY_SIZE(phandle));
	if (retval < 0)
		goto fail;

	sprt_pindat = fwk_of_find_node_by_phandle(mrt_nullptr, phandle[0]);
	sprt_pinctl = fwk_of_find_node_by_phandle(mrt_nullptr, phandle[1]);
	sprt_reset  = fwk_of_find_node_by_phandle(mrt_nullptr, phandle[2]);
	if (!sprt_pindat || !sprt_pinctl || !sprt_reset)
		goto fail;

	/* initial pin mux */
	fbdev_imx_pinctrl_read_and_set(sprt_pindat, 24, 0);
	fbdev_imx_pinctrl_read_and_set(sprt_pinctl, 4, 0);
	fbdev_imx_pinctrl_read_and_set(sprt_reset, 1, 0);

	fbdev_imx_configure_backlight(0);

	return NR_isWell;

fail:
	return -NR_isNotSuccess;
}

static ksint32_t fbdev_imx_driver_probe_timings(struct fwk_platdev *sprt_dev)
{
	struct fbdev_imx_drv *sprt_drv;
	struct fwk_device_node *sprt_node, *sprt_tim, *sprt_disp;
	struct fwk_fb_info *sprt_fb;
	struct fwk_fb_var_screen_info *sprt_var;
	kuint32_t phandle;
	ksint32_t retval = 0;

	sprt_node = sprt_dev->sgrt_device.sprt_node;
	sprt_drv = (struct fbdev_imx_drv *)fwk_platform_get_drvdata(sprt_dev);
	if (!isValid(sprt_node) || !isValid(sprt_drv))
		goto fail;

	sprt_fb = sprt_drv->sprt_fb;
	sprt_var = &sprt_fb->sgrt_var;
	
	retval = fwk_of_property_read_u32(sprt_node, "display", &phandle);
	sprt_disp = (retval < 0) ? mrt_nullptr : fwk_of_find_node_by_phandle(sprt_node, phandle);
	if (!isValid(sprt_disp))
		goto fail;

	sprt_tim = fwk_of_find_node_by_name(sprt_disp, "display-timings");
	if (!isValid(sprt_tim))
		goto fail;

	retval = fwk_of_property_read_u32(sprt_tim, "native-mode", &phandle);
	sprt_tim = (retval < 0) ? mrt_nullptr : fwk_of_find_node_by_phandle(sprt_tim, phandle);
	if (!isValid(sprt_tim))
		goto fail;

	/* get var info */
	retval = 0;
	retval |= fwk_of_property_read_u32(sprt_tim, "clock-frequency", &sprt_var->pixclock);
	retval |= fwk_of_property_read_u32(sprt_tim, "vactive", &sprt_var->yres);
	retval |= fwk_of_property_read_u32(sprt_tim, "hactive", &sprt_var->xres);
	retval |= fwk_of_property_read_u32(sprt_tim, "hback-porch", &sprt_var->left_margin);
	retval |= fwk_of_property_read_u32(sprt_tim, "hsync-len", &sprt_var->hsync_len);
	retval |= fwk_of_property_read_u32(sprt_tim, "hfront-porch", &sprt_var->right_margin);
	retval |= fwk_of_property_read_u32(sprt_tim, "vback-porch", &sprt_var->upper_margin);
	retval |= fwk_of_property_read_u32(sprt_tim, "vsync-len", &sprt_var->vsync_len);
	retval |= fwk_of_property_read_u32(sprt_tim, "vfront-porch", &sprt_var->lower_margin);

	retval |= fwk_of_property_read_u32(sprt_tim, "hsync-active", &sprt_drv->sgrt_phase.hsync_active);
	retval |= fwk_of_property_read_u32(sprt_tim, "vsync-active", &sprt_drv->sgrt_phase.vsync_active);
	retval |= fwk_of_property_read_u32(sprt_tim, "de-active", &sprt_drv->sgrt_phase.de_active);
	retval |= fwk_of_property_read_u32(sprt_tim, "pixelclk-active", &sprt_drv->sgrt_phase.pixelclk_active);

	retval |= fwk_of_property_read_u32(sprt_disp, "bits-per-pixel", &sprt_var->bits_per_pixel);
	retval |= fwk_of_property_read_u32(sprt_disp, "bus-width", &sprt_drv->sgrt_phase.bus_width);

	if (retval < 0)
		goto fail;

	return NR_isWell;

fail:
	return -NR_isNotSuccess;

}

/*!
 * @brief   fbdev_imx_driver_probe
 * @param   sprt_pdev
 * @retval  errno
 * @note    none
 */
static ksint32_t fbdev_imx_driver_probe(struct fwk_platdev *sprt_pdev)
{
	struct fbdev_imx_drv *sprt_drv;
	struct fwk_fb_info *sprt_fb;
	void *base;
	ksint32_t retval;

	sprt_fb = fwk_framebuffer_alloc(sizeof(*sprt_drv), &sprt_pdev->sgrt_device);
	if (!isValid(sprt_fb))
		return -NR_isMemErr;

	base = (void *)fwk_platform_get_address(sprt_pdev, 0);
	base = fwk_io_remap(base);
	if (!isValid(base))
		goto fail;

	sprt_drv = (struct fbdev_imx_drv *)fwk_fb_get_drvdata(sprt_fb);
	sprt_drv->minor = FBDEV_IMX_DRIVER_MINOR;
	sprt_drv->base = base;
	sprt_drv->sprt_fb = sprt_fb;
	sprt_drv->sprt_dev = &sprt_pdev->sgrt_device;

	fwk_platform_set_drvdata(sprt_pdev, sprt_drv);
	retval = fbdev_imx_driver_probe_timings(sprt_pdev);
	if (retval < 0)
		goto fail1;

	retval = fbdev_imx_driver_probe_mux(sprt_pdev);
	if (retval < 0)
		goto fail1;

	sprt_fb->sprt_fbops = &sgrt_fwk_fb_ops;
	sprt_fb->node = sprt_drv->minor;
	sprt_fb->sgrt_fix.smem_start = (kuaddr_t)(&g_fbdev_imx_buffer[0]);
	sprt_fb->sgrt_fix.smem_len = sizeof(g_fbdev_imx_buffer);

	retval = fwk_register_framebuffer(sprt_fb);
	if (retval < 0)
		goto fail;

	fbdev_imx_init(base, sprt_drv);

	return NR_isWell;

fail1:
	fwk_platform_set_drvdata(sprt_pdev, mrt_nullptr);
	fwk_io_unmap(base);
fail:
	kfree(sprt_fb);
	return -NR_isNotSuccess;
}

/*!
 * @brief   fbdev_imx_driver_remove
 * @param   sprt_dev
 * @retval  errno
 * @note    none
 */
static ksint32_t fbdev_imx_driver_remove(struct fwk_platdev *sprt_dev)
{
	struct fbdev_imx_drv *sprt_drv;
	struct fwk_fb_info *sprt_fb;

	sprt_drv = (struct fbdev_imx_drv *)fwk_platform_get_drvdata(sprt_dev);
	sprt_fb = sprt_drv->sprt_fb;

	fwk_unregister_framebuffer(sprt_fb);
	fwk_platform_set_drvdata(sprt_dev, mrt_nullptr);
	fwk_io_unmap(sprt_drv->base);
	kfree(sprt_fb);
	
	return NR_isWell;
}

/*!< device id for device-tree */
static struct fwk_of_device_id sgrt_fbdev_imx_driver_ids[] =
{
	{ .compatible = "fsl,imx6ul-lcdif", },
	{},
};

/*!< platform instance */
static struct fwk_platdrv sgrt_fbdev_imx_platdriver =
{
	.probe	= fbdev_imx_driver_probe,
	.remove	= fbdev_imx_driver_remove,
	
	.sgrt_driver =
	{
		.name 	= "fbdev-imx",
		.id 	= -1,
		.sprt_of_match_table = sgrt_fbdev_imx_driver_ids,
	},
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   fbdev_imx_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
ksint32_t __fwk_init fbdev_imx_driver_init(void)
{
	return fwk_register_platdriver(&sgrt_fbdev_imx_platdriver);
}

/*!
 * @brief   fbdev_imx_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit fbdev_imx_driver_exit(void)
{
	fwk_unregister_platdriver(&sgrt_fbdev_imx_platdriver);
}

IMPORT_DRIVER_INIT(fbdev_imx_driver_init);
IMPORT_DRIVER_EXIT(fbdev_imx_driver_exit);

/*!< end of file */
