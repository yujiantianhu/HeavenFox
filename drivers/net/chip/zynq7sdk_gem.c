/*
 * ZYNQ eMAC Of PS Driver
 *
 * File Name:   zynq7sdk_gem.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.11.30
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/of/fwk_of.h>
#include <platform/of/fwk_of_device.h>
#include <platform/base/fwk_platdev.h>
#include <platform/base/fwk_platdrv.h>
#include <platform/base/fwk_uaccess.h>

#include <kernel/sched.h>
#include <kernel/sleep.h>

#include <platform/net/fwk_if.h>
#include <platform/net/fwk_netdev.h>
#include <platform/net/fwk_ip.h>
#include <platform/net/fwk_skbuff.h>
#include <platform/net/fwk_ether.h>

#include <zynq7/zynq7_periph.h>
#include <zynq7/xemac/xemacpsif.h>
#include <zynq7/xemac/xemacps.h>
#include <zynq7/xemac/xemac_ieee_reg.h>

/*!< The defines */
typedef enum {
    NR_ETH_LINK_UNDEFINED = 0,
    NR_ETH_LINK_UP,
    NR_ETH_LINK_DOWN,
    NR_ETH_LINK_NEGOTIATING

} nrt_link_status_t;

#define XEMACPS_IS_ETH_0(x)                     ((x)->Config.BaseAddress == XPAR_PS7_ETHERNET_0_BASEADDR)

/*!< Bd Ring */
#define BD_ALIGNMENT                            (XEMACPS_DMABD_MINIMUM_ALIGNMENT << 1)

#define BD_SIZE                                 sizeof(XEmacPs_Bd)
#define XNET_TX_DESC_NUM                        (512)
#define XNET_RX_DESC_NUM                        (512)
#define XNET_TX_DESC_SIZE                       (XNET_TX_DESC_NUM * BD_SIZE)
#define XNET_RX_DESC_SIZE                       (XNET_RX_DESC_NUM * BD_SIZE)

#define XEMACPS_BD_TO_INDEX(sptr_ring, sptr_bd) \
        (((kuint32_t)(sptr_bd) - (kuint32_t)(sptr_ring)->BaseBdAddr) / (sptr_ring)->Separation)

struct xsdk_gem_phy {
    kint32_t link_speed;
    kuint32_t phymapemac[32];
    kuint32_t phyaddrforemac;

    nrt_link_status_t eth_link_status;
};

typedef enum __ERT_GEM_IRQ_STATUS
{
    NR_GEM_IRQ_TXE = 0,
    NR_GEM_IRQ_RXE = 1,
    NR_GEM_IRQ_NUM,

    NR_GEM_IRQ_RX_BIT = mr_bit(0),
    NR_GEM_IRQ_TX_BIT = mr_bit(1),
    NR_GEM_IRQ_TXE_BIT = mr_bit(2),
    NR_GEM_IRQ_RXE_BIT = mr_bit(3),
    NR_GEM_IRQ_ERR_BIT = NR_GEM_IRQ_TXE_BIT | NR_GEM_IRQ_RXE_BIT,

} nrt_gem_irq_stat_t;

struct xsdk_gem_drv_data {
    void *base;
    kint32_t irq;

    kuint8_t hwaddr[NET_MAC_ETH_ALEN];
    struct fwk_net_device *sptr_ndev;

    xemacpsif_s sgtc_xemacif;
    XEmacPs_Config sgtc_config;

    struct xsdk_gem_phy sgtc_phy;

    /*!< 4-bytes alignment */
    struct fwk_sk_buff *sptr_rxskbs[XNET_RX_DESC_NUM];
    struct fwk_sk_buff *sptr_txskbs[XNET_TX_DESC_NUM];

    XEmacPs_Bd *bd_rxterminate;
    XEmacPs_Bd *bd_txterminate;

    nrt_gem_irq_stat_t status;
    kuint32_t reg_value[NR_GEM_IRQ_NUM];
};

#define XSDK_GEM_DRIVER_NAME                    "gem0"

/*!< phy */
#define PHY_DETECT_REG                          1
#define PHY_IDENTIFIER_1_REG                    2
#define PHY_IDENTIFIER_2_REG                    3
#define PHY_DETECT_MASK                         0x1808
#define PHY_MARVELL_IDENTIFIER                  0x0141
#define PHY_TI_IDENTIFIER                       0x2000
#define PHY_REALTEK_IDENTIFIER                  0x001c
#define PHY_XILINX_PCS_PMA_ID1                  0x0174
#define PHY_XILINX_PCS_PMA_ID2                  0x0C00

#define PHY_REGCR                               0x0D
#define PHY_ADDAR                               0x0E
#define PHY_RGMIIDCTL                           0x86
#define PHY_RGMIICTL                            0x32
#define PHY_STS                                 0x11
#define PHY_TI_CR                               0x10
#define PHY_TI_CFG4                             0x31

#define PHY_REGCR_ADDR                          0x001F
#define PHY_REGCR_DATA                          0x401F
#define PHY_TI_CRVAL                            0x5048
#define PHY_TI_CFG4RESVDBIT7                    0x80

/*!< The globals */

/*!< The functions */

/*!< API function */
/*!
 * @brief   if phy is producted by ti, ...
 * @param   sptr_xemacps: emac structure pointer
 * @param   phy_addr: index of phy
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_get_ti_phy_speed(XEmacPs *sptr_xemacps, kuint32_t phy_addr)
{
    kuint16_t control;
    kuint16_t status;
    kuint16_t status_speed;
    kuint32_t timeout_counter = 0;
    kuint32_t phyregtemp;
    kint32_t RetStatus;

    print_info("Start PHY autonegotiation \r\n");

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, 0x1F, (kuint16_t *)&phyregtemp);
    phyregtemp |= 0x4000;
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, 0x1F, phyregtemp);

    RetStatus = XEmacPs_PhyRead(sptr_xemacps, phy_addr, 0x1F, (kuint16_t *)&phyregtemp);
    if (RetStatus) {
        print_err("Error during sw reset \n\r");
        return RetStatus;
    }

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, 0, (kuint16_t *)&phyregtemp);
    phyregtemp |= 0x8000;
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, 0, phyregtemp);

    /*!< Delay */
    msleep(20);

    RetStatus = XEmacPs_PhyRead(sptr_xemacps, phy_addr, 0, (kuint16_t *)&phyregtemp);
    if (RetStatus) {
        print_err("Error during reset \n\r");
        return RetStatus;
    }

    /*!< FIFO depth */
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_TI_CR, PHY_TI_CRVAL);
    RetStatus = XEmacPs_PhyRead(sptr_xemacps, phy_addr, PHY_TI_CR, (kuint16_t *)&phyregtemp);
    if (RetStatus) {
        print_err("Error writing to 0x10 \n\r");
        return RetStatus;
    }

    /*!< TX/RX tuning */
    /*!< Write to PHY_RGMIIDCTL */
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_REGCR, PHY_REGCR_ADDR);
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_ADDAR, PHY_RGMIIDCTL);
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_REGCR, PHY_REGCR_DATA);

    RetStatus = XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_ADDAR, 0xA8);
    if (RetStatus) {
        print_err("Error in tuning");
        return RetStatus;
    }

    /*!< Read PHY_RGMIIDCTL */
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_REGCR, PHY_REGCR_ADDR);
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_ADDAR, PHY_RGMIIDCTL);
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_REGCR, PHY_REGCR_DATA);
    
    RetStatus = XEmacPs_PhyRead(sptr_xemacps, phy_addr, PHY_ADDAR, (kuint16_t *)&phyregtemp);
    if (RetStatus) {
        print_err("Error in tuning");
        return RetStatus;
    }

    /*!< Write PHY_RGMIICTL */
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_REGCR, PHY_REGCR_ADDR);
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_ADDAR, PHY_RGMIICTL);
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_REGCR, PHY_REGCR_DATA);
    
    RetStatus = XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_ADDAR, 0xD3);
    if (RetStatus) {
        print_err("Error in tuning");
        return RetStatus;
    }

    /*!< Read PHY_RGMIICTL */
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_REGCR, PHY_REGCR_ADDR);
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_ADDAR, PHY_RGMIICTL);
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_REGCR, PHY_REGCR_DATA);
    
    RetStatus = XEmacPs_PhyRead(sptr_xemacps, phy_addr, PHY_ADDAR, (kuint16_t *)&phyregtemp);
    if (RetStatus) {
        print_err("Error in tuning");
        return RetStatus;
    }

    /*!< SW workaround for unstable link when RX_CTRL is not STRAP MODE 3 or 4 */
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_REGCR, PHY_REGCR_ADDR);
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_ADDAR, PHY_TI_CFG4);
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_REGCR, PHY_REGCR_DATA);

    RetStatus = XEmacPs_PhyRead(sptr_xemacps, phy_addr, PHY_ADDAR, (kuint16_t *)&phyregtemp);
    phyregtemp &= ~(PHY_TI_CFG4RESVDBIT7);
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_REGCR, PHY_REGCR_ADDR);
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_ADDAR, PHY_TI_CFG4);
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_REGCR, PHY_REGCR_DATA);
    RetStatus = XEmacPs_PhyWrite(sptr_xemacps, phy_addr, PHY_ADDAR, phyregtemp);

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &control);
    control |= IEEE_ASYMMETRIC_PAUSE_MASK;
    control |= IEEE_PAUSE_MASK;
    control |= ADVERTISE_100;
    control |= ADVERTISE_10;
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, control);

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, &control);
    control |= ADVERTISE_1000;
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, control);

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
    control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
    control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_STATUS_REG_OFFSET, &status);

    print_info("Waiting for PHY to complete autonegotiation.\r\n");

    while (!(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE)) {
        msleep(10);
        timeout_counter++;

        if (timeout_counter == 30) {
            print_err("Auto negotiation error \r\n");
            return -ER_TIMEOUT;
        }

        XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
    }

    print_info("autonegotiation complete \r\n");

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, PHY_STS, &status_speed);
    if ((status_speed & 0xC000) == 0x8000)
        return 1000;
    if ((status_speed & 0xC000) == 0x4000)
        return 100;

    return 10;
}

