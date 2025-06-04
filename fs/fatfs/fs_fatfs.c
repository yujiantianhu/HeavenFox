/*
 * General FatFs Interface
 *
 * File Name:   fs_fatfs.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.11.03
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#include <platform/fwk_mempool.h>
#include <platform/fwk_inode.h>
#include <platform/fwk_fcntl.h>
#include <platform/mmc/fwk_sdcard.h>
#include <fs/fs_intr.h>
#include <fs/fs_fatfs.h>

/*!< The defines */
#define FATFS_DISK_MAX_SS								_MAX_SS

#define FATFS_BLK_DEVICE_MAJOR                          10
#define FATFS_BLK_DEVICE_MINOR                          0

/*!< The globals */
static DECLARE_LIST_HEAD(sgtc_fatfs_drvList);

/*!< The functions */
static kint32_t fatfs_disk_mount(struct fwk_gendisk *sptr_gdisk);
static kint32_t fatfs_disk_unmount(struct fwk_gendisk *sptr_gdisk);
static kint32_t fatfs_disk_format(struct fwk_gendisk *sptr_gdisk);
static kint32_t fatfs_disk_mkdir(struct fwk_gendisk *sptr_gdisk, const kchar_t *dir_name);

/*!< API function */
/*!
 * @brief   mount fatfs
 * @param   sptr_gdisk
 * @retval  errno
 * @note    mount disk by fatfs
 */
static kint32_t fatfs_disk_mount(struct fwk_gendisk *sptr_gdisk)
{
    struct fatfs_disk *sptr_fdisk;
    struct fwk_kobject *sptr_kobj;
    kchar_t name[FATFS_DISK_PATH_LEN * 2] = {0};
    kuint8_t lenth;

    sptr_fdisk = mr_fatfs_disk_get(sptr_gdisk);

    /*!< Multiple mounts are not allowed */
    if (sptr_fdisk->is_mounted)
        return -ER_EXISTED;

    if (!sptr_fdisk->diskPath[0])
        return -ER_INVALID;

    /*!< FRESULT f_mount (FATFS* fs, const TCHAR* path, BYTE opt) */
    if (FR_OK != f_mount(&sptr_fdisk->sgtc_fatfs, (const TCHAR *)sptr_fdisk->diskPath, 1))
        return -ER_FAILD;

#if (_FS_RPATH >= 2U)
    if (FR_OK != f_chdrive((const TCHAR *)sptr_fdisk->diskPath))
        goto fail;
#endif

    print_info("mount fatfs disk \"%s\" successfully\r\n", sptr_fdisk->diskPath);

    lenth = sprintk(name, "/media/FAT32_%d/\0", sptr_fdisk->disk_number);
    sptr_kobj = fwk_kobject_populate(mr_nullptr, (const kchar_t *)name);
    if (!isValid(sptr_kobj))
    {
        if (PTR_ERR(sptr_kobj) != (-ER_FORBID))
        {
            print_err("populate disk path \"%s\" failed!\r\n", name);
            goto fail;
        }

        sptr_fdisk->path_lenth = 0;
    }
    else
    {
        fwk_inode_set_ops(sptr_kobj->sptr_inode, NR_TYPE_BLKDEV, 
                        MKE_DEV_NUM(sptr_gdisk->major, sptr_gdisk->first_minor));

        sptr_kobj->is_disk = true;
        sptr_fdisk->sptr_kobj = sptr_kobj;
        fwk_kobject_get(sptr_kobj);

        sptr_fdisk->path_lenth = lenth;
    }

    sptr_fdisk->is_mounted = true;
    return ER_NORMAL;

fail:
    fatfs_disk_unmount(sptr_gdisk);
    return -ER_FAILD;
}

/*!
 * @brief   unmount fatfs
 * @param   sptr_gdisk
 * @retval  errno
 * @note    unmount disk by fatfs
 */
static kint32_t fatfs_disk_unmount(struct fwk_gendisk *sptr_gdisk)
{
    struct fatfs_disk *sptr_fdisk;

    sptr_fdisk = mr_fatfs_disk_get(sptr_gdisk);
    if (!sptr_fdisk->is_mounted)
        return ER_NORMAL;

    /*!< FRESULT f_mount (FATFS* fs, const TCHAR* path, BYTE opt) */
    if (FR_OK != f_mount(mr_nullptr, sptr_fdisk->diskPath, 0))
    {
        print_err("unmount fatfs disk \"%s\" failed!\r\n", sptr_fdisk->diskPath);
        return -ER_FAILD;
    }

    if (sptr_fdisk->sptr_kobj)
        fwk_kset_kobject_remove(sptr_fdisk->sptr_kobj);

    sptr_fdisk->is_mounted = false;
    sptr_fdisk->path_lenth = 0;

    memset(&sptr_fdisk->sgtc_fatfs, 0, sizeof(sptr_fdisk->sgtc_fatfs));
    print_info("unmount fatfs disk \"%s\"\r\n", sptr_fdisk->diskPath);

    return ER_NORMAL;
}

