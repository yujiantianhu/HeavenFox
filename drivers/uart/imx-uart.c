/*
 * imx6ul uart controller
 *
 * File Name:   imx-uart.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.05.05
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/base/fwk_platdrv.h>
#include <platform/clk/fwk_clk.h>
#include <platform/dma/fwk_dma.h>
#include <platform/base/fwk_cdev.h>
#include <platform/base/fwk_chrdev.h>
#include <platform/base/fwk_inode.h>
#include <platform/base/fwk_fs.h>
#include <platform/base/fwk_fcntl.h>
#include <kernel/spinlock.h>
#include <kernel/wait.h>

#include <imx6/imx6ull_uart.h>

/*!< The defines */
enum __ERT_IMX_UART_DRV_FLAG 
{
    NR_IMX_UART_DRV_CONSOLE = mr_bit(0U),
    NR_IMX_UART_DRV_RXDMA = mr_bit(1U),
    NR_IMX_UART_DRV_TXDMA = mr_bit(2U),
    NR_IMX_UART_DRV_IRQEN = mr_bit(3U),
};

struct imx_uart_drv_data 
{
    srt_imx_uart_t *sptr_uart;
    kint32_t irq;
    kint32_t id;

    struct fwk_clk *sptr_ipgclk;
    struct fwk_clk *sptr_perclk;

    struct fwk_dma_chan *sptr_rxchan;
    struct fwk_dma_chan *sptr_txchan;
    struct fwk_dma_slave_config sgtc_rxcfg;
    struct fwk_dma_slave_config sgtc_txcfg;

    struct pq_queue *sptr_ring;

    kuint32_t devnum;
    struct fwk_cdev *sptr_cdev;
    struct fwk_device *sptr_idev;

    struct wait_queue_head sgtc_txwqh;
    struct wait_queue_head sgtc_rxwqh;
    kuint32_t flags;
};

#define IMX_UART_DRIVER_MAJOR                                   (NR_UART_MAJOR)
#define IMX_UART_DMA_RXBD_SIZE                                  (128)
#define IMX_UART_DMA_TXBD_SIZE                                  (128)
#define IMX_UART_RX_WATERMARK                                   (16)
#define IMX_UART_TX_WATERMARK                                   (16)

/*!< device id for device-tree */
static const struct fwk_of_device_id sgtc_imx_uart_driver_id[] =
{
    { .compatible = "fsl,imx6ul-uart", },
    {},
};

/*!< API functions */
/*!
 * @brief   Transfer Interrupt handler
 * @param   args: sptr_data
 * @retval  irq code
 * @note    none
 */
static irq_return_t imx_uart_isr(kint32_t irq, void *args)
{
    struct imx_uart_drv_data *sptr_data;
    srt_imx_uart_t *sptr_uart;

    sptr_data = (struct imx_uart_drv_data *)args;
    sptr_uart = sptr_data->sptr_uart;

    if (mr_isBitSetl(mr_bit(0), &sptr_uart->USR2)) 
    {

    }

    return NR_IRQ_HANDLED;
}

/*!
 * @brief   Rx DMA transfer complete callback
 * @param   callback_param: sptr_bdata
 * @retval  none
 * @note    none
 */
static void imx_uart_tx_complete(void *callback_param)
{
    struct fwk_dma_block_data *sptr_bdata, *sptr_cur;

    sptr_bdata = (struct fwk_dma_block_data *)callback_param;

    while (sptr_bdata) 
    {
        sptr_cur = sptr_bdata;
        sptr_bdata = sptr_bdata->sptr_next;

        fwk_dma_free_coherent(mr_nullptr, sptr_cur->length, sptr_cur->cpu_address, &sptr_cur->dma_address);
        kfree(sptr_cur);
    }
}

/*!
 * @brief   Uart hardware initialization
 * @param   sptr_data: driver data
 * @retval  none
 * @note    none
 */
