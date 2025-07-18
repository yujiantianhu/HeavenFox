/*
 * ZYNQ7 Peripheral APIs
 *
 * File Name:   periph.c
 * Author:      Yang Yujun (Copy from "Xilinx SDK")
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.06.20
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <configs/configs.h>
#include <arch/armv7/gcc_config.h>
#include <zynq7/zynq7_periph.h>
#include <common/time.h>
#include <common/api_string.h>

/*!< The globals */
static XGpioPs_Config sgtc_xgpio_ps_config_table[XPAR_XGPIOPS_NUM_INSTANCES] =
{
    {
        XPAR_PS7_GPIO_0_DEVICE_ID,
        XPAR_PS7_GPIO_0_BASEADDR
    }
};

static XScuTimer_Config sgtc_xscutimer_config_table[XPAR_XSCUTIMER_NUM_INSTANCES] =
{
    {
        XPAR_PS7_SCUTIMER_0_DEVICE_ID,
        XPAR_PS7_SCUTIMER_0_BASEADDR
    }
};

static XTtcPs_Config sgtc_xttcps_config_table[XPAR_XTTCPS_NUM_INSTANCES] =
{
    {
        XPAR_PS7_TTC_0_DEVICE_ID,
        XPAR_PS7_TTC_0_BASEADDR,
        XPAR_PS7_TTC_0_TTC_CLK_FREQ_HZ
    },
    {
        XPAR_PS7_TTC_1_DEVICE_ID,
        XPAR_PS7_TTC_1_BASEADDR,
        XPAR_PS7_TTC_1_TTC_CLK_FREQ_HZ
    },
    {
        XPAR_PS7_TTC_2_DEVICE_ID,
        XPAR_PS7_TTC_2_BASEADDR,
        XPAR_PS7_TTC_2_TTC_CLK_FREQ_HZ
    }
};

static XUartPs_Config sgtc_xuart_ps_config_table[XPAR_XUARTPS_NUM_INSTANCES] =
{
    {
        XPAR_PS7_UART_0_DEVICE_ID,
        XPAR_PS7_UART_0_BASEADDR,
        XPAR_PS7_UART_0_UART_CLK_FREQ_HZ,
        XPAR_PS7_UART_0_HAS_MODEM
    }
};

static XSdPs_Config sgtc_xsd_ps_config_table[XPAR_XSDPS_NUM_INSTANCES] =
{
    {
        XPAR_PS7_SD_0_DEVICE_ID,
        XPAR_PS7_SD_0_BASEADDR,
        XPAR_PS7_SD_0_SDIO_CLK_FREQ_HZ,
        XPAR_PS7_SD_0_HAS_CD,
        XPAR_PS7_SD_0_HAS_WP,
        XPAR_PS7_SD_0_BUS_WIDTH,
        XPAR_PS7_SD_0_MIO_BANK,
        XPAR_PS7_SD_0_HAS_EMIO,
        XPAR_PS7_SD_0_IS_CACHE_COHERENT
    }
};

/*!< API functions */
/*!
 * @brief   Sync L2 Cache
 * @param   none
 * @retval  none
 * @note    none
 */
void Xil_L2CacheSync(void)
{
    XSdPs_WriteReg(XPS_L2CC_BASEADDR, XPS_L2CC_CACHE_SYNC_OFFSET, 0x0U);
}

/*!
 * @brief   Flush L1 Cache
 * @param   none
 * @retval  none
 * @note    none
 */
void Xil_L1DCacheFlushLine(kuaddr_t adr)
{
    kuint32_t cache_val = 0U;

    mr_set_cp15(XREG_CP15_CACHE_SIZE_SEL, cache_val);

    cache_val = (kuint32_t)(adr & (~0x1FU));
    mr_set_cp15(XREG_CP15_CLEAN_INVAL_DC_LINE_MVA_POC, cache_val);

    /* Wait for L1 flush to complete */
    mr_dsb();
}

/*!
 * @brief   L2 Write Debug Ctrl
 * @param   none
 * @retval  none
 * @note    none
 */
void Xil_L2WriteDebugCtrl(kuint32_t Value)
{
    XSdPs_WriteReg(XPS_L2CC_BASEADDR, XPS_L2CC_DEBUG_CTRL_OFFSET, Value);
}

/*!
 * @brief   Flush L2 Cache
 * @param   none
 * @retval  none
 * @note    none
 */
void Xil_L2CacheFlushLine(kuaddr_t adr)
{
    XSdPs_WriteReg(XPS_L2CC_BASEADDR, XPS_L2CC_CACHE_CLEAN_PA_OFFSET, adr);
    XSdPs_WriteReg(XPS_L2CC_BASEADDR, XPS_L2CC_CACHE_INVLD_PA_OFFSET, adr);

    /* synchronize the processor */
    mr_dsb();
}

/*!
 * @brief   Flush D-Cache
 * @param   none
 * @retval  none
 * @note    none
 */
void Xil_DCacheFlushRange(kuaddr_t adr, kuint32_t len)
{
    kuint32_t LocalAddr = adr;
    const kuint32_t cacheline = 32U;
    kuint32_t end;
    kuint32_t currmask;
    volatile kuint32_t *L2CCOffset;

    L2CCOffset = (volatile kuint32_t *)(XPS_L2CC_BASEADDR + XPS_L2CC_CACHE_INV_CLN_PA_OFFSET);

    currmask = __get_cpsr();
    __set_cpsr(currmask | CPSR_BIT_I | CPSR_BIT_F);

    if (len) 
    {
        /* Back the starting address up to the start of a cache line
         * perform cache operations until adr+len
         */
        end = LocalAddr + len;
        LocalAddr &= ~(cacheline - 1U);

        while (LocalAddr < end) 
        {
            /* Flush L1 Data cache line */
            mr_set_cp15(XREG_CP15_CLEAN_INVAL_DC_LINE_MVA_POC, LocalAddr);

        #if (!defined(CONFIG_USE_AMP) || !CONFIG_USE_AMP)
            /* Flush L2 cache line */
            *L2CCOffset = LocalAddr;
           
            /*!< L2 Cache Sync */
            Xil_L2CacheSync();
        #endif
            LocalAddr += cacheline;
        }
    }

    mr_dsb();
    __set_cpsr(currmask);
}

/*!
 * @brief   Xil_DCacheInvalidateRange
 * @param   none
 * @retval  none
 * @note    none
 */
void Xil_DCacheInvalidateRange(kuaddr_t adr, kuint32_t len)
{
    const kuint32_t cacheline = 32U;
    kuint32_t end;
    kuint32_t tempadr = adr;
    kuint32_t tempend;
    kuint32_t currmask;
    volatile kuint32_t *L2CCOffset;
    kuint32_t cache_val = 0U;

    L2CCOffset = (volatile kuint32_t *)(XPS_L2CC_BASEADDR + XPS_L2CC_CACHE_INVLD_PA_OFFSET);

    currmask = __get_cpsr();
    __set_cpsr(currmask | CPSR_BIT_F);

    if (len) 
    {
        end = tempadr + len;
        tempend = end;

        /* Select L1 Data cache in CSSR */
        mr_set_cp15(XREG_CP15_CACHE_SIZE_SEL, cache_val);

        if (tempadr & (cacheline - 1U))
        {
            tempadr &= (~(cacheline - 1U));
            Xil_L1DCacheFlushLine(tempadr);

        #if (!defined(CONFIG_USE_AMP) || !CONFIG_USE_AMP)
            /* Disable Write-back and line fills */
            Xil_L2WriteDebugCtrl(0x3U);
            Xil_L2CacheFlushLine(tempadr);
            /* Enable Write-back and line fills */
            Xil_L2WriteDebugCtrl(0x0U);
            Xil_L2CacheSync();
        #endif
            tempadr += cacheline;
        }

        if (tempend & (cacheline - 1U)) 
        {
            tempend &= (~(cacheline - 1U));
            Xil_L1DCacheFlushLine(tempend);

        #if (!defined(CONFIG_USE_AMP) || !CONFIG_USE_AMP)
            /* Disable Write-back and line fills */
            Xil_L2WriteDebugCtrl(0x3U);
            Xil_L2CacheFlushLine(tempend);
            /* Enable Write-back and line fills */
            Xil_L2WriteDebugCtrl(0x0U);
            Xil_L2CacheSync();
        #endif
        }

        while (tempadr < tempend) 
        {
        #if (!defined(CONFIG_USE_AMP) || !CONFIG_USE_AMP)
            /* Invalidate L2 cache line */
            *L2CCOffset = tempadr;
            Xil_L2CacheSync();
        #endif

            /* Invalidate L1 Data cache line */
            mr_set_cp15(XREG_CP15_INVAL_DC_LINE_MVA_POC, tempadr);
            tempadr += cacheline;
        }
    }

    mr_dsb();
    __set_cpsr(currmask);
}

/*!
 * @brief   get gpio_config structure
 * @param   DeviceId
 * @retval  XGpioPs_Config
 * @note    none
 */
XGpioPs_Config *XGpioPs_LookupConfig(kuint16_t DeviceId)
{
    XGpioPs_Config *sptr_cfg = &sgtc_xgpio_ps_config_table[0];
    kuint32_t idx;

    for (idx = 0U; idx < XPAR_XGPIOPS_NUM_INSTANCES; idx++) 
    {
        if (sptr_cfg[idx].DeviceId == DeviceId) 
            return sptr_cfg;
    }

    return mr_nullptr;
}

/*!
 * @brief   fill sptr_gpio
 * @param   sptr_gpio, sptr_cfg
 * @param   address: base address of gpio bank
 * @retval  errno
 * @note    none
 */