/*!
 * @brief   format disk
 * @param   sptr_gdisk
 * @retval  errno
 * @note    format and clear disk
 */
static kint32_t fatfs_disk_format(struct fwk_gendisk *sptr_gdisk)
{
    struct fatfs_disk *sptr_fdisk;
    kuint8_t work[FATFS_DISK_MAX_SS];

    sptr_fdisk = mr_fatfs_disk_get(sptr_gdisk);
    if (sptr_fdisk->is_mounted)
        fatfs_disk_unmount(sptr_gdisk);

    /*!< FRESULT f_mkfs (const TCHAR* path, BYTE opt, DWORD au, void* work, UINT len) */
    if (FR_OK != f_mkfs(sptr_fdisk->diskPath, FM_FAT32, 0, (void *)work, sizeof(work)))
        return -ER_FAILD;

    return fatfs_disk_mount(sptr_gdisk);
}

/*!
 * @brief   create directory
 * @param   sptr_gdisk, dir_name (directory name)
 * @retval  errno
 * @note    mkdir
 */
static kint32_t fatfs_disk_mkdir(struct fwk_gendisk *sptr_gdisk, const kchar_t *dir_name)
{
    struct fatfs_disk *sptr_fdisk;
    FRESULT retval;

    sptr_fdisk = mr_fatfs_disk_get(sptr_gdisk);
    if (!sptr_fdisk->is_mounted)
        return -ER_NREADY;
    
    retval = f_mkdir(dir_name);
    if (retval && (retval != FR_EXIST))
        return -ER_FAILD;

    return ER_NORMAL;
}

/*!
 * @brief   delete directory
 * @param   sptr_gdisk, dir_name (directory name)
 * @retval  errno
 * @note    rmdir
 */
static kint32_t fatfs_disk_rmdir(struct fwk_gendisk *sptr_gdisk, const kchar_t *dir_name)
{
    struct fatfs_disk *sptr_fdisk;
    FRESULT retval;

    sptr_fdisk = mr_fatfs_disk_get(sptr_gdisk);
    if (!sptr_fdisk->is_mounted)
        return -ER_NREADY;
    
    retval = f_unlink(dir_name);
    if (retval != FR_OK)
        return -ER_FAILD;

    return ER_NORMAL;
}

/*!
 * @brief   reset sptr_list
 * @param   sptr_list
 * @retval  none
 * @note    none
 */
static void fatfs_dir_clean(struct fs_list *sptr_list)
{
    struct fs_item *sptr_item, *sptr_temp;

    if (!mr_list_empty(&sptr_list->sgtc_files))
    {
        foreach_list_next_entry_safe(sptr_item, sptr_temp, &sptr_list->sgtc_files, sgtc_link)
        {
            list_head_del(&sptr_item->sgtc_link);
            kfree(sptr_item);
        }
    }

    if (!mr_list_empty(&sptr_list->sgtc_dirs))
    {
        foreach_list_next_entry_safe(sptr_item, sptr_temp, &sptr_list->sgtc_dirs, sgtc_link)
        {
            list_head_del(&sptr_item->sgtc_link);
            kfree(sptr_item);
        }
    }

    sptr_list->dir_num = sptr_list->file_num = 0;
}

/*!
 * @brief   read directory
 * @param   sptr_list, pattern
 * @retval  errno
 * @note    none
 */
