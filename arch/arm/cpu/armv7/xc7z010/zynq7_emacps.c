/*
 * ZYNQ eMAC Of PS Interface
 *
 * File Name:   zynq7_emacps.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.11.24
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#include <common/generic.h>
#include <common/queue.h>

#include <boot/board_init.h>
#include <asm/armv7/gcc_config.h>

#include <zynq7/zynq7_periph.h>
#include <zynq7/xemac/xemacpsif.h>
#include <zynq7/xemac/xemacps.h>
#include <zynq7/xemac/xemac_ieee_reg.h>

/*!< The defines */
#define CONFIG_LINKSPEED_AUTODETECT	            1

struct xtopology
{
    kuaddr_t emac_baseaddr;
    kuaddr_t intc_baseaddr;
    kuaddr_t intc_emac_intr;	                /*!< valid only for xemac_type_xps_emaclite */
    kuaddr_t scugic_baseaddr;                   /*!< valid only for Zynq */
    kuaddr_t scugic_emac_intr;                  /*!< valid only for GEM */
};

#define XEMACPS_GMII2RGMII_SPEED1000_FD         0x140
#define XEMACPS_GMII2RGMII_SPEED100_FD          0x2100
#define XEMACPS_GMII2RGMII_SPEED10_FD           0x100
#define XEMACPS_GMII2RGMII_REG_NUM              0x10

/*!< Frequency setting */
#define SLCR_LOCK_ADDR                          (XPS_SYS_CTRL_BASEADDR + 0x4)
#define SLCR_UNLOCK_ADDR                        (XPS_SYS_CTRL_BASEADDR + 0x8)
#define SLCR_GEM0_CLK_CTRL_ADDR                 (XPS_SYS_CTRL_BASEADDR + 0x140)
#define SLCR_GEM1_CLK_CTRL_ADDR                 (XPS_SYS_CTRL_BASEADDR + 0x144)
#define SLCR_GEM_SRCSEL_EMIO                    0x40
#define SLCR_LOCK_KEY_VALUE                     0x767B
#define SLCR_UNLOCK_KEY_VALUE                   0xDF0D
#define SLCR_ADDR_GEM_RST_CTRL                  (XPS_SYS_CTRL_BASEADDR + 0x214)
#define EMACPS_SLCR_DIV_MASK                    0xFC0FC0FF

/*!< BdRing */
#define XEMACPS_RING_SEEKAHEAD(sptr_bdring, BdPtr, NumBd)   \
    do {    \
        kuint32_t Addr = (kuint32_t)(void *)(BdPtr);    \
        \
        Addr += ((sptr_bdring)->Separation * (NumBd));  \
        if ((Addr > (sptr_bdring)->HighBdAddr) ||   \
            ((kuint32_t)(void *)(BdPtr) > Addr))  \
            Addr -= (sptr_bdring)->Length;  \
        \
        (BdPtr) = (XEmacPs_Bd *)(void *)Addr;    \
    } while (0)

#define XEMACPS_RING_SEEKBACK(sptr_bdring, BdPtr, NumBd)     \
    do {    \
        kuint32_t Addr = (kuint32_t)(void *)(BdPtr);    \
        \
        Addr -= ((sptr_bdring)->Separation * (NumBd));  \
        if ((Addr < (sptr_bdring)->BaseBdAddr) ||   \
            ((kuint32_t)(void *)(BdPtr) < Addr))   \
            Addr += (sptr_bdring)->Length;  \
        \
        (BdPtr) = (XEmacPs_Bd *)(void*)Addr; \
    } while (0)

#define XEMACPS_BD_TO_INDEX(sptr_bdring, bdptr) \
    (((kuint32_t)bdptr - (kuint32_t)(sptr_bdring)->BaseBdAddr) / (sptr_bdring)->Separation)

/*!< The globals */
static struct xtopology sgtc_xsdk_topology[] = 
{
    {
        .emac_baseaddr = 0xE000B000,
        .intc_baseaddr = 0x0,
        .intc_emac_intr = 0x0,
        .scugic_baseaddr = 0xF8F00100,
        .scugic_emac_intr = 0x36,
    },
};
const kint32_t g_xsdk_topology_num = ARRAY_SIZE(sgtc_xsdk_topology);

static XEmacPs_Config sgtc_xmacps_configTable[XPAR_XEMACPS_NUM_INSTANCES] =
{
    {
        XPAR_PS7_ETHERNET_0_DEVICE_ID,
        XPAR_PS7_ETHERNET_0_BASEADDR,
        XPAR_PS7_ETHERNET_0_IS_CACHE_COHERENT
    }
};

/*!< The functions */

/*!< API functions */
/*!
 * @brief   set rx wrap
 * @param   BdPtr: Bd base address
 * @retval  none
 */
static void XEmacPs_BdSetRxWrap(kuint32_t BdPtr)
{
    kuint32_t DataValueRx;
    kuint32_t *TempPtr;

    BdPtr += (kuint32_t)(XEMACPS_BD_ADDR_OFFSET);
    TempPtr = (kuint32_t *)BdPtr;

    if (isValid(TempPtr))
    {
        DataValueRx = *TempPtr;
        DataValueRx |= XEMACPS_RXBUF_WRAP_MASK;
        *TempPtr = DataValueRx;
    }
}

/*!
 * @brief   set tx wrap
 * @param   BdPtr: Bd base address
 * @retval  none
 */
static void XEmacPs_BdSetTxWrap(kuint32_t BdPtr)
{
    kuint32_t DataValueTx;
    kuint32_t *TempPtr;

    BdPtr += (kuint32_t)(XEMACPS_BD_STAT_OFFSET);
    TempPtr = (kuint32_t *)BdPtr;

    if (isValid(TempPtr))
    {
        DataValueTx = *TempPtr;
        DataValueTx |= XEMACPS_TXBUF_WRAP_MASK;
        *TempPtr = DataValueTx;
    }
}

/*!
 * @brief   create Bd Ring
 * @param   sptr_bdring: BdRing structure pointer
 * @param   PhysAddr: phy address
 * @retval  errno
 */
kint32_t XEmacPs_BdRingCreate(XEmacPs_BdRing *sptr_bdring, kuint32_t PhysAddr,
                            kuint32_t VirtAddr, kuint32_t Alignment, kuint32_t BdCount)
{
    kuint32_t i;
    kuint32_t BdVirtAddr;
    kuint32_t BdPhyAddr;
    kuint32_t VirtAddrLoc = VirtAddr;

    /*!< 
     * In case there is a failure prior to creating list, make sure the
     * following attributes are 0 to prevent calls to other functions
     * from doing anything.
     */
    sptr_bdring->AllCnt = 0U;
    sptr_bdring->FreeCnt = 0U;
    sptr_bdring->HwCnt = 0U;
    sptr_bdring->PreCnt = 0U;
    sptr_bdring->PostCnt = 0U;

    /*!< Make sure Alignment parameter meets minimum requirements */
    if (Alignment < (kuint32_t)XEMACPS_DMABD_MINIMUM_ALIGNMENT)
        return -ER_INVALID;

    /*!< Make sure Alignment is a power of 2 */
    if (((Alignment - 0x00000001U) & Alignment) != 0x00000000U)
        return -ER_NOTALIGN;

    /*!< Make sure PhysAddr and VirtAddr are on same Alignment */
    if (((PhysAddr % Alignment) != (kuint32_t)0) || ((VirtAddrLoc % Alignment) != (kuint32_t)0))
        return -ER_NOTALIGN;

    /*!< Is BdCount reasonable? */
    if (BdCount == 0x00000000U)
        return -ER_INVALID;

    /*!< Figure out how many bytes will be between the start of adjacent BDs */
    sptr_bdring->Separation = ((kuint32_t)sizeof(XEmacPs_Bd));

    /*!< 
     * Must make sure the ring doesn't span address 0x00000000. If it does,
     * then the next/prev BD traversal macros will fail.
     */
    if (VirtAddrLoc > ((VirtAddrLoc + (sptr_bdring->Separation * BdCount)) - (kuint32_t)1))
        return -ER_FAULT;

    /*!< 
     * Initial ring setup:
     *  - Clear the entire space
     *  - Setup each BD's BDA field with the physical address of the next BD
     */
    memset((void *)VirtAddrLoc, 0, (sptr_bdring->Separation * BdCount));

    BdVirtAddr = VirtAddrLoc;
    BdPhyAddr = PhysAddr + sptr_bdring->Separation;
    for (i = 1U; i < BdCount; i++) 
    {
        BdVirtAddr += sptr_bdring->Separation;
        BdPhyAddr += sptr_bdring->Separation;
    }

    /*!< Setup and initialize pointers and counters */
    sptr_bdring->IsRunning = false;
    sptr_bdring->BaseBdAddr = VirtAddrLoc;
    sptr_bdring->PhysBaseAddr = PhysAddr;
    sptr_bdring->HighBdAddr = BdVirtAddr;
    sptr_bdring->Length = ((sptr_bdring->HighBdAddr - sptr_bdring->BaseBdAddr) + sptr_bdring->Separation);
    sptr_bdring->AllCnt = (kuint32_t)BdCount;
    sptr_bdring->FreeCnt = (kuint32_t)BdCount;
    sptr_bdring->FreeHead = (XEmacPs_Bd *)(void *)VirtAddrLoc;
    sptr_bdring->PreHead = (XEmacPs_Bd *)VirtAddrLoc;
    sptr_bdring->HwHead = (XEmacPs_Bd *)VirtAddrLoc;
    sptr_bdring->HwTail = (XEmacPs_Bd *)VirtAddrLoc;
    sptr_bdring->PostHead = (XEmacPs_Bd *)VirtAddrLoc;
    sptr_bdring->BdaRestart = (XEmacPs_Bd *)(void *)PhysAddr;

    return ER_NORMAL;
}

