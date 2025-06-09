/*
 * General SD Card Protocol
 *
 * File Name:   fwk_sdcard.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.10.09
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/time.h>
#include <platform/base/fwk_mempool.h>
#include <platform/mmc/fwk_sdcard.h>

/*!< The defines */
enum __ERT_SDCARD_RW_CTRL
{
    NR_SdCard_RWCtrlGet = 0U,
    NR_SdCard_RWCtrlExcute,
};

/*!< command callback */
typedef kint32_t (*fwk_sdcard_control_fn_t)(struct fwk_sdcard_cmd *, struct fwk_sdcard_if *, kuint32_t);

/*!< The function */
/*!< command transfer controller: as callback for "fwk_sdcard_control_fn_t" */
static kint32_t fwk_sdcard_command_go_idle(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags);
static kint32_t fwk_sdcard_command_send_if_cond(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags);
static kint32_t fwk_sdcard_command_app_before(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags);
static kint32_t fwk_sdcard_command_operation_cond(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags);
static kint32_t fwk_sdcard_command_get_cid(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags);
static kint32_t fwk_sdcard_command_get_csd(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags);
static kint32_t fwk_sdcard_command_get_rca(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags);
static kint32_t fwk_sdcard_command_get_scr(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags);
static kint32_t fwk_sdcard_command_select_card(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags);
static kint32_t fwk_sdcard_command_get_status(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags);
static kint32_t fwk_sdcard_command_stop_transmission(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags);
static kint32_t fwk_sdcard_command_switch_voltage(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags);
static kint32_t fwk_sdcard_command_set_buswidth(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags);
static kint32_t fwk_sdcard_command_set_blocksize(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags);
static kint32_t fwk_sdcard_command_pullup(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags);

static kint32_t fwk_sdcard_command_ctrl_function(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, 
                                            kuint32_t mode, kuint32_t group, kuint32_t funcIndex);
static kint32_t fwk_sdcard_command_select_function(struct fwk_sdcard *sptr_card, kuint32_t group, kuint32_t funcIndex);

static kint32_t fwk_sdcard_check_interface(struct fwk_sdcard_if *sptr_if);
static kint32_t fwk_sdcard_send_command(struct fwk_sdcard *sptr_card, fwk_sdcard_control_fn_t cmd_controller, kuint32_t args);
static kint32_t fwk_sdcard_send_app_command(struct fwk_sdcard *sptr_card, fwk_sdcard_control_fn_t cmd_controller, kuint32_t args);
static kubyte_t fwk_sdcard_calc_crc7(kubyte_t *chptr, kuint32_t lenth);
static void fwk_sdcard_add_prefix_suffix(struct fwk_sdcard_cmd *sptr_cmd);

static kint32_t fwk_sdcard_send_rw_command(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, 
                                    kuint32_t index, kuint32_t flags, kuint32_t errbit, kbool_t ctrl);
static kint32_t fwk_sdcard_read_blocks(struct fwk_sdcard *sptr_card, void *ptrBuffer, 
                                    kuint32_t iBlockStart, kuint32_t iBlockCount, kusize_t iBlockSize);
static kint32_t fwk_sdcard_write_blocks(struct fwk_sdcard *sptr_card, void *ptrBuffer, 
                                    kuint32_t iBlockStart, kuint32_t iBlockCount, kusize_t iBlockSize);
static kint32_t fwk_sdcard_erase_blocks(struct fwk_sdcard *sptr_card, kuint32_t iBlockStart, kuint32_t iBlockCount, kusize_t iBlockSize);

/*!< API function */
/*!
 * @brief   fwk_sdcard_check_interface
 * @param   none
 * @retval  none
 * @note    Check if interface is null
 */
static kint32_t fwk_sdcard_check_interface(struct fwk_sdcard_if *sptr_if)
{
    kbool_t blRetval = true;

#define mr_fwk_sdcard_check_interface(x)   \
    do {   \
        blRetval &= (!!(x));   \
    } while (0)

    mr_fwk_sdcard_check_interface(sptr_if);
    mr_fwk_sdcard_check_interface(sptr_if->sptr_host);
    mr_fwk_sdcard_check_interface(sptr_if->sptr_host->iHostIfBase);
    mr_fwk_sdcard_check_interface(sptr_if->sptr_host->iHostCDBase);

    mr_fwk_sdcard_check_interface(sptr_if->is_insert);
    mr_fwk_sdcard_check_interface(sptr_if->setBusWidth);
    mr_fwk_sdcard_check_interface(sptr_if->setClkFreq);
    mr_fwk_sdcard_check_interface(sptr_if->cardActive);
    mr_fwk_sdcard_check_interface(sptr_if->recvData);
    mr_fwk_sdcard_check_interface(sptr_if->sendData);
    mr_fwk_sdcard_check_interface(sptr_if->sendCommand);
    mr_fwk_sdcard_check_interface(sptr_if->recvResp);

#undef mr_fwk_sdcard_check_interface

    return blRetval ? ER_NORMAL : (-ER_ERROR);
}

/*!
 * @brief   fwk_sdcard_command_go_idle
 * @param   flags: none
 * @retval  none
 * @note    CMD0 Controller
 */
static kint32_t fwk_sdcard_command_go_idle(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags)
{
    kuint32_t iRespType;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_Cmd_GoIdle, FWK_SDCARD_CMD_NONE_ARGS, NR_SdCard_Response_0);

    /*!< read all response register for initial */
    iRespType = sptr_cmd->respType;
    sptr_cmd->respType = NR_SdCard_Response_2;
    sptr_if->recvResp(sptr_cmd);
    sptr_cmd->respType = iRespType;

    /*!< send command packet */
    return sptr_if->sendCommand(sptr_cmd);
}

/*!
 * @brief   fwk_sdcard_command_send_if_cond
 * @param   flags: Voltage supply
 * @retval  none
 * @note    CMD8 Controller
 */
static kint32_t fwk_sdcard_command_send_if_cond(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags)
{
    kuint32_t respValue;
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_Cmd_SendIfCond, FWK_SDCARD_CMD8_ARGS(flags), NR_SdCard_Response_7);

    mr_run_code_retry(10U,

        /*!< send command packet */
        iRetval = sptr_if->sendCommand(sptr_cmd);
        if (!(iRetval < 0))
        {
            sptr_if->recvResp(sptr_cmd);
            respValue = mr_bit_mask(*sptr_cmd->resp, FWK_SDCARD_CMD_CHECK_MASK, 0);

            if (!mr_isBitSetl(FWK_SDCARD_CMD_CHECK_PATTERN, &respValue))
                iRetval = -ER_CHECKERR;

            break;
        }
    )

    return iRetval;
}

/*!
 * @brief   fwk_sdcard_command_app_before
 * @param   flags: relative address
 * @retval  none
 * @note    CMD55 Controller
 */
static kint32_t fwk_sdcard_command_app_before(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags)
{
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_Cmd_AppCmd, mr_bit_nr(flags, 16U), NR_SdCard_Response_1);

    /*!< send command package */
    iRetval = sptr_if->sendCommand(sptr_cmd);
    if (iRetval)
        return iRetval;
    
    sptr_if->recvResp(sptr_cmd);

    /*!< if response have error bit ? */
    if (!mr_isBitResetl(NR_SdCard_R1_AllErrBit, &sptr_cmd->resp[0]))
        return -ER_SCMD_FAILD;

    /*!< if not support application command ? */
    if (!mr_isBitSetl(NR_SdCard_R1_AppCmdEnabled_Bit, &sptr_cmd->resp[0]))
        return -ER_NSUPPORT;

    return iRetval;
}

/*!
 * @brief   fwk_sdcard_command_operation_cond
 * @param   flags: Voltage supply
 * @retval  none
 * @note    CMD41 Controller
 */
static kint32_t fwk_sdcard_command_operation_cond(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags)
{
    struct fwk_sdcard *sptr_card;
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_ACmd_GetOpCond, flags, NR_SdCard_Response_3);

    /*!< get card structure */
    sptr_card = mr_container_of(sptr_if, struct fwk_sdcard, sgtc_if);

	/*!< send command package */
	iRetval = sptr_if->sendCommand(sptr_cmd);
	if (iRetval)
		return iRetval;

	sptr_if->recvResp(sptr_cmd);

	if (mr_isBitSetl(NR_SdCard_OcrNotBusy_Bit, &sptr_cmd->resp[0]))
	{
		sptr_card->ocr = sptr_cmd->resp[0];
		return ER_NORMAL;
	}

    return -ER_RETRY;
}

