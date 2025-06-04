/*
 * Video Driver : LCD with framebuffer (AXIVDMA + VTC)
 *
 * File Name:   xsdk_hdmi.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.11.01
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_inode.h>
#include <platform/fwk_fs.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/fwk_platdrv.h>
#include <platform/clk/fwk_clk.h>
#include <platform/fwk_pinctrl.h>
#include <platform/gpio/fwk_gpiodesc.h>
#include <platform/fwk_uaccess.h>
#include <platform/video/fwk_fbmem.h>
#include <platform/video/fwk_rgbmap.h>

#include <zynq7/zynq7_periph.h>

/*!< The defines */
#define XSDK_HDMI_XPRES_MAX                 (1920)
#define XSDK_HDMI_YPRES_MAX                 (1080)
#define XSDK_HDMI_BPP_MAX                   (4)

#define XSDK_HDMI_DRAM_SIZE    \
                    (XSDK_HDMI_XPRES_MAX * XSDK_HDMI_YPRES_MAX * XSDK_HDMI_BPP_MAX)

struct xsdk_hdmi_trigger
{
	kuint32_t hsync_active;
	kuint32_t vsync_active;
	kuint32_t de_active;
	kuint32_t pixelclk_active;
};

struct xsdk_hdmi_drv
{
    kuint32_t minor;

    void *base;
    struct fwk_fb_info *sptr_fb;
    struct fwk_device *sptr_dev;
    struct xsdk_hdmi_trigger sgtc_trig;

    DisplayCtrl sgtc_dctrl;
    XAxiVdma sgtc_axivdma;
    XAxiVdma_Config sgtc_axicfg;
    XVtc_Config sgtc_vcfg;
};

#define XSDK_HDMI_DRIVER_MINOR				0

/* The globals */

/*!< API function */
/*!
 * @brief   xsdk_hdmi_init
 * @param   base
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_hdmi_init(void *base, struct xsdk_hdmi_drv *sptr_drv)
{
    struct fwk_fb_info *sptr_fb;
    struct fwk_fb_var_screen_info *sptr_var;
	XAxiVdma_Config *sptr_axicfg;
    XVtc_Config *sptr_vcfg;
    VideoMode sgtc_vmode;
    kuint8_t *pFrames[DISPLAY_NUM_FRAMES];
    kint32_t retval;

    if (DISPLAY_NUM_FRAMES > 1)
        return -ER_INVALID;

    sptr_fb   = sptr_drv->sptr_fb;
    sptr_var  = &sptr_fb->sgtc_var;
    sptr_vcfg = &sptr_drv->sgtc_vcfg;
    sptr_axicfg = &sptr_drv->sgtc_axicfg;

    retval = XAxiVdma_CfgInitialize(&sptr_drv->sgtc_axivdma, sptr_axicfg, sptr_axicfg->BaseAddress);
    if (retval)
        return retval;

    sprintk(sgtc_vmode.label, "hdmi %dx%d@60Hz", sptr_var->xres, sptr_var->yres);
    sgtc_vmode.freq = ((kfloat_t)(sptr_var->pixclock / 1000)) / 1000.0;
    
    sgtc_vmode.width = sptr_var->xres;
    sgtc_vmode.height = sptr_var->yres;

    sgtc_vmode.hps = sptr_var->xres + sptr_var->right_margin;
    sgtc_vmode.hpe = sgtc_vmode.hps + sptr_var->hsync_len;
    sgtc_vmode.hmax = sgtc_vmode.hpe + sptr_var->left_margin - 1;
    sgtc_vmode.hpol = !sptr_drv->sgtc_trig.hsync_active;

    sgtc_vmode.vps = sptr_var->yres + sptr_var->lower_margin;
    sgtc_vmode.vpe = sgtc_vmode.vps + sptr_var->vsync_len;
    sgtc_vmode.vmax = sgtc_vmode.vpe + sptr_var->upper_margin - 1;
    sgtc_vmode.vpol = !sptr_drv->sgtc_trig.vsync_active;

    pFrames[0] = sptr_fb->screen_base;
    retval = DisplayInitialize( &sptr_drv->sgtc_dctrl, 
                                &sptr_drv->sgtc_axivdma, 
                                XPAR_VTC_0_DEVICE_ID, 
                                XPAR_AXI_DYNCLK_0_BASEADDR, 
                                pFrames, 
                                sptr_var->xres * (sptr_var->bits_per_pixel >> 3), 
                                &sgtc_vmode);
    if (retval)
        return retval;

    print_info("axivdma and vtc initialize successfully\r\n");
    print_info("screen's xres is %d, yres is: %d, pixel-width is: %d\r\n", 
                sptr_var->xres, sptr_var->yres, sptr_var->bits_per_pixel);

    retval = XVtc_CfgInitialize(&(sptr_drv->sgtc_dctrl.vtc), sptr_vcfg, sptr_vcfg->BaseAddress);
    if (retval)
        return retval;

    return ER_NORMAL;
}

/*!
 * @brief   driver open
 * @param   sptr_inode, user
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_hdmi_open(struct fwk_fb_info *sptr_info, kint32_t user)
{
    struct xsdk_hdmi_drv *sptr_drv;
    kint32_t retval;

    sptr_drv = fwk_fb_get_drvdata(sptr_info);
    retval = DisplayStart(&sptr_drv->sgtc_dctrl);
    if (retval)
        return -ER_FAILD;

    print_info("hdmi device is opened\r\n");

    memset_ex(sptr_info->screen_base, 0x00000000, sptr_info->screen_size);
    print_info("clear full screen with black color\r\n");

    return ER_NORMAL;
}

/*!
 * @brief   driver close
 * @param   sptr_inode, user
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_hdmi_close(struct fwk_fb_info *sptr_info, kint32_t user)
{
    struct xsdk_hdmi_drv *sptr_drv;
    kint32_t retval;

    sptr_drv = fwk_fb_get_drvdata(sptr_info);

    memset_ex(sptr_info->screen_base, 0x00000000, sptr_info->screen_size);
    retval = DisplayStop(&sptr_drv->sgtc_dctrl);
    if (!retval)
        print_info("hdmi device is closed\r\n");

    return retval;
}

/*!
 * @brief   driver ioctl
 * @param   sptr_inode, user
 * @retval  errno
 * @note    none
 */