/*!
 * @brief   if phy is producted by realtek, ...
 * @param   sptr_xemacps: emac structure pointer
 * @param   phy_addr: index of phy
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_get_realtek_phy_speed(XEmacPs *sptr_xemacps, kuint32_t phy_addr)
{
    kuint16_t control;
    kuint16_t status;
    kuint16_t status_speed;
    kuint32_t timeout_counter = 0;
    kuint32_t temp_speed;

    print_info("Start PHY autonegotiation \r\n");

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &control);
    control |= IEEE_ASYMMETRIC_PAUSE_MASK;
    control |= IEEE_PAUSE_MASK;
    control |= ADVERTISE_100;
    control |= ADVERTISE_10;
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, control);

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, &control);
    control |= ADVERTISE_1000;
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, control);

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
    control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
    control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
    control |= IEEE_CTRL_RESET_MASK;
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

    while (1) {
        XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
        if (!(control & IEEE_CTRL_RESET_MASK))        
            break;
    }

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_STATUS_REG_OFFSET, &status);

    print_info("Waiting for PHY to complete autonegotiation.\r\n");

    while (!(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE)) {
        msleep(10);
        timeout_counter++;

        if (timeout_counter == 30) {
            print_err("Auto negotiation error \r\n");
            return -ER_TIMEOUT;
        }

        XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
    }
    print_info("autonegotiation complete \r\n");

    XEmacPs_PhyRead(sptr_xemacps, phy_addr,IEEE_SPECIFIC_STATUS_REG, &status_speed);
    if (status_speed & 0x400) {
        temp_speed = status_speed & IEEE_SPEED_MASK;

        if (temp_speed == IEEE_SPEED_1000)
            return 1000;
        if (temp_speed == IEEE_SPEED_100)
            return 100;

        return 10;
    }

    return -ER_ERROR;
}

/*!
 * @brief   if phy is producted by marvell, ...
 * @param   sptr_xemacps: emac structure pointer
 * @param   phy_addr: index of phy
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_get_marvell_phy_speed(XEmacPs *sptr_xemacps, kuint32_t phy_addr)
{
    kuint16_t temp;
    kuint16_t control;
    kuint16_t status;
    kuint16_t status_speed;
    kuint32_t timeout_counter = 0;
    kuint32_t temp_speed;

    print_info("Start PHY autonegotiation \r\n");

    XEmacPs_PhyWrite(sptr_xemacps,phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 2);
    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_CONTROL_REG_MAC, &control);
    control |= IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK;
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, IEEE_CONTROL_REG_MAC, control);

    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0);

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &control);
    control |= IEEE_ASYMMETRIC_PAUSE_MASK;
    control |= IEEE_PAUSE_MASK;
    control |= ADVERTISE_100;
    control |= ADVERTISE_10;
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, control);

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, &control);
    control |= ADVERTISE_1000;
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, control);

    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, IEEE_PAGE_ADDRESS_REGISTER, 0);
    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_COPPER_SPECIFIC_CONTROL_REG, &control);
    /*!< max number of gigabit attempts */
    control |= (7 << 12);
    /*!< enable downshift */
    control |= (1 << 11);
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, IEEE_COPPER_SPECIFIC_CONTROL_REG, control);
    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
    control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
    control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
    control |= IEEE_CTRL_RESET_MASK;
    XEmacPs_PhyWrite(sptr_xemacps, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

    while (1) {
        XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
        if (!(control & IEEE_CTRL_RESET_MASK))
            break;
    }

    XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_STATUS_REG_OFFSET, &status);

    print_info("Waiting for PHY to complete autonegotiation.\r\n");

    while (!(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE)) {
        msleep(10);
        XEmacPs_PhyRead(sptr_xemacps, 
                phy_addr, IEEE_COPPER_SPECIFIC_STATUS_REG_2, &temp);
        timeout_counter++;

        if (timeout_counter == 30) 
        {
            print_err("Auto negotiation error \r\n");
            return -ER_TIMEOUT;
        }

        XEmacPs_PhyRead(sptr_xemacps, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
    }
    print_info("autonegotiation complete \r\n");

    XEmacPs_PhyRead(sptr_xemacps, phy_addr,IEEE_SPECIFIC_STATUS_REG, &status_speed);
    if (status_speed & 0x400) {
        temp_speed = status_speed & IEEE_SPEED_MASK;

        if (temp_speed == IEEE_SPEED_1000)
            return 1000;
        if(temp_speed == IEEE_SPEED_100)
            return 100;
        
        return 10;
    }

    return -ER_ERROR;
}

/*!
 * @brief   identitify phy id and get speed
 * @param   sptr_xemacps: emac structure pointer
 * @param   phy_addr: index of phy
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_gem_phy_ieee_speed(XEmacPs *sptr_emacps, kuint32_t phy_addr)
{
    kuint16_t phy_identity;
    kint32_t RetStatus;

    XEmacPs_PhyRead(sptr_emacps, phy_addr, PHY_IDENTIFIER_1_REG, &phy_identity);

    if (phy_identity == PHY_TI_IDENTIFIER)
        RetStatus = xsdk_get_ti_phy_speed(sptr_emacps, phy_addr);
    else if (phy_identity == PHY_REALTEK_IDENTIFIER)
        RetStatus = xsdk_get_realtek_phy_speed(sptr_emacps, phy_addr);
    else
        RetStatus = xsdk_get_marvell_phy_speed(sptr_emacps, phy_addr);

    return RetStatus;
}

/*!
 * @brief   identitify phy id and get speed
 * @param   sptr_xemacps: emac structure pointer
 * @param   phy_addr: index of phy
 * @retval  errno
 * @note    none
 */