/*
 * @brief   fwk_sdcard_command_get_cid
 * @param   flags: none
 * @retval  none
 * @note    CMD10 Controller
 */
static kint32_t fwk_sdcard_command_get_cid(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags)
{
    struct fwk_sdcard *sptr_card;
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_Cmd_AllSendCID, FWK_SDCARD_CMD_NONE_ARGS, NR_SdCard_Response_2);

    /*!< get card structure */
    sptr_card = mr_container_of(sptr_if, struct fwk_sdcard, sgtc_if);

    /*!< send command package */
    iRetval = sptr_if->sendCommand(sptr_cmd);
    if (iRetval)
        return iRetval;
    
    sptr_if->recvResp(sptr_cmd);

    /*!< update cid */
    sptr_card->sgtc_cid.manufacturerID = (kuint8_t)(sptr_cmd->resp[3] >> 24U);
    sptr_card->sgtc_cid.applicationID = (kuint16_t)(sptr_cmd->resp[3] >> 8U);
    sptr_card->sgtc_cid.productName[0] = (kuint8_t)(sptr_cmd->resp[3]);
    sptr_card->sgtc_cid.productName[1] = (kuint8_t)(sptr_cmd->resp[2] >> 24U);
    sptr_card->sgtc_cid.productName[2] = (kuint8_t)(sptr_cmd->resp[2] >> 16U);
    sptr_card->sgtc_cid.productName[3] = (kuint8_t)(sptr_cmd->resp[2] >> 8U);
    sptr_card->sgtc_cid.productName[4] = (kuint8_t)(sptr_cmd->resp[2]);
    sptr_card->sgtc_cid.productVersion = (kuint8_t)(sptr_cmd->resp[1] >> 24U);
    sptr_card->sgtc_cid.productSerialNumber  = (kuint32_t)((sptr_cmd->resp[1] << 8U)  & 0xffffff00U);
    sptr_card->sgtc_cid.productSerialNumber |= (kuint32_t)((sptr_cmd->resp[0] >> 24U) & 0x000000ffU);
    sptr_card->sgtc_cid.manufacturerData = (kuint16_t)(sptr_cmd->resp[0] >> 8U);
    
    return iRetval;
}

/*
 * @brief   fwk_sdcard_command_get_csd
 * @param   flags: none
 * @retval  none
 * @note    CMD9 Controller
 */
static kint32_t fwk_sdcard_command_get_csd(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags)
{
    struct fwk_sdcard *sptr_card;
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_Cmd_GetCSD, FWK_SDCARD_CMD_RCA_ARGS(flags), NR_SdCard_Response_2);

    /*!< get card structure */
    sptr_card = mr_container_of(sptr_if, struct fwk_sdcard, sgtc_if);

    /*!< send command package */
    iRetval = sptr_if->sendCommand(sptr_cmd);
    if (iRetval)
        return iRetval;
    
    sptr_if->recvResp(sptr_cmd);

    /*!< update csd */
    sptr_card->sgtc_csd.csdStructure = (kuint8_t)(sptr_cmd->resp[3] >> 30U);
    sptr_card->sgtc_csd.dataReadAccessTime1 = (kuint8_t)(sptr_cmd->resp[3] >> 16U);
    sptr_card->sgtc_csd.dataReadAccessTime2 = (kuint8_t)(sptr_cmd->resp[3] >> 8U);
    sptr_card->sgtc_csd.transferSpeed = (kuint8_t)(sptr_cmd->resp[3]);
    sptr_card->sgtc_csd.cardCommandClass = (kuint16_t)(sptr_cmd->resp[2] >> 20U);
    sptr_card->sgtc_csd.readBlockLength = (kuint8_t)((sptr_cmd->resp[2] >> 16U) & 0x0fU);

    /*!< It is different for Card 1.0(SDSC) and 2.0(SDHC/SDXC) */
    switch (sptr_card->sgtc_csd.csdStructure)
    {
        /*!< Version 1.0 */
        case 0U:
            sptr_card->sgtc_csd.deviceSize  = (kuint32_t)((sptr_cmd->resp[2] << 2U)  & 0x000000fcU);
            sptr_card->sgtc_csd.deviceSize |= (kuint32_t)((sptr_cmd->resp[1] >> 30U) & 0x00000003U);
            sptr_card->sgtc_csd.readCurrentVddMin    = (kuint8_t)((sptr_cmd->resp[1] >> 27U) & 0x07U);
            sptr_card->sgtc_csd.readCurrentVddMax    = (kuint8_t)((sptr_cmd->resp[1] >> 24U) & 0x07U);
            sptr_card->sgtc_csd.writeCurrentVddMin   = (kuint8_t)((sptr_cmd->resp[1] >> 21U) & 0x07U);
            sptr_card->sgtc_csd.writeCurrentVddMax   = (kuint8_t)((sptr_cmd->resp[1] >> 18U) & 0x07U);
            sptr_card->sgtc_csd.deviceSizeMultiplier = (kuint8_t)((sptr_cmd->resp[1] >> 15U) & 0x07U);

            /*!<
                * memory capacity = block_count * block_len
                * block_count = (deviceSize + 1) * 2 ^ (deviceSizeMultiplier + 2)
                * block_len = 2 ^ (readBlockLength)
                * 
                * ===> capacity = (((deviceSize + 1) << (deviceSizeMultiplier + 2)) << readBlockLength) (bytes)
                */
            sptr_card->blockCount = mr_bit_nr(sptr_card->sgtc_csd.deviceSize + 1, (sptr_card->sgtc_csd.deviceSizeMultiplier + 2));
            sptr_card->blockSize = mr_bit(sptr_card->sgtc_csd.readBlockLength);

            if (FWK_SDCARD_DEFAULT_BLOCKSIZE != sptr_card->blockSize)
            {
                /*!< do nothing */
            }

            sptr_card->capacity = sptr_card->blockCount << sptr_card->sgtc_csd.readBlockLength;
            break;

        /*!< Version 2.0 */
        case 1U:
            sptr_card->sgtc_csd.deviceSize  = (kuint32_t)((sptr_cmd->resp[2] << 16U) & 0x00003f00U);
            sptr_card->sgtc_csd.deviceSize |= (kuint32_t)((sptr_cmd->resp[1] >> 16U) & 0x000000ffU);

            /*!<
                * memory capacity = (deviceSize + 1) * 1024 * 512bytes
                *
                * The Minimum user area size of SDHC is 4211712 sectors (2GB + 8.5MB)
                * The Minimum value of deviceSize for SDHC in CSD version 2.0 is 0x1010
                * The Maximum user area size of SDHC Card is (32GB + 80MB)
                * The Maximum value of deviceSize for SDHC in CSD version 2.0 is 0xff5f
                */
            sptr_card->blockCount = (sptr_card->sgtc_csd.deviceSize + 1) << 10U;
            sptr_card->blockSize = FWK_SDCARD_DEFAULT_BLOCKSIZE;
            sptr_card->capacity = sptr_card->blockCount << 9;
            break;

        default: break;
    }

    sptr_card->sgtc_csd.eraseSingleBlockEnable = (kuint8_t)((sptr_cmd->resp[1] >> 8U) & 0x01U);
    sptr_card->sgtc_csd.eraseSectorSize = (kuint8_t)((sptr_cmd->resp[1] >> 7U) & 0x7fU);
    sptr_card->sgtc_csd.writeProtectGroupSize = (kuint8_t)(sptr_cmd->resp[1] & 0x7fU);
    sptr_card->sgtc_csd.writeSpeedFactor = (kuint8_t)((sptr_cmd->resp[0] >> 26U) & 0x07U);
    sptr_card->sgtc_csd.writeBlockLength = (kuint8_t)((sptr_cmd->resp[0] >> 22U) & 0x0fU);
    sptr_card->sgtc_csd.fileFormat = (kuint8_t)((sptr_cmd->resp[0] >> 10U) & 0x03U);

