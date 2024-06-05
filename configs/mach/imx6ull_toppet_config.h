/*
 * Imx6ull Toppet Board Configuration
 *
 * File Name:   imx6ull_toppet_config.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.12.16
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __IMX6ULL_TOPPET_CONFIG_H
#define __IMX6ULL_TOPPET_CONFIG_H

/*!< The includes */
#include <asm/armv7/asm_config.h>
#include <board/board_config.h>

/*!< The defines */
#define CONFIG_LITTILE_ENDIAN                   1
#define CONFIG_ARCH_NUMBER                      __ARCH_ARMV7_A7_NUMBER

/*!< Clock */
#define CONFIG_XTAL_FREQ_CLK                    (24000000U)         /*!< 24MHz */
#define CONFIG_RTC_FREQ_CLK                     (32768U)            /*!< 32.768KHz */

/*!< Memory */
#define CONFIG_RAM_DDR_ORIGIN                   (0x80000000)
#define CONFIG_RAM_DDR_LENTH                    (0x20000000)

/*!< RAM(->DDR) Configuration */
/*!< ------------------------------------------------------------------------ */
/*!<
 * 0x80000000 ~ 0x801FFFFF: 2MB, reserved
 * 0x80200000 ~ 0x805FFFFF: 4MB, early stack and params(global_data)
 * 0x80600000 ~ 0x807FFFFF: 2MB, malloc len (early pool)
 * 0x80800000 ~ 0x81FFFFFF: 24MB, params (boot -> kernel), extra memory reserved
 * 0x82000000 ~ 0x820003FF: 1KB, UND, stack
 * 0x82000400 ~ 0x820403FF: 256KB, FIQ, stack
 * 0x82040400 ~ 0x820803FF: 256KB, IRQ, stack
 * 0x82080400 ~ 0x820807FF: 1KB, ABT, stack
 * 0x82080800 ~ 0x822807FF: 2MB, SVC, stack
 * 0x82280800 ~ 0x824807FF: 2MB, SYS, stack
 * 0x82480800 ~ 0x82C807FF: 8MB, heap
 * 0x82C80800 ~ 0x83C807FF: 16MB, memory pool
 * 0x83C80700 ~ 0x85FFFFFF: reserved
 * 0x86000000 ~ 0x87FFFFFF: 32MB, device tree
 */

#define CONFIG_PROGRAM_BASE                     __ALIGN_INC_8(0x88000000)
#define CONFIG_DEVICE_TREE_BASE                 __ALIGN_INC_8(0x86000000)
#define CONFIG_MEMORY_BASE                      __ALIGN_INC_8(0x82000000)

/*!< boot */
#define CONFIG_BOARD_SP_ADDR                    __ALIGN_INC_8(0x80800000)
#define CONFIG_BOOT_MALLOC_LEN                  __ALIGN_INC_8(__BYTES_UNIT_MB(2))

/*!< kernel */
#define CONFIG_FDT_MAX_SIZE                     __ALIGN_INC_8(__BYTES_UNIT_MB(32))
#define CONFIG_HEAP_SIZE                        __ALIGN_INC_8(__BYTES_UNIT_MB(8))
#define CONFIG_SYS_STACK_SIZE                   __ALIGN_INC_8(__BYTES_UNIT_MB(2))
#define CONFIG_SVC_STACK_SIZE                   __ALIGN_INC_8(__BYTES_UNIT_MB(2))
#define CONFIG_ABT_STACK_SIZE                   __ALIGN_INC_8(__BYTES_UNIT_KB(1))
#define CONFIG_IRQ_STACK_SIZE                   __ALIGN_INC_8(__BYTES_UNIT_KB(256))
#define CONFIG_FIQ_STACK_SIZE                   __ALIGN_INC_8(__BYTES_UNIT_KB(256))
#define CONFIG_UND_STACK_SIZE                   __ALIGN_INC_8(__BYTES_UNIT_KB(1))
#define CONFIG_MEMPOOL_SIZE                     __ALIGN_INC_8(__BYTES_UNIT_MB(16))
#define CONFIG_FBUFFER_SIZE                     __ALIGN_INC_8(__BYTES_UNIT_MB(24))

#define CONFIG_UND_STACK_BASE                   (CONFIG_MEMORY_BASE     + CONFIG_UND_STACK_SIZE)
#define CONFIG_FIQ_STACK_BASE                   (CONFIG_UND_STACK_BASE  + CONFIG_FIQ_STACK_SIZE)
#define CONFIG_IRQ_STACK_BASE                   (CONFIG_FIQ_STACK_BASE  + CONFIG_IRQ_STACK_SIZE)
#define CONFIG_ABT_STACK_BASE                   (CONFIG_IRQ_STACK_BASE  + CONFIG_ABT_STACK_SIZE)
#define CONFIG_SVC_STACK_BASE                   (CONFIG_ABT_STACK_BASE  + CONFIG_SVC_STACK_SIZE)
#define CONFIG_SYS_STACK_BASE                   (CONFIG_SVC_STACK_BASE  + CONFIG_SYS_STACK_SIZE)

#define CONFIG_KERNEL_MEM_BASE                  (CONFIG_MEMORY_BASE)
#define CONFIG_HEAP_STACK_BASE                  (CONFIG_SYS_STACK_BASE)
#define CONFIG_MEMORY_POOL_BASE                 (CONFIG_HEAP_STACK_BASE + CONFIG_MEMPOOL_SIZE)

/*!< Board */
#define CONFIG_LCD_PIXELBIT                     (24)

#endif /*!< __IMX6ULL_TOPPET_CONFIG_H */
