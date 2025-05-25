/*
 * imx6ul sdma controller
 *
 * File Name:   imx-dma.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.13
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/fwk_platdrv.h>
#include <platform/dma/fwk_dma.h>
#include <platform/clk/fwk_clk.h>
#include <kernel/spinlock.h>

#include <imx6/imx6ull_periph.h>
#include <imx6/imx6ull_sdma.h>

/*!< The defines */
struct imx_sdma_domain;
struct imx_sdma_channel;

enum __ERT_IMX_SDMA_SWITCH_MODE 
{
    NR_IMX_SDMA_MODE_STATIC = 0U,                           /*!< SDMA context switch mode static */
    NR_IMX_SDMA_MODE_DYNC_LOW_PWR,                          /*!< SDMA context switch mode dynamic with low power */
    NR_IMX_SDMA_MODE_DYNC_NO_LOOP,                          /*!< SDMA context switch mode dynamic with no loop */
    NR_IMX_SDMA_MODE_DYNC,                                  /*!< SDMA context switch mode dynamic */
};

struct imx_sdma_script_context 
{
    kuint32_t PC : 14;
    kuint32_t unused1 : 1;
    kuint32_t T : 1;
    kuint32_t RPC : 14;
    kuint32_t unused0 : 1;
    kuint32_t SF : 1;
    kuint32_t SPC : 14;
    kuint32_t unused2 : 1;
    kuint32_t DF : 1;
    kuint32_t EPC : 14;
    kuint32_t LM : 2;
    kuint32_t GeneralReg[8];                                /*!< 8 general regsiters used for SDMA RISC core */
    kuint32_t MDA;
    kuint32_t MSA;
    kuint32_t MS;
    kuint32_t MD;
    kuint32_t PDA;
    kuint32_t PSA;
    kuint32_t PS;
    kuint32_t PD;
    kuint32_t CA;
    kuint32_t CS;
    kuint32_t DDA;
    kuint32_t DSA;
    kuint32_t DS;
    kuint32_t DD;
    kuint32_t Scratch0;
    kuint32_t Scratch1;
    kuint32_t Scratch2;
    kuint32_t Scratch3;
    kuint32_t Scratch4;
    kuint32_t Scratch5;
    kuint32_t Scratch6;
    kuint32_t Scratch7;
};

/*!< sdma channel control descriptor structure */
struct imx_sdma_ctrl_desc 
{
    kuint32_t cur_bd;                                       /*!< Address of current buffer descriptor processed  */
    kuint32_t base_bd;                                      /*!< The start address of the buffer descriptor array */
    kuint32_t chan_desc;                                    /*!< Optional for transfer */
    kuint32_t status;                                       /*!< Channel status */
};

enum __ERT_IMX_SDMA_BD_STATUS 
{
    NR_IMX_SDMA_BD_DONE = mr_bit(0U),                       /*!< BD ownership, 0 means ARM core owns the BD, while 1 means SDMA owns BD. */
    NR_IMX_SDMA_BD_WRAP = mr_bit(1U),                       /*!< While this BD is last one, the next BD will be the first one */
    NR_IMX_SDMA_BD_CONTINUE = mr_bit(2U),                   /*!< Buffer is allowed to transfer/receive to/from multiple buffers */
    NR_IMX_SDMA_BD_INTR = mr_bit(3U),                       /*!< While this BD finished, send an interrupt. */
    NR_IMX_SDMA_BD_ERROR = mr_bit(4U),                      /*!< Error occured on buffer descriptor command. */
    NR_IMX_SDMA_BD_LAST = mr_bit(5U),                       /*!< This BD is the last BD in this array. It means the transfer ended after this buffer */
    NR_IMX_SDMA_BD_EXTEND = mr_bit(6U),                     /*!< Buffer descriptor extend status for SDMA scripts */
};

enum __ERT_IMX_SDMA_BD_COMMAND 
{
    NR_IMX_SDMA_BD_SETDM = 0U,                              /*!< Load SDMA data memory from ARM core memory buffer. */
    NR_IMX_SDMA_BD_GETDM,                                   /*!< Copy SDMA data memory to ARM core memory buffer. */
    NR_IMX_SDMA_BD_SETPM,                                   /*!< Load SDMA program memory from ARM core memory buffer. */
    NR_IMX_SDMA_BD_GETPM,                                   /*!< Copy SDMA program memory to ARM core memory buffer. */
    NR_IMX_SDMA_BD_SETCTX,                                  /*!< Load context for one channel into SDMA RAM from ARM platform memory buffer. */
    NR_IMX_SDMA_BD_GETCTX                                   /*!< Copy context for one channel from SDMA RAM to ARM platform memory buffer. */
};

/*!< sdma channel buffer descriptor (BD) structure */
struct imx_sdma_buffer_desc 
{
    kuint16_t count;                                        /*!< Bytes of the buffer length for this buffer descriptor. */
    kuint8_t status;                                        /*!< E,R,I,C,W,D status bits stored here */
    kuint8_t command;                                       /*!< command mostlky used for channel 0 */
    kuint32_t buffer_addr;                                  /*!< Buffer start address for this descriptor. */
    kuint32_t extend_addr;                                  /*!< External buffer start address, this is an optional for a transfer. */
};

#define IMX_SDMA_CHANNEL(x)                                 (x)

struct imx_sdma_desc 
{
    kuaddr_t self_phy;

    struct imx_sdma_buffer_desc *sptr_bds;
    kuaddr_t bd_phys;
    kuint32_t bd_count;

    kuint32_t direction;

    struct fwk_dma_transfer_desc sgtc_txdesc;
    struct list_head sgtc_link;
};

#define IMX_SDMA_TRANSFER_4BYTES                            (0U)
#define IMX_SDMA_TRANSFER_1BYTES                            (1U)
#define IMX_SDMA_TRANSFER_2BYTES                            (2U)

struct imx_sdma_channel 
{
    struct fwk_dma_chan sgtc_chan;
    struct imx_sdma_domain *sptr_chip;
    struct imx_sdma_desc *sptr_desc;                        /*!< Current Desc Running */
    struct list_head sgtc_pending;                          /*!< Pending Descs Will Be Run */
    struct list_head sgtc_completed;                        /*!< Completed Descs Will Be Free */

    struct spin_lock sgtc_lock;
    struct imx6_sdma_dt_data sgtc_dt;

    kuint32_t script_addr;
    kuint32_t event_source;
    kuint32_t priority;

    /*!< Transfer Info */
    kuint32_t direction;
    kuaddr_t src_addr;
    kuaddr_t dst_addr;
    kuint32_t src_addr_width;
    kuint32_t dst_addr_width;
    kusize_t watermark;

    struct imx_sdma_script_context *sptr_context;
};

#define mr_imx_sdma_to_chan(sptr_chan)  \
        mr_container_of(sptr_chan, struct imx_sdma_channel, sgtc_chan)

struct imx_sdma_domain 
{
    struct fwk_dma_device sgtc_madev;
    srt_imx_sdma_t *sptr_sdma;

    kuint32_t n_channels;
    struct imx_sdma_ctrl_desc *sptr_cds;                    /*!< Control Desc Array[IMX6UL_SDMA_MODULE_CHANNEL] */
    struct imx_sdma_buffer_desc *sptr_bd0;                  /*!< Buffer Desc of Channel 0 */

    struct imx_sdma_script_context *sptr_context;           /*!< Context Desc Array[IMX6UL_SDMA_MODULE_CHANNEL] */
    kuint32_t ref;
};

/*!< script address */
struct imx_sdma_script_type 
{
    kint32_t periph;
    kuaddr_t script_addr;
};