static void imx_uart_initial(struct imx_uart_drv_data *sptr_data)
{
    srt_imx_uart_t *sptr_uart = sptr_data->sptr_uart;

    /*!< Disable Uart, and clear all settings (including automatic baud rate detection) */
    mr_writel(0, &sptr_uart->UCR1);

    /*!< Reset the Tx and Rx state machines, all FIFOs and USR1/2, UBIR, UBMR, UBRC, URXD, UTXD, UTS */
    mr_clrbitl(NR_IMX_UART_UCR2_SRST, &sptr_uart->UCR2);
    /*!< Wating for reseting finished */
//	while (!mr_isBitResetl(NR_IMX_UART_UCR2_SRST, &sptr_Uart->UCR2));

    /*!<
     * bit14: Ignore RTS Pin
     * bit8: Disable parity generator and checker (Not Odd/Even)
     * bit6: The transmitter sends 1 stop bit. The receiver expects 1 or more stop bits
     * bit5: 8-bit transmit and receive character length (not including START, STOP or PARITY bits)
     * bit2: TXEN. Enable the transmitter
     * bit1: RXEN. Enable the receiver
     */
    mr_clrbitl(NR_IMX_UART_UCR2_PREN  | NR_IMX_UART_UCR2_STPB, &sptr_uart->UCR2);
    mr_setbitl(NR_IMX_UART_UCR2_IRTS | NR_IMX_UART_UCR2_WS | 
               NR_IMX_UART_UCR2_TXEN | NR_IMX_UART_UCR2_RXEN, &sptr_uart->UCR2);

    /*!<
     * RXD Muxed Input Selected
     * 	<In I.MX6ULL, UARTs are used in MUXED mode, so that this bit should always be set>
     */
    mr_setbitl(NR_IMX_UART_UCR3_RXDMUXSEL, &sptr_uart->UCR3); 

    /*!< Clear Others */
    mr_resetl(&sptr_uart->UTIM);
    mr_resetl(&sptr_uart->ONEMS);
    mr_resetl(&sptr_uart->UMCR);

    /*!<
     * Set Baud Rate
     * Baud Rate = Ref_Freq / (16 * (UBMR + 1) / (UBIR + 1)), because uart_clk = 80MHz:
     * if Baud Rate is set to be 115200bps
     * ===> UFCR bit[9:7] = 101 (Divide input clock by 1), So Ref_Freq = uart_clk / 1 = 80MHz
     * ===> UBMR = 3124
     * ===> UBIR = 71
     * Baud Rate = 80000000 / (16 * (3124 + 1) / (71 + 1)) = (80000000 * 72) / (16 * 3125) = 115200
     */
    mr_setbitl(IMX_UART_UFCR_RFDIV_1, &sptr_uart->UFCR);
    mr_writel(IMX_UART_UBIR_INC(71), &sptr_uart->UBIR);
    mr_writel(IMX_UART_UBMR_INC(3124), &sptr_uart->UBMR);

    /*!< FIFO: Watermark */
    mr_clrbitl(NR_IMX_UART_UFCR_RXTL | NR_IMX_UART_UFCR_TXTL, &sptr_uart->UFCR);
    mr_setbitl(IMX_UART_UFCR_RXTL_RxFIFO(IMX_UART_TX_WATERMARK), &sptr_uart->UFCR);
    mr_setbitl(IMX_UART_UFCR_TXTL_TxFIFO(IMX_UART_RX_WATERMARK),  &sptr_uart->UFCR);

    /*!<
     * Clear status
     * bit6: Tx FIFO Empty, 0 (not empty), 1 (empty)
     * bit5: Rx FIFO Empty, 0 (not empty), 1 (empty)
     */
    mr_writel(NR_IMX_UART_UTS_RXEMPTY | NR_IMX_UART_UTS_TXEMPTY, &sptr_uart->UTS);

    /*!< Enable Uart */
    mr_setbitl(NR_IMX_UART_UCR1_UARTEN, &sptr_uart->UCR1);

    /*!< Enable Uart with DMA */
    if (sptr_data->flags & NR_IMX_UART_DRV_TXDMA)
        mr_setbitl(NR_IMX_UART_UCR1_TXDMAEN, &sptr_uart->UCR1);
    if (sptr_data->flags & NR_IMX_UART_DRV_RXDMA)
        mr_setbitl(NR_IMX_UART_UCR1_RXDMAEN, &sptr_uart->UCR1);
}

/*!
 * @brief   imx_uart_driver_open
 * @param   sptr_inode, sptr_file
 * @retval  errno
 * @note    none
 */
static kint32_t imx_uart_driver_open(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    struct imx_uart_drv_data *sptr_data;

    sptr_data = sptr_inode->sptr_cdev->privData;
    sptr_file->private_data = sptr_data;

    fwk_clk_enable(sptr_data->sptr_ipgclk);
    fwk_clk_enable(sptr_data->sptr_perclk);

    imx_uart_initial(sptr_data);
    return 0;
}