static kint32_t fatfs_dir_read(struct fs_list *sptr_list, const kchar_t *pattern)
{
    struct fs_item *sptr_item;
    DIR *sptr_dir;
    FILINFO sgtc_finfo, *sptr_finfo;
    FRESULT retval;

    sptr_dir = (DIR *)sptr_list->private_data;
    fatfs_dir_clean(sptr_list);

    for (;;)
    {
        retval = f_readdir(sptr_dir, &sgtc_finfo);
        if (retval != FR_OK)
            return -ER_FAILD;

        /*!< Read finished */
        if (!sgtc_finfo.fname[0])
            break;

        /*!< Jump over child dirs */
//      if (sgtc_finfo.fattrib & AM_DIR)
//          continue;

        /*!< Jump over hide files */
        if (sgtc_finfo.fname[0] == '.') 
            continue;

//      if (pattern && !pattern_match(sgtc_finfo.fname, pattern)) 
//          continue;

        /*!< Save current sgtc_finfo */
        sptr_item = kmalloc(sizeof(*sptr_item) + sizeof(sgtc_finfo), GFP_KERNEL);
        if (!isValid(sptr_item))
            return -ER_NOMEM;

        sptr_item->private_data = (void *)sptr_item + sizeof(*sptr_item);
        memcpy(sptr_item->private_data, &sgtc_finfo, sizeof(sgtc_finfo));

        sptr_finfo = (FILINFO *)sptr_item->private_data;
        sptr_item->name = sptr_finfo->fname;
        sptr_item->size = sptr_finfo->fsize;

        if (sgtc_finfo.fattrib & AM_DIR)
        {
            sptr_list->dir_num++;
            sptr_item->type = NR_FS_ITEM_DIR;
            list_head_add_tail(&sptr_list->sgtc_dirs, &sptr_item->sgtc_link);
        }
        else
        {
            sptr_list->file_num++;
            sptr_item->type = NR_FS_ITEM_FILE;
            list_head_add_tail(&sptr_list->sgtc_files, &sptr_item->sgtc_link);
        }
    }

    return (sptr_list->dir_num + sptr_list->file_num);
}

/*!
 * @brief   open directory
 * @param   sptr_blkdev, sptr_list
 * @retval  errno
 * @note    none
 */
static kint32_t fatfs_dir_open(struct fwk_gendisk *sptr_gdisk, struct fs_list *sptr_list)
{
    struct fatfs_disk *sptr_fdisk;
    DIR *sptr_dir;
    kchar_t *name;
    FRESULT retval;

    sptr_fdisk = mr_fatfs_disk_get(sptr_gdisk);
    if (!sptr_fdisk->is_mounted)
        return -ER_NREADY;

    name = kstrcut(sptr_list->path, sptr_fdisk->path_lenth);
    sptr_dir = kzalloc(sizeof(*sptr_dir), GFP_KERNEL);
    if (!isValid(sptr_dir))
        return -ER_NOMEM;

    retval = f_opendir(sptr_dir, name);
    if (retval != FR_OK)
    {
        kfree(sptr_dir);
        return retval;
    }

    sptr_list->private_data = sptr_dir;
    sptr_list->readdir = fatfs_dir_read;

    return ER_NORMAL;
}

/*!
 * @brief   close directory
 * @param   sptr_blkdev, sptr_list
 * @retval  errno
 * @note    none
 */
static kint32_t fatfs_dir_close(struct fwk_gendisk *sptr_gdisk, struct fs_list *sptr_list)
{
    DIR *sptr_dir;

    sptr_dir = (DIR *)sptr_list->private_data;
    if (!sptr_dir)
        return ER_NORMAL;

    if (FR_OK != f_closedir(sptr_dir))
        return -ER_FAILD;

    fatfs_dir_clean(sptr_list);
    kfree(sptr_dir);
    
    sptr_list->readdir = mr_nullptr;
    sptr_list->private_data = mr_nullptr;

    return ER_NORMAL;
}

/*!
 * @brief   open file in disk
 * @param   sptr_blkdev, sptr_file
 * @retval  errno
 * @note    open file and return file pointer
 */
static kint32_t fatfs_file_open(struct fwk_block_device *sptr_blkdev, struct fs_stream *sptr_file)
{
    struct fatfs_disk *sptr_fdisk;
    struct fwk_gendisk *sptr_gdisk;
    FIL *sptr_fp;
    kchar_t *name;
    kuint8_t mode = 0;

    sptr_gdisk = sptr_blkdev->sptr_gdisk;
    sptr_fdisk = mr_fatfs_disk_get(sptr_gdisk);

    name = kstrcut(sptr_file->full_name, sptr_fdisk->path_lenth);
    if ((*name == '\0') ||
        ((*name == '/' ) && (*(name + 1) == '\0')))
        return -ER_INVALID;
    
    /*!< struct FIL hope to satisfy x bytes alignment, it is not suggest to use "new" */
    sptr_fp = (FIL *)kzalloc(sizeof(FIL), GFP_KERNEL);
    if (!isValid(sptr_fp))
        return -ER_NOMEM;

    /*!<
     * FA_OPEN_ALWAYS(0x10) has a bug: if file has exsited, it will clear all content
     * FA_OPEN_APPEND(0x30) include FA_OPEN_APPEND, it's bug is as same to FA_OPEN_ALWAYS
     */
    mode |= (sptr_file->mode & O_CREAT ) ? FA_CREATE_NEW    : 0;
    mode |= (sptr_file->mode & O_WRONLY) ? FA_WRITE         : 0;
    mode |= (sptr_file->mode & O_RDONLY) ? FA_READ          : 0;
    mode |= (sptr_file->mode & O_APPEND) ? FA_OPEN_APPEND   : 0;
    mode |= (sptr_file->mode & O_EXCL  ) ? FA_OPEN_ALWAYS   : 0;
    if (!mode)
        mode = FA_OPEN_EXISTING;

    /*!< FRESULT f_open (FIL* fp, const TCHAR* path, BYTE mode); */
    if (f_open(sptr_fp, (const TCHAR *)name, mode))
        goto fail;

    sptr_file->private_data = sptr_fp;
    return ER_NORMAL;

fail:
    kfree(sptr_fp);
    return -ER_FAILD;
}

