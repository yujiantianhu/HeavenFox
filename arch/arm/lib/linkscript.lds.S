/*
 * Link Script Assembly (> arch/$(ARCH)/cpu/$(TYPE)/$(CPU)/linkscript.lds)
 *
 * File Name:   linkscript.lds.S
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.12
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#include <configs/mach_configs.h>

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

/*!< Program base address (DDR) */
#if (defined(CONFIG_RAM_DDR_BASE) && defined(CONFIG_RAM_DDR_SIZE) && \
     defined(CONFIG_PROGRAM_BASE) && defined(CONFIG_PROGRAM_SIZE))

_RAM_DDR_BASE  = CONFIG_RAM_DDR_BASE;
_RAM_DDR_SIZE  = CONFIG_RAM_DDR_SIZE;
_PROGRAM_START = CONFIG_PROGRAM_BASE;
_PROGRAM_SIZE  = CONFIG_PROGRAM_SIZE;

#else
/*!<
 * Example: DDR's total size is 512MB
 * size = 512MB = 536870912(Bytes), origin = 0x80000000, end = 0xA0000000
 * 0x80000000 ~ 0x87ffffff is used as memory space, and program basic address needs to be after 0x88000000
 * lenth = (0xA0000000 - 0x88000000) = 0x18000000
 */
_RAM_DDR_BASE  = 0x80000000;
_RAM_DDR_SIZE  = 0x20000000;
_PROGRAM_START = 0x88000000;
_PROGRAM_SIZE  = 0x18000000;
#endif

/*!< Reserve Memory */
#ifdef CONFIG_RESERVE_MEMORY_SIZE
_RESERVE_MEMORY_SIZE = CONFIG_RESERVE_MEMORY_SIZE;
#else
/*!< 4MB (4K = 1page, 4MB = 1024pages) */
_RESERVE_MEMORY_SIZE = 0x400000;
#endif

/*!< 0xA0000000 - 0x400000 */
_PROGRAM_END = (_RAM_DDR_BASE + _RAM_DDR_SIZE - _RESERVE_MEMORY_SIZE);

/*!< Heap and Stack size */
#ifdef CONFIG_HEAP_SIZE
_HEAP_SIZE = CONFIG_HEAP_SIZE;
#else
/*!< 8MB */
_HEAP_SIZE = 8 * 1024 * 1024;
#endif

#ifdef CONFIG_SVC_STACK_SIZE
_SVC_STACK_SIZE = CONFIG_SVC_STACK_SIZE;
#else
/*!< 2MB */
_SVC_STACK_SIZE = 2 * 1024 * 1024;
#endif

#ifdef CONFIG_SYS_STACK_SIZE
_SYS_STACK_SIZE = CONFIG_SYS_STACK_SIZE;
#else
/*!< 2MB */
_SYS_STACK_SIZE = 2 * 1024 * 1024;
#endif

#ifdef CONFIG_IRQ_STACK_SIZE
_IRQ_STACK_SIZE = CONFIG_IRQ_STACK_SIZE;
#else
/*!< 2MB */
_IRQ_STACK_SIZE = 2 * 1024 * 1024;
#endif

#ifdef CONFIG_FIQ_STACK_SIZE
_FIQ_STACK_SIZE = CONFIG_FIQ_STACK_SIZE;
#else
/*!< 2MB */
_FIQ_STACK_SIZE = 2 * 1024 * 1024;
#endif

#ifdef CONFIG_ABT_STACK_SIZE
_ABT_STACK_SIZE = CONFIG_ABT_STACK_SIZE;
#else
/*!< 1KB */
_ABT_STACK_SIZE = 1024;
#endif

#ifdef CONFIG_UND_STACK_SIZE
_UND_STACK_SIZE = CONFIG_UND_STACK_SIZE;
#else
/*!< 1KB */
_UND_STACK_SIZE = 1024;
#endif

/*!< DMA BUffer Size */
#ifdef CONFIG_DMA_AREA_SIZE
_DMA_BUFF_SIZE = CONFIG_DMA_AREA_SIZE;
#else
/*!< 16MB */
_DMA_BUFF_SIZE = 16 * 1024 * 1024;
#endif