/*!
 * @brief   Initial Bd Ring
 * @param   sptr_bdring: BdRing structure pointer
 * @param   PhysAddr: phy address
 * @retval  none
 */
void XEmacPs_BdRingInitial(XEmacPs_BdRing *sptr_bdring, kuint32_t PhysAddr,
                            kuint32_t VirtAddr, kuint32_t Alignment, kuint32_t BdCount)
{
    (void)XEmacPs_BdRingCreate(sptr_bdring, PhysAddr, VirtAddr, Alignment, BdCount);
}

/*!
 * @brief   copy Bd Ring
 * @param   sptr_bdring: BdRing structure pointer
 * @param   sptr_bd: Bd base address
 * @retval  errno
 */
kint32_t XEmacPs_BdRingClone(XEmacPs_BdRing *sptr_bdring, XEmacPs_Bd *sptr_bd, kuint8_t Direction)
{
    kuint32_t i;
    kuint32_t CurBd;

    /*!< Can't do this function if there isn't a ring */
    if (sptr_bdring->AllCnt == 0x00000000U)
        return -ER_EMPTY;

    /*!< Can't do this function with the channel running */
    if (sptr_bdring->IsRunning)
        return -ER_LOCKED;

    /*!< Can't do this function with some of the BDs in use */
    if (sptr_bdring->FreeCnt != sptr_bdring->AllCnt)
        return -ER_ERROR;

    if ((Direction != (kuint8_t)XEMACPS_SEND) && (Direction != (kuint8_t)XEMACPS_RECV))
        return -ER_INVALID;

    /*!< 
     * Starting from the top of the ring, save BD.Next, overwrite the entire
     * BD with the template, then restore BD.Next
     */
    CurBd = sptr_bdring->BaseBdAddr;
    for (i = 0U; i < sptr_bdring->AllCnt; i++) 
    {
        memcpy((void *)CurBd, sptr_bd, sizeof(XEmacPs_Bd));
        CurBd += sptr_bdring->Separation;
    }

    CurBd -= sptr_bdring->Separation;

    if (Direction == XEMACPS_RECV)
        XEmacPs_BdSetRxWrap(CurBd);
    else
        XEmacPs_BdSetTxWrap(CurBd);

    return ER_NORMAL;
}

/*!
 * @brief   allocate Bd Ring
 * @param   sptr_bdring: BdRing structure pointer
 * @param   NumBd: the length of bd
 * @retval  errno
 */
kint32_t XEmacPs_BdRingAlloc(XEmacPs_BdRing *sptr_bdring, kuint32_t NumBd, XEmacPs_Bd **sptr_bd)
{
    /*!< Enough free BDs available for the request? */
    if (sptr_bdring->FreeCnt < NumBd)
        return -ER_FAULT;

    /*!< Set the return argument and move FreeHead forward */
    *sptr_bd = sptr_bdring->FreeHead;
    XEMACPS_RING_SEEKAHEAD(sptr_bdring, sptr_bdring->FreeHead, NumBd);

    sptr_bdring->FreeCnt -= NumBd;
    sptr_bdring->PreCnt += NumBd;

    return ER_NORMAL;
}

/*!
 * @brief   unallocate Bd Ring
 * @param   sptr_bdring: BdRing structure pointer
 * @param   NumBd: the length of bd
 * @retval  errno
 */
kint32_t XEmacPs_BdRingUnAlloc(XEmacPs_BdRing *sptr_bdring, kuint32_t NumBd, XEmacPs_Bd *sptr_bd)
{
    if ((!sptr_bdring) ||
        (!sptr_bd))
        return -ER_INVALID;

    /*!< Enough BDs in the free state for the request? */
    if (sptr_bdring->PreCnt < NumBd)
        return -ER_FAULT;

    /*!< Set the return argument and move FreeHead backward */
    XEMACPS_RING_SEEKBACK(sptr_bdring, sptr_bdring->FreeHead, NumBd);
    sptr_bdring->FreeCnt += NumBd;
    sptr_bdring->PreCnt -= NumBd;

    return ER_NORMAL;
}

/*!
 * @brief   release Bd Ring
 * @param   sptr_bdring: BdRing structure pointer
 * @param   NumBd: the length of bd
 * @retval  errno
 */
kint32_t XEmacPs_BdRingFree(XEmacPs_BdRing *sptr_bdring, kuint32_t NumBd, XEmacPs_Bd *sptr_bd)
{
    /*!< if no bds to process, simply return. */
    if (0U == NumBd)
        return -ER_INVALID;

    /*!< Make sure we are in sync with XEmacPs_BdRingFromHw() */
    if ((sptr_bdring->PostCnt < NumBd) || (sptr_bdring->PostHead != sptr_bd))
        return -ER_ERROR;
    
    /*!< Update pointers and counters */
    sptr_bdring->FreeCnt += NumBd;
    sptr_bdring->PostCnt -= NumBd;
    XEMACPS_RING_SEEKAHEAD(sptr_bdring, sptr_bdring->PostHead, NumBd);

    return ER_NORMAL;
}

/*!
 * @brief   allocate Bd Ring
 * @param   sptr_bdring: BdRing structure pointer
 * @param   NumBd: the length of bd
 * @retval  errno
 */
kuint32_t XEmacPs_BdRingFromHwTx(XEmacPs_BdRing *sptr_bdring, kuint32_t BdLimit, XEmacPs_Bd **sptr_bd)
{
    XEmacPs_Bd *CurBdPtr;
    kuint32_t BdStr = 0U;
    kuint32_t BdCount;
    kuint32_t BdPartialCount;
    kuint32_t Sop = 0U;
    kuint32_t BdLimitLoc = BdLimit;

    CurBdPtr = sptr_bdring->HwHead;
    BdCount = 0U;
    BdPartialCount = 0U;

    /*!< If no BDs in work group, then there's nothing to search */
    if (sptr_bdring->HwCnt == 0x00000000U) 
        goto fail;

    if (BdLimitLoc > sptr_bdring->HwCnt)
        BdLimitLoc = sptr_bdring->HwCnt;

    /*!< 
     * Starting at HwHead, keep moving forward in the list until:
     *  - A BD is encountered with its new/used bit set which means
     *    hardware has not completed processing of that BD.
     *  - sptr_bdring->HwTail is reached and sptr_bdring->HwCnt is reached.
     *  - The number of requested BDs has been processed
     */
    while (BdCount < BdLimitLoc) 
    {
        /*!< Read the status */
        if (CurBdPtr != mr_nullptr)
            BdStr = XEmacPs_BdRead(CurBdPtr, XEMACPS_BD_STAT_OFFSET);

        if ((Sop == 0x00000000U) && ((BdStr & XEMACPS_TXBUF_USED_MASK) != 0x00000000U))
            Sop = 1U;

        if (Sop == 0x00000001U) 
        {
            BdCount++;
            BdPartialCount++;
        }

        /*!< 
         * hardware has processed this BD so check the "last" bit.
         * If it is clear, then there are more BDs for the current
         * packet. Keep a count of these partial packet BDs.
         */
        if ((Sop == 0x00000001U) && ((BdStr & XEMACPS_TXBUF_LAST_MASK) != 0x00000000U)) 
        {
            Sop = 0U;
            BdPartialCount = 0U;
        }

        /*!< Move on to next BD in work group */
        CurBdPtr = XEmacPs_BdRingNext(sptr_bdring, CurBdPtr);
    }

    /*!< Subtract off any partial packet BDs found */
    BdCount -= BdPartialCount;

    /*!< 
     * If BdCount is non-zero then BDs were found to return. Set return
     * parameters, update pointers and counters, return success
     */
    if (BdCount > 0x00000000U) 
    {
        *sptr_bd = sptr_bdring->HwHead;
        sptr_bdring->HwCnt -= BdCount;
        sptr_bdring->PostCnt += BdCount;
        XEMACPS_RING_SEEKAHEAD(sptr_bdring, sptr_bdring->HwHead, BdCount);

        return BdCount;
    }

fail:
    *sptr_bd = mr_nullptr;
    return 0;
}

/*!
 * @brief   allocate Bd Ring
 * @param   sptr_bdring: BdRing structure pointer
 * @param   NumBd: the length of bd
 * @retval  errno
 */