#define mr_fwk_sdcard_csd_setflagbit(resp, resp_bit, flagbit)  \
    do {    \
        if (mr_isBitSetl(mr_bit(resp_bit), resp)) \
            mr_setbitl(flagbit, &sptr_card->sgtc_csd.flagBit);  \
    } while (0)

    /*!< fill the flagBit field */
    mr_resetl(&sptr_card->sgtc_csd.flagBit);
    mr_fwk_sdcard_csd_setflagbit(&sptr_cmd->resp[0], 12U, NR_SdCard_CsdTempWriteProtect_Bit);
    mr_fwk_sdcard_csd_setflagbit(&sptr_cmd->resp[0], 13U, NR_SdCard_CsdPermWriteProtect_Bit);
    mr_fwk_sdcard_csd_setflagbit(&sptr_cmd->resp[0], 14U, NR_SdCard_CsdCopyFlag_Bit);
    mr_fwk_sdcard_csd_setflagbit(&sptr_cmd->resp[0], 15U, NR_SdCard_CsdFileFormatGroup_Bit);
    mr_fwk_sdcard_csd_setflagbit(&sptr_cmd->resp[0], 21U, NR_SdCard_CsdPartialBlockWrite_Bit);
    mr_fwk_sdcard_csd_setflagbit(&sptr_cmd->resp[0], 31U, NR_SdCard_CsdWriteProtectEnable_Bit);
    mr_fwk_sdcard_csd_setflagbit(&sptr_cmd->resp[1], 14U, NR_SdCard_CsdEraseBlockEnale_Bit);
    mr_fwk_sdcard_csd_setflagbit(&sptr_cmd->resp[2], 12U, NR_SdCard_CsdDsrImplemented_Bit);
    mr_fwk_sdcard_csd_setflagbit(&sptr_cmd->resp[2], 13U, NR_SdCard_CsdReadBlockMisalign_Bit);
    mr_fwk_sdcard_csd_setflagbit(&sptr_cmd->resp[2], 14U, NR_SdCard_CsdWriteBlockMisalign_Bit);
    mr_fwk_sdcard_csd_setflagbit(&sptr_cmd->resp[2], 15U, NR_SdCard_CsdPartialBlockRead_Bit);

#undef mr_fwk_sdcard_csd_setflagbit

    return iRetval;
}

/*
 * @brief   fwk_sdcard_command_get_rca
 * @param   flags: none
 * @retval  none
 * @note    CMD3 Controller
 */
static kint32_t fwk_sdcard_command_get_rca(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags)
{
    struct fwk_sdcard *sptr_card;
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_Cmd_IssueRCA, FWK_SDCARD_CMD_NONE_ARGS, NR_SdCard_Response_6);

    /*!< get card structure */
    sptr_card = mr_container_of(sptr_if, struct fwk_sdcard, sgtc_if);

    /*!< send command package */
    iRetval = sptr_if->sendCommand(sptr_cmd);
    if (iRetval)
        return iRetval;
    
    sptr_if->recvResp(sptr_cmd);

    /*!< update rca: get relative address */
    /*!< bit[15:0] is card status; bit[31:16] is RCA Register */
    sptr_card->relativeAddr = sptr_cmd->resp[0] >> 16U;

    return iRetval;
}

/*
 * @brief   fwk_sdcard_command_get_scr
 * @param   flags: none
 * @retval  none
 * @note    CMD51 Controller
 */
static kint32_t fwk_sdcard_command_get_scr(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags)
{
    struct fwk_sdcard *sptr_card;
    struct fwk_sdcard_data sgtc_data;
    kuint32_t *ptrDataBuffer, iDataBufferSize;
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_ACmd_GetSCR, FWK_SDCARD_CMD_NONE_ARGS, NR_SdCard_Response_1);
    
    sgtc_data.blockSize = 8U;
    sgtc_data.blockCount = 1U;

    /*!< tell host that data will be transferred after sending commond */
    mr_resetl(&sgtc_data.flags);
    mr_setbitl(NR_SdCard_CmdFlagsReadEnable, &sgtc_data.flags);

    /*!< sizeof(iScrDataBuffer) must greater than blockCount * align4(blockSize) */
    iDataBufferSize = mr_num_align4(sgtc_data.blockSize) * sgtc_data.blockCount + 8U;

    /*!< 8U is reserved */
    ptrDataBuffer = (kuint32_t *)kzalloc(iDataBufferSize, GFP_KERNEL);
    if (!isValid(ptrDataBuffer))
        return -ER_NOMEM;

    sgtc_data.ptrHost = sptr_cmd->ptrHost;
    sgtc_data.txBuffer = mr_nullptr;
    sgtc_data.rxBuffer = ptrDataBuffer;

    sptr_cmd->ptrData = &sgtc_data;
    if (sptr_if->setup_dma)
        sptr_if->setup_dma(&sgtc_data);

    /*!< send command package */
    iRetval = sptr_if->sendCommand(sptr_cmd);
    if (iRetval)
        goto END;

    sptr_if->recvResp(sptr_cmd);
    iRetval = mr_isBitResetl(NR_SdCard_R1_AllErrBit, &sptr_cmd->resp[0]) ? ER_NORMAL : -ER_ERROR;

    /*!< receive data from data line */
    if (iRetval)
        goto END;
    
    /*!< start to receive data */
    iRetval = sptr_if->recvData(&sgtc_data);
    if (iRetval)
    {
        iRetval = ((-ER_BUSY) == iRetval) || ((-ER_TIMEOUT) == iRetval) ? -ER_RETRY : iRetval;
        goto END;
    }
    
    /*!< data line: card to host is MSB first, Big Endian */
    /*!< if host is little endian, it needs to convert */
    sgtc_data.rxBuffer[0] = mr_be32_to_cpu(sgtc_data.rxBuffer[0]);
    sgtc_data.rxBuffer[1] = mr_be32_to_cpu(sgtc_data.rxBuffer[1]);

    /*!< get card structure */
    sptr_card = mr_container_of(sptr_if, struct fwk_sdcard, sgtc_if);

    /*!< fill scr */
    sptr_card->sgtc_scr.scrStructure = (kuint8_t)((sgtc_data.rxBuffer[0] >> 28U) & 0x0fU);
    sptr_card->sgtc_scr.sdSpecification = (kuint8_t)((sgtc_data.rxBuffer[0] >> 24U) & 0x0fU);
    sptr_card->sgtc_scr.sdSecurity = (kuint8_t)((sgtc_data.rxBuffer[0] >> 20U) & 0x07U);
    sptr_card->sgtc_scr.sdBusWidths = (kuint8_t)((sgtc_data.rxBuffer[0] >> 16U) & 0x0fU);
    sptr_card->sgtc_scr.extendedSecurity = (kuint8_t)((sgtc_data.rxBuffer[0] >> 11U) & 0x0fU);
    sptr_card->sgtc_scr.commandSupport = (kuint8_t)(sgtc_data.rxBuffer[0] & 0x03U);
    sptr_card->sgtc_scr.reservedForManufacturer = sgtc_data.rxBuffer[1];
    
#define mr_fwk_sdcard_scr_setflagbit(recv, recv_bit, flagbit)  \
    do {    \
        if (mr_isBitSetl(mr_bit(recv_bit), recv)) \
            mr_setbitl(flagbit, &sptr_card->sgtc_scr.flagBit);  \
    } while (0)

    /*!< fill the flagBit field */
    mr_resetl(&sptr_card->sgtc_scr.flagBit);
    mr_fwk_sdcard_scr_setflagbit(&sgtc_data.rxBuffer[0], 15U, NR_SdCard_ScrSdSpecification3);
    mr_fwk_sdcard_scr_setflagbit(&sgtc_data.rxBuffer[0], 23U, NR_SdCard_ScrDataStatusAfterErase);

#undef mr_fwk_sdcard_scr_setflagbit

    switch (sptr_card->sgtc_scr.sdSpecification)
    {
        case 0U:
            sptr_card->version = NR_SdCard_Version1_0;
            break;

        case 1U:
            sptr_card->version = NR_SdCard_Version1_1;
            break;

        case 2U:
            sptr_card->version = (mr_isBitSetl(NR_SdCard_ScrSdSpecification3, &sptr_card->sgtc_scr.flagBit) ? 
                                                                NR_SdCard_Version3_0 : NR_SdCard_Version2_0);
            break;

        default: break;
    }

END:
    kfree(ptrDataBuffer);
    return iRetval;
}

/*
 * @brief   fwk_sdcard_command_select_card
 * @param   flags: relative address
 * @retval  none
 * @note    CMD7 Controller
 */
static kint32_t fwk_sdcard_command_select_card(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags)
{
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_Cmd_SelectCard, FWK_SDCARD_CMD_RCA_ARGS(flags), 
                                                        flags ? NR_SdCard_Response_1 : NR_SdCard_Response_0);

    /*!< send command package */
    iRetval = sptr_if->sendCommand(sptr_cmd);
    if (iRetval)
        return iRetval;
    
    sptr_if->recvResp(sptr_cmd);

    /*!< check R1 Status */
    if (NR_SdCard_Response_1 == sptr_cmd->respType)
        iRetval = mr_isBitResetl(NR_SdCard_R1_AllErrBit, &sptr_cmd->resp[0]) ? ER_NORMAL : -ER_ERROR;

    return iRetval;
}