/*!< framebuffer ram */
#ifdef CONFIG_FB_DRAM_SIZE
_FB_DRAM_SIZE = CONFIG_FB_DRAM_SIZE;
#else
/*!< 
 * Default: support three buffer with 1080p
 * 1080p = 1920 * 1080 * 4 ≈ 8MB, three buffer is 24MB
 * 2K = 2560 * 1440 * 4 = 14MB
 * 4K = 3840 * 2160 * 4 = 31.65MB
 * therefore, support at less 2K
 */
_FB_DRAM_SIZE = 24 * 1024 * 1024;
#endif

/*!< NetWork Tx/Rx Buffer */
#ifdef CONFIG_NETWORK_BUFF_SIZE
_NETWORK_BUFF_SIZE = CONFIG_NETWORK_BUFF_SIZE;
#else
/*!< 8MB */
_NETWORK_BUFF_SIZE = 8 * 1024 * 1024;
#endif

/*!< RAM DDR Configuration  */
MEMORY
{
#if (defined(CONFIG_PROGRAM_BASE) && defined(CONFIG_PROGRAM_SIZE))
    ram_ddr_0 : ORIGIN = CONFIG_PROGRAM_BASE, LENGTH = CONFIG_PROGRAM_SIZE
#else
    ram_ddr_0 : ORIGIN = 0x88000000, LENGTH = 0x18000000
#endif
}

