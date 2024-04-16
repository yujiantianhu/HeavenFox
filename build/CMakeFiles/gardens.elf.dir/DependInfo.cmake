
# Consider dependencies only in project.
set(CMAKE_DEPENDS_IN_PROJECT_ONLY OFF)

# The set of languages for which implicit dependencies are needed:
set(CMAKE_DEPENDS_LANGUAGES
  "ASM"
  )
# The set of files for implicit dependencies of each language:
set(CMAKE_DEPENDS_CHECK_ASM
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/arch/arm/cpu/armv7/lowlevel_init.S" "/home/ubuntu/ProgramWorks/MyDesign/Gardens/build/CMakeFiles/gardens.elf.dir/arch/arm/cpu/armv7/lowlevel_init.S.o"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/arch/arm/cpu/armv7/start.S" "/home/ubuntu/ProgramWorks/MyDesign/Gardens/build/CMakeFiles/gardens.elf.dir/arch/arm/cpu/armv7/start.S.o"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/arch/arm/kernel/context.S" "/home/ubuntu/ProgramWorks/MyDesign/Gardens/build/CMakeFiles/gardens.elf.dir/arch/arm/kernel/context.S.o"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/arch/arm/kernel/head-common.S" "/home/ubuntu/ProgramWorks/MyDesign/Gardens/build/CMakeFiles/gardens.elf.dir/arch/arm/kernel/head-common.S.o"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/arch/arm/kernel/head.S" "/home/ubuntu/ProgramWorks/MyDesign/Gardens/build/CMakeFiles/gardens.elf.dir/arch/arm/kernel/head.S.o"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/arch/arm/lib/crt0.S" "/home/ubuntu/ProgramWorks/MyDesign/Gardens/build/CMakeFiles/gardens.elf.dir/arch/arm/lib/crt0.S.o"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/arch/arm/lib/lib1funcs.S" "/home/ubuntu/ProgramWorks/MyDesign/Gardens/build/CMakeFiles/gardens.elf.dir/arch/arm/lib/lib1funcs.S.o"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/arch/arm/lib/vectors.S" "/home/ubuntu/ProgramWorks/MyDesign/Gardens/build/CMakeFiles/gardens.elf.dir/arch/arm/lib/vectors.S.o"
  )
set(CMAKE_ASM_COMPILER_ID "GNU")

# Preprocessor definitions for this target.
set(CMAKE_TARGET_DEFINITIONS_ASM
  "CONFIG_DEBUG_JTAG"
  "CONFIG_MACH_IMX6ULL_TOPPET"
  )

# The include file search paths:
set(CMAKE_ASM_TARGET_INCLUDE_PATH
  "../"
  "../arch/arm/include"
  "../include"
  "../board/imx6ull"
  "../rootfs"
  "../rootfs/fatfs"
  )