/*!
 * @brief   imx_uart_driver_close
 * @param   sptr_inode, sptr_file
 * @retval  errno
 * @note    none
 */
static kint32_t imx_uart_driver_close(struct fwk_inode *sptr_inode, struct fwk_file *sptr_file)
{
    struct imx_uart_drv_data *sptr_data;

    sptr_data = (struct imx_uart_drv_data *)sptr_file->private_data;
    sptr_file->private_data = mr_nullptr;

    /*!< Serial console should not be closed */
    if (!(sptr_data->flags & NR_IMX_UART_DRV_CONSOLE)) 
    {
        srt_imx_uart_t *sptr_uart = sptr_data->sptr_uart;

        /*!< Disable Uart, and clear all settings (including automatic baud rate detection) */
        mr_writel(0, &sptr_uart->UCR1);

        /*!< Disable Uart with DMA */
        if (sptr_data->flags & NR_IMX_UART_DRV_TXDMA)
            mr_clrbitl(NR_IMX_UART_UCR1_TXDMAEN, &sptr_uart->UCR1);
        if (sptr_data->flags & NR_IMX_UART_DRV_RXDMA)
            mr_clrbitl(NR_IMX_UART_UCR1_RXDMAEN, &sptr_uart->UCR1);

        fwk_clk_disable(sptr_data->sptr_ipgclk);
        fwk_clk_disable(sptr_data->sptr_perclk);
    }

    return 0;
}

/*!
 * @brief   imx_uart_driver_write
 * @param   sptr_file, ptrBuffer, size
 * @retval  errno
 * @note    none
 */
static kssize_t imx_uart_driver_write(struct fwk_file *sptr_file, const kbuffer_t *ptrBuffer, kssize_t size)
{
    struct imx_uart_drv_data *sptr_data;
    
    sptr_data = (struct imx_uart_drv_data *)sptr_file->private_data;
    
    if (!(sptr_data->flags & NR_IMX_UART_DRV_TXDMA)) 
    {
        srt_imx_uart_t *sptr_uart = sptr_data->sptr_uart;
        kchar_t msgs[1024];
        kusize_t real_size = CMP_MIN2(size, sizeof(msgs));

        fwk_copy_from_user(msgs, ptrBuffer, real_size);

        /*!< Wait for last fifo send finished */
        if (!mr_imx_uart_tx_empty(sptr_uart))
            wait_event(&sptr_data->sgtc_txwqh, mr_imx_uart_tx_empty(sptr_uart));

        for (kusize_t i = 0; i < real_size; i++) 
        {
            /*!< If TxFIFO is full, waitting for a while */
            while (mr_imx_uart_tx_full(sptr_uart))
                mr_delay_nop();

            /*!< Send Data */
            mr_imx_uart_send_byte(sptr_uart, msgs[i]);
        }
    }
    else
    {
        struct fwk_dma_transfer_desc *sptr_txdesc;
        struct fwk_dma_block_data *sptr_bdata;
        kuaddr_t tx_phy;
        void *tx_buffer;

        sptr_bdata = kmalloc(sizeof(*sptr_bdata), GFP_ATOMIC);
        if (!isValid(sptr_bdata))
            return -ER_NOMEM;

        tx_buffer = fwk_dma_alloc_coherent(mr_nullptr, size, &tx_phy, GFP_ATOMIC | GFP_ZERO);
        if (!isValid(tx_buffer)) 
        {
            kfree(sptr_bdata);
            return -ER_NOMEM;
        }

        fwk_dma_block_data_init(sptr_bdata, tx_buffer, tx_phy, 0, size);
        fwk_copy_from_user(tx_buffer, ptrBuffer, size);

        sptr_txdesc = fwk_dma_transfer_prepare(sptr_data->sptr_txchan,
                                            sptr_bdata, 1, mr_nullptr, 0, DMA_MEM_TO_DEVICE, 0, mr_nullptr);
        if (!isValid(sptr_txdesc)) 
        {
            fwk_dma_free_coherent(mr_nullptr, size, tx_buffer, &tx_phy);
            kfree(sptr_bdata);
            return -ER_NOMEM;
        }

        sptr_txdesc->transfer_callback = imx_uart_tx_complete;
        sptr_txdesc->callback_param = sptr_bdata;

        fwk_dma_submit(sptr_txdesc);
        fwk_dma_issue_pending(sptr_data->sptr_txchan);
    }

    return size;
}

