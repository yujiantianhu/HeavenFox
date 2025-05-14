/*
 * Platform DMA Interface
 *
 * File Name:   fwk_dma.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.04.21
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_DMA_H
#define __FWK_DMA_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_platform.h>
#include <platform/of/fwk_of.h>

/*!< The defines */
struct fwk_dma_device;

/*!< Direction */
#define DMA_MEM_TO_MEM                              (0U)
#define DMA_MEM_TO_DEVICE                           (1U)
#define DMA_DEVICE_TO_MEM                           (2U)

enum __ERT_DMA_TRX_WIDTH {
    NR_DMA_TRX_WIDTH_1BYTE = 0U,                    /*!< kuint8_t */
    NR_DMA_TRX_WIDTH_2BYTE,                         /*!< kuint16_t */
    NR_DMA_TRX_WIDTH_4BYTE,                         /*!< kuint32_t */
};

struct fwk_dma_slave_config 
{
    kuint32_t direction;
    kuaddr_t src_addr;
    kuaddr_t dst_addr;
    kuint32_t src_addr_width;
    kuint32_t dst_addr_width;
};

struct fwk_dma_chan
{
    kint32_t flags;

    struct fwk_dma_device *sptr_device;
    kuint32_t client_count;
    struct list_head sgtc_link;

    void *private_data;
};

struct fwk_dma_block_data
{
    void *cpu_address;
    kuaddr_t dma_address;
    kuint32_t offset;
    kuint32_t length;

    struct fwk_dma_block_data *sptr_next;
};

struct fwk_dma_transfer_desc
{
    kint32_t flags;

    kuaddr_t phys;
    struct fwk_dma_chan *sptr_chan;

    kint32_t (*submit)(struct fwk_dma_transfer_desc *sptr_txdesc);
    void (*transfer_callback)(void *callback_param);
    void *callback_param;
};

struct fwk_dma_device
{
    struct fwk_device_node *sptr_node;
    
    kuint32_t n_channels;
    kuint32_t num_sources;

    struct fwk_device *sptr_dev;
    struct list_head sgtc_link;
    struct list_head sgtc_channels;

    struct fwk_dma_chan *(*of_dma_xlate)(struct fwk_of_phandle_args *sptr_args, struct fwk_dma_device *sptr_mdev);
    kint32_t (*alloc_chan_resources)(struct fwk_dma_chan *sptr_chan);
    kint32_t (*free_chan_resources)(struct fwk_dma_chan *sptr_chan);
    void (*issue_pending)(struct fwk_dma_chan *sptr_chan);
    kint32_t (*config)(struct fwk_dma_chan *sptr_chan, struct fwk_dma_slave_config *sptr_config);
    kint32_t (*pause)(struct fwk_dma_chan *sptr_chan);
    kint32_t (*resume)(struct fwk_dma_chan *sptr_chan);
    kint32_t (*terminate_all)(struct fwk_dma_chan *sptr_chan);

    /*!< m2m, p2m, m2p */
    struct fwk_dma_transfer_desc *(*transfer_prepare)(struct fwk_dma_chan *sptr_chan, 
                                    struct fwk_dma_block_data *sptr_src, kusize_t src_count, 
                                    struct fwk_dma_block_data *sptr_dst, kusize_t dst_count, 
                                    kuint32_t direction, kuint32_t flags, void *context);
};

/*!< The functions */
extern kint32_t fwk_dma_device_register(struct fwk_dma_device *sptr_dmadev);
extern void fwk_dma_device_unregister(struct fwk_dma_device *sptr_dmadev);
extern struct fwk_dma_chan *fwk_of_dma_request_chan(struct fwk_device *sptr_dev, const kchar_t *con_id);
extern struct fwk_dma_chan *fwk_dma_find_candidate(struct fwk_dma_device *sptr_mdev, 
                                kbool_t (*dma_filter_fn)(struct fwk_dma_chan *sptr_chan, void *filter_param), void *filter_param);
extern struct fwk_dma_chan *fwk_dma_request_chan(struct fwk_device *sptr_dev, const kchar_t *con_id);
extern void fwk_dma_release_chan(struct fwk_dma_chan *sptr_chan);

extern void *fwk_dma_alloc_coherent(struct fwk_device *sptr_dev, kusize_t size, kuaddr_t *dma_handle, nrt_gfp_t mask);
extern void fwk_dma_free_coherent(void *dma_buffer, kusize_t size, void *cpu_addr, kuaddr_t *dma_handle);
extern kint32_t fwk_dma_config(struct fwk_dma_chan *sptr_chan, struct fwk_dma_slave_config *sptr_config);
extern struct fwk_dma_transfer_desc *fwk_dma_transfer_prepare(struct fwk_dma_chan *sptr_chan, 
                                        struct fwk_dma_block_data *sptr_src, kusize_t src_count, 
                                        struct fwk_dma_block_data *sptr_dst, kusize_t dst_count, 
                                        kuint32_t direction, kuint32_t flag, void *context);
extern kint32_t fwk_dma_terminate_all(struct fwk_dma_chan *sptr_chan);
extern kint32_t fwk_dma_pause(struct fwk_dma_chan *sptr_chan);
extern kint32_t fwk_dma_resume(struct fwk_dma_chan *sptr_chan);
extern kint32_t fwk_dma_submit(struct fwk_dma_transfer_desc *sptr_desc);
extern void fwk_dma_issue_pending(struct fwk_dma_chan *sptr_chan);

/*!< API functions */
static inline void fwk_dma_block_data_init(struct fwk_dma_block_data *sptr_bdata,
                        void *cpu_address, kuaddr_t dma_address, kuint32_t offset, kuint32_t length)
{
    sptr_bdata->cpu_address = cpu_address;
    sptr_bdata->dma_address = dma_address;
    sptr_bdata->offset = offset;
    sptr_bdata->length = length;
    sptr_bdata->sptr_next = mr_nullptr;
}

#ifdef __cplusplus
    }
#endif

#endif /* __FWK_DMA_H */