kuint32_t XEmacPs_BdRingFromHwRx(XEmacPs_BdRing *sptr_bdring, kuint32_t BdLimit, XEmacPs_Bd **sptr_bd)
{
    XEmacPs_Bd *CurBdPtr;
    kuint32_t BdStr = 0U;
    kuint32_t BdCount;
    kuint32_t BdPartialCount;

    CurBdPtr = sptr_bdring->HwHead;
    BdCount = 0U;
    BdPartialCount = 0U;

    /*!< If no BDs in work group, then there's nothing to search */
    if (sptr_bdring->HwCnt == 0x00000000U)
        goto fail;

    /*!< 
     * Starting at HwHead, keep moving forward in the list until:
     *  - A BD is encountered with its new/used bit set which means
     *    hardware has completed processing of that BD.
     *  - sptr_bdring->HwTail is reached and sptr_bdring->HwCnt is reached.
     *  - The number of requested BDs has been processed
     */
    while (BdCount < BdLimit)
    {
        /*!< Read the status */
        if (CurBdPtr != mr_nullptr)
            BdStr = XEmacPs_BdRead(CurBdPtr, XEMACPS_BD_STAT_OFFSET);

        if (!XEmacPs_BdIsRxNew(CurBdPtr))
            break;

        BdCount++;

        /*!< hardware has processed this BD so check the "last" bit. If
         * it is clear, then there are more BDs for the current packet.
         * Keep a count of these partial packet BDs.
         */
        if ((BdStr & XEMACPS_RXBUF_EOF_MASK) != 0x00000000U)
            BdPartialCount = 0U;
        else
            BdPartialCount++;

        /*!< Move on to next BD in work group */
        CurBdPtr = XEmacPs_BdRingNext(sptr_bdring, CurBdPtr);
    }

    /*!< Subtract off any partial packet BDs found */
    BdCount -= BdPartialCount;

    /*!< 
     * If BdCount is non-zero then BDs were found to return. Set return
     * parameters, update pointers and counters, return success
     */
    if (BdCount > 0x00000000U) 
    {
        *sptr_bd = sptr_bdring->HwHead;
        sptr_bdring->HwCnt -= BdCount;
        sptr_bdring->PostCnt += BdCount;
        XEMACPS_RING_SEEKAHEAD(sptr_bdring, sptr_bdring->HwHead, BdCount);

        return BdCount;
    }

fail:
    *sptr_bd = mr_nullptr;
    return 0;
}

/*!
 * @brief   allocate Bd Ring
 * @param   sptr_bdring: BdRing structure pointer
 * @param   NumBd: the length of bd
 * @retval  errno
 */
kint32_t XEmacPs_BdRingToHw(XEmacPs_BdRing *sptr_bdring, kuint32_t NumBd, XEmacPs_Bd *sptr_bd)
{
    XEmacPs_Bd *CurBdPtr;
    kuint32_t i;

    /*!< if no bds to process, simply return. */
    if (0U == NumBd)
        return -ER_INVALID;
 
    /*!< Make sure we are in sync with XEmacPs_BdRingAlloc() */
    if ((sptr_bdring->PreCnt < NumBd) || (sptr_bdring->PreHead != sptr_bd))
        return -ER_ERROR;

    CurBdPtr = sptr_bd;
    for (i = 0U; i < NumBd; i++)
        CurBdPtr = (XEmacPs_Bd *)((void *)XEmacPs_BdRingNext(sptr_bdring, CurBdPtr));

    /*!< Adjust ring pointers & counters */
    XEMACPS_RING_SEEKAHEAD(sptr_bdring, sptr_bdring->PreHead, NumBd);
    sptr_bdring->PreCnt -= NumBd;
    sptr_bdring->HwTail = CurBdPtr;
    sptr_bdring->HwCnt += NumBd;

    return ER_NORMAL;
}

/*!< -------------------------------------------------------------------------- */
/*!
 * @brief   read emac phy maintaince reg
 * @param   sptr_emacps: emac structure pointer
 * @retval  errno
 */
kint32_t XEmacPs_PhyRead(XEmacPs *sptr_emacps, kuint32_t PhyAddress, kuint32_t RegisterNum, kuint16_t *PhyDataPtr)
{
    kuint32_t Mgtcr;
    volatile kuint32_t Ipisr;
    kuint32_t Status;

    if (!sptr_emacps)
        return -ER_NREADY;

    /*!< Make sure no other PHY operation is currently in progress */
    Status = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWSR_OFFSET);
    if (!(Status & XEMACPS_NWSR_MDIOIDLE_MASK))
        return -ER_BUSY;

    /*!< Construct Mgtcr mask for the operation */
    Mgtcr = XEMACPS_PHYMNTNC_OP_MASK | XEMACPS_PHYMNTNC_OP_R_MASK |
            (PhyAddress << XEMACPS_PHYMNTNC_PHAD_SHFT_MSK) |
            (RegisterNum << XEMACPS_PHYMNTNC_PREG_SHFT_MSK);

    /*!< Write Mgtcr and wait for completion */
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_PHYMNTNC_OFFSET, Mgtcr);

    do {
        Ipisr = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWSR_OFFSET);

    } while ((Ipisr & XEMACPS_NWSR_MDIOIDLE_MASK) == 0U);

    /*!< Read data */
    *PhyDataPtr = (kuint16_t)XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_PHYMNTNC_OFFSET);

    return ER_NORMAL;
}

/*!
 * @brief   write emac phy maintaince reg
 * @param   sptr_emacps: emac structure pointer
 * @retval  errno
 */
kint32_t XEmacPs_PhyWrite(XEmacPs *sptr_emacps, kuint32_t PhyAddress, kuint32_t RegisterNum, kuint16_t PhyData)
{
    kuint32_t Mgtcr;
    volatile kuint32_t Ipisr;
    kuint32_t Status;

    if (!sptr_emacps)
        return -ER_NREADY;

    /*!< Make sure no other PHY operation is currently in progress */
    Status = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWSR_OFFSET);
    if (!(Status & XEMACPS_NWSR_MDIOIDLE_MASK))
        return -ER_BUSY;

    /*!< Construct Mgtcr mask for the operation */
    Mgtcr = XEMACPS_PHYMNTNC_OP_MASK | XEMACPS_PHYMNTNC_OP_W_MASK |
            (PhyAddress << XEMACPS_PHYMNTNC_PHAD_SHFT_MSK) |
            (RegisterNum << XEMACPS_PHYMNTNC_PREG_SHFT_MSK) | (kuint32_t)PhyData;

    /*!< Write Mgtcr and wait for completion */
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_PHYMNTNC_OFFSET, Mgtcr);

    do {
        Ipisr = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWSR_OFFSET);

    } while ((Ipisr & XEMACPS_NWSR_MDIOIDLE_MASK) == 0U);

    return ER_NORMAL;
}

/*!
 * @brief   check if phy is detected
 * @param   sptr_emacps: emac structure pointer
 * @param   phy_addr: phy address
 * @retval  1: yes; 0: no
 */
