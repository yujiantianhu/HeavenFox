/*
 * ZYNQ7 Peripheral APIs For AXIVDMA and VTC
 *
 * File Name:   zynq7_axivtc.c
 * Author:      Yang Yujun (Copy from "Xilinx SDK")
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.10.30
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <configs/configs.h>
#include <asm/armv7/gcc_config.h>
#include <zynq7/zynq7_periph.h>
#include <common/time.h>
#include <common/api_string.h>

/*!< The defines */
/* The polling upon starting the hardware
 *
 * We have the assumption that reset is fast upon hardware start
 */
#define INITIALIZATION_POLLING                  100000

/* Translate virtual address to physical address */
#define XAXIVDMA_VIRT_TO_PHYS(VirtAddr)         (VirtAddr)

/* Debug Configuration Parameter Constants (C_ENABLE_DEBUG_INFO_*) */
#define XAXIVDMA_ENABLE_DBG_THRESHOLD_REG	    0x01
#define XAXIVDMA_ENABLE_DBG_FRMSTORE_REG	    0x02
#define XAXIVDMA_ENABLE_DBG_FRM_CNTR	        0x04
#define XAXIVDMA_ENABLE_DBG_DLY_CNTR	        0x08
#define XAXIVDMA_ENABLE_DBG_ALL_FEATURES	    0x0F

/** GenLock Mode Constants */
#define XAXIVDMA_GENLOCK_MASTER		            0
#define XAXIVDMA_GENLOCK_SLAVE		            1
#define XAXIVDMA_DYN_GENLOCK_MASTER	            2
#define XAXIVDMA_DYN_GENLOCK_SLAVE	            3

/** VDMA data transfer direction */
#define XAXIVDMA_WRITE                          1           /**< DMA transfer into memory */
#define XAXIVDMA_READ                           2           /**< DMA transfer from memory */

/**
* This typedef contains Timing (typically in Display Timing) format
* configuration information for the VTC core.
*/
typedef struct 
{
    /* Horizontal Timing */
    kuint16_t HActiveVideo;	                                /**< Horizontal Active Video Size */
    kuint16_t HFrontPorch;	                                /**< Horizontal Front Porch Size */
    kuint16_t HSyncWidth;		                            /**< Horizontal Sync Width */
    kuint16_t HBackPorch;		                            /**< Horizontal Back Porch Size */
    kuint16_t HSyncPolarity;	                            /**< Horizontal Sync Polarity */

    /* Vertical Timing */
    kuint16_t VActiveVideo;	                                /**< Vertical Active Video Size */
    kuint16_t V0FrontPorch;	                                /**< Vertical Front Porch Size */
    kuint16_t V0SyncWidth;	                                /**< Vertical Sync Width */
    kuint16_t V0BackPorch;	                                /**< Horizontal Back Porch Size */

    kuint16_t V1FrontPorch;	                                /**< Vertical Front Porch Size */
    kuint16_t V1SyncWidth;	                                /**< Vertical Sync Width */
    kuint16_t V1BackPorch;	                                /**< Vertical Back Porch Size */

    kuint16_t VSyncPolarity;	                            /**< Vertical Sync Polarity */

    kuint8_t Interlaced;		                            /**< Interlaced / Progressive video */

} __align(4) XVtc_Timing;

/**
 * This typedef contains Polarity configuration information for a VTC core.
 */
typedef struct 
{
    kuint8_t ActiveChromaPol;	                            /**< Active Chroma Output Polarity */
    kuint8_t ActiveVideoPol;	                            /**< Active Video Output Polarity */
    kuint8_t FieldIdPol;		                            /**< Field ID Output Polarity */
    kuint8_t VBlankPol;		                                /**< Vertical Blank Output Polarity */
    kuint8_t VSyncPol;		                                /**< Vertical Sync Output Polarity */
    kuint8_t HBlankPol;		                                /**< Horizontal Blank Output Polarity */
    kuint8_t HSyncPol;		                                /**< Horizontal Sync Output Polarity */

} __align(4) XVtc_Polarity;

/**
 * This typedef contains the VTC signal configuration used by the
 * Generator/Detector modules in a VTC device.
 */
typedef struct 
{
    kuint16_t OriginMode;		                            /**< Origin Mode */
    kuint16_t HTotal;		                                /**< Horizontal total clock cycles per Line */
    kuint16_t HFrontPorchStart;	                            /**< Horizontal Front Porch Start Cycle Count */
    kuint16_t HSyncStart;		                            /**< Horizontal Sync Start Cycle Count */
    kuint16_t HBackPorchStart;	                            /**< Horizontal Back Porch Start Cycle Count */
    kuint16_t HActiveStart;	                                /**< Horizontal Active Video Start Cycle Count */

    kuint16_t V0Total;		                                /**< Total lines per Frame (Field 0) */
    kuint16_t V0FrontPorchStart;	                        /**< Vertical Front Porch Start Line Count * (Field 0) */
    kuint16_t V0SyncStart;	                                /**< Vertical Sync Start Line Count (Field 0) */
    kuint16_t V0BackPorchStart;	                            /**< Vertical Back Porch Start Line Count *  (Field 0) */
    kuint16_t V0ActiveStart;	                            /**< Vertical Active Video Start Line Count *  (Field 0) */
    kuint16_t V0ChromaStart;	                            /**< Active Chroma Start Line Count (Field 0) */

    kuint16_t V1Total;		                                /**< Total lines per Frame (Field 1) */
    kuint16_t V1FrontPorchStart;	                        /**< Vertical Front Porch Start Line Count *  (Field 1) */
    kuint16_t V1SyncStart;	                                /**< Vertical Sync Start Line Count (Field 1) */
    kuint16_t V1BackPorchStart;	                            /**< Vertical Back Porch Start Line Count  (Field 1) */
    kuint16_t V1ActiveStart;	                            /**< Vertical Active Video Start Line Count (Field 1) */
    kuint16_t V1ChromaStart;	                            /**< Active Chroma Start Line Count (Field 1) */
    kuint8_t Interlaced;		                            /**< Interlaced / Progressive video */

} __align(4) XVtc_Signal;

/**
 * This typedef contains Detector/Generator VBlank/VSync Horizontal Offset
 * configuration information for a VTC device.
 */
typedef struct 
{
    kuint16_t V0BlankHoriStart;	                            /**< Vertical Blank Hori Offset Start (field 0) */
    kuint16_t V0BlankHoriEnd;	                            /**< Vertical Blank Hori Offset End (field 0) */
    kuint16_t V0SyncHoriStart;	                            /**< Vertical Sync  Hori Offset Start (field 0) */
    kuint16_t V0SyncHoriEnd;	                            /**< Vertical Sync  Hori Offset End (field 0) */
    kuint16_t V1BlankHoriStart;	                            /**< Vertical Blank Hori Offset Start (field 1) */
    kuint16_t V1BlankHoriEnd;	                            /**< Vertical Blank Hori Offset End (field 1) */
    kuint16_t V1SyncHoriStart;	                            /**< Vertical Sync  Hori Offset Start (field 1) */
    kuint16_t V1SyncHoriEnd;	                            /**< Vertical Sync  Hori Offset End (field 1) */

} __align(4) XVtc_HoriOffsets;

/**
 * This typedef contains Source Selection configuration information for a
 * VTC core.
 */
typedef struct 
{
    kuint8_t FieldIdPolSrc;	                                /**< Field ID Output Polarity Source */
    kuint8_t ActiveChromaPolSrc;	                        /**< Active Chroma Output Polarity Source */
    kuint8_t ActiveVideoPolSrc;	                            /**< Active Video Output Polarity Source */
    kuint8_t HSyncPolSrc;		                            /**< Horizontal Sync Output Polarity Source */
    kuint8_t VSyncPolSrc;		                            /**< Vertical Sync Output Polarity Source */
    kuint8_t HBlankPolSrc;	                                /**< Horizontal Blank Output Polarity Source */
    kuint8_t VBlankPolSrc;	                                /**< Vertical Blank Output Polarity Source */

    kuint8_t VChromaSrc;		                            /**< Start of Active Chroma Register Source Select */
    kuint8_t VActiveSrc;		                            /**< Vertical Active Video Start Register Source Select */
    kuint8_t VBackPorchSrc;	                                /**< Vertical Back Porch Start Register Source Select */
    kuint8_t VSyncSrc;		                                /**< Vertical Sync Start Register Source Select */
    kuint8_t VFrontPorchSrc;	                            /**< Vertical Front Porch Start Register Source Select */
    kuint8_t VTotalSrc;		                                /**< Vertical Total Register Source Select */
    kuint8_t HActiveSrc;		                            /**< Horizontal Active Video Start Register Source Select */
    kuint8_t HBackPorchSrc;	                                /**< Horizontal Back Porch Start Register Source Select */
    kuint8_t HSyncSrc;		                                /**< Horizontal Sync Start Register Source Select */
    kuint8_t HFrontPorchSrc;	                            /**< Horizontal Front Porch Start Register Source Select */
    kuint8_t HTotalSrc;		                                /**< Horizontal Total Register Source Select */
    kuint8_t InterlacedMode;	                            /**< Interelaced mode */

} __align(4) XVtc_SourceSelect;

/*!< The globals */
static XAxiVdma_Config XAxiVdma_ConfigTable[XPAR_XAXIVDMA_NUM_INSTANCES] =
{
    {
        XPAR_AXI_VDMA_0_DEVICE_ID,
        XPAR_AXI_VDMA_0_BASEADDR,

        XPAR_AXI_VDMA_0_NUM_FSTORES,

        XPAR_AXI_VDMA_0_INCLUDE_MM2S,
        XPAR_AXI_VDMA_0_INCLUDE_MM2S_DRE,
        XPAR_AXI_VDMA_0_M_AXI_MM2S_DATA_WIDTH,
        XPAR_AXI_VDMA_0_INCLUDE_S2MM,
        XPAR_AXI_VDMA_0_INCLUDE_S2MM_DRE,
        XPAR_AXI_VDMA_0_M_AXI_S2MM_DATA_WIDTH,
        XPAR_AXI_VDMA_0_INCLUDE_SG,
        XPAR_AXI_VDMA_0_ENABLE_VIDPRMTR_READS,

        XPAR_AXI_VDMA_0_USE_FSYNC,
        XPAR_AXI_VDMA_0_FLUSH_ON_FSYNC,
        XPAR_AXI_VDMA_0_MM2S_LINEBUFFER_DEPTH,
        XPAR_AXI_VDMA_0_S2MM_LINEBUFFER_DEPTH,
        XPAR_AXI_VDMA_0_MM2S_GENLOCK_MODE,
        XPAR_AXI_VDMA_0_S2MM_GENLOCK_MODE,
        XPAR_AXI_VDMA_0_INCLUDE_INTERNAL_GENLOCK,
        XPAR_AXI_VDMA_0_S2MM_SOF_ENABLE,
        XPAR_AXI_VDMA_0_M_AXIS_MM2S_TDATA_WIDTH,
        XPAR_AXI_VDMA_0_S_AXIS_S2MM_TDATA_WIDTH,
        XPAR_AXI_VDMA_0_ENABLE_DEBUG_INFO_1,
        XPAR_AXI_VDMA_0_ENABLE_DEBUG_INFO_5,
        XPAR_AXI_VDMA_0_ENABLE_DEBUG_INFO_6,
        XPAR_AXI_VDMA_0_ENABLE_DEBUG_INFO_7,
        XPAR_AXI_VDMA_0_ENABLE_DEBUG_INFO_9,
        XPAR_AXI_VDMA_0_ENABLE_DEBUG_INFO_13,
        XPAR_AXI_VDMA_0_ENABLE_DEBUG_INFO_14,
        XPAR_AXI_VDMA_0_ENABLE_DEBUG_INFO_15,
        XPAR_AXI_VDMA_0_ENABLE_DEBUG_ALL,
        XPAR_AXI_VDMA_0_ADDR_WIDTH,
        XPAR_AXI_VDMA_0_ENABLE_VERT_FLIP
    }
};

static XVtc_Config XVtc_ConfigTable[XPAR_XVTC_NUM_INSTANCES] =
{
    {
        XPAR_V_TC_0_DEVICE_ID,
        XPAR_V_TC_0_BASEADDR
    }
};

static const kuint64_t lock_lookup[64] = 
{
    0b0011000110111110100011111010010000000001,
    0b0011000110111110100011111010010000000001,
    0b0100001000111110100011111010010000000001,
    0b0101101011111110100011111010010000000001,
    0b0111001110111110100011111010010000000001,
    0b1000110001111110100011111010010000000001,
    0b1001110011111110100011111010010000000001,
    0b1011010110111110100011111010010000000001,
    0b1100111001111110100011111010010000000001,
    0b1110011100111110100011111010010000000001,
    0b1111111111111000010011111010010000000001,
    0b1111111111110011100111111010010000000001,
    0b1111111111101110111011111010010000000001,
    0b1111111111101011110011111010010000000001,
    0b1111111111101000101011111010010000000001,
    0b1111111111100111000111111010010000000001,
    0b1111111111100011111111111010010000000001,
    0b1111111111100010011011111010010000000001,
    0b1111111111100000110111111010010000000001,
    0b1111111111011111010011111010010000000001,
    0b1111111111011101101111111010010000000001,
    0b1111111111011100001011111010010000000001,
    0b1111111111011010100111111010010000000001,
    0b1111111111011001000011111010010000000001,
    0b1111111111011001000011111010010000000001,
    0b1111111111010111011111111010010000000001,
    0b1111111111010101111011111010010000000001,
    0b1111111111010101111011111010010000000001,
    0b1111111111010100010111111010010000000001,
    0b1111111111010100010111111010010000000001,
    0b1111111111010010110011111010010000000001,
    0b1111111111010010110011111010010000000001,
    0b1111111111010010110011111010010000000001,
    0b1111111111010001001111111010010000000001,
    0b1111111111010001001111111010010000000001,
    0b1111111111010001001111111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001,
    0b1111111111001111101011111010010000000001
};