/*!
 * @brief   close file
 * @param   sptr_blkdev, sptr_file
 * @retval  errno
 * @note    close file and release file pointer
 */
static kint32_t fatfs_file_close(struct fwk_block_device *sptr_blkdev, struct fs_stream *sptr_file)
{
    FIL *sptr_fp;

    sptr_fp = (FIL *)sptr_file->private_data;
    if (!sptr_fp)
        return ER_NORMAL;

    /* FRESULT f_close (FIL* fp); */
    if (f_close(sptr_fp))
        return -ER_FAILD;

    kfree(sptr_fp);
    sptr_file->private_data = mr_nullptr;

    return ER_NORMAL;
}

/*!
 * @brief   locate file offset
 * @param   sptr_file, offset
 * @retval  errno
 * @note    set file pointer add offset
 */
static kint32_t fatfs_file_lssek(struct fs_stream *sptr_file, kuint32_t offset)
{
    FIL *sptr_fp;

    sptr_fp = (FIL *)sptr_file->private_data;
    if (!sptr_fp)
        return -ER_NODEV;

    /*!< FRESULT f_lseek (FIL* fp, FSIZE_t ofs); */
    if (f_lseek(sptr_fp, (FSIZE_t)offset))
        return -ER_FAILD;

    return ER_NORMAL;
}

/*!
 * @brief   write data to disk
 * @param   sptr_file, buffer, size, offset
 * @retval  errno
 * @note    write file
 */
static kssize_t fatfs_file_write(struct fs_stream *sptr_file, const void *buffer, kuint32_t size, kuint32_t offset)
{
    FIL *sptr_fp;
    kusize_t bytes = 0;

    sptr_fp = (FIL *)sptr_file->private_data;
    if (!sptr_fp)
        return -ER_NODEV;

    if (!(sptr_file->mode & O_WRONLY))
        return -ER_FORBID;

    if (offset)
        f_lseek(sptr_fp, (FSIZE_t)offset);

    /*!< FRESULT f_write (FIL* fp, const void* buff, UINT btw, UINT* bw); */
    if (f_write(sptr_fp, buffer, (UINT)size, (UINT *)&bytes))
        return -ER_FAILD;

    /*!< FRESULT f_sync (FIL* fp); */
    f_sync(sptr_fp);
    return bytes;
}

/*!
 * @brief   read data from disk
 * @param   sptr_file, buffer, size, offset
 * @retval  errno
 * @note    read file
 */
static kssize_t fatfs_file_read(struct fs_stream *sptr_file, void *buffer, kuint32_t size, kuint32_t offset)
{
    FIL *sptr_fp;
    kusize_t bytes = 0;

    sptr_fp = (FIL *)sptr_file->private_data;
    if (!sptr_fp)
        return -ER_NODEV;

    if (!(sptr_file->mode & O_RDONLY))
        return -ER_FORBID;

    if (offset)
        f_lseek(sptr_fp, (FSIZE_t)offset);

    /*!< FRESULT f_write (FIL* fp, const void* buff, UINT btw, UINT* bw); */
    if (f_read(sptr_fp, buffer, (UINT)size, (UINT *)&bytes))
        return -ER_FAILD;

    return bytes;
}

/*!
 * @brief   get file size
 * @param   sptr_file
 * @retval  size
 * @note    none
 */
static kssize_t fatfs_file_get_size(struct fs_stream *sptr_file)
{
    FIL *sptr_fp;

    sptr_fp = (FIL *)sptr_file->private_data;
    if (!sptr_fp)
        return -ER_NODEV;

    return f_size(sptr_fp);
}

/*!
 * @brief   get file read/write pointer
 * @param   sptr_file
 * @retval  size
 * @note    none
 */
static kssize_t fatfs_file_tell(struct fs_stream *sptr_file)
{
    FIL *sptr_fp;

    sptr_fp = (FIL *)sptr_file->private_data;
    if (!sptr_fp)
        return -ER_NODEV;

    return f_tell(sptr_fp);
}

