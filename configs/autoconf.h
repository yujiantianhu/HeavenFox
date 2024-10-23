#define CONFIG_ARCH "arm"
#define CONFIG_TYPE "armv7"
#define CONFIG_VERDOR "xilinx"
#define CONFIG_CPU "xc7z010"
#define CONFIG_COMPILER "arm-linux-gnueabihf-"
#define CONFIG_COMPILER_PATH "/usr/local/arm/gcc-linaro-5.3-2016.02-x86_64_arm-linux-gnueabihf"
#define CONFIG_ARCH_ARMV7 1
#define CONFIG_MACH_XC7Z010_MICROPHASE 1
#define CONFIG_LITTILE_ENDIAN 1
#define CONFIG_XTAL_FREQ_CLK (24000000)
#define CONFIG_RTC_FREQ_CLK (32768)
#define CONFIG_RAM_DDR_ORIGIN (0x00100000)
#define CONFIG_RAM_DDR_LENTH (0x0FF00000)
#define CONFIG_PROGRAM_OFFSET (0x04000000)
#define CONFIG_PROGRAM_BASE (0x00100000)
#define CONFIG_DEVICE_TREE_BASE (0x02100000)
#define CONFIG_MEMORY_BASE (0x01900000)
#define CONFIG_BOARD_SP_ADDR (0x00900000)
#define CONFIG_BOOT_MALLOC_LEN (2 * 1024 * 1024)
#define CONFIG_FDT_MAX_SIZE (32 * 1024 * 1024)
#define CONFIG_SYS_STACK_SIZE (2 * 1024 * 1024)
#define CONFIG_SVC_STACK_SIZE (2 * 1024 * 1024)
#define CONFIG_ABT_STACK_SIZE (1 * 1024)
#define CONFIG_IRQ_STACK_SIZE (256 * 1024)
#define CONFIG_FIQ_STACK_SIZE (256 * 1024)
#define CONFIG_UND_STACK_SIZE (1 * 1024)
#define CONFIG_MEMPOOL_SIZE (16 * 1024 * 1024)
#define CONFIG_FBUFFER_SIZE (24 * 1024 * 1024)
#define CONFIG_UND_STACK_BASE (CONFIG_MEMORY_BASE     + CONFIG_UND_STACK_SIZE)
#define CONFIG_FIQ_STACK_BASE (CONFIG_UND_STACK_BASE  + CONFIG_FIQ_STACK_SIZE)
#define CONFIG_IRQ_STACK_BASE (CONFIG_FIQ_STACK_BASE  + CONFIG_IRQ_STACK_SIZE)
#define CONFIG_ABT_STACK_BASE (CONFIG_IRQ_STACK_BASE  + CONFIG_ABT_STACK_SIZE)
#define CONFIG_SVC_STACK_BASE (CONFIG_ABT_STACK_BASE  + CONFIG_SVC_STACK_SIZE)
#define CONFIG_SYS_STACK_BASE (CONFIG_SVC_STACK_BASE  + CONFIG_SYS_STACK_SIZE)
#define CONFIG_LCD_PIXELBIT (24)
#define CONFIG_OF 1
#define CONFIG_PINCTRL 1
#define CONFIG_MMC 1
#define CONFIG_I2C 1
#define CONFIG_INPUTDEV 1
#define CONFIG_VIDEO 1
#define CONFIG_USB 1
#define CONFIG_CPU_ZYNQ 1
#define CONFIG_CLOCK_ZYNQ 1
#define CONFIG_BUTTON_APP 1
#define CONFIG_DISPLAY_APP 1
#define CONFIG_ENV_MONITOR_APP 1
#define CONFIG_LIGHT_APP 1