kint32_t XGpioPs_CfgInitialize(XGpioPs *sptr_gpio, XGpioPs_Config *sptr_cfg, kuint32_t address)
{
    kuint32_t idx, RegOffset;

    /*!<
     * Set some default values for instance data, don't indicate the device
     * is ready to use until everything has been initialized successfully.
     */
    sptr_gpio->IsReady = false;
    sptr_gpio->sgtc_cfg.BaseAddr = address;
    sptr_gpio->sgtc_cfg.DeviceId = sptr_cfg->DeviceId;

    /*!< Initialize the Bank data based on platform */
    /*!<
     * Max pins in the GPIO device
     * 0 - 31,  Bank 0
     * 32 - 53, Bank 1
     * 54 - 85, Bank 2
     * 86 - 117, Bank 3
     */
    sptr_gpio->MaxPinNum = XGPIOPS_DEVICE_MAX_PIN_NUM;
    sptr_gpio->MaxBanks = XGPIOPS_MAX_BANKS;

    /*!<
     * By default, interrupts are not masked in GPIO. Disable
     * interrupts for all pins in all the 4 banks.
     */
    for (idx = 0U; idx < sptr_gpio->MaxBanks; idx++) 
    {
        /*!<
         * XGPIOPS_REG_MASK_OFFSET: the lenth of one bank
         * XGPIOPS_INTDIS_OFFSET: the RegOffset of intr register
         */
        RegOffset = idx * XGPIOPS_REG_MASK_OFFSET + XGPIOPS_INTDIS_OFFSET;
        XSdPs_WriteReg(sptr_gpio->sgtc_cfg.BaseAddr, RegOffset, 0xffffffffU);
    }

    /*!< Indicate the component is now ready to use. */
    sptr_gpio->IsReady = true;

    return ER_NORMAL;
}

/*!
 * @brief   get bank number and pin number in bank
 * @param   PinNumber: 0 ~ 117
 * @param   BankNumber: 0 ~ 3
 * @param   PinNumberInBank: 0 ~ 31
 * @retval  none
 * @note    convert PinNumber to PinNumberInBank
 */
void XGpioPs_GetBankPin(kuint32_t PinNumber, kuint8_t *BankNumber, kuint8_t *PinNumberInBank)
{
    kuint32_t XGpioPsPinTable[4] = 
    {
        XGPIOPS_BANK0_MAX_PIN + 1,
        XGPIOPS_BANK1_MAX_PIN + 1,
        XGPIOPS_BANK2_MAX_PIN + 1,
        XGPIOPS_BANK3_MAX_PIN + 1
    };

    *BankNumber = 0U;
    while (*BankNumber < 4U) 
    {
        if (PinNumber < XGpioPsPinTable[*BankNumber])
            break;
        (*BankNumber)++;
    }

    /*!<
     * PinNumber:   
     *  for 0 ~ 31, PinNumberInBank = PinNumber;
     *  for 32 ~ 117, such as PinNumber = 35 ===> PinNumberInBank = 3
     */
    if (*BankNumber == 0U)
        *PinNumberInBank = PinNumber;
    else
        *PinNumberInBank = (kuint8_t)(PinNumber % XGpioPsPinTable[*BankNumber - 1]);
}

/*!
 * @brief   get I/O direction of the pin
 * @param   sptr_gpio
 * @param   Pin: 0 ~ 117
 * @retval  errno
 * @note    none
 */
kint32_t XGpioPs_GetDirectionPin(XGpioPs *sptr_gpio, kuint32_t Pin)
{
    kuint8_t Bank;
    kuint8_t PinNumber;
    kuint32_t RegOffset, Value;

    if ((!sptr_gpio) ||
        (!sptr_gpio->IsReady) ||
        (Pin >= sptr_gpio->MaxPinNum))
        return -ER_FAULT;

    /*!< Get the Bank number and Pin number within the bank. */
    XGpioPs_GetBankPin(Pin, &Bank, &PinNumber);

    RegOffset = ((kuint32_t)(Bank) * XGPIOPS_REG_MASK_OFFSET) + XGPIOPS_DIRM_OFFSET;
    Value = XSdPs_ReadReg(sptr_gpio->sgtc_cfg.BaseAddr, RegOffset);

    return !!(Value & mr_bit(PinNumber));
}

/*!
 * @brief   set I/O direction of the pin
 * @param   sptr_gpio
 * @param   Pin: 0 ~ 117
 * @param   Direction: 0, input; 1, output
 * @retval  errno
 * @note    none
 */
kint32_t XGpioPs_SetDirectionPin(XGpioPs *sptr_gpio, kuint32_t Pin, kuint32_t Direction)
{
    kuint8_t Bank;
    kuint8_t PinNumber;
    kuint32_t DirModeReg, RegOffset;

    if ((!sptr_gpio) ||
        (!sptr_gpio->IsReady) ||
        (Pin >= sptr_gpio->MaxPinNum) ||
        (Direction > 1U))
        return -ER_FAULT;

    /*!< Get the Bank number and Pin number within the bank. */
    XGpioPs_GetBankPin(Pin, &Bank, &PinNumber);

    RegOffset = ((kuint32_t)(Bank) * XGPIOPS_REG_MASK_OFFSET) + XGPIOPS_DIRM_OFFSET;
    DirModeReg = XSdPs_ReadReg(sptr_gpio->sgtc_cfg.BaseAddr, RegOffset);

    /*!< Output Direction */
    if (Direction)
        mr_setbitl(mr_bit(PinNumber), &DirModeReg);
    
    /*!< Input Direction */
    else 
        mr_clrbitl(mr_bit(PinNumber), &DirModeReg);

    XSdPs_WriteReg(sptr_gpio->sgtc_cfg.BaseAddr, RegOffset, DirModeReg);

    return ER_NORMAL;
}

/*!
 * @brief   get output gain of the pin
 * @param   sptr_gpio
 * @param   Pin: 0 ~ 117
 * @retval  errno
 * @note    none
 */
kint32_t XGpioPs_GetOutputEnablePin(XGpioPs *sptr_gpio, kuint32_t Pin)
{
    kuint8_t Bank;
    kuint8_t PinNumber;
    kuint32_t RegOffset, Value;

    if ((!sptr_gpio) ||
        (!sptr_gpio->IsReady) ||
        (Pin >= sptr_gpio->MaxPinNum))
        return -ER_FAULT;

    /*!< Get the Bank number and Pin number within the bank. */
    XGpioPs_GetBankPin(Pin, &Bank, &PinNumber);

    RegOffset = ((kuint32_t)(Bank) * XGPIOPS_REG_MASK_OFFSET) + XGPIOPS_OUTEN_OFFSET;
    Value = XSdPs_ReadReg(sptr_gpio->sgtc_cfg.BaseAddr, RegOffset);

    return !!(Value & mr_bit(PinNumber));
}

/*!
 * @brief   enable output gain of the pin
 * @param   sptr_gpio
 * @param   Pin: 0 ~ 117
 * @param   OpEnable: 0, disable; 1, enable
 * @retval  errno
 * @note    none
 */
kint32_t XGpioPs_SetOutputEnablePin(XGpioPs *sptr_gpio, kuint32_t Pin, kbool_t OpEnable)
{
    kuint8_t Bank;
    kuint8_t PinNumber;
    kuint32_t OpEnableReg, RegOffset;

    if ((!sptr_gpio) ||
        (!sptr_gpio->IsReady) ||
        (Pin >= sptr_gpio->MaxPinNum))
        return -ER_FAULT;

    /*!< Get the Bank number and Pin number within the bank. */
    XGpioPs_GetBankPin(Pin, &Bank, &PinNumber);

    RegOffset = ((kuint32_t)(Bank) * XGPIOPS_REG_MASK_OFFSET) + XGPIOPS_OUTEN_OFFSET;
    OpEnableReg = XSdPs_ReadReg(sptr_gpio->sgtc_cfg.BaseAddr, RegOffset);

    /*!< Enable Output Enable */
    if (OpEnable)
        mr_setbitl(mr_bit(PinNumber), &OpEnableReg);

    /*!< Disable Output Enable */
    else
        mr_clrbitl(mr_bit(PinNumber), &OpEnableReg);

    XSdPs_WriteReg(sptr_gpio->sgtc_cfg.BaseAddr, RegOffset, OpEnableReg);

    return ER_NORMAL;
}

/*!
 * @brief   read value from the pin (DATA_RO)
 * @param   sptr_gpio
 * @param   Pin: 0 ~ 117
 * @retval  errno
 * @note    pin should be configured to input direction
 */
kint32_t XGpioPs_ReadPin(XGpioPs *sptr_gpio, kuint32_t Pin)
{
    kuint32_t RegOffset, Value;
    kuint8_t Bank;
    kuint8_t PinNumber;

    if ((!sptr_gpio) ||
        (!sptr_gpio->IsReady) ||
        (Pin >= sptr_gpio->MaxPinNum))
        return -ER_FAULT;

    /*!< Get the Bank number and Pin number within the bank. */
    XGpioPs_GetBankPin(Pin, &Bank, &PinNumber);

    RegOffset = (kuint32_t)(Bank) * XGPIOPS_DATA_BANK_OFFSET + XGPIOPS_DATA_RO_OFFSET;
    Value = XSdPs_ReadReg(sptr_gpio->sgtc_cfg.BaseAddr, RegOffset);
    
    return !!(Value & mr_bit(PinNumber));
}

/*!
 * @brief   write value to the pin (DATA_LSW, DATA_MSW)
 * @param   sptr_gpio
 * @param   Pin: 0 ~ 117
 * @param   Data: 0, low level; 1, high level
 * @retval  errno
 * @note    pin should be configured to output direction
 */