static void xsdk_gem_phy_detect(XEmacPs *sptr_emacps, kuint32_t *phymapemac)
{
    kuint16_t phy_reg;
    kuint32_t phy_addr;

    for (phy_addr = 31; phy_addr > 0; phy_addr--) {
        XEmacPs_PhyRead(sptr_emacps, phy_addr, PHY_DETECT_REG, &phy_reg);

        if ((phy_reg != 0xFFFF) &&
            ((phy_reg & PHY_DETECT_MASK) == PHY_DETECT_MASK)) {
            
            /*!< Found a valid PHY address */
            print_info("XEmacPs %s: PHY detected at address %d.\r\n", __FUNCTION__, phy_addr);

            phymapemac[phy_addr] = true;
            XEmacPs_PhyRead(sptr_emacps, phy_addr, PHY_IDENTIFIER_1_REG, &phy_reg);

            if ((phy_reg != PHY_MARVELL_IDENTIFIER) &&
                (phy_reg != PHY_TI_IDENTIFIER) &&
                (phy_reg != PHY_REALTEK_IDENTIFIER))
                print_warn("WARNING: Not a Marvell or TI or Realtek Ethernet PHY. Please verify the initialization sequence\r\n");
        }
    }
}

/*!
 * @brief   enable transfer
 * @param   sptr_xemacps: emac structure pointer
 * @retval  none
 * @note    none
 */
static void xsdk_gem_xmit_trigger(xemacpsif_s *sptr_xemacif)
{
    kuint32_t reg;

    /*!< Start transmit */
    reg  = XEmacPs_ReadReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_NWCTRL_OFFSET);
    reg |= XEMACPS_NWCTRL_STARTTX_MASK;
    XEmacPs_WriteReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_NWCTRL_OFFSET, reg);
}

/*!
 * @brief   initialize emac
 * @param   sptr_data: driver data structure pointer
 * @retval  none
 * @note    none
 */
static void xsdk_gem_emac_init(struct xsdk_gem_drv_data *sptr_data)
{
    struct fwk_net_device *sptr_ndev = sptr_data->sptr_ndev;
    XEmacPs *sptr_emacps;
    struct xsdk_gem_phy *sptr_phy;
    kint32_t status;
    kuint32_t i;
    kbool_t phyfoundforemac = false;
    kuint32_t link_speed;

    sptr_emacps = &sptr_data->sgtc_xemacif.sgtc_emacps;
    sptr_phy = &sptr_data->sgtc_phy;

    /*!< set mac address */
    status = XEmacPs_SetMacAddress(sptr_emacps, sptr_ndev->dev_addr, 1);
    if (status)
        print_warn("In %s: Emac Mac Address set failed...\r\n", __func__);

    XEmacPs_SetMdioDivisor(sptr_emacps, MDC_DIV_224);
    xsdk_gem_phy_detect(sptr_emacps, &sptr_phy->phymapemac[0]);

    for (i = 31; i > 0; i--) {
        if (sptr_phy->phymapemac[i] == true) {
            link_speed = xsdk_gem_phy_ieee_speed(sptr_emacps, i);
            sptr_phy->link_speed = XEmacPs_PhySetup(sptr_emacps, i, link_speed);
            phyfoundforemac = true;
            sptr_phy->phyaddrforemac = i;
        }
    }

    /*!< If no PHY was detected, use broadcast PHY address of 0 */
    if (phyfoundforemac == false) {
        link_speed = xsdk_gem_phy_ieee_speed(sptr_emacps, 0);
        sptr_phy->link_speed = XEmacPs_PhySetup(sptr_emacps, 0, link_speed);
    }

    if (sptr_phy->link_speed < 0) {
        sptr_phy->eth_link_status = NR_ETH_LINK_DOWN;
        print_err("Assert due to phy setup failure \n\r", __func__);
    } 
    else {
        sptr_phy->eth_link_status = NR_ETH_LINK_UP;
    }

    XEmacPs_SetOperatingSpeed(sptr_emacps, sptr_phy->link_speed);

    /*!< Setting the operating speed of the MAC needs a delay. */
    msleep(1);
}

/*!
 * @brief   clear and build tx-dma buffer again
 * @param   sptr_data: driver data structure pointer
 * @retval  none
 * @note    none
 */
static void xsdk_gem_dma_tx_descs_clean(struct xsdk_gem_drv_data *sptr_data)
{
    XEmacPs_Bd sgtc_bd;
    XEmacPs_BdRing *sptr_txbdring;
    xemacpsif_s *sptr_xemacif = &sptr_data->sgtc_xemacif;

    sptr_txbdring = &XEmacPs_GetTxRing(&sptr_xemacif->sgtc_emacps);

    XEmacPs_BdClear(&sgtc_bd);
    XEmacPs_BdSetStatus(&sgtc_bd, XEMACPS_TXBUF_USED_MASK);

    /*!< Create the TxBD ring */
    XEmacPs_BdRingInitial(sptr_txbdring, (kuint32_t)sptr_xemacif->tx_bdspace,
            (kuint32_t)sptr_xemacif->tx_bdspace, BD_ALIGNMENT, XNET_TX_DESC_NUM);
    XEmacPs_BdRingClone(sptr_txbdring, &sgtc_bd, XEMACPS_SEND);
}

/*!
 * @brief   get current number of freeBDs
 * @param   sptr_data: driver data structure pointer
 * @retval  count
 * @note    none
 */
static kint32_t xsdk_gem_get_tx_freecnt(struct xsdk_gem_drv_data *sptr_data)
{
    xemacpsif_s *sptr_xemacif = &sptr_data->sgtc_xemacif;
    XEmacPs_BdRing *sptr_txbdring;

    sptr_txbdring = &XEmacPs_GetTxRing(&sptr_xemacif->sgtc_emacps);

    /*!< tx space is available as long as there are valid BD's */
    return XEmacPs_BdRingGetFreeCnt(sptr_txbdring);
}

/*!
 * @brief   build rx-dma buffer BDs
 * @param   sptr_data: driver data structure pointer
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_gem_setup_rxbd(struct xsdk_gem_drv_data *sptr_data, nrt_gfp_t mask)
{
    xemacpsif_s *sptr_xemacif = &sptr_data->sgtc_xemacif;
    XEmacPs_BdRing *sptr_rxring;
    struct fwk_sk_buff *sptr_skb;
    void *buffer;
    kuint32_t freebds;
    kuint32_t bdindex;
    XEmacPs_Bd *sptr_rxbd;
    kuint32_t *temp;
    kint32_t status;

    sptr_rxring = &XEmacPs_GetRxRing(&sptr_xemacif->sgtc_emacps);
    freebds = XEmacPs_BdRingGetFreeCnt(sptr_rxring);

    while (freebds--) {
        sptr_skb = fwk_alloc_skb(XEMACPS_MAX_FRAME_SIZE + 2 * ARCH_PER_SIZE, mask);
        if (!isValid(sptr_skb)) {
            print_err("%s: unable to alloc sk_buff\r\n", __func__);
            return -ER_NOMEM;
        }

        /*!< DMA Rx Buffer (e.g sptr_skb->data) must be 4-bytes alignment */
        fwk_skb_reserve(sptr_skb, ARCH_PER_SIZE);
        buffer = fwk_skb_put(sptr_skb, XEMACPS_MAX_FRAME_SIZE);
        if (!isValid(buffer)) {
            fwk_free_skb(sptr_skb);
            print_err("%s: unable to build rx buffer\r\n", __func__);

            return -ER_NOMEM;
        }

        /*!< free_BD ---> pre_BD */
        status = XEmacPs_BdRingAlloc(sptr_rxring, 1, &sptr_rxbd);
        if (status) {
            fwk_free_skb(sptr_skb);
            print_err("%s: Error allocating RxBD\r\n", __FUNCTION__);

            return -ER_BUSY;
        }

        /*!< pre_BD ---> work_BD */
        status = XEmacPs_BdRingToHw(sptr_rxring, 1, sptr_rxbd);
        if (status) {
            fwk_free_skb(sptr_skb);
            XEmacPs_BdRingUnAlloc(sptr_rxring, 1, sptr_rxbd);
            print_err("%s: Error: committing RxBD to HW\r\n", __FUNCTION__);

            return -ER_BUSY;
        }

        bdindex = XEMACPS_BD_TO_INDEX(sptr_rxring, sptr_rxbd);
        temp = (kuint32_t *)sptr_rxbd;

        *temp = *(temp + 1) = 0;
        if (bdindex == (XNET_RX_DESC_NUM - 1))
            *temp = 0x00000002;

        mr_dsb();

        if (!sptr_xemacif->sgtc_emacps.Config.IsCacheCoherent)
            Xil_DCacheInvalidateRange((kuaddr_t)buffer, sptr_skb->len);

        XEmacPs_BdSetAddressRx(sptr_rxbd, buffer);
        sptr_data->sptr_rxskbs[bdindex] = sptr_skb;
    }

    return ER_NORMAL;
}