/*
 * @brief   fwk_sdcard_command_get_status
 * @param   flags: relative address
 * @retval  none
 * @note    CMD13 Controller
 */
static kint32_t fwk_sdcard_command_get_status(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags)
{
    kbool_t blRetval;
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_Cmd_GetStatus, FWK_SDCARD_CMD_RCA_ARGS(flags), NR_SdCard_Response_1);

    do
    {
        /*!< send command package */
        iRetval = sptr_if->sendCommand(sptr_cmd);
        if (iRetval)
            continue;

        sptr_if->recvResp(sptr_cmd);

        /*!< check R1 Status */
        blRetval  = mr_isBitResetl(NR_SdCard_R1_AllErrBit, &sptr_cmd->resp[0]);
        blRetval &= mr_isBitSetl(NR_SdCard_R1_ReadyForData_Bit, &sptr_cmd->resp[0]);
        blRetval &= !mr_isBitSetl(NR_SdCard_R1_StateProgram_Bit, &sptr_cmd->resp[0]);

        iRetval = blRetval ? ER_NORMAL : (-ER_ERROR);

    } while (iRetval);

    return iRetval;
}

/*
 * @brief   fwk_sdcard_command_stop_transmission
 * @param   flags: relative address
 * @retval  none
 * @note    CMD12 Controller
 */
static kint32_t fwk_sdcard_command_stop_transmission(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags)
{
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_Cmd_StopTrans, FWK_SDCARD_CMD_NONE_ARGS, NR_SdCard_Response_1b);

    do
    {
        /*!< send command package */
        iRetval = sptr_if->sendCommand(sptr_cmd);
        if (iRetval)
            continue;
        
        sptr_if->recvResp(sptr_cmd);
        /*!< check R1 Status */


    } while (iRetval);

    return iRetval;
}

/*
 * @brief   fwk_sdcard_command_switch_voltage
 * @param   flags: none
 * @retval  none
 * @note    CMD11 Controller
 */
static kint32_t fwk_sdcard_command_switch_voltage(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags)
{
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_Cmd_SwitchVoltage, FWK_SDCARD_CMD_NONE_ARGS, NR_SdCard_Response_1);

    mr_assert(sptr_if->switchVoltage);

    /*!< send command package */
    iRetval = sptr_if->sendCommand(sptr_cmd);
    if (iRetval)
        return iRetval;

    sptr_if->recvResp(sptr_cmd);

    /*!< check R1 Status */
    if (mr_isBitResetl(NR_SdCard_R1_AllErrBit, &sptr_cmd->resp[0]))
        iRetval = sptr_if->switchVoltage(sptr_cmd->ptrHost, NR_SdCard_toVoltage1_8V);
    else
        iRetval = -ER_ERROR;

    return iRetval;
}

/*
 * @brief   fwk_sdcard_command_set_buswidth
 * @param   flags: bus width
 * @retval  none
 * @note    ACMD6 Controller
 */
static kint32_t fwk_sdcard_command_set_buswidth(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags)
{
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_ACmd_SetBusWidth, flags, NR_SdCard_Response_1);

    /*!< send command package */
    iRetval = sptr_if->sendCommand(sptr_cmd);
    if (iRetval)
        return iRetval;
    
    sptr_if->recvResp(sptr_cmd);

    /*!< check R1 Status */
    if (!mr_isBitResetl(NR_SdCard_R1_AllErrBit, &sptr_cmd->resp[0]))
        return -ER_ERROR;

    return iRetval;
}

/*
 * @brief   fwk_sdcard_command_set_blocksize
 * @param   flags: block size
 * @retval  none
 * @note    CMD16 Controller
 */
static kint32_t fwk_sdcard_command_set_blocksize(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags)
{
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_Cmd_SetBlockLenth, flags, NR_SdCard_Response_1);

    /*!< send command package */
    iRetval = sptr_if->sendCommand(sptr_cmd);
    if (iRetval)
        return iRetval;
    
    sptr_if->recvResp(sptr_cmd);

    /*!< check R1 Status */
    if (!mr_isBitResetl(NR_SdCard_R1_AllErrBit, &sptr_cmd->resp[0]))
        return -ER_ERROR;

    return iRetval;
}

/*
 * @brief   fwk_sdcard_command_ctrl_function
 * @param   flags: none
 * @retval  none
 * @note    CMD6 Controller
 */
static kint32_t fwk_sdcard_command_ctrl_function(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, 
                                                        kuint32_t mode, kuint32_t group, kuint32_t funcIndex)
{
    struct fwk_sdcard *sptr_card;
    struct fwk_sdcard_cmd6_data *sptr_func;
    struct fwk_sdcard_data sgtc_data;
    kuint32_t *ptrDataBuffer, iDataBufferSize;
    kuint8_t bufferCnt;
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_Cmd_SelectFunc, 
                                        FWK_SDCARD_CMD6_ARGS(mode, group, funcIndex), NR_SdCard_Response_1);

    /*!< get card and register structure */
    sptr_card = mr_container_of(sptr_if, struct fwk_sdcard, sgtc_if);

    /*!< if not support function switch */
    if (sptr_card->version < NR_SdCard_Version1_0)
        return -ER_NSUPPORT;

    if (mr_isBitResetl(NR_SdCard_CmdClassSwitch, &sptr_card->sgtc_csd.cardCommandClass))
        return -ER_NSUPPORT;

    sgtc_data.blockCount = 1U;
    sgtc_data.blockSize = 64U;

    /*!< set block size */
    iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_set_blocksize, sgtc_data.blockSize);
    if (iRetval)
    	return iRetval;

    /*!< tell host that data will be transferred after sending commond */
    mr_resetl(&sgtc_data.flags);
    mr_setbitl(NR_SdCard_CmdFlagsReadEnable, &sgtc_data.flags);

    sptr_func = (struct fwk_sdcard_cmd6_data *)sptr_cmd->ptrData;
    sptr_cmd->ptrData = &sgtc_data;

    iDataBufferSize = mr_num_align4(sgtc_data.blockSize) * sgtc_data.blockCount + 8U;

    /*!< 8U is reserved */
    ptrDataBuffer = (kuint32_t *)kzalloc(iDataBufferSize, GFP_KERNEL);
    if (!isValid(ptrDataBuffer))
        return -ER_NOMEM;

    sgtc_data.rxBuffer  = ptrDataBuffer;
    sgtc_data.txBuffer  = mr_nullptr;
    sgtc_data.ptrHost   = sptr_cmd->ptrHost;

    if (sptr_if->setup_dma)
        sptr_if->setup_dma(&sgtc_data);

    /*!< send command package */
    iRetval = sptr_if->sendCommand(sptr_cmd);
    if (iRetval)
        goto END;

    sptr_if->recvResp(sptr_cmd);

    /*!< check R1 Status */
    iRetval = mr_isBitResetl(NR_SdCard_R1_AllErrBit, &sptr_cmd->resp[0]) ? ER_NORMAL : -ER_ERROR;
    if (iRetval)
        goto END;
    
    /*!< start to receive data */
    iRetval = sptr_if->recvData(&sgtc_data);
    if (iRetval)
        goto END;

    /*!< endian convert */
    /*!<
    * sgtc_data.rxBuffer[0]: bit[511:480]
    * sgtc_data.rxBuffer[1]: bit[479:448]
    * sgtc_data.rxBuffer[2]: bit[447:416]
    * sgtc_data.rxBuffer[3]: bit[415:384]
    * sgtc_data.rxBuffer[4]: bit[383:352]
    */
    for (bufferCnt = 0; bufferCnt < 5; bufferCnt++)
        sgtc_data.rxBuffer[bufferCnt] = mr_be32_to_cpu(sgtc_data.rxBuffer[bufferCnt]);

    sptr_func->funcSupport[5U] = (kuint16_t)sgtc_data.rxBuffer[0U];
    sptr_func->funcSupport[4U] = (kuint16_t)(sgtc_data.rxBuffer[1U] >> 16U);
    sptr_func->funcSupport[3U] = (kuint16_t)sgtc_data.rxBuffer[1U];
    sptr_func->funcSupport[2U] = (kuint16_t)(sgtc_data.rxBuffer[2U] >> 16U);
    sptr_func->funcSupport[1U] = (kuint16_t)sgtc_data.rxBuffer[2U];
    sptr_func->funcSupport[0U] = (kuint16_t)(sgtc_data.rxBuffer[3U] >> 16U);

    sptr_func->funcMode[5U] = (kuint8_t)((sgtc_data.rxBuffer[3U] >> 12U) & 0xfU);
    sptr_func->funcMode[4U] = (kuint8_t)((sgtc_data.rxBuffer[3U] >> 8U) & 0xfU);
    sptr_func->funcMode[3U] = (kuint8_t)((sgtc_data.rxBuffer[3U] >> 4U) & 0xfU);
    sptr_func->funcMode[2U] = (kuint8_t)((sgtc_data.rxBuffer[3U]) & 0xfU);
    sptr_func->funcMode[1U] = (kuint8_t)((sgtc_data.rxBuffer[4U] >> 28U) & 0xfU);
    sptr_func->funcMode[0U] = (kuint8_t)((sgtc_data.rxBuffer[4U] >> 24U) & 0xfU);