kint32_t XGpioPs_WritePin(XGpioPs *sptr_gpio, kuint32_t Pin, kuint32_t Data)
{
    kuint32_t RegOffset;
    kuint32_t Value, Mask;
    kuint8_t Bank;
    kuint8_t PinNumber;
    kuint32_t DataVar = !!Data;

    if ((!sptr_gpio) ||
        (!sptr_gpio->IsReady) ||
        (Pin >= sptr_gpio->MaxPinNum))
        return -ER_FAULT;

    /*!< Get the Bank number and Pin number within the bank. */
    XGpioPs_GetBankPin(Pin, &Bank, &PinNumber);

    if (PinNumber < 16U) 
        RegOffset = XGPIOPS_DATA_LSW_OFFSET;
    else
    {
        /*!< There are only 16 data bits in bit maskable register. */
        PinNumber -= 16;
        RegOffset = XGPIOPS_DATA_MSW_OFFSET;
    }

    RegOffset += ((kuint32_t)(Bank) * XGPIOPS_DATA_MASK_OFFSET);

    /*!<
     * Get the 32 bit value to be written to the Mask/Data register where
     * the upper 16 bits is the mask and lower 16 bits is the data.
     */   
    Value = mr_bit_nr(DataVar, PinNumber) | 0xffff0000U;
    Mask = ~mr_bit(PinNumber + 16U);
    XSdPs_WriteReg(sptr_gpio->sgtc_cfg.BaseAddr, RegOffset, mr_mask(Value, Mask));

    return ER_NORMAL;
}

/*!
 * @brief   get scu_timer config structure
 * @param   DeviceId
 * @retval  XGpioPs_Config
 * @note    none
 */
XScuTimer_Config *XScuTimer_LookupConfig(kuint16_t DeviceId)
{
    XScuTimer_Config *sptr_cfg = &sgtc_xscutimer_config_table[0];
    kuint32_t idx;

    for (idx = 0U; idx < XPAR_XUARTPS_NUM_INSTANCES; idx++) 
    {
        if (sptr_cfg[idx].DeviceId == DeviceId) 
            return sptr_cfg;
    }

    return mr_nullptr;
}

/*!
 * @brief   ScuTimer Initialization
 * @param   BaseAddr: address of timer register
 * @retval  error code
 * @note    none
 */
kint32_t XScuTimer_CfgInitialize(XScuTimer *sptr_scutimer,
                        XScuTimer_Config *sptr_cfg, kuint32_t BaseAddr)
{
    if ((!sptr_scutimer) || 
        (!sptr_cfg))
        return -ER_FAULT;

    /*!<
     * If the device is started, disallow the initialize and return a
     * status indicating it is started. This allows the user to stop the
     * device and reinitialize, but prevents a user from inadvertently
     * initializing.
     */
    if (!sptr_scutimer->IsStarted)
    {
        /*!< Copy configuration into the instance structure. */
        sptr_scutimer->Config.DeviceId = sptr_cfg->DeviceId;

        /*!<
         * Save the base address pointer such that the registers of the block
         * can be accessed and indicate it has not been started yet.
         */
        sptr_scutimer->Config.BaseAddr = BaseAddr;
        sptr_scutimer->IsStarted = false;

        /*!< Indicate the instance is ready to use, successfully initialized. */
        sptr_scutimer->IsReady = true;
    }

    return 0;
}

/*!
 * @brief   ScuTimer start
 * @param   sptr_scutimer
 * @retval  none
 * @note    none
 */
void XScuTimer_Start(XScuTimer *sptr_scutimer)
{
    kint32_t Register;

    if ((!sptr_scutimer) ||
        (!sptr_scutimer->IsReady))
        return;

    /*!< Read the contents of the Control register. */
    Register = mr_readl(sptr_scutimer->Config.BaseAddr + XSCUTIMER_CONTROL_OFFSET);

    /*!< Set the 'timer enable' bit in the register. */
    Register |= XSCUTIMER_CONTROL_ENABLE_MASK;

    /*!< Update the Control register with the new value. */
    mr_writel(Register, sptr_scutimer->Config.BaseAddr + XSCUTIMER_CONTROL_OFFSET);

    /*!< Indicate that the device is started. */
    sptr_scutimer->IsStarted = true;
}

/*!
 * @brief   ScuTimer stop
 * @param   sptr_scutimer
 * @retval  none
 * @note    none
 */
void XScuTimer_Stop(XScuTimer *sptr_scutimer)
{
    kint32_t Register;

    if ((!sptr_scutimer) ||
        (!sptr_scutimer->IsReady))
        return;

    /*!< Read the contents of the Control register. */
    Register = mr_readl(sptr_scutimer->Config.BaseAddr + XSCUTIMER_CONTROL_OFFSET);

    /*!< Set the 'timer enable' bit in the register. */
    Register &= (kuint32_t)(~XSCUTIMER_CONTROL_ENABLE_MASK);

    /*!< Update the Control register with the new value. */
    mr_writel(Register, sptr_scutimer->Config.BaseAddr + XSCUTIMER_CONTROL_OFFSET);

    /*!< Indicate that the device is started. */
    sptr_scutimer->IsStarted = false;
}

/*!
 * @brief   load timer period
 * @param   sptr_scutimer, value (period)
 * @retval  none
 * @note    none
 */
void XScuTimer_LoadTimer(XScuTimer *sptr_scutimer, kuint32_t value)
{
    mr_writel(value, sptr_scutimer->Config.BaseAddr + XSCUTIMER_LOAD_OFFSET);
}

/*!
 * @brief   restart timer
 * @param   sptr_scutimer
 * @retval  none
 * @note    none
 */
void XScuTimer_RestartTimer(XScuTimer *sptr_scutimer)
{
    kuint32_t value;

    value = mr_readl(sptr_scutimer->Config.BaseAddr + XSCUTIMER_LOAD_OFFSET);
    XScuTimer_LoadTimer(sptr_scutimer, value);
}

/*!
 * @brief   enable timer to load period automatically
 * @param   sptr_scutimer
 * @retval  none
 * @note    none
 */
void XScuTimer_EnableAutoReload(XScuTimer *sptr_scutimer)
{
    kuint32_t value;

    value  = mr_readl(sptr_scutimer->Config.BaseAddr + XSCUTIMER_CONTROL_OFFSET);
    value |= XSCUTIMER_CONTROL_AUTO_RELOAD_MASK;
    mr_writel(value, sptr_scutimer->Config.BaseAddr + XSCUTIMER_CONTROL_OFFSET);
}

/*!
 * @brief   disable timer to load period automatically
 * @param   sptr_scutimer
 * @retval  none
 * @note    none
 */
void XScuTimer_DisableAutoReload(XScuTimer *sptr_scutimer)
{
    kuint32_t value;

    value  = mr_readl(sptr_scutimer->Config.BaseAddr + XSCUTIMER_CONTROL_OFFSET);
    value &= ~XSCUTIMER_CONTROL_AUTO_RELOAD_MASK;
    mr_writel(value, sptr_scutimer->Config.BaseAddr + XSCUTIMER_CONTROL_OFFSET);
}

/*!
 * @brief   timer interrupt enable
 * @param   sptr_scutimer
 * @retval  none
 * @note    none
 */
void XScuTimer_EnableInterrupt(XScuTimer *sptr_scutimer)
{
    kuint32_t value;

    value  = mr_readl(sptr_scutimer->Config.BaseAddr + XSCUTIMER_CONTROL_OFFSET);
    value |= XSCUTIMER_CONTROL_IRQ_ENABLE_MASK;
    mr_writel(value, sptr_scutimer->Config.BaseAddr + XSCUTIMER_CONTROL_OFFSET);
}

/*!
 * @brief   timer interrupt disable
 * @param   sptr_scutimer
 * @retval  none
 * @note    none
 */
void XScuTimer_DisableInterrupt(XScuTimer *sptr_scutimer)
{
    kuint32_t value;

    value  = mr_readl(sptr_scutimer->Config.BaseAddr + XSCUTIMER_CONTROL_OFFSET);
    value &= ~XSCUTIMER_CONTROL_IRQ_ENABLE_MASK;
    mr_writel(value, sptr_scutimer->Config.BaseAddr + XSCUTIMER_CONTROL_OFFSET);
}

/*!
 * @brief   get timer interrupt status
 * @param   sptr_scutimer
 * @retval  none
 * @note    none
 */
kuint32_t XScuTimer_GetInterruptStatus(XScuTimer *sptr_scutimer)
{
    return mr_readl(sptr_scutimer->Config.BaseAddr + XSCUTIMER_ISR_OFFSET);
}

/*!
 * @brief   clear timer interrupt status
 * @param   sptr_scutimer
 * @retval  none
 * @note    none
 */
void XScuTimer_ClearInterruptStatus(XScuTimer *sptr_scutimer)
{
    mr_writel(XSCUTIMER_ISR_EVENT_FLAG_MASK, 
            sptr_scutimer->Config.BaseAddr + XSCUTIMER_ISR_OFFSET);
}

/*!
 * @brief   get ttc_timer config structure
 * @param   DeviceId
 * @retval  XTtcPs_Config
 * @note    none
 */
XTtcPs_Config *XTtcPs_LookupConfig(kuint16_t DeviceId)
{
    XTtcPs_Config *sptr_cfg = &sgtc_xttcps_config_table[0];
    kuint32_t idx;

    for (idx = 0U; idx < XPAR_XTTCPS_NUM_INSTANCES; idx++) 
    {
        if (sptr_cfg[idx].DeviceId == DeviceId) 
            return sptr_cfg;
    }

    return mr_nullptr;
}