/*!
 * @brief   initialize dma
 * @param   sptr_data: driver data structure pointer
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_gem_dma_init(struct xsdk_gem_drv_data *sptr_data)
{
    xemacpsif_s *sptr_xemacif = &sptr_data->sgtc_xemacif;
    XEmacPs_BdRing *sptr_rxring, *sptr_txring;
    kuint32_t gige_version;
    void *bd_space;
    XEmacPs_Bd sgtc_bdtemp;
    kint32_t status;

    sptr_rxring = &XEmacPs_GetRxRing(&sptr_xemacif->sgtc_emacps);
    sptr_txring = &XEmacPs_GetTxRing(&sptr_xemacif->sgtc_emacps);

    /*!< allocate 8K for Rx bds, and 8K for Tx bds */
    bd_space = kmalloc((XNET_RX_DESC_SIZE + XNET_TX_DESC_SIZE) * 2, GFP_ATOMIC);
    if (!isValid(bd_space))
        return -ER_NOMEM;

    sptr_xemacif->rx_bdspace = bd_space;
    sptr_xemacif->tx_bdspace = bd_space + (XNET_RX_DESC_SIZE << 1);

    /*!<
     * Setup RxBD space.
     *
     * Setup a BD template for the Rx channel. This template will be copied to
     * every RxBD. We will not have to explicitly set these again.
     */
    XEmacPs_BdClear(&sgtc_bdtemp);

    /*!< Create the RxBD ring */
    status = XEmacPs_BdRingCreate(sptr_rxring, (kuint32_t)sptr_xemacif->rx_bdspace,
                (kuint32_t)sptr_xemacif->rx_bdspace, BD_ALIGNMENT, XNET_RX_DESC_NUM);
    if (status) {
        print_err("Error setting up RxBD space\r\n");
        goto fail;
    }

    /*!< initial sptr_rxring with sgtc_bdtemp */
    status = XEmacPs_BdRingClone(sptr_rxring, &sgtc_bdtemp, XEMACPS_RECV);
    if (status) {
        print_err("Error initializing RxBD space\r\n");
        goto fail;
    }

    /*!< Setup TxBD */
    XEmacPs_BdClear(&sgtc_bdtemp);
    XEmacPs_BdSetStatus(&sgtc_bdtemp, XEMACPS_TXBUF_USED_MASK);

    /*!< Create the TxBD ring */
    status = XEmacPs_BdRingCreate(sptr_txring, (kuint32_t)sptr_xemacif->tx_bdspace,
                (kuint32_t)sptr_xemacif->tx_bdspace, BD_ALIGNMENT, XNET_TX_DESC_NUM);
    if (status)
        goto fail;

    /*!< We reuse the bd template, as the same one will work for both rx and tx. */
    status = XEmacPs_BdRingClone(sptr_txring, &sgtc_bdtemp, XEMACPS_SEND);
    if (status)
        goto fail;

    /*!< 
     * Build Rx buffer
     * Method 1: define an array with 1500 * 512 bytes; but it needs to be copied to sk_buff again
     * Method 2: allocate a sk_buff in advance, and the received packets are stored directly to skb->data by DMA 
     */
    xsdk_gem_setup_rxbd(sptr_data, GFP_ATOMIC);
    XEmacPs_SetQueuePtr(&sptr_xemacif->sgtc_emacps, sptr_xemacif->sgtc_emacps.RxBdRing.BaseBdAddr, 0, XEMACPS_RECV);

    gige_version = (XEmacPs_ReadReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, 0xFC) >> 16) & 0xfff;
    if (gige_version > 2) {
        XEmacPs_Bd *sptr_rxbd_terminate, *sptr_txbd_terminate;
        void *bd_terminate;

        /*!< allocate 8K for Rx bds, and 8K for Tx bds */
        bd_terminate = kmalloc((XNET_RX_DESC_SIZE + XNET_TX_DESC_SIZE) * 2, GFP_ATOMIC);
        if (!isValid(bd_terminate))
            goto out;

        sptr_rxbd_terminate = (XEmacPs_Bd *)bd_terminate;
        sptr_txbd_terminate = (XEmacPs_Bd *)(bd_terminate + (XNET_RX_DESC_SIZE << 1));

        XEmacPs_SetQueuePtr(&sptr_xemacif->sgtc_emacps, sptr_xemacif->sgtc_emacps.TxBdRing.BaseBdAddr, 1, XEMACPS_SEND);

        /*!<
         * This version of GEM supports priority queuing and the current
         * dirver is using tx priority queue 1 and normal rx queue for
         * packet transmit and receive. The below code ensure that the
         * other queue pointers are parked to known state for avoiding
         * the controller to malfunction by fetching the descriptors
         * from these queues.
         */
        XEmacPs_BdClear(sptr_rxbd_terminate);
        XEmacPs_BdSetAddressRx(sptr_rxbd_terminate, XEMACPS_RXBUF_NEW_MASK | XEMACPS_RXBUF_WRAP_MASK);
        XEmacPs_WriteReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_RXQ1BASE_OFFSET, (kuint32_t)sptr_rxbd_terminate);

        XEmacPs_BdClear(sptr_txbd_terminate);
        XEmacPs_BdSetStatus(sptr_txbd_terminate, XEMACPS_TXBUF_USED_MASK | XEMACPS_TXBUF_WRAP_MASK);
        XEmacPs_WriteReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_TXQBASE_OFFSET, (kuint32_t)sptr_txbd_terminate);
    
        sptr_data->bd_rxterminate = sptr_rxbd_terminate;
        sptr_data->bd_txterminate = sptr_txbd_terminate;
    }
    else {
        XEmacPs_SetQueuePtr(&sptr_xemacif->sgtc_emacps, sptr_xemacif->sgtc_emacps.TxBdRing.BaseBdAddr, 0, XEMACPS_SEND);
    }

out:
    return ER_NORMAL;

fail:
    kfree(bd_space);
    memset(sptr_rxring, 0, sizeof(*sptr_rxring));
    memset(sptr_txring, 0, sizeof(*sptr_txring));
    sptr_xemacif->rx_bdspace = sptr_xemacif->tx_bdspace = mr_nullptr;

    return -ER_FAILD;
}

/*!
 * @brief   send BD
 * @param   sptr_data: driver data structure pointer
 * @param   sptr_skb: data needed to be sent
 * @retval  none
 * @note    none
 */
