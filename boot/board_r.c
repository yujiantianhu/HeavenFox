/*
 * IMX6ULL Board General Initial
 *
 * File Name:   board_r.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.10
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes*/
#include <boot/board_init.h>
#include <platform/base/fwk_fcntl.h>
#include <fs/fs_fatfs.h>
#include <fs/fs_intr.h>

/*!< The defines */
struct boot_images
{
    kuaddr_t os_entry;

    struct global_data *sptr_gd;
    struct tag_params *sptr_params;
    struct m_area sgtc_fdt;

    struct fatfs_disk *sptr_fdisk;
};

struct boot_mode_stack
{
    kuaddr_t stack_base;
    kusize_t stack_size;
    kusize_t percpu_stack_size;
};

/*!< The globals */
struct boot_images sgtc_boot_images;

/*!< API function */
/*!
 * @brief   system_boot_initial
 * @param   none
 * @retval  none
 * @note    initial heap and stack
 */
kint32_t system_boot_initial(void)
{
#if CONFIG_STACK_WITH_LDS
    struct boot_mode_stack sgtc_svc = { SVC_MODE_STACK_BASE, SVC_MODE_STACK_SIZE, SVC_MODE_STACK_SIZE / CONFIG_CORE_NUM };
    struct boot_mode_stack sgtc_sys = { SYS_MODE_STACK_BASE, SYS_MODE_STACK_SIZE, SYS_MODE_STACK_SIZE / CONFIG_CORE_NUM };
    struct boot_mode_stack sgtc_fiq = { FIQ_MODE_STACK_BASE, FIQ_MODE_STACK_SIZE, FIQ_MODE_STACK_SIZE / CONFIG_CORE_NUM };
    struct boot_mode_stack sgtc_irq = { IRQ_MODE_STACK_BASE, IRQ_MODE_STACK_SIZE, IRQ_MODE_STACK_SIZE / CONFIG_CORE_NUM };
    struct boot_mode_stack sgtc_abt = { ABT_MODE_STACK_BASE, ABT_MODE_STACK_SIZE, ABT_MODE_STACK_SIZE / CONFIG_CORE_NUM };
    struct boot_mode_stack sgtc_und = { UND_MODE_STACK_BASE, UND_MODE_STACK_SIZE, UND_MODE_STACK_SIZE / CONFIG_CORE_NUM };

#else
    struct boot_mode_stack sgtc_svc = { CONFIG_SVC_STACK_BASE, CONFIG_SVC_STACK_SIZE, CONFIG_SVC_STACK_SIZE / CONFIG_CORE_NUM };
    struct boot_mode_stack sgtc_sys = { CONFIG_SYS_STACK_BASE, CONFIG_SYS_STACK_SIZE, CONFIG_SYS_STACK_SIZE / CONFIG_CORE_NUM };
    struct boot_mode_stack sgtc_fiq = { CONFIG_FIQ_STACK_BASE, CONFIG_FIQ_STACK_SIZE, CONFIG_FIQ_STACK_SIZE / CONFIG_CORE_NUM };
    struct boot_mode_stack sgtc_irq = { CONFIG_IRQ_STACK_BASE, CONFIG_IRQ_STACK_SIZE, CONFIG_IRQ_STACK_SIZE / CONFIG_CORE_NUM };
    struct boot_mode_stack sgtc_abt = { CONFIG_ABT_STACK_BASE, CONFIG_ABT_STACK_SIZE, CONFIG_ABT_STACK_SIZE / CONFIG_CORE_NUM };
    struct boot_mode_stack sgtc_und = { CONFIG_UND_STACK_BASE, CONFIG_UND_STACK_SIZE, CONFIG_UND_STACK_SIZE / CONFIG_CORE_NUM };
#endif

    _SVC_MODE_STACK_BASE = sgtc_svc.stack_base;
    _SYS_MODE_STACK_BASE = sgtc_sys.stack_base;
    _FIQ_MODE_STACK_BASE = sgtc_fiq.stack_base;
    _IRQ_MODE_STACK_BASE = sgtc_irq.stack_base;
    _ABT_MODE_STACK_BASE = sgtc_abt.stack_base;
    _UND_MODE_STACK_BASE = sgtc_und.stack_base;

    for (kint32_t cpuid = 0; cpuid < CONFIG_CORE_NUM; cpuid++)
    {
        set_svc_mode_stack(cpuid, sgtc_svc.stack_base - (cpuid * sgtc_svc.percpu_stack_size));
        set_sys_mode_stack(cpuid, sgtc_sys.stack_base - (cpuid * sgtc_sys.percpu_stack_size));
        set_fiq_mode_stack(cpuid, sgtc_fiq.stack_base - (cpuid * sgtc_fiq.percpu_stack_size));
        set_irq_mode_stack(cpuid, sgtc_irq.stack_base - (cpuid * sgtc_irq.percpu_stack_size));
        set_abt_mode_stack(cpuid, sgtc_abt.stack_base - (cpuid * sgtc_abt.percpu_stack_size));
        set_und_mode_stack(cpuid, sgtc_und.stack_base - (cpuid * sgtc_und.percpu_stack_size));
    }

    boot_text_print();
    return RET_BOOT_PASS;
}