/*!
 * @brief   Ttc Initialization
 * @param   BaseAddr: address of timer register
 * @retval  error code
 * @note    none
 */
kint32_t XTtcPs_CfgInitialize(XTtcPs *sptr_ttc, XTtcPs_Config *sptr_cfg, kuint32_t EffectiveAddr)
{
    kuint32_t IsStartResult;

    if (!sptr_ttc || !sptr_cfg)
        return -ER_NULLPTR;

    /*!< Set some default values */
    sptr_ttc->Config.DeviceId = sptr_cfg->DeviceId;
    sptr_ttc->Config.BaseAddress = EffectiveAddr;
    sptr_ttc->Config.InputClockHz = sptr_cfg->InputClockHz;

    IsStartResult = XTtcPs_IsStarted(sptr_ttc);

    /*!
     * If the timer counter has already started, return an error
     * Device should be stopped first.
     */
    if(IsStartResult == (kuint32_t)true)
        return -ER_NREADY;

    /*!< stop the timer before configuring */
    XTtcPs_Stop(sptr_ttc);
    /*!< Reset the count control register to it's default value. */
    XTtcPs_WriteReg(sptr_ttc->Config.BaseAddress, XTTCPS_CNT_CNTRL_OFFSET, XTTCPS_CNT_CNTRL_RESET_VALUE);

    /*!< Reset the rest of the registers to the default values. */
    XTtcPs_WriteReg(sptr_ttc->Config.BaseAddress, XTTCPS_CLK_CNTRL_OFFSET, 0x00U);
    XTtcPs_WriteReg(sptr_ttc->Config.BaseAddress, XTTCPS_INTERVAL_VAL_OFFSET, 0x00U);
    XTtcPs_WriteReg(sptr_ttc->Config.BaseAddress, XTTCPS_MATCH_0_OFFSET, 0x00U);
    XTtcPs_WriteReg(sptr_ttc->Config.BaseAddress, XTTCPS_MATCH_1_OFFSET, 0x00U);
    XTtcPs_WriteReg(sptr_ttc->Config.BaseAddress, XTTCPS_MATCH_2_OFFSET, 0x00U);
    XTtcPs_WriteReg(sptr_ttc->Config.BaseAddress, XTTCPS_IER_OFFSET, 0x00U);
    XTtcPs_WriteReg(sptr_ttc->Config.BaseAddress, XTTCPS_ISR_OFFSET, XTTCPS_IXR_ALL_MASK);

    sptr_ttc->IsReady = true;

    /*!< Reset the counter value */
    XTtcPs_ResetCounterValue(sptr_ttc);

    return ER_NORMAL;
}

/*!
 * @brief   Ttc Set Match Value
 * @param   sptr_ttc: Ttc handler
 * @retval  none
 * @note    none
 */
void XTtcPs_SetMatchValue(XTtcPs *sptr_ttc, kuint8_t MatchIndex, XMatchRegValue Value)
{
    if (!sptr_ttc || 
        !sptr_ttc->IsReady ||
        (MatchIndex >= (kuint8_t)XTTCPS_NUM_MATCH_REG))
        return;

    /*!< Write the value to the correct match register with MatchIndex */
    XTtcPs_WriteReg(sptr_ttc->Config.BaseAddress, XTtcPs_Match_N_Offset(MatchIndex), Value);
}

/*!
 * @brief   Ttc Get Match Value
 * @param   sptr_ttc: Ttc handler
 * @retval  none
 * @note    none
 */
XMatchRegValue XTtcPs_GetMatchValue(XTtcPs *sptr_ttc, kuint8_t MatchIndex)
{
    kuint32_t MatchReg;

    if (!sptr_ttc || 
        !sptr_ttc->IsReady ||
        (MatchIndex >= (kuint8_t)XTTCPS_NUM_MATCH_REG))
        return -ER_NREADY;

    MatchReg = XTtcPs_ReadReg(sptr_ttc->Config.BaseAddress, XTtcPs_Match_N_Offset(MatchIndex));

    return (XMatchRegValue) MatchReg;
}

/*!
 * @brief   Ttc Set Prescaler
 * @param   sptr_ttc: Ttc handler
 * @retval  none
 * @note    none
 */
void XTtcPs_SetPrescaler(XTtcPs *sptr_ttc, kuint8_t PrescalerValue)
{
    kuint32_t ClockReg;

    if (!sptr_ttc || 
        !sptr_ttc->IsReady ||
        (PrescalerValue > XTTCPS_CLK_CNTRL_PS_DISABLE))
        return;

    /*!< Read the clock control register */
    ClockReg = XTtcPs_ReadReg(sptr_ttc->Config.BaseAddress, XTTCPS_CLK_CNTRL_OFFSET);

    /*!< Clear all of the prescaler control bits in the register */
    ClockReg &= ~(XTTCPS_CLK_CNTRL_PS_VAL_MASK | XTTCPS_CLK_CNTRL_PS_EN_MASK);

    if (PrescalerValue < XTTCPS_CLK_CNTRL_PS_DISABLE) 
    {
        /*!< Set the prescaler value and enable prescaler */
        ClockReg |= (kuint32_t)(((kuint32_t)PrescalerValue << (kuint32_t)XTTCPS_CLK_CNTRL_PS_VAL_SHIFT) &
                    (kuint32_t)XTTCPS_CLK_CNTRL_PS_VAL_MASK);
        ClockReg |= (kuint32_t)XTTCPS_CLK_CNTRL_PS_EN_MASK;
    }

    /*!< Write the register with the new values. */
    XTtcPs_WriteReg(sptr_ttc->Config.BaseAddress, XTTCPS_CLK_CNTRL_OFFSET, ClockReg);
}

/*!
 * @brief   Ttc Get Prescaler
 * @param   sptr_ttc: Ttc handler
 * @retval  errno
 * @note    none
 */
kuint8_t XTtcPs_GetPrescaler(XTtcPs *sptr_ttc)
{
    kuint8_t Status;
    kuint32_t ClockReg;

    if (!sptr_ttc || 
        !sptr_ttc->IsReady)
        return -ER_NREADY;

    /*!< Read the clock control register */
    ClockReg = XTtcPs_ReadReg(sptr_ttc->Config.BaseAddress, XTTCPS_CLK_CNTRL_OFFSET);

    if (0 == (ClockReg & XTTCPS_CLK_CNTRL_PS_EN_MASK)) 
    {
        /*!< Prescaler is disabled. Return the correct flag value */
        Status = (kuint8_t)XTTCPS_CLK_CNTRL_PS_DISABLE;
    }
    else 
    {
        Status = (kuint8_t)((ClockReg & (kuint32_t)XTTCPS_CLK_CNTRL_PS_VAL_MASK) >>
                    (kuint32_t)XTTCPS_CLK_CNTRL_PS_VAL_SHIFT);
    }

    return Status;
}

/*!
 * @brief   Ttc Calculate Interval
 * @param   sptr_ttc: Ttc handler
 * @retval  none
 * @note    none
 */
void XTtcPs_CalcIntervalFromFreq(XTtcPs *sptr_ttc, kuint32_t Freq, XInterval *Interval, kuint8_t *Prescaler)
{
    kuint8_t TmpPrescaler;
    kuaddr_t TempValue;
    kuint32_t InputClock;

    InputClock = sptr_ttc->Config.InputClockHz;
    /*!
     * Find the smallest prescaler that will work for a given frequency. The
     * smaller the prescaler, the larger the count and the more accurate the PWM setting.
     */
    TempValue = InputClock / Freq;

    if (TempValue < 4U) 
    {
        /*!
         * The frequency is too high, it is too close to the input
         * clock value. Use maximum values to signal caller.
         */
        *Interval = XTTCPS_MAX_INTERVAL_COUNT;
        *Prescaler = 0xFFU;

        return;
    }

    /*!< First, do we need a prescaler or not ? */
    if (((kuaddr_t)XTTCPS_MAX_INTERVAL_COUNT) > TempValue) 
    {
        /*!< We do not need a prescaler, so set the values appropriately */
        *Interval = (XInterval)TempValue;
        *Prescaler = XTTCPS_CLK_CNTRL_PS_DISABLE;

        return;
    }

    for (TmpPrescaler = 0U; TmpPrescaler < XTTCPS_CLK_CNTRL_PS_DISABLE; TmpPrescaler++) 
    {
        TempValue =	InputClock / (Freq * (1U << (TmpPrescaler + 1U)));

        /*!< The first value less than 2^16 is the best bet */
        if (((kuaddr_t)XTTCPS_MAX_INTERVAL_COUNT) > TempValue) 
        {
            /*!< Set the values appropriately */
            *Interval = (XInterval)TempValue;
            *Prescaler = TmpPrescaler;

            return;
        }
    }

    /*!
     * Can not find interval values that work for the given frequency.
     * Return maximum values to signal caller.
     */
    *Interval = XTTCPS_MAX_INTERVAL_COUNT;
    *Prescaler = 0XFFU;

    return;
}

typedef struct 
{
    kuint32_t Option;
    kuint32_t Mask;
    kuint32_t Register;

} OptionsMap;

