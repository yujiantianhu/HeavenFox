/*
 * IMX6ULL Board SDMA Initial
 *
 * File Name:   imx6_sdma.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.04.06
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include "imx6_common.h"
#include <platform/fwk_mempool.h>

/*!< The defines */
struct imx_sdma_domain;

enum __ERT_IMX_SDMA_SWITCH_MODE
{
    NR_IMX_SDMA_MODE_STATIC = 0U,                   /*!< SDMA context switch mode static */
    NR_IMX_SDMA_MODE_DYNC_LOW_PWR,                  /*!< SDMA context switch mode dynamic with low power */
    NR_IMX_SDMA_MODE_DYNC_NO_LOOP,                  /*!< SDMA context switch mode dynamic with no loop */
    NR_IMX_SDMA_MODE_DYNC,                          /*!< SDMA context switch mode dynamic */
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
    kuint32_t GeneralReg[8];                        /*!< 8 general regsiters used for SDMA RISC core */
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

/*!< script address */
struct imx_sdma_script_type
{
    kint32_t periph;
    kuaddr_t script_addr;
};

#define IMX_SDMA_MEM_TO_MEM                         (0U)
#define IMX_SDMA_MEM_TO_PERIPH                      (1U)
#define IMX_SDMA_PERIPH_TO_MEM                      (2U)

#define IMX_SDMA_PERIPH_NORMAL                      (0U)
#define IMX_SDMA_PERIPH_NORMAL_SP                   (1U)
#define IMX_SDMA_PERIPH_UART                        (2U)
#define IMX_SDMA_PERIPH_UART_SP                     (3U)
#define IMX_SDMA_PERPH_SPDIF                        (4U)

/*!< sdma channel control descriptor structure */
struct imx_sdma_ctrl_desc
{
    kuint32_t cur_bd;                               /*!< Address of current buffer descriptor processed  */
    kuint32_t base_bd;                              /*!< The start address of the buffer descriptor array */
    kuint32_t chan_desc;                            /*!< Optional for transfer */
    kuint32_t status;                               /*!< Channel status */
};

/*!< sdma channel buffer descriptor (BD) structure */
struct imx_sdma_buffer_desc
{
    kuint16_t count;                                /*!< Bytes of the buffer length for this buffer descriptor. */
    kuint8_t status;                                /*!< E,R,I,C,W,D status bits stored here */
    kuint8_t command;                               /*!< command mostlky used for channel 0 */
    kuint32_t buffer_addr;                          /*!< Buffer start address for this descriptor. */
    kuint32_t extend_addr;                          /*!< External buffer start address, this is an optional for a transfer. */
};

#define IMX_SDMA_CHANNEL(x)                         (x)

struct imx_sdma_desc
{
    struct imx_sdma_domain *sprt_chip;
    struct imx_sdma_script_context *sprt_context;

    kuint32_t cur_channel;
    kuint32_t event_source;

    kuint32_t bd_count;
    kuint32_t priority;

    struct {
        kuaddr_t src_addr;
        kuaddr_t dest_addr;
        kusize_t size_of_src_transfer;
        kusize_t size_of_dest_transfer;
        kuint32_t script_addr;
        kusize_t bytes_to_transfer;
        kusize_t bytes_minor_loop;
    } s;
};

#define mrt_imx_sdma_set_address(sprt_desc, _src_addr, _dest_addr)    \
    do {    \
        (sprt_desc)->src_addr = _src_addr; \
        (sprt_desc)->dest_addr = _dest_addr; \
    } while (0)

#define IMX_SDMA_TRANSFER_4BYTES                    (0U)
#define IMX_SDMA_TRANSFER_1BYTES                    (1U)
#define IMX_SDMA_TRANSFER_2BYTES                    (2U)

#define mrt_imx_sdma_set_size(sprt_desc, _total_bytes, _per_bytes, _src_size, _dest_size)   \
    do {    \
        (sprt_desc)->size_of_src_transfer = _src_size;  \
        (sprt_desc)->size_of_dest_transfer = _dest_size;  \
        (sprt_desc)->bytes_to_transfer = _total_bytes;  \
        (sprt_desc)->bytes_minor_loop = _per_bytes;  \
    } while (0)

struct imx_sdma_domain
{
    srt_imx_sdma_t *sprt_sdma;

