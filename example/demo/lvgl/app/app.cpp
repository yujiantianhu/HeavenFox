/*
 * User Thread Instance (button task) Interface
 *
 * File Name:   button_app.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.25
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <platform/video/fwk_rgbmap.h>

#include "app.h"

/*!< The functions */
static void lvgl_time_event(lv_timer_t *sptr_timer);

/*!< The class */
class crt_main_ui_t
{
    friend void lvgl_time_event(lv_timer_t *sptr_timer);

public:
    crt_main_ui_t() {}
    ~crt_main_ui_t() {}

    void setup_desktop(void);
    void setup_status_bar(void);
    void setup_icon(void);

private:
    lv_obj_t *sptr_desktop;

    lv_style_t sgtc_barstyle;
    lv_obj_t *sptr_statusbar;

    lv_style_t sgtc_iconstyle;
    lv_obj_t *sptr_tagicon;
    lv_obj_t *sptr_timicon;
    lv_obj_t *sptr_pwricon;
    lv_obj_t *sptr_bellicon;
    lv_obj_t *sptr_wifiicon;
    lv_obj_t *sptr_bleicon;
    lv_obj_t *sptr_gpsicon;

    lv_timer_t *sptr_timer;
};

/*!< API functions */
void crt_main_ui_t::setup_desktop(void)
{
    lv_obj_t *sptr_obj;

    sptr_obj = lv_obj_create(lv_scr_act());
    if (!sptr_obj)
        return;

    lv_obj_set_pos(sptr_obj, 0, 0);
    lv_obj_set_size(sptr_obj, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(sptr_obj, lv_color_hex(RGB_BLACK), 0);
    lv_obj_set_style_pad_all(sptr_obj, 0, 0);
    lv_obj_set_style_radius(sptr_obj, 0, 0);
    lv_obj_set_style_bg_img_src(sptr_obj, CONFIG_WALL_PAPER, 0);
    lv_obj_set_scrollbar_mode(sptr_obj, LV_SCROLLBAR_MODE_OFF); 
    lv_obj_set_style_border_width(sptr_obj, 0, 0);
    lv_obj_center(sptr_obj);

    this->sptr_desktop = sptr_obj;
}

void crt_main_ui_t::setup_status_bar(void)
{
    lv_obj_t *sptr_obj;
    lv_style_t *sptr_style = &this->sgtc_barstyle;

    sptr_obj = lv_obj_create(this->sptr_desktop);
    if (!sptr_obj)
        return;

    lv_style_init(sptr_style);
    lv_style_set_bg_color(sptr_style, lv_color_hex(RGB_BLACK));
    lv_style_set_bg_img_opa(sptr_style, 0);
    lv_style_set_border_width(sptr_style, 0);

    lv_obj_set_width(sptr_obj, lv_pct(100));
    lv_obj_set_height(sptr_obj, 32);
    lv_obj_add_style(sptr_obj, sptr_style, 0);
    lv_obj_set_align(sptr_obj, LV_ALIGN_TOP_MID);

    this->sptr_statusbar = sptr_obj;
}

void crt_main_ui_t::setup_icon(void)
{
    lv_obj_t *sptr_obj;
    lv_obj_t *sptr_neibh = this->sptr_statusbar;
    lv_style_t *sptr_style = &this->sgtc_iconstyle;

    lv_style_init(sptr_style);
    lv_style_set_text_color(sptr_style, lv_color_hex(RGB_WHITE));
    lv_style_set_text_font(sptr_style, &lv_font_montserrat_16);

    /*!< heavenfox text */
    sptr_obj = lv_label_create(this->sptr_statusbar);
    if (sptr_obj) {
        lv_label_set_text(sptr_obj, "heavenfox");
        lv_obj_add_style(sptr_obj, sptr_style, 0);
        lv_obj_align_to(sptr_obj, this->sptr_statusbar, LV_ALIGN_LEFT_MID, 0, 0);

        this->sptr_tagicon = sptr_obj;
    }

    /*!< time text */
    sptr_obj = lv_label_create(this->sptr_statusbar);
    if (sptr_obj) {
        lv_timer_t *sptr_timer;

//      struct time_clock *sptr_tclk;
//      sptr_tclk = &sgtc_systime_clock;
//      lv_label_set_text_fmt(sptr_obj, "system run time: %d-%d-%d %d:%d:%d:%d",
//              sptr_tclk->year, sptr_tclk->month, sptr_tclk->day,
//              sptr_tclk->hour, sptr_tclk->minute, sptr_tclk->second, sptr_tclk->milsecond);

       lv_label_set_text_fmt(sptr_obj, "yujiantianhu@163.com");

        lv_obj_add_style(sptr_obj, sptr_style, 0);
        lv_obj_align_to(sptr_obj, this->sptr_statusbar, LV_ALIGN_CENTER, 0, 0);

        sptr_timer = lv_timer_create(lvgl_time_event, 10, this);

        this->sptr_timicon = sptr_obj;
        this->sptr_timer = sptr_timer;
    }

    /*!< power icon */
    sptr_obj = lv_label_create(this->sptr_statusbar);
    if (sptr_obj) {
        lv_label_set_text(sptr_obj, LV_SYMBOL_BATTERY_3);
        lv_obj_add_style(sptr_obj, sptr_style, 0);
        lv_obj_align_to(sptr_obj, sptr_neibh, LV_ALIGN_RIGHT_MID, 0, 0);

        sptr_neibh = sptr_obj;
        this->sptr_pwricon = sptr_obj;
    }

    /*!< bell icon */
    sptr_obj = lv_label_create(this->sptr_statusbar);
    if (sptr_obj) {
        lv_label_set_text(sptr_obj, LV_SYMBOL_BELL);
        lv_obj_add_style(sptr_obj, sptr_style, 0);
        lv_obj_align_to(sptr_obj, sptr_neibh, LV_ALIGN_OUT_LEFT_MID, -10, 0);

        sptr_neibh = sptr_obj;
        this->sptr_bellicon = sptr_obj;
    }

    /*!< wifi icon */
    sptr_obj = lv_label_create(this->sptr_statusbar);
    if (sptr_obj) {
        lv_label_set_text(sptr_obj, LV_SYMBOL_WIFI);
        lv_obj_add_style(sptr_obj, sptr_style, 0);
        lv_obj_align_to(sptr_obj, sptr_neibh, LV_ALIGN_OUT_LEFT_MID, -10, 0);

        sptr_neibh = sptr_obj;
        this->sptr_wifiicon = sptr_obj;
    }

    /*!< BlueTooth icon */
    sptr_obj = lv_label_create(this->sptr_statusbar);
    if (sptr_obj) {
        lv_label_set_text(sptr_obj, LV_SYMBOL_BLUETOOTH);
        lv_obj_add_style(sptr_obj, sptr_style, 0);
        lv_obj_align_to(sptr_obj, sptr_neibh, LV_ALIGN_OUT_LEFT_MID, -10, 0);

        sptr_neibh = sptr_obj;
        this->sptr_bleicon = sptr_obj;
    }

    /*!< gps icon */
    sptr_obj = lv_label_create(this->sptr_statusbar);
    if (sptr_obj) {
        lv_label_set_text(sptr_obj, LV_SYMBOL_GPS);
        lv_obj_add_style(sptr_obj, sptr_style, 0);
        lv_obj_align_to(sptr_obj, sptr_neibh, LV_ALIGN_OUT_LEFT_MID, -10, 0);

        sptr_neibh = sptr_obj;
        this->sptr_gpsicon = sptr_obj;
    }
}

static void lvgl_time_event(lv_timer_t *sptr_timer)
{
//  crt_main_ui_t *sptr_this;
//  struct time_clock *sptr_tclk;
//  
//  sptr_this = (crt_main_ui_t *)sptr_timer->user_data;
//  sptr_tclk = &sgtc_systime_clock;
//  lv_label_set_text_fmt(sptr_this->sptr_timicon, "system run time: %d-%d-%d %d:%d:%d:%d",
//          sptr_tclk->year, sptr_tclk->month, sptr_tclk->day,
//          sptr_tclk->hour, sptr_tclk->minute, sptr_tclk->second, sptr_tclk->milsecond);
}

/*!
 * @brief  start up
 * @param  sptr_dctrl
 * @retval none
 * @note   none
 */
void lvgl_task_setup(void *args)
{
    crt_main_ui_t *cptr_ui;

    cptr_ui = new crt_main_ui_t;
    if (!cptr_ui)
        return;

    cptr_ui->setup_desktop();
    cptr_ui->setup_status_bar();
    cptr_ui->setup_icon();
}

/*!
 * @brief  main
 * @param  args
 * @retval none
 * @note   none
 */
void lvgl_task(void *args)
{
    lv_timer_handler();
}

/*!< end of file */