static const kuint32_t filter_lookup_low[64] = 
{
    0b0001011111,
    0b0001010111,
    0b0001111011,
    0b0001011011,
    0b0001101011,
    0b0001110011,
    0b0001110011,
    0b0001110011,
    0b0001110011,
    0b0001001011,
    0b0001001011,
    0b0001001011,
    0b0010110011,
    0b0001010011,
    0b0001010011,
    0b0001010011,
    0b0001010011,
    0b0001010011,
    0b0001010011,
    0b0001010011,
    0b0001010011,
    0b0001010011,
    0b0001010011,
    0b0001100011,
    0b0001100011,
    0b0001100011,
    0b0001100011,
    0b0001100011,
    0b0001100011,
    0b0001100011,
    0b0001100011,
    0b0001100011,
    0b0001100011,
    0b0001100011,
    0b0001100011,
    0b0001100011,
    0b0001100011,
    0b0010010011,
    0b0010010011,
    0b0010010011,
    0b0010010011,
    0b0010010011,
    0b0010010011,
    0b0010010011,
    0b0010010011,
    0b0010010011,
    0b0010010011,
    0b0010100011,
    0b0010100011,
    0b0010100011,
    0b0010100011,
    0b0010100011,
    0b0010100011,
    0b0010100011,
    0b0010100011,
    0b0010100011,
    0b0010100011,
    0b0010100011,
    0b0010100011,
    0b0010100011,
    0b0010100011,
    0b0010100011,
    0b0010100011,
    0b0010100011
};

/*!< The functions */
/*!
 * TODO:This function currently requires that the reference clock is 100MHz.
 * 		This should be changed so that the ref. clock can be specified, or read directly
 * 		out of hardware. This has been done in the linux driver, it just needs to be
 * 		ported here.
 */
kfloat_t ClkFindParams(float freq, ClkMode *sptr_bestPick)
{
    kfloat_t bestError = 2000.0;
    kfloat_t curError;
    kfloat_t curClkMult;
    kfloat_t curFreq;
    kuint32_t curDiv, curFb, curClkDiv;
    kuint32_t minFb = 0;
    kuint32_t maxFb = 0;

    /*!
     * This is necessary because the MMCM actual is generating 5x the desired pixel clock, and that
     * clock is then run through a BUFR that divides it by 5 to generate the pixel clock. Note this
     * means the pixel clock is on the Regional clock network, not the global clock network. In the
     * future if options like these are parameterized in the axi_dynclk core, then this function will
     * need to change.
     */
    freq = freq * 5.0;
    sptr_bestPick->freq = 0.0;

    /*! TODO: replace with a smarter algorithm that doesn't doesn't check every possible combination */
    for (curDiv = 1; curDiv <= 10; curDiv++)
    {
        /*< This accounts for the 100MHz input and the 600MHz minimum VCO */
        minFb = curDiv * 6; 
        /*!< This accounts for the 100MHz input and the 1200MHz maximum VCO */
        maxFb = curDiv * 12; 
        if (maxFb > 64)
            maxFb = 64;

        /*!< This multiplier is used to find the best clkDiv value for each FB value */
        curClkMult = (100.0 / (kfloat_t) curDiv) / freq; 

        curFb = minFb;
        while (curFb <= maxFb)
        {
            curClkDiv = (kuint32_t) ((curClkMult * (kfloat_t)curFb) + 0.5);
            curFreq = ((100.0 / (kfloat_t) curDiv) / (kfloat_t) curClkDiv) * (kfloat_t) curFb;
            curError = fabs(curFreq - freq);
            if (curError < bestError)
            {
                bestError = curError;
                sptr_bestPick->clkdiv = curClkDiv;
                sptr_bestPick->fbmult = curFb;
                sptr_bestPick->maindiv = curDiv;
                sptr_bestPick->freq = curFreq;
            }

            curFb++;
        }
    }

    /*!
     * We want the ClkMode struct and errors to be based on the desired frequency. 
     * Need to check this doesn't introduce rounding errors.
     */
    sptr_bestPick->freq = sptr_bestPick->freq / 5.0;
    bestError = bestError / 5.0;

    return bestError;
}

kuint32_t ClkDivider(kuint32_t divide)
{
    kuint32_t output = 0;
    kuint32_t highTime = 0;
    kuint32_t lowTime = 0;

    if ((divide < 1) || (divide > 128))
        return ERR_CLKDIVIDER;

    if (divide == 1)
        return 0x1041;

    highTime = divide / 2;

    /*!< if divide is odd */
    if (divide & 0b1)
    {
        lowTime = highTime + 1;
        output = 1 << CLK_BIT_WEDGE;
    }
    else
    {
        lowTime = highTime;
    }

    output |= 0x03F & lowTime;
    output |= 0xFC0 & (highTime << 6);

    return output;
}

kuint32_t ClkCountCalc(kuint32_t divide)
{
    kuint32_t output = 0;
    kuint32_t divCalc = 0;

    divCalc = ClkDivider(divide);
    if (divCalc == ERR_CLKDIVIDER)
        output = ERR_CLKCOUNTCALC;
    else
        output = (0xFFF & divCalc) | ((divCalc << 10) & 0x00C00000);

    return output;
}

void ClkWriteReg(ClkConfig *sptr_regVal, kuint32_t dynClkAddr)
{
    mr_writel(sptr_regVal->clk0L, dynClkAddr + OFST_DYNCLK_CLK_L);
    mr_writel(sptr_regVal->clkFBL, dynClkAddr + OFST_DYNCLK_FB_L);
    mr_writel(sptr_regVal->clkFBH_clk0H, dynClkAddr + OFST_DYNCLK_FB_H_CLK_H);
    mr_writel(sptr_regVal->divclk, dynClkAddr + OFST_DYNCLK_DIV);
    mr_writel(sptr_regVal->lockL, dynClkAddr + OFST_DYNCLK_LOCK_L);
    mr_writel(sptr_regVal->fltr_lockH, dynClkAddr + OFST_DYNCLK_FLTR_LOCK_H);
}

kuint32_t ClkFindReg(ClkConfig *sptr_regVal, ClkMode *sptr_params)
{
    if ((sptr_params->fbmult < 2) || sptr_params->fbmult > 64 )
        return 0;

    sptr_regVal->clk0L = ClkCountCalc(sptr_params->clkdiv);
    if (sptr_regVal->clk0L == ERR_CLKCOUNTCALC)
        return 0;

    sptr_regVal->clkFBL = ClkCountCalc(sptr_params->fbmult);
    if (sptr_regVal->clkFBL == ERR_CLKCOUNTCALC)
        return 0;

    sptr_regVal->clkFBH_clk0H = 0;

    sptr_regVal->divclk = ClkDivider(sptr_params->maindiv);
    if (sptr_regVal->divclk == ERR_CLKDIVIDER)
        return 0;

    sptr_regVal->lockL = (kuint32_t)(lock_lookup[sptr_params->fbmult - 1] & 0xFFFFFFFF);

    sptr_regVal->fltr_lockH = (kuint32_t)((lock_lookup[sptr_params->fbmult - 1] >> 32) & 0x000000FF);
    sptr_regVal->fltr_lockH |= ((filter_lookup_low[sptr_params->fbmult - 1] << 16) & 0x03FF0000);

    return 1;
}

void ClkStart(kuint32_t dynClkAddr)
{
    mr_writel((1 << BIT_DYNCLK_START), dynClkAddr + OFST_DYNCLK_CTRL);
    while (!(mr_readl(dynClkAddr + OFST_DYNCLK_STATUS) & (1 << BIT_DYNCLK_RUNNING)));
}

void ClkStop(kuint32_t dynClkAddr)
{
    mr_writel(0, dynClkAddr + OFST_DYNCLK_CTRL);
    while ((mr_readl(dynClkAddr + OFST_DYNCLK_STATUS) & (1 << BIT_DYNCLK_RUNNING)));
}

/*!
 * Read one word from BD
 *
 * @param sptr_bd is the BD to work on
 * @param Offset is the byte offset to read from
 *
 * @return
 *  The word value
 *
 */
static kuint32_t XAxiVdma_BdRead(XAxiVdma_Bd *sptr_bd, kint32_t Offset)
{
    return mr_readl((kuint32_t)sptr_bd + Offset);
}

/*!
 * Set one word in BD
 *
 * @param sptr_bd is the BD to work on
 * @param Offset is the byte offset to write to
 * @param Value is the value to write to the BD
 *
 * @return
 *  None
 */
static void XAxiVdma_BdWrite(XAxiVdma_Bd *sptr_bd, kint32_t Offset, kuint32_t Value)
{
    mr_writel(Value, (kuint32_t)sptr_bd + Offset);
}

/*!
 * Set the next ptr from BD
 *
 * @param sptr_bd is the BD to work on
 * @param NextPtr is the next ptr to set in BD
 *
 * @return
 *  None
 */
static void XAxiVdma_BdSetNextPtr(XAxiVdma_Bd *sptr_bd, kuint32_t NextPtr)
{
    XAxiVdma_BdWrite(sptr_bd, XAXIVDMA_BD_NDESC_OFFSET, NextPtr);
}

/*!
 * Set the vertical size for a BD
 *
 * @param sptr_bd is the BD to work on
 * @param Vsize is the vertical size to set in BD
 *
 * @return
 *  - ER_NORMAL if successful
 *  - ER_INVALID if argument Vsize is invalid
 */
static kint32_t XAxiVdma_BdSetVsize(XAxiVdma_Bd *sptr_bd, kint32_t Vsize)
{
    if ((Vsize <= 0) || (Vsize > XAXIVDMA_VSIZE_MASK))
        return -ER_INVALID;

    XAxiVdma_BdWrite(sptr_bd, XAXIVDMA_BD_VSIZE_OFFSET, Vsize);
    return ER_NORMAL;
}

/*!
 * Set the horizontal size for a BD
 *
 * @param sptr_bd is the BD to work on
 * @param Hsize is the horizontal size to set in BD
 *
 * @return
 *  - ER_NORMAL if successful
 *  - ER_INVALID if argument Hsize is invalid
 */
static kint32_t XAxiVdma_BdSetHsize(XAxiVdma_Bd *sptr_bd, kint32_t Hsize)
{
    if ((Hsize <= 0) || (Hsize > XAXIVDMA_HSIZE_MASK))
        return -ER_INVALID;

    XAxiVdma_BdWrite(sptr_bd, XAXIVDMA_BD_HSIZE_OFFSET, Hsize);
    return ER_NORMAL;
}

/*!
 * Set the stride size for a BD
 *
 * @param BdPtr is the BD to work on
 * @param Stride is the stride size to set in BD
 *
 * @return
 *  - ER_NORMAL if successful
 *  - ER_INVALID if argument Stride is invalid
 */
static kint32_t XAxiVdma_BdSetStride(XAxiVdma_Bd *sptr_bd, kint32_t Stride)
{
    kuint32_t Bits;

    if ((Stride <= 0) || (Stride > XAXIVDMA_STRIDE_MASK))
        return -ER_INVALID;

    Bits = XAxiVdma_BdRead(sptr_bd, XAXIVDMA_BD_STRIDE_OFFSET) & (~XAXIVDMA_STRIDE_MASK);
    XAxiVdma_BdWrite(sptr_bd, XAXIVDMA_BD_STRIDE_OFFSET, Bits | Stride);

    return ER_NORMAL;
}

/*!
 * Set the frame delay for a BD
 *
 * @param sptr_bd is the BD to work on
 * @param FrmDly is the frame delay value to set in BD
 *
 * @return
 *  - ER_NORMAL if successful
 *  - ER_INVALID if argument FrmDly is invalid
 */
static kint32_t XAxiVdma_BdSetFrmDly(XAxiVdma_Bd *sptr_bd, kint32_t FrmDly)
{
    kuint32_t Bits;

    if ((FrmDly < 0) || (FrmDly > XAXIVDMA_FRMDLY_MAX))
        return -ER_INVALID;

    Bits = XAxiVdma_BdRead(sptr_bd, XAXIVDMA_BD_STRIDE_OFFSET) & (~XAXIVDMA_FRMDLY_MASK);
    XAxiVdma_BdWrite(sptr_bd, XAXIVDMA_BD_STRIDE_OFFSET, Bits | (FrmDly << XAXIVDMA_FRMDLY_SHIFT));

    return ER_NORMAL;
}

/*!
 * Set the start address from BD
 *
 * The address is physical address.
 *
 * @param sptr_bd is the BD to work on
 * @param Addr is the address to set in BD
 *
 * @return
 *  None
 */
static void XAxiVdma_BdSetAddr(XAxiVdma_Bd *sptr_bd, kuint32_t Addr)
{
    XAxiVdma_BdWrite(sptr_bd, XAXIVDMA_BD_START_ADDR_OFFSET, Addr);
}

/*!
 * @brief   get vdma's major
 * @param   sptr_vdma
 * @retval  XAxiVdma
 * @note    none
 */
static kuint32_t XAxiVdma_Major(XAxiVdma *sptr_vdma) 
{
    kuint32_t Reg;

    Reg = XAxiVdma_ReadReg(sptr_vdma->BaseAddr, XAXIVDMA_VERSION_OFFSET);
    return ((Reg & XAXIVDMA_VERSION_MAJOR_MASK) >> XAXIVDMA_VERSION_MAJOR_SHIFT);
}

/*!
 * Get a channel
 *
 * @param sptr_vdma is the DMA engine to work on
 * @param Direction is the direction for the channel to get
 *
 * @return
 * The pointer to the channel. Upon error, return NULL.
 *
 * @note
 * Since this function is internally used, we assume Direction is valid
 */
XAxiVdma_Channel *XAxiVdma_GetChannel(XAxiVdma *sptr_vdma, kuint32_t Direction)
{
    if (Direction == XAXIVDMA_READ)
        return &(sptr_vdma->ReadChannel);

    else if (Direction == XAXIVDMA_WRITE)
        return &(sptr_vdma->WriteChannel);

    else 
        return mr_nullptr;
}

/*!
 * Initialize a channel of a DMA engine
 *
 * This function initializes the BD ring for this channel
 *
 * @param sptr_chan is the pointer to the DMA channel to work on
 *
 * @return
 *   None
 */
void XAxiVdma_ChannelInit(XAxiVdma_Channel *sptr_chan)
{
    kint32_t i;
    kint32_t NumFrames;
    XAxiVdma_Bd *FirstBdPtr = &(sptr_chan->BDs[0]);
    XAxiVdma_Bd *LastBdPtr;

    /*!< Initialize the BD variables, so proper memory management can be done */
    NumFrames = sptr_chan->NumFrames;

    sptr_chan->IsValid = 0;
    sptr_chan->HeadBdPhysAddr = 0;
    sptr_chan->HeadBdAddr = 0;
    sptr_chan->TailBdPhysAddr = 0;
    sptr_chan->TailBdAddr = 0;

    LastBdPtr = &(sptr_chan->BDs[NumFrames - 1]);

    /*!< Setup the BD ring */
    memset((void *)FirstBdPtr, 0, NumFrames * sizeof(XAxiVdma_Bd));

    for (i = 0; i < NumFrames; i++) 
    {
        XAxiVdma_Bd *BdPtr;
        XAxiVdma_Bd *NextBdPtr;

        BdPtr = &(sptr_chan->BDs[i]);

        /*!< The last BD connects to the first BD */
        if (i == (NumFrames - 1))
            NextBdPtr = FirstBdPtr;
        else
            NextBdPtr = &(sptr_chan->BDs[i + 1]);

        XAxiVdma_BdSetNextPtr(BdPtr, XAXIVDMA_VIRT_TO_PHYS((kuint32_t)NextBdPtr));
    }

    sptr_chan->AllCnt = NumFrames;

    /*!< Setup the BD addresses so that access the head/tail BDs fast */
    sptr_chan->HeadBdAddr = (kuint32_t)FirstBdPtr;
    sptr_chan->HeadBdPhysAddr = XAXIVDMA_VIRT_TO_PHYS((kuint32_t)FirstBdPtr);

    sptr_chan->TailBdAddr = (kuint32_t)LastBdPtr;
    sptr_chan->TailBdPhysAddr = XAXIVDMA_VIRT_TO_PHYS((kuint32_t)LastBdPtr);

    sptr_chan->IsValid = true;
}

