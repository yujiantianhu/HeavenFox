/*
 * User Thread Instance (display task) Interface
 *
 * File Name:   display_task.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.01
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <common/io_stream.h>
#include <platform/fwk_fcntl.h>
#include <platform/video/fwk_fbmem.h>
#include <platform/video/fwk_font.h>
#include <platform/video/fwk_disp.h>
#include <platform/video/fwk_rgbmap.h>
#include <platform/video/fwk_bitmap.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/mutex.h>
#include <kernel/mailbox.h>
#include <fs/fs_intr.h>

#include "../task.h"
#include "test_task.h"

using namespace tsk;
using namespace bsc;

/*!< The defines */
#define DISPLAY_TASK_STACK_SIZE                      THREAD_STACK_PAGE(2)    /*!< 2 page (2kbytes) */

class crt_disp_task_t;
class crt_disp_base_t;
class crt_disp_bmp_t;
class crt_disp_text_t;

/*!< The functions */
static void display_task_clear(crt_disp_base_t &cgrt_this);
static void display_task_cursor(crt_disp_base_t &cgrt_this, 
                        kuint32_t x_start, kuint32_t y_start, kuint32_t x_end, kuint32_t y_end);
static kssize_t display_task_text(crt_disp_task_t &cgrt_dtsk, crt_disp_text_t &cgrt_text,
                        struct fs_stream *sptr_file);

/*!< The defines */
/*!< base class for display */
class crt_disp_base_t 
{
    friend void display_task_clear(crt_disp_base_t &cgrt_this);
    friend void display_task_cursor(crt_disp_base_t &cgrt_this, 
                        kuint32_t x_start, kuint32_t y_start, kuint32_t x_end, kuint32_t y_end);

public:
    crt_disp_base_t(crt_disp_task_t &cgrt_dtsk);
    ~crt_disp_base_t() {}

protected:
    struct fwk_disp_ctrl sgtc_dctrl;
};

/*!< bmp class */
class crt_disp_bmp_t : virtual public crt_disp_base_t
{
public:
    crt_disp_bmp_t(crt_disp_task_t &cgrt_dtsk) 
        : crt_disp_base_t(cgrt_dtsk)
        , bmp(mr_nullptr) 
    {}
    ~crt_disp_bmp_t() {}

    void set_src(const kchar_t *bmp)
    {
        this->bmp = bmp;
    }
    void show(crt_disp_task_t &cgrt_dtsk);

private:
    const kchar_t *bmp;
};

/*!< text class */
class crt_disp_text_t : virtual public crt_disp_base_t
{
    friend kssize_t display_task_text(crt_disp_task_t &cgrt_dtsk, crt_disp_text_t &cgrt_text,
                        struct fs_stream *sptr_file);
    
public:
    crt_disp_text_t(crt_disp_task_t &cgrt_dtsk, kuint32_t pages)
        : crt_disp_base_t(cgrt_dtsk)
        , text(mr_nullptr)
        , num(0)
        , cur_text(0)
        , pages(pages)
        , cur_page(0)
    {
        this->text_pages = new kuint32_t[pages];
        if (this->text_pages)
            memset(this->text_pages, 0, pages * sizeof(kuint32_t));
    }
    ~crt_disp_text_t() 
    {
        if (this->text_pages)
            delete[] this->text_pages;
    }

    void set_src(const kchar_t *path, const kchar_t **text, kuint32_t num)
    {
        this->path = path;
        this->text = text;
        this->num  = num;
    }

    void show(crt_disp_task_t &cgrt_dtsk);

private:
    const kchar_t *path;
    const kchar_t **text;
    kuint32_t num;
    kuint32_t cur_text;

    kuint32_t *text_pages;
    kuint32_t pages;
    kuint32_t cur_page;
};

/*!< major class for display task */
class crt_disp_task_t 
{
    friend crt_disp_base_t;
    friend crt_disp_bmp_t;
    friend crt_disp_text_t;

public:
    crt_disp_task_t(crt_task_t *cprt_task, const kchar_t *file, kuint32_t mode);
    ~crt_disp_task_t();

    crt_task_t *cprt_task;
    kint32_t fd;

private:
    struct fwk_fb_fix_screen_info sgtc_fix;
	struct fwk_fb_var_screen_info sgtc_var;
    kuint32_t *fb_buffer1;
    kuint32_t *fb_buffer2;

