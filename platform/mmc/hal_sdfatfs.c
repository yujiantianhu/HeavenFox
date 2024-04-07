/*
 * General SD Card FatFs Interface
 *
 * File Name:   hal_sdfatfs.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.11.11
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#include <platform/kmem_pool.h>
#include <platform/mmc/hal_sdcard.h>
#include <platform/mmc/hal_sdfatfs.h>

/*!< The globals */
static srt_hal_sdcard_t sgrt_sdFatfsDisk;

/*!< API function */
/*!
 * @brief   hal_sdfatfs_write
 * @param   none
 * @retval  none
 * @note    write SD Card by Fatfs
 */
DRESULT hal_sdfatfs_write(kuint8_t physicalDrive, const kuint8_t *buffer, kuint32_t sector, kuint8_t count)
{
    if (physicalDrive != SDDISK)
    {
        return RES_PARERR;
    }

    if (!hal_sdcard_rw_blocks(&sgrt_sdFatfsDisk, (void *)buffer, sector, count, Ert_HAL_SDCard_WriteToCard))
    {
        return RES_ERROR;
    }

    return RES_OK;
}

/*!
 * @brief   hal_sdfatfs_read
 * @param   none
 * @retval  none
 * @note    read SD Card by Fatfs
 */
DRESULT hal_sdfatfs_read(kuint8_t physicalDrive, kuint8_t *buffer, kuint32_t sector, kuint8_t count)
{
    if (physicalDrive != SDDISK)
    {
        return RES_PARERR;
    }

    if (!hal_sdcard_rw_blocks(&sgrt_sdFatfsDisk, buffer, sector, count, Ert_HAL_SDCard_ReadToHost))
    {
        return RES_ERROR;
    }

    return RES_OK;
}

/*!
 * @brief   hal_sdfatfs_ioctl
 * @param   none
 * @retval  none
 * @note    control SD Card by Fatfs
 */
DRESULT hal_sdfatfs_ioctl(kuint8_t physicalDrive, kuint8_t command, void *buffer)
{
    DRESULT result = RES_OK;

    if (physicalDrive != SDDISK)
    {
        return RES_PARERR;
    }

    switch (command)
    {
        case GET_SECTOR_COUNT:
            if (buffer)
            {
                *(kuint32_t *)buffer = sgrt_sdFatfsDisk.blockCount;
            }
            else
            {
                result = RES_PARERR;
            }
            break;
        case GET_SECTOR_SIZE:
            if (buffer)
            {
                *(kuint32_t *)buffer = sgrt_sdFatfsDisk.blockSize;
            }
            else
            {
                result = RES_PARERR;
            }
            break;
        case GET_BLOCK_SIZE:
            if (buffer)
            {
                *(kuint32_t *)buffer = sgrt_sdFatfsDisk.sgrt_csd.eraseSectorSize;
            }
            else
            {
                result = RES_PARERR;
            }
            break;
        case CTRL_SYNC:
            result = RES_OK;
            break;
        default:
            result = RES_PARERR;
            break;
    }

    return result;
}

/*!
 * @brief   hal_sdfatfs_status
 * @param   none
 * @retval  none
 * @note    check if SD Card
 */
DSTATUS hal_sdfatfs_status(kuint8_t physicalDrive)
{
    if (physicalDrive != SDDISK)
    {
        return STA_NOINIT;
    }

	if (mrt_isBitResetl(Ert_HAL_SDCard_Transfer_State, &sgrt_sdFatfsDisk.mode))
	{
		return STA_NOINIT;
	}

    return 0;
}

/*!
 * @brief   hal_sdfatfs_initial
 * @param   none
 * @retval  none
 * @note    initial SD Card by Fatfs
 */
DSTATUS hal_sdfatfs_initial(kuint8_t physicalDrive)
{
    srt_hal_sdcard_t *sprt_card;
    ksint32_t iRetval;

    if (physicalDrive != SDDISK)
    {
        return STA_NOINIT;
    }

    /*!< allocate sdcard structure or get host */
    sprt_card = hal_sdcard_allocate_device(&sgrt_sdFatfsDisk);
    if (!mrt_isValid(sprt_card))
    {
        return STA_NOINIT;
    }

    /*!< detect and initial */
    iRetval = hal_sdcard_initial_device(sprt_card);
    if (mrt_isErr(iRetval))
    {
       hal_sdcard_free_device(sprt_card);
       return STA_NOINIT;
    }

    return 0;
}

/*!
 * @brief   hal_sdfatfs_release
 * @param   none
 * @retval  none
 * @note    release SD Card by Fatfs
 */
DSTATUS hal_sdfatfs_release(kuint8_t physicalDrive)
{
    if (physicalDrive != SDDISK)
    {
        return STA_NOINIT;
    }

	hal_sdcard_inactive_device(&sgrt_sdFatfsDisk);

	return 0;
}

/*!< end of file */