END:
    kfree(ptrDataBuffer);
    return iRetval;
}

/*
 * @brief   fwk_sdcard_command_select_function
 * @param   flags: none
 * @retval  none
 * @note    CMD6 Controller
 */
static kint32_t fwk_sdcard_command_select_function(struct fwk_sdcard *sptr_card, kuint32_t group, kuint32_t funcIndex)
{
    mr_assert(sptr_card);

    struct fwk_sdcard_cmd sgtc_check_cmd;
    struct fwk_sdcard_cmd sgtc_switch_cmd;
    struct fwk_sdcard_cmd6_data sgtc_func;
    kint32_t iRetval;

    /*!< initial sgtc_check_cmd */
    iRetval = fwk_sdcard_command_ctrl_function(&sgtc_check_cmd, mr_nullptr, NR_SdCard_Cmd6ModeCheck, group, funcIndex);
    if (iRetval)
        return iRetval;

    if (sptr_card->sgtc_if.addHeadTail)
        sptr_card->sgtc_if.addHeadTail(&sgtc_check_cmd);

    sgtc_check_cmd.ptrHost = sptr_card->sgtc_if.sptr_host;

    /*!< initial sgtc_switch_cmd */
    iRetval = fwk_sdcard_command_ctrl_function(&sgtc_switch_cmd, mr_nullptr, NR_SdCard_Cmd6ModeSwitch, group, funcIndex);
    if (iRetval)
        return iRetval;

    if (sptr_card->sgtc_if.addHeadTail)
        sptr_card->sgtc_if.addHeadTail(&sgtc_switch_cmd);

    sgtc_switch_cmd.ptrHost = sptr_card->sgtc_if.sptr_host;

    /*!< -------------------------------------------------------------------------- */
    memset((void *)&sgtc_func, 0, sizeof(sgtc_func));
    sgtc_check_cmd.ptrData = (void *)&sgtc_func;
    iRetval = fwk_sdcard_command_ctrl_function(&sgtc_check_cmd, &sptr_card->sgtc_if, NR_SdCard_Cmd6ModeCheck, group, funcIndex);
    if (iRetval)
        return iRetval;

    /*!< check if support function [i] */
    iRetval = mr_isBitSetw(mr_bit(funcIndex), &sgtc_func.funcSupport[group]);
    if (!iRetval)
        return -ER_NSUPPORT;

    /*!< check if function [i] can be switched */
    iRetval = (funcIndex == sgtc_func.funcMode[group]);
    if (!iRetval)
        return -ER_NSUPPORT;

    /*!< -------------------------------------------------------------------------- */
    memset((void *)&sgtc_func, 0, sizeof(sgtc_func));
    sgtc_switch_cmd.ptrData = (void *)&sgtc_func;
    iRetval = fwk_sdcard_command_ctrl_function(&sgtc_switch_cmd, &sptr_card->sgtc_if, NR_SdCard_Cmd6ModeSwitch, group, funcIndex);
    if (iRetval)
        return iRetval;

    /*!< check if function [i] can be switched */
    iRetval = (funcIndex == sgtc_func.funcMode[group]);
    if (!iRetval)
        return -ER_NSUPPORT;

    return ER_NORMAL;
}

/*
 * @brief   fwk_sdcard_command_pullup
 * @param   flags: none
 * @retval  none
 * @note    ACMD42 Controller
 */
static kint32_t fwk_sdcard_command_pullup(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, kuint32_t flags)
{
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!sptr_if)
        return fwk_sdcard_initial_command(sptr_cmd, NR_SdCard_ACmd_PullUp, FWK_SDCARD_CMD_NONE_ARGS, NR_SdCard_Response_1);

    /*!< send command package */
    iRetval = sptr_if->sendCommand(sptr_cmd);
    if (iRetval)
        return iRetval;
    
    sptr_if->recvResp(sptr_cmd);

    /*!< check R1 Status */
    if (!mr_isBitResetl(NR_SdCard_R1_AllErrBit, &sptr_cmd->resp[0]))
        return -ER_ERROR;

    return iRetval;
}

/*!< -------------------------------------------------------------------------- */
/*!
 * @brief   fwk_sdcard_send_command
 * @param   none
 * @retval  none
 * @note    The size of command is 5 bytes: = 1byte(start + transfer + index) + 4bytes(args) + 1byte(crc + stop)
 */
static kint32_t fwk_sdcard_send_command(struct fwk_sdcard *sptr_card, 
                                    fwk_sdcard_control_fn_t cmd_controller, kuint32_t args)
{
    mr_assert(sptr_card);
    mr_assert(cmd_controller);

    struct fwk_sdcard_cmd sgtc_cmds;
    kint32_t iRetval;

    /*!< initial sgtc_cmds */
    iRetval = cmd_controller(&sgtc_cmds, mr_nullptr, args);
    if (iRetval)
        return iRetval;

    if (sptr_card->sgtc_if.addHeadTail)
        sptr_card->sgtc_if.addHeadTail(&sgtc_cmds);

    sgtc_cmds.ptrHost = sptr_card->sgtc_if.sptr_host;

    return cmd_controller(&sgtc_cmds, &sptr_card->sgtc_if, 0U);
}

/*!
 * @brief   fwk_sdcard_send_app_command
 * @param   none
 * @retval  none
 * @note    The size of command is 5 bytes: = 1byte(start + transfer + index) + 4bytes(args) + 1byte(crc + stop)
 */
static kint32_t fwk_sdcard_send_app_command(struct fwk_sdcard *sptr_card, 
                                        fwk_sdcard_control_fn_t cmd_controller, kuint32_t args)
{
    kuint32_t iRetry = 100U;
    kint32_t iRetval;

    do
    {    
        /*!< send cmd55 first */
        iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_app_before, sptr_card->relativeAddr);
        if (!iRetval)
        {
            /*!< send App Command */
            iRetval = fwk_sdcard_send_command(sptr_card, cmd_controller, args);
        }

        delay_ms(20);

    } while ((-ER_RETRY == iRetval) && (--iRetry));

    if ((-ER_RETRY == iRetval) || (!iRetry))
        iRetval = -ER_FAILD;

    return iRetval;
}

/*!
 * @brief   fwk_sdcard_calc_crc7
 * @param   none
 * @retval  none
 * @note    Caculate CRC Automatically
 */
static kubyte_t fwk_sdcard_calc_crc7(kubyte_t *chptr, kuint32_t lenth)
{
    kuint32_t bitCnt, byteCnt;
    kubyte_t crc, data;

    for (byteCnt = 0, crc = 0; byteCnt < lenth; byteCnt++)
    {
        data = *(chptr + byteCnt);

        for (bitCnt = 0; bitCnt < sizeof(data); bitCnt++)
        {
            crc <<= 1;

            if ((data & 0x80) ^ (crc & 0x80))
                crc ^= 0x09;

            data <<= 1;
        }
    }

    return (crc << 1);
}

/*!
 * @brief   fwk_sdcard_add_prefix_suffix
 * @param   none
 * @retval  none
 * @note    add start bit and stop bit
 */
static void fwk_sdcard_add_prefix_suffix(struct fwk_sdcard_cmd *sptr_cmd)
{
    kuint32_t calcLenth;

    if (!sptr_cmd)
        return;

    /*!< add start bit and transfer bit */
    mr_setbitl(mr_bit(7U) | mr_bit(6U), &sptr_cmd->index);

    /*!< caculate CRC */
    calcLenth = (kuint32_t)mr_member_offset(struct fwk_sdcard_cmd, crc);
    sptr_cmd->crc = fwk_sdcard_calc_crc7((kubyte_t *)sptr_cmd, calcLenth);

    /*!< bit[7:1] is CRC; bit0 is stop bit, which value is 1 */
    mr_setbitl(mr_bit(0U), &sptr_cmd->crc);
}

/*!
 * @brief   fwk_sdcard_send_rw_command
 * @param   none
 * @retval  none
 * @note    card command transfer for read-write
 */