/*!
 * @brief   imx_uart_driver_read
 * @param   sptr_file, ptrBuffer, size
 * @retval  errno
 * @note    none
 */
static kssize_t imx_uart_driver_read(struct fwk_file *sptr_file, kbuffer_t *ptrBuffer, kssize_t size)
{
    struct imx_uart_drv_data *sptr_data;
    srt_imx_uart_t *sptr_uart;
    kchar_t data[128], *buffer = mr_nullptr;
    kchar_t *msgs;
    kuint32_t count = 0;
    
    sptr_data = (struct imx_uart_drv_data *)sptr_file->private_data;
    sptr_uart = sptr_data->sptr_uart;

    if (mr_imx_uart_rx_empty(sptr_uart))
    {
        if (sptr_file->mode & O_NONBLOCK)
            return -ER_EMPTY;

        wait_event(&sptr_data->sgtc_rxwqh, !mr_imx_uart_rx_empty(sptr_uart));
    }

    if (size < 128)
        msgs = data;
    else
    {
        buffer = kzalloc(size, GFP_KERNEL);
        if (!isValid(buffer))
            return -ER_NOMEM;

        msgs = buffer;
    }

    while (!mr_imx_uart_rx_empty(sptr_uart))
    {
        msgs[count++] = mr_imx_uart_recv_byte(sptr_uart);
        if (count >= size)
            break;

        delay_ms(5);
    }

    fwk_copy_to_user(ptrBuffer, msgs, count);

    if (buffer)
        kfree(buffer);

    return count;
}