    struct fwk_disp_info sgtc_disp;
};

/*!< The globals */
static kchar_t g_display_buffer[(1920 * 1080) * 4 + 4] __align(4);
static const kchar_t *g_display_text_path = "/media/FAT32_2/home/fox/text";
static const kchar_t *g_display_ebook_list[] =
{
    "tangzhan.txt",
    "lianjian.txt",
    "feidao.txt",
    "tianlong.txt",
};
static const kchar_t *g_display_logo = "/media/FAT32_2/boot/logo/logo768.bmp";// CONFIG_POWER_LOGO;

/*!< API functions */
/*!
 * @brief	constructor of crt_disp_task_t
 * @param  	none
 * @retval 	none
 * @note   	none
 */
crt_disp_task_t::crt_disp_task_t(crt_task_t *cprt_task, const kchar_t *file, kuint32_t mode)
    : cprt_task(cprt_task)
    , fb_buffer1(mr_nullptr)
    , fb_buffer2(mr_nullptr)
{
    fd = virt_open(file, mode);
    if (fd < 0)
        return;

    virt_ioctl(fd, NR_FB_IOGET_VARINFO, &sgtc_var);
    virt_ioctl(fd, NR_FB_IOGET_FIXINFO, &sgtc_fix);

    fb_buffer1 = (kuint32_t *)virt_mmap(mr_nullptr, sgtc_fix.smem_len, 0, 0, fd, 0);
    if (!isValid(fb_buffer1))
        goto fail2;

    fb_buffer2 = (kuint32_t *)virt_mmap(mr_nullptr, sgtc_fix.smem_len, 0, 0, fd, sgtc_fix.smem_len);
    if (!isValid(fb_buffer2))
        goto fail3;

    fwk_display_ctrl_init(&sgtc_disp, 
                    fb_buffer1, 
                    fb_buffer2, 
                    sgtc_fix.smem_len, 
                    sgtc_var.xres, 
                    sgtc_var.yres, 
                    sgtc_var.bits_per_pixel);

    return;

    virt_munmap(fb_buffer2, sgtc_fix.smem_len);
fail3:
    virt_munmap(fb_buffer1, sgtc_fix.smem_len);
fail2:
    virt_close(fd);

    fd = -1;
    fb_buffer1 = fb_buffer2 = mr_nullptr;
}

/*!
 * @brief	destructor of crt_disp_task_t
 * @param  	none
 * @retval 	none
 * @note   	none
 */
crt_disp_task_t::~crt_disp_task_t()
{
    if (fd < 0)
        return;

    virt_munmap(fb_buffer2, sgtc_fix.smem_len);
    virt_munmap(fb_buffer1, sgtc_fix.smem_len);
    virt_close(fd);
}

/*!
 * @brief  initial display common settings
 * @param  none
 * @retval none
 * @note   do display
 */
static void display_task_settings_init(struct fwk_font_setting *sptr_set)
{
    sptr_set->color = RGB_BLACK;
    sptr_set->background = RGB_WHITE;
    sptr_set->font = NR_FWK_FONT_SONG;
    sptr_set->line_spacing = 8;
    sptr_set->word_spacing = 2;
    sptr_set->ptr_ascii = (void *)g_font_ascii_song16;
    sptr_set->ptr_hz = fwk_font_hz_song16_get()->base;
    sptr_set->size = FWK_FONT_16;

    sptr_set->left_spacing  = 16;
    sptr_set->right_spacing = 8;
    sptr_set->upper_spacing = 8;
    sptr_set->down_spacing  = 16;
}

/*!
 * @brief  clear full screen
 * @param  none
 * @retval none
 * @note   do display
 */
static void display_task_clear(crt_disp_base_t &cgrt_this)
{
    struct fwk_disp_ctrl &sgtc_dctrl = cgrt_this.sgtc_dctrl;
    fwk_display_clear(sgtc_dctrl.sptr_di, sgtc_dctrl.sgtc_set.background);
}

/*!
 * @brief  set cursor
 * @param  none
 * @retval none
 * @note   do display
 */