/*!
 * This function resets one DMA channel
 *
 * The registers will be default values after the reset
 *
 * @param sptr_chan is the pointer to the DMA channel to work on
 *
 * @return
 *  None
 */
void XAxiVdma_ChannelReset(XAxiVdma_Channel *sptr_chan)
{
    XAxiVdma_WriteReg(sptr_chan->ChanBase, XAXIVDMA_CR_OFFSET, XAXIVDMA_CR_RESET_MASK);
}

/*!
 * This function checks whether reset operation is done
 *
 * @param sptr_chan is the pointer to the DMA channel to work on
 *
 * @return
 * - 0 if reset is done
 * - 1 if reset is still going
 */
kuint32_t XAxiVdma_ChannelResetNotDone(XAxiVdma_Channel *sptr_chan)
{
    return (XAxiVdma_ReadReg(sptr_chan->ChanBase, XAXIVDMA_CR_OFFSET) & XAXIVDMA_CR_RESET_MASK);
}

/*!
 * Check whether a DMA channel is busy
 *
 * @param sptr_chan is the pointer to the channel to work on
 *
 * @return
 * - non zero if the channel is busy
 * - 0 is the channel is idle
 */
kbool_t XAxiVdma_ChannelIsBusy(XAxiVdma_Channel *sptr_chan)
{
    kuint32_t Bits;

    /*!< If the channel is idle, then it is not busy */
    Bits = XAxiVdma_ReadReg(sptr_chan->ChanBase, XAXIVDMA_SR_OFFSET) & XAXIVDMA_SR_IDLE_MASK;
    if (Bits)
        return false;

    /*!< If the channel is halted, then it is not busy */
    Bits = XAxiVdma_ReadReg(sptr_chan->ChanBase, XAXIVDMA_SR_OFFSET) & XAXIVDMA_SR_HALTED_MASK;
    if (Bits)
        return false;

    /*!< Otherwise, it is busy */
    return true;
}

/*!
 * Check whether a DMA channel is running
 *
 * @param Channel is the pointer to the channel to work on
 *
 * @return
 * - non zero if the channel is running
 * - 0 is the channel is idle
 */
kbool_t XAxiVdma_ChannelIsRunning(XAxiVdma_Channel *sptr_chan)
{
    kuint32_t Bits;

    /*!< If halted bit set, channel is not running */
    Bits = XAxiVdma_ReadReg(sptr_chan->ChanBase, XAXIVDMA_SR_OFFSET) & XAXIVDMA_SR_HALTED_MASK;
    if (Bits)
        return false;

    /*!< If Run/Stop bit low, then channel is not running */
    Bits = XAxiVdma_ReadReg(sptr_chan->ChanBase, XAXIVDMA_CR_OFFSET) & XAXIVDMA_CR_RUNSTOP_MASK;
    if (!Bits)
        return false;

    return true;
}

/*!
 * Set the channel to disable access higher Frame Buffer Addresses (SG=0)
 *
 * @param sptr_chan is the pointer to the channel to work on
 */
void XAxiVdma_ChannelHiFrmAddrDisable(XAxiVdma_Channel *sptr_chan)
{
    XAxiVdma_WriteReg(sptr_chan->ChanBase,
                    XAXIVDMA_HI_FRMBUF_OFFSET, (XAXIVDMA_REGINDEX_MASK >> 1));
}

/*!
 * Set the channel to enable access to higher Frame Buffer Addresses (SG=0)
 *
 * @param sptr_chan is the pointer to the channel to work on
 */
void XAxiVdma_ChannelHiFrmAddrEnable(XAxiVdma_Channel *sptr_chan)
{
    XAxiVdma_WriteReg(sptr_chan->ChanBase,
                    XAXIVDMA_HI_FRMBUF_OFFSET, XAXIVDMA_REGINDEX_MASK);
}

/*!
 * Configure buffer addresses for one DMA channel
 *
 * The buffer addresses are physical addresses.
 * Access to 32 Frame Buffer Addresses in direct mode is done through
 * XAxiVdma_ChannelHiFrmAddrEnable/Disable Functions.
 * 0 - Access Bank0 Registers (0x5C - 0x98)
 * 1 - Access Bank1 Registers (0x5C - 0x98)
 *
 * @param sptr_chan is the pointer to the channel to work on
 * @param BufferAddrSet is the set of addresses for the transfers
 * @param NumFrames is the number of frames to set the address
 *
 * @return
 * - ER_NORMAL if successful
 * - ER_FAILD if channel has not being initialized
 * - ER_INVALID if buffer address not valid, for example, unaligned
 * address with no DRE built in the hardware
 */
kint32_t XAxiVdma_ChannelSetBufferAddr(XAxiVdma_Channel *sptr_chan,
                                    kuint32_t *BufferAddrSet, kint32_t NumFrames)
{
    kint32_t i;
    kuint32_t WordLenBits;
    kint32_t HiFrmAddr = 0;
    kint32_t FrmBound;
    kint32_t Loop16 = 0;

    if (sptr_chan->AddrWidth > 32)
        FrmBound = (XAXIVDMA_MAX_FRAMESTORE_64) / 2 - 1;
    else
        FrmBound = (XAXIVDMA_MAX_FRAMESTORE) / 2 - 1;

    if (!sptr_chan->IsValid)
        return -ER_FAILD;

    WordLenBits = (kuint32_t)(sptr_chan->WordLength - 1);

    /*!< If hardware has no DRE, then buffer addresses must be word-aligned */
    for (i = 0; i < NumFrames; i++) 
    {
        if (!sptr_chan->HasDRE) 
        {
            if (BufferAddrSet[i] & WordLenBits)
                return -ER_INVALID;
        }
    }

    for (i = 0; i < NumFrames; i++, Loop16++) 
    {
        XAxiVdma_Bd *BdPtr = (XAxiVdma_Bd *)(sptr_chan->HeadBdAddr + i * sizeof(XAxiVdma_Bd));

        if (sptr_chan->HasSG)
            XAxiVdma_BdSetAddr(BdPtr, BufferAddrSet[i]);
        else 
        {
            if ((i > FrmBound) && !HiFrmAddr) 
            {
                XAxiVdma_ChannelHiFrmAddrEnable(sptr_chan);
                HiFrmAddr = 1;
                Loop16 = 0;
            }

            if (sptr_chan->AddrWidth > 32) 
            {
                /*!< For a 40-bit address XAXIVDMA_MAX_FRAMESTORE value should be set to 16 */
                XAxiVdma_WriteReg(sptr_chan->StartAddrBase, 
                            XAXIVDMA_START_ADDR_OFFSET + Loop16 * XAXIVDMA_START_ADDR_LEN + i * 4,
                            (kuint32_t)(BufferAddrSet[i]));

                XAxiVdma_WriteReg(sptr_chan->StartAddrBase,
                            XAXIVDMA_START_ADDR_MSB_OFFSET + Loop16 * XAXIVDMA_START_ADDR_LEN + i * 4,
                            ((kuint64_t)(BufferAddrSet[i]) >> 32));
            } 
            else 
            {
                XAxiVdma_WriteReg(sptr_chan->StartAddrBase,
                            XAXIVDMA_START_ADDR_OFFSET + Loop16 * XAXIVDMA_START_ADDR_LEN,
                            BufferAddrSet[i]);
            }

            if ((NumFrames > FrmBound) && (i == (NumFrames - 1)))
                XAxiVdma_ChannelHiFrmAddrDisable(sptr_chan);
        }
    }

    return ER_NORMAL;
}

/*!
 * Check DMA channel errors
 *
 * @param sptr_chan is the pointer to the channel to work on
 *
 * @return
 *  	Error bits of the channel, 0 means no errors
 */
kuint32_t XAxiVdma_ChannelErrors(XAxiVdma_Channel *sptr_chan)
{
    return (XAxiVdma_ReadReg(sptr_chan->ChanBase, XAXIVDMA_SR_OFFSET) & XAXIVDMA_SR_ERR_ALL_MASK);
}

/*!
 * Clear DMA channel errors
 *
 * @param   sptr_chan is the pointer to the channel to work on
 * @param   ErrorMask is the mask of error bits to clear.
 *
 * @return  None
 */
void XAxiVdma_ClearChannelErrors(XAxiVdma_Channel *sptr_chan, kuint32_t ErrorMask)
{
    kuint32_t SrBits;

    /*!< Write on Clear bits */
    SrBits = XAxiVdma_ReadReg(sptr_chan->ChanBase, XAXIVDMA_SR_OFFSET) | ErrorMask;
    XAxiVdma_WriteReg(sptr_chan->ChanBase, XAXIVDMA_SR_OFFSET, SrBits);
}

/*!
 * Configure one DMA channel using the configuration structure
 *
 * Setup the control register and BDs, however, BD addresses are not set.
 *
 * @param sptr_chan is the pointer to the channel to work on
 * @param sptr_setup is the pointer to the setup structure
 *
 * @return
 * - ER_NORMAL if successful
 * - ER_FAULT if channel has not being initialized
 * - ER_BUSY if the DMA channel is not idle
 * - ER_INVALID if fields in ChannelCfgPtr is not valid
 */
kint32_t XAxiVdma_ChannelConfig(XAxiVdma_Channel *sptr_chan, XAxiVdma_ChannelSetup *sptr_setup)
{
    kuint32_t CrBits;
    kint32_t i;
    kint32_t NumBds;
    kint32_t Status;
    kuint32_t hsize_align;
    kuint32_t stride_align;

    if (!sptr_chan->IsValid)
        return -ER_FAULT;

    if (sptr_chan->HasSG && XAxiVdma_ChannelIsBusy(sptr_chan))
        return -ER_BUSY;

    sptr_chan->Vsize = sptr_setup->VertSizeInput;

    /*!< Check whether Hsize is properly aligned */
    if (sptr_chan->direction == XAXIVDMA_WRITE) 
    {
        if (sptr_setup->HoriSizeInput < sptr_chan->WordLength)
            hsize_align = (kuint32_t)sptr_chan->WordLength;
        else 
        {
            hsize_align = (kuint32_t)(sptr_setup->HoriSizeInput % sptr_chan->WordLength);
            if (hsize_align > 0)
                hsize_align = (sptr_chan->WordLength - hsize_align);
        }
    }
    else 
    {
        if (sptr_setup->HoriSizeInput < sptr_chan->WordLength)
            hsize_align = (kuint32_t)sptr_chan->WordLength;
        else 
        {
            hsize_align = (kuint32_t)(sptr_setup->HoriSizeInput % sptr_chan->StreamWidth);
            if (hsize_align > 0)
                hsize_align = (sptr_chan->StreamWidth - hsize_align);
        }
    }

    /*!< Check whether Stride is properly aligned */
    if (sptr_setup->Stride < sptr_chan->WordLength)
        stride_align = (kuint32_t)sptr_chan->WordLength;
    else 
    {
        stride_align = (kuint32_t)(sptr_setup->Stride % sptr_chan->WordLength);
        if (stride_align > 0)
            stride_align = (sptr_chan->WordLength - stride_align);
    }

    /*!< If hardware has no DRE, then Hsize and Stride must be word-aligned */
    if (!sptr_chan->HasDRE) 
    {
        if (hsize_align != 0) 
        {
            /*!< Adjust hsize to multiples of stream/mm data width */
            sptr_setup->HoriSizeInput += hsize_align;
        }
        if (stride_align != 0) 
        {
            /*!< Adjust stride to multiples of stream/mm data width */
            sptr_setup->Stride += stride_align;
        }
    }

    sptr_chan->Hsize = sptr_setup->HoriSizeInput;

    CrBits = XAxiVdma_ReadReg(sptr_chan->ChanBase, XAXIVDMA_CR_OFFSET);
    CrBits = XAxiVdma_ReadReg(sptr_chan->ChanBase, XAXIVDMA_CR_OFFSET) &
                ~(XAXIVDMA_CR_TAIL_EN_MASK | XAXIVDMA_CR_SYNC_EN_MASK |
                XAXIVDMA_CR_FRMCNT_EN_MASK | XAXIVDMA_CR_RD_PTR_MASK);

    if (sptr_setup->EnableCircularBuf)
        CrBits |= XAXIVDMA_CR_TAIL_EN_MASK;
    else 
    {
        /*!< Park mode */
        kuint32_t FrmBits;
        kuint32_t RegValue;

        if ((!XAxiVdma_ChannelIsRunning(sptr_chan)) && sptr_chan->HasSG)
            return -ER_INVALID;

        if (sptr_setup->FixedFrameStoreAddr > XAXIVDMA_FRM_MAX)
            return -ER_INVALID;

        if (sptr_chan->IsRead) 
        {
            FrmBits = sptr_setup->FixedFrameStoreAddr & XAXIVDMA_PARKPTR_READREF_MASK;

            RegValue  = XAxiVdma_ReadReg(sptr_chan->InstanceBase, XAXIVDMA_PARKPTR_OFFSET);
            RegValue &= ~XAXIVDMA_PARKPTR_READREF_MASK;
            RegValue |= FrmBits;

            XAxiVdma_WriteReg(sptr_chan->InstanceBase, XAXIVDMA_PARKPTR_OFFSET, RegValue);
        }
        else 
        {
            FrmBits = sptr_setup->FixedFrameStoreAddr << XAXIVDMA_WRTREF_SHIFT;

            FrmBits  &= XAXIVDMA_PARKPTR_WRTREF_MASK;
            RegValue  = XAxiVdma_ReadReg(sptr_chan->InstanceBase, XAXIVDMA_PARKPTR_OFFSET);
            RegValue &= ~XAXIVDMA_PARKPTR_WRTREF_MASK;
            RegValue |= FrmBits;

            XAxiVdma_WriteReg(sptr_chan->InstanceBase, XAXIVDMA_PARKPTR_OFFSET, RegValue);
        }
    }

    if (sptr_setup->EnableSync) 
    {
        if (sptr_chan->GenLock != XAXIVDMA_GENLOCK_MASTER)
            CrBits |= XAXIVDMA_CR_SYNC_EN_MASK;
    }

    if (sptr_setup->GenLockRepeat) 
    {
        if ((sptr_chan->GenLock == XAXIVDMA_GENLOCK_MASTER) ||
            (sptr_chan->GenLock == XAXIVDMA_DYN_GENLOCK_MASTER))
            CrBits |= XAXIVDMA_CR_GENLCK_RPT_MASK;
    }

    if (sptr_setup->EnableFrameCounter)
        CrBits |= XAXIVDMA_CR_FRMCNT_EN_MASK;

    CrBits |= (sptr_setup->PointNum << XAXIVDMA_CR_RD_PTR_SHIFT) & XAXIVDMA_CR_RD_PTR_MASK;

    /*!< Write the control register value out */
    XAxiVdma_WriteReg(sptr_chan->ChanBase, XAXIVDMA_CR_OFFSET, CrBits);

    if (sptr_chan->HasVFlip && !sptr_chan->IsRead) 
    {
        kuint32_t RegValue;

        RegValue  = XAxiVdma_ReadReg(sptr_chan->InstanceBase, XAXIVDMA_VFLIP_OFFSET);
        RegValue &= ~XAXIVDMA_VFLIP_EN_MASK;
        RegValue |= (sptr_setup->EnableVFlip & XAXIVDMA_VFLIP_EN_MASK);

        XAxiVdma_WriteReg(sptr_chan->InstanceBase, XAXIVDMA_VFLIP_OFFSET, RegValue);
    }

    if (sptr_chan->HasSG) 
    {
        /*!<
         * Setup the information in BDs
         *
         * All information is available except the buffer addrs
         * Buffer addrs are set through XAxiVdma_ChannelSetBufferAddr()
         */
        NumBds = sptr_chan->AllCnt;

        for (i = 0; i < NumBds; i++) 
        {
            XAxiVdma_Bd *sptr_bd = (XAxiVdma_Bd *)(sptr_chan->HeadBdAddr + i * sizeof(XAxiVdma_Bd));

            Status = XAxiVdma_BdSetVsize(sptr_bd, sptr_setup->VertSizeInput);
            if (Status)
                return Status;

            Status = XAxiVdma_BdSetHsize(sptr_bd, sptr_setup->HoriSizeInput);
            if (Status)
                return Status;

            Status = XAxiVdma_BdSetStride(sptr_bd, sptr_setup->Stride);
            if (Status)
                return Status;

            Status = XAxiVdma_BdSetFrmDly(sptr_bd, sptr_setup->FrameDelay);
            if (Status)
                return Status;
        }
    }
    else 
    {   /*!< direct register mode */
        if ((sptr_setup->VertSizeInput > XAXIVDMA_MAX_VSIZE) ||
            (sptr_setup->VertSizeInput <= 0) ||
            (sptr_setup->HoriSizeInput > XAXIVDMA_MAX_HSIZE) ||
            (sptr_setup->HoriSizeInput <= 0) ||
            (sptr_setup->Stride > XAXIVDMA_MAX_STRIDE) ||
            (sptr_setup->Stride <= 0) ||
            (sptr_setup->FrameDelay < 0) ||
            (sptr_setup->FrameDelay > XAXIVDMA_FRMDLY_MAX))
            return -ER_INVALID;

        XAxiVdma_WriteReg(sptr_chan->StartAddrBase,
                        XAXIVDMA_HSIZE_OFFSET, sptr_setup->HoriSizeInput);

        XAxiVdma_WriteReg(sptr_chan->StartAddrBase, XAXIVDMA_STRD_FRMDLY_OFFSET,
                        (sptr_setup->FrameDelay << XAXIVDMA_FRMDLY_SHIFT) | sptr_setup->Stride);
    }

    return ER_NORMAL;
}