    kuint32_t n_channels;
    struct imx_sdma_ctrl_desc *sprt_cds;
    struct imx_sdma_buffer_desc *sprt_bds;

    struct imx_sdma_desc **sprt_descs;
};

/*!< The globals */
static const struct imx_sdma_script_type sgrt_imx_sdma_script_table[3][5] =
{
    [IMX_SDMA_MEM_TO_MEM] = 
    {
        { .periph = -1,                         .script_addr = IMX6UL_SDMA_M2M_ADDR         },
    },

    [IMX_SDMA_MEM_TO_PERIPH] = 
    {
        { .periph = IMX_SDMA_PERIPH_NORMAL,     .script_addr = IMX6UL_SDMA_M2P_ADDR         },
        { .periph = IMX_SDMA_PERIPH_NORMAL_SP,  .script_addr = IMX6UL_SDMA_M2SHP_ADDR       },
        { .periph = IMX_SDMA_PERIPH_UART,       .script_addr = IMX6UL_SDMA_M2P_ADDR         },
        { .periph = IMX_SDMA_PERIPH_UART_SP,    .script_addr = IMX6UL_SDMA_M2SHP_ADDR       },
        { .periph = IMX_SDMA_PERPH_SPDIF,       .script_addr = IMX6UL_SDMA_M2SPDIF_ADDR     },
    },

    [IMX_SDMA_PERIPH_TO_MEM] = 
    {
        { .periph = IMX_SDMA_PERIPH_NORMAL,     .script_addr = IMX6UL_SDMA_P2M_ADDR         },
        { .periph = IMX_SDMA_PERIPH_NORMAL_SP,  .script_addr = IMX6UL_SDMA_M2SHP_ADDR       },
        { .periph = IMX_SDMA_PERIPH_UART,       .script_addr = IMX6UL_SDMA_UART2M_ADDR      },
        { .periph = IMX_SDMA_PERIPH_UART_SP,    .script_addr = IMX6UL_SDMA_UARTSH2M_ADDR    },
        { .periph = IMX_SDMA_PERPH_SPDIF,       .script_addr = IMX6UL_SDMA_SPDIF2M_ADDR     },
    },
};

static struct imx_sdma_ctrl_desc sgrt_imx_sdma_ctrl_descs[IMX6UL_SDMA_MODULE_CHANNEL] __align(8);
static struct imx_sdma_buffer_desc sgrt_imx_sdma_buffer_descs[IMX6UL_SDMA_MODULE_CHANNEL] __align(8);

/*!< API function */
/*!
 * @brief   set priority of channel
 * @param   sprt_sdma: base address of sdma
 * @param   channel: number
 * @param   priority: current channel's attribute
 * @retval  none
 * @note    set SDMA_CHNPRI; the default value is 0 for all channels, priority 0 will prevents
 *          channel from starting, so the priority must be set before start a channel
 */
void imx_sdma_set_channel_priority(srt_imx_sdma_t *sprt_sdma, kuint32_t channel, kuint8_t priority)
{
    mrt_writel(priority, &sprt_sdma->SDMA_CHNPRI[channel]);
}

/*!
 * @brief   set sdma request source mapping channel
 * @param   sprt_sdma: base address of sdma
 * @param   source: request source number
 * @param   mask: channel mask, e.g. 0x3 is channel 0 and 1
 * @retval  none
 * @note    set CHNENBL; sdma supports an event trigger multi-channel. 
 *          A channel can also be triggered by several source events
 */
void imx_sdma_set_source_channel(srt_imx_sdma_t *sprt_sdma, kuint32_t source, kuint32_t mask)
{
    mrt_writel(mask, &sprt_sdma->CHNENBL[source]);
}

/*!
 * @brief   start a channel by software
 * @param   sprt_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    set HSTART
 */
void imx_sdma_start_channel(srt_imx_sdma_t *sprt_sdma, kuint32_t channel)
{
    mrt_writel(mrt_bit(channel), &sprt_sdma->HSTART);
}

/*!
 * @brief   start a channel by hardware event
 * @param   sprt_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    set EVTPEND
 */
void imx_sdma_start_channel_event(srt_imx_sdma_t *sprt_sdma, kuint32_t channel)
{
    mrt_writel(mrt_bit(channel), &sprt_sdma->EVTPEND);
}

/*!
 * @brief   stop a channel
 * @param   sprt_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    set STOP_STAT
 */
void imx_sdma_stop_channel(srt_imx_sdma_t *sprt_sdma, kuint32_t channel)
{
    mrt_writel(mrt_bit(channel), &sprt_sdma->STOP_STAT);
}

/*!
 * @brief   get interrupt status of all channels
 * @param   sprt_sdma: base address of sdma
 * @retval  *INTR
 * @note    get *INTR
 */
kuint32_t imx_sdma_get_int_status(srt_imx_sdma_t *sprt_sdma)
{
    return mrt_readl(&sprt_sdma->INTR);
}

/*!
 * @brief   set interrupt status ---> clear one status
 * @param   sprt_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_channel_int_status(srt_imx_sdma_t *sprt_sdma, kuint32_t channel)
{
    mrt_writel(mrt_bit(channel), &sprt_sdma->INTR);
}

/*!
 * @brief   set interrupt status ---> clear multiple status
 * @param   sprt_sdma: base address of sdma
 * @param   mask: e.g. 0x3 for channel 0 and 1
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_multiple_int_status(srt_imx_sdma_t *sprt_sdma, kuint32_t mask)
{
    mrt_writel(mask, &sprt_sdma->INTR);
}

/*!
 * @brief   set interrupt status ---> clear all status
 * @param   sprt_sdma: base address of sdma
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_all_int_status(srt_imx_sdma_t *sprt_sdma)
{
    kuint32_t mask = mrt_readl(&sprt_sdma->INTR);
    mrt_writel(mask, &sprt_sdma->INTR);
}

/*!
 * @brief   get stop status of all channels
 * @param   sprt_sdma: base address of sdma
 * @retval  *STOP_STAT
 * @note    get *STOP_STAT
 */
kuint32_t imx_sdma_get_stop_status(srt_imx_sdma_t *sprt_sdma)
{
    return mrt_readl(&sprt_sdma->STOP_STAT);
}

/*!
 * @brief   get stop status of one channel
 * @param   sprt_sdma: base address of sdma
 * @retval  *STOP_STAT
 * @note    get *STOP_STAT
 */
kbool_t imx_sdma_get_channel_stop_status(srt_imx_sdma_t *sprt_sdma, kuint32_t channel)
{
    return !!mrt_getbitl(mrt_bit(channel), &sprt_sdma->STOP_STAT);
}

/*!
 * @brief   set stop status ---> clear one status
 * @param   sprt_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_channel_stop_status(srt_imx_sdma_t *sprt_sdma, kuint32_t channel)
{
    mrt_writel(mrt_bit(channel), &sprt_sdma->STOP_STAT);
}

/*!
 * @brief   set stop status ---> clear multiple status
 * @param   sprt_sdma: base address of sdma
 * @param   mask: e.g. 0x3 for channel 0 and 1
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_multiple_stop_status(srt_imx_sdma_t *sprt_sdma, kuint32_t mask)
{
    mrt_writel(mask, &sprt_sdma->STOP_STAT);
}

/*!
 * @brief   set stop status ---> clear all status
 * @param   sprt_sdma: base address of sdma
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_all_stop_status(srt_imx_sdma_t *sprt_sdma)
{
    kuint32_t mask = mrt_readl(&sprt_sdma->STOP_STAT);
    mrt_writel(mask, &sprt_sdma->STOP_STAT);
}

/*!
 * @brief   get pending status of all channels
 * @param   sprt_sdma: base address of sdma
 * @retval  *EVTPEND
 * @note    get *EVTPEND
 */
kuint32_t imx_sdma_get_pend_status(srt_imx_sdma_t *sprt_sdma)
{
    return mrt_readl(&sprt_sdma->EVTPEND);
}

/*!
 * @brief   set pending status ---> clear one status
 * @param   sprt_sdma: base address of sdma
 * @param   channel: number
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_channel_pend_status(srt_imx_sdma_t *sprt_sdma, kuint32_t channel)
{
    mrt_writel(mrt_bit(channel), &sprt_sdma->EVTPEND);
}

/*!
 * @brief   set pending status ---> clear multiple status
 * @param   sprt_sdma: base address of sdma
 * @param   mask: e.g. 0x3 for channel 0 and 1
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_multiple_pend_status(srt_imx_sdma_t *sprt_sdma, kuint32_t mask)
{
    mrt_writel(mask, &sprt_sdma->EVTPEND);
}

/*!
 * @brief   set pending status ---> clear all status
 * @param   sprt_sdma: base address of sdma
 * @retval  none
 * @note    none
 */
void imx_sdma_clear_all_pend_status(srt_imx_sdma_t *sprt_sdma)
{
    kuint32_t mask = mrt_readl(&sprt_sdma->EVTPEND);
    mrt_writel(mask, &sprt_sdma->EVTPEND);
}

/*!
 * @brief   get error status of all channels
 * @param   sprt_sdma: base address of sdma
 * @retval  *EVTERR
 * @note    get *EVTERR
 */
kuint32_t imx_sdma_get_error_status(srt_imx_sdma_t *sprt_sdma)
{
    return mrt_readl(&sprt_sdma->EVTERR);
}

/*!
 * @brief   set context switch mode
 * @param   sprt_sdma: base address of sdma
 * @param   mode: __ERT_IMX_SDMA_SWITCH_MODE
 * @retval  none
 * @note    none
 */
void imx_sdma_set_context_switch_mode(srt_imx_sdma_t *sprt_sdma, kuint32_t mode)
{
    kuint32_t val;

    val = mrt_readl(&sprt_sdma->CONFIG) & (~IMX6UL_SDMA_CONFIG_CSM_MASK);
    mrt_writel(val | mode, &sprt_sdma->CONFIG);
}

/*!
 * @brief   set context switch mode
 * @param   sprt_sdma: base address of sdma
 * @param   source: request source
 * @retval  *EVT_MIRROR & mrt_bit(source)
 * @note    none
 */
kbool_t imx_sdma_get_request_src_status(srt_imx_sdma_t *sprt_sdma, kuint32_t source)
{
    return !!((source < 32U) ? mrt_getbitl(mrt_bit(source), &sprt_sdma->EVT_MIRROR) : 
                               mrt_getbitl(mrt_bit(source - 32U), &sprt_sdma->EVT_MIRROR2));
}

/*!< ---------------------------------------------------------------------------- */
/*!
 * @brief   reset sdma
 * @param   sprt_sdma: base address of sdma
 * @retval  none
 * @note    none
 */
static void imx_sdma_run_channel0(srt_imx_sdma_t *sprt_sdma)
{
    imx_sdma_start_channel(sprt_sdma, 0);

    /*!< Waitting for "sprt_sdma->STOP_STAT & mrt_bit(1)" reset */
    while (imx_sdma_get_channel_stop_status(sprt_sdma, 0));

    /*!< clear interrupt status, and set context switch mode to dynamic */
    imx_sdma_clear_channel_int_status(sprt_sdma, 0);
    imx_sdma_set_context_switch_mode(sprt_sdma, NR_IMX_SDMA_MODE_DYNC);
}

/*!
 * @brief   reset sdma
 * @param   sprt_sdma: base address of sdma
 * @retval  none
 * @note    none
 */
static void imx_sdma_reset_module(srt_imx_sdma_t *sprt_sdma)
{
    /*!< Channel 0 */
    mrt_resetl(&sprt_sdma->MC0PTR);

    /*!< set sprt_sdma->INTR to 0 */
    imx_sdma_clear_all_int_status(sprt_sdma);
    /*!< set sprt_sdma->STOP_STAT to 0 */
    imx_sdma_clear_all_stop_status(sprt_sdma);

    mrt_resetl(&sprt_sdma->EVTOVR);
    mrt_writel(IMX6UL_SDMA_DSPOVR_DO_U32(~0U), &sprt_sdma->DSPOVR);
    mrt_resetl(&sprt_sdma->HOSTOVR);

    /*!< set sprt_sdma->EVTPEND to 0 */
    imx_sdma_clear_all_pend_status(sprt_sdma);

    mrt_resetl(&sprt_sdma->INTRMASK);

    /*!< set sprt_sdma->CHNENBL[0-47] to 0, disable all events */
    for (kuint32_t event = 0; event < IMX6UL_SDMA_EVENT_NUM; event++)
        imx_sdma_set_source_channel(sprt_sdma, event, 0);

    /*!< set sprt_sdma->SDMA_CHNPRI[0-31] to 0, clear all channel's priority */
    for (kuint32_t chan = 0; chan < IMX6UL_SDMA_MODULE_CHANNEL; chan++)
        imx_sdma_set_channel_priority(sprt_sdma, chan, 0);
}

/*!
 * @brief   allocate sdma descriptor
 * @param   sprt_sdma: base address of sdma
 * @retval  sprt_domain
 * @note    none
 */
struct imx_sdma_domain *imx_sdma_domain_alloc(srt_imx_sdma_t *sprt_sdma)
{
    struct imx_sdma_domain *sprt_domain;
    kuint32_t n_channels;