static void display_task_cursor(crt_disp_base_t &cgrt_this, 
                        kuint32_t x_start, kuint32_t y_start, kuint32_t x_end, kuint32_t y_end)
{
    struct fwk_disp_ctrl &sgtc_dctrl = cgrt_this.sgtc_dctrl;
    struct fwk_disp_info *sptr_disp = sgtc_dctrl.sptr_di;
    struct fwk_font_setting *sptr_set = &sgtc_dctrl.sgtc_set;

    fwk_display_set_cursor(&sgtc_dctrl, x_start, y_start, x_end, y_end);
    fwk_display_fill_rectangle(sptr_disp, x_start, y_start, 
                                        x_end, y_end, sptr_set->background);
}

/*!
 * @brief	constructor of crt_disp_base_t
 * @param  	none
 * @retval 	none
 * @note   	none
 */
crt_disp_base_t::crt_disp_base_t(crt_disp_task_t &cgrt_dtsk)
{
    struct fwk_disp_ctrl &sgtc_dctrl = this->sgtc_dctrl;
    struct fwk_font_setting &sgtc_set = this->sgtc_dctrl.sgtc_set;

    sgtc_dctrl.sptr_di = &cgrt_dtsk.sgtc_disp;
    if (cgrt_dtsk.fd < 0)
        return;

    display_task_settings_init(&sgtc_set);
}

/*!
 * @brief  display logo
 * @param  none
 * @retval none
 * @note   do display
 */
void crt_disp_bmp_t::show(crt_disp_task_t &cgrt_dtsk)
{
    struct fwk_disp_ctrl &sgtc_dctrl = this->sgtc_dctrl;
    struct fwk_disp_info *sptr_disp = sgtc_dctrl.sptr_di;
    struct fs_stream *sptr_file;
    struct fwk_bmp_ctrl sgtc_bctl;
    struct fwk_fb_var_screen_info sgtc_var;
    kuint8_t bytes_per_pixel;
    kssize_t size;

    if (!this->bmp)
        return;

    fwk_display_frame_exchange(sptr_disp);
    display_task_clear(*this);
    display_task_cursor(*this, 0, 0, sptr_disp->width, sptr_disp->height);

    sptr_file = file_open(this->bmp, O_RDONLY);
    if (!isValid(sptr_file))
        return;

    bytes_per_pixel = sptr_disp->bpp >> 3;
    size = file_read(sptr_file, g_display_buffer, 
                sptr_disp->width * sptr_disp->height * bytes_per_pixel);
    if (size <= 0)
        goto END;

    fwk_bitmap_ctrl_init(&sgtc_bctl, sptr_disp, 0, 0);
    fwk_display_whole_bitmap(&sgtc_bctl, (const kuint8_t *)g_display_buffer);

    virt_ioctl(cgrt_dtsk.fd, NR_FB_IOGET_VARINFO, &sgtc_var);
    if (!sgtc_var.yoffset)
        sgtc_var.yoffset += sgtc_var.yres;
    else
        sgtc_var.yoffset = 0;
    
    virt_ioctl(cgrt_dtsk.fd, NR_FB_IOSET_VARINFO, &sgtc_var);

END:
    file_close(sptr_file);
}

/*!
 * @brief  display text
 * @param  none
 * @retval none
 * @note   do display
 */
static kssize_t display_task_text(crt_disp_task_t &cgrt_dtsk, crt_disp_text_t &cgrt_text,
                        struct fs_stream *sptr_file)
{
    struct mailbox &sgtc_mb = cgrt_dtsk.cprt_task->get_mailbox();
    struct fwk_disp_ctrl &sgtc_dctrl = cgrt_text.sgtc_dctrl;
    struct fwk_disp_info *sptr_disp = sgtc_dctrl.sptr_di;
    struct mail *sptr_mail;
    struct fwk_fb_var_screen_info sgtc_var;
    kuint8_t op = 0;
    kssize_t size, offset = 0;
    kusize_t page_index = 0;

    fwk_display_frame_exchange(sptr_disp);
    display_task_clear(cgrt_text);
    display_task_cursor(cgrt_text, 0, 0, sptr_disp->width, sptr_disp->height);