/*!
 * Start one DMA channel
 *
 * @param Channel is the pointer to the channel to work on
 *
 * @return
 * - ER_NORMAL if successful
 * - ER_FAILD if channel is not initialized
 * - ER_FAULT if:
 *   . The DMA channel fails to stop
 *   . The DMA channel fails to start
 * - ER_BUSY is the channel is doing transfers
 */
kint32_t XAxiVdma_ChannelStart(XAxiVdma_Channel *sptr_chan)
{
    kuint32_t CrBits;

    if (!sptr_chan->IsValid)
        return -ER_FAILD;

    if (sptr_chan->HasSG && XAxiVdma_ChannelIsBusy(sptr_chan))
        return -ER_BUSY;

    /*!< If sptr_chan is not running, setup the CDESC register and set the sptr_chan to run */
    if (!XAxiVdma_ChannelIsRunning(sptr_chan)) 
    {
        if (sptr_chan->HasSG) 
        {
            /*!<
             * Set up the current bd register
             *
             * Can only setup current bd register when sptr_chan is halted
             */
            XAxiVdma_WriteReg(sptr_chan->ChanBase, XAXIVDMA_CDESC_OFFSET,
                            sptr_chan->HeadBdPhysAddr);
        }

        /*!< Start DMA hardware */
        CrBits = XAxiVdma_ReadReg(sptr_chan->ChanBase, XAXIVDMA_CR_OFFSET);
        CrBits = XAxiVdma_ReadReg(sptr_chan->ChanBase,
                                XAXIVDMA_CR_OFFSET) | XAXIVDMA_CR_RUNSTOP_MASK;
        XAxiVdma_WriteReg(sptr_chan->ChanBase, XAXIVDMA_CR_OFFSET, CrBits);

    }

    if (XAxiVdma_ChannelIsRunning(sptr_chan)) 
    {
        /*!< Start DMA transfers */
        if (sptr_chan->HasSG) 
        {
            /*!<
             * SG mode:
             * Update the tail pointer so that hardware will start
             * fetching BDs
             */
            XAxiVdma_WriteReg(sptr_chan->ChanBase, XAXIVDMA_TDESC_OFFSET,
                                sptr_chan->TailBdPhysAddr);
        }
        else 
        {
            /*!<
             * Direct register mode:
             * Update vsize to start the sptr_chan
             */
            XAxiVdma_WriteReg(sptr_chan->StartAddrBase,
                                XAXIVDMA_VSIZE_OFFSET, sptr_chan->Vsize);
        }

        return ER_NORMAL;
    }

    return -ER_FAULT;
}

/*!
 * Stop one DMA channel
 *
 * @param   sptr_chan is the pointer to the channel to work on
 *
 * @return  None
 */
void XAxiVdma_ChannelStop(XAxiVdma_Channel *sptr_chan)
{
    kuint32_t CrBits;

    if (!XAxiVdma_ChannelIsRunning(sptr_chan))
        return;

    /*!< Clear the RS bit in CR register */
    CrBits = XAxiVdma_ReadReg(sptr_chan->ChanBase, XAXIVDMA_CR_OFFSET) & (~XAXIVDMA_CR_RUNSTOP_MASK);
    XAxiVdma_WriteReg(sptr_chan->ChanBase, XAXIVDMA_CR_OFFSET, CrBits);
}

/*!
 * Set the channel to run in parking mode
 *
 * @param Channel is the pointer to the channel to work on
 *
 * @return
 *   - ER_NORMAL if everything is fine
 *   - ER_FAULT if hardware is not running
 *
 */
kint32_t XAxiVdma_ChannelStartParking(XAxiVdma_Channel *sptr_chan)
{
    kuint32_t CrBits;

    if (!XAxiVdma_ChannelIsRunning(sptr_chan))
        return -ER_FAULT;

    CrBits = XAxiVdma_ReadReg(sptr_chan->ChanBase, XAXIVDMA_CR_OFFSET) & (~XAXIVDMA_CR_TAIL_EN_MASK);
    XAxiVdma_WriteReg(sptr_chan->ChanBase, XAXIVDMA_CR_OFFSET, CrBits);

    return ER_NORMAL;
}

/*!
 * Check for DMA Channel Errors.
 *
 * @param 	sptr_vdma is the XAxiVdma instance to operate on
 * @param	Direction is the channel to work on, use XAXIVDMA_READ/WRITE
 *
 * @return	- Errors seen on the channel
 *		- ER_INVALID, when channel pointer is invalid.
 *		- ER_NOTFOUND, when the channel is not valid.
 *
 * @note	None
 */
kint32_t XAxiVdma_GetDmaChannelErrors(XAxiVdma *sptr_vdma, kuint16_t Direction)
{
    XAxiVdma_Channel *sptr_chan;

    sptr_chan = XAxiVdma_GetChannel(sptr_vdma, Direction);
    if (!sptr_chan)
        return -ER_INVALID;

    if (sptr_chan->IsValid)
        return XAxiVdma_ChannelErrors(sptr_chan);

    return -ER_NOTFOUND;
}

/*!
 * Clear DMA Channel Errors.
 *
 * @param 	sptr_vdma is the XAxiVdma instance to operate on
 * @param	Direction is the channel to work on, use XAXIVDMA_READ/WRITE
 * @param	ErrorMask is the mask of error bits to clear
 *
 * @return	- ER_NORMAL, when error bits are cleared.
 *		- ER_NOTFOUND, when channel pointer is invalid.
 *		- ER_INVALID, when the channel is not valid.
 *
 * @note	None
 */
kint32_t XAxiVdma_ClearDmaChannelErrors(XAxiVdma *sptr_vdma, kuint16_t Direction, kuint32_t ErrorMask)
{
    XAxiVdma_Channel *sptr_chan;

    sptr_chan = XAxiVdma_GetChannel(sptr_vdma, Direction);
    if (!sptr_chan)
        return -ER_INVALID;

    if (sptr_chan->IsValid) 
    {
        XAxiVdma_ClearChannelErrors(sptr_chan, ErrorMask);
        return ER_NORMAL;
    }

    return -ER_NOTFOUND;
}

/*!
 * Configure one DMA channel using the configuration structure
 *
 * @param sptr_vdma is the pointer to the DMA engine to work on
 * @param Direction is the DMA channel to work on
 * @param DmaConfigPtr is the pointer to the setup structure
 *
 * @return
 * - ER_NORMAL if successful
 * - ER_BUSY if the DMA channel is not idle, BDs are still being used
 * - ER_INVALID if buffer address not valid, for example, unaligned
 *   address with no DRE built in the hardware, or Direction invalid
 * - ER_NOTFOUND if the channel is invalid
 */
kint32_t XAxiVdma_DmaConfig(XAxiVdma *sptr_vdma, kuint16_t Direction, XAxiVdma_DmaSetup *sptr_setup)
{
    XAxiVdma_Channel *sptr_chan;

    sptr_chan = XAxiVdma_GetChannel(sptr_vdma, Direction);
    if (!sptr_chan)
        goto fail;

    if (sptr_chan->IsValid)
        return XAxiVdma_ChannelConfig(sptr_chan, (XAxiVdma_ChannelSetup *)sptr_setup);

fail:
    return -ER_NOTFOUND;
}

/*!
 * Configure buffer addresses for one DMA channel
 *
 * @param sptr_vdma is the pointer to the DMA engine to work on
 * @param Direction is the DMA channel to work on
 * @param BufferAddrSet is the set of addresses for the transfers
 *
 * @return
 * - ER_NORMAL if successful
 * - ER_BUSY if the DMA channel is not idle, BDs are still being used
 * - ER_INVALID if buffer address not valid, for example, unaligned
 *   address with no DRE built in the hardware, or Direction invalid
 * - ER_NOTFOUND if the channel is invalid
 */
kint32_t XAxiVdma_DmaSetBufferAddr(XAxiVdma *sptr_vdma, kuint16_t Direction, kuint32_t *BufferAddrSet)
{
    XAxiVdma_Channel *sptr_chan;

    sptr_chan = XAxiVdma_GetChannel(sptr_vdma, Direction);
    if (!sptr_chan)
        goto fail;

    if (sptr_chan->IsValid)
        return XAxiVdma_ChannelSetBufferAddr(sptr_chan, BufferAddrSet, sptr_chan->NumFrames);

fail:
    return -ER_NOTFOUND;
}

/*!
 * Start one DMA channel
 *
 * @param sptr_vdma is the pointer to the DMA engine to work on
 * @param Direction is the DMA channel to work on
 *
 * @return
 * - ER_NORMAL if channel started successfully
 * - ER_FAILD otherwise
 * - ER_NOTFOUND if the channel is invalid
 * - ER_INVALID if Direction invalid
 */
kint32_t XAxiVdma_DmaStart(XAxiVdma *sptr_vdma, kuint16_t Direction)
{
    XAxiVdma_Channel *sptr_chan;

    sptr_chan = XAxiVdma_GetChannel(sptr_vdma, Direction);
    if (!sptr_chan)
        goto fail;

    if (sptr_chan->IsValid)
        return XAxiVdma_ChannelStart(sptr_chan);

fail:
    return -ER_NOTFOUND;
}

/*!
 * Stop one DMA channel
 *
 * @param sptr_vdma is the pointer to the DMA engine to work on
 * @param Direction is the DMA channel to work on
 *
 * @return
 *  None
 *
 * @note
 * If channel is invalid, then do nothing on that channel
 */
void XAxiVdma_DmaStop(XAxiVdma *sptr_vdma, kuint16_t Direction)
{
    XAxiVdma_Channel *sptr_chan;

    sptr_chan = XAxiVdma_GetChannel(sptr_vdma, Direction);
    if (!sptr_chan)
        return;

    if (sptr_chan->IsValid)
        XAxiVdma_ChannelStop(sptr_chan);
}

/*!
 * Check whether a DMA channel is busy
 *
 * @param sptr_vdma is the pointer to the DMA engine to work on
 * @param Direction is the channel to work on, use XAXIVDMA_READ/WRITE
 *
 * @return
 * - Non-zero if the channel is busy
 * - Zero if the channel is idle
 */
kint32_t XAxiVdma_IsBusy(XAxiVdma *sptr_vdma, kuint16_t Direction)
{
    XAxiVdma_Channel *sptr_chan;

    sptr_chan = XAxiVdma_GetChannel(sptr_vdma, Direction);
    if (!sptr_chan)
        return 0;

    if (sptr_chan->IsValid)
        return XAxiVdma_ChannelIsBusy(sptr_chan);

    /*!< An invalid channel is never busy */
    return 0;
}

/*!
 * Start parking mode on a certain frame
 *
 * @param sptr_vdma is the pointer to the DMA engine to work on
 * @param FrameIndex is the frame to park on
 * @param Direction is the channel to work on, use XAXIVDMA_READ/WRITE
 *
 * @return
 *  - ER_NORMAL if everything is fine
 *  - ER_INVALID if
 *    . channel is invalid
 *    . FrameIndex is invalid
 *    . Direction is invalid
 */