/*!< Peripheral type */
#define IMX_SDMA_PERIPH_NORMAL                              (0U)
#define IMX_SDMA_PERIPH_NORMAL_SP                           (1U)
#define IMX_SDMA_PERIPH_UART                                (2U)
#define IMX_SDMA_PERIPH_UART_SP                             (3U)
#define IMX_SDMA_PERPH_SPDIF                                (4U)

struct imx_sdma_platform_data 
{
    kuint32_t event_num;
    const struct imx_sdma_script_type (*sptr_scripts)[5U];
};

/*!< Driver Data Structure */
struct imx_sdma_drv_data 
{
    struct imx_sdma_domain sgtc_domain;
    const struct imx_sdma_platform_data *sptr_priv;

    struct fwk_clk *sptr_ipgclk;
    struct fwk_clk *sptr_ahbclk;
    void *base;
    kint32_t irq;
    kchar_t *sram_script_file;

    struct imx_sdma_channel sgtc_channels[IMX6UL_SDMA_MODULE_CHANNEL];
};

#define mr_imx_sdma_domain_to_data(sptr_domain)    \
        mr_container_of(sptr_domain, struct imx_sdma_drv_data, sgtc_domain)
#define mr_imx_sdma_desc_to_data(sptr_desc)    \
        mr_imx_sdma_domain_to_data((sptr_desc)->sptr_channel->sptr_chip)

#define mr_imx_sdma_madev_to_data(sptr_madev)   \
    ({  \
        const struct imx_sdma_domain *sptr_domain = mr_container_of(sptr_madev, struct imx_sdma_domain, sgtc_madev);    \
        mr_imx_sdma_domain_to_data(sptr_domain);    \
    })

/*!< struct imx_sdma_channel ---> index of struct imx_sdma_drv_data::sgtc_channels[] */
#define mr_imx_sdma_handle_to_channel(sptr_channel) \
    ({  \
        const struct imx_sdma_drv_data *sptr_data = mr_imx_sdma_domain_to_data((sptr_channel)->sptr_chip);   \
        (kusize_t)((sptr_channel) - &sptr_data->sgtc_channels[0]);   \
    })

/*!< The globals */
static const struct imx_sdma_script_type sgtc_imx_sdma_script_table[3][5] = 
{
    [DMA_MEM_TO_MEM] = {
        { .periph = -1,                         .script_addr = IMX6UL_SDMA_M2M_ADDR         },
    },

    [DMA_MEM_TO_DEVICE] = {
        { .periph = IMX_SDMA_PERIPH_NORMAL,     .script_addr = IMX6UL_SDMA_M2P_ADDR         },
        { .periph = IMX_SDMA_PERIPH_NORMAL_SP,  .script_addr = IMX6UL_SDMA_M2SHP_ADDR       },
        { .periph = IMX_SDMA_PERIPH_UART,       .script_addr = IMX6UL_SDMA_M2P_ADDR         },
        { .periph = IMX_SDMA_PERIPH_UART_SP,    .script_addr = IMX6UL_SDMA_M2SHP_ADDR       },
        { .periph = IMX_SDMA_PERPH_SPDIF,       .script_addr = IMX6UL_SDMA_M2SPDIF_ADDR     },
    },

    [DMA_DEVICE_TO_MEM] = {
        { .periph = IMX_SDMA_PERIPH_NORMAL,     .script_addr = IMX6UL_SDMA_P2M_ADDR         },
        { .periph = IMX_SDMA_PERIPH_NORMAL_SP,  .script_addr = IMX6UL_SDMA_M2SHP_ADDR       },
        { .periph = IMX_SDMA_PERIPH_UART,       .script_addr = IMX6UL_SDMA_UART2M_ADDR      },
        { .periph = IMX_SDMA_PERIPH_UART_SP,    .script_addr = IMX6UL_SDMA_UARTSH2M_ADDR    },
        { .periph = IMX_SDMA_PERPH_SPDIF,       .script_addr = IMX6UL_SDMA_SPDIF2M_ADDR     },
    },
};

static const struct imx_sdma_platform_data sgtc_imx_sdma_platform_data = 
{
    .event_num = IMX6UL_SDMA_EVENT_NUM,
    .sptr_scripts = sgtc_imx_sdma_script_table,
};

/*!< device id for device-tree */
static const struct fwk_of_device_id sgtc_imx_dma_driver_id[] = 
{
    { .compatible = "fsl,imx6ul-sdma", .data = &sgtc_imx_sdma_platform_data },
    {},
};

/*!< API functions */
/*!
 * @brief   Reset sdma
 * @param   sptr_sdma: base address of sdma
 * @retval  none
 * @note    none
 */
static void imx_sdma_run_channel0(srt_imx_sdma_t *sptr_sdma)
{
    imx_sdma_start_channel_software(sptr_sdma, 0);

    /*!< Waitting for "sptr_sdma->STOP_STAT & mr_bit(1)" reset */
//  while (imx_sdma_get_channel_stop_status(sptr_sdma, 0));

    /*!< clear interrupt status, and set context switch mode to dynamic */
    imx_sdma_clear_channel_int_status(sptr_sdma, 0);
    imx_sdma_set_context_switch_mode(sptr_sdma, NR_IMX_SDMA_MODE_DYNC);
}

/*!
 * @brief   Reset sdma
 * @param   sptr_sdma: base address of sdma
 * @retval  none
 * @note    none
 */
static void imx_sdma_reset_module(srt_imx_sdma_t *sptr_sdma, kuint32_t channel_num, kuint32_t event_num)
{
    /*!< Channel 0 */
    mr_resetl(&sptr_sdma->MC0PTR);

    /*!< set sptr_sdma->INTR to 0 */
    imx_sdma_clear_all_int_status(sptr_sdma);
    /*!< set sptr_sdma->STOP_STAT to 0 */
    imx_sdma_clear_all_stop_status(sptr_sdma);

    mr_resetl(&sptr_sdma->EVTOVR);
    mr_writel(IMX6UL_SDMA_DSPOVR_DO_U32(~0U), &sptr_sdma->DSPOVR);
    mr_resetl(&sptr_sdma->HOSTOVR);
    mr_resetl(&sptr_sdma->INTRMASK);

    /*!< set sptr_sdma->CHNENBL[0-47] to 0, disable all events */
    for (kuint32_t event = 0; event < event_num; event++)
        imx_sdma_clr_source_channel(sptr_sdma, event, 0xFFFFFFFFU);

    /*!< set sptr_sdma->EVTPEND to 0 */
    imx_sdma_clear_all_pend_status(sptr_sdma);

    /*!< set sptr_sdma->SDMA_CHNPRI[0-31] to 0, clear all channel's priority */
    for (kuint32_t chan = 0; chan < channel_num; chan++)
        imx_sdma_set_channel_priority(sptr_sdma, chan, 0);
}

/*!< --------------------------------------------------------------------------------------- */
/*!
 * @brief   Request bd0
 * @param   sptr_domain: base address of sdma
 * @retval  errno
 * @note    none
 */
static kint32_t imx_sdma_bd0_request(struct imx_sdma_domain *sptr_domain)
{
    kuaddr_t bd0_phys;

    if (!sptr_domain)
        return -ER_NODEV;

    sptr_domain->sptr_bd0 = fwk_dma_alloc_coherent(mr_nullptr, 
                                        sizeof(*sptr_domain->sptr_bd0), &bd0_phys, GFP_KERNEL | GFP_ZERO);
    if (!isValid(sptr_domain->sptr_bd0))
        return -ER_NOMEM;

    sptr_domain->sptr_cds[0U].cur_bd = (kuint32_t)bd0_phys;
    sptr_domain->sptr_cds[0U].base_bd = (kuint32_t)bd0_phys;

    return ER_NORMAL;
}

