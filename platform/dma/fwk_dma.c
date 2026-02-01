/*
 * Platform DMA Interface
 *
 * File Name:   fwk_dma.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.04.06
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/of/fwk_of_prop.h>
#include <platform/dma/fwk_dma.h>
#include <kernel/mutex.h>

/*!< The defines */

/*!< The globals */
static DECLARE_LIST_HEAD(sgtc_fwk_dma_devices);
static struct mutex_lock sgtc_fwk_dma_mutex;

/*!< API functions */
/*!
 * @brief   Find DMA controller from the global device list by device_node
 * @param   sptr_node: device tree node
 * @retval  DMA controller
 * @note    none
 */
struct fwk_dma_device *fwk_of_dma_find_controller(struct fwk_device_node *sptr_node)
{
    struct fwk_dma_device *sptr_mdev;

    mutex_lock(&sgtc_fwk_dma_mutex);

    foreach_list_next_entry(sptr_mdev, &sgtc_fwk_dma_devices, sgtc_link)
    {
        /*!< By device-node */
        if (sptr_mdev->sptr_node &&
           (sptr_mdev->sptr_node == sptr_node))
        {
            mutex_unlock(&sgtc_fwk_dma_mutex);
            return sptr_mdev;
        }
    }

    mutex_unlock(&sgtc_fwk_dma_mutex);
    return mr_nullptr;
}

/*!
 * @brief   Register DMA controller
 * @param   sptr_dmadev: controller structure
 * @retval  errno
 * @note    none
 */
kint32_t fwk_dma_device_register(struct fwk_dma_device *sptr_dmadev)
{
    if (!sptr_dmadev ||
        !sptr_dmadev->n_channels)
        return -ER_INVALID;

    if (sptr_dmadev->sptr_node)
    {
        if (fwk_of_dma_find_controller(sptr_dmadev->sptr_node))
            return -ER_EXISTED;
    }

    mutex_lock(&sgtc_fwk_dma_mutex);
    list_head_add_tail(&sgtc_fwk_dma_devices, &sptr_dmadev->sgtc_link);
    mutex_unlock(&sgtc_fwk_dma_mutex);

    return ER_NORMAL;
}

/*!
 * @brief   Unregister DMA controller
 * @param   sptr_dmadev: controller structure
 * @retval  none
 * @note    none
 */
void fwk_dma_device_unregister(struct fwk_dma_device *sptr_dmadev)
{
    if (!sptr_dmadev)
        return;

    mutex_lock(&sgtc_fwk_dma_mutex);
    list_head_del(&sptr_dmadev->sgtc_link);
    mutex_unlock(&sgtc_fwk_dma_mutex);
}

/*!
 * @brief   Request DMA channel by device node
 * @param   sptr_dev: &sptr_pdev->sgtc_dev (to get sptr_dev->sptr_node)
 * @param   con_id: device-node property name (such as "tx", "rx")
 * @retval  channel requested
 * @note    none
 */
struct fwk_dma_chan *fwk_of_dma_request_chan(struct fwk_device *sptr_dev, const kchar_t *con_id)
{
    struct fwk_dma_device *sptr_mdev;
    struct fwk_device_node *sptr_node;
    struct fwk_of_phandle_args sgtc_args;
    kint32_t index;

    sptr_node = sptr_dev->sptr_node;

    index = fwk_of_property_match_string(sptr_node, "dma-names", con_id);
    if (index < 0)
        return ERR_PTR(-ER_NOTFOUND);

    /*!< Get the index of "dma-names" by con_id, then find the property "dmas" and save to sgtc_args */
    if (fwk_of_parse_phandle_with_args(sptr_node, "dmas", "#dma-cells", 0, index, &sgtc_args))
        return ERR_PTR(-ER_NOTFOUND);

    /*!< sgtc_args.sptr_node: dma controller node */
    sptr_mdev = fwk_of_dma_find_controller(sgtc_args.sptr_node);
    if (!isValid(sptr_mdev))
        return ERR_PTR(-ER_NOTFOUND);

    /*!< Parse property "dmas" and request idle channel */
    if (sptr_mdev->of_dma_xlate)
        return sptr_mdev->of_dma_xlate(&sgtc_args, sptr_mdev);

    return ERR_PTR(-ER_INVALID);
}

/*!
 * @brief   Add channel reference count
 * @param   sptr_chan: current channel
 * @retval  errno
 * @note    if is called for the first time, "alloc_chan_resources" will be called
 */