/*!< fatfs operations */
static const struct fwk_block_device_oprts sgtc_fatfs_bops =
{  
    .open   = fatfs_file_open,
    .close  = fatfs_file_close,
    .write  = fatfs_file_write,
    .read   = fatfs_file_read,
    .lseek  = fatfs_file_lssek,
    .fsize  = fatfs_file_get_size,
    .fpos   = fatfs_file_tell,
};

/*!
 * @brief   fatfs create and initial
 * @param   number: disk type
 * @retval  errno
 * @note    register block device and gendisk
 */
struct fatfs_disk *fs_alloc_fatfs(kuint16_t number)
{
    struct fatfs_disk *sptr_fdisk;
    struct fwk_gendisk *sptr_gdisk;

    sptr_fdisk = (struct fatfs_disk *)kzalloc(sizeof(*sptr_fdisk), GFP_KERNEL);
    if (!isValid(sptr_fdisk))
        return ERR_PTR(-ER_NOMEM);

    sptr_gdisk = &sptr_fdisk->sgtc_gdisk;
    fwk_gendisk_init(sptr_gdisk, &sgtc_fatfs_bops);

    sprintk(sptr_fdisk->diskPath, "%d:/\0", number);
    sptr_fdisk->disk_number = number;

    sptr_gdisk->mount = fatfs_disk_mount;
    sptr_gdisk->unmount = fatfs_disk_unmount;
    sptr_gdisk->mkfs = fatfs_disk_format;
    sptr_gdisk->mkdir = fatfs_disk_mkdir;
    sptr_gdisk->rmdir = fatfs_disk_rmdir;
    sptr_gdisk->opendir = fatfs_dir_open;
    sptr_gdisk->closedir = fatfs_dir_close;

    return sptr_fdisk;
}

/*!
 * @brief   fatfs create and initial
 * @param   number: disk type
 * @retval  errno
 * @note    register block device and gendisk
 */
kint32_t fs_register_fatfs(struct fatfs_disk *sptr_fdisk)
{
    struct fwk_gendisk *sptr_gdisk;
    kint32_t retval;

    if (!isValid(sptr_fdisk))
        return -ER_NODEV;
    
    sptr_gdisk = &sptr_fdisk->sgtc_gdisk;

    sptr_gdisk->major = FATFS_BLK_DEVICE_MAJOR;
    sptr_gdisk->first_minor = sptr_fdisk->disk_number;
    sptr_gdisk->minors = 1;

    retval = fwk_add_gendisk(sptr_gdisk);
    if (retval)
    {
        print_err("add new gendisk mmc \"%s\" failed\r\n", sptr_fdisk->diskPath);
        return -ER_FAILD;
    }

    list_head_add_tail(&sgtc_fatfs_drvList, &sptr_fdisk->sgtc_link);

    return ER_NORMAL;
}

/*!
 * @brief   fatfs create and initial
 * @param   number: disk type
 * @retval  errno
 * @note    register block device and gendisk
 */
void fs_unregister_fatfs(struct fatfs_disk *sptr_fdisk)
{
    fwk_del_gendisk(&sptr_fdisk->sgtc_gdisk);
    list_head_del(&sptr_fdisk->sgtc_link);
}

/*!< ------------------------------------------------------------- */
/*!
 * @brief   fs_fatfs_init
 * @param   none
 * @retval  none
 * @note    fatfs init
 */
kint32_t __plat_init fs_fatfs_init(void)
{
    kint32_t retval;

    retval = fwk_register_blkdev(FATFS_BLK_DEVICE_MAJOR, "mmc");
    if (retval)
    {
        print_err("register mmc fatfs block device failed!\r\n");
        return retval;
    }

    print_info("register mmc fatfs block device successfully\r\n");
    return ER_NORMAL;
}

/*!
 * @brief   fs_fatfs_exit
 * @param   none
 * @retval  none
 * @note    fatfs exit
 */
void __plat_exit fs_fatfs_exit(void)
{
    struct fatfs_disk *sptr_fdisk, *sptr_temp;

    fwk_unregister_blkdev(FATFS_BLK_DEVICE_MAJOR, "mmc");
    print_info("unregistered mmc fatfs block device\r\n");

    foreach_list_next_entry_safe(sptr_fdisk, sptr_temp, &sgtc_fatfs_drvList, sgtc_link)
        fs_unregister_fatfs(sptr_fdisk);
}

IMPORT_ROOTFS_INIT(fs_fatfs_init);
IMPORT_ROOTFS_EXIT(fs_fatfs_exit);