/*!
 * @brief   fdt_boot_initial
 * @param   none
 * @retval  none
 * @note    copy dtb to ram
 */
kint32_t fix_data_mempool_initial(void)
{
    kint32_t retval;

    retval = memory_block_self_defines(GFP_FIXED, 
                                CONFIG_B2K_FIXDATA_BASE, CONFIG_B2K_FIXDATA_SIZE);
    return retval ? RET_BOOT_PASS : RET_BOOT_ERR;
}

/*!
 * @brief   extdisk_mount
 * @param   none
 * @retval  none
 * @note    mount disk
 */
kint32_t extdisk_mount(struct boot_images *sptr_image)
{
    struct fatfs_disk *sptr_fdisk;
    struct fwk_gendisk *sptr_gdisk;
    kint32_t retval;

    sptr_fdisk = fs_alloc_fatfs(SDDISK);
    if (!isValid(sptr_fdisk))
        return RET_BOOT_ERR;

    sptr_gdisk = &sptr_fdisk->sgtc_gdisk;
    retval = sptr_gdisk->mount(sptr_gdisk);
    if (retval)
        return RET_BOOT_ERR;

    sptr_image->sptr_fdisk = sptr_fdisk;
    return RET_BOOT_PASS;
}

/*!
 * @brief   extdisk_mount
 * @param   none
 * @retval  none
 * @note    mount disk
 */
void extdisk_unmount(struct boot_images *sptr_image)
{
    struct fatfs_disk *sptr_fdisk;
    struct fwk_gendisk *sptr_gdisk;

    sptr_fdisk = sptr_image->sptr_fdisk;
    if (!isValid(sptr_fdisk))
        return;

    sptr_gdisk = &sptr_fdisk->sgtc_gdisk;
    sptr_gdisk->unmount(sptr_gdisk);
    kfree(sptr_fdisk);

    sptr_image->sptr_fdisk = mr_nullptr;
}

/*!
 * @brief   prepare video params
 * @param   sptr_image, sptr_params
 * @retval  none
 * @note    fill sptr_params
 */
kint32_t tag_params_video(struct boot_images *sptr_image, struct tag_params *sptr_params)
{
    struct fatfs_disk *sptr_fdisk;
    struct fwk_gendisk *sptr_gdisk;
    struct fs_stream sgtc_file;
    struct fwk_block_device sgtc_blkdev;
    struct video_params *sptr_vdp;
    void *fontlib;
    kssize_t size;

    sptr_fdisk = sptr_image->sptr_fdisk;
    if (!isValid(sptr_fdisk))
        return RET_BOOT_ERR;

    sptr_gdisk = &sptr_fdisk->sgtc_gdisk;
    sgtc_blkdev.sptr_gdisk = sptr_gdisk;

    sptr_vdp = &sptr_params->u.sgtc_vdp;
    sptr_params->sgtc_hdr.size = sizeof(*sptr_vdp);
    sptr_params->sgtc_hdr.type = TAG_PARAM_VIDEO;

    sgtc_file.full_name = "/lib/font/HZK16";
    sgtc_file.mode = O_RDONLY;
    if (!sptr_gdisk->sptr_bops->open(&sgtc_blkdev, &sgtc_file))
    {
        size = sptr_gdisk->sptr_bops->fsize(&sgtc_file);
        if (size <= 0)
            goto END;

        fontlib = kmalloc(mr_align(size, 8), GFP_FIXED);
        if (!isValid(fontlib))
            goto END;
        
        size = sptr_gdisk->sptr_bops->read(&sgtc_file, fontlib, size, 0);
        if (size > 0)
        {
            sptr_vdp->sgtc_hz16x16.base = fontlib;
            sptr_vdp->sgtc_hz16x16.size = size;
            sptr_vdp->sgtc_hz16x16.offset = 0;
        }
    }

END:
    sptr_gdisk->sptr_bops->close(&sgtc_blkdev, &sgtc_file);
    return RET_BOOT_PASS;
}

/*!
 * @brief   prepare params for kernel
 * @param   sptr_image
 * @retval  none
 * @note    fill sptr_params
 */
kint32_t boot_params_prep(struct boot_images *sptr_image)
{
    struct global_data *sptr_gd;
    struct tag_params *sptr_params;
    kint32_t retval;

    sptr_gd = sptr_image->sptr_gd;
    sptr_image->os_entry = sptr_gd->os_entry;

    sptr_params = (struct tag_params *)CONFIG_B2K_PARAM_BASE;
    sptr_image->sptr_params = sptr_params;
    
    sptr_params->sgtc_hdr.size = 0;
    sptr_params->sgtc_hdr.type = 0;

    /*!< ---------------------------------------------------------*/
    sptr_params = TAG_PARAM_NEXT(sptr_params);
    retval = tag_params_video(sptr_image, sptr_params);
    if (retval)
        goto fail;

    /*!< ---------------------------------------------------------*/
    sptr_params = TAG_PARAM_NEXT(sptr_params);
    sptr_params->sgtc_hdr.size = sizeof(sptr_params->u.sgtc_fdt);
    sptr_params->sgtc_hdr.type = TAG_PARAM_FDT;
    memcpy(&sptr_params->u.sgtc_fdt.sgtc_fdt, &sptr_image->sgtc_fdt, sizeof(struct m_area));

    /*!< ---------------------------------------------------------*/
    sptr_params = TAG_PARAM_NEXT(sptr_params);
    sptr_params->sgtc_hdr.size = 0;
    sptr_params->sgtc_hdr.type = -1;

    return RET_BOOT_PASS;

fail:
    return RET_BOOT_ERR;
}