    sprt_domain = kmalloc(sizeof(*sprt_domain), GFP_KERNEL);
    if (!isValid(sprt_domain))
        return ERR_PTR(-ER_NOMEM);

    sprt_domain->sprt_sdma = sprt_sdma;
    sprt_domain->n_channels = ARRAY_SIZE(sgrt_imx_sdma_ctrl_descs);
    sprt_domain->sprt_cds = &sgrt_imx_sdma_ctrl_descs[0];
    sprt_domain->sprt_bds = &sgrt_imx_sdma_buffer_descs[0];

    sprt_domain->sprt_descs = kzalloc(sprt_domain->n_channels * sizeof(sprt_domain->sprt_descs), GFP_KERNEL);
    if (!isValid(sprt_domain->sprt_descs))
    {
        kfree(sprt_domain);
        return ERR_PTR(-ER_NOMEM);
    }

    memset(sprt_domain->sprt_descs, 0, sizeof(sprt_domain->sprt_descs));
    return sprt_domain;
}

void imx_sdma_domain_destroy(struct imx_sdma_domain *sprt_domain)
{
    if (!sprt_domain)
        return;
    
    if (sprt_domain->sprt_descs)
        kfree(sprt_domain->sprt_descs);

    kfree(sprt_domain);
}

kint32_t imx_sdma_bd_configure(struct imx_sdma_domain *sprt_domain, kuint32_t channel)
{
    if (!sprt_domain ||
        (channel >= sprt_domain->n_channels))
        return -ER_UNVALID;

    /*!< Configure Ctrl Desc */
    sprt_domain->sprt_cds[channel].cur_bd  = (kuint32_t)&sprt_domain->sprt_bds[channel];
    sprt_domain->sprt_cds[channel].base_bd = sprt_domain->sprt_cds[channel].cur_bd;

    return ER_NORMAL;
}

/*!
 * @brief   initialize sdma
 * @param   sprt_domain: base address of sdma domain
 * @retval  errno
 * @note    none
 */
kint32_t imx_sdma_domain_initial(struct imx_sdma_domain *sprt_domain)
{
    srt_imx_sdma_t *sprt_sdma;
    kuint32_t reg_value;

    if (!sprt_domain ||
        !sprt_domain->sprt_sdma ||
        !sprt_domain->sprt_cds ||
        !sprt_domain->sprt_bds)
        return -ER_EMPTY;

    sprt_sdma = sprt_domain->sprt_sdma;

    /*!< Reset all SDMA registers */
    imx_sdma_reset_module(sprt_sdma);

    /*!< 0: channel 0 */
    imx_sdma_set_channel_priority(sprt_sdma, 0, 0x7U);

    /*!< Set channel 0 ownership */
    mrt_writel(0U, &sprt_sdma->HOSTOVR);
    mrt_writel(1U, &sprt_sdma->EVTOVR);

    /*!< Configure Ctrl Desc */
    if (imx_sdma_bd_configure(sprt_domain, IMX_SDMA_CHANNEL(0U)))
        return -ER_FAILD;

    /*!< 
     * Core clock ratio: half of ARM platform;
     * Real Time Debug Pin: 0U
     */
    reg_value = mrt_readl(&sprt_sdma->CONFIG);
    mrt_clrbitl(IMX6UL_SDMA_CONFIG_ACR_MASK | IMX6UL_SDMA_CONFIG_RTDOBS_MASK, &reg_value);
    mrt_clrbitl(IMX6UL_SDMA_CONFIG_CSM_MASK, &reg_value);
    mrt_writel(reg_value, &sprt_sdma->CONFIG);

    reg_value = mrt_readl(&sprt_sdma->SDMA_LOCK);
    mrt_clrbitl(IMX6UL_SDMA_LOCK_SRESET_LOCK_CLR_MASK, &reg_value);
    mrt_setbitl(IMX6UL_SDMA_LOCK_SRESET_LOCK_CLR(1U), &reg_value);
    mrt_writel(reg_value, &sprt_sdma->SDMA_LOCK);

    /*!< Set the context size to 32 bytes */
    mrt_writel(0x4050U, &sprt_sdma->CHN0ADDR);

    /*!< Set channle 0 CCB address */
    mrt_writel(sprt_domain->sprt_cds + IMX_SDMA_CHANNEL(0U), &sprt_sdma->MC0PTR);

    return ER_NORMAL;
}

struct imx_sdma_desc *imx_sdma_desc_alloc(kuint32_t channel, kuint32_t event)
{
    struct imx_sdma_desc *sprt_desc;