/*!< imx_uart driver operation */
const struct fwk_file_oprts sgtc_imx_uart_driver_oprts =
{
    .open	= imx_uart_driver_open,
    .close	= imx_uart_driver_close,
    .write	= imx_uart_driver_write,
    .read	= imx_uart_driver_read,
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   imx_uart_driver_probe
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_uart_driver_probe_dt(struct fwk_platdev *sptr_pdev, struct imx_uart_drv_data *sptr_data)
{
    struct fwk_dma_chan *sptr_chan;
    void *base;
    kusize_t address_length;

    sptr_data->id = (sptr_pdev->id < 0) ? fwk_of_get_alias_id(sptr_pdev->sgtc_dev.sptr_node) : sptr_pdev->id;
    base = (void *)fwk_platform_get_address(sptr_pdev, 0);
    if (!isValid(base))
        return -ER_NOMEM;

    address_length = fwk_platform_get_address_size(sptr_pdev, 0);
    sptr_data->sptr_uart = (srt_imx_uart_t *)fwk_io_remap(base, address_length);
    if (!isValid(sptr_data->sptr_uart))
        return -ER_NOMEM;

    sptr_data->irq = fwk_platform_get_irq(sptr_pdev, 0);
    if (sptr_data->irq < 0)
        goto fail1;

    /*!< Clock */
    sptr_data->sptr_ipgclk = fwk_clk_get(&sptr_pdev->sgtc_dev, "ipg");
    if (!isValid(sptr_data->sptr_ipgclk))
        goto fail1;

    sptr_data->sptr_perclk = fwk_clk_get(&sptr_pdev->sgtc_dev, "per");
    if (!isValid(sptr_data->sptr_perclk))
        goto fail2;

    /*!< DMA */
    sptr_chan = fwk_dma_request_chan(&sptr_pdev->sgtc_dev, "rx");
    if (isValid(sptr_chan))
        sptr_data->sptr_rxchan = sptr_chan;

    sptr_chan = fwk_dma_request_chan(&sptr_pdev->sgtc_dev, "tx");
    if (isValid(sptr_chan))
        sptr_data->sptr_txchan = sptr_chan;

    return ER_NORMAL;

fail2:
    fwk_clk_put(sptr_data->sptr_ipgclk);
fail1:
    fwk_io_unmap(sptr_data->sptr_uart);
    return -ER_FAILD;
}

/*!
 * @brief   remove dt
 * @param   sptr_data
 * @retval  none
 * @note    none
 */
static void imx_uart_driver_remove_dt(struct imx_uart_drv_data *sptr_data)
{
    if (sptr_data->sptr_rxchan)
        fwk_dma_release_chan(sptr_data->sptr_rxchan);
    if (sptr_data->sptr_txchan)
        fwk_dma_release_chan(sptr_data->sptr_txchan);
    
    fwk_clk_put(sptr_data->sptr_ipgclk);
    fwk_clk_put(sptr_data->sptr_perclk);
    fwk_io_unmap(sptr_data->sptr_uart);

    sptr_data->sptr_uart   = mr_nullptr;
    sptr_data->irq = -1;
    sptr_data->sptr_ipgclk = mr_nullptr;
    sptr_data->sptr_perclk = mr_nullptr;
    sptr_data->sptr_rxchan = mr_nullptr;
    sptr_data->sptr_txchan = mr_nullptr;
}

/*!
 * @brief   imx_uart_driver_probe
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_uart_driver_register(struct imx_uart_drv_data *sptr_data)
{
    struct fwk_cdev *sptr_cdev;
    struct fwk_device *sptr_idev;
    kuint32_t devnum;
    kint32_t retval;

    devnum = MKE_DEV_NUM(IMX_UART_DRIVER_MAJOR, sptr_data->id);
    retval = fwk_register_chrdev(devnum, 1, "imx-uart");
    if (retval < 0)
        return -ER_FAILD;

    sptr_cdev = fwk_cdev_alloc(&sgtc_imx_uart_driver_oprts);
    if (!isValid(sptr_cdev))
        goto fail1;

    retval = fwk_cdev_add(sptr_cdev, devnum, 1);
    if (retval < 0)
        goto fail2;

    sptr_idev = fwk_device_create(NR_TYPE_CHRDEV, devnum, "uart%d", sptr_data->id);
    if (!isValid(sptr_idev))
        goto fail3;

    sptr_data->devnum = devnum;
    sptr_data->sptr_cdev = sptr_cdev;
    sptr_cdev->privData = sptr_data;
    sptr_data->sptr_idev = sptr_idev;

    return ER_NORMAL;

fail3:
    fwk_cdev_del(sptr_cdev);
fail2:
    kfree(sptr_cdev);
fail1:
    fwk_unregister_chrdev(devnum, 1);
    return -ER_FAILD;
}

/*!
 * @brief   unregister chardev
 * @param   sptr_data
 * @retval  none
 * @note    none
 */
static void imx_uart_driver_unregister(struct imx_uart_drv_data *sptr_data)
{
    fwk_device_destroy(sptr_data->sptr_idev);
    fwk_cdev_del(sptr_data->sptr_cdev);
    kfree(sptr_data->sptr_cdev);
    fwk_unregister_chrdev(sptr_data->devnum, 1);

    sptr_data->sptr_cdev = mr_nullptr;
    sptr_data->sptr_idev = mr_nullptr;
    sptr_data->devnum = 0;
}

/*!
 * @brief   imx_uart_driver_probe
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_uart_driver_probe(struct fwk_platdev *sptr_pdev)
{
    struct imx_uart_drv_data *sptr_data;
    kchar_t *dev_name;

    sptr_data = kzalloc(sizeof(*sptr_data), GFP_KERNEL);
    if (!isValid(sptr_data))
        return -ER_NOMEM;

    /*!< parse device-tree */
    if (imx_uart_driver_probe_dt(sptr_pdev, sptr_data))
        goto fail1;

    if (imx_uart_driver_register(sptr_data))
        goto fail2;

    /*!< Offset to device name (such as "/dev/uart0" ---> "uart0") */
    dev_name = kstrnchr(CONFIG_CONSOLE_DEVICE, '/', -1);
    if (!dev_name)
        dev_name = (kchar_t *)CONFIG_CONSOLE_DEVICE;
    else 
    {
        /*!< Jump over '/' */
        dev_name += 1;
    }

    /*!< If not serial console */
    if (!kstrcmp(dev_name, mr_dev_get_name(sptr_data->sptr_idev)))
        sptr_data->flags |= NR_IMX_UART_DRV_CONSOLE;

    /*!< Enable clock */
    fwk_clk_enable(sptr_data->sptr_ipgclk);
    fwk_clk_enable(sptr_data->sptr_perclk);

    if (sptr_data->sptr_rxchan) 
    {
        sptr_data->sgtc_rxcfg.direction = DMA_DEVICE_TO_MEM;
        sptr_data->sgtc_rxcfg.src_addr = (kuaddr_t)&sptr_data->sptr_uart->URXD;
        sptr_data->sgtc_rxcfg.src_addr_width = NR_DMA_TRX_WIDTH_1BYTE;

        if (!fwk_dma_config(sptr_data->sptr_rxchan, &sptr_data->sgtc_rxcfg))
            sptr_data->flags |= NR_IMX_UART_DRV_RXDMA;
    }

    if (sptr_data->sptr_txchan) 
    {
        sptr_data->sgtc_txcfg.direction = DMA_MEM_TO_DEVICE;
        sptr_data->sgtc_txcfg.dst_addr = (kuaddr_t)&sptr_data->sptr_uart->UTXD;
        sptr_data->sgtc_txcfg.dst_addr_width = NR_DMA_TRX_WIDTH_1BYTE;
        
        if (!fwk_dma_config(sptr_data->sptr_txchan, &sptr_data->sgtc_txcfg))
            sptr_data->flags |= NR_IMX_UART_DRV_TXDMA;
    }

    if (!(sptr_data->flags & NR_IMX_UART_DRV_CONSOLE)) 
    {
        fwk_clk_disable(sptr_data->sptr_ipgclk);
        fwk_clk_disable(sptr_data->sptr_perclk);
    }

#if (!defined(CONFIG_CONSOLE_RXDMA) || !CONFIG_CONSOLE_RXDMA)
    if (sptr_data->flags & NR_IMX_UART_DRV_CONSOLE)
        sptr_data->flags &= ~NR_IMX_UART_DRV_RXDMA;
#endif

#if (!defined(CONFIG_CONSOLE_TXDMA) || !CONFIG_CONSOLE_TXDMA)
    if (sptr_data->flags & NR_IMX_UART_DRV_CONSOLE)
        sptr_data->flags &= ~NR_IMX_UART_DRV_TXDMA;
#endif

    if (!(sptr_data->flags & NR_IMX_UART_DRV_RXDMA))
    {
        if (fwk_request_irq(sptr_data->irq, imx_uart_isr, 0, mr_dev_get_name(sptr_data->sptr_idev), sptr_data))
            goto fail2;

        fwk_disable_irq(sptr_data->irq);
    }

    init_waitqueue_head(&sptr_data->sgtc_txwqh);
    init_waitqueue_head(&sptr_data->sgtc_rxwqh);
    fwk_platform_set_drvdata(sptr_pdev, sptr_data);
    
    return ER_NORMAL;

fail2:
    imx_uart_driver_remove_dt(sptr_data);
fail1:
    kfree(sptr_data);
    return -ER_FAILD;
}