static kint32_t fwk_dma_chan_get(struct fwk_dma_chan *sptr_chan)
{
    if (!sptr_chan || !sptr_chan->sptr_device)
        return -ER_NODEV;

    if (!sptr_chan->client_count)
    {
        struct fwk_dma_device *sptr_mdev = sptr_chan->sptr_device;

        /*!< Only alloc chan for the first time  */
        if (sptr_mdev->alloc_chan_resources)
            sptr_mdev->alloc_chan_resources(sptr_chan);
    }

    sptr_chan->client_count++;
    return ER_NORMAL;
}

/*!
 * @brief   Dec channel reference count
 * @param   sptr_chan: current channel
 * @retval  errno
 * @note    if is called for the last time, "free_chan_resources" will be called
 */
static void fwk_dma_chan_put(struct fwk_dma_chan *sptr_chan)
{
    /*!< No client, do not put again */
    if (!sptr_chan->client_count)
        return;

    if ((--sptr_chan->client_count) == 0)
    {
        struct fwk_dma_device *sptr_mdev = sptr_chan->sptr_device;

        /*!< Only alloc chan for the first time  */
        if (sptr_mdev->free_chan_resources)
            sptr_mdev->free_chan_resources(sptr_chan);
    }
}

/*!
 * @brief   Find avaliable channel by dma_filter_fn
 * @param   sptr_mdev: dma controller
 * @param   dma_filter_fn: check channel is avaliable
 * @param   filter_param: private data for dma_filter_fn
 * @retval  sptr_chan
 * @note    none
 */
struct fwk_dma_chan *fwk_dma_find_candidate(struct fwk_dma_device *sptr_mdev, 
                                kbool_t (*dma_filter_fn)(struct fwk_dma_chan *sptr_chan, void *filter_param), void *filter_param)
{
    struct fwk_dma_chan *sptr_chan;

    if (!sptr_mdev)
        return ERR_PTR(-ER_NODEV);

    foreach_list_next_entry(sptr_chan, &sptr_mdev->sgtc_channels, sgtc_link)
    {
        /*!< Busy */
        if (sptr_chan->client_count)
            continue;

        if (dma_filter_fn && dma_filter_fn(sptr_chan, filter_param))
        {
            fwk_dma_chan_get(sptr_chan);
            return sptr_chan;
        }
    }

    return mr_nullptr;
}

/*!
 * @brief   Request DMA channel
 * @param   sptr_dev: &sptr_pdev->sgtc_dev (to get sptr_dev->sptr_node)
 * @param   con_id: device-node property name (such as "tx", "rx")
 * @retval  channel requested
 * @note    none
 */
struct fwk_dma_chan *fwk_dma_request_chan(struct fwk_device *sptr_dev, const kchar_t *con_id)
{
    struct fwk_dma_chan *sptr_chan;

    sptr_chan = fwk_of_dma_request_chan(sptr_dev, con_id);
    if (!isValid(sptr_chan))
        return ERR_PTR(-ER_FAILD);

    return sptr_chan;
}

/*!
 * @brief   Release DMA channel
 * @param   sptr_chan: current channel
 * @retval  none
 * @note    none
 */
void fwk_dma_release_chan(struct fwk_dma_chan *sptr_chan)
{
    if (sptr_chan && sptr_chan->client_count)
    {
        fwk_dma_chan_put(sptr_chan);
        sptr_chan->private_data = mr_nullptr;
    }
}

/*!
 * @brief   allocate DMA Buffer
 * @param   sptr_dev: It can be NULL
 * @param   size: size of buffer (bytes)
 * @param   cpu_addr: virtual address
 * @param   dma_handle: the physical address of the buffer allocated
 * @param   mask: GFP_WAIT/GFP_KERNEL/GFP_ATOMIC, ...
 * @retval  the virtual address of the buffer allocated
 * @note    none
 */
void *fwk_dma_alloc_coherent(struct fwk_device *sptr_dev, kusize_t size, kuaddr_t *dma_handle, nrt_gfp_t mask)
{
    void *virt;

    virt = kmalloc(size, GFP_GET_FLAG(mask) | GFP_DMA);
    if (!isValid(virt))
        return ERR_PTR(-ER_NOMEM);

    if (dma_handle)
        *dma_handle = (kuaddr_t)fwk_virt_to_phys(virt);

    return virt;
}

/*!
 * @brief   release DMA Buffer
 * @param   dma_buffer: It can be NULL
 * @param   cpu_addr: virtual address
 * @param   dma_handle: the physical address of the buffer allocated
 * @retval  none
 * @note    none
 */