    sprt_desc = kmalloc(sizeof(*sprt_desc), GFP_KERNEL);
    if (!isValid(sprt_desc))
        return ERR_PTR(-ER_NOMEM);

    sprt_desc->bd_count = 1U;
    sprt_desc->cur_channel = channel;
    sprt_desc->event_source = event;
    sprt_desc->priority = 0U;
    sprt_desc->sprt_chip = sprt_desc->sprt_context = mrt_nullptr;

    return sprt_desc;
}

void imx_sdma_desc_destroy(struct imx_sdma_desc *sprt_desc)
{
    if (sprt_desc)
        kfree(sprt_desc);
}

kint32_t imx_sdma_desc_add(struct imx_sdma_domain *sprt_domain, struct imx_sdma_desc *sprt_desc)
{
    srt_imx_sdma_t *sprt_sdma;
    kuint32_t channel;

    if (!sprt_domain ||
        !sprt_desc)
        return -ER_NODEV;
    
    channel = sprt_desc->cur_channel;
    if (sprt_domain->sprt_descs[channel])
        return -ER_EXISTED;

    /*!< Configure Ctrl Desc */
    if (imx_sdma_bd_configure(sprt_domain, IMX_SDMA_CHANNEL(channel)))
        return -ER_FAILD;

    sprt_desc->sprt_chip = sprt_domain;
    sprt_domain->sprt_descs[channel] = sprt_desc;

    /*!< Hardware Configure */
    sprt_sdma = sprt_domain->sprt_sdma;

    imx_sdma_set_source_channel(sprt_sdma, sprt_desc->event_source, mrt_bit(channel));
    if (!sprt_desc->priority)
        imx_sdma_set_channel_priority(sprt_sdma, channel, sprt_desc->priority);

    return ER_NORMAL;
}

struct imx_sdma_desc *imx_sdma_desc_get(struct imx_sdma_domain *sprt_domain, kuint32_t channel)
{
    return sprt_domain ? sprt_domain->sprt_descs[channel] : mrt_nullptr;
}

srt_imx_sdma_t *imx_sdma_handle_get(struct imx_sdma_desc *sprt_desc)
{
    return sprt_desc->sprt_chip->sprt_sdma;
}

void imx_sdma_desc_del(struct imx_sdma_desc *sprt_desc)
{
    struct imx_sdma_domain *sprt_domain;
    srt_imx_sdma_t *sprt_sdma;
    kuint32_t channel;

    if (!sprt_desc ||
        !sprt_desc->sprt_chip)
        return;

    channel = sprt_desc->cur_channel;
    sprt_sdma = imx_sdma_handle_get(sprt_desc);

    imx_sdma_set_source_channel(sprt_sdma, 0U, mrt_bit(channel));
    if (!sprt_desc->priority)
        imx_sdma_set_channel_priority(sprt_sdma, channel, 0U);

    sprt_domain = sprt_desc->sprt_chip;
    sprt_desc->sprt_chip = mrt_nullptr;
    sprt_domain->sprt_descs[channel] = mrt_nullptr;
}

/*!
 * @brief   get sdma script address
 * @param   type: memory to memory, memory to peripheral, or peripheral to memory
 * @param   periph: normal, uart, or spdif
 * @retval  script address
 * @note    none
 */
void imx_sdma_set_script_address(struct imx_sdma_desc *sprt_desc, kint32_t type, kint32_t periph)
{
    struct imx_sdma_script_type **sprt_tab;
    
    sprt_tab = &sgrt_imx_sdma_script_table[type];
    sprt_desc->s.script_addr = ((type == IMX_SDMA_MEM_TO_MEM) ? 
                        sprt_tab[0]->script_addr : sprt_tab[periph]->script_addr);
}

void imx_sdma_config(struct imx_sdma_desc *sprt_desc)
{
    srt_imx_sdma_t *sprt_sdma;
    kuint32_t channel;

    sprt_sdma = imx_sdma_handle_get(sprt_desc);
    channel = sprt_desc->cur_channel;

    /*!< DO register shall always set */
    mrt_setbitl(mrt_bit(channel), &sprt_sdma->DSPOVR);

    if (sprt_desc->s.script_addr == IMX6UL_SDMA_M2M_ADDR)
    {
        mrt_setbitl(mrt_bit(channel), &sprt_sdma->EVTOVR);
        mrt_clrbitl(mrt_bit(channel), &sprt_sdma->HOSTOVR);
    }
    else
    {

    }
}

kint32_t imx_sdma_start_transfer(struct imx_sdma_desc *sprt_desc)
{
    srt_imx_sdma_t *sprt_sdma;
    kuint32_t channel;

    if (!sprt_desc ||
        !sprt_desc->sprt_chip)
        return -ER_NODEV;

    sprt_sdma = imx_sdma_handle_get(sprt_desc);
    channel = sprt_desc->cur_channel;

    
}

/* end of file*/