kint32_t XAxiVdma_StartParking(XAxiVdma *sptr_vdma, kint32_t FrameIndex, kuint16_t Direction)
{
    XAxiVdma_Channel *sptr_chan;
    kuint32_t FrmBits;
    kuint32_t RegValue;
    kint32_t Status;

    if (FrameIndex > XAXIVDMA_FRM_MAX)
        return -ER_INVALID;

    if (Direction == XAXIVDMA_READ) 
    {
        FrmBits = FrameIndex & XAXIVDMA_PARKPTR_READREF_MASK;

        RegValue = XAxiVdma_ReadReg(sptr_vdma->BaseAddr, XAXIVDMA_PARKPTR_OFFSET);
        RegValue &= ~XAXIVDMA_PARKPTR_READREF_MASK;
        RegValue |= FrmBits;

        XAxiVdma_WriteReg(sptr_vdma->BaseAddr, XAXIVDMA_PARKPTR_OFFSET, RegValue);
    }
    else if (Direction == XAXIVDMA_WRITE) 
    {
        FrmBits = FrameIndex << XAXIVDMA_WRTREF_SHIFT;
        FrmBits &= XAXIVDMA_PARKPTR_WRTREF_MASK;

        RegValue = XAxiVdma_ReadReg(sptr_vdma->BaseAddr,
                      XAXIVDMA_PARKPTR_OFFSET);
        RegValue &= ~XAXIVDMA_PARKPTR_WRTREF_MASK;
        RegValue |= FrmBits;

        XAxiVdma_WriteReg(sptr_vdma->BaseAddr, XAXIVDMA_PARKPTR_OFFSET, RegValue);
    }
    else 
    {
        /*!< Invalid direction, do nothing */
        return -ER_INVALID;
    }

    sptr_chan = XAxiVdma_GetChannel(sptr_vdma, Direction);
    if (sptr_chan->IsValid) 
    {
        Status = XAxiVdma_ChannelStartParking(sptr_chan);
        if (Status)
            return Status;
    }

    return ER_NORMAL;
}

XAxiVdma_Config *XAxiVdma_LookupConfig(kuint16_t DeviceId)
{
    XAxiVdma_Config *sptr_cfg = mr_nullptr;
    kuint32_t i;

    for (i = 0U; i < XPAR_XAXIVDMA_NUM_INSTANCES; i++) 
    {
        if (XAxiVdma_ConfigTable[i].DeviceId == DeviceId) 
        {
            sptr_cfg = &XAxiVdma_ConfigTable[i];
            break;
        }
    }

    return sptr_cfg;
}

/*!
 * Initialize the driver with hardware configuration
 *
 * @param sptr_vdma is the pointer to the DMA engine to work on
 * @param sptr_cfg is the pointer to the hardware configuration structure
 * @param EffectiveAddr is the virtual address map for the device
 *
 * @return
 *  - ER_NORMAL if everything goes fine
 *  - ER_FAILD if reset the hardware failed, need system reset to recover
 *
 * @note
 * If channel fails reset,  then it will be set as invalid
 */
kint32_t XAxiVdma_CfgInitialize(XAxiVdma *sptr_vdma, XAxiVdma_Config *sptr_cfg, kuint32_t EffectiveAddr)
{
    XAxiVdma_Channel *sptr_rdchan;
    XAxiVdma_Channel *sptr_wrchan;
    kint32_t Polls;

    /*!< Validate parameters */
    if ((!sptr_vdma) ||
        (!sptr_cfg))
        return -ER_NULLPTR;

    /*!< Initially, no interrupt callback functions */
    sptr_vdma->ReadCallBack.CompletionCallBack = 0x0;
    sptr_vdma->ReadCallBack.ErrCallBack = 0x0;
    sptr_vdma->WriteCallBack.CompletionCallBack = 0x0;
    sptr_vdma->WriteCallBack.ErrCallBack = 0x0;

    sptr_vdma->BaseAddr = EffectiveAddr;
    sptr_vdma->MaxNumFrames = sptr_cfg->MaxFrameStoreNum;
    sptr_vdma->HasMm2S = sptr_cfg->HasMm2S;
    sptr_vdma->HasS2Mm = sptr_cfg->HasS2Mm;
    sptr_vdma->UseFsync = sptr_cfg->UseFsync;
    sptr_vdma->InternalGenLock = sptr_cfg->InternalGenLock;
    sptr_vdma->AddrWidth = sptr_cfg->AddrWidth;

    if (XAxiVdma_Major(sptr_vdma) < 3)
        sptr_vdma->HasSG = 1;
    else
        sptr_vdma->HasSG = sptr_cfg->HasSG;

    /*!< The channels are not valid until being initialized */
    sptr_rdchan = XAxiVdma_GetChannel(sptr_vdma, XAXIVDMA_READ);
    sptr_rdchan->IsValid = 0;

    sptr_wrchan = XAxiVdma_GetChannel(sptr_vdma, XAXIVDMA_WRITE);
    sptr_wrchan->IsValid = 0;

    if (sptr_vdma->HasMm2S) 
    {
        sptr_rdchan->direction = XAXIVDMA_READ;
        sptr_rdchan->ChanBase = sptr_vdma->BaseAddr + XAXIVDMA_TX_OFFSET;
        sptr_rdchan->InstanceBase = sptr_vdma->BaseAddr;
        sptr_rdchan->HasSG = sptr_vdma->HasSG;
        sptr_rdchan->IsRead = 1;
        sptr_rdchan->StartAddrBase = sptr_vdma->BaseAddr + XAXIVDMA_MM2S_ADDR_OFFSET;
        sptr_rdchan->NumFrames = sptr_cfg->MaxFrameStoreNum;

        /*!< Flush on Sync */
        sptr_rdchan->FlushonFsync = sptr_cfg->FlushonFsync;

        /*!< Dynamic Line Buffers Depth */
        sptr_rdchan->LineBufDepth = sptr_cfg->Mm2SBufDepth;
        if (sptr_rdchan->LineBufDepth > 0)
            sptr_rdchan->LineBufThreshold = XAxiVdma_ReadReg(sptr_rdchan->ChanBase, XAXIVDMA_BUFTHRES_OFFSET);

        sptr_rdchan->HasDRE = sptr_cfg->HasMm2SDRE;
        sptr_rdchan->WordLength = sptr_cfg->Mm2SWordLen >> 3;
        sptr_rdchan->StreamWidth = sptr_cfg->Mm2SStreamWidth >> 3;
        sptr_rdchan->AddrWidth = sptr_vdma->AddrWidth;

        /*!< Internal GenLock */
        sptr_rdchan->GenLock = sptr_cfg->Mm2SGenLock;

        /*!< Debug Info Parameter flags */
        if (!sptr_cfg->EnableAllDbgFeatures) 
        {
            if (sptr_cfg->Mm2SThresRegEn)
                sptr_rdchan->DbgFeatureFlags |= XAXIVDMA_ENABLE_DBG_THRESHOLD_REG;

            if (sptr_cfg->Mm2SFrmStoreRegEn)
                sptr_rdchan->DbgFeatureFlags |= XAXIVDMA_ENABLE_DBG_FRMSTORE_REG;

            if (sptr_cfg->Mm2SDlyCntrEn)
                sptr_rdchan->DbgFeatureFlags |= XAXIVDMA_ENABLE_DBG_DLY_CNTR;

            if (sptr_cfg->Mm2SFrmCntrEn)
                sptr_rdchan->DbgFeatureFlags |= XAXIVDMA_ENABLE_DBG_FRM_CNTR;
        } 
        else 
        {
            sptr_rdchan->DbgFeatureFlags = XAXIVDMA_ENABLE_DBG_ALL_FEATURES;
        }

        XAxiVdma_ChannelInit(sptr_rdchan);
        XAxiVdma_ChannelReset(sptr_rdchan);

        /*!< At time of initialization, no transfers are going on, reset is expected to be quick */
        Polls = INITIALIZATION_POLLING;
        while (Polls && XAxiVdma_ChannelResetNotDone(sptr_rdchan))
            Polls -= 1;

        if (!Polls)
            return -ER_FAILD;
    }

    if (sptr_vdma->HasS2Mm) 
    {
        sptr_wrchan->direction = XAXIVDMA_WRITE;
        sptr_wrchan->ChanBase = sptr_vdma->BaseAddr + XAXIVDMA_RX_OFFSET;
        sptr_wrchan->InstanceBase = sptr_vdma->BaseAddr;
        sptr_wrchan->HasSG = sptr_vdma->HasSG;
        sptr_wrchan->IsRead = 0;
        sptr_wrchan->StartAddrBase = sptr_vdma->BaseAddr + XAXIVDMA_S2MM_ADDR_OFFSET;
        sptr_wrchan->NumFrames = sptr_cfg->MaxFrameStoreNum;
        sptr_wrchan->AddrWidth = sptr_vdma->AddrWidth;
        sptr_wrchan->HasVFlip = sptr_cfg->HasVFlip;

        /*!< Flush on Sync */
        sptr_wrchan->FlushonFsync = sptr_cfg->FlushonFsync;

        /*!< Dynamic Line Buffers Depth */
        sptr_wrchan->LineBufDepth = sptr_cfg->S2MmBufDepth;
        if (sptr_wrchan->LineBufDepth > 0)
            sptr_wrchan->LineBufThreshold = XAxiVdma_ReadReg(sptr_wrchan->ChanBase, XAXIVDMA_BUFTHRES_OFFSET);

        sptr_wrchan->HasDRE = sptr_cfg->HasS2MmDRE;
        sptr_wrchan->WordLength = sptr_cfg->S2MmWordLen >> 3;
        sptr_wrchan->StreamWidth = sptr_cfg->S2MmStreamWidth >> 3;

        /*!< Internal GenLock */
        sptr_wrchan->GenLock = sptr_cfg->S2MmGenLock;

        /*!< Frame Sync Source Selection*/
        sptr_wrchan->S2MmSOF = sptr_cfg->S2MmSOF;

        /*!< Debug Info Parameter flags */
        if (!sptr_cfg->EnableAllDbgFeatures) 
        {
            if (sptr_cfg->S2MmThresRegEn)
                sptr_wrchan->DbgFeatureFlags |= XAXIVDMA_ENABLE_DBG_THRESHOLD_REG;

            if (sptr_cfg->S2MmFrmStoreRegEn)
                sptr_wrchan->DbgFeatureFlags |= XAXIVDMA_ENABLE_DBG_FRMSTORE_REG;

            if (sptr_cfg->S2MmDlyCntrEn)
                sptr_wrchan->DbgFeatureFlags |= XAXIVDMA_ENABLE_DBG_DLY_CNTR;

            if (sptr_cfg->S2MmFrmCntrEn)
                sptr_wrchan->DbgFeatureFlags |= XAXIVDMA_ENABLE_DBG_FRM_CNTR;

        } 
        else 
        {
            sptr_wrchan->DbgFeatureFlags = XAXIVDMA_ENABLE_DBG_ALL_FEATURES;
        }

        XAxiVdma_ChannelInit(sptr_wrchan);
        XAxiVdma_ChannelReset(sptr_wrchan);

        /*!< At time of initialization, no transfers are going on, reset is expected to be quick */
        Polls = INITIALIZATION_POLLING;
        while (Polls && XAxiVdma_ChannelResetNotDone(sptr_wrchan)) {
            Polls -= 1;
        }

        if (!Polls)
            return -ER_FAILD;
    }

    sptr_vdma->IsReady = true;

    return ER_NORMAL;
}

void XVtc_RegUpdateEnable(XVtc *sptr_vtc)
{
    kuint32_t reg;

    reg  = XVtc_ReadReg(sptr_vtc->Config.BaseAddress, XVTC_CTL_OFFSET);
    XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_CTL_OFFSET, reg | XVTC_CTL_RU_MASK);
}