/*!
 * @brief   fdt_boot_initial
 * @param   none
 * @retval  none
 * @note    copy dtb to ram
 */
kint32_t fdt_boot_initial(struct boot_images *sptr_image)
{
    struct fs_stream sgtc_file;
    struct fatfs_disk *sptr_fdisk;
    struct fwk_gendisk *sptr_gdisk;
    struct fwk_block_device sgtc_blkdev;
    kssize_t size;
    kint32_t retval;

    if ((PROGRAM_RAM_START <= (CONFIG_DEVICE_TREE_BASE + CONFIG_FDT_MAX_SIZE - 1)))
        return RET_BOOT_ERR;

    sptr_fdisk = sptr_image->sptr_fdisk;
    if (!isValid(sptr_fdisk))
        return RET_BOOT_ERR;

    sptr_gdisk = &sptr_fdisk->sgtc_gdisk;
    sgtc_blkdev.sptr_gdisk = sptr_gdisk;
    sgtc_file.full_name = "/boot/firmware.dtb";
    sgtc_file.mode = O_RDONLY;
    retval = sptr_gdisk->sptr_bops->open(&sgtc_blkdev, &sgtc_file);
    if (retval)
        return RET_BOOT_ERR;

    size = sptr_gdisk->sptr_bops->read(&sgtc_file, 
                                    (void *)CONFIG_DEVICE_TREE_BASE, CONFIG_FDT_MAX_SIZE, 0);
    sptr_gdisk->sptr_bops->close(&sgtc_blkdev, &sgtc_file);

    if (size <= 0)
    {
        print_err("can not read device-tree file!\r\n");
        return RET_BOOT_ERR;
    }

    sptr_image->sgtc_fdt.base = (void *)CONFIG_DEVICE_TREE_BASE;
    sptr_image->sgtc_fdt.size = size;

    return RET_BOOT_PASS;
}

/*!
 * @brief   jump_to_kernel
 * @param   gd
 * @retval  none
 * @note    jump to assembly entry: kernel_entry
 */
void jump_to_kernel(struct boot_images *sptr_image)
{
    kuint32_t r2;
    void (*kernel_entry)(kint32_t zero, kint32_t arch, kuint32_t params);

    r2 = (kuint32_t)sptr_image->sptr_params;
    kernel_entry = (void (*)(kint32_t, kint32_t, kuint32_t))sptr_image->os_entry;

    /*!< jump to head.S */
    kernel_entry(0, 0, r2);
}

/*!
 * @brief   boot_main_loop
 * @param   none
 * @retval  none
 * @note    initial heap and stack
 */
kint32_t boot_main_loop(void)
{
    struct boot_images *sptr_image;
    kint32_t retval;

    sptr_image = &sgtc_boot_images;
    if (extdisk_mount(sptr_image))
        return RET_BOOT_ERR;

    for (;;)
    {
        break;
    }

    /*!< device-tree prepare */
    retval = fdt_boot_initial(sptr_image);
    if (retval)
        goto exit;

    retval = boot_params_prep(sptr_image);
    if (retval)
        goto exit;

exit:
    extdisk_unmount(sptr_image);

    /*!< Destroy Memory Pool of Bootloader */
    memory_block_self_destroy(-1);

    return retval ? RET_BOOT_ERR : RET_BOOT_PASS;
}

/*!< board init queue lately */
board_init_t board_init_sequence_r[] =
{
    /*!< boot initial */
    system_boot_initial,

    /*!< fix memory initial */
    fix_data_mempool_initial,

    /*!< peripheral initial*/
//	board_init_light,

    /*!< boot main loop */
    boot_main_loop,

    mr_nullptr,
};

/*!
 * @brief   board_init_r
 * @param   none
 * @retval  none
 * @note    board late initial
 */
void board_init_r(void)
{
    struct boot_images *sptr_image;
    srt_gd_t *sptr_gd;

    sptr_gd = board_get_gd();

    /*!< save kernel entry address */
    __asm__ __volatile__ (
        " mov %0, r10 \n"
        : "=&r"(sptr_gd->os_entry)
    );

    sptr_image = &sgtc_boot_images;
    sptr_image->sptr_gd = sptr_gd;

    /*!< initial */
    if (board_initcall_run_list(board_init_sequence_r))
        mr_assert(false);

    /*!< if initialize finished, start kernel */
    jump_to_kernel(sptr_image);
}

/* end of file */
