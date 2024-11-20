/*
 * @Description: None
 * @version: V1.0.0
 * @Author: LILYGO_L
 * @Date: 2023-11-06 11:32:03
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-09-19 22:01:22
 * @License: GPL 3.0
 */

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"

void setup_scr_Key_Mode(lv_ui *ui)
{
    My_UI.Window_Load_Anim_Delay_Lock_Flag = false;
    My_UI.Window_Load_Anim_Delay = 4294967295LL; // 窗口等待动画加载延时，初始化设置最大
    My_UI.Window_Unlock_Flag = false;
    My_UI.Window_Initialization_Number = 1;

    // N085_Screen_Set(N085_Screen_ALL);

    // Write codes Key_Mode
    ui->Key_Mode = lv_obj_create(NULL);
    lv_obj_set_size(ui->Key_Mode, 128, 128);
    lv_obj_set_scrollbar_mode(ui->Key_Mode, LV_SCROLLBAR_MODE_OFF);

    // Write style for Key_Mode, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->Key_Mode, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ui->Key_Mode, &_1_alpha_128x128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui->Key_Mode, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes Key_Mode_imgbtn_1
    ui->Key_Mode_imgbtn_1 = lv_imgbtn_create(ui->Key_Mode);
    lv_obj_add_flag(ui->Key_Mode_imgbtn_1, LV_OBJ_FLAG_CHECKABLE);
    lv_imgbtn_set_src(ui->Key_Mode_imgbtn_1, LV_IMGBTN_STATE_RELEASED, NULL, &_15_alpha_128x128, NULL);
    lv_imgbtn_set_src(ui->Key_Mode_imgbtn_1, LV_IMGBTN_STATE_PRESSED, NULL, &_15_alpha_128x128, NULL);
    lv_obj_add_flag(ui->Key_Mode_imgbtn_1, LV_OBJ_FLAG_CHECKABLE);
    ui->Key_Mode_imgbtn_1_label = lv_label_create(ui->Key_Mode_imgbtn_1);
    lv_label_set_text(ui->Key_Mode_imgbtn_1_label, "");
    lv_label_set_long_mode(ui->Key_Mode_imgbtn_1_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->Key_Mode_imgbtn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->Key_Mode_imgbtn_1, 0, LV_STATE_DEFAULT);
    lv_obj_set_pos(ui->Key_Mode_imgbtn_1, 0, 0);
    lv_obj_set_size(ui->Key_Mode_imgbtn_1, 128, 128);
    lv_obj_set_scrollbar_mode(ui->Key_Mode_imgbtn_1, LV_SCROLLBAR_MODE_OFF);

    // Write style for Key_Mode_imgbtn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_img_opa(ui->Key_Mode_imgbtn_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->Key_Mode_imgbtn_1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->Key_Mode_imgbtn_1, &lv_font_montserratMedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->Key_Mode_imgbtn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->Key_Mode_imgbtn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write style for Key_Mode_imgbtn_1, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
    lv_obj_set_style_img_opa(ui->Key_Mode_imgbtn_1, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(ui->Key_Mode_imgbtn_1, lv_color_hex(0xFF33FF), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_font(ui->Key_Mode_imgbtn_1, &lv_font_montserratMedium_16, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(ui->Key_Mode_imgbtn_1, 0, LV_PART_MAIN | LV_STATE_PRESSED);

    // Write style for Key_Mode_imgbtn_1, Part: LV_PART_MAIN, State: LV_STATE_CHECKED.
    lv_obj_set_style_img_opa(ui->Key_Mode_imgbtn_1, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(ui->Key_Mode_imgbtn_1, lv_color_hex(0xFF33FF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_font(ui->Key_Mode_imgbtn_1, &lv_font_montserratMedium_16, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(ui->Key_Mode_imgbtn_1, 0, LV_PART_MAIN | LV_STATE_CHECKED);

    // Update current screen layout.
    lv_obj_update_layout(ui->Key_Mode);

    // Init events for screen.
    events_init_Key_Mode(ui);
}