/*!
* This function converts the video timing structure into the VTC signal
* configuration structure, horizontal offsets structure and the
* polarity structure.
*
* @param	sptr_vtc is a pointer to the VTC instance to be worked on.
* @param	sptr_timing is a pointer to a Video Timing structure to be read.
* @param	sptr_signal is a pointer to a VTC signal configuration to be set.
* @param	sptr_horiOff is a pointer to a VTC horizontal offsets structure to be set.
* @param	sptr_polarity is a pointer to a VTC polarity structure to be set.
*
* @return	None.
*
* @note		None.
*/
void XVtc_ConvTiming2Signal(XVtc *sptr_vtc, XVtc_Timing *sptr_timing,
                        XVtc_Signal *sptr_signal, XVtc_HoriOffsets *sptr_horiOff,
                        XVtc_Polarity *sptr_polarity)
{
    /* Verify arguments. */
    if ((!sptr_vtc) ||
        (!sptr_vtc->IsReady) ||
        (!sptr_timing) ||
        (!sptr_signal) ||
        (!sptr_horiOff) ||
        (!sptr_polarity))
        return;

    /*!< Setting up VTC Polarity.  */
    memset((void *)sptr_polarity, 0, sizeof(XVtc_Polarity));

    sptr_polarity->ActiveChromaPol = 1;
    sptr_polarity->ActiveVideoPol = 1;
    sptr_polarity->FieldIdPol = 1;

    /*!< Vblank matches Vsync Polarity */
    sptr_polarity->VBlankPol = sptr_timing->VSyncPolarity;
    sptr_polarity->VSyncPol = sptr_timing->VSyncPolarity;

    /*!< hblank matches hsync Polarity */
    sptr_polarity->HBlankPol = sptr_timing->HSyncPolarity;
    sptr_polarity->HSyncPol = sptr_timing->HSyncPolarity;

    memset((void *)sptr_signal, 0, sizeof(XVtc_Signal));
    memset((void *)sptr_horiOff, 0, sizeof(XVtc_HoriOffsets));

    /*!< Populate the VTC Signal config structure. */
    /*!< Active Video starts at 0 */
    sptr_signal->OriginMode = 1;
    sptr_signal->HActiveStart = 0;
    sptr_signal->HFrontPorchStart = sptr_timing->HActiveVideo;
    sptr_signal->HSyncStart = sptr_signal->HFrontPorchStart + sptr_timing->HFrontPorch;
    sptr_signal->HBackPorchStart = sptr_signal->HSyncStart + sptr_timing->HSyncWidth;
    sptr_signal->HTotal = sptr_signal->HBackPorchStart + sptr_timing->HBackPorch;

    sptr_signal->V0ChromaStart = 0;
    sptr_signal->V0ActiveStart = 0;
    sptr_signal->V0FrontPorchStart = sptr_timing->VActiveVideo;
    sptr_signal->V0SyncStart = sptr_signal->V0FrontPorchStart + sptr_timing->V0FrontPorch - 1;
    sptr_signal->V0BackPorchStart = sptr_signal->V0SyncStart + sptr_timing->V0SyncWidth;
    sptr_signal->V0Total = sptr_signal->V0BackPorchStart + sptr_timing->V0BackPorch + 1;

    sptr_horiOff->V0BlankHoriStart = sptr_signal->HFrontPorchStart;
    sptr_horiOff->V0BlankHoriEnd = sptr_signal->HFrontPorchStart;
    sptr_horiOff->V0SyncHoriStart = sptr_signal->HSyncStart;
    sptr_horiOff->V0SyncHoriEnd = sptr_signal->HSyncStart;

    if (sptr_timing->Interlaced == 1) 
    {
        sptr_signal->V1ChromaStart = 0;
        sptr_signal->V1ActiveStart = 0;
        sptr_signal->V1FrontPorchStart = sptr_timing->VActiveVideo;
        sptr_signal->V1SyncStart = sptr_signal->V1FrontPorchStart + sptr_timing->V1FrontPorch - 1;
        sptr_signal->V1BackPorchStart = sptr_signal->V1SyncStart + sptr_timing->V1SyncWidth;
        sptr_signal->V1Total = sptr_signal->V1BackPorchStart + sptr_timing->V1BackPorch + 1;
        sptr_signal->Interlaced = 1;

        /*!< Align to H blank */
        sptr_horiOff->V1BlankHoriStart = sptr_signal->HFrontPorchStart;
        /*!< Align to H Blank */
        sptr_horiOff->V1BlankHoriEnd = sptr_signal->HFrontPorchStart;

        /*!< Align to half line */
        sptr_horiOff->V1SyncHoriStart = sptr_signal->HSyncStart - (sptr_signal->HTotal / 2);
        sptr_horiOff->V1SyncHoriEnd = sptr_signal->HSyncStart - (sptr_signal->HTotal / 2);
    }
    /*!< Progressive formats */
    else
    {
        /*!< Set Field 1 same as Field 0 */
        sptr_signal->V1ChromaStart = sptr_signal->V0ChromaStart;
        sptr_signal->V1ActiveStart = sptr_signal->V0ActiveStart;
        sptr_signal->V1FrontPorchStart = sptr_signal->V0FrontPorchStart;
        sptr_signal->V1SyncStart = sptr_signal->V0SyncStart;
        sptr_signal->V1BackPorchStart = sptr_signal->V0BackPorchStart;
        sptr_signal->V1Total = sptr_signal->V0Total;
        sptr_signal->Interlaced = 0;

        sptr_horiOff->V1BlankHoriStart = sptr_horiOff->V0BlankHoriStart;
        sptr_horiOff->V1BlankHoriEnd = sptr_horiOff->V0BlankHoriEnd;
        sptr_horiOff->V1SyncHoriStart = sptr_horiOff->V0SyncHoriStart;
        sptr_horiOff->V1SyncHoriEnd = sptr_horiOff->V0SyncHoriEnd;
    }
}

/*!
* This function converts the VTC signal structure, horizontal offsets
* structure and the polarity structure into the Video Timing structure.
*
* @param	sptr_vtc is a pointer to the VTC instance to be
*		    worked on.
* @param	sptr_signal is a pointer to a VTC signal configuration to
*		    be read
* @param	sptr_horiOff is a pointer to a VTC horizontal offsets structure
*		    to be read
* @param	sptr_polarity is a pointer to a VTC polarity structure to be
*		    read.
* @param	sptr_timing is a pointer to a Video Timing structure to be set.
*
* @return	None.
*
* @note		None.
*/
void XVtc_ConvSignal2Timing(XVtc *sptr_vtc, XVtc_Signal *sptr_signal,
                        XVtc_HoriOffsets *sptr_horiOff, XVtc_Polarity *sptr_polarity,
                        XVtc_Timing *sptr_timing)
{
    /*!< Verify arguments. */
    if ((!sptr_vtc) ||
        (!sptr_vtc->IsReady) ||
        (!sptr_timing) ||
        (!sptr_signal) ||
        (sptr_signal->OriginMode != 1) ||
        (!sptr_horiOff) ||
        (!sptr_polarity))
        return;

    memset((void *)sptr_timing, 0, sizeof(XVtc_Timing));

    /*!< Set Polarity */
    sptr_timing->VSyncPolarity = sptr_polarity->VSyncPol;
    sptr_timing->HSyncPolarity = sptr_polarity->HSyncPol;

    /*!< Horizontal Timing */
    sptr_timing->HActiveVideo = sptr_signal->HFrontPorchStart;

    sptr_timing->HFrontPorch = sptr_signal->HSyncStart - sptr_signal->HFrontPorchStart;
    sptr_timing->HSyncWidth = sptr_signal->HBackPorchStart - sptr_signal->HSyncStart;
    sptr_timing->HBackPorch = sptr_signal->HTotal - sptr_signal->HBackPorchStart;

    /*!< Vertical Timing */
    sptr_timing->VActiveVideo = sptr_signal->V0FrontPorchStart;
    sptr_timing->V0FrontPorch = sptr_signal->V0SyncStart - sptr_signal->V0FrontPorchStart + 1;
    sptr_timing->V0SyncWidth = sptr_signal->V0BackPorchStart - sptr_signal->V0SyncStart + 1;
    sptr_timing->V0BackPorch = sptr_signal->V0Total - sptr_signal->V0BackPorchStart;

    sptr_timing->V1FrontPorch = sptr_signal->V1SyncStart - sptr_signal->V1FrontPorchStart + 1;
    sptr_timing->V1SyncWidth = sptr_signal->V1BackPorchStart - sptr_signal->V1SyncStart + 1;
    sptr_timing->V1BackPorch = sptr_signal->V1Total - sptr_signal->V1BackPorchStart;

    /*!< Interlaced */
    sptr_timing->Interlaced = sptr_signal->Interlaced;
}

/*!
* This function sets up the output polarity of the VTC core.
*
* @param	sptr_vtc is a pointer to the VTC instance to be worked on.
* @param	sptr_polarity points to a Polarity configuration structure with the setting to use on the VTC core.
*
* @return	None.
*
* @note		None.
*/
void XVtc_SetPolarity(XVtc *sptr_vtc, XVtc_Polarity *sptr_polarity)
{
    kuint32_t PolRegValue;

    /*!< Verify arguments. */
    if ((!sptr_vtc) ||
        (!sptr_vtc->IsReady) ||
        (!sptr_polarity))
        return;

    /*!< Read Control register value back and clear all polarity bits first */
    PolRegValue  = XVtc_ReadReg(sptr_vtc->Config.BaseAddress, (XVTC_GPOL_OFFSET));
    PolRegValue &= (kuint32_t)(~(XVTC_POL_ALLP_MASK));

    /*!< Change the register value according to the setting in the Polarity configuration structure */
    if (sptr_polarity->ActiveChromaPol)
        PolRegValue |= XVTC_POL_ACP_MASK;

    if (sptr_polarity->ActiveVideoPol)
        PolRegValue |= XVTC_POL_AVP_MASK;

    if (sptr_polarity->FieldIdPol)
        PolRegValue |= XVTC_POL_FIP_MASK;

    if (sptr_polarity->VBlankPol)
        PolRegValue |= XVTC_POL_VBP_MASK;

    if (sptr_polarity->VSyncPol)
        PolRegValue |= XVTC_POL_VSP_MASK;

    if (sptr_polarity->HBlankPol)
        PolRegValue |= XVTC_POL_HBP_MASK;

    if (sptr_polarity->HSyncPol)
        PolRegValue |= XVTC_POL_HSP_MASK;

    XVtc_WriteReg(sptr_vtc->Config.BaseAddress, (XVTC_GPOL_OFFSET), PolRegValue);
}

/*!
 * This function sets the VBlank/VSync Horizontal Offsets for the Generator
 * in a VTC device.
 *
 * @param  sptr_vtc is a pointer to the VTC device instance to be worked on.
 * @param  sptr_horiOff points to a VBlank/VSync Horizontal Offset configuration
 *	   with the setting to use on the VTC device.
 * @return NONE.
 */
void XVtc_SetGeneratorHoriOffset(XVtc *sptr_vtc, XVtc_HoriOffsets *sptr_horiOff)
{
    kuint32_t RegValue;

    /*!< Assert bad arguments and conditions */
    if ((!sptr_vtc) ||
        (!sptr_vtc->IsReady) ||
        (!sptr_horiOff))
        return;

    /*!< Calculate and update Generator VBlank Hori. Offset 0 register value */
    RegValue  = (sptr_horiOff->V0BlankHoriStart) & XVTC_XVXHOX_HSTART_MASK;
    RegValue |= (sptr_horiOff->V0BlankHoriEnd << XVTC_XVXHOX_HEND_SHIFT) & XVTC_XVXHOX_HEND_MASK;
    XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GVBHOFF_OFFSET, RegValue);

    /*!< Calculate and update Generator VSync Hori. Offset 0 register value */
    RegValue  = (sptr_horiOff->V0SyncHoriStart) & XVTC_XVXHOX_HSTART_MASK;
    RegValue |= (sptr_horiOff->V0SyncHoriEnd << XVTC_XVXHOX_HEND_SHIFT) & XVTC_XVXHOX_HEND_MASK;
    XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GVSHOFF_OFFSET, RegValue);

    /*!< Calculate and update Generator VBlank Hori. Offset 1 register value */
    RegValue = (sptr_horiOff->V1BlankHoriStart) & XVTC_XVXHOX_HSTART_MASK;
    RegValue |= (sptr_horiOff->V1BlankHoriEnd << XVTC_XVXHOX_HEND_SHIFT) & XVTC_XVXHOX_HEND_MASK;
    XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GVBHOFF_F1_OFFSET, RegValue);

    /*!< Calculate and update Generator VSync Hori. Offset 1 register value */
    RegValue = (sptr_horiOff->V1SyncHoriStart) & XVTC_XVXHOX_HSTART_MASK;
    RegValue |= (sptr_horiOff->V1SyncHoriEnd << XVTC_XVXHOX_HEND_SHIFT) & XVTC_XVXHOX_HEND_MASK;

    XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GVSHOFF_F1_OFFSET, RegValue);
}

/*!
 * This function sets up VTC signal to be used by the Generator module
 * in the VTC core.
 *
 * @param	sptr_vtc is a pointer to the VTC instance to be
 *		    worked on.
 * @param	sptr_signal is a pointer to the VTC signal configuration
 *		    to be used by the Generator module in the VTC core.
 *
 * @return	None.
 *
 * @note	None.
 */