kint32_t fwk_sdcard_send_rw_command(struct fwk_sdcard_cmd *sptr_cmd, struct fwk_sdcard_if *sptr_if, 
                                    kuint32_t index, kuint32_t flags, kuint32_t errbit, kbool_t ctrl)
{
    kint32_t iRetval;

    if (!sptr_cmd)
        return -ER_NULLPTR;

    if (!ctrl)
    {   
        iRetval = fwk_sdcard_initial_command(sptr_cmd, index, flags, 
                                (NR_SdCard_Cmd_Erase == index) ? NR_SdCard_Response_1b : NR_SdCard_Response_1);

        if (sptr_if->addHeadTail)
            sptr_if->addHeadTail(sptr_cmd);

        sptr_cmd->ptrHost = sptr_if->sptr_host;

        return iRetval;
    }

    /*!< send command package */
    iRetval = sptr_if->sendCommand(sptr_cmd);
    if (iRetval)
        return iRetval;
    
    sptr_if->recvResp(sptr_cmd);

    /*!< check R1 Status */
    return mr_isBitResetl(errbit, &sptr_cmd->resp[0]) ? ER_NORMAL : -ER_ERROR;
}

/*!
 * @brief   fwk_sdcard_read_blocks
 * @param   none
 * @retval  none
 * @note    read blocks
 */
kint32_t fwk_sdcard_read_blocks(struct fwk_sdcard *sptr_card, void *ptrBuffer, 
                                kuint32_t iBlockStart, kuint32_t iBlockCount, kusize_t iBlockSize)
{
    struct fwk_sdcard_if *sptr_if;
    struct fwk_sdcard_cmd sgtc_cmd;
    struct fwk_sdcard_data sgtc_data;
    kuint32_t index, argument;
    kuint32_t iRetry = 1U;
    kint32_t iRetval;

    sptr_if = &sptr_card->sgtc_if;
    
    /*!< get and wait for "ready for data" response */
    iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_get_status, sptr_card->relativeAddr);
    if (iRetval)
        return -ER_NREADY;

    /*!< fill cmd structure */
    index = (1U == iBlockCount) ? NR_SdCard_Cmd_ReadSingleBlock : NR_SdCard_Cmd_ReadMultiBlock;
    argument = (sptr_card->sgtc_csd.csdStructure) ? iBlockStart : (iBlockStart * iBlockSize);

    iRetval = fwk_sdcard_send_rw_command(&sgtc_cmd, sptr_if, index, argument, 0U, false);
    if (iRetval)
        return -ER_FAULT;

    /*!< fill data structure */
    sgtc_data.ptrHost = sptr_if->sptr_host;
    sgtc_data.blockCount = iBlockCount;
    sgtc_data.blockSize = iBlockSize;
    sgtc_data.rxBuffer = (kuint32_t *)ptrBuffer;
    sgtc_data.txBuffer = mr_nullptr;
    mr_resetl(&sgtc_data.flags);
    mr_setbitl(NR_SdCard_CmdFlagsReadEnable, &sgtc_data.flags);
    mr_setbitl(NR_SdCard_CmdFlagsWithBlock, &sgtc_data.flags);

    sgtc_cmd.ptrData = &sgtc_data;

    /*!< setup DMA before reading */
    if (sptr_if->setup_dma)
        sptr_if->setup_dma(&sgtc_data);

    do
    {
        iRetval = fwk_sdcard_send_rw_command(&sgtc_cmd, sptr_if,
        					index, argument, NR_SdCard_R1_AllErrBit, true);
        if (!iRetval)
            iRetval = sptr_if->recvData(&sgtc_data);

    } while (iRetval && (--iRetry));

    if (iRetval)
        return -ER_RDATA_FAILD;

    /*!< for reading multi blocks, it must send CMD12 to stop transmission */
    if ((sgtc_data.blockCount > 1U) &&
    	(mr_isBitResetl(NR_SdCard_CmdFlagsAuto12Enable, &sgtc_data.flags)))
    {
        iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_stop_transmission, 0U);
        if (iRetval)
            return -ER_STOP_FAILD;
    }

    return ER_NORMAL;
}

/*!
 * @brief   fwk_sdcard_write_blocks
 * @param   none
 * @retval  none
 * @note    write blocks
 */
kint32_t fwk_sdcard_write_blocks(struct fwk_sdcard *sptr_card, void *ptrBuffer, 
                                kuint32_t iBlockStart, kuint32_t iBlockCount, kusize_t iBlockSize)
{
    struct fwk_sdcard_if *sptr_if;
    struct fwk_sdcard_cmd sgtc_cmd;
    struct fwk_sdcard_data sgtc_data;
    kuint32_t index, argument;
    kuint32_t iRetry = 1U;
    kint32_t iRetval;

    sptr_if = &sptr_card->sgtc_if;
    
    /*!< get and wait for "ready for data" response */
    iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_get_status, sptr_card->relativeAddr);
    if (iRetval)
        return -ER_NREADY;

    /*!< fill cmd structure */
    index = (1U == iBlockCount) ? NR_SdCard_Cmd_WriteSingleBlock : NR_SdCard_Cmd_WriteMultiBlock;
    argument = (sptr_card->sgtc_csd.csdStructure) ? iBlockStart : (iBlockStart * iBlockSize);

    iRetval = fwk_sdcard_send_rw_command(&sgtc_cmd, sptr_if, index, argument, 0U, false);
    if (iRetval)
        return -ER_FAULT;

    /*!< fill data structure */
    sgtc_data.ptrHost = sptr_if->sptr_host;
    sgtc_data.blockCount = iBlockCount;
    sgtc_data.blockSize = iBlockSize;
    sgtc_data.rxBuffer = mr_nullptr;
    sgtc_data.txBuffer = (const kuint32_t *)ptrBuffer;
    mr_resetl(&sgtc_data.flags);
    mr_clrbitl(NR_SdCard_CmdFlagsReadEnable, &sgtc_data.flags);
    mr_setbitl(NR_SdCard_CmdFlagsWithBlock, &sgtc_data.flags);

    sgtc_cmd.ptrData = &sgtc_data;

    /*!< setup DMA before reading */
    if (sptr_if->setup_dma)
        sptr_if->setup_dma(&sgtc_data);

    do
    {
        iRetval = fwk_sdcard_send_rw_command(&sgtc_cmd, sptr_if,
        						index, argument, NR_SdCard_R1_AllErrBit, true);
        if (!iRetval)
            iRetval = sptr_if->sendData(&sgtc_data);

    } while (iRetval && (--iRetry));

    if (iRetval)
        return -ER_SDATA_FAILD;

    /*!< for reading multi blocks, it must send CMD12 to stop transmission */
    if ((sgtc_data.blockCount > 1U) &&
    	(mr_isBitResetl(NR_SdCard_CmdFlagsAuto12Enable, &sgtc_data.flags)))
    {
        iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_stop_transmission, 0U);
        if (iRetval)
            return -ER_STOP_FAILD;
    }

    return ER_NORMAL;
}

/*!
 * @brief   fwk_sdcard_erase_blocks
 * @param   none
 * @retval  none
 * @note    erase blocks
 */
kint32_t fwk_sdcard_erase_blocks(struct fwk_sdcard *sptr_card, kuint32_t iBlockStart, kuint32_t iBlockCount, kusize_t iBlockSize)
{
    struct fwk_sdcard_if *sptr_if;
    struct fwk_sdcard_cmd sgtc_blkStart;
    struct fwk_sdcard_cmd sgtc_blkEnd;
    struct fwk_sdcard_cmd sgtc_erase;
    kuint32_t index, argument;
    kuint32_t iRetry = 1U;
    kint32_t iRetval;

    sptr_if = &sptr_card->sgtc_if;
    
    /*!< get and wait for "ready for data" response */
    iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_get_status, sptr_card->relativeAddr);
    if (iRetval)
        return -ER_NREADY;

    /*!< --------------------------------------------------------------------- */
    /*!< fill cmd structure */
    index = NR_SdCard_Cmd_SetEraseBlkStart;
    argument = (sptr_card->sgtc_csd.csdStructure) ? iBlockStart : (iBlockStart * iBlockSize);
    iRetval = fwk_sdcard_send_rw_command(&sgtc_blkStart, sptr_if, index, argument, 0U, false);
    if (iRetval)
        return -ER_FAULT;

    do
    {
        iRetval = fwk_sdcard_send_rw_command(&sgtc_blkStart, sptr_if, index, argument, NR_SdCard_R1_AllErrBit, true);

    } while (iRetval && (--iRetry));

    if (iRetval)
        return -ER_SCMD_FAILD;

    /*!< -------------------------------------------------------------------- */
    index = NR_SdCard_Cmd_SetEraseBlkEnd;
    argument = (sptr_card->sgtc_csd.csdStructure) ? (iBlockStart + iBlockCount - 1U): ((iBlockStart + iBlockCount - 1U) * iBlockSize);
    iRetval = fwk_sdcard_send_rw_command(&sgtc_blkEnd, sptr_if, index, argument, 0U, false);
    if (iRetval)
        return -ER_FAULT;

    iRetval = fwk_sdcard_send_rw_command(&sgtc_blkEnd, sptr_if, index, argument, NR_SdCard_R1_AllErrBit, true);
    if (iRetval)
        return -ER_SCMD_FAILD;

    /*!< -------------------------------------------------------------------- */
    index = NR_SdCard_Cmd_Erase;
    argument = FWK_SDCARD_CMD_NONE_ARGS;
    iRetval = fwk_sdcard_send_rw_command(&sgtc_erase, sptr_if, index, argument, 0U, false);
    if (iRetval)
        return -ER_FAULT;

    iRetval = fwk_sdcard_send_rw_command(&sgtc_erase, sptr_if, index, argument, NR_SdCard_R1_AllErrBit, true);
    if (iRetval)
        return -ER_SCMD_FAILD;

    return ER_NORMAL;
}