/*!
 * @brief   Free bd0
 * @param   sptr_domain: base address of sdma
 * @retval  none
 * @note    none
 */
static void imx_sdma_bd0_release(struct imx_sdma_domain *sptr_domain)
{
    kuaddr_t bd0_phys;

    if (sptr_domain && sptr_domain->sptr_bd0) 
    {
        bd0_phys = sptr_domain->sptr_cds[0U].base_bd;
        fwk_dma_free_coherent(mr_nullptr, 
                    sizeof(*sptr_domain->sptr_bd0), sptr_domain->sptr_bd0, &bd0_phys);
    }

    sptr_domain->sptr_cds[0U].cur_bd = 0;
    sptr_domain->sptr_cds[0U].base_bd = 0;
}

/*!
 * @brief   Initialize sdma
 * @param   sptr_domain: base address of sdma domain
 * @param   sptr_pdata: private board data
 * @retval  errno
 * @note    none
 */
static kint32_t imx_sdma_domain_initial(struct imx_sdma_domain *sptr_domain, const struct imx_sdma_platform_data *sptr_pdata)
{
    srt_imx_sdma_t *sptr_sdma;
    kuaddr_t cds_phys;
    kusize_t alloc_size;
    kuint32_t reg_value;

    if (!sptr_domain ||
        !sptr_domain->sptr_sdma ||
        !sptr_domain->n_channels)
        return -ER_EMPTY;

    /*!< Ctrl Descs Allocate */
    alloc_size = sptr_domain->n_channels * (sizeof(*sptr_domain->sptr_cds) + sizeof(*sptr_domain->sptr_context));
    sptr_domain->sptr_cds = fwk_dma_alloc_coherent(mr_nullptr, 
                                        alloc_size, &cds_phys, GFP_KERNEL | GFP_ZERO);
    if (!isValid(sptr_domain->sptr_cds))
        return -ER_NOMEM;

    sptr_domain->sptr_context = (void *)sptr_domain->sptr_cds + sptr_domain->n_channels * sizeof(*sptr_domain->sptr_cds);
    sptr_sdma = sptr_domain->sptr_sdma;

    /*!< Reset all SDMA registers */
    imx_sdma_reset_module(sptr_sdma, sptr_domain->n_channels, sptr_pdata->event_num);

    /*!< 0: channel 0 */
    imx_sdma_set_channel_priority(sptr_sdma, 0, 0x7U);

    /*!< Set channel 0 ownership */
    mr_writel(0U, &sptr_sdma->HOSTOVR);
    mr_writel(1U, &sptr_sdma->EVTOVR);

    /*!< Configure Ctrl Desc */
    if (imx_sdma_bd0_request(sptr_domain)) 
    {
        fwk_dma_free_coherent(mr_nullptr, alloc_size, sptr_domain->sptr_cds, &cds_phys);
        return -ER_FAILD;
    }

    /*!< 
     * Core clock ratio: 0U, half of ARM platform;
     * Real Time Debug Pin: 0U
     */
    reg_value = mr_readl(&sptr_sdma->CONFIG);
    mr_clrbitl(IMX6UL_SDMA_CONFIG_ACR_MASK | IMX6UL_SDMA_CONFIG_RTDOBS_MASK, &reg_value);
    mr_clrbitl(IMX6UL_SDMA_CONFIG_CSM_MASK, &reg_value);
    mr_writel(reg_value, &sptr_sdma->CONFIG);

    /*!< Enable Software Reset Clear Lock */
    reg_value = mr_readl(&sptr_sdma->SDMA_LOCK);
    mr_clrbitl(IMX6UL_SDMA_LOCK_SRESET_LOCK_CLR_MASK, &reg_value);
    mr_setbitl(IMX6UL_SDMA_LOCK_SRESET_LOCK_CLR(1U), &reg_value);
    mr_writel(reg_value, &sptr_sdma->SDMA_LOCK);

    /*!< Set the context size to 32 bytes */
    mr_writel(0x4050U, &sptr_sdma->CHN0ADDR);

    /*!< Set channle 0 CCB address */
    mr_writel(cds_phys, &sptr_sdma->MC0PTR);

    return ER_NORMAL;
}

/*!
 * @brief   Release sdma
 * @param   sptr_domain: base address of sdma
 * @retval  none
 * @note    none
 */
static void imx_sdma_domain_deinit(struct imx_sdma_domain *sptr_domain)
{
    srt_imx_sdma_t *sptr_sdma = sptr_domain->sptr_sdma;

    /*!< Clear channle 0 CCB address */
    mr_writel(0U, &sptr_sdma->MC0PTR);

    /*!< Free sptr_domain->sptr_bd0 */
    imx_sdma_bd0_release(sptr_domain);

    /*!< Free sptr_domain->sptr_cds and sptr_domain->sptr_context */
    kfree(sptr_domain->sptr_cds);
    sptr_domain->sptr_cds = mr_nullptr;
    sptr_domain->sptr_context = mr_nullptr;
}

/*!
 * @brief   Get sdma script address
 * @param   direction: memory to memory, memory to peripheral, or peripheral to memory
 * @param   periph: normal, uart, or spdif
 * @retval  script address
 * @note    none
 */
static void imx_sdma_set_script_address(struct imx_sdma_channel *sptr_channel, kint32_t direction, kint32_t periph)
{
    struct imx_sdma_drv_data *sptr_data;
    const struct imx_sdma_script_type *sptr_tab;
    
    sptr_data = mr_imx_sdma_domain_to_data(sptr_channel->sptr_chip);
    sptr_tab = sptr_data->sptr_priv->sptr_scripts[direction];

    sptr_channel->direction = direction;
    sptr_channel->script_addr = ((direction == DMA_MEM_TO_MEM) ? 
                        sptr_tab[0].script_addr : sptr_tab[periph].script_addr);
}

/*!
 * @brief   Get slave device class
 * @param   peripheral_type: device type
 * @retval  class (such as "NORMAL", "UART" .etc)
 * @note    none
 */
static kint32_t imx_sdma_get_map_periph(kuint32_t peripheral_type)
{
    kint32_t periph = 0;

    switch (peripheral_type) 
    {
        case NR_IMX_DMATYPE_MEMORY:
            periph = -1;
            break;

        case NR_IMX_DMATYPE_UART:
            periph = IMX_SDMA_PERIPH_UART;
            break;

        case NR_IMX_DMATYPE_UART_SP:
            periph = IMX_SDMA_PERIPH_UART_SP;
            break;

        case NR_IMX_DMATYPE_CSPI:
        case NR_IMX_DMATYPE_EXT:
        case NR_IMX_DMATYPE_SSI:
        case NR_IMX_DMATYPE_SAI:
            periph = IMX_SDMA_PERIPH_NORMAL;
            break;

        case NR_IMX_DMATYPE_SSI_SP:
        case NR_IMX_DMATYPE_MMC:
        case NR_IMX_DMATYPE_SDHC:
        case NR_IMX_DMATYPE_CSPI_SP:
        case NR_IMX_DMATYPE_ESAI:
        case NR_IMX_DMATYPE_MSHC_SP:
            periph = IMX_SDMA_PERIPH_NORMAL_SP;
            break;

        case NR_IMX_DMATYPE_SPDIF:
            periph = IMX_SDMA_PERPH_SPDIF;
            break;

        case NR_IMX_DMATYPE_DSP:
        case NR_IMX_DMATYPE_FIRI:
        case NR_IMX_DMATYPE_ATA:
        case NR_IMX_DMATYPE_SSI_DUAL:
        case NR_IMX_DMATYPE_ASRC:
        case NR_IMX_DMATYPE_ASRC_SP:
        case NR_IMX_DMATYPE_MSHC:
        case NR_IMX_DMATYPE_CCM:
        case NR_IMX_DMATYPE_IPU_MEMORY:
        case NR_IMX_DMATYPE_HDMI:
            break;

        default:
            break;
    }

    return periph;
}