void XVtc_SetGenerator(XVtc *sptr_vtc, XVtc_Signal *sptr_signal)
{
    kuint32_t RegValue;
    kuint32_t r_htotal, r_vtotal, r_hactive, r_vactive;
    XVtc_HoriOffsets sgtc_horiOff;

    /*!< Verify arguments. */
    if ((!sptr_vtc) ||
        (!sptr_vtc->IsReady) ||
        (!sptr_signal))
        return;

    if (sptr_signal->OriginMode == 0)
    {
        r_htotal = sptr_signal->HTotal+1;
        r_vtotal = sptr_signal->V0Total+1;

        r_hactive = r_htotal - sptr_signal->HActiveStart;
        r_vactive = r_vtotal - sptr_signal->V0ActiveStart;

        RegValue = (r_htotal) & XVTC_SB_START_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GHSIZE_OFFSET, RegValue);

        RegValue = (r_vtotal) & XVTC_VSIZE_F0_MASK;
        RegValue |= ((sptr_signal->V1Total+1) << XVTC_VSIZE_F1_SHIFT) & XVTC_VSIZE_F1_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GVSIZE_OFFSET, RegValue);

        RegValue = (r_hactive) & XVTC_ASIZE_HORI_MASK;
        RegValue |= ((r_vactive) << XVTC_ASIZE_VERT_SHIFT ) & XVTC_ASIZE_VERT_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GASIZE_OFFSET, RegValue);

        /*!<
         * For some resolutions, the FIELD1 vactive size is different
         * from FIELD0, e.g. XVIDC_VM_720x486_60_I (SDI NTSC),
         * As there is no vactive FIELD1 entry in the video common
         * library, program it separately. For resolutions where
         * vactive values are different, it should be taken care in
         * corrosponding driver. Otherwise program same values in
         * FIELD0 and FIELD1 registers 
         */
        RegValue = ((r_vactive) << XVTC_ASIZE_VERT_SHIFT) & XVTC_ASIZE_VERT_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GASIZE_F1_OFFSET, RegValue);

        /*!< Update the Generator Horizontal 1 Register */
        RegValue = (sptr_signal->HSyncStart + r_hactive) & XVTC_SB_START_MASK;
        RegValue |= ((sptr_signal->HBackPorchStart + r_hactive) << XVTC_SB_END_SHIFT) & XVTC_SB_END_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GHSYNC_OFFSET, RegValue);

        /*!< Update the Generator Vertical 1 Register (field 0) */
        RegValue = (sptr_signal->V0SyncStart + r_vactive -1) & XVTC_SB_START_MASK;
        RegValue |= ((sptr_signal->V0BackPorchStart + r_vactive -1) << XVTC_SB_END_SHIFT) & XVTC_SB_END_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GVSYNC_OFFSET, RegValue);

        /*!< Update the Generator Vertical Sync Register (field 1) */
        RegValue = (sptr_signal->V1SyncStart + r_vactive -1) & XVTC_SB_START_MASK;
        RegValue |= ((sptr_signal->V1BackPorchStart + r_vactive -1) << XVTC_SB_END_SHIFT) & XVTC_SB_END_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GVSYNC_F1_OFFSET, RegValue);

        /*!< Chroma Start */
        RegValue = XVtc_ReadReg(sptr_vtc->Config.BaseAddress, XVTC_GFENC_OFFSET);
        RegValue &= ~XVTC_ENC_CPARITY_MASK;
        RegValue = ((((sptr_signal->V0ChromaStart - sptr_signal->V0ActiveStart) << XVTC_ENC_CPARITY_SHIFT) &
                    XVTC_ENC_CPARITY_MASK) | RegValue);
        RegValue &= ~XVTC_ENC_PROG_MASK;
        RegValue |= (sptr_signal->Interlaced << XVTC_ENC_PROG_SHIFT) & XVTC_ENC_PROG_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GFENC_OFFSET, RegValue);

        /*!<
         * Setup default Horizontal Offsets - can override later with
         * XVtc_SetGeneratorHoriOffset()
         */
        sgtc_horiOff.V0BlankHoriStart = r_hactive;
        sgtc_horiOff.V0BlankHoriEnd = r_hactive;
        sgtc_horiOff.V0SyncHoriStart = sptr_signal->HSyncStart + r_hactive;
        sgtc_horiOff.V0SyncHoriEnd = sptr_signal->HSyncStart + r_hactive;

        sgtc_horiOff.V1BlankHoriStart = r_hactive;
        sgtc_horiOff.V1BlankHoriEnd = r_hactive;
        sgtc_horiOff.V1SyncHoriStart = sptr_signal->HSyncStart + r_hactive;
        sgtc_horiOff.V1SyncHoriEnd = sptr_signal->HSyncStart + r_hactive;
    }
    else
    {
        /*!< Total in mode=1 is the line width */
        r_htotal = sptr_signal->HTotal;
        /*!< Total in mode=1 is the frame height */
        r_vtotal = sptr_signal->V0Total;
        r_hactive = sptr_signal->HFrontPorchStart;
        r_vactive = sptr_signal->V0FrontPorchStart;

        RegValue = (r_htotal) & XVTC_SB_START_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GHSIZE_OFFSET, RegValue);

        RegValue = (r_vtotal) & XVTC_VSIZE_F0_MASK;
        RegValue |= ((sptr_signal->V1Total) << XVTC_VSIZE_F1_SHIFT) & XVTC_VSIZE_F1_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GVSIZE_OFFSET, RegValue);

        RegValue = (r_hactive) & XVTC_ASIZE_HORI_MASK;
        RegValue |= ((r_vactive) << XVTC_ASIZE_VERT_SHIFT) & XVTC_ASIZE_VERT_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GASIZE_OFFSET, RegValue);

        /*!<
         * For some resolutions, the FIELD1 vactive size is different
         * from FIELD0, e.g. XVIDC_VM_720x486_60_I (SDI NTSC),
         * As there is no vactive FIELD1 entry in the video common
         * library, program it separately. For resolutions where
         * vactive values are different, it should be taken care in
         * corrosponding driver. Otherwise program same values in
         * FIELD0 and FIELD1 registers 
         */
        RegValue = ((r_vactive) << XVTC_ASIZE_VERT_SHIFT) & XVTC_ASIZE_VERT_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GASIZE_F1_OFFSET, RegValue);

        /*!< Update the Generator Horizontal 1 Register */
        RegValue = (sptr_signal->HSyncStart) & XVTC_SB_START_MASK;
        RegValue |= ((sptr_signal->HBackPorchStart) << XVTC_SB_END_SHIFT) & XVTC_SB_END_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GHSYNC_OFFSET, RegValue);

        /*!< Update the Generator Vertical Sync Register (field 0) */
        RegValue = (sptr_signal->V0SyncStart) & XVTC_SB_START_MASK;
        RegValue |= ((sptr_signal->V0BackPorchStart) << XVTC_SB_END_SHIFT) & XVTC_SB_END_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GVSYNC_OFFSET, RegValue);

        /*!< Update the Generator Vertical Sync Register (field 1) */
        RegValue = (sptr_signal->V1SyncStart) & XVTC_SB_START_MASK;
        RegValue |= ((sptr_signal->V1BackPorchStart) << XVTC_SB_END_SHIFT) & XVTC_SB_END_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GVSYNC_F1_OFFSET, RegValue);

        /*!< Chroma Start */
        RegValue = XVtc_ReadReg(sptr_vtc->Config.BaseAddress, XVTC_GFENC_OFFSET);
        RegValue &= ~XVTC_ENC_CPARITY_MASK;
        RegValue = ((((sptr_signal->V0ChromaStart - sptr_signal->V0ActiveStart) << XVTC_ENC_CPARITY_SHIFT)
                    & XVTC_ENC_CPARITY_MASK) | RegValue);
        RegValue &= ~XVTC_ENC_PROG_MASK;
        RegValue |= (sptr_signal->Interlaced << XVTC_ENC_PROG_SHIFT) & XVTC_ENC_PROG_MASK;
        XVtc_WriteReg(sptr_vtc->Config.BaseAddress, XVTC_GFENC_OFFSET, RegValue);

        /*!<
         * Setup default Horizontal Offsets - can override later with
         * XVtc_SetGeneratorHoriOffset()
         */
        sgtc_horiOff.V0BlankHoriStart = r_hactive;
        sgtc_horiOff.V0BlankHoriEnd = r_hactive;
        sgtc_horiOff.V0SyncHoriStart = sptr_signal->HSyncStart;
        sgtc_horiOff.V0SyncHoriEnd = sptr_signal->HSyncStart;
        sgtc_horiOff.V1BlankHoriStart = r_hactive;
        sgtc_horiOff.V1BlankHoriEnd = r_hactive;
        sgtc_horiOff.V1SyncHoriStart = sptr_signal->HSyncStart;
        sgtc_horiOff.V1SyncHoriEnd = sptr_signal->HSyncStart;
    }

    XVtc_SetGeneratorHoriOffset(sptr_vtc, &sgtc_horiOff);
}

/*!
 * This function sets up the generator (Polarity, H/V values and horizontal
 * offsets) by reading the configuration from a video timing structure.
 *
 * @param	sptr_vtc is a pointer to the VTC instance to be
 *		    worked on.
 * @param	sptr_timing is a pointer to a Video Timing Structure to be read.
 *
 * @return	None.
 *
 * @note	None.
 */
void XVtc_SetGeneratorTiming(XVtc *sptr_vtc, XVtc_Timing * sptr_timing)
{
    XVtc_Polarity sgtc_polarity;
    XVtc_Signal sgtc_signal;
    XVtc_HoriOffsets sgtc_horiOff;

    /*!< Verify arguments. */
    if ((!sptr_vtc) ||
        (!sptr_vtc->IsReady) ||
        (!sptr_timing))
        return;

    XVtc_ConvTiming2Signal(sptr_vtc, sptr_timing, &sgtc_signal, &sgtc_horiOff, &sgtc_polarity);
    XVtc_SetPolarity(sptr_vtc, &sgtc_polarity);
    XVtc_SetGenerator(sptr_vtc, &sgtc_signal);
    XVtc_SetGeneratorHoriOffset(sptr_vtc, &sgtc_horiOff);
}

/*!
 * This function enables the VTC Generator core.
 *
 * @param	sptr_vtc is a pointer to the VTC instance to be
 *		    worked on.
 *
 * @return	None.
 *
 * @note	None.
 */
void XVtc_EnableGenerator(XVtc *sptr_vtc)
{
    kuint32_t CtrlRegValue;

    /*!< Verify arguments. */
    if ((!sptr_vtc) ||
        (!sptr_vtc->IsReady))
        return;

    /*!< Read Control register value back */
    CtrlRegValue = XVtc_ReadReg(sptr_vtc->Config.BaseAddress, (XVTC_CTL_OFFSET));

    /*!< Change the value according to the enabling type and write it back */
    CtrlRegValue |= XVTC_CTL_GE_MASK;

    XVtc_WriteReg(sptr_vtc->Config.BaseAddress, (XVTC_CTL_OFFSET), CtrlRegValue);
}

/*!
 * This function disables the VTC Generator core.
 *
 * @param	InstancePtr is a pointer to the VTC instance to be
 *		    worked on.
 *
 * @return	None.
 *
 * @note	None.
 */
void XVtc_DisableGenerator(XVtc *sptr_vtc)
{
    kuint32_t CtrlRegValue;

    /*!< Verify arguments. */
    if ((!sptr_vtc) ||
        (!sptr_vtc->IsReady))
        return;

    /*!< Read Control register value back */
    CtrlRegValue = XVtc_ReadReg(sptr_vtc->Config.BaseAddress, (XVTC_CTL_OFFSET));

    /*!< Change the value according to the disabling type and write it back */
    CtrlRegValue &= (kuint32_t)(~(XVTC_CTL_GE_MASK));

    XVtc_WriteReg(sptr_vtc->Config.BaseAddress, (XVTC_CTL_OFFSET), CtrlRegValue);
}

/*!
 * This function sets up the source selecting of the VTC core.
 *
 * @param	sptr_vtc is a pointer to the VTC instance to be
 *		    worked on
 * @param 	sptr_srcSel points to a Source Selecting configuration structure
 *		    with the setting to use on the VTC device.
 *
 * @return	None.
 *
 * @note	None.
 */
void XVtc_SetSource(XVtc *sptr_vtc, XVtc_SourceSelect *sptr_srcSel)
{
    kuint32_t CtrlRegValue;

    /*!< Verify arguments. */
    if ((!sptr_vtc) ||
        (!sptr_vtc->IsReady) ||
        (!sptr_srcSel))
        return;

    /*!< Read Control register value back and clear all source selection bits first */
    CtrlRegValue = XVtc_ReadReg(sptr_vtc->Config.BaseAddress, (XVTC_CTL_OFFSET));
    CtrlRegValue &= ~XVTC_CTL_ALLSS_MASK;

    /*!< Change the register value according to the setting in the source selection configuration structure */
    if (sptr_srcSel->FieldIdPolSrc)
        CtrlRegValue |= XVTC_CTL_FIPSS_MASK;

    if (sptr_srcSel->ActiveChromaPolSrc)
        CtrlRegValue |= XVTC_CTL_ACPSS_MASK;

    if (sptr_srcSel->ActiveVideoPolSrc)
        CtrlRegValue |= XVTC_CTL_AVPSS_MASK;

    if (sptr_srcSel->HSyncPolSrc)
        CtrlRegValue |= XVTC_CTL_HSPSS_MASK;

    if (sptr_srcSel->VSyncPolSrc)
        CtrlRegValue |= XVTC_CTL_VSPSS_MASK;

    if (sptr_srcSel->HBlankPolSrc)
        CtrlRegValue |= XVTC_CTL_HBPSS_MASK;

    if (sptr_srcSel->VBlankPolSrc)
        CtrlRegValue |= XVTC_CTL_VBPSS_MASK;

    if (sptr_srcSel->VChromaSrc)
        CtrlRegValue |= XVTC_CTL_VCSS_MASK;

    if (sptr_srcSel->VActiveSrc)
        CtrlRegValue |= XVTC_CTL_VASS_MASK;

    if (sptr_srcSel->VBackPorchSrc)
        CtrlRegValue |= XVTC_CTL_VBSS_MASK;

    if (sptr_srcSel->VSyncSrc)
        CtrlRegValue |= XVTC_CTL_VSSS_MASK;

    if (sptr_srcSel->VFrontPorchSrc)
        CtrlRegValue |= XVTC_CTL_VFSS_MASK;

    if (sptr_srcSel->VTotalSrc)
        CtrlRegValue |= XVTC_CTL_VTSS_MASK;

    if (sptr_srcSel->HBackPorchSrc)
        CtrlRegValue |= XVTC_CTL_HBSS_MASK;

    if (sptr_srcSel->HSyncSrc)
        CtrlRegValue |= XVTC_CTL_HSSS_MASK;

    if (sptr_srcSel->HFrontPorchSrc)
        CtrlRegValue |= XVTC_CTL_HFSS_MASK;

    if (sptr_srcSel->HTotalSrc)
        CtrlRegValue |= XVTC_CTL_HTSS_MASK;

    if (sptr_srcSel->InterlacedMode)
        CtrlRegValue |= XVTC_CTL_INTERLACE_MASK;

    XVtc_WriteReg(sptr_vtc->Config.BaseAddress, (XVTC_CTL_OFFSET), CtrlRegValue);
}

/*!
 * This function reads version register of the VTC core and compares with zero
 * as part of self test.
 *
 * @param	sptr_vtc is a pointer to the XVtc instance.
 *
 * @return
 *		    - ER_NORMAL if the Version register read test was successful.
 *		    - ER_FAULT if the Version register read test failed.
 *
 * @note	None.
 */
kint32_t XVtc_SelfTest(XVtc *sptr_vtc)
{
    kuint32_t Version;

    /*!< Verify argument. */
    if (!sptr_vtc)
        return -ER_NULLPTR;

    /*!< Read VTC core version register. */
    Version = XVtc_ReadReg((sptr_vtc)->Config.BaseAddress, (XVTC_VER_OFFSET));

    /*!< Compare version with zero */
    if (!Version)
        return -ER_FAULT;

    return ER_NORMAL;
}

XVtc_Config *XVtc_LookupConfig(kuint16_t DeviceId)
{
    XVtc_Config *sptr_vcfg = mr_nullptr;
    kint32_t i;

    /*!< Checking for device id for which instance it is matching */
    for (i = 0; i < XPAR_XVTC_NUM_INSTANCES; i++) 
    {
        /*!< Assigning address of config table if both device ids are matched */
        if (XVtc_ConfigTable[i].DeviceId == DeviceId) 
        {
            sptr_vcfg = &XVtc_ConfigTable[i];
            break;
        }
    }

    return sptr_vcfg;
}

/*!
 * This function initializes the VTC core. This function must be called
 * prior to using the VTC core. Initialization of the VTC includes setting up
 * the instance data, and ensuring the hardware is in a quiescent state.
 *
 * @param	sptr_vtc is a pointer to the VTC core instance to be
 *		    worked on.
 * @param	CfgPtr points to the configuration structure associated with
 *		    the VTC core.
 * @param	EffectiveAddr is the base address of the device. If address
 *		    translation is being used, then this parameter must reflect the
 *		    virtual base address. Otherwise, the physical address should be
 *		    used.
 *
 * @return
 *		    - ER_NORMAL if XVtc_CfgInitialize was successful.
 *
 * @note	None.
 */
kint32_t XVtc_CfgInitialize(XVtc *sptr_vtc, XVtc_Config *sptr_vcfg, kuint32_t EffectiveAddr)
{
    /*!< Verify arguments */
    if ((!sptr_vtc) ||
        (!sptr_vcfg) ||
        (!EffectiveAddr))
        return -ER_NULLPTR;

    /*!< Setup the instance */
    memset((void *)sptr_vtc, 0, sizeof(XVtc));

    memcpy((void *)&(sptr_vtc->Config), (const void *)sptr_vcfg, sizeof(XVtc_Config));
    sptr_vtc->Config.BaseAddress = EffectiveAddr;

    /*!< Set all handlers to stub values, let user configure this data later */
    sptr_vtc->FrameSyncCallBack = mr_nullptr;
    sptr_vtc->LockCallBack = mr_nullptr;
    sptr_vtc->DetectorCallBack = mr_nullptr;
    sptr_vtc->GeneratorCallBack = mr_nullptr;
    sptr_vtc->ErrCallBack = mr_nullptr;

    /*!< Set the flag to indicate the driver is ready */
    sptr_vtc->IsReady = true;

    return ER_NORMAL;
}