kbool_t XEmacPs_PhyLinkDetect(XEmacPs *sptr_emacps, kuint32_t phy_addr)
{
    kuint16_t status;

    /*!< Read Phy Status register twice to get the confirmation of the current link status. */
    XEmacPs_PhyRead(sptr_emacps, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
    XEmacPs_PhyRead(sptr_emacps, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
    
    return !!(status & IEEE_STAT_LINK_STATUS);
}

/*!
 * @brief   check if phy is configured with auto...
 * @param   sptr_emacps: emac structure pointer
 * @param   phy_addr: phy address
 * @retval  1: yes; 0: no
 */
kbool_t XEmacPs_PhyAutoNegStatus(XEmacPs *sptr_emacps, kuint32_t phy_addr)
{
    kuint16_t status;

    /*!< Read Phy Status register twice to get the confirmation of the current link status. */
    XEmacPs_PhyRead(sptr_emacps, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
    XEmacPs_PhyRead(sptr_emacps, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
   
    return !!(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE);
}

/*!
 * @brief   check if phy is configured with auto...
 * @param   sptr_emacps: emac structure pointer
 * @param   phy_addr: phy address
 * @retval  1: yes; 0: no
 */
kint32_t XEmacPs_PhySetup(XEmacPs *sptr_emacps, kuint32_t phy_addr, kint32_t link_speed)
{
    kuint32_t conv_present = 0;
    kuint32_t convspeeddupsetting = 0;
    kuint32_t convphyaddr = 0;

#ifdef XPAR_GMII2RGMIICON_0N_ETH0_ADDR
    convphyaddr = XPAR_GMII2RGMIICON_0N_ETH0_ADDR;
    conv_present = 1;
#endif
#ifdef XPAR_GMII2RGMIICON_0N_ETH1_ADDR
    convphyaddr = XPAR_GMII2RGMIICON_0N_ETH1_ADDR;
    conv_present = 1;
#endif

#ifdef CONFIG_LINKSPEED_AUTODETECT
    if (link_speed == 1000) 
    {
        XEmacPs_SetUpSLCRDivisors(sptr_emacps->Config.BaseAddress, 1000);
        convspeeddupsetting = XEMACPS_GMII2RGMII_SPEED1000_FD;
    } 
    else if (link_speed == 100) 
    {
        XEmacPs_SetUpSLCRDivisors(sptr_emacps->Config.BaseAddress, 100);
        convspeeddupsetting = XEMACPS_GMII2RGMII_SPEED100_FD;
    }
    else if (link_speed == 10)
    {
        XEmacPs_SetUpSLCRDivisors(sptr_emacps->Config.BaseAddress, 10);
        convspeeddupsetting = XEMACPS_GMII2RGMII_SPEED10_FD;
    } 
    else 
    {
        print_debug("Phy setup error \r\n");
        return -ER_INVALID;
    }

#elif defined(CONFIG_LINKSPEED1000)
    XEmacPs_SetUpSLCRDivisors(sptr_emacps->Config.BaseAddress,1000);
    link_speed = 1000;
    configure_IEEE_phy_speed(sptr_emacps, phy_addr, link_speed);
    convspeeddupsetting = XEMACPS_GMII2RGMII_SPEED1000_FD;
    delay_s(1);

#elif defined(CONFIG_LINKSPEED100)
    XEmacPs_SetUpSLCRDivisors(sptr_emacps->Config.BaseAddress,100);
    link_speed = 100;
    configure_IEEE_phy_speed(sptr_emacps, phy_addr, link_speed);
    convspeeddupsetting = XEMACPS_GMII2RGMII_SPEED100_FD;
    delay_s(1);

#elif defined(CONFIG_LINKSPEED10)
    XEmacPs_SetUpSLCRDivisors(sptr_emacps->Config.BaseAddress,10);
    link_speed = 10;
    configure_IEEE_phy_speed(sptr_emacps, phy_addr, link_speed);
    convspeeddupsetting = XEMACPS_GMII2RGMII_SPEED10_FD;
    delay_s(1);
#endif

    if (conv_present) 
    {
        XEmacPs_PhyWrite(sptr_emacps, convphyaddr,
                XEMACPS_GMII2RGMII_REG_NUM, convspeeddupsetting);
    }

    print_debug("link speed for phy address %d: %d\r\n", phy_addr, link_speed);
    return link_speed;
}

/*!< ------------------------------------------------------------------------- */
/*!
 * @brief   stop emac
 * @param   sptr_emacps: emac structure pointer
 * @retval  errno
 */
kint32_t XEmacPs_Stop(XEmacPs *sptr_emacps)
{
    kuint32_t Reg;

    if ((!sptr_emacps) ||
        (!sptr_emacps->IsReady))
        return -ER_NREADY;

    /*!< Disable all interrupts */
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, 
                        XEMACPS_IDR_OFFSET, XEMACPS_IXR_ALL_MASK);

    /*!< Disable the receiver & transmitter */
    Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET);
    Reg &= (kuint32_t)(~XEMACPS_NWCTRL_RXEN_MASK);
    Reg &= (kuint32_t)(~XEMACPS_NWCTRL_TXEN_MASK);
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET, Reg);

    /*!< Mark as stopped */
    sptr_emacps->IsStarted = false;
    return ER_NORMAL;
}

/*!
 * @brief   set queue address to register
 * @param   sptr_emacps: emac structure pointer
 * @param   QPtr: queue base address
 * @param   QueueNum: queue length
 * @param   Direction: mark whether it is sending queue or recving queue
 * @retval  errno
 */
kint32_t XEmacPs_SetQueuePtr(XEmacPs *sptr_emacps, kuint32_t QPtr, kuint8_t QueueNum, kuint16_t Direction)
{
    /*!< Assert bad arguments and conditions */
    if ((!sptr_emacps) ||
        (!sptr_emacps->IsReady))
        return -ER_NREADY;

    /*!< If already started, then there is nothing to do */
    if (sptr_emacps->IsStarted)
         return ER_NORMAL;

    if (QueueNum == 0x00U) 
    {
        if (Direction == XEMACPS_SEND)
            XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, 
                            XEMACPS_TXQBASE_OFFSET, (QPtr & ULONG64_LO_MASK));
        else
            XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, 
                            XEMACPS_RXQBASE_OFFSET, (QPtr & ULONG64_LO_MASK));
    }
    else
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, 
                            XEMACPS_TXQ1BASE_OFFSET, (QPtr & ULONG64_LO_MASK));

    return ER_NORMAL;
}

/*!
 * @brief   set mac address to register
 * @param   sptr_emacps: emac structure pointer
 * @param   AddressPtr: mac address
 * @param   Index: mac index (0 ~ 5)
 * @retval  errno
 */
kint32_t XEmacPs_SetMacAddress(XEmacPs *sptr_emacps, void *AddressPtr, kuint8_t Index)
{
    kuint32_t MacAddr;
    kuint8_t *Aptr = (kuint8_t *)AddressPtr;
    kuint8_t IndexLoc = Index;

    if ((!sptr_emacps) ||
        (!Aptr) ||
        (!sptr_emacps->IsReady) ||
        (!Index) ||
        (IndexLoc > (kuint8_t)XEMACPS_MAX_TYPE_ID))
        return -ER_NREADY;

    /*!< Be sure device has been stopped */
    if (sptr_emacps->IsStarted)
        return -ER_LOCKED;

    /*!< Index ranges 1 to 4, for offset calculation is 0 to 3. */
    IndexLoc--;

    /*!< Set the MAC bits [31:0] in BOT */
    MacAddr = *(Aptr);
    MacAddr |= ((kuint32_t)(*(Aptr + 1)) << 8U);
    MacAddr |= ((kuint32_t)(*(Aptr + 2)) << 16U);
    MacAddr |= ((kuint32_t)(*(Aptr + 3)) << 24U);
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress,
                ((kuint32_t)XEMACPS_LADDR1L_OFFSET + ((kuint32_t)IndexLoc * (kuint32_t)8)), MacAddr);

    /*!< There are reserved bits in TOP so don't affect them */
    MacAddr = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress,
                    ((kuint32_t)XEMACPS_LADDR1H_OFFSET + ((kuint32_t)IndexLoc * (kuint32_t)8)));

    MacAddr &= (kuint32_t)(~XEMACPS_LADDR_MACH_MASK);

    /*!< Set MAC bits [47:32] in TOP */
    MacAddr |= (kuint32_t)(*(Aptr + 4));
    MacAddr |= (kuint32_t)(*(Aptr + 5)) << 8U;

    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress,
                ((kuint32_t)XEMACPS_LADDR1H_OFFSET + ((kuint32_t)IndexLoc * (kuint32_t)8)), MacAddr);

    return ER_NORMAL;
}

/*!
 * @brief   set ID MATCH register
 * @param   sptr_emacps: emac structure pointer
 * @param   Id_Check: ID number
 * @param   Index: ID MATCH reg's number
 * @retval  errno
 */
kint32_t XEmacPs_SetTypeIdCheck(XEmacPs *sptr_emacps, kuint32_t Id_Check, kuint8_t Index)
{
    kuint8_t IndexLoc = Index;

    if ((!sptr_emacps) ||
        (!sptr_emacps->IsReady) ||
        (!Index) ||
        (IndexLoc > (kuint8_t)XEMACPS_MAX_TYPE_ID))
        return -ER_NREADY;

    /*!< Be sure device has been stopped */
    if (sptr_emacps->IsStarted)
        return -ER_LOCKED;

    /*!< Index ranges 1 to 4, for offset calculation is 0 to 3. */
    IndexLoc--;

    /*!< Set the ID bits in MATCHx register */
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress,
                ((kuint32_t)XEMACPS_MATCH1_OFFSET + ((kuint32_t)IndexLoc * (kuint32_t)4)), Id_Check);

    return ER_NORMAL;
}

/*!
 * @brief   set network cfg register
 * @param   sptr_emacps: emac structure pointer
 * @param   Divisor: freq divisor
 * @retval  errno
 */
kint32_t XEmacPs_SetMdioDivisor(XEmacPs *sptr_emacps, XEmacPs_MdcDiv Divisor)
{
    kuint32_t Reg;

    if ((!sptr_emacps) ||
        (!sptr_emacps->IsReady) ||
        (Divisor > (XEmacPs_MdcDiv)0x7))	            /*!< only last three bits are valid */
        return -ER_NREADY;

    Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCFG_OFFSET);

    /*!< clear these three bits, could be done with mask */
    Reg &= (kuint32_t)(~XEMACPS_NWCFG_MDCCLKDIV_MASK);
    Reg |= ((kuint32_t)Divisor << XEMACPS_NWCFG_MDC_SHIFT_MASK);

    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCFG_OFFSET, Reg);
    return ER_NORMAL;
}

/*!
 * @brief   set emac operating speed
 * @param   sptr_emacps: emac structure pointer
 * @param   Speed: phy speed
 * @retval  errno
 */
kint32_t XEmacPs_SetOperatingSpeed(XEmacPs *sptr_emacps, kuint16_t Speed)
{
    kuint32_t Reg, bps_mask = 0;

    if ((!sptr_emacps) ||
        (!sptr_emacps->IsReady))
        return -ER_NREADY;

    switch (Speed) 
    {
        case 10:
            break;

        case 100:
            bps_mask |= XEMACPS_NWCFG_100_MASK;
            break;

        case 1000:
            bps_mask |= XEMACPS_NWCFG_1000_MASK;
            break;

        default:
            return -ER_NSUPPORT;
    }

    Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCFG_OFFSET);
    Reg &= (kuint32_t)(~(XEMACPS_NWCFG_1000_MASK | XEMACPS_NWCFG_100_MASK));

    /*!< Set register and return */
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCFG_OFFSET, Reg | bps_mask);
    
    return ER_NORMAL;
}