/*!< Sections  */
ENTRY(_start)
SECTIONS
{
    . = _PROGRAM_START;
    . = ALIGN(16);

    __ram_ddr_start = .;

    .text :
    {
        . = ALIGN(16);
        __vector_table = .;

        KEEP(*(.vectors))
        KEEP(*(.start))
        KEEP(*(.head))
        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t.*)
        *(.plt)
        *(.gnu_warning)
        *(.gcc_execpt_table)
        *(.glue_7)
        *(.glue_7t)
        *(.vfp11_veneer)
        *(.ARM.extab)
        *(.gnu.linkonce.armextab.*)
        *(.note.gnu.build-id)
    } > ram_ddr_0

    __text_sec_end = .;

    .init : 
    {
        KEEP (*(.init))
    } > ram_ddr_0

    .fini : 
    {
        KEEP (*(.fini))
    } > ram_ddr_0

    .dync_init : 
    {
        . = ALIGN(16);
        __dync_init_start = .;
        *(.dync_init)
        __dync_init_0_start = .;
        KEEP(*(.dync_init.0*))
        __dync_init_1_start = .;
        KEEP(*(.dync_init.1*))
        __dync_init_2_start = .;
        KEEP(*(.dync_init.2*))
        __dync_init_3_start = .;
        KEEP(*(.dync_init.3*))
        __dync_init_4_start = .;
        KEEP(*(.dync_init.4*))
        __dync_init_5_start = .;
        KEEP(*(.dync_init.5*))
        __dync_init_6_start = .;
        KEEP(*(.dync_init.6*))
        __dync_init_7_start = .;
        KEEP(*(.dync_init.7*))
        __dync_init_8_start = .;
        KEEP(*(.dync_init.8*))
        __dync_init_9_start = .;
        KEEP(*(.dync_init.9*))
        __dync_init_9_end = .;
        __dync_init_end = .;
    } > ram_ddr_0

    .dync_exit : 
    {
        . = ALIGN(16);
        __dync_exit_start = .;
        *(.dync_exit)
        __dync_exit_0_start = .;
        KEEP(*(.dync_exit.0*))
        __dync_exit_1_start = .;
        KEEP(*(.dync_exit.1*))
        __dync_exit_2_start = .;
        KEEP(*(.dync_exit.2*))
        __dync_exit_3_start = .;
        KEEP(*(.dync_exit.3*))
        __dync_exit_4_start = .;
        KEEP(*(.dync_exit.4*))
        __dync_exit_5_start = .;
        KEEP(*(.dync_exit.5*))
        __dync_exit_6_start = .;
        KEEP(*(.dync_exit.6*))
        __dync_exit_7_start = .;
        KEEP(*(.dync_exit.7*))
        __dync_exit_8_start = .;
        KEEP(*(.dync_exit.8*))
        __dync_exit_9_start = .;
        KEEP(*(.dync_exit.9*))
        __dync_exit_9_end = .;
        __dync_exit_end = .;
    } > ram_ddr_0

    __dync_sec_end = .;

    . = ALIGN(16);
    .rodata :
    {
        __rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        __rodata_end = .;
    } > ram_ddr_0

    . = ALIGN(16);
    .data :
    {
        __data_start = .;
        *(.data)
        *(.data.*)
        __data_end = .;
    } > ram_ddr_0

    . = ALIGN(16);
    .ARM.exidx : 
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        *(.gnu.linkonce.armexidix.*.*)
        __exidx_end = .;
    } > ram_ddr_0

    .preinit_array : 
    {
        __preinit_array_start = .;
        KEEP (*(SORT(.preinit_array.*)))
        KEEP (*(.preinit_array))
        __preinit_array_end = .;
    } > ram_ddr_0

    .init_array : 
    {
        __init_array_start = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;
    } > ram_ddr_0

    .fini_array : 
    {
        __fini_array_start = .;
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array))
        __fini_array_end = .;
    } > ram_ddr_0

    . = ALIGN(16);
    .ARM.attributes 0 : 
    { 
        __ARM_attributes_start = .;
        *(.ARM.attributes)
        __ARM_attributes_end = .;
    } > ram_ddr_0

    .sdata :
    {
        __sdata_start = .;
        *(.sdata)
        *(.sdata.*)
        __sdata_end = .;

    } > ram_ddr_0

    .tdata :
    {
        __tdata_start = .;
        *(.tdata)
        *(.tdata.*)
        __tdata_end = .;

    } > ram_ddr_0

    . = ALIGN(16);
    .bss (NOLOAD) :
    {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    } > ram_ddr_0
    
    /*!< heap */
    .heap (NOLOAD) :
    {
        . = ALIGN(16);
        __heap_start = .;
        . += _HEAP_SIZE;
        __heap_end = .;
    } > ram_ddr_0

    /*!< stack */
    .stack (NOLOAD) :
    {
        . = ALIGN(16);
        __stack_start = .;
        __svc_stack_start = .;
        . += _SVC_STACK_SIZE;
        . = ALIGN(16);
        __svc_stack_end = .;
        __sys_stack_start = .;
        . += _SYS_STACK_SIZE;
        . = ALIGN(16);
        __sys_stack_end = .;
        __irq_stack_start = .;
        . += _IRQ_STACK_SIZE;
        . = ALIGN(16);
        __irq_stack_end = .;
        __fiq_stack_start = .;
        . += _FIQ_STACK_SIZE;
        . = ALIGN(16);
        __fiq_stack_end = .;
        __abt_stack_start = .;
        . += _ABT_STACK_SIZE;
        . = ALIGN(16);
        __abt_stack_end = .;
        __und_stack_start = .;
        . += _UND_STACK_SIZE;
        . = ALIGN(16);
        __und_stack_end = .;
        __stack_end = .;
    } > ram_ddr_0

    /*!< DMA */
    .dma_area (NOLOAD) :
    {
        . = ALIGN(16);
        __dma_area_start = .;
        . += _DMA_BUFF_SIZE;
        __dma_area_end = .;
    } > ram_ddr_0

    /*!< DRAM */
    .fb_dram (NOLOAD) :
    {
        . = ALIGN(16);
        __fb_dram_start = .;
        . += _FB_DRAM_SIZE;
        __fb_dram_end = .;
    } > ram_ddr_0

    /*!< Network Buffer */
    .network_buffer (NOLOAD) :
    {
        . = ALIGN(16);
        __sk_buffer_start = .;
        . += _NETWORK_BUFF_SIZE;
        __sk_buffer_end = .;
    } > ram_ddr_0

    /*!< memory pool: must at the end of RAM */
    . = ALIGN(16);
    __mem_pool_size = _PROGRAM_END - .;

    .mem_pool (NOLOAD) :
    {
        __mem_pool_start = .;
        . += __mem_pool_size;
        __mem_pool_end = .;
    } > ram_ddr_0

    __ram_ddr_end = .;
}