static void xsdk_gem_send_bd(struct xsdk_gem_drv_data *sptr_data, struct fwk_sk_buff *sptr_skb)
{
    struct fwk_net_device *sptr_ndev = sptr_data->sptr_ndev;
    xemacpsif_s *sptr_xemacif = &sptr_data->sgtc_xemacif;
    XEmacPs_BdRing *sptr_txring;
    XEmacPs_Bd *sptr_txbd;
    kuint32_t bdindex;
    kint32_t status;

    sptr_txring = &XEmacPs_GetTxRing(&sptr_xemacif->sgtc_emacps);

    /*!< free-bd ---> pre-bd */
    status = XEmacPs_BdRingAlloc(sptr_txring, 1, &sptr_txbd);
    if (status) {
        print_err("%s: error allocating TxBD\r\n", __func__);
        goto fail;
    }

    bdindex = XEMACPS_BD_TO_INDEX(sptr_txring, sptr_txbd);
    if (sptr_data->sptr_txskbs[bdindex]) {
        print_err("%s: last skb is still existed! bdindex is invalid\r\n", __func__);
        goto fail;
    }

    /*!< 
     * Send the data from the skb to the interface, one skb at a time. 
     * The size of the data in each skb is kept in the skb->len variable. 
     */
    if (!sptr_xemacif->sgtc_emacps.Config.IsCacheCoherent)
        Xil_DCacheFlushRange((kuint32_t)sptr_skb->data, (kuint32_t)sptr_skb->len);

    XEmacPs_BdSetAddressTx(sptr_txbd, (kuint32_t)sptr_skb->data);

    if (sptr_skb->len > sptr_ndev->mtu)
        XEmacPs_BdSetLength(sptr_txbd, sptr_ndev->mtu & 0x3fff);
    else
        XEmacPs_BdSetLength(sptr_txbd, sptr_skb->len & 0x3fff);

    sptr_data->sptr_txskbs[bdindex] = sptr_skb;
    XEmacPs_BdSetLast(sptr_txbd);
    XEmacPs_BdClearTxUsed(sptr_txbd);
    mr_dsb();

    status = XEmacPs_BdRingToHw(sptr_txring, 1, sptr_txbd);
    if (status) {
        print_err("%s: Error submitting TxBD\r\n", __func__);
        goto fail;
    }

    /*!< Start transmit */
    xsdk_gem_xmit_trigger(sptr_xemacif);
    return;

fail:
    return;
}

/*!
 * @brief   aftercare after sending data
 * @param   sptr_data: driver data structure pointer
 * @retval  none
 * @note    none
 */
static void xsdk_gem_sent_complete(struct xsdk_gem_drv_data *sptr_data)
{
    xemacpsif_s *sptr_xemacif = &sptr_data->sgtc_xemacif;
    XEmacPs_BdRing *sptr_txring;
    XEmacPs_Bd *sptr_txbd, *sptr_curbd;
    struct fwk_sk_buff *sptr_skb;
    kint32_t n_bds = 0, n_bds_freed;
    kint32_t bdindex;
    kuint32_t *temp;
    kint32_t status;

    sptr_txring = &XEmacPs_GetTxRing(&sptr_xemacif->sgtc_emacps);

    /*!< 
     * work-bd ---> post-bd
     * sptr_txbd: the first work-bd
     * n_bds: the number of post-bds
     */
    while ((n_bds = XEmacPs_BdRingFromHwTx(sptr_txring, 
                        XNET_TX_DESC_NUM, &sptr_txbd))) {
        n_bds_freed = n_bds;
        sptr_curbd = sptr_txbd;

        while (n_bds_freed--) {
            bdindex = XEMACPS_BD_TO_INDEX(sptr_txring, sptr_curbd);
            temp = (kuint32_t *)sptr_curbd;

            *temp = *(temp + 1) = 0;
            if (bdindex == (XNET_TX_DESC_NUM - 1))
                *temp = 0xc0000000;
            else
                *temp = 0x80000000;

            mr_dsb();

            sptr_skb = sptr_data->sptr_txskbs[bdindex];
            if (sptr_skb)
                fwk_free_skb(sptr_skb);

            sptr_data->sptr_txskbs[bdindex] = mr_nullptr;
            sptr_curbd = XEmacPs_BdRingNext(sptr_txring, sptr_curbd);
        }

        status = XEmacPs_BdRingFree(sptr_txring, n_bds, sptr_txbd);
        if (status)
            print_err("Failure while freeing in Tx Done ISR\r\n");
    }
}

/*!
 * @brief   release tx buffer
 * @param   sptr_data: driver data structure pointer
 * @retval  none
 * @note    none
 */
static void xsdk_gem_free_txbuf_any(struct xsdk_gem_drv_data *sptr_data)
{
    struct fwk_sk_buff **sptr_skb;
    kint32_t index;

    for (index = 0; index < XNET_TX_DESC_NUM; index++) {
        sptr_skb = &sptr_data->sptr_txskbs[index];

        if (*sptr_skb) {
            fwk_free_skb(*sptr_skb);
            *sptr_skb = mr_nullptr;
        }
    }
}

/*!
 * @brief   release rx buffer
 * @param   sptr_data: driver data structure pointer
 * @retval  none
 * @note    none
 */
static void xsdk_gem_free_rxbuf_any(struct xsdk_gem_drv_data *sptr_data)
{
    struct fwk_sk_buff **sptr_skb;
    kint32_t index;

    for (index = 0; index < XNET_RX_DESC_NUM; index++) {
        sptr_skb = &sptr_data->sptr_rxskbs[index];

        if (*sptr_skb) {
            fwk_free_skb(*sptr_skb);
            *sptr_skb = mr_nullptr;
        }
    }
}

/*!
 * @brief   sending irq hander
 * @param   sptr_data: driver data structure pointer
 * @retval  none
 * @note    none
 */
static void xsdk_gem_send_handler(struct xsdk_gem_drv_data *sptr_data)
{
    xemacpsif_s *sptr_xemacif = &sptr_data->sgtc_xemacif;
    kuint32_t reg;

    /*!< Clear interrupt status bit */
    reg = XEmacPs_ReadReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_TXSR_OFFSET);
    XEmacPs_WriteReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_TXSR_OFFSET, reg);

    /*!< If Transmit done interrupt is asserted, process completed BD's */
    xsdk_gem_sent_complete(sptr_data);
}

/*!
 * @brief   recieving irq hander
 * @param   sptr_data: driver data structure pointer
 * @retval  none
 * @note    none
 */
static void xsdk_gem_recv_handler(struct xsdk_gem_drv_data *sptr_data)
{
    xemacpsif_s *sptr_xemacif = &sptr_data->sgtc_xemacif;
    XEmacPs_BdRing *sptr_rxring;
    struct fwk_sk_buff *sptr_skb;
    struct fwk_net_device *sptr_ndev = sptr_data->sptr_ndev;
    kuint32_t reg, tempcntr;
    kuint32_t gige_version;
    kuint32_t n_bds;
    XEmacPs_Bd *sptr_rxbd;
    XEmacPs_Bd *sptr_curbd;
    kuint32_t bdindex;
    kuint32_t count;

    /*!<
     * If Reception done interrupt is asserted, call RX call back function
     * to handle the processed BDs and then raise the according flag.
     */
    reg = XEmacPs_ReadReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_RXSR_OFFSET);
    XEmacPs_WriteReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_RXSR_OFFSET, reg);

    gige_version = ((XEmacPs_ReadReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, 0xFC)) >> 16) & 0xFFF;
    if (gige_version == 2) {
        tempcntr = XEmacPs_ReadReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_RXCNT_OFFSET);
        
        if ((!tempcntr) && (!sptr_xemacif->last_rx_frms_cntr)) {
            reg = XEmacPs_ReadReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_NWCTRL_OFFSET);
            reg &= ~XEMACPS_NWCTRL_RXEN_MASK;
            XEmacPs_WriteReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_NWCTRL_OFFSET, reg);

            reg = XEmacPs_ReadReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_NWCTRL_OFFSET);
            reg |= XEMACPS_NWCTRL_RXEN_MASK;
            XEmacPs_WriteReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_NWCTRL_OFFSET, reg);
        }

        sptr_xemacif->last_rx_frms_cntr = tempcntr;
    }

    /*!< deal with rx buffer */
    sptr_rxring = &XEmacPs_GetRxRing(&sptr_xemacif->sgtc_emacps);

    /*!< work-BD ---> post-BD */
    while ((n_bds = XEmacPs_BdRingFromHwRx(sptr_rxring, 
                        XNET_RX_DESC_NUM, &sptr_rxbd))) {
        for (count = 0, sptr_curbd = sptr_rxbd; count < n_bds; count++) {
            bdindex = XEMACPS_BD_TO_INDEX(sptr_rxring, sptr_curbd);
            sptr_skb = sptr_data->sptr_rxskbs[bdindex];

            /*!< Adjust the buffer size to the actual number of bytes received. */
            sptr_skb->len = XEmacPs_BdGetLength(sptr_curbd);
            sptr_skb->tail = sptr_skb->data + sptr_skb->len;
            sptr_skb->protocol = fwk_eth_type_trans(sptr_skb, sptr_ndev);
            sptr_skb->sptr_ndev = sptr_ndev;

            fwk_skb_set_mac_header(sptr_skb, 0);
            fwk_skb_set_network_header(sptr_skb, NET_ETHER_HDR_LEN);

            sptr_ndev->sgtc_stats.rx_packets++;
            sptr_ndev->sgtc_stats.rx_bytes += sptr_skb->len;
            fwk_netif_rx(sptr_skb);

            sptr_data->sptr_rxskbs[bdindex] = mr_nullptr;
            sptr_curbd = XEmacPs_BdRingNext(sptr_rxring, sptr_curbd);
        }

        /*!< free up the BD's: post-BD ---> free-BD */
        XEmacPs_BdRingFree(sptr_rxring, n_bds, sptr_rxbd);
        xsdk_gem_setup_rxbd(sptr_data, GFP_ATOMIC);
    }
}