kint32_t xsdk_hdmi_ioctl(struct fwk_fb_info *sptr_info, kuint32_t cmd, kuaddr_t arg)
{
    struct xsdk_hdmi_drv *sptr_drv;
    struct fwk_fb_var_screen_info *sptr_var;
    kuaddr_t new_smem;

    sptr_drv = fwk_fb_get_drvdata(sptr_info);
    if (!arg)
        return -ER_NULLPTR;

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
            if (DisplayChangeFrameBuffer(&sptr_drv->sgtc_dctrl, new_smem, sptr_info->sgtc_fix.smem_len))
                return -ER_FAILD;

            break;

        default:
            break;
    }

    return ER_NORMAL;
}

static const struct fwk_fb_oprts sgtc_fwk_fb_ops =
{
    .fb_open = xsdk_hdmi_open,
    .fb_release = xsdk_hdmi_close,
    .fb_ioctl = xsdk_hdmi_ioctl,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   probe axivdma
 * @param   sptr_pdev
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_hdmi_driver_probe_axivdma(struct fwk_platdev *sptr_pdev)
{
    struct xsdk_hdmi_drv *sptr_drv;
    struct fwk_device_node *sptr_node, *sptr_axivdma = mr_nullptr;
    XAxiVdma_Config *sptr_axicfg;
    void *reg;
    kuint32_t temp;
    kint32_t retval = 0;

    struct fwk_of_device_id sgtc_axivdma_ids[] =
    {
        { .compatible = "xlnx, pl, axivdma", },
        {},
    };

    sptr_node = sptr_pdev->sgtc_dev.sptr_node;
    sptr_drv = (struct xsdk_hdmi_drv *)fwk_platform_get_drvdata(sptr_pdev);
    if (!isValid(sptr_node) || !isValid(sptr_drv))
        return -ER_NODEV;

    sptr_axivdma = fwk_of_find_matching_child_and_match(sptr_node, sgtc_axivdma_ids, mr_nullptr);
    if (!isValid(sptr_axivdma))
        return -ER_NOTFOUND;

    sptr_axicfg = &sptr_drv->sgtc_axicfg;

    retval |= fwk_of_property_read_u32(sptr_axivdma, "max-frame-store-num", &temp);
    if (!retval)
        sptr_axicfg->MaxFrameStoreNum = (kuint16_t)temp;

    retval |= fwk_of_property_read_u32(sptr_axivdma, "has-Vertical-filp", &temp);
    if (!retval)
        sptr_axicfg->HasVFlip = (kuint8_t)temp;
    
    retval |= fwk_of_property_read_u32(sptr_axivdma, "has-mm2s", (kuint32_t *)(&sptr_axicfg->HasMm2S));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "has-mm2s-dre", (kuint32_t *)(&sptr_axicfg->HasMm2SDRE));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "mm2s-word-len", (kuint32_t *)(&sptr_axicfg->Mm2SWordLen));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "has-s2mm", (kuint32_t *)(&sptr_axicfg->HasS2Mm));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "has-s2mm-dre", (kuint32_t *)(&sptr_axicfg->HasS2MmDRE));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "s2mm-word-len", (kuint32_t *)(&sptr_axicfg->S2MmWordLen));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "has-sg", (kuint32_t *)(&sptr_axicfg->HasSG));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "enable-vid-param-read", (kuint32_t *)(&sptr_axicfg->EnableVIDParamRead));
    
    retval |= fwk_of_property_read_u32(sptr_axivdma, "use-fsync", (kuint32_t *)(&sptr_axicfg->UseFsync));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "flush-on-fsync", (kuint32_t *)(&sptr_axicfg->FlushonFsync));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "mm2s-buf-depth", (kuint32_t *)(&sptr_axicfg->Mm2SBufDepth));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "s2mm-buf-depth", (kuint32_t *)(&sptr_axicfg->S2MmBufDepth));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "mm2s-gen-lock", (kuint32_t *)(&sptr_axicfg->Mm2SGenLock));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "s2mm-gen-lock", (kuint32_t *)(&sptr_axicfg->S2MmGenLock));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "internal-gen-lock", (kuint32_t *)(&sptr_axicfg->InternalGenLock));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "s2mm-sof", (kuint32_t *)(&sptr_axicfg->S2MmSOF));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "mm2s-stream-width", (kuint32_t *)(&sptr_axicfg->Mm2SStreamWidth));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "s2mm-stream-width", (kuint32_t *)(&sptr_axicfg->S2MmStreamWidth));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "mm2s-thres-reg-en", (kuint32_t *)(&sptr_axicfg->Mm2SThresRegEn));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "mm2s-frm-store-reg-en", (kuint32_t *)(&sptr_axicfg->Mm2SFrmStoreRegEn));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "mm2s-dly-cntr-en", (kuint32_t *)(&sptr_axicfg->Mm2SDlyCntrEn));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "mm2s-frm-cntr-en", (kuint32_t *)(&sptr_axicfg->Mm2SFrmCntrEn));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "s2mm-thres-reg-en", (kuint32_t *)(&sptr_axicfg->S2MmThresRegEn));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "s2mm-frm-store-reg-en", (kuint32_t *)(&sptr_axicfg->S2MmFrmStoreRegEn));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "s2mm-sly-cntr-en", (kuint32_t *)(&sptr_axicfg->S2MmDlyCntrEn));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "s2mm-frm-cntr-en", (kuint32_t *)(&sptr_axicfg->S2MmFrmCntrEn));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "enable-all-dbg-features", (kuint32_t *)(&sptr_axicfg->EnableAllDbgFeatures));
    retval |= fwk_of_property_read_u32(sptr_axivdma, "addr-width", (kuint32_t *)(&sptr_axicfg->AddrWidth));
    
    if (retval)
        return -ER_NOTFOUND;

    reg = fwk_of_iomap(sptr_axivdma, 0);
    if (!isValid(reg))
        return PTR_ERR(reg);
    
    sptr_axicfg->BaseAddress = (kuint32_t)reg;
    sptr_axicfg->DeviceId = XPAR_AXI_VDMA_0_DEVICE_ID;

    return ER_NORMAL;
}