/*!	
 * DisplayInitialize(DisplayCtrl *sptr_disp, XAxiVdma *vdma, kuint16_t vtcId, 
 *                  kuint32_t dynClkAddr, u8 *framePtr[DISPLAY_NUM_FRAMES], kuint32_t stride)
 *
 *	Parameters:
 *		sptr_disp - Pointer to the struct that will be initialized
 *		vdma - Pointer to initialized VDMA struct
 *		vtcId - Device ID of the VTC core as found in xparameters.h
 *		dynClkAddr - BASE ADDRESS of the axi_dynclk core
 *		framePtr - array of pointers to the frame buffers. The frame buffers must be instantiated above this driver, and there must be 3
 *		stride - line stride of the frame buffers. This is the number of bytes between the start of one line and the start of another.
 *
 *	Return Value: kint32_t
 *		XST_SUCCESS if successful, XST_FAILURE otherwise
 *
 *	Errors:
 *
 *	Description:
 *		Initializes the driver struct for use.
 */
kint32_t DisplayInitialize(DisplayCtrl *sptr_disp, XAxiVdma *sptr_vdma, kuint16_t vtcId, 
                    kuint32_t dynClkAddr, kuint8_t *framePtr[DISPLAY_NUM_FRAMES], 
                    kuint32_t stride, VideoMode *sptr_vmode)
{
//  kint32_t Status;
    kint32_t i;
//  XVtc_Config *sptr_vcfg;
    ClkConfig sgtc_clkcfg;
    ClkMode sgtc_clkmode;

    /*!< Initialize all the fields in the DisplayCtrl struct */
    sptr_disp->curFrame = 0;
    sptr_disp->dynClkAddr = dynClkAddr;
    for (i = 0; i < DISPLAY_NUM_FRAMES; i++)
        sptr_disp->framePtr[i] = framePtr[i];

    sptr_disp->state = NR_XIL_DISPLAY_STOPPED;
    sptr_disp->stride = stride;

    /*!< Supported resolution */
    memcpy(&sptr_disp->vMode, sptr_vmode, sizeof(*sptr_vmode));

    ClkFindParams(sptr_disp->vMode.freq, &sgtc_clkmode);

    /*!<
     * Store the obtained frequency to pxlFreq. It is possible that the PLL was not able to
     * exactly generate the desired pixel clock, so this may differ from vMode.freq.
     */
    sptr_disp->pxlFreq = sgtc_clkmode.freq;

    /*!< Write to the PLL dynamic configuration registers to configure it with the calculated parameters. */
    if (!ClkFindReg(&sgtc_clkcfg, &sgtc_clkmode))
        return -ER_FAILD;

    ClkWriteReg(&sgtc_clkcfg, sptr_disp->dynClkAddr);

    /*!< Enable the dynamically generated clock */
    ClkStart(sptr_disp->dynClkAddr);

    /*!<
     * Initialize the VTC driver so that it's ready to use look up
     * configuration in the config table, then initialize it.
     */
//  sptr_vcfg = XVtc_LookupConfig(vtcId);
//  /*!< Checking Config variable */
//  if (mr_nullptr == sptr_vcfg)
//      return -ER_FAILD;

//  Status = XVtc_CfgInitialize(&(sptr_disp->vtc), sptr_vcfg, sptr_vcfg->BaseAddress);
//  /*!< Checking status */
//  if (Status)
//      return Status;

    sptr_disp->vdma = sptr_vdma;

    /*!< Initialize the VDMA Read configuration struct */
    sptr_disp->vdmaConfig.FrameDelay = 0;
    sptr_disp->vdmaConfig.EnableCircularBuf = 1;
    sptr_disp->vdmaConfig.EnableSync = 0;
    sptr_disp->vdmaConfig.PointNum = 0;
    sptr_disp->vdmaConfig.EnableFrameCounter = 0;

    return ER_NORMAL;
}

/*!
 *	DisplayChangeFrameBuffer(DisplayCtrl *sptr_disp, kuint32_t FrameAddr, kusize_t FrameSize)
 *
 *	Parameters:
 *		sptr_disp - Pointer to the initialized DisplayCtrl struct
 *      FrameAddr - New FrameBuffer Address
 *      
 *	Return Value: kint32_t
 *		ER_NORMAL if successful, ER_INVALID otherwise
 *
 *	Errors:
 *
 *	Description:
 *		Set FrameBuffer Address
 */
kint32_t DisplayChangeFrameBuffer(DisplayCtrl *sptr_disp, kuint32_t FrameAddr, kusize_t FrameSize)
{
    kint32_t index, Status;

    /*!< If already stopped, do nothing */
    if (sptr_disp->state == NR_XIL_DISPLAY_STOPPED)
        return -ER_NREADY;

    /*!<  Stop the VDMA core */
//  XAxiVdma_DmaStop(sptr_disp->vdma, XAXIVDMA_READ);
//  while (XAxiVdma_IsBusy(sptr_disp->vdma, XAXIVDMA_READ));

    index = sptr_disp->curFrame;
    sptr_disp->framePtr[index] = (kuint8_t *)FrameAddr;
    sptr_disp->vdmaConfig.FrameStoreStartAddr[index] = FrameAddr;
//  Xil_DCacheFlushRange(FrameAddr, FrameSize);

    Status = XAxiVdma_DmaSetBufferAddr(sptr_disp->vdma, XAXIVDMA_READ, 
                                    sptr_disp->vdmaConfig.FrameStoreStartAddr);
    if (Status)
        return Status;

    Status = XAxiVdma_DmaStart(sptr_disp->vdma, XAXIVDMA_READ);
    if (Status)
        return Status;

    return ER_NORMAL;
}

/*!
 *	DisplayStart(DisplayCtrl *sptr_disp)
 *
 *	Parameters:
 *		sptr_disp - Pointer to the initialized DisplayCtrl struct
 *
 *	Return Value: kint32_t
 *		ER_NORMAL if successful, ER_INVALID otherwise
 *
 *	Errors:
 *
 *	Description:
 *		Starts the display.
 */
kint32_t DisplayStart(DisplayCtrl *sptr_disp)
{
    kint32_t Status;
    ClkConfig sgtc_clkcfg;
    ClkMode sgtc_clkmode;
    kint32_t i;
    XVtc_Timing sgtc_timing;
    XVtc_SourceSelect sgtc_srcSel;

    /*!< If already started, do nothing */
    if (sptr_disp->state == NR_XIL_DISPLAY_RUNNING)
        return ER_NORMAL;

    /*!< Calculate the PLL divider parameters based on the required pixel clock frequency */
    ClkFindParams(sptr_disp->vMode.freq, &sgtc_clkmode);

    /*!<
     * Store the obtained frequency to pxlFreq. It is possible that the PLL was not able to
     * exactly generate the desired pixel clock, so this may differ from vMode.freq.
     */
    sptr_disp->pxlFreq = sgtc_clkmode.freq;

    /*!<
     * Write to the PLL dynamic configuration registers to configure it with the calculated
     * parameters.
     */
    if (!ClkFindReg(&sgtc_clkcfg, &sgtc_clkmode))
        return -ER_NOTFOUND;

    ClkWriteReg(&sgtc_clkcfg, sptr_disp->dynClkAddr);

    /*!< Enable the dynamically generated clock */
    ClkStop(sptr_disp->dynClkAddr);
    ClkStart(sptr_disp->dynClkAddr);

    /*!< Configure the vtc core with the display mode timing parameters */
    sgtc_timing.HActiveVideo = sptr_disp->vMode.width;						        /*!< Horizontal Active Video Size */
    sgtc_timing.HFrontPorch = sptr_disp->vMode.hps - sptr_disp->vMode.width;	    /*!< Horizontal Front Porch Size */
    sgtc_timing.HSyncWidth = sptr_disp->vMode.hpe - sptr_disp->vMode.hps;		    /*!< Horizontal Sync Width */
    sgtc_timing.HBackPorch = sptr_disp->vMode.hmax - sptr_disp->vMode.hpe + 1;      /*!< Horizontal Back Porch Size */
    sgtc_timing.HSyncPolarity = sptr_disp->vMode.hpol;	                            /*!< Horizontal Sync Polarity */
    sgtc_timing.VActiveVideo = sptr_disp->vMode.height;	                            /*!< Vertical Active Video Size */
    sgtc_timing.V0FrontPorch = sptr_disp->vMode.vps - sptr_disp->vMode.height;      /*!< Vertical Front Porch Size */
    sgtc_timing.V0SyncWidth = sptr_disp->vMode.vpe - sptr_disp->vMode.vps;	        /*!< Vertical Sync Width */
    sgtc_timing.V0BackPorch = sptr_disp->vMode.vmax - sptr_disp->vMode.vpe + 1;;	/*!< Horizontal Back Porch Size */
    sgtc_timing.V1FrontPorch = sptr_disp->vMode.vps - sptr_disp->vMode.height;	    /*!< Vertical Front Porch Size */
    sgtc_timing.V1SyncWidth = sptr_disp->vMode.vpe - sptr_disp->vMode.vps;	        /*!< Vertical Sync Width */
    sgtc_timing.V1BackPorch = sptr_disp->vMode.vmax - sptr_disp->vMode.vpe + 1;;	/*!< Horizontal Back Porch Size */
    sgtc_timing.VSyncPolarity = sptr_disp->vMode.vpol;	                            /*!< Vertical Sync Polarity */
    sgtc_timing.Interlaced = 0;		                                                /*!< Interlaced / Progressive video */

    /*!< Setup the VTC Source Select config structure. */
    /*!< 1=Generator registers are source */
    /*!< 0=Detector registers are source */
    memset((void *)&sgtc_srcSel, 0, sizeof(sgtc_srcSel));

    sgtc_srcSel.VBlankPolSrc = 1;
    sgtc_srcSel.VSyncPolSrc = 1;
    sgtc_srcSel.HBlankPolSrc = 1;
    sgtc_srcSel.HSyncPolSrc = 1;
    sgtc_srcSel.ActiveVideoPolSrc = 1;
    sgtc_srcSel.ActiveChromaPolSrc= 1;
    sgtc_srcSel.VChromaSrc = 1;
    sgtc_srcSel.VActiveSrc = 1;
    sgtc_srcSel.VBackPorchSrc = 1;
    sgtc_srcSel.VSyncSrc = 1;
    sgtc_srcSel.VFrontPorchSrc = 1;
    sgtc_srcSel.VTotalSrc = 1;
    sgtc_srcSel.HActiveSrc = 1;
    sgtc_srcSel.HBackPorchSrc = 1;
    sgtc_srcSel.HSyncSrc = 1;
    sgtc_srcSel.HFrontPorchSrc = 1;
    sgtc_srcSel.HTotalSrc = 1;

    XVtc_SelfTest(&(sptr_disp->vtc));

    XVtc_RegUpdateEnable(&(sptr_disp->vtc));
    XVtc_SetGeneratorTiming(&(sptr_disp->vtc), &sgtc_timing);
    XVtc_SetSource(&(sptr_disp->vtc), &sgtc_srcSel);

    /*!< Enable VTC core, releasing backpressure on VDMA */
    XVtc_EnableGenerator(&sptr_disp->vtc);

    /*!< Configure the VDMA to access a frame with the same dimensions as the current mode */
    sptr_disp->vdmaConfig.VertSizeInput = sptr_disp->vMode.height;
    sptr_disp->vdmaConfig.HoriSizeInput = (sptr_disp->vMode.width) * 4;
    sptr_disp->vdmaConfig.FixedFrameStoreAddr = sptr_disp->curFrame;
    /*!< Also reset the stride and address values, in case the user manually changed them */
    sptr_disp->vdmaConfig.Stride = sptr_disp->stride;

    for (i = 0; i < DISPLAY_NUM_FRAMES; i++)
        sptr_disp->vdmaConfig.FrameStoreStartAddr[i] = (kuint32_t)sptr_disp->framePtr[i];

    /*!<
     * Perform the VDMA driver calls required to start a transfer. Note that no data is actually
     * transferred until the disp_ctrl core signals the VDMA core by pulsing fsync.
     */
    Status = XAxiVdma_DmaConfig(sptr_disp->vdma, XAXIVDMA_READ, &(sptr_disp->vdmaConfig));
    if (Status)
        return Status;

    Status = XAxiVdma_DmaSetBufferAddr(sptr_disp->vdma, XAXIVDMA_READ, sptr_disp->vdmaConfig.FrameStoreStartAddr);
    if (Status)
        return Status;

    Status = XAxiVdma_DmaStart(sptr_disp->vdma, XAXIVDMA_READ);
    if (Status)
        return Status;

    Status = XAxiVdma_StartParking(sptr_disp->vdma, sptr_disp->curFrame, XAXIVDMA_READ);
    if (Status)
        return Status;

    sptr_disp->state = NR_XIL_DISPLAY_RUNNING;

    return ER_NORMAL;
}

/*!
 *	DisplayStop(DisplayCtrl *dispPtr)
 *
 *	Parameters:
 *		dispPtr - Pointer to the initialized DisplayCtrl struct
 *
 *	Return Value: int
 *		XST_SUCCESS if successful.
 *		XST_DMA_ERROR if an error was detected on the DMA channel. The
 *			Display is still successfully stopped, and the error is
 *			cleared so that subsequent DisplayStart calls will be
 *			successful. This typically indicates insufficient bandwidth
 *			on the AXI Memory-Map Interconnect (VDMA<->DDR)
 *
 *	Description:
 *		Halts output to the display
 */
kint32_t DisplayStop(DisplayCtrl *sptr_dispctrl)
{
    /*!< If already stopped, do nothing */
    if (sptr_dispctrl->state == NR_XIL_DISPLAY_STOPPED)
        return ER_NORMAL;

    /*!<
     * Disable the disp_ctrl core, and wait for the current frame to finish 
     * (the core cannot stop mid-frame)
     */
    XVtc_DisableGenerator(&sptr_dispctrl->vtc);

    /*!<  Stop the VDMA core */
    XAxiVdma_DmaStop(sptr_dispctrl->vdma, XAXIVDMA_READ);
    while (XAxiVdma_IsBusy(sptr_dispctrl->vdma, XAXIVDMA_READ));

    /*!< Update Struct state */
    sptr_dispctrl->state = NR_XIL_DISPLAY_STOPPED;

    /*! TODO: consider stopping the clock here, perhaps after a check to see if the VTC is finished */

    if (XAxiVdma_GetDmaChannelErrors(sptr_dispctrl->vdma, XAXIVDMA_READ))
    {
        XAxiVdma_ClearDmaChannelErrors(sptr_dispctrl->vdma, XAXIVDMA_READ, 0xFFFFFFFF);
        return -ER_FAULT;
    }

    return ER_NORMAL;
}

/* end of file */