/*!
 * @brief   Load context to bd0
 * @param   sptr_channel: sdma channel
 * @retval  none
 * @note    none
 */
static void imx_sdma_load_context(struct imx_sdma_channel *sptr_channel)
{
    struct imx_sdma_buffer_desc *sptr_bd0;
    struct imx_sdma_script_context *sptr_context;
    kuint32_t channel;

    if (!sptr_channel || 
        !sptr_channel->sptr_context)
        return;

    channel = mr_imx_sdma_handle_to_channel(sptr_channel);
    sptr_context = sptr_channel->sptr_context;
    sptr_bd0 = sptr_channel->sptr_chip->sptr_bd0;

    memset(sptr_context, 0, sizeof(*sptr_context));
    sptr_context->PC = sptr_channel->script_addr;

    /*!< Set the request source into context */
    if (sptr_channel->event_source >= 32)
        sptr_context->GeneralReg[0] = mr_bit(sptr_channel->event_source - 32);
    else
        sptr_context->GeneralReg[1] = mr_bit(sptr_channel->event_source);

    /*!< Set source address and dest address for p2p, m2p and p2m */
    if (sptr_channel->direction == DMA_MEM_TO_DEVICE) 
    {
        sptr_context->GeneralReg[2] = (kuint32_t)sptr_channel->dst_addr;
        sptr_context->GeneralReg[6] = (kuint32_t)sptr_channel->dst_addr;
    }
    else 
    {
        sptr_context->GeneralReg[2] = (kuint32_t)sptr_channel->src_addr;
        sptr_context->GeneralReg[6] = (kuint32_t)sptr_channel->src_addr;
    }

    /*!< Set watermark for p2p, m2p and p2m into context */
    sptr_context->GeneralReg[7] = sptr_channel->watermark;

    sptr_bd0->command = NR_IMX_SDMA_BD_SETDM;
    sptr_bd0->status = NR_IMX_SDMA_BD_DONE | NR_IMX_SDMA_BD_WRAP | NR_IMX_SDMA_BD_INTR;
    sptr_bd0->count = sizeof(*sptr_context) / 4U;
    sptr_bd0->buffer_addr = (kuint32_t)sptr_context;
    sptr_bd0->extend_addr = 2048 + (sizeof(*sptr_context) / 4) * channel;

    imx_sdma_run_channel0(sptr_channel->sptr_chip->sptr_sdma);
}

/*!
 * @brief   Start transfer
 * @param   sptr_channel: sdma channel
 * @retval  errno
 * @note    none
 */
static kint32_t imx_sdma_start_transfer(struct imx_sdma_channel *sptr_channel)
{
    srt_imx_sdma_t *sptr_sdma;
    kuint32_t channel;
    kuint32_t reg_value;

    if (!sptr_channel)
        return -ER_NODEV;

    sptr_sdma = sptr_channel->sptr_chip->sptr_sdma;
    channel = mr_imx_sdma_handle_to_channel(sptr_channel);

    reg_value = imx_sdma_get_channel_priority(sptr_sdma, channel);

    if (!sptr_channel->priority) 
    {
        /*!< Set the channel priority */
        sptr_channel->priority = reg_value;
    }
    else if (!reg_value) 
    {
        /*!< Set priority if regsiter bit is 0*/
        imx_sdma_set_channel_priority(sptr_sdma, channel, sptr_channel->priority);
    }
    
    if (sptr_channel->event_source)
        imx_sdma_start_channel_event(sptr_sdma, channel);
    else
        imx_sdma_start_channel_software(sptr_sdma, channel);

    return ER_NORMAL;
}

/*!
 * @brief   Stop transfer
 * @param   sptr_channel: sdma channel
 * @retval  none
 * @note    none
 */
static void imx_sdma_stop_transfer(struct imx_sdma_channel *sptr_channel)
{
    kuint32_t channel;

    if (!sptr_channel)
        return;

    channel = mr_imx_sdma_handle_to_channel(sptr_channel);
    imx_sdma_stop_channel(sptr_channel->sptr_chip->sptr_sdma, channel);
}

/*!
 * @brief   Quit transfer
 * @param   sptr_channel: sdma channel
 * @retval  none
 * @note    none
 */
static void imx_sdma_abort_transfer(struct imx_sdma_channel *sptr_channel)
{
    srt_imx_sdma_t *sptr_sdma;
    kuint32_t channel;

    imx_sdma_stop_transfer(sptr_channel);

    sptr_sdma = sptr_channel->sptr_chip->sptr_sdma;
    channel = mr_imx_sdma_handle_to_channel(sptr_channel);

    imx_sdma_clr_source_channel(sptr_sdma, sptr_channel->event_source, mr_bit(channel));

    /*!< Clear the channel priority */
    imx_sdma_set_channel_priority(sptr_sdma, channel, 0U);
}

/*!< --------------------------------------------------------------------------------------- */
/*!
 * @brief   Get sdma base address
 * @param   sptr_desc: sdma transfer descripter
 * @retval  handle
 * @note    none
 */
static srt_imx_sdma_t *imx_sdma_handle_get(struct imx_sdma_desc *sptr_desc)
{
    struct fwk_dma_chan *sptr_chan = sptr_desc->sgtc_txdesc.sptr_chan;
    struct imx_sdma_channel *sptr_channel = mr_imx_sdma_to_chan(sptr_chan);

    return sptr_channel->sptr_chip->sptr_sdma;
}

/*!
 * @brief   Allocate and initialize transfer descripter
 * @param   sptr_channel: which channel the desc will use
 * @param   bd_count: how many BDs
 * @retval  desc
 * @note    none
 */
static struct imx_sdma_desc *imx_sdma_desc_alloc(struct imx_sdma_channel *sptr_channel, kuint32_t bd_count)
{
    struct imx_sdma_desc *sptr_desc;
    struct fwk_dma_transfer_desc *sptr_txdesc;
    kuaddr_t desc_phys;

    sptr_desc = fwk_dma_alloc_coherent(mr_nullptr, 
                            sizeof(*sptr_desc), &desc_phys, GFP_KERNEL | GFP_ZERO);
    if (!isValid(sptr_desc))
        return ERR_PTR(-ER_NOMEM);

    sptr_desc->self_phy = desc_phys;
    sptr_desc->bd_count = bd_count;
    init_list_head(&sptr_desc->sgtc_link);

    sptr_txdesc = &sptr_desc->sgtc_txdesc;
    sptr_txdesc->sptr_chan = &sptr_channel->sgtc_chan;
    sptr_txdesc->phys = (kuaddr_t)fwk_virt_to_phys(sptr_txdesc);

    return sptr_desc;
}

/*!
 * @brief   Allocate BDs for transfer descripter
 * @param   sptr_desc: transfer descripter
 * @retval  errno
 * @note    none
 */
static kint32_t imx_sdma_desc_alloc_bds(struct imx_sdma_desc *sptr_desc)
{
    kuaddr_t bd_phys;

    if (!sptr_desc || !sptr_desc->bd_count)
        return -ER_INVALID;

    if (sptr_desc->sptr_bds || sptr_desc->bd_phys)
        return -ER_EXISTED;

    sptr_desc->sptr_bds = fwk_dma_alloc_coherent(mr_nullptr, 
                                sptr_desc->bd_count * sizeof(struct imx_sdma_buffer_desc), 
                                &bd_phys, GFP_KERNEL | GFP_ZERO);
    if (!isValid(sptr_desc->sptr_bds))
        return PTR_ERR(sptr_desc->sptr_bds);

    sptr_desc->bd_phys = bd_phys;
    return ER_NORMAL;
}

