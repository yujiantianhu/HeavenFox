/*
 * IMX6ULL Board General Initial
 *
 * File Name:   board_f.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <boot/board_init.h>

/*!< API function */
/*!
 * @brief   board_init_f_alloc_reserve
 * @param   none
 * @retval  none
 * @note    reserve Malloc_Len and sizeof(Global data)
 */
kuaddr_t board_init_f_alloc_reserve(kuaddr_t base)
{
    base -= CONFIG_BOOT_MALLOC_LEN;
    base  = mr_align(base, 16);

    base -= sizeof(srt_gd_t);
    base  = mr_align(base, 16);

    return base;
}

/*!
 * @brief   board_init_f_init_reserve
 * @param   none
 * @retval  none
 * @note    fill global data: gd structure
 */
void board_init_f_init_reserve(kuaddr_t base)
{
    srt_gd_t *sptr_gd;

    sptr_gd = (srt_gd_t *)base;
    kmemzero(sptr_gd, sizeof(srt_gd_t));

    /*!< save stack base address */
    sptr_gd->iboot_sp = (kuaddr_t)base;

    base += sizeof(srt_gd_t);
    base  = mr_align(base, 16);

    sptr_gd->malloc_addr = base;
    sptr_gd->malloc_len  = CONFIG_BOOT_MALLOC_LEN;

}

/*!
 * @brief   board_init_malloc_space
 * @param   none
 * @retval  none
 * @note    initial malloc_space
 */
kint32_t board_init_malloc_space(void)
{
    srt_gd_t *sptr_gd;
    kbool_t retval;

    sptr_gd = board_get_gd();

    retval = memory_block_self_defines(-1, sptr_gd->malloc_addr, sptr_gd->malloc_len);
    return (retval) ? RET_BOOT_PASS : RET_BOOT_ERR;
}

/*!
 * @brief   board_init_console
 * @param   none
 * @retval  none
 * @note    initial console
 */
kint32_t board_init_console(void)
{
	board_init_serial();

    /*!< Output Board Information */
    print_info("\r\n");

    for (kint32_t i = 0; i < 80; i++)
    {
        print_info(">");
        mdelay(5U);
    }

    print_info("\r\n");
    print_info("Console Initial Finished ...\r\n");
    print_info("Welcome to Use HeavenFox OS!\r\n");
    print_info("\r\n");

    return RET_BOOT_PASS;
}

/*!< board init queue early */
board_init_t board_init_sequence_f[] =
{
    /*!< init memory alloc sapce */
    board_init_malloc_space,

    /*!< init serial output */
    board_init_console,

    /*!< init sdio peripheral */
    board_init_sdmmc,
    
    mr_nullptr,
};

/*!
 * @brief   board_init_f
 * @param   none
 * @retval  none
 * @note    board early initial
 */
void board_init_f(void)
{
    /*!< initial */
    if (board_initcall_run_list(board_init_sequence_f))
        for (;;);
}

/* end of file */