/*!
 * @brief   remove axivdma
 * @param   sptr_pdev
 * @retval  errno
 * @note    none
 */
static void xsdk_hdmi_driver_remove_axivdma(struct fwk_platdev *sptr_pdev)
{
    struct xsdk_hdmi_drv *sptr_drv;
    XAxiVdma_Config *sptr_axicfg;

    sptr_drv = (struct xsdk_hdmi_drv *)fwk_platform_get_drvdata(sptr_pdev);
    if (!isValid(sptr_drv))
        return;

    sptr_axicfg = &sptr_drv->sgtc_axicfg;
    fwk_io_unmap((void *)sptr_axicfg->BaseAddress);
}

/*!
 * @brief   probe display-timings
 * @param   sptr_pdev
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_hdmi_driver_probe_timings(struct fwk_platdev *sptr_pdev)
{
    struct xsdk_hdmi_drv *sptr_drv;
    struct fwk_device_node *sptr_node, *sptr_tim;
    struct fwk_fb_info *sptr_fb;
    struct fwk_fb_var_screen_info *sptr_var;
    kuint32_t phandle;
    kchar_t *format = mr_nullptr;
    kint32_t retval = 0;

    sptr_node = sptr_pdev->sgtc_dev.sptr_node;
    sptr_drv = (struct xsdk_hdmi_drv *)fwk_platform_get_drvdata(sptr_pdev);
    if (!isValid(sptr_node) || !isValid(sptr_drv))
        return -ER_NODEV;

    sptr_fb = sptr_drv->sptr_fb;
    sptr_var = &sptr_fb->sgtc_var;
    
    retval = fwk_of_property_read_u32(sptr_node, "display-timings", &phandle);
    sptr_tim = retval ? mr_nullptr : fwk_of_find_node_by_phandle(sptr_node, phandle);
    if (!isValid(sptr_tim))
        return -ER_NOTFOUND;

    /*!< get var info */
    retval  = 0;
    retval |= fwk_of_property_read_u32(sptr_tim, "clock-freq", &sptr_var->pixclock);
    retval |= fwk_of_property_read_u32(sptr_tim, "height", &sptr_var->xres);
    retval |= fwk_of_property_read_u32(sptr_tim, "width", &sptr_var->yres);
    retval |= fwk_of_property_read_u32(sptr_tim, "h-back-porch", &sptr_var->left_margin);
    retval |= fwk_of_property_read_u32(sptr_tim, "h-sync-width", &sptr_var->hsync_len);
    retval |= fwk_of_property_read_u32(sptr_tim, "h-front-porch", &sptr_var->right_margin);
    retval |= fwk_of_property_read_u32(sptr_tim, "v-back-porch", &sptr_var->upper_margin);
    retval |= fwk_of_property_read_u32(sptr_tim, "v-sync-width", &sptr_var->vsync_len);
    retval |= fwk_of_property_read_u32(sptr_tim, "v-front-porch", &sptr_var->lower_margin);

    retval |= fwk_of_property_read_u32(sptr_tim, "h-sync-active", &sptr_drv->sgtc_trig.hsync_active);
    retval |= fwk_of_property_read_u32(sptr_tim, "v-sync-active", &sptr_drv->sgtc_trig.vsync_active);
    retval |= fwk_of_property_read_u32(sptr_tim, "de-active", &sptr_drv->sgtc_trig.de_active);
    retval |= fwk_of_property_read_u32(sptr_tim, "pixelclk-active", &sptr_drv->sgtc_trig.pixelclk_active);

    if (retval)
        return -ER_NOTFOUND;

    retval = fwk_of_property_read_string(sptr_node, "xlnx,pixel-format", &format);
    if (retval || (!format))
        return -ER_NOTFOUND;

    format = kstrcut(format, 3);
    if (!format)
        sptr_var->bits_per_pixel = 32;
    else
    {
        if (!kstrcmp(format, "8"))
            sptr_var->bits_per_pixel = 8;
        else if (!kstrcmp(format, "16"))
            sptr_var->bits_per_pixel = 16;
        else if (!kstrcmp(format, "24"))
            sptr_var->bits_per_pixel = 32;
        else
            sptr_var->bits_per_pixel = 32;
    }

    return ER_NORMAL;
}