static OptionsMap TmrCtrOptionsTable[] = 
{
    { XTTCPS_OPTION_EXTERNAL_CLK,   XTTCPS_CLK_CNTRL_SRC_MASK,      XTTCPS_CLK_CNTRL_OFFSET },
    { XTTCPS_OPTION_CLK_EDGE_NEG,   XTTCPS_CLK_CNTRL_EXT_EDGE_MASK, XTTCPS_CLK_CNTRL_OFFSET },
    { XTTCPS_OPTION_INTERVAL_MODE,  XTTCPS_CNT_CNTRL_INT_MASK,      XTTCPS_CNT_CNTRL_OFFSET },
    { XTTCPS_OPTION_DECREMENT,      XTTCPS_CNT_CNTRL_DECR_MASK,     XTTCPS_CNT_CNTRL_OFFSET },
    { XTTCPS_OPTION_MATCH_MODE,     XTTCPS_CNT_CNTRL_MATCH_MASK,    XTTCPS_CNT_CNTRL_OFFSET },
    { XTTCPS_OPTION_WAVE_DISABLE,   XTTCPS_CNT_CNTRL_EN_WAVE_MASK,  XTTCPS_CNT_CNTRL_OFFSET },
    { XTTCPS_OPTION_WAVE_POLARITY,  XTTCPS_CNT_CNTRL_POL_WAVE_MASK, XTTCPS_CNT_CNTRL_OFFSET },
};

#define XTTCPS_NUM_TMRCTR_OPTIONS               ARRAY_SIZE((TmrCtrOptionsTable))

/*!
 * @brief   Ttc Set Match Value
 * @param   sptr_ttc: Ttc handler
 * @retval  errno
 * @note    none
 */
kint32_t XTtcPs_SetOptions(XTtcPs *sptr_ttc, kuint32_t Options)
{
    kuint32_t CountReg;
    kuint32_t ClockReg;
    kuint32_t Index;
    kint32_t Status = ER_NORMAL;

    if (!sptr_ttc || 
        !sptr_ttc->IsReady)
        return -ER_NREADY;

    ClockReg = XTtcPs_ReadReg(sptr_ttc->Config.BaseAddress, XTTCPS_CLK_CNTRL_OFFSET);
    CountReg = XTtcPs_ReadReg(sptr_ttc->Config.BaseAddress, XTTCPS_CNT_CNTRL_OFFSET);

    /*!<
     * Loop through the options table, turning the option on or off
     * depending on whether the bit is set in the incoming options flag.
     */
    for (Index = 0U; Index < XTTCPS_NUM_TMRCTR_OPTIONS; Index++) 
    {
        if (Status == ER_NORMAL) 
        {
            if ((Options & TmrCtrOptionsTable[Index].Option) != (kuint32_t)0) 
            {
                switch (TmrCtrOptionsTable[Index].Register) 
                {
                    case XTTCPS_CLK_CNTRL_OFFSET:
                        /*!< Add option */
                        ClockReg |= TmrCtrOptionsTable[Index].Mask;
                        break;

                    case XTTCPS_CNT_CNTRL_OFFSET:
                        /*!< Add option */
                        CountReg |= TmrCtrOptionsTable[Index].Mask;
                        break;

                    default:
                        Status = -ER_FAILD;
                        break;
                }
            }
            else 
            {
                switch (TmrCtrOptionsTable[Index].Register) 
                {
                    case XTTCPS_CLK_CNTRL_OFFSET:
                        /*!< Remove option*/
                        ClockReg &= ~TmrCtrOptionsTable[Index].Mask;
                        break;

                    case XTTCPS_CNT_CNTRL_OFFSET:
                        /*!< Remove option*/
                        CountReg &= ~TmrCtrOptionsTable[Index].Mask;
                        break;

                    default:
                        Status = -ER_FAILD;
                        break;
                }
            }
        }
    }

    /*!< Now write the registers. Leave it to the upper layers to restart the device */
    if (Status == ER_NORMAL)
    {
        XTtcPs_WriteReg(sptr_ttc->Config.BaseAddress, XTTCPS_CLK_CNTRL_OFFSET, ClockReg);
        XTtcPs_WriteReg(sptr_ttc->Config.BaseAddress, XTTCPS_CNT_CNTRL_OFFSET, CountReg);
    }

    return Status;
}

/*!
 * @brief   Ttc Get Options
 * @param   sptr_ttc: Ttc handler
 * @retval  errno
 * @note    none
 */
kuint32_t XTtcPs_GetOptions(XTtcPs *sptr_ttc)
{
    kuint32_t OptionsFlag = 0U;
    kuint32_t Register;
    kuint32_t Index;

    if (!sptr_ttc || 
        !sptr_ttc->IsReady)
        return -ER_NREADY;

    /*!< Loop through the options table to determine which options are set */
    for (Index = 0U; Index < XTTCPS_NUM_TMRCTR_OPTIONS; Index++) 
    {
        /*!< Get the control register to determine which options are currently set. */
        Register = XTtcPs_ReadReg(sptr_ttc->Config.BaseAddress,
                          TmrCtrOptionsTable[Index].Register);

        if ((Register & TmrCtrOptionsTable[Index].Mask) != (kuint32_t)0)
            OptionsFlag |= TmrCtrOptionsTable[Index].Option;
    }

    return OptionsFlag;
}

/*!
 * @brief   get the configuration of UartPs
 * @param   DeviceId
 * @retval  XUartPs_Config
 * @note    none
 */
XUartPs_Config *XUartPs_LookupConfig(kuint16_t DeviceId)
{
    XUartPs_Config *sptr_cfg = &sgtc_xuart_ps_config_table[0];
    kuint32_t idx;

    for (idx = 0U; idx < XPAR_XUARTPS_NUM_INSTANCES; idx++) 
    {
        if (sptr_cfg[idx].DeviceId == DeviceId) 
            return sptr_cfg;
    }

    return mr_nullptr;
}

/*!
 * @brief   enable Tx and Rx
 * @param   sptr_uart
 * @retval  none
 * @note    none
 */
void XUartPs_EnableUart(XUartPs *sptr_uart)
{
    kuint32_t CrReg;

    CrReg = mr_readl(sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_CR_OFFSET);
    mr_clrbitl(XUARTPS_CR_EN_DIS_MASK, &CrReg);
    mr_setbitl(XUARTPS_CR_RX_EN | XUARTPS_CR_TX_EN, &CrReg);
    mr_writel(CrReg, sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_CR_OFFSET);
}

/*!
 * @brief   disable Tx and Rx
 * @param   sptr_uart
 * @retval  none
 * @note    none
 */
void XUartPs_DisableUart(XUartPs *sptr_uart)
{
    kuint32_t CrReg;

    CrReg = mr_readl(sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_CR_OFFSET);
    mr_clrbitl(XUARTPS_CR_EN_DIS_MASK, &CrReg);
    mr_setbitl(XUARTPS_CR_RX_DIS | XUARTPS_CR_TX_DIS, &CrReg);
    mr_writel(CrReg, sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_CR_OFFSET);
}

/*!
 * @brief   configure baund rate
 * @param   IterBAUDDIV: Iterator for available baud divisor values
 * @param   BRGR_Value: Calculated value for baud rate generator
 * @param   CalcBaudRate: Calculated baud rate
 * @param   BaudError: Diff between calculated and requested baud rate
 * @param   Best_BRGR: Best value for baud rate generator
 * @param   Best_BAUDDIV: Best value for baud divisor
 * @retval  errno
 * @note    none
 */
kint32_t XUartPs_SetBaudRate(XUartPs *sptr_uart, kuint32_t BaudRate)
{
    kuint32_t IterBAUDDIV, BRGR_Value;
    kuint32_t CalcBaudRate, BaudError;
    kuint32_t Best_BRGR = 0U;
    kuint8_t Best_BAUDDIV = 0U;
    kuint32_t Best_Error = (kuint32_t)(~0U);
    kuint32_t PercentError, ModeReg, InputClk;

    if ((!sptr_uart) ||
        (!sptr_uart->IsReady) ||
        (BaudRate > XUARTPS_MAX_RATE) ||
        (BaudRate < XUARTPS_MIN_RATE))
        return -ER_FAULT;

    /*!<
     * Make sure the baud rate is not impossilby large.
     * Fastest possible baud rate is Input Clock / 2.
     */
    if ((BaudRate * 2) > sptr_uart->sgtc_cfg.InputClockHz)
        return -ER_CHECKERR;

    /*!< Check whether the input clock is divided by 8 */
    ModeReg = mr_readl(sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_MR_OFFSET);

    InputClk = sptr_uart->sgtc_cfg.InputClockHz;
    if (mr_isBitSetl(XUARTPS_MR_CLKSEL, &ModeReg))
        InputClk = sptr_uart->sgtc_cfg.InputClockHz / 8;

    /*!<
     * Determine the Baud divider. It can be 4 to 254.
     * Loop through all possible combinations
     */
    for (IterBAUDDIV = 4; IterBAUDDIV < 255; IterBAUDDIV++) 
    {
        /*!<
         * InputClk: 100000000Hz = 100000KHz = 100MHz;
         * BaudRate: such as 115200
         * IterBAUDDIV: 4 ~ 255
         * 
         * for example, IterBAUDDIV = 4:
         *      BRGR_Value = 100000000 / (115200 * (4 + 1)) = 173.611111 = 173
         *      CalcBaudRate = 100000000 / (173 * (4 + 1)) = 115606.9364 = 115606
         *      BaudError = 115606 - 115200 = 406 = Best_Error
         * 
         * the root cause is that the decimal places are lost by using integer calculation instead of decimal calculations,
         * therefore, it is necessary to traverse to find the integer solution with the smallest errors.
         */

        /*!< Calculate the value for BRGR register */
        BRGR_Value = InputClk / (BaudRate * (IterBAUDDIV + 1));

        /*!< Calculate the baud rate from the BRGR value */
        CalcBaudRate = InputClk / (BRGR_Value * (IterBAUDDIV + 1));

        /*!< Avoid unsigned integer underflow */
        BaudError = mr_usub(BaudRate, CalcBaudRate);

        /*!< Find the calculated baud rate closest to requested baud rate. */
        if (BaudError < Best_Error) 
        {
            Best_BRGR = BRGR_Value;
            Best_BAUDDIV = IterBAUDDIV;
            Best_Error = BaudError;
        }
    }

    /*!< 
     * Make sure the best error is not too large. 
     * error = (|precent_baund - baundrate| / baundrate) * 100%
     * the maximun error allowed: 3%
     */
    PercentError = (Best_Error * 100) / BaudRate;
    if (PercentError > 3)
        return -ER_FAULT;

    /*!< Disable TX and RX to avoid glitches when setting the baud rate. */
    XUartPs_DisableUart(sptr_uart);

    mr_writel(Best_BRGR, sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_BAUDGEN_OFFSET);
    mr_writel(Best_BAUDDIV, sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_BAUDDIV_OFFSET);

    /*!< RX and TX SW reset */
    mr_writel(XUARTPS_CR_TXRST | XUARTPS_CR_RXRST, sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_CR_OFFSET);

    /*!< Enable device */
    XUartPs_EnableUart(sptr_uart);
    sptr_uart->BaudRate = BaudRate;

    return ER_NORMAL;
}