/*!
 * @brief   setup emac slcr divisor
 * @param   mac_baseaddr: emac base address
 * @param   Speed: phy speed
 * @retval  errno
 */
void XEmacPs_SetUpSLCRDivisors(kuint32_t mac_baseaddr, kint32_t speed)
{
    volatile kuint32_t slcrBaseAddress;
    kuint32_t SlcrDiv0 = 0;
    kuint32_t SlcrDiv1 = 0;
    kuint32_t SlcrTxClkCntrl;
    kuint32_t gigeversion;
    volatile kuint32_t CrlApbBaseAddr;
    kuint32_t CrlApbDiv0 = 0;
    kuint32_t CrlApbDiv1 = 0;
    kuint32_t CrlApbGemCtrl;

    gigeversion = ((mr_readl(mac_baseaddr + 0xFC)) >> 16) & 0xFFF;
    if (gigeversion == 2) 
    {
        *(volatile kuint32_t *)(SLCR_UNLOCK_ADDR) = SLCR_UNLOCK_KEY_VALUE;

        if (mac_baseaddr == ZYNQ_EMACPS_0_BASEADDR)
            slcrBaseAddress = SLCR_GEM0_CLK_CTRL_ADDR;
        else
            slcrBaseAddress = SLCR_GEM1_CLK_CTRL_ADDR;

        if ((*(volatile kuint32_t *)slcrBaseAddress) & SLCR_GEM_SRCSEL_EMIO)
            return;

        if (speed == 1000) 
        {
            if (mac_baseaddr == XPAR_XEMACPS_0_BASEADDR) 
            {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0
                SlcrDiv0 = XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0;
                SlcrDiv1 = XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV1;
#endif
            } 
            else 
            {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV0
                SlcrDiv0 = XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV0;
                SlcrDiv1 = XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV1;
#endif
            }
        }
        else if (speed == 100) 
        {
            if (mac_baseaddr == XPAR_XEMACPS_0_BASEADDR) 
            {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV0
                SlcrDiv0 = XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV0;
                SlcrDiv1 = XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV1;
#endif
            } 
            else 
            {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_100MBPS_DIV0
                SlcrDiv0 = XPAR_PS7_ETHERNET_1_ENET_SLCR_100MBPS_DIV0;
                SlcrDiv1 = XPAR_PS7_ETHERNET_1_ENET_SLCR_100MBPS_DIV1;
#endif
            }
        } 
        else 
        {
            if (mac_baseaddr == XPAR_XEMACPS_0_BASEADDR) 
            {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV0
                SlcrDiv0 = XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV0;
                SlcrDiv1 = XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV1;
#endif
            } 
            else 
            {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_10MBPS_DIV0
                SlcrDiv0 = XPAR_PS7_ETHERNET_1_ENET_SLCR_10MBPS_DIV0;
                SlcrDiv1 = XPAR_PS7_ETHERNET_1_ENET_SLCR_10MBPS_DIV1;
#endif
            }
        }

        if (SlcrDiv0 != 0 && SlcrDiv1 != 0) 
        {
            SlcrTxClkCntrl = *(volatile kuint32_t *)(kuint32_t)(slcrBaseAddress);
            SlcrTxClkCntrl &= EMACPS_SLCR_DIV_MASK;
            SlcrTxClkCntrl |= (SlcrDiv1 << 20);
            SlcrTxClkCntrl |= (SlcrDiv0 << 8);
            *(volatile kuint32_t *)(kuint32_t)(slcrBaseAddress) = SlcrTxClkCntrl;
            *(volatile kuint32_t *)(SLCR_LOCK_ADDR) = SLCR_LOCK_KEY_VALUE;
        } 
        else
            print_debug("Clock Divisors incorrect - Please check\r\n");
    }
    else if (gigeversion > 2) 
    {
        /*!< Setup divisors in CRL_APB for Zynq Ultrascale+ MPSoC */
        if (mac_baseaddr == ZYNQMP_EMACPS_0_BASEADDR)
            CrlApbBaseAddr = CRL_APB_GEM0_REF_CTRL;
        else if (mac_baseaddr == ZYNQMP_EMACPS_1_BASEADDR)
            CrlApbBaseAddr = CRL_APB_GEM1_REF_CTRL;
        else if (mac_baseaddr == ZYNQMP_EMACPS_2_BASEADDR)
            CrlApbBaseAddr = CRL_APB_GEM2_REF_CTRL;
        else if (mac_baseaddr == ZYNQMP_EMACPS_3_BASEADDR)
            CrlApbBaseAddr = CRL_APB_GEM3_REF_CTRL;

        if (speed == 1000) 
        {
            if (mac_baseaddr == ZYNQMP_EMACPS_0_BASEADDR) 
            {
#ifdef XPAR_PSU_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0
                CrlApbDiv0 = XPAR_PSU_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0;
                CrlApbDiv1 = XPAR_PSU_ETHERNET_0_ENET_SLCR_1000MBPS_DIV1;
#endif
            } 
            else if (mac_baseaddr == ZYNQMP_EMACPS_1_BASEADDR) 
            {
#ifdef XPAR_PSU_ETHERNET_1_ENET_SLCR_1000MBPS_DIV0
                CrlApbDiv0 = XPAR_PSU_ETHERNET_1_ENET_SLCR_1000MBPS_DIV0;
                CrlApbDiv1 = XPAR_PSU_ETHERNET_1_ENET_SLCR_1000MBPS_DIV1;
#endif
            } 
            else if (mac_baseaddr == ZYNQMP_EMACPS_2_BASEADDR) 
            {
#ifdef XPAR_PSU_ETHERNET_2_ENET_SLCR_1000MBPS_DIV0
                CrlApbDiv0 = XPAR_PSU_ETHERNET_2_ENET_SLCR_1000MBPS_DIV0;
                CrlApbDiv1 = XPAR_PSU_ETHERNET_2_ENET_SLCR_1000MBPS_DIV1;
#endif
            } 
            else if (mac_baseaddr == ZYNQMP_EMACPS_3_BASEADDR) 
            {
#ifdef XPAR_PSU_ETHERNET_3_ENET_SLCR_1000MBPS_DIV0
                CrlApbDiv0 = XPAR_PSU_ETHERNET_3_ENET_SLCR_1000MBPS_DIV0;
                CrlApbDiv1 = XPAR_PSU_ETHERNET_3_ENET_SLCR_1000MBPS_DIV1;
#endif
            }
        } 
        else if (speed == 100) 
        {
            if (mac_baseaddr == ZYNQMP_EMACPS_0_BASEADDR) 
            {
#ifdef XPAR_PSU_ETHERNET_0_ENET_SLCR_100MBPS_DIV0
                CrlApbDiv0 = XPAR_PSU_ETHERNET_0_ENET_SLCR_100MBPS_DIV0;
                CrlApbDiv1 = XPAR_PSU_ETHERNET_0_ENET_SLCR_100MBPS_DIV1;
#endif
            } 
            else if (mac_baseaddr == ZYNQMP_EMACPS_1_BASEADDR) 
            {
#ifdef XPAR_PSU_ETHERNET_1_ENET_SLCR_100MBPS_DIV0
                CrlApbDiv0 = XPAR_PSU_ETHERNET_1_ENET_SLCR_100MBPS_DIV0;
                CrlApbDiv1 = XPAR_PSU_ETHERNET_1_ENET_SLCR_100MBPS_DIV1;
#endif
            } 
            else if (mac_baseaddr == ZYNQMP_EMACPS_2_BASEADDR) 
            {
#ifdef XPAR_PSU_ETHERNET_2_ENET_SLCR_100MBPS_DIV0
                CrlApbDiv0 = XPAR_PSU_ETHERNET_2_ENET_SLCR_100MBPS_DIV0;
                CrlApbDiv1 = XPAR_PSU_ETHERNET_2_ENET_SLCR_100MBPS_DIV1;
#endif
            } 
            else if (mac_baseaddr == ZYNQMP_EMACPS_3_BASEADDR) 
            {
#ifdef XPAR_PSU_ETHERNET_3_ENET_SLCR_100MBPS_DIV0
                CrlApbDiv0 = XPAR_PSU_ETHERNET_3_ENET_SLCR_100MBPS_DIV0;
                CrlApbDiv1 = XPAR_PSU_ETHERNET_3_ENET_SLCR_100MBPS_DIV1;
#endif
            }
        } 
        else 
        {
            if (mac_baseaddr == ZYNQMP_EMACPS_0_BASEADDR) 
            {
#ifdef XPAR_PSU_ETHERNET_0_ENET_SLCR_10MBPS_DIV0
                CrlApbDiv0 = XPAR_PSU_ETHERNET_0_ENET_SLCR_10MBPS_DIV0;
                CrlApbDiv1 = XPAR_PSU_ETHERNET_0_ENET_SLCR_10MBPS_DIV1;
#endif
            } 
            else if (mac_baseaddr == ZYNQMP_EMACPS_1_BASEADDR) 
            {
#ifdef XPAR_PSU_ETHERNET_1_ENET_SLCR_10MBPS_DIV0
                CrlApbDiv0 = XPAR_PSU_ETHERNET_1_ENET_SLCR_10MBPS_DIV0;
                CrlApbDiv1 = XPAR_PSU_ETHERNET_1_ENET_SLCR_10MBPS_DIV1;
#endif
            } 
            else if (mac_baseaddr == ZYNQMP_EMACPS_2_BASEADDR) 
            {
#ifdef XPAR_PSU_ETHERNET_2_ENET_SLCR_10MBPS_DIV0
                CrlApbDiv0 = XPAR_PSU_ETHERNET_2_ENET_SLCR_10MBPS_DIV0;
                CrlApbDiv1 = XPAR_PSU_ETHERNET_2_ENET_SLCR_10MBPS_DIV1;
#endif
            } 
            else if (mac_baseaddr == ZYNQMP_EMACPS_3_BASEADDR) 
            {
#ifdef XPAR_PSU_ETHERNET_3_ENET_SLCR_10MBPS_DIV0
                CrlApbDiv0 = XPAR_PSU_ETHERNET_3_ENET_SLCR_10MBPS_DIV0;
                CrlApbDiv1 = XPAR_PSU_ETHERNET_3_ENET_SLCR_10MBPS_DIV1;
#endif
            }
        }

        if (CrlApbDiv0 != 0 && CrlApbDiv1 != 0) 
        {
        #if (defined(EL1_NONSECURE) && (EL1_NONSECURE))
            XSmc_OutVar RegRead;
            RegRead = Xil_Smc(MMIO_READ_SMC_FID, (kuint64_t)(CrlApbBaseAddr),
                                0, 0, 0, 0, 0, 0);
            CrlApbGemCtrl = RegRead.Arg0 >> 32;
        #else
            CrlApbGemCtrl = *(volatile kuint32_t *)(kuint32_t)(CrlApbBaseAddr);
        #endif

            CrlApbGemCtrl &= ~CRL_APB_GEM_DIV0_MASK;
            CrlApbGemCtrl |= CrlApbDiv0 << CRL_APB_GEM_DIV0_SHIFT;
            CrlApbGemCtrl &= ~CRL_APB_GEM_DIV1_MASK;
            CrlApbGemCtrl |= CrlApbDiv1 << CRL_APB_GEM_DIV1_SHIFT;

        #if (defined(EL1_NONSECURE) && (EL1_NONSECURE))
            Xil_Smc(MMIO_WRITE_SMC_FID, (kuint64_t)(CrlApbBaseAddr) | ((kuint64_t)(0xFFFFFFFF) << 32),
                    (kuint64_t)CrlApbGemCtrl, 0, 0, 0, 0, 0);
            do {
                RegRead = Xil_Smc(MMIO_READ_SMC_FID, (kuint64_t)(CrlApbBaseAddr), 0, 0, 0, 0, 0, 0);
            } while((RegRead.Arg0 >> 32) != CrlApbGemCtrl);
        #else
            *(volatile kuint32_t *)(kuint32_t)(CrlApbBaseAddr) = CrlApbGemCtrl;
        #endif
        } 
        else
            print_debug("Clock Divisors incorrect - Please check\r\n");
    }
}