/*!
 * @brief   Release BDs in transfer descripter
 * @param   sptr_desc: transfer descripter
 * @retval  none
 * @note    none
 */
static void imx_sdma_desc_free_bds(struct imx_sdma_desc *sptr_desc)
{
    if (!sptr_desc || !sptr_desc->bd_phys)
        return;

    fwk_dma_free_coherent(mr_nullptr, 
            sptr_desc->bd_count * sizeof(struct imx_sdma_buffer_desc), 
            sptr_desc->sptr_bds, &sptr_desc->bd_phys);

    sptr_desc->bd_phys = 0;
    sptr_desc->sptr_bds = mr_nullptr;
}

/*!
 * @brief   Release transfer descripter
 * @param   sptr_desc: transfer descripter
 * @retval  none
 * @note    none
 */
static void imx_sdma_desc_destroy(struct imx_sdma_desc *sptr_desc)
{
    if (sptr_desc) {
        if (sptr_desc->bd_phys)
            imx_sdma_desc_free_bds(sptr_desc);

        fwk_dma_free_coherent(mr_nullptr, sizeof(*sptr_desc), sptr_desc, &sptr_desc->self_phy);
    }
}

/*!
 * @brief   Load desc to channel
 * @param   sptr_desc: transfer descripter
 * @retval  errno
 * @note    none
 */
static kint32_t imx_sdma_desc_load(struct imx_sdma_desc *sptr_desc)
{
    struct imx_sdma_channel *sptr_channel;
    struct imx_sdma_domain *sptr_domain;
    kuint32_t channel;

    if (!sptr_desc ||
        !sptr_desc->bd_phys)
        return -ER_NODEV;

    /*!< Does not add to sptr_channel->pending */
    if (mr_list_head_empty(&sptr_desc->sgtc_link))
        return -ER_EMPTY;

    sptr_channel = mr_imx_sdma_to_chan(sptr_desc->sgtc_txdesc.sptr_chan);
    channel = mr_imx_sdma_handle_to_channel(sptr_channel);
    sptr_domain  = sptr_channel->sptr_chip;

    /*!< take desc to excute */
    sptr_channel->sptr_desc = sptr_desc;
    sptr_channel->direction = sptr_desc->direction;
    list_head_del(&sptr_desc->sgtc_link);

    sptr_domain->sptr_cds[channel].cur_bd = (kuint32_t)sptr_desc->bd_phys;
    sptr_domain->sptr_cds[channel].base_bd = (kuint32_t)sptr_desc->bd_phys;

    return ER_NORMAL;
}

/*!
 * @brief   Pre-configure channel
 * @param   sptr_desc: transfer descripter
 * @retval  none
 * @note    none
 */
static void imx_sdma_desc_prepare(struct imx_sdma_desc *sptr_desc)
{
    struct imx_sdma_channel *sptr_channel;
    srt_imx_sdma_t *sptr_sdma;
    kuint32_t channel;

    sptr_channel = mr_imx_sdma_to_chan(sptr_desc->sgtc_txdesc.sptr_chan);

    spin_lock_irqsave(&sptr_channel->sgtc_lock);
    if (imx_sdma_desc_load(sptr_desc)) 
    {
        spin_unlock_irqrestore(&sptr_channel->sgtc_lock);
        return;
    }

    spin_unlock_irqrestore(&sptr_channel->sgtc_lock);

    sptr_sdma = imx_sdma_handle_get(sptr_desc);
    channel = mr_imx_sdma_handle_to_channel(sptr_channel);

    /*!< DO register shall always set */
    mr_setbitl(mr_bit(channel), &sptr_sdma->DSPOVR);

    if (sptr_channel->direction == DMA_MEM_TO_MEM) 
    {
        /*!< Configure EO bit */
        mr_setbitl(mr_bit(channel), &sptr_sdma->EVTOVR);
        mr_clrbitl(mr_bit(channel), &sptr_sdma->HOSTOVR);
    }
    else 
    {
        /*!< Configure HO bits */
        mr_clrbitl(mr_bit(channel), &sptr_sdma->EVTOVR);
        mr_setbitl(mr_bit(channel), &sptr_sdma->HOSTOVR);
    }
}

/*!
 * @brief   Start transfer
 * @param   sptr_desc: transfer descripter
 * @retval  none
 * @note    none
 */
static void imx_sdma_desc_start(struct imx_sdma_desc *sptr_desc)
{
    struct imx_sdma_channel *sptr_channel;

    sptr_channel = mr_imx_sdma_to_chan(sptr_desc->sgtc_txdesc.sptr_chan);

    imx_sdma_desc_prepare(sptr_desc);
    imx_sdma_start_transfer(sptr_channel);
}

/*!< --------------------------------------------------------------------------------------- */
/*!
 * @brief   Upper interrupt function
 * @param   args: struct imx_sdma_drv_data
 * @retval  irq status
 * @note    none
 */
static irq_return_t imx_sdma_isr(kint32_t irq, void *args)
{
    struct imx_sdma_drv_data *sptr_data;
    srt_imx_sdma_t *sptr_sdma;
    kuint32_t reg_value, channel = 1U;
    kuint32_t chan_mask;

    sptr_data = (struct imx_sdma_drv_data *)args;
    sptr_sdma = sptr_data->sgtc_domain.sptr_sdma;

    /*!< Get Interrupt Status */
    reg_value = mr_readl(&sptr_sdma->INTR);
    /*!< Clear Interrupt Status */
    mr_writel(reg_value, &sptr_sdma->INTR);

    /*!< Ignore Channel 0 */
    chan_mask = reg_value >> 1;

    while (chan_mask) 
    {
        if (chan_mask & 1U) 
        {
            struct imx_sdma_channel *sptr_channel;
            struct imx_sdma_desc *sptr_desc;

            sptr_channel = &sptr_data->sgtc_channels[channel];
            if (!sptr_channel->sptr_desc)
                continue;

            sptr_desc = sptr_channel->sptr_desc;
            spin_lock(&sptr_channel->sgtc_lock);

            /*!< Current desc is completed, move to "completed list" */
            list_head_add_tail(&sptr_channel->sgtc_completed, &sptr_desc->sgtc_link);

            if (mr_list_head_empty(&sptr_channel->sgtc_pending)) 
            {
                /*!< No desc needs to load */
                sptr_channel->sptr_desc = mr_nullptr;
                imx_sdma_stop_transfer(sptr_channel);
                spin_unlock(&sptr_channel->sgtc_lock);
            }
            else
            {
                /*!< take next desc, and set sptr_channel->sptr_desc = sptr_desc */
                sptr_desc = mr_list_first_entry(&sptr_channel->sgtc_pending, struct imx_sdma_desc, sgtc_link);
                spin_unlock(&sptr_channel->sgtc_lock);
                imx_sdma_desc_start(sptr_desc);
            }
        }

        channel++;
        chan_mask >>= 1;
    }

    /*!< At least one channel that is not channel 0 */
    if (reg_value != 1)
        return NR_IRQ_WAKE_THREAD;

    return NR_IRQ_HANDLED;
}

/*!
 * @brief   Bottom interrupt function
 * @param   args: struct imx_sdma_drv_data
 * @retval  irq status
 * @note    none
 */