/*!
 * @brief   fill sptr_uart
 * @param   sptr_uart
 * @retval  errno
 * @note    none
 */
kint32_t XUartPs_CfgInitialize(XUartPs *sptr_uart, XUartPs_Config *sptr_cfg, kuint32_t address)
{
    kint32_t Status;
    kuint32_t ModeRegister;

    if (!sptr_uart || !sptr_cfg)
        return -ER_NOMEM;

    /*!< Setup the driver instance using passed in parameters */
    sptr_uart->sgtc_cfg.BaseAddress = address;
    sptr_uart->sgtc_cfg.InputClockHz = sptr_cfg->InputClockHz;
    sptr_uart->sgtc_cfg.ModemPinsConnected = sptr_cfg->ModemPinsConnected;

    sptr_uart->sgtc_txbuf.NextBytePtr = mr_nullptr;
    sptr_uart->sgtc_txbuf.RemainingBytes = 0U;
    sptr_uart->sgtc_txbuf.RequestedBytes = 0U;

    sptr_uart->sgtc_rxbuf.NextBytePtr = mr_nullptr;
    sptr_uart->sgtc_rxbuf.RemainingBytes = 0U;
    sptr_uart->sgtc_rxbuf.RequestedBytes = 0U;

    sptr_uart->is_rxbs_error = false;

    /*!< Flag that the driver instance is ready to use */
    sptr_uart->IsReady = true;

    /*!< Set the default baud rate here, can be changed prior to starting the device */
    Status = XUartPs_SetBaudRate(sptr_uart, XUARTPS_DFT_BAUDRATE);
    if (Status)
    {
        sptr_uart->IsReady = 0U;
        return Status;
    }
    
    /*!< Set up the default data format: 8 bit data, 1 stop bit, no parity */
    ModeRegister = mr_readl(sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_MR_OFFSET);

    /*!< Mask off what's already there */
    mr_clrbitl(XUARTPS_MR_CHARLEN_MASK | XUARTPS_MR_STOPMODE_MASK | 
                XUARTPS_MR_PARITY_MASK, &ModeRegister);

    /*!< Set the register value to the desired data format */
    mr_setbitl(XUARTPS_MR_CHARLEN_8_BIT | XUARTPS_MR_STOPMODE_1_BIT | 
                XUARTPS_MR_PARITY_NONE, &ModeRegister);

    /*!< Write the mode register out */
    mr_writel(ModeRegister, sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_MR_OFFSET);

    /*!< Set the RX FIFO trigger at 8 data bytes. */
    mr_writel(0x08U, sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_RXWM_OFFSET);

    /*!< Set the RX timeout to 1, which will be 4 character time */
    mr_writel(0x01U, sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_RXTOUT_OFFSET);

    /*!< Disable all interrupts, polled mode is the default */
    mr_writel(XUARTPS_IXR_MASK, sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_IDR_OFFSET);

    return ER_NORMAL;
}

/*!
 * @brief   check if tx fifo is full
 * @param   BaseAddress
 * @retval  1: full; 0, not full
 * @note    none
 */
kbool_t XUartPs_IsSendFull(kuint32_t BaseAddress)
{
    kuint32_t SrReg;

    SrReg = mr_readl(BaseAddress + XUARTPS_SR_OFFSET);
    return mr_isBitSetl(XUARTPS_SR_TXFULL, &SrReg);
}

/*!
 * @brief   check if tx fifo is empty
 * @param   BaseAddress
 * @retval  1: empty; 0, not empty
 * @note    none
 */
kbool_t XUartPs_IsSendEmpty(kuint32_t BaseAddress)
{
    kuint32_t SrReg;

    SrReg = mr_readl(BaseAddress + XUARTPS_SR_OFFSET);
    return mr_isBitSetl(XUARTPS_SR_TXEMPTY, &SrReg);
}

/*!
 * @brief   check if rx fifo is full
 * @param   BaseAddress
 * @retval  1: full; 0, not full
 * @note    none
 */
kbool_t XUartPs_IsRecvFull(kuint32_t BaseAddress)
{
    kuint32_t SrReg;

    SrReg = mr_readl(BaseAddress + XUARTPS_SR_OFFSET);
    return mr_isBitSetl(XUARTPS_SR_RXFULL, &SrReg);
}

/*!
 * @brief   check if rx fifo is empty
 * @param   BaseAddress
 * @retval  1: empty; 0, not empty
 * @note    none
 */
kbool_t XUartPs_IsRecvEmpty(kuint32_t BaseAddress)
{
    kuint32_t SrReg;

    SrReg = mr_readl(BaseAddress + XUARTPS_SR_OFFSET);
    return mr_isBitSetl(XUARTPS_SR_RXEMPTY, &SrReg);
}

/*!
 * @brief   send multi-bytes
 * @param   sptr_uart
 * @retval  errno, or the number of bytes sent
 * @note    none
 */
kint32_t XUartPs_SendBuffer(XUartPs *sptr_uart)
{
    kuint32_t SentCount = 0U;
    kuint32_t ImrRegister;
    kuint32_t BaseAddress;

    BaseAddress = sptr_uart->sgtc_cfg.BaseAddress;

    if (!XUartPs_IsRecvEmpty(BaseAddress))
        return -1;

    /*!< wait for last data sending finish */
    while (!XUartPs_IsSendEmpty(BaseAddress));

    /*!<
     * If the TX FIFO is full, send nothing.
     * Otherwise put bytes into the TX FIFO unil it is full, or all of the
     * data has been put into the FIFO.
     */
    while (!XUartPs_IsSendFull(BaseAddress)) 
    {
        if (SentCount >= sptr_uart->sgtc_txbuf.RemainingBytes)
            break;

        /*!< Fill the FIFO from the buffer */
        mr_writel(sptr_uart->sgtc_txbuf.NextBytePtr[SentCount],
                            BaseAddress + XUARTPS_FIFO_OFFSET);

        /*!< Increment the send count. */
        SentCount++;
    }

    /*!< Update the buffer to reflect the bytes that were sent from it */
    sptr_uart->sgtc_txbuf.NextBytePtr += SentCount;
    sptr_uart->sgtc_txbuf.RemainingBytes -= SentCount;

    /*!<
     * If interrupts are enabled as indicated by the receive interrupt, then
     * enable the TX FIFO empty interrupt, so further action can be taken
     * for this sending.
     */
    ImrRegister = mr_readl(BaseAddress + XUARTPS_IMR_OFFSET);
    if (!mr_isBitResetl(XUARTPS_IXR_RXFULL | 
            XUARTPS_IXR_RXEMPTY | XUARTPS_IXR_RXOVR, &ImrRegister))
        mr_writel(ImrRegister | XUARTPS_IXR_TXEMPTY, BaseAddress + XUARTPS_IER_OFFSET);

    return SentCount;
}

/*!
 * @brief   receive multi-bytes
 * @param   sptr_uart
 * @retval  errno, or the number of bytes received
 * @note    none
 */
kint32_t XUartPs_ReceiveBuffer(XUartPs *sptr_uart)
{
    kuint32_t ReceivedCount = 0U;

    /*!<
     * Loop until there is no more data in RX FIFO or the specified
     * number of bytes has been received
     */
    while (!XUartPs_IsRecvEmpty(sptr_uart->sgtc_cfg.BaseAddress))
    {
        if (ReceivedCount >= sptr_uart->sgtc_rxbuf.RemainingBytes)
            break;

        sptr_uart->sgtc_rxbuf.NextBytePtr[ReceivedCount] = mr_readl(sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_FIFO_OFFSET);   
        ReceivedCount++;

        if (XUartPs_IsRecvEmpty(sptr_uart->sgtc_cfg.BaseAddress))
            udelay(100U);
    }
    sptr_uart->is_rxbs_error = 0;

    /*!<
     * Update the receive buffer to reflect the number of bytes just
     * received
     */
    if(sptr_uart->sgtc_rxbuf.NextBytePtr)
        sptr_uart->sgtc_rxbuf.NextBytePtr += ReceivedCount;
    sptr_uart->sgtc_rxbuf.RemainingBytes -= ReceivedCount;

    return ReceivedCount;
}

/*!
 * @brief   send multi-bytes
 * @param   sptr_uart
 * @retval  errno, or the number of bytes sent
 * @note    none
 */