/*!
 * @brief   set network cfg register
 * @param   sptr_emacps: emac structure pointer
 * @param   Options: value
 * @retval  errno
 */
kint32_t XEmacPs_SetOptions(XEmacPs *sptr_emacps, kuint32_t Options)
{
    kuint32_t Reg;		                                /*!< Generic register contents */
    kuint32_t RegNetCfg;		                        /*!< Reflects original contents of NET_CONFIG */
    kuint32_t RegNewNetCfg;	                            /*!< Reflects new contents of NET_CONFIG */

    if ((!sptr_emacps) ||
        (!sptr_emacps->IsReady))
        return -ER_NREADY;

    /*!< Be sure device has been stopped */
    if (sptr_emacps->IsStarted)
        return -ER_LOCKED;

    /*!< 
     * Many of these options will change the NET_CONFIG registers.
     * To reduce the amount of IO to the device, group these options here
     * and change them all at once.
     */

    /*!< Grab current register contents */
    RegNetCfg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCFG_OFFSET);
    RegNewNetCfg = RegNetCfg;

    /*!< It is configured to max 1536. */
    if ((Options & XEMACPS_FRAME1536_OPTION) != 0x00000000U)
        RegNewNetCfg |= (XEMACPS_NWCFG_1536RXEN_MASK);

    /*!< Turn on VLAN packet only, only VLAN tagged will be accepted */
    if ((Options & XEMACPS_VLAN_OPTION) != 0x00000000U)
        RegNewNetCfg |= XEMACPS_NWCFG_NVLANDISC_MASK;

    /*!< Turn on FCS stripping on receive packets */
    if ((Options & XEMACPS_FCS_STRIP_OPTION) != 0x00000000U)
        RegNewNetCfg |= XEMACPS_NWCFG_FCSREM_MASK;

    /*!< Turn on length/type field checking on receive packets */
    if ((Options & XEMACPS_LENTYPE_ERR_OPTION) != 0x00000000U)
        RegNewNetCfg |= XEMACPS_NWCFG_LENERRDSCRD_MASK;

    /*!< Turn on flow control */
    if ((Options & XEMACPS_FLOW_CONTROL_OPTION) != 0x00000000U)
        RegNewNetCfg |= XEMACPS_NWCFG_PAUSEEN_MASK;

    /*!< Turn on promiscuous frame filtering (all frames are received) */
    if ((Options & XEMACPS_PROMISC_OPTION) != 0x00000000U)
        RegNewNetCfg |= XEMACPS_NWCFG_COPYALLEN_MASK;

    /*!< Allow broadcast address reception */
    if ((Options & XEMACPS_BROADCAST_OPTION) != 0x00000000U)
        RegNewNetCfg &= (kuint32_t)(~XEMACPS_NWCFG_BCASTDI_MASK);

    /*!< Allow multicast address filtering */
    if ((Options & XEMACPS_MULTICAST_OPTION) != 0x00000000U)
        RegNewNetCfg |= XEMACPS_NWCFG_MCASTHASHEN_MASK;

    /*!< enable RX checksum offload */
    if ((Options & XEMACPS_RX_CHKSUM_ENABLE_OPTION) != 0x00000000U)
        RegNewNetCfg |= XEMACPS_NWCFG_RXCHKSUMEN_MASK;

    /*!< Enable jumbo frames */
    if (((Options & XEMACPS_JUMBO_ENABLE_OPTION) != 0x00000000U) &&
        (sptr_emacps->Version > 2)) 
    {
        RegNewNetCfg |= XEMACPS_NWCFG_JUMBO_MASK;
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress,
                XEMACPS_JUMBOMAXLEN_OFFSET, XEMACPS_RX_BUF_SIZE_JUMBO);
        
        Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_DMACR_OFFSET);
        Reg &= ~XEMACPS_DMACR_RXBUF_MASK;
        Reg |= (((((kuint32_t)XEMACPS_RX_BUF_SIZE_JUMBO / (kuint32_t)XEMACPS_RX_BUF_UNIT) +
                (((((kuint32_t)XEMACPS_RX_BUF_SIZE_JUMBO %
                (kuint32_t)XEMACPS_RX_BUF_UNIT))!=(kuint32_t)0) ? 1U : 0U)) <<
                (kuint32_t)(XEMACPS_DMACR_RXBUF_SHIFT)) &
                (kuint32_t)(XEMACPS_DMACR_RXBUF_MASK));

        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_DMACR_OFFSET, Reg);

        sptr_emacps->MaxMtuSize = XEMACPS_MTU_JUMBO;
        sptr_emacps->MaxFrameSize = XEMACPS_MTU_JUMBO + XEMACPS_HDR_SIZE + XEMACPS_TRL_SIZE;
        sptr_emacps->MaxVlanFrameSize = sptr_emacps->MaxFrameSize + XEMACPS_HDR_VLAN_SIZE;
        sptr_emacps->RxBufMask = XEMACPS_RXBUF_LEN_JUMBO_MASK;
    }

    if (((Options & XEMACPS_SGMII_ENABLE_OPTION) != 0x00000000U) &&
        (sptr_emacps->Version > 2)) 
        RegNewNetCfg |= (XEMACPS_NWCFG_SGMIIEN_MASK | XEMACPS_NWCFG_PCSSEL_MASK);

    /*!< Officially change the NET_CONFIG registers if it needs to be modified. */
    if (RegNetCfg != RegNewNetCfg)
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCFG_OFFSET, RegNewNetCfg);

    /*!< Enable TX checksum offload */
    if ((Options & XEMACPS_TX_CHKSUM_ENABLE_OPTION) != 0x00000000U) 
    {
        Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_DMACR_OFFSET);
        Reg |= XEMACPS_DMACR_TCPCKSUM_MASK;
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_DMACR_OFFSET, Reg);
    }

    /*!< Enable transmitter */
    if ((Options & XEMACPS_TRANSMITTER_ENABLE_OPTION) != 0x00000000U) 
    {
        Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET);
        Reg |= XEMACPS_NWCTRL_TXEN_MASK;
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET, Reg);
    }

    /*!< Enable receiver */
    if ((Options & XEMACPS_RECEIVER_ENABLE_OPTION) != 0x00000000U) 
    {
        Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET);
        Reg |= XEMACPS_NWCTRL_RXEN_MASK;
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET, Reg);
    }

    /*!< 
     * The remaining options not handled here are managed elsewhere in the
     * driver. No register modifications are needed at this time. Reflecting
     * the option in sptr_emacps->Options is good enough for now.
     */

    /*!< Set options word to its new value */
    sptr_emacps->Options |= Options;

    return ER_NORMAL;
}