/*!< -------------------------------------------------------------------------- */
/*!
 * @brief   host_sdmmc_card_initial
 * @param   none
 * @retval  none
 * @note    initial host of SD Card
 */
__weak void *host_sdmmc_card_initial(struct fwk_sdcard *sptr_card)
{
    return mr_nullptr;
}

/*!
 * @brief   check if card is inserted
 * @param   sptr_card
 * @retval  1: insert; 0: none
 * @note    card detection
 */
kbool_t fwk_sdcard_is_insert(struct fwk_sdcard *sptr_card)
{
    struct fwk_sdcard_if *sptr_if;
    struct fwk_sdcard_host *sptr_host;

    if (!isValid(sptr_card))
        return false;

    sptr_if = &sptr_card->sgtc_if;
    sptr_host = sptr_if->sptr_host;

    /*!< if card is not inserted, wait a loop */
    return sptr_if->is_insert(sptr_host);
}

/*!
 * @brief   fwk_sdcard_allocate_device
 * @param   none
 * @retval  sptr_card
 * @note    allocate card attribute
 */
void *fwk_sdcard_allocate_device(void *sptr_sd)
{
    struct fwk_sdcard *sptr_card;
    struct fwk_sdcard_host *sptr_host;
    
    if (!isValid(sptr_sd))
    {
        sptr_card = (struct fwk_sdcard *)kmalloc(sizeof(struct fwk_sdcard), GFP_KERNEL);
        if (!isValid(sptr_card))
            goto fail1;
    }
    else
        sptr_card = (struct fwk_sdcard *)sptr_sd;

    memset(sptr_card, 0, sizeof(struct fwk_sdcard));

    sptr_host = host_sdmmc_card_initial(sptr_card);
    if (!isValid(sptr_host))
        goto fail2;

    sptr_card->sgtc_if.sptr_host = sptr_host;
    sptr_card->isDynamic = mr_isNull(sptr_sd);

    /*!< Check valid */
    if (fwk_sdcard_check_interface(&sptr_card->sgtc_if))
        goto fail3;
    
    mr_setbitl(NR_SdCard_Idle_State, &sptr_card->mode);

    return sptr_card;

fail3:
    kfree(sptr_host);

fail2:
    if (!isValid(sptr_sd))
        kfree(sptr_card);

fail1:
    return mr_nullptr;
}

/*!
 * @brief   fwk_sdcard_free_device
 * @param   sptr_card
 * @retval  none
 * @note    free card attribute
 */
void fwk_sdcard_free_device(struct fwk_sdcard *sptr_card)
{
    struct fwk_sdcard_host *sptr_host;

    if (!isValid(sptr_card))
        return;

    sptr_host = sptr_card->sgtc_if.sptr_host;
    
    if (isValid(sptr_host) && sptr_host->isSelfDync)
    {
        memset(sptr_host, 0, sizeof(struct fwk_sdcard_host));
        kfree(sptr_host);
    }

    memset(sptr_card, 0, sizeof(struct fwk_sdcard));

    if (sptr_card->isDynamic)
        kfree(sptr_card);
}

/*!
 * @brief   fwk_sdcard_initial_device
 * @param   none
 * @retval  none
 * @note    detect and initial sdcard
 */
kint32_t fwk_sdcard_initial_device(struct fwk_sdcard *sptr_card)
{
    kbool_t blRetval;

    /*!< Check valid */
    if (fwk_sdcard_check_interface(&sptr_card->sgtc_if))
        return -ER_FAULT;

    blRetval = fwk_sdcard_detect(sptr_card);
    if (!blRetval)
    {
        print_debug("Can not detect SD Card! Please check if card is inserted ...\r\n");
        return -ER_NREADY;
    }

    mr_setbitl(NR_SdCard_Identify_State, &sptr_card->mode);    

    blRetval = fwk_sdcard_to_normal(sptr_card);
    if (!blRetval)
    {
        print_debug("Can not configure SD Card! Please check if card is avaliable ...\r\n");
        return -ER_FAILD;
    }

    mr_setbitl(NR_SdCard_Transfer_State, &sptr_card->mode);

    return ER_NORMAL;
}

/*!
 * @brief   fwk_sdcard_inactive_device
 * @param   sptr_card
 * @retval  none
 * @note    inactive SD Card
 */
void fwk_sdcard_inactive_device(struct fwk_sdcard *sptr_card)
{
    kint32_t iRetval;

    if (!isValid(sptr_card) || !isValid(sptr_card->sgtc_if.sptr_host))
        return;

    if (mr_isBitResetl(NR_SdCard_Transfer_State, &sptr_card->mode))
        return;

    /*!< data lines change to 1 */
    iRetval = fwk_sdcard_send_app_command(sptr_card, fwk_sdcard_command_set_buswidth, FWK_SDCARD_ACMD6_BUS_WIDTH_1);
    if (iRetval)
    {
        /*!< do nothing */
    }

    /*!< cancel card */
    iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_select_card, 0U);
    if (iRetval)
    {
        /*!< do nothing */
    }

    /*!< go idle state */
    iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_go_idle, 0U);
    if (iRetval)
    {
        /*!< do nothing */
    }

    fwk_sdcard_free_device(sptr_card);
}

/*!
 * @brief   fwk_sdcard_detect
 * @param   none
 * @retval  none
 * @note    detect and initial sdcard
 */
kbool_t fwk_sdcard_detect(struct fwk_sdcard *sptr_card)
{
    mr_assert(sptr_card);

    struct fwk_sdcard_if *sptr_if;
    struct fwk_sdcard_host *sptr_host;
    kuint32_t iOpCondCmdArgs = 0U;
    kint32_t iRetval;
    kbool_t blRetval;

    sptr_if = &sptr_card->sgtc_if;
    sptr_host = sptr_if->sptr_host;

    if (!sptr_if->addHeadTail)
        sptr_if->addHeadTail = fwk_sdcard_add_prefix_suffix;

    /*!< if card is not inserted, wait a loop */
    while (!sptr_if->is_insert(sptr_host));

    /*!< Delay some time to make card stable after inserting */
    delay_ms(500U);
    sptr_card->relativeAddr = 0U;
    sptr_card->voltage = NR_SdCard_toVoltage3_3V;

    /*!< set DATA bus width = 1bit */
    sptr_if->setBusWidth(sptr_host, NR_SdCard_BusWidth_1Bit);

    /*!< set card frequency to 400KHz */
    sptr_if->setClkFreq(sptr_host, NR_SdCard_ClkFreq_400KHz);

    /*!< sent 80 clock to sdcard, and wait done */
    blRetval = sptr_if->cardActive(sptr_host, 100U);
    if (!blRetval)
        goto fail;

    /*!< send CMD0 */
    /*!< reset sdcard, and enter idle status */
    iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_go_idle, 0U);
    if (iRetval)
        goto fail;

    /*!< send CMD8 */
    /*!< if return well, SDHC or SDXC is supported; if not, it is a SDSC card */
    iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_send_if_cond, NR_SdCard_Cmd8Vhs3_3V);
    if (!iRetval)
        mr_setbitl(NR_SdCard_OcrCapacity_Bit, &iOpCondCmdArgs);
    else
    {
        /*!< it is a SDSC. Reset sdcard to free CMD8 */
        iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_go_idle, 0U);
        if (iRetval)
            goto fail;
    }

    /*!< Voltage supply */
    mr_setbitl(NR_SdCard_OcrVdd32_33_Bit, &iOpCondCmdArgs);
    mr_setbitl(NR_SdCard_OcrVdd33_34_Bit, &iOpCondCmdArgs);