static irq_return_t imx_sdma_thread_isr(kint32_t irq, void *args)
{
    struct imx_sdma_drv_data *sptr_data;
    struct imx_sdma_channel *sptr_channel;
    struct imx_sdma_desc *sptr_desc, *sprt_temp;

    sptr_data = (struct imx_sdma_drv_data *)args;
    for (kint32_t channel = 0; channel < sptr_data->sgtc_domain.n_channels; channel++) 
    {
        DECLARE_LIST_HEAD(sgtc_copy);

        sptr_channel = &sptr_data->sgtc_channels[channel];

        /*!< copy sgtc_completed to sgtc_copy, and initialize sgtc_completed */
        spin_lock_irqsave(&sptr_channel->sgtc_lock);
        list_head_splice_init(&sptr_channel->sgtc_completed, &sgtc_copy);
        spin_unlock_irqrestore(&sptr_channel->sgtc_lock);

        if (mr_list_head_empty(&sgtc_copy))
            continue;

        /*!< detach and destroy desc */
        foreach_list_next_entry_safe(sptr_desc, sprt_temp, &sgtc_copy, sgtc_link) 
        {
            if (sptr_desc->sgtc_txdesc.transfer_callback)
                sptr_desc->sgtc_txdesc.transfer_callback(sptr_desc->sgtc_txdesc.callback_param);
            
            list_head_del(&sptr_desc->sgtc_link);
            imx_sdma_desc_destroy(sptr_desc);
        }
    }

    return NR_IRQ_NONE;
}

/*!
 * @brief   Add sptr_txdesc to pending list
 * @param   sptr_txdesc: transfer desc
 * @retval  errno
 * @note    none
 */
static kint32_t imx_sdma_desc_submit(struct fwk_dma_transfer_desc *sptr_txdesc)
{
    struct imx_sdma_channel *sptr_channel;
    struct imx_sdma_desc *sptr_desc;

    if (!sptr_txdesc ||
        !sptr_txdesc->sptr_chan)
        return -ER_NODEV;

    sptr_desc = mr_container_of(sptr_txdesc, struct imx_sdma_desc, sgtc_txdesc);
    sptr_channel = mr_imx_sdma_to_chan(sptr_txdesc->sptr_chan);

    /*!< Add to pending */
    spin_lock_irqsave(&sptr_channel->sgtc_lock);
    list_head_add_tail(&sptr_channel->sgtc_pending, &sptr_desc->sgtc_link);
    spin_unlock_irqrestore(&sptr_channel->sgtc_lock);

    return ER_NORMAL;
}

/*!
 * @brief   Get filter_param and save
 * @param   sptr_chan: current channel
 * @param   filter_param: private device-tree information
 * @retval  default true
 * @note    none
 */
static kbool_t imx_sdma_filter(struct fwk_dma_chan *sptr_chan, void *filter_param)
{
    struct imx6_sdma_dt_data *sptr_dt;
    struct imx_sdma_channel *sptr_channel;
    kuint32_t channel;

    sptr_dt = (struct imx6_sdma_dt_data *)filter_param;
    sptr_channel = mr_imx_sdma_to_chan(sptr_chan);
    channel = mr_imx_sdma_handle_to_channel(sptr_channel);

    /*!< Channel0 can not be requested */
    if (0 == channel)
        return false;

    memcpy(&sptr_channel->sgtc_dt, sptr_dt, sizeof(sptr_channel->sgtc_dt));
    sptr_chan->private_data = &sptr_channel->sgtc_dt;

    return true;
}

/*!
 * @brief   Parse device-tree for imx
 * @param   sptr_args: device-tree property
 * @param   sptr_mdev: sdma controller
 * @retval  idle channel
 * @note    none
 */
static struct fwk_dma_chan *imx_of_dma_xlate(struct fwk_of_phandle_args *sptr_args, struct fwk_dma_device *sptr_mdev)
{
    struct imx6_sdma_dt_data sgtc_dt = {};

    if (sptr_args->args_count != 3)
        return ERR_PTR(-ER_LACK);

    sgtc_dt.dma_request = sptr_args->args[0];
    sgtc_dt.peripheral_type = sptr_args->args[1];
    sgtc_dt.priority = sptr_args->args[2];

    return fwk_dma_find_candidate(sptr_mdev, imx_sdma_filter, &sgtc_dt);
}

/*!
 * @brief   Sdma channel enable (open clock and set event source)
 * @param   sptr_chan: channel will be opened
 * @retval  errno
 * @note    it will be called by platform API "fwk_dma_chan_get";
 *          sptr_chan->client_count++
 */
static kint32_t imx_alloc_chan_resources(struct fwk_dma_chan *sptr_chan)
{
    struct imx_sdma_drv_data *sptr_data;
    struct imx_sdma_channel *sptr_channel;
    struct imx6_sdma_dt_data *sptr_dt;
    srt_imx_sdma_t *sptr_sdma;
    kuint32_t channel;

    sptr_channel = mr_imx_sdma_to_chan(sptr_chan);
    sptr_data = mr_imx_sdma_domain_to_data(sptr_channel->sptr_chip);
    sptr_sdma = sptr_channel->sptr_chip->sptr_sdma;
    channel = mr_imx_sdma_handle_to_channel(sptr_channel);

    sptr_dt = (struct imx6_sdma_dt_data *)sptr_chan->private_data;
    sptr_channel->priority = sptr_dt->priority;
    sptr_channel->event_source = sptr_dt->dma_request;

    fwk_clk_enable(sptr_data->sptr_ahbclk);
    fwk_clk_enable(sptr_data->sptr_ipgclk);

    imx_sdma_set_source_channel(sptr_sdma, sptr_channel->event_source, mr_bit(channel));
    if (sptr_channel->priority)
        imx_sdma_set_channel_priority(sptr_sdma, channel, sptr_channel->priority);

    fwk_enable_irq(sptr_data->irq);
    sptr_data->sgtc_domain.ref++;

    return ER_NORMAL;
}

/*!
 * @brief   Sdma channel disable (close clock)
 * @param   sptr_chan: channel will be opened
 * @retval  errno
 * @note    it will be called by platform API "fwk_dma_chan_put";
 *          if another devices is using the channel (sptr_chan->client_count != 0), it will be not called
 */
static kint32_t imx_free_chan_resources(struct fwk_dma_chan *sptr_chan)
{
    struct imx_sdma_drv_data *sptr_data;
    struct imx_sdma_channel *sptr_channel;

    sptr_channel = mr_imx_sdma_to_chan(sptr_chan);
    sptr_data = mr_imx_sdma_domain_to_data(sptr_channel->sptr_chip);

    /*!< All channels are idle */
    if (!(sptr_data->sgtc_domain.ref--)) 
    {
        fwk_disable_irq(sptr_data->irq);
        imx_sdma_abort_transfer(sptr_channel);

        fwk_clk_disable(sptr_data->sptr_ahbclk);
        fwk_clk_disable(sptr_data->sptr_ipgclk);
    }

    return ER_NORMAL;
}

/*!
 * @brief   Configure sptr_channel
 * @param   sptr_chan: channel structure
 * @param   sptr_config: configs
 * @retval  errno
 * @note    none
 */