kint32_t XUartPs_Send(XUartPs *sptr_uart, kuint8_t *BufferPtr, kuint32_t NumBytes)
{
    if (!sptr_uart || !BufferPtr || !sptr_uart->IsReady)
        return -ER_NOMEM;

    /*!<
     * Disable the UART transmit interrupts to allow this call to stop a
     * previous operation that may be interrupt driven.
     */
    mr_writel(XUARTPS_IXR_TXEMPTY | XUARTPS_IXR_TXFULL, sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_IDR_OFFSET);

    /*!< Setup the buffer parameters */
    sptr_uart->sgtc_txbuf.RequestedBytes = NumBytes;
    sptr_uart->sgtc_txbuf.RemainingBytes = NumBytes;
    sptr_uart->sgtc_txbuf.NextBytePtr = BufferPtr;

    /*!<
     * Transmit interrupts will be enabled in XUartPs_SendBuffer(), after
     * filling the TX FIFO.
     */
    return XUartPs_SendBuffer(sptr_uart);
}

/*!
 * @brief   receive multi-bytes
 * @param   sptr_uart
 * @retval  errno, or the number of bytes received
 * @note    none
 */
kint32_t XUartPs_Recv(XUartPs *sptr_uart, kuint8_t *BufferPtr, kuint32_t NumBytes)
{
    kuint32_t ReceivedCount;
    kuint32_t ImrRegister;

    if (!sptr_uart || !BufferPtr || !sptr_uart->IsReady)
        return -ER_NOMEM;

    if (XUartPs_IsRecvEmpty(sptr_uart->sgtc_cfg.BaseAddress))
        return -ER_TIMEOUT;

    /*!<
     * Disable all the interrupts.
     * This stops a previous operation that may be interrupt driven
     */
    ImrRegister = mr_readl(sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_IMR_OFFSET);
    mr_writel(XUARTPS_IXR_MASK, sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_IDR_OFFSET);

    /*!< Setup the buffer parameters */
    sptr_uart->sgtc_rxbuf.RequestedBytes = NumBytes;
    sptr_uart->sgtc_rxbuf.RemainingBytes = NumBytes;
    sptr_uart->sgtc_rxbuf.NextBytePtr = BufferPtr;

    /*!< Receive the data from the device */
    ReceivedCount = XUartPs_ReceiveBuffer(sptr_uart);

    /*!< Restore the interrupt state */
    mr_writel(ImrRegister, sptr_uart->sgtc_cfg.BaseAddress + XUARTPS_IER_OFFSET);

    return ReceivedCount;
}

/*!
 * @brief   get sd config structure
 * @param   DeviceId
 * @retval  XSdPs_Config
 * @note    none
 */
XSdPs_Config *XSdPs_LookupConfig(kuint16_t DeviceId)
{
    XSdPs_Config *sptr_cfg = &sgtc_xsd_ps_config_table[0];
    kuint32_t idx;

    for (idx = 0U; idx < XPAR_XSDPS_NUM_INSTANCES; idx++) 
    {
        if (sptr_cfg[idx].DeviceId == DeviceId) 
            return sptr_cfg;
    }

    return mr_nullptr;
}

kint32_t XSdPs_Change_ClkFreq(XSdPs *sptr_sd, kuint32_t SelFreq)
{
    kuint32_t BaseAddress;
    kuint16_t DivCnt;
    kuint16_t ClockReg;
    kuint16_t Divisor = 0U;
    kuint16_t ExtDivisor;

    if (!sptr_sd || !sptr_sd->IsReady)
        return -ER_NOMEM;

    BaseAddress = sptr_sd->sgtc_cfg.BaseAddress;

    /* Disable clock */
    ClockReg = XSdPs_ReadReg16(BaseAddress, XSDPS_CLK_CTRL_OFFSET);
    mr_clrbitw(XSDPS_CC_SD_CLK_EN_MASK | XSDPS_CC_INT_CLK_EN_MASK, &ClockReg);
    XSdPs_WriteReg16(BaseAddress, XSDPS_CLK_CTRL_OFFSET, ClockReg);

    if (sptr_sd->HC_Version == XSDPS_HC_SPEC_V3) 
    {
        /* Calculate divisor */
        for (DivCnt = 0x1U; DivCnt <= XSDPS_CC_EXT_MAX_DIV_CNT; DivCnt++) 
        {
            if (((sptr_sd->sgtc_cfg.InputClockHz) / DivCnt) <= SelFreq) 
            {
                Divisor = DivCnt >> 1;
                break;
            }
        }

        /* No valid divisor found for given frequency */
        if (DivCnt > XSDPS_CC_EXT_MAX_DIV_CNT)
            return -ER_FAILD;
    } 
    else 
    {
        /* Calculate divisor */
        DivCnt = 0x1U;
        while (DivCnt <= XSDPS_CC_MAX_DIV_CNT) 
        {
            if (((sptr_sd->sgtc_cfg.InputClockHz) / DivCnt) <= SelFreq) 
            {
                Divisor = DivCnt / 2U;
                break;
            }

            DivCnt = DivCnt << 1U;
        }

        /* No valid divisor found for given frequency */
        if (DivCnt > XSDPS_CC_MAX_DIV_CNT)
            return -ER_FAILD;
    }

    ClockReg = XSdPs_ReadReg16(BaseAddress, XSDPS_CLK_CTRL_OFFSET);
 
    /* Set clock divisor */
    if (sptr_sd->HC_Version == XSDPS_HC_SPEC_V3) 
    {
        mr_clrbitw(XSDPS_CC_SDCLK_FREQ_SEL_MASK | XSDPS_CC_SDCLK_FREQ_SEL_EXT_MASK, &ClockReg);

        ExtDivisor = Divisor >> 8;
        ExtDivisor <<= XSDPS_CC_EXT_DIV_SHIFT;
        ExtDivisor &= XSDPS_CC_SDCLK_FREQ_SEL_EXT_MASK;

        Divisor <<= XSDPS_CC_DIV_SHIFT;
        Divisor &= XSDPS_CC_SDCLK_FREQ_SEL_MASK;
        ClockReg |= Divisor | ExtDivisor | (kuint16_t)XSDPS_CC_INT_CLK_EN_MASK;
    } 
    else 
    {
        ClockReg = XSdPs_ReadReg16(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_CLK_CTRL_OFFSET);
        ClockReg &= (~XSDPS_CC_SDCLK_FREQ_SEL_MASK);

        Divisor <<= XSDPS_CC_DIV_SHIFT;
        Divisor &= XSDPS_CC_SDCLK_FREQ_SEL_MASK;
        ClockReg |= Divisor | (kuint16_t)XSDPS_CC_INT_CLK_EN_MASK;
    }

    XSdPs_WriteReg16(BaseAddress, XSDPS_CLK_CTRL_OFFSET, ClockReg);

    /* Wait for internal clock to stabilize */
    while (mr_isBitResetw(XSDPS_CC_INT_CLK_STABLE_MASK, BaseAddress + XSDPS_CLK_CTRL_OFFSET));

    /* Enable SD clock */
    mr_setbitw(XSDPS_CC_SD_CLK_EN_MASK, BaseAddress + XSDPS_CLK_CTRL_OFFSET);

    return ER_NORMAL;
}