/*!
 * @brief   xsdk_hdmi_driver_probe
 * @param   sptr_pdev
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_hdmi_driver_probe(struct fwk_platdev *sptr_pdev)
{
    struct xsdk_hdmi_drv *sptr_drv;
    struct fwk_device_node *sptr_node;
    struct fwk_fb_info *sptr_fb;
    void *base, *buffer;
    kuint32_t isHdmi = 0;
    kusize_t buffer_len, mem_size;
    kint32_t retval;

    sptr_node = sptr_pdev->sgtc_dev.sptr_node;
    if (!isValid(sptr_node))
        return -ER_NODEV;

    retval = fwk_of_property_read_u32(sptr_node, "is-hdmi-device", &isHdmi);
    if (retval)
        isHdmi = 1;

    if (!isHdmi) 
    {
        print_warn("it is not a hdmi device!");
        return -ER_CHECKERR;
    }

    sptr_fb = fwk_framebuffer_alloc(sizeof(*sptr_drv), &sptr_pdev->sgtc_dev);
    if (!isValid(sptr_fb))
        return -ER_NOMEM;

    base = (void *)fwk_platform_get_address(sptr_pdev, 0);
    base = fwk_io_remap(base, ARCH_PER_SIZE);
    if (!isValid(base))
        goto fail1;

    sptr_drv = (struct xsdk_hdmi_drv *)fwk_fb_get_drvdata(sptr_fb);
    sptr_drv->minor = XSDK_HDMI_DRIVER_MINOR;
    sptr_drv->sgtc_vcfg.BaseAddress = (kuint32_t)base;
    sptr_drv->sgtc_vcfg.DeviceId = XPAR_V_TC_0_DEVICE_ID;
    sptr_drv->sptr_fb = sptr_fb;
    sptr_drv->sptr_dev = &sptr_pdev->sgtc_dev;

    fwk_platform_set_drvdata(sptr_pdev, sptr_drv);
    retval = xsdk_hdmi_driver_probe_axivdma(sptr_pdev);
    if (retval)
        goto fail2;
    
    retval = xsdk_hdmi_driver_probe_timings(sptr_pdev);
    if (retval)
        goto fail3;

    sptr_fb->sptr_fbops = &sgtc_fwk_fb_ops;
    sptr_fb->node = sptr_drv->minor;
    sptr_fb->sgtc_fix.smem_len = sptr_fb->sgtc_var.xres * sptr_fb->sgtc_var.yres * (sptr_fb->sgtc_var.bits_per_pixel >> 3);
    buffer_len = sptr_fb->sgtc_fix.smem_len;

    mem_size = kmget_size(GFP_DRAM);
    if (mem_size < buffer_len)
        goto fail3;

#if (defined(CONFIG_FBUFFER_NUM) && (CONFIG_FBUFFER_NUM >= 2))
    if (mem_size >= (buffer_len << 1))
        buffer_len <<= 1;
#endif

    buffer = kmalloc(buffer_len, GFP_DRAM);
    if (!isValid(buffer))
        goto fail3;

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
        goto fail4;

    print_info("register a new framebuffer (hdmi)\r\n");

    retval = xsdk_hdmi_init(base, sptr_drv);
    if (retval)
        goto fail5;

    return ER_NORMAL;

fail5:
    fwk_unregister_framebuffer(sptr_fb);
fail4:
    kfree(buffer);
fail3:
    xsdk_hdmi_driver_remove_axivdma(sptr_pdev);
fail2:
    fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);
    fwk_io_unmap(base);
fail1:
    kfree(sptr_fb);
    return -ER_FAILD;
}

/*!
 * @brief   xsdk_hdmi_driver_remove
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_hdmi_driver_remove(struct fwk_platdev *sptr_pdev)
{
    struct xsdk_hdmi_drv *sptr_drv;
    struct fwk_fb_info *sptr_fb;

    sptr_drv = (struct xsdk_hdmi_drv *)fwk_platform_get_drvdata(sptr_pdev);
    sptr_fb = sptr_drv->sptr_fb;

    fwk_unregister_framebuffer(sptr_fb);
    xsdk_hdmi_driver_remove_axivdma(sptr_pdev);
    fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);

    fwk_io_unmap(sptr_drv->base);
    kfree((void *)sptr_fb->sgtc_fix.smem_start);
    kfree(sptr_fb);
    
    return ER_NORMAL;
}

/*!< device id for device-tree */
static const struct fwk_of_device_id sgtc_xsdk_hdmi_driver_ids[] =
{
    { .compatible = "xlnx, vdma, hdmi", },
    {},
};

/*!< platform instance */
static struct fwk_platdrv sgtc_xsdk_hdmi_platdriver =
{
    .probe	= xsdk_hdmi_driver_probe,
    .remove	= xsdk_hdmi_driver_remove,
    
    .sgtc_driver =
    {
        .name 	= "xlnx, zynq7sdk, axivtc",
        .id 	= -1,
        .sptr_of_match_table = sgtc_xsdk_hdmi_driver_ids,
    },
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   xsdk_hdmi_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init xsdk_hdmi_driver_init(void)
{
    return fwk_register_platdriver(&sgtc_xsdk_hdmi_platdriver);
}

/*!
 * @brief   xsdk_hdmi_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit xsdk_hdmi_driver_exit(void)
{
    fwk_unregister_platdriver(&sgtc_xsdk_hdmi_platdriver);
}

IMPORT_DRIVER_INIT(xsdk_hdmi_driver_init);
IMPORT_DRIVER_EXIT(xsdk_hdmi_driver_exit);

/*!< end of file */