static kint32_t imx_sdma_config(struct fwk_dma_chan *sptr_chan, struct fwk_dma_slave_config *sptr_config)
{
    struct imx_sdma_channel *sptr_channel;
    struct imx6_sdma_dt_data *sptr_dt;
    kuint32_t periph;

    sptr_channel = mr_imx_sdma_to_chan(sptr_chan);

    switch (sptr_config->src_addr_width) 
    {
        case NR_DMA_TRX_WIDTH_1BYTE:
            sptr_channel->src_addr_width = IMX_SDMA_TRANSFER_1BYTES;
            break;
        case NR_DMA_TRX_WIDTH_2BYTE:
            sptr_channel->src_addr_width = IMX_SDMA_TRANSFER_2BYTES;
            break;
        default:
            sptr_channel->src_addr_width = IMX_SDMA_TRANSFER_4BYTES;
            break;
    }

    switch (sptr_config->dst_addr_width) 
    {
        case NR_DMA_TRX_WIDTH_1BYTE:
            sptr_channel->dst_addr_width = IMX_SDMA_TRANSFER_1BYTES;
            break;
        case NR_DMA_TRX_WIDTH_2BYTE:
            sptr_channel->dst_addr_width = IMX_SDMA_TRANSFER_2BYTES;
            break;
        default:
            sptr_channel->dst_addr_width = IMX_SDMA_TRANSFER_4BYTES;
            break;
    }

    sptr_channel->src_addr = sptr_config->src_addr;
    sptr_channel->dst_addr = sptr_config->dst_addr;

    /*!< minimum bytes is 1 (kuint8_t) */
    sptr_channel->watermark = sizeof(kuint8_t);

    sptr_dt = (struct imx6_sdma_dt_data *)sptr_chan->private_data;
    periph = imx_sdma_get_map_periph(sptr_dt->peripheral_type);

    /*!< Set Direction and Script Address */
    imx_sdma_set_script_address(sptr_channel, sptr_config->direction, periph);
    /*!< Load Context */
    imx_sdma_load_context(sptr_channel);

    return ER_NORMAL;
}

/*!
 * @brief   Prepare transfer
 * @param   sptr_chan: SDMA channel
 * @param   sptr_src: Source memory address (for "DMA_DEVICE_TO_MEM", it can be set to NULL)
 * @param   src_count: The number of source memory blocks (for "DMA_DEVICE_TO_MEM", it can be set to 0)
 * @param   sptr_dst: Destination memory address (for "DMA_MEM_TO_DEVICE", it can be set to NULL)
 * @param   dst_count: The number of destination memory blocks (for "DMA_MEM_TO_DEVICE", it can be set to 0)
 * @param   direction: Transfer direction, it must be DMA_MEM_TO_MEM, DMA_MEM_TO_DEVICE, or DMA_DEVICE_TO_MEM
 * @param   flag: Transfer flag
 * @param   context: Private context data for dma controller
 * @retval  Allocated transfer_desc
 */
struct fwk_dma_transfer_desc *imx_sdma_transfer_prepare(struct fwk_dma_chan *sptr_chan, 
                                        struct fwk_dma_block_data *sptr_src, kusize_t src_count,
                                        struct fwk_dma_block_data *sptr_dst, kusize_t dst_count,
                                        kuint32_t direction, kuint32_t flags, void *context)
{
    struct imx_sdma_channel *sptr_channel;
    struct imx_sdma_desc *sptr_desc = mr_nullptr;

    if (!sptr_chan)
        return ERR_PTR(-ER_INVALID);

    sptr_channel = mr_imx_sdma_to_chan(sptr_chan);

    if (direction == DMA_MEM_TO_MEM) 
    {
        if (!sptr_src || !src_count ||
            !sptr_dst || !dst_count)
            return ERR_PTR(-ER_EMPTY);

        /*!< Reserved */
    }
    else
    {
        struct fwk_dma_block_data *sptr_bdata;
        struct imx_sdma_buffer_desc *sptr_bd;
        kuint32_t data_count;
        kuint32_t bd_index = 0;

        /*!< Get Memory (Not prepheral) */
        sptr_bdata = (direction == DMA_MEM_TO_DEVICE) ? sptr_src : sptr_dst;
        data_count = (direction == DMA_MEM_TO_DEVICE) ? src_count : dst_count;

        if (!sptr_bdata || !data_count)
            return ERR_PTR(-ER_EMPTY);

        sptr_desc = imx_sdma_desc_alloc(sptr_channel, data_count);
        if (!isValid(sptr_desc))
            return ERR_PTR(-ER_NOMEM);

        if (imx_sdma_desc_alloc_bds(sptr_desc)) 
        {
            imx_sdma_desc_destroy(sptr_desc);
            return mr_nullptr;
        }

        /*!< Configure every BD */
        for (; sptr_bdata && (bd_index < sptr_desc->bd_count); 
               sptr_bdata = sptr_bdata->sptr_next, bd_index++) 
        {
            sptr_bd = &sptr_desc->sptr_bds[bd_index];
            sptr_bd->buffer_addr = sptr_bdata->dma_address + sptr_bdata->offset;
            sptr_bd->extend_addr = 0U;
            sptr_bd->count = sptr_bdata->length;
            sptr_bd->command = (direction == DMA_MEM_TO_DEVICE) ? sptr_channel->dst_addr_width : sptr_channel->src_addr_width;
            sptr_bd->status = NR_IMX_SDMA_BD_DONE;

            /*!< Last BD */
            if ((bd_index + 1) == sptr_desc->bd_count)
                sptr_bd->status |= (NR_IMX_SDMA_BD_LAST | NR_IMX_SDMA_BD_INTR); 
            else
                sptr_bd->status |= NR_IMX_SDMA_BD_CONTINUE;
        }

        sptr_desc->direction = direction;
        sptr_desc->sgtc_txdesc.flags = flags;
        sptr_desc->sgtc_txdesc.submit = imx_sdma_desc_submit;

        return &sptr_desc->sgtc_txdesc;
    }

    return mr_nullptr;
}

/*!
 * @brief   Take out desc from pending list
 * @param   sptr_chan: channel structure
 * @retval  none
 * @note    none
 */
static void imx_sdma_issue_pending(struct fwk_dma_chan *sptr_chan)
{
    struct imx_sdma_channel *sptr_channel;
    struct imx_sdma_desc *sptr_desc;

    sptr_channel = mr_imx_sdma_to_chan(sptr_chan);

    /*!< Busy */
    if (sptr_channel->sptr_desc)
        return;

    spin_lock_irqsave(&sptr_channel->sgtc_lock);
    sptr_desc = mr_list_first_valid_entry(&sptr_channel->sgtc_pending, struct imx_sdma_desc, sgtc_link);
    spin_unlock_irqrestore(&sptr_channel->sgtc_lock);

    if (isValid(sptr_desc))
        imx_sdma_desc_start(sptr_desc);
}

/*!
 * @brief   Quit transfer
 * @param   sptr_chan: channel structure
 * @retval  errno
 * @note    none
 */
static kint32_t imx_sdma_pause(struct fwk_dma_chan *sptr_chan)
{
    return ER_NORMAL;
}

/*!
 * @brief   Resume transfer
 * @param   sptr_chan: channel structure
 * @retval  errno
 * @note    none
 */
static kint32_t imx_sdma_resume(struct fwk_dma_chan *sptr_chan)
{
    return ER_NORMAL;
}

/*!
 * @brief   Stop all transfer
 * @param   sptr_chan: channel structure
 * @retval  errno
 * @note    none
 */
static kint32_t imx_sdma_terminate_all(struct fwk_dma_chan *sptr_chan)
{
    return ER_NORMAL;
}