void fwk_dma_free_coherent(void *dma_buffer, kusize_t size, void *cpu_addr, kuaddr_t *dma_handle)
{
    if (cpu_addr)
        kfree(cpu_addr);
}

/*!
 * @brief   Configure DMA channel
 * @param   sptr_chan: current channel
 * @param   sptr_config: configs
 * @retval  errno
 * @note    none
 */
kint32_t fwk_dma_config(struct fwk_dma_chan *sptr_chan, struct fwk_dma_slave_config *sptr_config)
{
    if (sptr_chan->sptr_device->config)
        return sptr_chan->sptr_device->config(sptr_chan, sptr_config);

    return -ER_INVALID;
}

/*!
 * @brief   Prepare transfer
 * @param   sptr_chan: DMA channel
 * @param   sptr_src: Source memory address (for "DMA_DEVICE_TO_MEM", it can be set to NULL)
 * @param   src_count: The number of source memory blocks (for "DMA_DEVICE_TO_MEM", it can be set to 0)
 * @param   sptr_dst: Destination memory address (for "DMA_MEM_TO_DEVICE", it can be set to NULL)
 * @param   dst_count: The number of destination memory blocks (for "DMA_MEM_TO_DEVICE", it can be set to 0)
 * @param   direction: Transfer direction, it must be DMA_MEM_TO_MEM, DMA_MEM_TO_DEVICE, or DMA_DEVICE_TO_MEM
 * @param   flag: Transfer flag
 * @param   context: Private context data for dma controller
 * @retval  Allocated transfer_desc
 */
struct fwk_dma_transfer_desc *fwk_dma_transfer_prepare(struct fwk_dma_chan *sptr_chan, 
                                    struct fwk_dma_block_data *sptr_src, kusize_t src_count, 
                                    struct fwk_dma_block_data *sptr_dst, kusize_t dst_count, 
                                    kuint32_t direction, kuint32_t flags, void *context)
{
    if (sptr_chan->sptr_device->transfer_prepare)
        return sptr_chan->sptr_device->transfer_prepare(sptr_chan, 
                        sptr_src, src_count, sptr_dst, dst_count, direction, flags, context);

    return mr_nullptr;
}

/*!
 * @brief   Stop DMA channel
 * @param   sptr_chan: current channel
 * @retval  errno
 * @note    none
 */
kint32_t fwk_dma_terminate_all(struct fwk_dma_chan *sptr_chan)
{
    if (sptr_chan->sptr_device->terminate_all)
        return sptr_chan->sptr_device->terminate_all(sptr_chan);

    return -ER_INVALID;
}

/*!
 * @brief   Quit DMA channel
 * @param   sptr_chan: current channel
 * @retval  errno
 * @note    none
 */
kint32_t fwk_dma_pause(struct fwk_dma_chan *sptr_chan)
{
    if (sptr_chan->sptr_device->pause)
        return sptr_chan->sptr_device->pause(sptr_chan);

    return -ER_INVALID;
}

/*!
 * @brief   Resume DMA channel
 * @param   sptr_chan: current channel
 * @retval  errno
 * @note    none
 */
kint32_t fwk_dma_resume(struct fwk_dma_chan *sptr_chan)
{
    if (sptr_chan->sptr_device->resume)
        return sptr_chan->sptr_device->resume(sptr_chan);

    return -ER_INVALID;
}

/*!
 * @brief   Submit transfer desc to DMA channel
 * @param   sptr_desc: transfer desc
 * @retval  errno
 * @note    none
 */
kint32_t fwk_dma_submit(struct fwk_dma_transfer_desc *sptr_desc)
{
    if (!sptr_desc || !sptr_desc->submit)
        return -ER_NODEV;

    return sptr_desc->submit(sptr_desc);
}

/*!
 * @brief   Take out transfer desc to excute
 * @param   sptr_chan: current channel
 * @retval  none
 * @note    none
 */
void fwk_dma_issue_pending(struct fwk_dma_chan *sptr_chan)
{
    if (sptr_chan->sptr_device->issue_pending)
        sptr_chan->sptr_device->issue_pending(sptr_chan);
}

/*!< --------------------------------------------------------------------------- */
/*!
 * @brief   dma init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __plat_init fwk_dma_global_init(void)
{
    mutex_init(&sgtc_fwk_dma_mutex);
    return ER_NORMAL;
}

/*!
 * @brief   dma exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __plat_exit fwk_dma_global_exit(void)
{

}

IMPORT_LATE_INIT(fwk_dma_global_init);
IMPORT_LATE_EXIT(fwk_dma_global_exit);

/* end of file */
