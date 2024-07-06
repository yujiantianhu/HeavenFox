/*
 * xc7z010 microphase Board Configuration
 *
 * File Name:   zynq7_microphase_config.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.12.16
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __ZYNQ7_MICROPHASE_CONFIG_H
#define __ZYNQ7_MICROPHASE_CONFIG_H

/*!< The includes */
#include <asm/armv7/asm_config.h>
#include <board/board_config.h>

/*!< The defines */
#define CONFIG_LITTILE_ENDIAN                   1
#define CONFIG_ARCH_NUMBER                      __ARCH_ARMV7_A9_NUMBER

/*!< Clock */
#define CONFIG_XTAL_FREQ_CLK                    (24000000U)         /*!< 24MHz */
#define CONFIG_RTC_FREQ_CLK                     (32768U)            /*!< 32.768KHz */

/*!< Memory */
#define CONFIG_RAM_DDR_ORIGIN                   (0x00100000)
#define CONFIG_RAM_DDR_LENTH                    (0x0FF00000)

#define CONFIG_PROGRAM_OFFSET                   (0x04000000)        /*!< 64MB */

/*!< RAM(->DDR) Configuration */
/*!< ------------------------------------------------------------------------ */
/*!<
 * ----------------< 64MB >-------------------------
 * 0x00100000 ~ 0x002FFFFF: 2MB, reserved
 * 0x00300000 ~ 0x006FFFFF: 4MB, early stack and params(global_data)
 * 0x00700000 ~ 0x008FFFFF: 2MB, malloc len (early pool)
 * 0x00900000 ~ 0x018FFFFF: 16MB, params (boot -> kernel), extra memory reserved
 * 0x01900000 ~ 0x019003FF: 1KB, UND, stack
 * 0x01900400 ~ 0x019403FF: 256KB, FIQ, stack
 * 0x01940400 ~ 0x019803FF: 256KB, IRQ, stack
 * 0x01980400 ~ 0x019807FF: 1KB, ABT, stack
 * 0x01980800 ~ 0x01B807FF: 2MB, SVC, stack
 * 0x01B80800 ~ 0x01D807FF: 2MB, SYS, stack
 * 0x01D80800 ~ 0x020FFFFF: reserved
 * 0x02100000 ~ 0x040FFFFF: 32MB, device tree
 */

#define CONFIG_PROGRAM_BASE                     __ALIGN_INC_8(0x00100000)
#define CONFIG_DEVICE_TREE_BASE                 __ALIGN_INC_8(0x02100000)
#define CONFIG_MEMORY_BASE                      __ALIGN_INC_8(0x01900000)

/*!< boot */
#define CONFIG_BOARD_SP_ADDR                    __ALIGN_INC_8(0x00900000)
#define CONFIG_BOOT_MALLOC_LEN                  __ALIGN_INC_8(__BYTES_UNIT_MB(2))

/*!< kernel */
#define CONFIG_FDT_MAX_SIZE                     __ALIGN_INC_8(__BYTES_UNIT_MB(32))
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

/*!< Board */
#define CONFIG_LCD_PIXELBIT                     (24)

#endif /*!< __ZYNQ7_MICROPHASE_CONFIG_H */