/*!
 * @brief   imx_dma_driver_probe
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_dma_driver_probe_dt(struct fwk_platdev *sptr_pdev)
{
    struct imx_sdma_drv_data *sptr_data;
    struct fwk_device_node *sptr_node;
    struct fwk_of_device_id *sptr_idt;

    sptr_data = fwk_platform_get_drvdata(sptr_pdev);
    sptr_node = sptr_pdev->sgtc_dev.sptr_node;

    /*!< Get Private Data (Hardware Parameters) */
    sptr_idt = fwk_of_get_device_id(sgtc_imx_dma_driver_id, &sptr_pdev->sgtc_dev);
    if (sptr_idt)
        sptr_data->sptr_priv = sptr_idt->data;
    else
        sptr_data->sptr_priv = &sgtc_imx_sdma_platform_data;

    /*!< Get SDMA Base Address */
    sptr_data->base = fwk_of_iomap(sptr_node, 0);
    if (mr_unlikely(!isValid(sptr_data->base)))
        return PTR_ERR(sptr_data->base);

    /*!< Get IPG and AHB Clock */
    sptr_data->sptr_ipgclk = fwk_clk_get(&sptr_pdev->sgtc_dev, "ipg");
    if (!isValid(sptr_data->sptr_ipgclk))
        goto fail1;

    sptr_data->sptr_ahbclk = fwk_clk_get(&sptr_pdev->sgtc_dev, "ahb");
    if (!isValid(sptr_data->sptr_ahbclk))
        goto fail2;

    /*!< Get IRQ Number */
    sptr_data->irq = fwk_platform_get_irq(sptr_pdev, 0);
    if (sptr_data->irq < 0)
        goto fail3;

    /*!< Get The Location of Script-File On SRAM */
    fwk_of_property_read_string(sptr_node, "fsl,sdma-ram-script-name", &sptr_data->sram_script_file);

    return ER_NORMAL;

fail3:
    fwk_clk_put(sptr_data->sptr_ahbclk);
fail2:
    fwk_clk_put(sptr_data->sptr_ipgclk);
fail1:
    fwk_io_unmap(sptr_data->base);
    return -ER_FAULT;
}

/*!
 * @brief   imx_dma_driver_probe
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_dma_driver_probe(struct fwk_platdev *sptr_pdev)
{
    struct imx_sdma_drv_data *sptr_data;
    struct imx_sdma_domain *sptr_domain;
    struct fwk_dma_device *sptr_madev;
    struct imx_sdma_channel *sptr_chans;

    sptr_data = kzalloc(sizeof(*sptr_data), GFP_KERNEL);
    if (mr_unlikely(!isValid(sptr_data)))
        return PTR_ERR(sptr_data);

    fwk_platform_set_drvdata(sptr_pdev, sptr_data);
    if (imx_dma_driver_probe_dt(sptr_pdev))
        goto fail1;

    /*!< Enable clock before DMA Register Initialization */
    fwk_clk_prepare_enable(sptr_data->sptr_ipgclk);
    fwk_clk_prepare_enable(sptr_data->sptr_ahbclk);

    sptr_domain = &sptr_data->sgtc_domain;
    sptr_domain->sptr_sdma = (srt_imx_sdma_t *)sptr_data->base;
    sptr_domain->n_channels = IMX6UL_SDMA_MODULE_CHANNEL;

    /*!< SDMA Initial */
    if (imx_sdma_domain_initial(sptr_domain, sptr_data->sptr_priv))
        goto fail2;

    sptr_madev = &sptr_domain->sgtc_madev;
    sptr_madev->sptr_node = sptr_pdev->sgtc_dev.sptr_node;
    sptr_madev->n_channels = sptr_domain->n_channels;
    sptr_madev->num_sources = sptr_data->sptr_priv->event_num;
    sptr_madev->sptr_dev = &sptr_pdev->sgtc_dev;
    init_list_head(&sptr_madev->sgtc_link);
    init_list_head(&sptr_madev->sgtc_channels);

    /*!< Set Insterface */
    sptr_madev->of_dma_xlate = imx_of_dma_xlate;
    sptr_madev->alloc_chan_resources = imx_alloc_chan_resources;
    sptr_madev->free_chan_resources = imx_free_chan_resources;
    sptr_madev->config = imx_sdma_config;
    sptr_madev->issue_pending = imx_sdma_issue_pending;
    sptr_madev->pause = imx_sdma_pause;
    sptr_madev->resume = imx_sdma_resume;
    sptr_madev->terminate_all = imx_sdma_terminate_all;
    sptr_madev->transfer_prepare = imx_sdma_transfer_prepare;

    /*!< Initialize All Channels */
    for (kint32_t channel = 0; channel < sptr_madev->n_channels; channel++) 
    {
        sptr_chans = &sptr_data->sgtc_channels[channel];

        sptr_chans->sptr_chip = sptr_domain;
        sptr_chans->sptr_context = sptr_domain->sptr_context + channel;
        sptr_chans->sgtc_chan.sptr_device = sptr_madev;

        init_list_head(&sptr_chans->sgtc_pending);
        init_list_head(&sptr_chans->sgtc_completed);
        init_list_head(&sptr_chans->sgtc_chan.sgtc_link);
        spin_lock_init(&sptr_chans->sgtc_lock);

        list_head_add_tail(&sptr_madev->sgtc_channels, &sptr_chans->sgtc_chan.sgtc_link);
    }

    if (fwk_request_threaded_irq(sptr_data->irq, imx_sdma_isr, imx_sdma_thread_isr, 0, "imx-sdma", sptr_data))
        goto fail3;

    /*!< Register DMA Controller */
    if (fwk_dma_device_register(sptr_madev))
        goto fail4;

    fwk_clk_disable_unprepare(sptr_data->sptr_ahbclk);
    fwk_clk_disable_unprepare(sptr_data->sptr_ipgclk);
    
    return ER_NORMAL;

fail4:
    fwk_free_irq(sptr_data->irq, sptr_data);
fail3:
    imx_sdma_domain_deinit(sptr_domain);
fail2:
    fwk_clk_disable_unprepare(sptr_data->sptr_ahbclk);
    fwk_clk_disable_unprepare(sptr_data->sptr_ipgclk);
    fwk_clk_put(sptr_data->sptr_ahbclk);
    fwk_clk_put(sptr_data->sptr_ipgclk);
    fwk_io_unmap(sptr_data->base);
fail1:
    kfree(sptr_data);
    fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);

    return -ER_FAILD;
}

/*!
 * @brief   imx_dma_driver_remove
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_dma_driver_remove(struct fwk_platdev *sptr_pdev)
{
    struct imx_sdma_drv_data *sptr_data;
    struct imx_sdma_domain *sptr_domain;
    struct fwk_dma_device *sptr_madev;

    sptr_data = fwk_platform_get_drvdata(sptr_pdev);
    sptr_domain = &sptr_data->sgtc_domain;
    sptr_madev = &sptr_domain->sgtc_madev;

    fwk_dma_device_unregister(sptr_madev);

    fwk_disable_irq(sptr_data->irq);
    fwk_free_irq(sptr_data->irq, sptr_data);
    imx_sdma_domain_deinit(sptr_domain);
    fwk_clk_disable_unprepare(sptr_data->sptr_ahbclk);
    fwk_clk_disable_unprepare(sptr_data->sptr_ipgclk);
    fwk_clk_put(sptr_data->sptr_ahbclk);
    fwk_clk_put(sptr_data->sptr_ipgclk);
    fwk_io_unmap(sptr_data->base);
    kfree(sptr_data);
    fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);

    return ER_NORMAL;
}

/*!< platform instance */
static struct fwk_platdrv sgtc_imx_sdma_platdrv = 
{
    .probe	= imx_dma_driver_probe,
    .remove	= imx_dma_driver_remove,
    
    .sgtc_driver = 
    {
        .name 	= "fsl,sdma",
        .id 	= -1,
        .sptr_of_match_table = sgtc_imx_dma_driver_id,
    },
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   imx_dma_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init imx_dma_driver_init(void)
{
    return fwk_register_platdriver(&sgtc_imx_sdma_platdrv);
}

/*!
 * @brief   imx_dma_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit imx_dma_driver_exit(void)
{
    fwk_unregister_platdriver(&sgtc_imx_sdma_platdrv);
}

IMPORT_PATTERN_INIT(imx_dma_driver_init);
IMPORT_PATTERN_EXIT(imx_dma_driver_exit);

/*!< end of file */