/*!
 * @brief   reset network cfg register
 * @param   sptr_emacps: emac structure pointer
 * @param   Options: value
 * @retval  errno
 */
kint32_t XEmacPs_ClearOptions(XEmacPs *sptr_emacps, kuint32_t Options)
{
    kuint32_t Reg;		                                /*!< Generic */
    kuint32_t RegNetCfg;		                        /*!< Reflects original contents of NET_CONFIG */
    kuint32_t RegNewNetCfg;	                            /*!< Reflects new contents of NET_CONFIG */

    if ((!sptr_emacps) ||
        (!sptr_emacps->IsReady))
        return -ER_NREADY;

    /*!< Be sure device has been stopped */
    if (sptr_emacps->IsStarted)
        return -ER_LOCKED;

    /*!< 
     * Many of these options will change the NET_CONFIG registers.
     * To reduce the amount of IO to the device, group these options here
     * and change them all at once.
     */

    /*!< Grab current register contents */
    RegNetCfg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress,
                      XEMACPS_NWCFG_OFFSET);
    RegNewNetCfg = RegNetCfg;

    /*!< 
     * There is only RX configuration!?
     * It is configured in two different length, upto 1536 and 10240 bytes
     */
    if ((Options & XEMACPS_FRAME1536_OPTION) != 0x00000000U)
        RegNewNetCfg &= (kuint32_t)(~XEMACPS_NWCFG_1536RXEN_MASK);

    /*!< Turn off VLAN packet only */
    if ((Options & XEMACPS_VLAN_OPTION) != 0x00000000U)
        RegNewNetCfg &= (kuint32_t)(~XEMACPS_NWCFG_NVLANDISC_MASK);

    /*!< Turn off FCS stripping on receive packets */
    if ((Options & XEMACPS_FCS_STRIP_OPTION) != 0x00000000U)
        RegNewNetCfg &= (kuint32_t)(~XEMACPS_NWCFG_FCSREM_MASK);

    /*!< Turn off length/type field checking on receive packets */
    if ((Options & XEMACPS_LENTYPE_ERR_OPTION) != 0x00000000U)
        RegNewNetCfg &= (kuint32_t)(~XEMACPS_NWCFG_LENERRDSCRD_MASK);

    /*!< Turn off flow control */
    if ((Options & XEMACPS_FLOW_CONTROL_OPTION) != 0x00000000U)
        RegNewNetCfg &= (kuint32_t)(~XEMACPS_NWCFG_PAUSEEN_MASK);

    /*!< Turn off promiscuous frame filtering (all frames are received) */
    if ((Options & XEMACPS_PROMISC_OPTION) != 0x00000000U)
        RegNewNetCfg &= (kuint32_t)(~XEMACPS_NWCFG_COPYALLEN_MASK);

    /*!< Disallow broadcast address filtering => broadcast reception */
    if ((Options & XEMACPS_BROADCAST_OPTION) != 0x00000000U)
        RegNewNetCfg |= XEMACPS_NWCFG_BCASTDI_MASK;

    /*!< Disallow multicast address filtering */
    if ((Options & XEMACPS_MULTICAST_OPTION) != 0x00000000U)
        RegNewNetCfg &= (kuint32_t)(~XEMACPS_NWCFG_MCASTHASHEN_MASK);

    /*!< Disable RX checksum offload */
    if ((Options & XEMACPS_RX_CHKSUM_ENABLE_OPTION) != 0x00000000U)
        RegNewNetCfg &= (kuint32_t)(~XEMACPS_NWCFG_RXCHKSUMEN_MASK);

    /*!< Disable jumbo frames */
    if (((Options & XEMACPS_JUMBO_ENABLE_OPTION) != 0x00000000U) &&
        (sptr_emacps->Version > 2))
    {
        RegNewNetCfg &= (kuint32_t)(~XEMACPS_NWCFG_JUMBO_MASK);
        Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_DMACR_OFFSET);

        Reg &= ~XEMACPS_DMACR_RXBUF_MASK;
        Reg |= (((((kuint32_t)XEMACPS_RX_BUF_SIZE / (kuint32_t)XEMACPS_RX_BUF_UNIT) +
            (((((kuint32_t)XEMACPS_RX_BUF_SIZE %
            (kuint32_t)XEMACPS_RX_BUF_UNIT))!=(kuint32_t)0) ? 1U : 0U)) <<
            (kuint32_t)(XEMACPS_DMACR_RXBUF_SHIFT)) &
            (kuint32_t)(XEMACPS_DMACR_RXBUF_MASK));

        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_DMACR_OFFSET, Reg);

        sptr_emacps->MaxMtuSize = XEMACPS_MTU;
        sptr_emacps->MaxFrameSize = XEMACPS_MTU + XEMACPS_HDR_SIZE + XEMACPS_TRL_SIZE;
        sptr_emacps->MaxVlanFrameSize = sptr_emacps->MaxFrameSize + XEMACPS_HDR_VLAN_SIZE;
        sptr_emacps->RxBufMask = XEMACPS_RXBUF_LEN_MASK;
    }

    if (((Options & XEMACPS_SGMII_ENABLE_OPTION) != 0x00000000U) &&
        (sptr_emacps->Version > 2))
        RegNewNetCfg &= (kuint32_t)(~(XEMACPS_NWCFG_SGMIIEN_MASK | XEMACPS_NWCFG_PCSSEL_MASK));

    /*!< Officially change the NET_CONFIG registers if it needs to be modified. */
    if (RegNetCfg != RegNewNetCfg)
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress,
                   XEMACPS_NWCFG_OFFSET, RegNewNetCfg);

    /*!< Disable TX checksum offload */
    if ((Options & XEMACPS_TX_CHKSUM_ENABLE_OPTION) != 0x00000000U) 
    {
        Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_DMACR_OFFSET);
        Reg &= (kuint32_t)(~XEMACPS_DMACR_TCPCKSUM_MASK);
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_DMACR_OFFSET, Reg);
    }

    /*!< Disable transmitter */
    if ((Options & XEMACPS_TRANSMITTER_ENABLE_OPTION) != 0x00000000U) 
    {
        Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET);
        Reg &= (kuint32_t)(~XEMACPS_NWCTRL_TXEN_MASK);
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET, Reg);
    }

    /*!< Disable receiver */
    if ((Options & XEMACPS_RECEIVER_ENABLE_OPTION) != 0x00000000U) 
    {
        Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET);
        Reg &= (kuint32_t)(~XEMACPS_NWCTRL_RXEN_MASK);
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET, Reg);
    }

    /*!< 
     * The remaining options not handled here are managed elsewhere in the
     * driver. No register modifications are needed at this time. Reflecting
     * option in sptr_emacps->Options is good enough for now.
     */

    /*!< Set options word to its new value */
    sptr_emacps->Options &= ~Options;

    return ER_NORMAL;
}

/*!
 * @brief   clear emac hash
 * @param   sptr_emacps: emac structure pointer
 * @retval  errno
 */
kint32_t XEmacPs_ClearHash(XEmacPs *sptr_emacps)
{
    if ((!sptr_emacps) ||
        (!sptr_emacps->IsReady))
        return -ER_NREADY;

    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress,
                    XEMACPS_HASHL_OFFSET, 0x0U);

    /*!< write bits [63:32] in TOP */
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress,
                    XEMACPS_HASHH_OFFSET, 0x0U);

    return ER_NORMAL;
}

/*!
 * @brief   reset emac
 * @param   sptr_emacps: emac structure pointer
 * @retval  errno
 */