    page_index = cgrt_text.cur_page;
    offset = cgrt_text.text_pages[page_index];
    while (!IS_DISP_FRAME_FULL(&sgtc_dctrl))
    {
        file_lseek(sptr_file, offset);
        size = file_read(sptr_file, g_display_buffer, sizeof(g_display_buffer) - 4);
        if (size <= 0)
            return 0;

        g_display_buffer[size] = '\0';
        offset += fwk_display_word(&sgtc_dctrl, g_display_buffer);
    }

    virt_ioctl(cgrt_dtsk.fd, NR_FB_IOGET_VARINFO, &sgtc_var);
    if (!sgtc_var.yoffset)
        sgtc_var.yoffset += sgtc_var.yres;
    else
        sgtc_var.yoffset = 0;
    
    virt_ioctl(cgrt_dtsk.fd, NR_FB_IOSET_VARINFO, &sgtc_var);

    do {
        sptr_mail = mail_recv(&sgtc_mb, 0);
        if (!isValid(sptr_mail))
        {
            msleep(200);
            continue;
        }

        op = 1;
        if (sptr_mail->sptr_msg->type == NR_MAIL_TYPE_SERIAL)
        {
            kchar_t *buffer = (kchar_t *)sptr_mail->sptr_msg[0].buffer;

            if (!kstrncmp(buffer, "up", 2))
                page_index = page_index ? (page_index - 1) : 0;
            else if (!kstrncmp(buffer, "down", 4)) 
            {
                if (page_index < cgrt_text.pages)
                    page_index++;
                cgrt_text.text_pages[page_index] = offset;
            }
            else
                op = 0;
        }

        mail_recv_finish(sptr_mail);

    } while (!op);

    cgrt_text.cur_page = page_index;
    return size;
}

/*!
 * @brief  show text
 * @param  none
 * @retval none
 * @note   do display
 */
void crt_disp_text_t::show(crt_disp_task_t &cgrt_dtsk)
{
    struct fs_stream *sptr_file;
    kchar_t full_path[128];

    if (!this->text_pages)
        return;

    sprintk(full_path, "%s/%s", this->path, this->text[this->cur_text]);
    sptr_file = file_open(full_path, O_RDONLY);
    if (!isValid(sptr_file))
        goto END;

    this->cur_page = 0;
    memset_ex(this->text_pages, 0, this->pages);

    while (display_task_text(cgrt_dtsk, *this, sptr_file) > 0);
    file_close(sptr_file);

END:
    if ((this->cur_text++) >= this->num)
        this->cur_text = 0;
}

/*!
 * @brief  display task
 * @param  none
 * @retval none
 * @note   do display
 */
static void *display_task_entry(void *args)
{
    crt_task_t *cprt_this = (crt_task_t *)args;
    crt_disp_task_t cgrt_dtsk(cprt_this, "/dev/fb0", O_RDWR);
    crt_disp_bmp_t cgrt_logo(cgrt_dtsk);
    crt_disp_text_t cgrt_txt(cgrt_dtsk, 1024);

    if (cgrt_dtsk.fd < 0)
        goto fail;

    cgrt_logo.set_src(g_display_logo);
    cgrt_txt.set_src(g_display_text_path, g_display_ebook_list, ARRAY_SIZE(g_display_ebook_list));

    mr_preempt_disable();
    cgrt_logo.show(cgrt_dtsk);
    mr_preempt_enable();
    sleep(5);

    for (;;)
    {
        cgrt_txt.show(cgrt_dtsk);
        msleep(200);
    }

fail:
    schedule_self_suspend();

    return args;
}

/*!
 * @brief	create display app task
 * @param  	none
 * @retval 	error code
 * @note   	none
 */
kint32_t display_task_init(void)
{
    static kuint8_t g_display_task_stack[DISPLAY_TASK_STACK_SIZE];

    crt_task_t *cprt_task = new crt_task_t("display_task", 
                                            display_task_entry, 
                                            g_display_task_stack, 
                                            sizeof(g_display_task_stack),
                                            THREAD_PROTY_DEFAULT,
                                            100);
    if (!cprt_task)
        return -ER_FAILD;

    struct mailbox &sgtc_mb = cprt_task->get_mailbox();
    mailbox_init(&sgtc_mb, cprt_task->get_self(), "display-task-mailbox");

    return ER_NORMAL;
}

/*!< end of file */