/*!
 * @brief   error irq hander
 * @param   sptr_data: driver data structure pointer
 * @retval  none
 * @note    none
 */
static void xsdk_gem_error_handler(struct xsdk_gem_drv_data *sptr_data)
{
    struct fwk_net_device *sptr_ndev = sptr_data->sptr_ndev;
    xemacpsif_s *sptr_xemacif = &sptr_data->sgtc_xemacif;
    kuint32_t reg;
    kint32_t retval;

    xsdk_gem_free_txbuf_any(sptr_data);
    xsdk_gem_free_rxbuf_any(sptr_data);

    retval = XEmacPs_CfgInitialize(&sptr_xemacif->sgtc_emacps, 
                        &sptr_data->sgtc_config, sptr_data->sgtc_config.BaseAddress);
    if (retval)
        print_err("%s: EmacPs Configuration Failed....\r\n", __func__);

    /*!< set mac address */
    retval = XEmacPs_SetMacAddress(&sptr_xemacif->sgtc_emacps, sptr_ndev->dev_addr, 1);
    if (retval)
        print_err("%s: Emac Mac Address set failed...\r\n",__func__);

    XEmacPs_SetOperatingSpeed(&sptr_xemacif->sgtc_emacps, sptr_data->sgtc_phy.link_speed);

    /*!< Setting the operating speed of the MAC needs a delay */
//  sleep(2);

    reg = XEmacPs_ReadReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_DMACR_OFFSET);
    XEmacPs_WriteReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_DMACR_OFFSET, reg | 0x01000000);

    xsdk_gem_dma_init(sptr_data);
    XEmacPs_Start(&sptr_xemacif->sgtc_emacps);
}

/*!
 * @brief   sending error irq hander
 * @param   sptr_data: driver data structure pointer
 * @retval  none
 * @note    none
 */
static void xsdk_gem_send_error(struct xsdk_gem_drv_data *sptr_data)
{
    xemacpsif_s *sptr_xemacif = &sptr_data->sgtc_xemacif;
    kuint32_t reg;

    reg = XEmacPs_ReadReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_NWCTRL_OFFSET);
    reg &= ~XEMACPS_NWCTRL_TXEN_MASK;
    XEmacPs_WriteReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_NWCTRL_OFFSET, reg);

    xsdk_gem_free_txbuf_any(sptr_data);
    xsdk_gem_dma_tx_descs_clean(sptr_data);

    reg = XEmacPs_ReadReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_NWCTRL_OFFSET);
    reg |= XEMACPS_NWCTRL_TXEN_MASK;
    XEmacPs_WriteReg(sptr_xemacif->sgtc_emacps.Config.BaseAddress, XEMACPS_NWCTRL_OFFSET, reg);
}

/*!
 * @brief   tx error irq hander
 * @param   sptr_data: driver data structure pointer
 * @param   regVal: TxSR register value
 * @retval  none
 * @note    none
 */
static void xsdk_gem_tx_error_handler(struct xsdk_gem_drv_data *sptr_data, kuint32_t regVal)
{
    if (!regVal)
        return;

    if (regVal & XEMACPS_TXSR_HRESPNOK_MASK) {
        print_err("Transmit DMA error\r\n");
        xsdk_gem_error_handler(sptr_data);
    }

    if (regVal & XEMACPS_TXSR_URUN_MASK) {
        print_err("Transmit under run\r\n");
        xsdk_gem_send_error(sptr_data);
    }

    if (regVal & XEMACPS_TXSR_BUFEXH_MASK) {
        print_err("Transmit buffer exhausted\r\n");
        xsdk_gem_send_error(sptr_data);
    }

    if (regVal & XEMACPS_TXSR_RXOVR_MASK) {
        print_err("Transmit retry excessed limits\r\n");
        xsdk_gem_send_error(sptr_data);
    }

    if (regVal & XEMACPS_TXSR_FRAMERX_MASK) {
        print_err("Transmit collision\r\n");
        xsdk_gem_sent_complete(sptr_data);
    }
}

/*!
 * @brief   rx error irq hander
 * @param   sptr_data: driver data structure pointer
 * @param   regVal: RxSR register value
 * @retval  none
 * @note    none
 */
static void xsdk_gem_rx_error_handler(struct xsdk_gem_drv_data *sptr_data, kuint32_t regVal)
{
    if (!regVal)
        return;

    if (regVal & XEMACPS_RXSR_HRESPNOK_MASK) {
        print_err("Receive DMA error\r\n");
        xsdk_gem_error_handler(sptr_data);
    }

    if (regVal & XEMACPS_RXSR_RXOVR_MASK) {
        print_warn("Receive over run\r\n");
        xsdk_gem_recv_handler(sptr_data);
        xsdk_gem_setup_rxbd(sptr_data, GFP_ATOMIC);
    } 

    if (regVal & XEMACPS_RXSR_BUFFNA_MASK) {
        print_warn("Receive buffer not available\r\n");
        xsdk_gem_recv_handler(sptr_data);
        xsdk_gem_setup_rxbd(sptr_data, GFP_ATOMIC);
    }
}

/*!< ------------------------------------------------------------------------ */
/*!<                            net_device_ops                                */
/*!< ------------------------------------------------------------------------ */
/*!
 * @brief   gem init
 * @param   sptr_ndev: net_device structure pointer
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_gem_ndo_init(struct fwk_net_device *sptr_ndev)
{
    struct xsdk_gem_drv_data *sptr_data;
    xemacpsif_s *sptr_xemacif;
    XEmacPs_Config *sptr_config;
    struct xsdk_gem_phy *sptr_phy;
    kint32_t retval;

    sptr_data = (struct xsdk_gem_drv_data *)fwk_netdev_priv(sptr_ndev);
    
    sptr_xemacif = &sptr_data->sgtc_xemacif;
    sptr_xemacif->sptr_txq = mr_nullptr;
    sptr_xemacif->sptr_rxq = mr_nullptr;

    sptr_config = &sptr_data->sgtc_config;
    sptr_config->DeviceId = XPAR_PS7_ETHERNET_0_DEVICE_ID;
    sptr_config->BaseAddress = (kuint32_t)sptr_data->base;
    sptr_config->IsCacheCoherent = XPAR_PS7_ETHERNET_0_IS_CACHE_COHERENT;

    retval = XEmacPs_CfgInitialize(&sptr_xemacif->sgtc_emacps, sptr_config, sptr_config->BaseAddress);
    if (retval)
        return retval;

    sptr_phy = &sptr_data->sgtc_phy;
    sptr_phy->link_speed = 100;
    sptr_phy->eth_link_status = NR_ETH_LINK_UNDEFINED;
    sptr_phy->phyaddrforemac = 0;
    memset(&sptr_phy->phymapemac[0], 0, sizeof(sptr_phy->phymapemac));

    for (kint32_t index = 0; index < NET_MAC_ETH_ALEN; index++)
        sptr_ndev->dev_addr[index] = (kuint8_t)sptr_data->hwaddr[index];

    xsdk_gem_emac_init(sptr_data);
    xsdk_gem_dma_init(sptr_data);

    return ER_NORMAL;
}

/*!
 * @brief   gem deinit
 * @param   sptr_ndev: net_device structure pointer
 * @retval  none
 * @note    none
 */