kint32_t XEmacPs_Reset(XEmacPs *sptr_emacps)
{
    kuint32_t Reg;
    kuint8_t i;
    kint8_t EmacPs_zero_MAC[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

    if ((!sptr_emacps) ||
        (!sptr_emacps->IsReady))
        return -ER_NREADY;

    /*!< Stop the device and reset hardware */
    XEmacPs_Stop(sptr_emacps);

    sptr_emacps->Options = XEMACPS_DEFAULT_OPTIONS;
    sptr_emacps->Version = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, 0xFC);
    sptr_emacps->Version = (sptr_emacps->Version >> 16) & 0xFFF;
    sptr_emacps->MaxMtuSize = XEMACPS_MTU;
    sptr_emacps->MaxFrameSize = XEMACPS_MTU + XEMACPS_HDR_SIZE + XEMACPS_TRL_SIZE;
    sptr_emacps->MaxVlanFrameSize = sptr_emacps->MaxFrameSize + XEMACPS_HDR_VLAN_SIZE;
    sptr_emacps->RxBufMask = XEMACPS_RXBUF_LEN_MASK;

    /*!< Setup hardware with default values */
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress,
            XEMACPS_NWCTRL_OFFSET,
            (XEMACPS_NWCTRL_STATCLR_MASK |
            XEMACPS_NWCTRL_MDEN_MASK) &
            (kuint32_t)(~XEMACPS_NWCTRL_LOOPEN_MASK));

    Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCFG_OFFSET);
    Reg &= XEMACPS_NWCFG_MDCCLKDIV_MASK;
    Reg |= ((kuint32_t)XEMACPS_NWCFG_100_MASK | (kuint32_t)XEMACPS_NWCFG_FDEN_MASK | (kuint32_t)XEMACPS_NWCFG_UCASTHASHEN_MASK);
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCFG_OFFSET, Reg);

    if (sptr_emacps->Version > 2) 
    {
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCFG_OFFSET,
            (XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCFG_OFFSET) |
                XEMACPS_NWCFG_DWIDTH_64_MASK));
    }

    /*!< dma control reg */
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_DMACR_OFFSET,
            (((((kuint32_t)XEMACPS_RX_BUF_SIZE / (kuint32_t)XEMACPS_RX_BUF_UNIT) +
            (((((kuint32_t)XEMACPS_RX_BUF_SIZE % (kuint32_t)XEMACPS_RX_BUF_UNIT)) != (kuint32_t)0) ? 1U : 0U)) <<
            (kuint32_t)(XEMACPS_DMACR_RXBUF_SHIFT)) &
            (kuint32_t)(XEMACPS_DMACR_RXBUF_MASK)) |
            (kuint32_t)XEMACPS_DMACR_RXSIZE_MASK |
            (kuint32_t)XEMACPS_DMACR_TXSIZE_MASK);
    
    if (sptr_emacps->Version > 2) 
    {
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_DMACR_OFFSET,
            (XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_DMACR_OFFSET) |
            (kuint32_t)XEMACPS_DMACR_INCR16_AHB_BURST));
    }

    /*!< clear send status reg */
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress,
               XEMACPS_TXSR_OFFSET, 0x0U);

    /*!< set queue ptr (tx and rx) */
    XEmacPs_SetQueuePtr(sptr_emacps, 0, 0x00U, (kuint16_t)XEMACPS_SEND);
    if (sptr_emacps->Version > 2)
        XEmacPs_SetQueuePtr(sptr_emacps, 0, 0x01U, (kuint16_t)XEMACPS_SEND);
    XEmacPs_SetQueuePtr(sptr_emacps, 0, 0x00U, (kuint16_t)XEMACPS_RECV);

    /*!< clear recv status reg */
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress,
               XEMACPS_RXSR_OFFSET, 0x0U);

    /*!< disable interrupt reg*/
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_IDR_OFFSET,
               XEMACPS_IXR_ALL_MASK);

    /*!< clear interrupt status reg (write 1 to clear)*/
    Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_ISR_OFFSET);
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_ISR_OFFSET, Reg);

    XEmacPs_ClearHash(sptr_emacps);

    /*!< clear mac and id register*/
    for (i = 1U; i < 5U; i++) 
    {
        XEmacPs_SetMacAddress(sptr_emacps, EmacPs_zero_MAC, i);
        XEmacPs_SetTypeIdCheck(sptr_emacps, 0x00000000U, i);
    }

    /*!< clear all counters */
    for (i = 0U; i < (kuint8_t)((XEMACPS_LAST_OFFSET - XEMACPS_OCTTXL_OFFSET) / 4U); i++)
        XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress,
                            XEMACPS_OCTTXL_OFFSET + (kuint32_t)(((kuint32_t)i) * ((kuint32_t)4)));

    /*!< Disable the receiver */
    Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET);
    Reg &= (kuint32_t)(~XEMACPS_NWCTRL_RXEN_MASK);
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET, Reg);

    /*!< 
     * Sync default options with hardware but leave receiver and
     * transmitter disabled. They get enabled with XEmacPs_Start() if
     * XEMACPS_TRANSMITTER_ENABLE_OPTION and
     * XEMACPS_RECEIVER_ENABLE_OPTION are set.
     */
    XEmacPs_SetOptions(sptr_emacps, sptr_emacps->Options &
                ~((kuint32_t)XEMACPS_TRANSMITTER_ENABLE_OPTION |
                  (kuint32_t)XEMACPS_RECEIVER_ENABLE_OPTION));

    XEmacPs_ClearOptions(sptr_emacps, ~sptr_emacps->Options);

    return ER_NORMAL;
}

/*!
 * @brief   start emac
 * @param   sptr_emacps: emac structure pointer
 * @retval  errno
 */
kint32_t XEmacPs_Start(XEmacPs *sptr_emacps)
{
    kuint32_t Reg;

    /*!< Assert bad arguments and conditions */
    if ((!sptr_emacps) ||
        (!sptr_emacps->IsReady))
        return -ER_NREADY;

    /*!< Start DMA */
    /*!< 
     * When starting the DMA channels, both transmit and receive sides
     * need an initialized BD list.
     */
    if (sptr_emacps->Version == 2) 
    {
        if ((!sptr_emacps->RxBdRing.BaseBdAddr) ||
            (!sptr_emacps->TxBdRing.BaseBdAddr))
            return -ER_INVALID;

        /*!< dma transmit channels (tx/rx buffer) */
        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_RXQBASE_OFFSET,
                        sptr_emacps->RxBdRing.BaseBdAddr);

        XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_TXQBASE_OFFSET,
                        sptr_emacps->TxBdRing.BaseBdAddr);
    }

    /*!< clear any existed int status */
    XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress, XEMACPS_ISR_OFFSET, XEMACPS_IXR_ALL_MASK);

    /*!< Enable transmitter if not already enabled */
    if ((sptr_emacps->Options & (kuint32_t)XEMACPS_TRANSMITTER_ENABLE_OPTION) != 0x00000000U) 
    {
        Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET);
        if (!(Reg & XEMACPS_NWCTRL_TXEN_MASK))
        {
            XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress,
                        XEMACPS_NWCTRL_OFFSET,
                        Reg | (kuint32_t)XEMACPS_NWCTRL_TXEN_MASK);
        }
    }

    /*!< Enable receiver if not already enabled */
    if ((sptr_emacps->Options & XEMACPS_RECEIVER_ENABLE_OPTION) != 0x00000000U) 
    {
        Reg = XEmacPs_ReadReg(sptr_emacps->Config.BaseAddress, XEMACPS_NWCTRL_OFFSET);
        if (!(Reg & XEMACPS_NWCTRL_RXEN_MASK)) 
        {
            XEmacPs_WriteReg(sptr_emacps->Config.BaseAddress,
                        XEMACPS_NWCTRL_OFFSET,
                        Reg | (kuint32_t)XEMACPS_NWCTRL_RXEN_MASK);
        }
    }

    /*!< Enable TX and RX interrupts */
    XEmacPs_IntEnable(sptr_emacps, 
            (XEMACPS_IXR_TX_ERR_MASK | XEMACPS_IXR_RX_ERR_MASK | 
            (kuint32_t)XEMACPS_IXR_FRAMERX_MASK | (kuint32_t)XEMACPS_IXR_TXCOMPL_MASK));

    /*!< Enable TX Q1 Interrupts */
    if (sptr_emacps->Version > 2)
        XEmacPs_IntQ1Enable(sptr_emacps, XEMACPS_INTQ1_IXR_ALL_MASK);

    /*!< Mark as started */
    sptr_emacps->IsStarted = true;
    return ER_NORMAL;
}

/*!< ------------------------------------------------------------------------------ */
/*!
 * @brief   search corresponding config
 * @param   DeviceId: the number of emac config table
 * @retval  emac config address
 */
XEmacPs_Config *XEmacPs_LookupConfig(kuint16_t DeviceId)
{
    XEmacPs_Config *sptr_config = mr_nullptr;
    kuint32_t i;

    for (i = 0U; i < (kuint32_t)XPAR_XEMACPS_NUM_INSTANCES; i++) 
    {
        if (sgtc_xmacps_configTable[i].DeviceId == DeviceId) 
        {
            sptr_config = &sgtc_xmacps_configTable[i];
            break;
        }
    }

    return sptr_config;
}

/*!
 * @brief   initialize emac configuation
 * @param   sptr_emacps: emac structure pointer
 * @param   sptr_config: emac configuration structure
 * @param   EffectiveAddress: emac physical address
 * @retval  errno
 */
kint32_t XEmacPs_CfgInitialize(XEmacPs *sptr_emacps, XEmacPs_Config *sptr_config, kuint32_t EffectiveAddress)
{
    /*!< Verify arguments */
    if ((!sptr_emacps) ||
        (!sptr_config))
        return -ER_NULLPTR;

    /*!< Set device base address and ID */
    sptr_emacps->Config.DeviceId = sptr_config->DeviceId;
    sptr_emacps->Config.BaseAddress = EffectiveAddress;
    sptr_emacps->Config.IsCacheCoherent = sptr_config->IsCacheCoherent;

    /*!< Set callbacks to an initial stub routine */
    sptr_emacps->SendHandler = (XEmacPs_Handler)mr_nullptr;
    sptr_emacps->RecvHandler = (XEmacPs_Handler)mr_nullptr;
    sptr_emacps->ErrorHandler = (XEmacPs_ErrHandler)mr_nullptr;

    /*!< Reset the hardware and set default options */
    sptr_emacps->IsReady = true;
    return XEmacPs_Reset(sptr_emacps);
}