kint32_t XSdPs_CfgInitialize(XSdPs *sptr_sd, XSdPs_Config *sptr_cfg, kuint32_t address)
{
    kint32_t Status;
    kuint8_t PowerLevel;
    kuint16_t ReadReg16;

    if (!sptr_sd || !sptr_cfg)
        return -ER_NOMEM;

    /* Set some default values. */
    sptr_sd->sgtc_cfg.DeviceId = sptr_cfg->DeviceId;
    sptr_sd->sgtc_cfg.BaseAddress = address;
    sptr_sd->sgtc_cfg.InputClockHz = sptr_cfg->InputClockHz;
    sptr_sd->IsReady = true;
    sptr_sd->sgtc_cfg.CardDetect = sptr_cfg->CardDetect;
    sptr_sd->sgtc_cfg.WriteProtect = sptr_cfg->WriteProtect;
    sptr_sd->sgtc_cfg.BusWidth = sptr_cfg->BusWidth;
    sptr_sd->sgtc_cfg.BankNumber = sptr_cfg->BankNumber;
    sptr_sd->sgtc_cfg.HasEMIO = sptr_cfg->HasEMIO;
    sptr_sd->sgtc_cfg.IsCacheCoherent = sptr_cfg->IsCacheCoherent;
    sptr_sd->SectorCount = 0;
    sptr_sd->Mode = XSDPS_DEFAULT_SPEED_MODE;
    sptr_sd->TransferMode = 0U;
    sptr_sd->Dma64BitAddr = 0U;

    /* Disable bus power and issue emmc hw reset */
    ReadReg16 = mr_getbitw(XSDPS_HC_SPEC_VER_MASK, 
                            sptr_sd->sgtc_cfg.BaseAddress + XSDPS_HOST_CTRL_VER_OFFSET);
    
    if (XSDPS_HC_SPEC_V3 == ReadReg16)
        XSdPs_WriteReg8(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_POWER_CTRL_OFFSET, XSDPS_PC_EMMC_HW_RST_MASK);
    else
        XSdPs_WriteReg8(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_POWER_CTRL_OFFSET, 0x0);

    /* Delay to poweroff card */
    udelay(1000);

    /* "Software reset for all" is initiated */
    XSdPs_WriteReg8(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_SW_RST_OFFSET, XSDPS_SWRST_ALL_MASK);

    /* Proceed with initialization only after reset is complete */
    while (!mr_isBitResetb(XSDPS_SWRST_ALL_MASK, sptr_sd->sgtc_cfg.BaseAddress + XSDPS_SW_RST_OFFSET));
    
    /* Host Controller version is read. */
    sptr_sd->HC_Version = (kuint8_t)mr_getbitw(XSDPS_HC_SPEC_VER_MASK, 
                                            sptr_sd->sgtc_cfg.BaseAddress + XSDPS_HOST_CTRL_VER_OFFSET);

    /*
     * Read capabilities register and update it in Instance pointer.
     * It is sufficient to read this once on power on.
     */
    sptr_sd->Host_Caps = XSdPs_ReadReg(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_CAPS_OFFSET);

    /* Select voltage and enable bus power. */
    if (sptr_sd->HC_Version == XSDPS_HC_SPEC_V3)
    {
        XSdPs_WriteReg8(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_POWER_CTRL_OFFSET, 
                                mr_mask(XSDPS_PC_BUS_VSEL_3V3_MASK | XSDPS_PC_BUS_PWR_MASK, ~XSDPS_PC_EMMC_HW_RST_MASK));

        /* Delay before issuing the command after emmc reset */
        if (XSDPS_CAPS_EMB_SLOT == mr_mask(sptr_sd->Host_Caps, XSDPS_CAPS_SLOT_TYPE_MASK))
            udelay(200);
    }
    else
    {
        XSdPs_WriteReg8(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_POWER_CTRL_OFFSET, 
                                XSDPS_PC_BUS_VSEL_3V3_MASK | XSDPS_PC_BUS_PWR_MASK);
    }

    /* Change the clock frequency to 400 KHz */
    Status = XSdPs_Change_ClkFreq(sptr_sd, XSDPS_CLK_400_KHZ);
    if (Status)
        return Status;

    if (!mr_isBitResetl(XSDPS_CAP_VOLT_3V3_MASK, &sptr_sd->Host_Caps))
        PowerLevel = XSDPS_PC_BUS_VSEL_3V3_MASK;
    else if (!mr_isBitResetl(XSDPS_CAP_VOLT_3V0_MASK, &sptr_sd->Host_Caps))
        PowerLevel = XSDPS_PC_BUS_VSEL_3V0_MASK;
    else if (!mr_isBitResetl(XSDPS_CAP_VOLT_1V8_MASK, &sptr_sd->Host_Caps))
        PowerLevel = XSDPS_PC_BUS_VSEL_1V8_MASK;
    else
        PowerLevel = 0U;

    /* Select voltage based on capability and enable bus power. */
    XSdPs_WriteReg8(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_POWER_CTRL_OFFSET, PowerLevel | XSDPS_PC_BUS_PWR_MASK);

    if (sptr_sd->HC_Version == XSDPS_HC_SPEC_V3) 
    {
        /* Enable ADMA2 in 64bit mode. */
        XSdPs_WriteReg8(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_HOST_CTRL1_OFFSET, XSDPS_HC_DMA_ADMA2_64_MASK);
    } 
    else 
    {
        /*!< 
         * bit5: Bus width (if set to 1, bus width is 8)
         * bit4: Enable ADMA2 in 32bit mode. 
         * bit1: Bus width (if set to 1, bus width is 4)
         */
        XSdPs_WriteReg8(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_HOST_CTRL1_OFFSET, XSDPS_HC_DMA_ADMA2_32_MASK);
    }

    /* Enable all interrupt status except card interrupt initially */
    XSdPs_WriteReg16(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_NORM_INTR_STS_EN_OFFSET, 
                                                mr_mask(XSDPS_NORM_INTR_ALL_MASK, ~XSDPS_INTR_CARD_MASK));
    XSdPs_WriteReg16(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_ERR_INTR_STS_EN_OFFSET, XSDPS_ERROR_INTR_ALL_MASK);

    /* Disable all interrupt signals by default. */
    XSdPs_WriteReg16(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_NORM_INTR_SIG_EN_OFFSET, 0x0U);
    XSdPs_WriteReg16(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_ERR_INTR_SIG_EN_OFFSET, 0x0U);

    /*
     * Transfer mode register - default value
     * DMA enabled, block count enabled, data direction card to host(read)
     */
    sptr_sd->TransferMode = XSDPS_TM_DMA_EN_MASK | XSDPS_TM_BLK_CNT_EN_MASK | XSDPS_TM_DAT_DIR_SEL_MASK;

    /* Set block size to 512 by default */
    XSdPs_WriteReg16(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_BLK_SIZE_OFFSET, XSDPS_BLK_SIZE_512_MASK);

    return ER_NORMAL;
}

kbool_t XSdPs_IsCardDetected(XSdPs *sptr_sd)
{
    kuint32_t PresentStateReg;

    if (!sptr_sd)
        return false;

    if ((sptr_sd->HC_Version != XSDPS_HC_SPEC_V3) ||
        ((sptr_sd->Host_Caps & XSDPS_CAPS_SLOT_TYPE_MASK) != XSDPS_CAPS_EMB_SLOT)) 
    {
        if (sptr_sd->sgtc_cfg.CardDetect) 
        {
            /*
             * Check the present state register to make sure
             * card is inserted and detected by host controller
             */
            PresentStateReg = XSdPs_ReadReg(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_PRES_STATE_OFFSET);
            if (mr_isBitResetl(XSDPS_PSR_CARD_INSRT_MASK, &PresentStateReg))
                return false;
        }
    }

    return true;
}

kint32_t XSdPs_Change_BusWidth(XSdPs *sptr_sd, kuint32_t width)
{
    kuint32_t StatusReg;

    if (!sptr_sd || !sptr_sd->IsReady)
        return -ER_FAULT;

    if (sptr_sd->CardType != XSDPS_CARD_SD)
        return -ER_NODEV;

    if ((width != XSDPS_8_BIT_WIDTH) &&
        (width != XSDPS_4_BIT_WIDTH) &&
        (width != XSDPS_1_BIT_WIDTH))
        return -ER_FAULT;

    /*
     * check for bus width for 3.0 controller and return if
     * bus width is <4
     */
    if ((sptr_sd->HC_Version == XSDPS_HC_SPEC_V3) &&
        (sptr_sd->sgtc_cfg.BusWidth < XSDPS_WIDTH_4))
        return ER_NORMAL;

    StatusReg = XSdPs_ReadReg8(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_HOST_CTRL1_OFFSET);

    /* Width setting in controller */
    if (sptr_sd->BusWidth == XSDPS_8_BIT_WIDTH)
        StatusReg |= XSDPS_HC_EXT_BUS_WIDTH;
    else if (sptr_sd->BusWidth == XSDPS_4_BIT_WIDTH)
        StatusReg |= XSDPS_HC_BUS_WIDTH_4;
    else
        StatusReg &= ~(XSDPS_HC_EXT_BUS_WIDTH | XSDPS_HC_BUS_WIDTH_4);

    XSdPs_WriteReg8(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_HOST_CTRL1_OFFSET, (kuint8_t)StatusReg);
    udelay(100U);

    if (sptr_sd->Mode == XSDPS_DDR52_MODE) 
    {
        StatusReg = XSdPs_ReadReg16(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_HOST_CTRL2_OFFSET);
        StatusReg &= (kuint16_t)(~XSDPS_HC2_UHS_MODE_MASK);
        StatusReg |= sptr_sd->Mode;
        XSdPs_WriteReg16(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_HOST_CTRL2_OFFSET, StatusReg);
    }

    return ER_NORMAL;
}

kint32_t XSdPs_SetBlkSize(XSdPs *sptr_sd, kuint32_t BlkCnt, kuint32_t BlkSize)
{
    XSdPs_WriteReg16(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_BLK_CNT_OFFSET, BlkCnt);
    XSdPs_WriteReg16(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_BLK_SIZE_OFFSET, BlkSize);

    udelay(100U);
    return ER_NORMAL;
}

void XSdPs_SetupADMA2DescTbl(XSdPs *sptr_sd, kuint32_t BlkCnt, const kuint8_t *Buff)
{
    kuint32_t DescNum = 0;
    kuint32_t BlkSize, TotalSize, RemainSize;

    /* Setup ADMA2 - Write descriptor table and point ADMA SAR to it */
    BlkSize = mr_getbitw(XSDPS_BLK_SIZE_MASK, sptr_sd->sgtc_cfg.BaseAddress + XSDPS_BLK_SIZE_OFFSET);

    TotalSize = BlkCnt * BlkSize;
    if (!TotalSize)
        return;

    RemainSize = TotalSize;
    do {
        sptr_sd->Adma2_DescrTbl[DescNum].Address =
                (kuint32_t)((kuaddr_t)Buff + (DescNum * XSDPS_DESC_MAX_LENGTH));
        sptr_sd->Adma2_DescrTbl[DescNum].Attribute = XSDPS_DESC_TRAN | XSDPS_DESC_VALID;

        if (RemainSize > XSDPS_DESC_MAX_LENGTH)
        {
            /* This will write '0' to length field which indicates 65536 */
            sptr_sd->Adma2_DescrTbl[DescNum].Length = (kuint16_t)XSDPS_DESC_MAX_LENGTH;
            RemainSize -= XSDPS_DESC_MAX_LENGTH;
        }
        else
        {
            sptr_sd->Adma2_DescrTbl[DescNum].Attribute |= XSDPS_DESC_END;
            sptr_sd->Adma2_DescrTbl[DescNum].Length = (kuint16_t)RemainSize;
            RemainSize = 0;
        }

        DescNum++;

    } while (RemainSize);

    XSdPs_WriteReg(sptr_sd->sgtc_cfg.BaseAddress, XSDPS_ADMA_SAR_OFFSET, 
                                (kuaddr_t)&(sptr_sd->Adma2_DescrTbl[0]));

    if (!sptr_sd->sgtc_cfg.IsCacheCoherent)
        Xil_DCacheFlushRange((kuaddr_t)&(sptr_sd->Adma2_DescrTbl[0]), sizeof(XSdPs_Adma2Descriptor) * 32U);
}


/*!< end of file */