static void xsdk_gem_ndo_uninit(struct fwk_net_device *sptr_ndev)
{
    struct xsdk_gem_drv_data *sptr_data;
    xemacpsif_s *sptr_xemacif;

    sptr_data = (struct xsdk_gem_drv_data *)fwk_netdev_priv(sptr_ndev);
    sptr_xemacif = &sptr_data->sgtc_xemacif;
    
    fwk_disable_irq(sptr_data->irq);
    XEmacPs_Stop(&sptr_xemacif->sgtc_emacps);
}

/*!
 * @brief   gem open
 * @param   sptr_ndev: net_device structure pointer
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_gem_ndo_open(struct fwk_net_device *sptr_ndev)
{
    struct xsdk_gem_drv_data *sptr_data;
    xemacpsif_s *sptr_xemacif;

    sptr_data = (struct xsdk_gem_drv_data *)fwk_netdev_priv(sptr_ndev);
    sptr_xemacif = &sptr_data->sgtc_xemacif;

    if (XEmacPs_Start(&sptr_xemacif->sgtc_emacps))
        return -ER_FAILD;

    fwk_enable_irq(sptr_data->irq);
    return ER_NORMAL;
}

/*!
 * @brief   gem close
 * @param   sptr_ndev: net_device structure pointer
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_gem_ndo_stop(struct fwk_net_device *sptr_ndev)
{
    struct xsdk_gem_drv_data *sptr_data;
    xemacpsif_s *sptr_xemacif;

    sptr_data = (struct xsdk_gem_drv_data *)fwk_netdev_priv(sptr_ndev);
    sptr_xemacif = &sptr_data->sgtc_xemacif;
    
    fwk_disable_irq(sptr_data->irq);
    XEmacPs_Stop(&sptr_xemacif->sgtc_emacps);

    return ER_NORMAL;
}

/*!
 * @brief   gem start transmission (for sending skb)
 * @param   sptr_skb: data will be sent
 * @param   sptr_ndev: net_device structure pointer
 * @retval  tx_bytes
 * @note    none
 */
static netdev_tx_t xsdk_gem_ndo_start_xmit(struct fwk_sk_buff *sptr_skb, struct fwk_net_device *sptr_ndev)
{
    struct xsdk_gem_drv_data *sptr_data;
    netdev_tx_t size = 0;

    sptr_data = (struct xsdk_gem_drv_data *)fwk_netdev_priv(sptr_ndev);

    /*!< prohibit sending (lock sk_buff) */
    fwk_netif_stop_queue(sptr_ndev);
    sptr_ndev->sgtc_stats.tx_packets++;
    sptr_ndev->sgtc_stats.tx_bytes += sptr_skb->len;

    /*!< the number of idle BDs is seriously insufficient */
    if (xsdk_gem_get_tx_freecnt(sptr_data) <= 5)
        xsdk_gem_sent_complete(sptr_data);

    if (xsdk_gem_get_tx_freecnt(sptr_data)) {
        size = sptr_skb->len;
        xsdk_gem_send_bd(sptr_data, sptr_skb);
    }

    fwk_free_skb(sptr_skb);
    fwk_netif_wake_queue(sptr_ndev);

    return size;
}

/*!< the global netdev_ops */
static const struct fwk_netdev_ops sgtc_xsdk_gem_drv_oprts =
{
    .ndo_init = xsdk_gem_ndo_init,
    .ndo_uninit = xsdk_gem_ndo_uninit,
    .ndo_open = xsdk_gem_ndo_open,
    .ndo_stop = xsdk_gem_ndo_stop,
    .ndo_start_xmit = xsdk_gem_ndo_start_xmit,
};

/*!< ------------------------------------------------------------------------ */
/*!<                            net_device                                    */
/*!< ------------------------------------------------------------------------ */
/*!
 * @brief   net device int
 * @param   sptr_ndev: net_device structure pointer
 * @retval  none
 * @note    initialize sptr_ndev
 */
static void xsdk_gem_driver_setup(struct fwk_net_device *sptr_ndev)
{
    struct xsdk_gem_drv_data *sptr_data;

    sptr_data = (struct xsdk_gem_drv_data *)fwk_netdev_priv(sptr_ndev);
    sptr_data->sptr_ndev = sptr_ndev;

    sptr_ndev->mtu = XEMACPS_MTU;
    sptr_ndev->sptr_netdev_oprts = &sgtc_xsdk_gem_drv_oprts;
    fwk_eth_random_addr(sptr_ndev->dev_addr);

    fwk_eth_broadcast_addr(sptr_ndev->broadcast);
    sptr_ndev->tx_queue_len = 1000;
    sptr_ndev->hard_header_len = NET_ETHER_HDR_LEN;
    sptr_ndev->min_header_len = NET_ETHER_HDR_LEN;
}

/*!
 * @brief   net device irq handler (upper irq)
 * @param   args: sptr_data
 * @retval  irq enum
 * @note    none
 */
static irq_return_t xsdk_gem_driver_isr(kint32_t irq, void *args)
{
    struct xsdk_gem_drv_data *sptr_data;
    XEmacPs *sptr_emacps;
    kuint32_t RegISR;
    kuint32_t RegSR;
    kuint32_t RegCtrl;
    kuint32_t RegQ1ISR = 0U;

    sptr_data = (struct xsdk_gem_drv_data *)args;
    sptr_emacps = &sptr_data->sgtc_xemacif.sgtc_emacps;

    /*!< 
     * This ISR will try to handle as many interrupts as it can in a single
     * call. However, in most of the places where the user's error handler
     * is called, this ISR exits because it is expected that the user will
     * reset the device in nearly all instances.
     */
    RegISR = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_ISR_OFFSET);

    /*!< Read Transmit Q1 ISR */
    if (sptr_emacps->Version > 2)
        RegQ1ISR = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_INTQ1_STS_OFFSET);

	/* Clear the interrupt status register */
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_ISR_OFFSET, RegISR);

    /*!< ---------------------------------------------------------------------------- */
    /*!< Receive complete interrupt */
    if (RegISR & XEMACPS_IXR_FRAMERX_MASK) {
        /*!< 
         * Clear RX status register RX complete indication but preserve
         * error bits if there is any 
         */
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_RXSR_OFFSET,
                   ((kuint32_t)XEMACPS_RXSR_FRAMERX_MASK | (kuint32_t)XEMACPS_RXSR_BUFFNA_MASK));
        
        sptr_data->status |= NR_GEM_IRQ_RX_BIT;
    }

    /*!< Receive error conditions interrupt */
    if (RegISR & XEMACPS_IXR_RX_ERR_MASK) {

        /*!< Clear RX status register */
        RegSR = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_RXSR_OFFSET);
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_RXSR_OFFSET, RegSR);

        /*!< 
         * Fix for CR # 692702. Write to bit 18 of net_ctrl
         * register to flush a packet out of Rx SRAM upon
         * an error for receive buffer not available. 
         */
        if ((RegISR & XEMACPS_IXR_RXUSED_MASK) != 0x00000000U) {
            RegCtrl = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET);
            RegCtrl |= (kuint32_t)XEMACPS_NWCTRL_FLUSH_DPRAM_MASK;
            XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET, RegCtrl);
        }

        if (RegSR)
        {
            sptr_data->reg_value[NR_GEM_IRQ_RXE] = RegISR;
            sptr_data->status |= NR_GEM_IRQ_RXE_BIT;
        }
    }

    /*!< ---------------------------------------------------------------------------- */
    /*!< Transmit Q1 complete interrupt */
    if (sptr_emacps->Version > 2) {
        if (RegQ1ISR & XEMACPS_INTQ1SR_TXCOMPL_MASK) {

            /*!< Clear TX status register TX complete indication but preserve
            * error bits if there is any */
            XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_INTQ1_STS_OFFSET, XEMACPS_INTQ1SR_TXCOMPL_MASK);
            XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_TXSR_OFFSET,
                    ((kuint32_t)XEMACPS_TXSR_TXCOMPL_MASK | (kuint32_t)XEMACPS_TXSR_USEDREAD_MASK));
            
            mr_setbit(sptr_data->status, NR_GEM_IRQ_TX_BIT);

            /*!< Transmit Q1 error conditions interrupt */
            if (RegQ1ISR & XEMACPS_INTQ1SR_TXERR_MASK) {

                /*!< Clear Interrupt Q1 status register */
                XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_INTQ1_STS_OFFSET, RegQ1ISR);

                sptr_data->reg_value[NR_GEM_IRQ_TXE] = RegQ1ISR;
                sptr_data->status |= NR_GEM_IRQ_TXE_BIT;
            }
        }
    }

    /*!< Transmit complete interrupt */
    if (RegISR & XEMACPS_IXR_TXCOMPL_MASK) {
        /*!< 
         * Clear TX status register TX complete indication but preserve
         * error bits if there is any 
         */
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_TXSR_OFFSET,
                   ((kuint32_t)XEMACPS_TXSR_TXCOMPL_MASK | (kuint32_t)XEMACPS_TXSR_USEDREAD_MASK));
        
        sptr_data->status |= NR_GEM_IRQ_TX_BIT;
    }
    else if (RegISR & XEMACPS_IXR_TX_ERR_MASK) {
        /*!< Clear TX status register */
        RegSR = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_TXSR_OFFSET);
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_TXSR_OFFSET, RegSR);

        sptr_data->reg_value[NR_GEM_IRQ_TXE] = RegISR;
        sptr_data->status |= NR_GEM_IRQ_TXE_BIT;
    }

    if (sptr_data->status & NR_GEM_IRQ_ERR_BIT)
        fwk_disable_irq(sptr_data->irq);

    return NR_IRQ_WAKE_THREAD;
}