//  mr_setbitl(NR_SdCard_OcrToV18Request_Bit, &iOpCondCmdArgs);

    /*!< send ACMD41 */
    iRetval = fwk_sdcard_send_app_command(sptr_card, fwk_sdcard_command_operation_cond, iOpCondCmdArgs);
    if (iRetval)
        goto fail;

    blRetval  = mr_isBitSetl(NR_SdCard_SupportVoltage1_8V, &sptr_if->sptr_host->flagBit);
    blRetval &= mr_isBitSetl(NR_SdCard_OcrToV18Request_Bit, &sptr_card->ocr);

    /*!< switch voltage 1.8V before reading cid/csd/... */
    if (blRetval)
    {
        iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_switch_voltage, 0U);
        if (iRetval)
            goto fail;

        sptr_card->voltage = NR_SdCard_toVoltage1_8V;
    }

    /*!< get cid */
    iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_get_cid, 0U);
    if (iRetval)
        goto fail;

    /*!< get rca */
    iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_get_rca, 0U);
    if (iRetval)
        goto fail;

    if (!sptr_card->relativeAddr)
        goto fail;

    return true;

fail:
    return false;
}

/*!
 * @brief   fwk_sdcard_to_normal
 * @param   none
 * @retval  none
 * @note    configure card to work in normal mode
 */
kbool_t fwk_sdcard_to_normal(struct fwk_sdcard *sptr_card)
{
    mr_assert(sptr_card);

    struct fwk_sdcard_if *sptr_if;
    struct fwk_sdcard_host *sptr_host;
    kint32_t iRetval;

    sptr_if = &sptr_card->sgtc_if;
    sptr_host = sptr_if->sptr_host;

    if (!sptr_card->relativeAddr)
        goto fail1;

    /*!< get csd */
    iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_get_csd, sptr_card->relativeAddr);
    if (iRetval)
        goto fail1;

    /*!< set card frequency to 25MHz */
    sptr_if->setClkFreq(sptr_host, NR_SdCard_ClkFreq_25MHz);

    /*!< select card */
    iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_select_card, sptr_card->relativeAddr);
    if (iRetval)
        goto fail1;

    /*!< Pull-up disconnected during data transfer */
    iRetval = fwk_sdcard_send_app_command(sptr_card, fwk_sdcard_command_pullup, 0U);
    if (iRetval)
        goto fail2;
    
    /*!< get scr: ACMD51 */
    iRetval = fwk_sdcard_send_app_command(sptr_card, fwk_sdcard_command_get_scr, 0U);
    if (iRetval)
        goto fail2;

    /*!< check if card and host support 4bits width */
    if (!mr_isBitSetb(0x4U, &sptr_card->sgtc_scr.sdBusWidths) || 
         mr_isBitResetl(NR_SdCard_Support4BitWidth, &sptr_host->flagBit))
        goto fail2;

    /*!< set bus width */
#if defined(CONFIG_SDMMC_BUS_WIDTH) && (CONFIG_SDMMC_BUS_WIDTH == 1)
    /*!< set card DATA bus width = 1bit */
	iRetval = fwk_sdcard_send_app_command(sptr_card, fwk_sdcard_command_set_buswidth, FWK_SDCARD_ACMD6_BUS_WIDTH_1);
	if (iRetval)
		goto fail2;

    /*!< set host DATA bus width = 1bit */
	sptr_if->setBusWidth(sptr_host, NR_SdCard_BusWidth_1Bit);
#else
    /*!< set card DATA bus width = 4bit */
	iRetval = fwk_sdcard_send_app_command(sptr_card, fwk_sdcard_command_set_buswidth, FWK_SDCARD_ACMD6_BUS_WIDTH_4);
	if (iRetval)
		goto fail2;

    /*!< set host DATA bus width = 1bit */
	sptr_if->setBusWidth(sptr_host, NR_SdCard_BusWidth_4Bit);

    /*!< set card with high speed */
    iRetval = fwk_sdcard_command_select_function(sptr_card, NR_SdCard_Cmd6GroupAccess, NR_SdCard_Cmd6SpeedHigh);
    if (iRetval)
        goto fail2;
#endif

    /*!< set card drive strenth */
    iRetval = fwk_sdcard_command_select_function(sptr_card, NR_SdCard_Cmd6GroupDrvStrenth, NR_SdCard_Cmd6DriverDefault);
    if (iRetval)
        goto fail2;

    /*!< set card power current */
    iRetval = fwk_sdcard_command_select_function(sptr_card, NR_SdCard_Cmd6GroupPowerLimit, NR_SdCard_Cmd6PowerDefault);
    if (iRetval)
        goto fail2;

    /*!< set card block lenth */
    iRetval = fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_set_blocksize, FWK_SDCARD_DEFAULT_BLOCKSIZE);
    if (iRetval)
        goto fail2;

    return true;

fail2:
    /*!< cancel card */
    fwk_sdcard_send_command(sptr_card, fwk_sdcard_command_select_card, 0U);

fail1:
    return false;
}

/*!
 * @brief   fwk_sdcard_rw_blocks
 * @param   none
 * @retval  none
 * @note    read or write blocks
 */
kbool_t fwk_sdcard_rw_blocks(struct fwk_sdcard *sptr_card, void *ptrBuffer, 
                                kuint32_t iBlockStart, kuint32_t iBlockCount, nrt_fwk_sdcard_rw_t egrt_rw)
{
    mr_assert(sptr_card);

    struct fwk_sdcard_host *sptr_host;
    kuint8_t *ptrNextBuffer;
    kuint32_t iBlockTrans, iBlockSize;
    kint32_t iRetval = -ER_ERROR;

    if (!isValid(ptrBuffer))
        return false;

    sptr_host = sptr_card->sgtc_if.sptr_host;
//  if ((!sptr_host->maxBlockCount) || ((iBlockStart + iBlockCount) > sptr_host->maxBlockCount))
//      return false;

    ptrNextBuffer = (kuint8_t *)ptrBuffer;
    iBlockTrans = 0U;
    iBlockSize = FWK_SDCARD_DEFAULT_BLOCKSIZE;

    while (iBlockCount)
    {
        switch (egrt_rw)
        {
            case NR_SdCard_ReadToHost:
                iBlockTrans = mr_ret_min2(iBlockCount, sptr_host->maxBlockCount);
                iRetval = fwk_sdcard_read_blocks(sptr_card, ptrNextBuffer, iBlockStart, iBlockTrans, iBlockSize);
                break;

            case NR_SdCard_WriteToCard:
                iBlockTrans = mr_ret_min2(iBlockCount, sptr_host->maxBlockCount);
                iRetval = fwk_sdcard_write_blocks(sptr_card, ptrNextBuffer, iBlockStart, iBlockTrans, iBlockSize);
                break;

            default:
                iRetval = -ER_ERROR;
                break;
        }

        if (iRetval)
            break;

        iBlockStart += iBlockTrans;
        iBlockCount -= iBlockTrans;
        ptrNextBuffer += iBlockTrans * iBlockSize;
    }

    return (iRetval < 0) ? false : true;
}

/*!
 * @brief   fwk_sdcard_format_blocks
 * @param   none
 * @retval  none
 * @note    format(erase) blocks
 */
kbool_t fwk_sdcard_format_blocks(struct fwk_sdcard *sptr_card, kuint32_t iBlockStart, kuint32_t iBlockCount)
{
    mr_assert(sptr_card);

    kuint32_t iBlockTrans, iBlockSize;
    kint32_t iRetval = -ER_ERROR;

    if ((!sptr_card->sgtc_csd.eraseSectorSize) || ((iBlockStart + iBlockCount) > (sptr_card->sgtc_csd.eraseSectorSize + 1U)))
        return false;

    iBlockTrans = 0U;
    iBlockSize = FWK_SDCARD_DEFAULT_BLOCKSIZE;

    while (iBlockCount)
    {
        iBlockTrans = mr_ret_min2(iBlockCount, sptr_card->sgtc_csd.eraseSectorSize + 1U);
        iRetval = fwk_sdcard_erase_blocks(sptr_card, iBlockStart, iBlockTrans, iBlockSize);

        if (iRetval)
            break;

        iBlockStart += iBlockTrans;
        iBlockCount -= iBlockTrans;
    }

    return (iRetval < 0) ? false : true;
}

/* end of file */