# The set of dependency files which are needed:
set(CMAKE_DEPENDS_DEPENDENCY_FILES
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/arch/arm/cpu/armv7/imx6ull/board.c" "CMakeFiles/gardens.elf.dir/arch/arm/cpu/armv7/imx6ull/board.c.o" "gcc" "CMakeFiles/gardens.elf.dir/arch/arm/cpu/armv7/imx6ull/board.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/arch/arm/cpu/armv7/irq.c" "CMakeFiles/gardens.elf.dir/arch/arm/cpu/armv7/irq.c.o" "gcc" "CMakeFiles/gardens.elf.dir/arch/arm/cpu/armv7/irq.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/arch/arm/lib/exception.c" "CMakeFiles/gardens.elf.dir/arch/arm/lib/exception.c.o" "gcc" "CMakeFiles/gardens.elf.dir/arch/arm/lib/exception.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/arch/arm/lib/interrupt.c" "CMakeFiles/gardens.elf.dir/arch/arm/lib/interrupt.c.o" "gcc" "CMakeFiles/gardens.elf.dir/arch/arm/lib/interrupt.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/board/mach-imx6ull/board_init.c" "CMakeFiles/gardens.elf.dir/board/mach-imx6ull/board_init.c.o" "gcc" "CMakeFiles/gardens.elf.dir/board/mach-imx6ull/board_init.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/board/mach-imx6ull/imx6_console.c" "CMakeFiles/gardens.elf.dir/board/mach-imx6ull/imx6_console.c.o" "gcc" "CMakeFiles/gardens.elf.dir/board/mach-imx6ull/imx6_console.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/board/mach-imx6ull/imx6_gadget.c" "CMakeFiles/gardens.elf.dir/board/mach-imx6ull/imx6_gadget.c.o" "gcc" "CMakeFiles/gardens.elf.dir/board/mach-imx6ull/imx6_gadget.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/board/mach-imx6ull/imx6_led.c" "CMakeFiles/gardens.elf.dir/board/mach-imx6ull/imx6_led.c.o" "gcc" "CMakeFiles/gardens.elf.dir/board/mach-imx6ull/imx6_led.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/board/mach-imx6ull/imx6_sdmmc.c" "CMakeFiles/gardens.elf.dir/board/mach-imx6ull/imx6_sdmmc.c.o" "gcc" "CMakeFiles/gardens.elf.dir/board/mach-imx6ull/imx6_sdmmc.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/board/mach-imx6ull/imx6_systick.c" "CMakeFiles/gardens.elf.dir/board/mach-imx6ull/imx6_systick.c.o" "gcc" "CMakeFiles/gardens.elf.dir/board/mach-imx6ull/imx6_systick.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/board/mach-imx6ull/imx_common.c" "CMakeFiles/gardens.elf.dir/board/mach-imx6ull/imx_common.c.o" "gcc" "CMakeFiles/gardens.elf.dir/board/mach-imx6ull/imx_common.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/boot/board_f.c" "CMakeFiles/gardens.elf.dir/boot/board_f.c.o" "gcc" "CMakeFiles/gardens.elf.dir/boot/board_f.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/boot/board_r.c" "CMakeFiles/gardens.elf.dir/boot/board_r.c.o" "gcc" "CMakeFiles/gardens.elf.dir/boot/board_r.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/boot/dynamic/implicit_call.c" "CMakeFiles/gardens.elf.dir/boot/dynamic/implicit_call.c.o" "gcc" "CMakeFiles/gardens.elf.dir/boot/dynamic/implicit_call.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/common/generic/api_string.c" "CMakeFiles/gardens.elf.dir/common/generic/api_string.c.o" "gcc" "CMakeFiles/gardens.elf.dir/common/generic/api_string.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/common/generic/errors.c" "CMakeFiles/gardens.elf.dir/common/generic/errors.c.o" "gcc" "CMakeFiles/gardens.elf.dir/common/generic/errors.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/common/generic/io_stream.c" "CMakeFiles/gardens.elf.dir/common/generic/io_stream.c.o" "gcc" "CMakeFiles/gardens.elf.dir/common/generic/io_stream.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/common/generic/time.c" "CMakeFiles/gardens.elf.dir/common/generic/time.c.o" "gcc" "CMakeFiles/gardens.elf.dir/common/generic/time.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/common/mempool/mem_malloc.c" "CMakeFiles/gardens.elf.dir/common/mempool/mem_malloc.c.o" "gcc" "CMakeFiles/gardens.elf.dir/common/mempool/mem_malloc.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/common/mempool/mem_simple.c" "CMakeFiles/gardens.elf.dir/common/mempool/mem_simple.c.o" "gcc" "CMakeFiles/gardens.elf.dir/common/mempool/mem_simple.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/drivers/clk/imx-clks.c" "CMakeFiles/gardens.elf.dir/drivers/clk/imx-clks.c.o" "gcc" "CMakeFiles/gardens.elf.dir/drivers/clk/imx-clks.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/drivers/gpio/led_template.c" "CMakeFiles/gardens.elf.dir/drivers/gpio/led_template.c.o" "gcc" "CMakeFiles/gardens.elf.dir/drivers/gpio/led_template.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/drivers/usb/gadget/fsl_mouse.c" "CMakeFiles/gardens.elf.dir/drivers/usb/gadget/fsl_mouse.c.o" "gcc" "CMakeFiles/gardens.elf.dir/drivers/usb/gadget/fsl_mouse.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/drivers/video/fbdev-imx.c" "CMakeFiles/gardens.elf.dir/drivers/video/fbdev-imx.c.o" "gcc" "CMakeFiles/gardens.elf.dir/drivers/video/fbdev-imx.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/example/display_app.c" "CMakeFiles/gardens.elf.dir/example/display_app.c.o" "gcc" "CMakeFiles/gardens.elf.dir/example/display_app.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/example/light_app.c" "CMakeFiles/gardens.elf.dir/example/light_app.c.o" "gcc" "CMakeFiles/gardens.elf.dir/example/light_app.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/example/thread_table.c" "CMakeFiles/gardens.elf.dir/example/thread_table.c.o" "gcc" "CMakeFiles/gardens.elf.dir/example/thread_table.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/init/main.c" "CMakeFiles/gardens.elf.dir/init/main.c.o" "gcc" "CMakeFiles/gardens.elf.dir/init/main.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/kernel/kthread/idle.c" "CMakeFiles/gardens.elf.dir/kernel/kthread/idle.c.o" "gcc" "CMakeFiles/gardens.elf.dir/kernel/kthread/idle.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/kernel/kthread/init_proc.c" "CMakeFiles/gardens.elf.dir/kernel/kthread/init_proc.c.o" "gcc" "CMakeFiles/gardens.elf.dir/kernel/kthread/init_proc.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/kernel/kthread/kthread.c" "CMakeFiles/gardens.elf.dir/kernel/kthread/kthread.c.o" "gcc" "CMakeFiles/gardens.elf.dir/kernel/kthread/kthread.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/kernel/mutex.c" "CMakeFiles/gardens.elf.dir/kernel/mutex.c.o" "gcc" "CMakeFiles/gardens.elf.dir/kernel/mutex.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/kernel/sched.c" "CMakeFiles/gardens.elf.dir/kernel/sched.c.o" "gcc" "CMakeFiles/gardens.elf.dir/kernel/sched.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/kernel/sleep.c" "CMakeFiles/gardens.elf.dir/kernel/sleep.c.o" "gcc" "CMakeFiles/gardens.elf.dir/kernel/sleep.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/kernel/spinlock.c" "CMakeFiles/gardens.elf.dir/kernel/spinlock.c.o" "gcc" "CMakeFiles/gardens.elf.dir/kernel/spinlock.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/kernel/thread.c" "CMakeFiles/gardens.elf.dir/kernel/thread.c.o" "gcc" "CMakeFiles/gardens.elf.dir/kernel/thread.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/bus_type/fwk_platdev.c" "CMakeFiles/gardens.elf.dir/platform/bus_type/fwk_platdev.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/bus_type/fwk_platdev.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/bus_type/fwk_platdrv.c" "CMakeFiles/gardens.elf.dir/platform/bus_type/fwk_platdrv.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/bus_type/fwk_platdrv.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/bus_type/fwk_platform.c" "CMakeFiles/gardens.elf.dir/platform/bus_type/fwk_platform.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/bus_type/fwk_platform.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/chardev/fwk_cdev.c" "CMakeFiles/gardens.elf.dir/platform/chardev/fwk_cdev.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/chardev/fwk_cdev.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/chardev/fwk_chrdev.c" "CMakeFiles/gardens.elf.dir/platform/chardev/fwk_chrdev.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/chardev/fwk_chrdev.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/framebuffer/fwk_ascii.c" "CMakeFiles/gardens.elf.dir/platform/framebuffer/fwk_ascii.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/framebuffer/fwk_ascii.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/framebuffer/fwk_bitmap.c" "CMakeFiles/gardens.elf.dir/platform/framebuffer/fwk_bitmap.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/framebuffer/fwk_bitmap.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/framebuffer/fwk_disp.c" "CMakeFiles/gardens.elf.dir/platform/framebuffer/fwk_disp.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/framebuffer/fwk_disp.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/framebuffer/fwk_fbmem.c" "CMakeFiles/gardens.elf.dir/platform/framebuffer/fwk_fbmem.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/framebuffer/fwk_fbmem.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/fwk_fcntl.c" "CMakeFiles/gardens.elf.dir/platform/fwk_fcntl.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/fwk_fcntl.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/fwk_fs.c" "CMakeFiles/gardens.elf.dir/platform/fwk_fs.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/fwk_fs.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/fwk_inode.c" "CMakeFiles/gardens.elf.dir/platform/fwk_inode.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/fwk_inode.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/fwk_kobj.c" "CMakeFiles/gardens.elf.dir/platform/fwk_kobj.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/fwk_kobj.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/fwk_uaccess.c" "CMakeFiles/gardens.elf.dir/platform/fwk_uaccess.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/fwk_uaccess.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/gpio_desc/fwk_gpiodesc.c" "CMakeFiles/gardens.elf.dir/platform/gpio_desc/fwk_gpiodesc.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/gpio_desc/fwk_gpiodesc.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/irq/fwk_irq.c" "CMakeFiles/gardens.elf.dir/platform/irq/fwk_irq.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/irq/fwk_irq.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/irq/fwk_irq_desc.c" "CMakeFiles/gardens.elf.dir/platform/irq/fwk_irq_desc.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/irq/fwk_irq_desc.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/irq/fwk_irq_domain.c" "CMakeFiles/gardens.elf.dir/platform/irq/fwk_irq_domain.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/irq/fwk_irq_domain.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/irq/fwk_irq_types.c" "CMakeFiles/gardens.elf.dir/platform/irq/fwk_irq_types.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/irq/fwk_irq_types.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/mem/fwk_mempool.c" "CMakeFiles/gardens.elf.dir/platform/mem/fwk_mempool.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/mem/fwk_mempool.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/mmc/fwk_sdcard.c" "CMakeFiles/gardens.elf.dir/platform/mmc/fwk_sdcard.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/mmc/fwk_sdcard.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/mmc/fwk_sdfatfs.c" "CMakeFiles/gardens.elf.dir/platform/mmc/fwk_sdfatfs.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/mmc/fwk_sdfatfs.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/net_device/fwk_netdev.c" "CMakeFiles/gardens.elf.dir/platform/net_device/fwk_netdev.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/net_device/fwk_netdev.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/of/fwk_of.c" "CMakeFiles/gardens.elf.dir/platform/of/fwk_of.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/of/fwk_of.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/of/fwk_of_device.c" "CMakeFiles/gardens.elf.dir/platform/of/fwk_of_device.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/of/fwk_of_device.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/of/fwk_of_prop.c" "CMakeFiles/gardens.elf.dir/platform/of/fwk_of_prop.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/of/fwk_of_prop.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/usb/fwk_usb.c" "CMakeFiles/gardens.elf.dir/platform/usb/fwk_usb.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/usb/fwk_usb.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/platform/usb/host/fwk_usb_ehci.c" "CMakeFiles/gardens.elf.dir/platform/usb/host/fwk_usb_ehci.c.o" "gcc" "CMakeFiles/gardens.elf.dir/platform/usb/host/fwk_usb_ehci.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/rootfs/fatfs/diskio.c" "CMakeFiles/gardens.elf.dir/rootfs/fatfs/diskio.c.o" "gcc" "CMakeFiles/gardens.elf.dir/rootfs/fatfs/diskio.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/rootfs/fatfs/ff.c" "CMakeFiles/gardens.elf.dir/rootfs/fatfs/ff.c.o" "gcc" "CMakeFiles/gardens.elf.dir/rootfs/fatfs/ff.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/Gardens/rootfs/fatfs/rfs_fatfs.c" "CMakeFiles/gardens.elf.dir/rootfs/fatfs/rfs_fatfs.c.o" "gcc" "CMakeFiles/gardens.elf.dir/rootfs/fatfs/rfs_fatfs.c.o.d"
  )

# Targets to which this target links.
set(CMAKE_TARGET_LINKED_INFO_FILES
  )

# Fortran module output directory.
set(CMAKE_Fortran_TARGET_MODULE_DIR "")
