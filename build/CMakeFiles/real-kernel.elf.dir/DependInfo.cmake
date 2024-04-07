
# Consider dependencies only in project.
set(CMAKE_DEPENDS_IN_PROJECT_ONLY OFF)

# The set of languages for which implicit dependencies are needed:
set(CMAKE_DEPENDS_LANGUAGES
  "ASM"
  )
# The set of files for implicit dependencies of each language:
set(CMAKE_DEPENDS_CHECK_ASM
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/arch/arm/cpu/armv7/lowlevel_init.S" "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/build/CMakeFiles/real-kernel.elf.dir/arch/arm/cpu/armv7/lowlevel_init.S.o"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/arch/arm/cpu/armv7/start.S" "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/build/CMakeFiles/real-kernel.elf.dir/arch/arm/cpu/armv7/start.S.o"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/arch/arm/kernel/context.S" "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/build/CMakeFiles/real-kernel.elf.dir/arch/arm/kernel/context.S.o"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/arch/arm/kernel/head-common.S" "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/build/CMakeFiles/real-kernel.elf.dir/arch/arm/kernel/head-common.S.o"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/arch/arm/kernel/head.S" "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/build/CMakeFiles/real-kernel.elf.dir/arch/arm/kernel/head.S.o"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/arch/arm/lib/crt0.S" "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/build/CMakeFiles/real-kernel.elf.dir/arch/arm/lib/crt0.S.o"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/arch/arm/lib/lib1funcs.S" "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/build/CMakeFiles/real-kernel.elf.dir/arch/arm/lib/lib1funcs.S.o"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/arch/arm/lib/vectors.S" "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/build/CMakeFiles/real-kernel.elf.dir/arch/arm/lib/vectors.S.o"
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
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/arch/arm/cpu/armv7/imx6ull/board.c" "CMakeFiles/real-kernel.elf.dir/arch/arm/cpu/armv7/imx6ull/board.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/arch/arm/cpu/armv7/imx6ull/board.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/arch/arm/cpu/armv7/irq.c" "CMakeFiles/real-kernel.elf.dir/arch/arm/cpu/armv7/irq.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/arch/arm/cpu/armv7/irq.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/arch/arm/lib/exception.c" "CMakeFiles/real-kernel.elf.dir/arch/arm/lib/exception.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/arch/arm/lib/exception.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/arch/arm/lib/interrupt.c" "CMakeFiles/real-kernel.elf.dir/arch/arm/lib/interrupt.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/arch/arm/lib/interrupt.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/board/mach-imx6ull/board_init.c" "CMakeFiles/real-kernel.elf.dir/board/mach-imx6ull/board_init.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/board/mach-imx6ull/board_init.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/board/mach-imx6ull/imx6_console.c" "CMakeFiles/real-kernel.elf.dir/board/mach-imx6ull/imx6_console.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/board/mach-imx6ull/imx6_console.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/board/mach-imx6ull/imx6_gadget.c" "CMakeFiles/real-kernel.elf.dir/board/mach-imx6ull/imx6_gadget.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/board/mach-imx6ull/imx6_gadget.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/board/mach-imx6ull/imx6_led.c" "CMakeFiles/real-kernel.elf.dir/board/mach-imx6ull/imx6_led.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/board/mach-imx6ull/imx6_led.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/board/mach-imx6ull/imx6_sdmmc.c" "CMakeFiles/real-kernel.elf.dir/board/mach-imx6ull/imx6_sdmmc.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/board/mach-imx6ull/imx6_sdmmc.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/board/mach-imx6ull/imx6_systick.c" "CMakeFiles/real-kernel.elf.dir/board/mach-imx6ull/imx6_systick.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/board/mach-imx6ull/imx6_systick.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/board/mach-imx6ull/imx_common.c" "CMakeFiles/real-kernel.elf.dir/board/mach-imx6ull/imx_common.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/board/mach-imx6ull/imx_common.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/boot/board_f.c" "CMakeFiles/real-kernel.elf.dir/boot/board_f.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/boot/board_f.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/boot/board_r.c" "CMakeFiles/real-kernel.elf.dir/boot/board_r.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/boot/board_r.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/boot/dynamic/implicit_call.c" "CMakeFiles/real-kernel.elf.dir/boot/dynamic/implicit_call.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/boot/dynamic/implicit_call.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/common/generic/api_string.c" "CMakeFiles/real-kernel.elf.dir/common/generic/api_string.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/common/generic/api_string.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/common/generic/errors.c" "CMakeFiles/real-kernel.elf.dir/common/generic/errors.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/common/generic/errors.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/common/generic/io_stream.c" "CMakeFiles/real-kernel.elf.dir/common/generic/io_stream.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/common/generic/io_stream.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/common/generic/time.c" "CMakeFiles/real-kernel.elf.dir/common/generic/time.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/common/generic/time.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/common/mempool/mem_malloc.c" "CMakeFiles/real-kernel.elf.dir/common/mempool/mem_malloc.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/common/mempool/mem_malloc.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/common/mempool/mem_simple.c" "CMakeFiles/real-kernel.elf.dir/common/mempool/mem_simple.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/common/mempool/mem_simple.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/drivers/clk/imx-clks.c" "CMakeFiles/real-kernel.elf.dir/drivers/clk/imx-clks.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/drivers/clk/imx-clks.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/drivers/gpio/led_template.c" "CMakeFiles/real-kernel.elf.dir/drivers/gpio/led_template.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/drivers/gpio/led_template.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/drivers/usb/gadget/fsl_mouse.c" "CMakeFiles/real-kernel.elf.dir/drivers/usb/gadget/fsl_mouse.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/drivers/usb/gadget/fsl_mouse.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/drivers/video/fbdev-imx.c" "CMakeFiles/real-kernel.elf.dir/drivers/video/fbdev-imx.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/drivers/video/fbdev-imx.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/example/display_app.c" "CMakeFiles/real-kernel.elf.dir/example/display_app.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/example/display_app.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/example/light_app.c" "CMakeFiles/real-kernel.elf.dir/example/light_app.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/example/light_app.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/example/thread_table.c" "CMakeFiles/real-kernel.elf.dir/example/thread_table.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/example/thread_table.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/init/main.c" "CMakeFiles/real-kernel.elf.dir/init/main.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/init/main.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/kernel/kthread/idle.c" "CMakeFiles/real-kernel.elf.dir/kernel/kthread/idle.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/kernel/kthread/idle.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/kernel/kthread/init_proc.c" "CMakeFiles/real-kernel.elf.dir/kernel/kthread/init_proc.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/kernel/kthread/init_proc.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/kernel/kthread/kthread.c" "CMakeFiles/real-kernel.elf.dir/kernel/kthread/kthread.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/kernel/kthread/kthread.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/kernel/mutex.c" "CMakeFiles/real-kernel.elf.dir/kernel/mutex.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/kernel/mutex.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/kernel/sched.c" "CMakeFiles/real-kernel.elf.dir/kernel/sched.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/kernel/sched.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/kernel/sleep.c" "CMakeFiles/real-kernel.elf.dir/kernel/sleep.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/kernel/sleep.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/kernel/spinlock.c" "CMakeFiles/real-kernel.elf.dir/kernel/spinlock.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/kernel/spinlock.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/kernel/thread.c" "CMakeFiles/real-kernel.elf.dir/kernel/thread.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/kernel/thread.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/bus_type/hal_platdev.c" "CMakeFiles/real-kernel.elf.dir/platform/bus_type/hal_platdev.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/bus_type/hal_platdev.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/bus_type/hal_platdrv.c" "CMakeFiles/real-kernel.elf.dir/platform/bus_type/hal_platdrv.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/bus_type/hal_platdrv.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/bus_type/hal_platform.c" "CMakeFiles/real-kernel.elf.dir/platform/bus_type/hal_platform.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/bus_type/hal_platform.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/chardev/hal_cdev.c" "CMakeFiles/real-kernel.elf.dir/platform/chardev/hal_cdev.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/chardev/hal_cdev.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/chardev/hal_chrdev.c" "CMakeFiles/real-kernel.elf.dir/platform/chardev/hal_chrdev.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/chardev/hal_chrdev.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/framebuffer/hal_ascii.c" "CMakeFiles/real-kernel.elf.dir/platform/framebuffer/hal_ascii.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/framebuffer/hal_ascii.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/framebuffer/hal_bitmap.c" "CMakeFiles/real-kernel.elf.dir/platform/framebuffer/hal_bitmap.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/framebuffer/hal_bitmap.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/framebuffer/hal_fbmem.c" "CMakeFiles/real-kernel.elf.dir/platform/framebuffer/hal_fbmem.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/framebuffer/hal_fbmem.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/gpio_desc/hal_gpiodesc.c" "CMakeFiles/real-kernel.elf.dir/platform/gpio_desc/hal_gpiodesc.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/gpio_desc/hal_gpiodesc.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/hal_fcntl.c" "CMakeFiles/real-kernel.elf.dir/platform/hal_fcntl.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/hal_fcntl.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/hal_fs.c" "CMakeFiles/real-kernel.elf.dir/platform/hal_fs.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/hal_fs.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/hal_inode.c" "CMakeFiles/real-kernel.elf.dir/platform/hal_inode.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/hal_inode.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/hal_kobj.c" "CMakeFiles/real-kernel.elf.dir/platform/hal_kobj.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/hal_kobj.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/hal_uaccess.c" "CMakeFiles/real-kernel.elf.dir/platform/hal_uaccess.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/hal_uaccess.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/irq/hal_irq.c" "CMakeFiles/real-kernel.elf.dir/platform/irq/hal_irq.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/irq/hal_irq.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/irq/hal_irq_desc.c" "CMakeFiles/real-kernel.elf.dir/platform/irq/hal_irq_desc.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/irq/hal_irq_desc.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/irq/hal_irq_domain.c" "CMakeFiles/real-kernel.elf.dir/platform/irq/hal_irq_domain.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/irq/hal_irq_domain.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/irq/hal_irq_types.c" "CMakeFiles/real-kernel.elf.dir/platform/irq/hal_irq_types.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/irq/hal_irq_types.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/mem/kmem_pool.c" "CMakeFiles/real-kernel.elf.dir/platform/mem/kmem_pool.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/mem/kmem_pool.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/mmc/hal_sdcard.c" "CMakeFiles/real-kernel.elf.dir/platform/mmc/hal_sdcard.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/mmc/hal_sdcard.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/mmc/hal_sdfatfs.c" "CMakeFiles/real-kernel.elf.dir/platform/mmc/hal_sdfatfs.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/mmc/hal_sdfatfs.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/net_device/hal_netdev.c" "CMakeFiles/real-kernel.elf.dir/platform/net_device/hal_netdev.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/net_device/hal_netdev.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/of/hal_of.c" "CMakeFiles/real-kernel.elf.dir/platform/of/hal_of.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/of/hal_of.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/of/hal_of_device.c" "CMakeFiles/real-kernel.elf.dir/platform/of/hal_of_device.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/of/hal_of_device.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/of/hal_of_prop.c" "CMakeFiles/real-kernel.elf.dir/platform/of/hal_of_prop.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/of/hal_of_prop.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/usb/hal_usb.c" "CMakeFiles/real-kernel.elf.dir/platform/usb/hal_usb.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/usb/hal_usb.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/platform/usb/host/hal_usb_ehci.c" "CMakeFiles/real-kernel.elf.dir/platform/usb/host/hal_usb_ehci.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/platform/usb/host/hal_usb_ehci.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/rootfs/fatfs/diskio.c" "CMakeFiles/real-kernel.elf.dir/rootfs/fatfs/diskio.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/rootfs/fatfs/diskio.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/rootfs/fatfs/ff.c" "CMakeFiles/real-kernel.elf.dir/rootfs/fatfs/ff.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/rootfs/fatfs/ff.c.o.d"
  "/home/ubuntu/ProgramWorks/MyDesign/real-kernel/rootfs/fatfs/rfs_fatfs.c" "CMakeFiles/real-kernel.elf.dir/rootfs/fatfs/rfs_fatfs.c.o" "gcc" "CMakeFiles/real-kernel.elf.dir/rootfs/fatfs/rfs_fatfs.c.o.d"
  )

# Targets to which this target links.
set(CMAKE_TARGET_LINKED_INFO_FILES
  )

# Fortran module output directory.
set(CMAKE_Fortran_TARGET_MODULE_DIR "")
