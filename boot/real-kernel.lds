/*
 * Link Script
 *
 * File Name:   real-kernel.lds
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.12
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

/*!< Program base address */
_PROGRAM_START = DEFINED(CONFIG_PROGRAM_BASE) ? CONFIG_PROGRAM_BASE : 0x88000000;

/*!< Heap and Stack size */
_HEAP_SIZE = DEFINED(CONFIG_HEAP_SIZE) ? CONFIG_HEAP_SIZE : (8 * 1024 * 1024);
_SVC_STACK_SIZE = DEFINED(CONFIG_SVC_STACK_SIZE) ? CONFIG_SVC_STACK_SIZE : (2 * 1024 * 1024);
_SYS_STACK_SIZE = DEFINED(CONFIG_SYS_STACK_SIZE) ? CONFIG_SYS_STACK_SIZE : (2 * 1024 * 1024);
_IRQ_STACK_SIZE = DEFINED(CONFIG_IRQ_STACK_SIZE) ? CONFIG_IRQ_STACK_SIZE : (256 * 1024);
_FIQ_STACK_SIZE = DEFINED(CONFIG_FIQ_STACK_SIZE) ? CONFIG_FIQ_STACK_SIZE : (256 * 1024);
_ABT_STACK_SIZE = DEFINED(CONFIG_ABT_STACK_SIZE) ? CONFIG_ABT_STACK_SIZE : (1024);
_UND_STACK_SIZE = DEFINED(CONFIG_UND_STACK_SIZE) ? CONFIG_UND_STACK_SIZE : (1024);

/*!< memory pool size */
_MEM_POOL_SIZE  = DEFINED(CONFIG_MEM_POOL_SIZE) ? CONFIG_MEM_POOL_SIZE : (16 * 1024 * 1024);

MEMORY
{
    /*!<
     * DDR: size = 512MB = 536870912(Bytes), origin = 0x80000000, end = 0xA0000000
     * 0x80000000 ~ 0x88000000 is used as memory space, and program basic address needs to be after 0x88000000
     * lenth = (0xA0000000 - 0x88000000) = 0x18000000
     */
    ram_ddr_0 : ORIGIN = 0x88000000, LENGTH = 0x18000000
}

ENTRY(_start)
SECTIONS
{
    . = _PROGRAM_START;
    . = ALIGN(16);

    __ram_ddr_start = .;

    .text :
    {
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

    .ARM.exidx : 
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        *(.gnu.linkonce.armexidix.*.*)
        __exidx_end = .;
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

    /*!< memory pool */
    .mem_pool (NOLOAD) :
    {
        . = ALIGN(16);
        __mem_pool_start = .;
        . += _MEM_POOL_SIZE;
        __mem_pool_end = .;
    } > ram_ddr_0

    __ram_ddr_end = .;

    .ARM.attributes 0 : 
    { 
        __ARM_attributes_start = .;
        *(.ARM.attributes)
        __ARM_attributes_end = .;
    }
}