/*!
 * @brief   net device irq handler (bottom irq)
 * @param   args: sptr_data
 * @retval  irq enum
 * @note    none
 */
static irq_return_t xsdk_gem_driver_bottom_isr(kint32_t irq, void *args)
{
    struct xsdk_gem_drv_data *sptr_data;
    nrt_gem_irq_stat_t status;

    sptr_data = (struct xsdk_gem_drv_data *)args;
    status = sptr_data->status;
    sptr_data->status = 0;

    if (status & NR_GEM_IRQ_RX_BIT)
        xsdk_gem_recv_handler(sptr_data);
    if (status & NR_GEM_IRQ_TX_BIT)
        xsdk_gem_send_handler(sptr_data);
    
    if (status & NR_GEM_IRQ_ERR_BIT) {
        if (status & NR_GEM_IRQ_RXE_BIT)
            xsdk_gem_rx_error_handler(sptr_data, sptr_data->reg_value[NR_GEM_IRQ_RXE]);
        if (status & NR_GEM_IRQ_TXE_BIT)
            xsdk_gem_tx_error_handler(sptr_data, sptr_data->reg_value[NR_GEM_IRQ_TXE]);

        memset(sptr_data->reg_value, 0, sizeof(sptr_data->reg_value));
        fwk_enable_irq(sptr_data->irq);
    }

    return NR_IRQ_HANDLED;
}

/*!
 * @brief   get and set property
 * @param   sptr_pdev, sptr_data
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_gem_driver_probe_dt(struct fwk_platdev *sptr_pdev, struct xsdk_gem_drv_data *sptr_data)
{
    struct fwk_device_node *sptr_node, *sptr_phy;
    kuint32_t phandle;
    void *base;

    sptr_node = sptr_pdev->sgtc_dev.sptr_node;
    if (!isValid(sptr_node))
        return PTR_ERR(sptr_node);

    fwk_of_property_read_u32(sptr_node, "phy-handle", &phandle);
    sptr_phy = fwk_of_find_node_by_phandle(mr_nullptr, phandle);
    if (!isValid(sptr_phy))
        return PTR_ERR(sptr_phy);

    base = (void *)fwk_platform_get_address(sptr_pdev, 0);
    sptr_data->base = fwk_io_remap(base, ARCH_PER_SIZE);
    sptr_data->irq = fwk_platform_get_irq(sptr_pdev, 0);

    fwk_of_property_read_u8_array(sptr_phy, "local-mac-address", sptr_data->hwaddr, NET_MAC_ETH_ALEN);
    return ER_NORMAL;
}

/*!
 * @brief   xsdk_gem_driver_probe
 * @param   sptr_pdev
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_gem_driver_probe(struct fwk_platdev *sptr_pdev)
{
    struct xsdk_gem_drv_data *sptr_data;
    struct fwk_net_device *sptr_ndev;
    kint32_t retval;

    sptr_ndev = fwk_alloc_netdev(sizeof(*sptr_data), "eth%d", xsdk_gem_driver_setup);
    if (!isValid(sptr_ndev))
        return -ER_FAILD;
    
    sptr_data = (struct xsdk_gem_drv_data *)fwk_netdev_priv(sptr_ndev);
    if (!isValid(sptr_data))
        goto fail1;

    if (xsdk_gem_driver_probe_dt(sptr_pdev, sptr_data))
        goto fail1;

    fwk_platform_set_drvdata(sptr_pdev, sptr_data);
    retval = fwk_request_threaded_irq(sptr_data->irq, xsdk_gem_driver_isr, 
                            xsdk_gem_driver_bottom_isr, 0, XSDK_GEM_DRIVER_NAME, sptr_data);
    if (retval)
        goto fail2;

    fwk_disable_irq(sptr_data->irq);
    retval = fwk_register_netdevice(sptr_ndev);
    if (retval)
        goto fail3;

    print_info("register a new netdevice (GEM)\r\n");
    return ER_NORMAL;

fail3:
    fwk_free_irq(sptr_data->irq, sptr_data);
fail2:
    fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);
fail1:
    fwk_free_netdev(sptr_ndev);
    return -ER_FAILD;
}

/*!
 * @brief   xsdk_gem_driver_remove
 * @param   sptr_pdev
 * @retval  errno
 * @note    none
 */
static kint32_t xsdk_gem_driver_remove(struct fwk_platdev *sptr_pdev)
{
    struct fwk_net_device *sptr_ndev;
    struct xsdk_gem_drv_data *sptr_data;

    sptr_ndev = (struct fwk_net_device *)fwk_platform_get_drvdata(sptr_pdev);
    if (!isValid(sptr_ndev))
        return -ER_NULLPTR;

    sptr_data = (struct xsdk_gem_drv_data *)fwk_netdev_priv(sptr_ndev);

    fwk_unregister_netdevice(sptr_ndev);
    fwk_free_irq(sptr_data->irq, sptr_data);
    fwk_platform_set_drvdata(sptr_pdev, mr_nullptr);
    fwk_free_netdev(sptr_ndev);

    return ER_NORMAL;
}

/*!< device id for device-tree */
static const struct fwk_of_device_id sgtc_xsdk_gem_driver_id[] =
{
    { .compatible = "cdns,zynq-gem", },
    {},
};

/*!< platform instance */
static struct fwk_platdrv sgtc_xsdk_gem_platdriver =
{
    .probe	= xsdk_gem_driver_probe,
    .remove	= xsdk_gem_driver_remove,
    
    .sgtc_driver =
    {
        .name 	= XSDK_GEM_DRIVER_NAME,
        .id 	= -1,
        .sptr_of_match_table = sgtc_xsdk_gem_driver_id,
    },
};

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   xsdk_gem_driver_init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __fwk_init xsdk_gem_driver_init(void)
{
    return fwk_register_platdriver(&sgtc_xsdk_gem_platdriver);
}

/*!
 * @brief   xsdk_gem_driver_exit
 * @param   none
 * @retval  none
 * @note    none
 */
void __fwk_exit xsdk_gem_driver_exit(void)
{
    fwk_unregister_platdriver(&sgtc_xsdk_gem_platdriver);
}

IMPORT_DRIVER_INIT(xsdk_gem_driver_init);
IMPORT_DRIVER_EXIT(xsdk_gem_driver_exit);

/*!< end of file */