/*!
 * @brief   imx_uart_driver_remove
 * @param   sptr_dev
 * @retval  errno
 * @note    none
 */
static kint32_t imx_uart_driver_remove(struct fwk_platdev *sptr_pdev)
{
    struct imx_uart_drv_data *sptr_data;

    sptr_data = fwk_platform_get_drvdata(sptr_pdev);

    if (!(sptr_data->flags & NR_IMX_UART_DRV_CONSOLE)) 
    {
        fwk_clk_disable(sptr_data->sptr_ipgclk);
        fwk_clk_disable(sptr_data->sptr_perclk);
    }

    if (!(sptr_data->flags & NR_IMX_UART_DRV_RXDMA)) 
    {
        fwk_disable_irq(sptr_data->irq);
        fwk_free_irq(sptr_data->irq, sptr_data);
    }

    imx_uart_driver_unregister(sptr_data);
    imx_uart_driver_remove_dt(sptr_data);
    kfree(sptr_data);
    fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);

    return ER_NORMAL;
}

/*!< platform instance */
static struct fwk_platdrv sgtc_imx_sdma_platdrv =
{
    .probe	= imx_uart_driver_probe,
    .remove	= imx_uart_driver_remove,
    
    .sgtc_driver =
    {
        .name 	= "fsl,uart",
        .id 	= -1,
        .sptr_of_match_table = sgtc_imx_uart_driver_id,
    },
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   imx_uart_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init imx_uart_driver_init(void)
{
    return fwk_register_platdriver(&sgtc_imx_sdma_platdrv);
}

/*!
 * @brief   imx_uart_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit imx_uart_driver_exit(void)
{
    fwk_unregister_platdriver(&sgtc_imx_sdma_platdrv);
}

IMPORT_PATTERN_INIT(imx_uart_driver_init);
IMPORT_PATTERN_EXIT(imx_uart_driver_exit);

/*!< end of file */
