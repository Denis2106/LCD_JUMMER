#include "scr_test.h"
#include <lvgl.h>
#include "debug.h"

#include "lvgl_fonts/font_rus.h"
#include "data_mode.h"


static const lv_font_t * font_large;
static const lv_font_t * font_normal;

static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_icon;
static lv_style_t style_bullet;

static lv_obj_t * tabview;

// Элементы таба "Режимы"
static lv_obj_t* table;

// Элементы таба "Настройка"
static int cur_mode_idx = 0;
static lv_obj_t* dd_mode;
static lv_obj_t* ta_freq_min;
static lv_obj_t* ta_freq_max;
static lv_obj_t* cb_sf6;
static lv_obj_t* cb_sf7;
static lv_obj_t* cb_sf8;
static lv_obj_t* cb_sf9;

static lv_obj_t* load_btn;
static lv_obj_t* save_btn;


#define MODE_STR_SIZE 30
static char mode_buf[MODE_STR_SIZE];
static char res_buf[(MODE_STR_SIZE+1)*MAX_MODE_COUNT];


void modes_table_fill(ModeData* mode, int row)
{
    lv_table_set_cell_value_fmt(table, row, 0, "%d", row+1);
    char buf[30];
    sprintf(buf, "%6.1f - %6.1f", mode->freq_min, mode->freq_max);
    lv_table_set_cell_value(table, row, 1, buf);
    lv_table_set_cell_value(table, row, 2, mode_get_sf_list(mode, ','));
}


/**
 * Загружает список режимов в modes.Table и mode.DropDown
 */
void tab_modes_load()
{
    // Очистка таблицы режимов
    lv_table_set_row_cnt(table, 0);

    // Формирование строки с перечисленим режимов
    int pos;
    pos = 0;
    for (int i=0; i<mode_count; i++) {        
        ModeData* mode = mode_get(i);

        // TODO отображение списка в таблицу
        modes_table_fill(mode, i);

        // Формирование строки для DropDown
        sprintf(&(res_buf[pos]), "%d: %.1f-%.1f  SF=", i+1, mode->freq_min, mode->freq_max);
        pos = strlen(res_buf);

        // Формирование списка SF и копирование в строку результата
        char* sf_list_buf = mode_get_sf_list(mode, ',');
        for (int sf_i=0; sf_list_buf[sf_i]; sf_i++)
            res_buf[pos++] = sf_list_buf[sf_i];

        res_buf[pos++] = '\n';
        res_buf[pos] = 0;
    }

    lv_dropdown_set_options_static(dd_mode, res_buf);
//    lv_dropdown_set_options_static(dd_mode, "1: 700-750 6\n2: 750-800 6,7\n3: 800-830 6,7,8,9");
}


void set_checked(lv_obj_t* checkbox, int value)
{
    if (value)
        lv_obj_add_state(checkbox, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(checkbox, LV_STATE_CHECKED);
}

/**
 * Загружает свойства выбранного режима
 */
void tab_mode_load()
{
    cur_mode_idx = lv_dropdown_get_selected(dd_mode);
    ModeData* mode = mode_get(cur_mode_idx);

    char buf[20];

    sprintf(buf, "%.1f", mode->freq_min);
    lv_textarea_set_text(ta_freq_min, buf);

    sprintf(buf, "%.1f", mode->freq_max);
    lv_textarea_set_text(ta_freq_max, buf);

    set_checked(cb_sf6, mode_get_sf_bit(mode, 6));
    set_checked(cb_sf7, mode_get_sf_bit(mode, 7));
    set_checked(cb_sf8, mode_get_sf_bit(mode, 8));
    set_checked(cb_sf9, mode_get_sf_bit(mode, 9));
}


void tab_mode_save()
{
    cur_mode_idx = lv_dropdown_get_selected(dd_mode);
    ModeData* mode = mode_get(cur_mode_idx);

    mode->freq_min = atof( lv_textarea_get_text(ta_freq_min) );
    mode->freq_max = atof( lv_textarea_get_text(ta_freq_max) );

    mode_set_sf_bit(mode, 6, lv_obj_get_state(cb_sf6) & LV_STATE_CHECKED);
    mode_set_sf_bit(mode, 7, lv_obj_get_state(cb_sf7) & LV_STATE_CHECKED);
    mode_set_sf_bit(mode, 8, lv_obj_get_state(cb_sf8) & LV_STATE_CHECKED);
    mode_set_sf_bit(mode, 9, lv_obj_get_state(cb_sf9) & LV_STATE_CHECKED);

    mode_commit(cur_mode_idx);

    tab_modes_load();
    lv_dropdown_set_selected(dd_mode, cur_mode_idx);
}


static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);

    if (btn == load_btn)
        tab_mode_load();

    else if (btn == save_btn)
        tab_mode_save();

}


static void ta_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = (lv_obj_t*)lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        if(lv_indev_get_type(lv_indev_get_act()) != LV_INDEV_TYPE_KEYPAD) {
            lv_keyboard_set_textarea(kb, ta);
            lv_obj_set_style_max_height(kb, LV_HOR_RES * 2 / 3, 0);
            lv_obj_update_layout(tabview);   /*Be sure the sizes are recalculated*/
            lv_obj_set_height(tabview, LV_VER_RES - lv_obj_get_height(kb));
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
            lv_obj_scroll_to_view_recursive(ta, LV_ANIM_OFF);
        }
    }

    else if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_set_height(tabview, LV_VER_RES);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_indev_reset(NULL, ta);
    }

    else if(code == LV_EVENT_CANCEL) {
        lv_obj_set_height(tabview, LV_VER_RES);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_state(ta, LV_STATE_FOCUSED);
        lv_indev_reset(NULL, ta);   /*To forget the last clicked object to make it focusable again*/
    }

    else if(code == LV_EVENT_READY) {
        lv_obj_clear_state(ta, LV_STATE_FOCUSED);
        lv_indev_reset(NULL, ta);   /*To forget the last clicked object to make it focusable again*/

        if (ta == ta_freq_min) {
            lv_obj_add_state(ta_freq_max, LV_STATE_FOCUSED);
            lv_keyboard_set_textarea(kb, ta_freq_max);

        } else {
            lv_obj_set_height(tabview, LV_VER_RES);
            lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        }
    }
}


static void slider_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        lv_coord_t * s = (lv_coord_t*)lv_event_get_param(e);
        *s = LV_MAX(*s, 60);
    }
    else if(code == LV_EVENT_DRAW_PART_END) {
        lv_obj_draw_part_dsc_t * dsc = (lv_obj_draw_part_dsc_t*)lv_event_get_param(e);
        if(dsc->part == LV_PART_KNOB && lv_obj_has_state(obj, LV_STATE_PRESSED)) {
            char buf[8];
            lv_snprintf(buf, sizeof(buf), "%"LV_PRId32, lv_slider_get_value(obj));

            lv_point_t text_size;
            lv_txt_get_size(&text_size, buf, font_normal, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            lv_area_t txt_area;
            txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2 - text_size.x / 2;
            txt_area.x2 = txt_area.x1 + text_size.x;
            txt_area.y2 = dsc->draw_area->y1 - 10;
            txt_area.y1 = txt_area.y2 - text_size.y;

            lv_area_t bg_area;
            bg_area.x1 = txt_area.x1 - LV_DPX(8);
            bg_area.x2 = txt_area.x2 + LV_DPX(8);
            bg_area.y1 = txt_area.y1 - LV_DPX(8);
            bg_area.y2 = txt_area.y2 + LV_DPX(8);

            lv_draw_rect_dsc_t rect_dsc;
            lv_draw_rect_dsc_init(&rect_dsc);
            rect_dsc.bg_color = lv_palette_darken(LV_PALETTE_GREY, 3);
            rect_dsc.radius = LV_DPX(5);
            lv_draw_rect(dsc->draw_ctx, &rect_dsc, &bg_area);

            lv_draw_label_dsc_t label_dsc;
            lv_draw_label_dsc_init(&label_dsc);
            label_dsc.color = lv_color_white();
            label_dsc.font = font_normal;
            lv_draw_label(dsc->draw_ctx, &label_dsc, &txt_area, buf, NULL);
        }
    }
}


static void tab_info_create(lv_obj_t * parent)
{
    static lv_coord_t grid_main_col_dsc[] = {
        LV_GRID_FR(1), 
        LV_GRID_TEMPLATE_LAST
    };
    static lv_coord_t grid_main_row_dsc[] = {
        LV_GRID_CONTENT, 
        LV_GRID_CONTENT, 
        LV_GRID_CONTENT, 
        LV_GRID_TEMPLATE_LAST
    };
    lv_obj_set_grid_dsc_array(parent, grid_main_col_dsc, grid_main_row_dsc);


    lv_obj_t * panel1 = lv_obj_create(parent);
    lv_obj_set_height(panel1, LV_SIZE_CONTENT);
    lv_obj_set_grid_cell(panel1, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    //LV_IMG_DECLARE(img_demo_widgets_avatar);
    //lv_obj_t * avatar = lv_img_create(panel1);
    //lv_img_set_src(avatar, &img_demo_widgets_avatar);

    lv_obj_t * name = lv_label_create(panel1);
    lv_label_set_text(name, "Персонаж 1");
    lv_obj_add_style(name, &style_title, 0);

    lv_obj_t * dsc = lv_label_create(panel1);
    lv_obj_add_style(dsc, &style_text_muted, 0);
    lv_label_set_text(dsc, "This is a short description of me. Take a look at my profile!");
    lv_label_set_long_mode(dsc, LV_LABEL_LONG_WRAP);

    lv_obj_t * email_icn = lv_label_create(panel1);
    lv_obj_add_style(email_icn, &style_icon, 0);
    lv_label_set_text(email_icn, LV_SYMBOL_ENVELOPE);

    lv_obj_t * email_label = lv_label_create(panel1);
    lv_label_set_text(email_label, "elena@smith.com");

    lv_obj_t * call_icn = lv_label_create(panel1);
    lv_obj_add_style(call_icn, &style_icon, 0);
    lv_label_set_text(call_icn, LV_SYMBOL_CALL);

    lv_obj_t * call_label = lv_label_create(panel1);
    lv_label_set_text(call_label, "+79 246 123 4567");

    lv_obj_t * log_out_btn = lv_btn_create(panel1);
    lv_obj_set_height(log_out_btn, LV_SIZE_CONTENT);

    lv_obj_t * label = lv_label_create(log_out_btn);
    lv_label_set_text(label, "Log out");
    lv_obj_center(label);

    lv_obj_t * invite_btn = lv_btn_create(panel1);
    lv_obj_add_state(invite_btn, LV_STATE_DISABLED);
    lv_obj_set_height(invite_btn, LV_SIZE_CONTENT);

    label = lv_label_create(invite_btn);
    lv_label_set_text(label, "Invite");
    lv_obj_center(label);

    /*Create the top panel*/
    static lv_coord_t grid_1_col_dsc[] = {
        LV_GRID_CONTENT, 
        LV_GRID_FR(1), 
        LV_GRID_TEMPLATE_LAST
    };
    static lv_coord_t grid_1_row_dsc[] = {
        LV_GRID_CONTENT, /*Avatar*/
        LV_GRID_CONTENT, /*Name*/
        LV_GRID_CONTENT, /*Description*/
        LV_GRID_CONTENT, /*Email*/
        LV_GRID_CONTENT, /*Phone number*/
        LV_GRID_CONTENT, /*Button1*/
        LV_GRID_CONTENT, /*Button2*/
        LV_GRID_TEMPLATE_LAST
    };

    lv_obj_set_grid_dsc_array(panel1, grid_1_col_dsc, grid_1_row_dsc);

    lv_obj_set_style_text_align(dsc, LV_TEXT_ALIGN_CENTER, 0);

    //lv_obj_set_grid_cell(avatar, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_grid_cell(name, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_set_grid_cell(dsc, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_START, 2, 1);
    lv_obj_set_grid_cell(email_icn, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    lv_obj_set_grid_cell(email_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    lv_obj_set_grid_cell(call_icn, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_set_grid_cell(call_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_set_grid_cell(log_out_btn, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 5, 1);
    lv_obj_set_grid_cell(invite_btn, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 6, 1);


 //------------------------------------------------------------------------------------
    static lv_coord_t grid_2_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_2_row_dsc[] = {
        LV_GRID_CONTENT,  /*Title*/
        5,                /*Separator*/
        LV_GRID_CONTENT,  /*Box title*/
        40,               /*Box*/
        LV_GRID_CONTENT,  /*Box title*/
        40,               /*Box*/
        LV_GRID_CONTENT,  /*Box title*/
        40,               /*Box*/
        LV_GRID_CONTENT,  /*Box title*/
        40, LV_GRID_TEMPLATE_LAST               /*Box*/
    };

    /*Create the second panel*/
    lv_obj_t * panel2 = lv_obj_create(parent);
    lv_obj_set_height(panel2, LV_SIZE_CONTENT);
    lv_obj_set_grid_dsc_array(panel2, grid_2_col_dsc, grid_2_row_dsc);
    lv_obj_set_grid_cell(panel2, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 1, 1);

    lv_obj_t * panel2_title = lv_label_create(panel2);
    lv_label_set_text(panel2_title, "Your profile");
    lv_obj_add_style(panel2_title, &style_title, 0);

    lv_obj_t * user_name_label = lv_label_create(panel2);
    lv_label_set_text(user_name_label, "User name");
    lv_obj_add_style(user_name_label, &style_text_muted, 0);

    /*Create a keyboard*/
    lv_obj_t* kb = lv_keyboard_create(lv_scr_act());
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t * user_name = lv_textarea_create(panel2);
    lv_textarea_set_one_line(user_name, true);
    lv_textarea_set_placeholder_text(user_name, "Your name");
    lv_obj_add_event_cb(user_name, ta_event_cb, LV_EVENT_ALL, kb);

    lv_obj_t * gender_label = lv_label_create(panel2);
    lv_label_set_text(gender_label, "Gender");
    lv_obj_add_style(gender_label, &style_text_muted, 0);

    lv_obj_t * gender = lv_dropdown_create(panel2);
    lv_dropdown_set_options_static(gender, "Male\nFemale\nOther");

    lv_obj_set_grid_cell(panel2_title, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_grid_cell(user_name_label, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 2, 1);
    lv_obj_set_grid_cell(user_name, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_START, 3, 1);
    lv_obj_set_grid_cell(gender_label, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 4, 1);
    lv_obj_set_grid_cell(gender, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_START, 5, 1);

  //-------------------------------------------------------------------------------

    /*Create the third panel*/
    lv_obj_t * panel3 = lv_obj_create(parent);
    lv_obj_set_grid_dsc_array(panel3, grid_2_col_dsc, grid_2_row_dsc);
    lv_obj_set_height(panel3, LV_SIZE_CONTENT);
    lv_obj_set_grid_cell(panel3, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 2, 1);

    lv_obj_t * panel3_title = lv_label_create(panel3);
    lv_label_set_text(panel3_title, "Your skills");
    lv_obj_add_style(panel3_title, &style_title, 0);

    lv_obj_t * experience_label = lv_label_create(panel3);
    lv_label_set_text(experience_label, "Experience");
    lv_obj_add_style(experience_label, &style_text_muted, 0);

    lv_obj_t * slider1 = lv_slider_create(panel3);
    lv_obj_set_width(slider1, LV_PCT(95));
    lv_obj_add_event_cb(slider1, slider_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_refresh_ext_draw_size(slider1);

    lv_obj_t * team_player_label = lv_label_create(panel3);
    lv_label_set_text(team_player_label, "Team player");
    lv_obj_add_style(team_player_label, &style_text_muted, 0);

    lv_obj_t * sw1 = lv_switch_create(panel3);

    lv_obj_t * hard_working_label = lv_label_create(panel3);
    lv_label_set_text(hard_working_label, "Hard-working");
    lv_obj_add_style(hard_working_label, &style_text_muted, 0);
    
    lv_obj_t * sw2 = lv_switch_create(panel3);

    lv_obj_set_grid_cell(panel3_title, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_grid_cell(experience_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
    lv_obj_set_grid_cell(slider1, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 3, 1);
    lv_obj_set_grid_cell(hard_working_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 4, 1);
    lv_obj_set_grid_cell(sw1, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 5, 1);
    lv_obj_set_grid_cell(team_player_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 4, 1);
    lv_obj_set_grid_cell(sw2, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 5, 1);
}


static void tab_modes_create(lv_obj_t* parent)
{
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* header = lv_table_create(parent);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(header, 300);
    lv_obj_set_height(header, 40);
    lv_table_set_col_width(header, 0, 70);
    lv_table_set_col_width(header, 1, 120);
    lv_table_set_col_width(header, 2, 100);
    lv_table_set_cell_value(header, 0, 0, "Номер");
    lv_table_set_cell_value(header, 0, 1, "Частота, МГц");
    lv_table_set_cell_value(header, 0, 2, "SF");

    table = lv_table_create(parent);
    lv_obj_set_y(table, 40);
    lv_obj_set_width(table, 300);
    lv_obj_set_height(table, 140);
    lv_obj_add_flag(table, LV_OBJ_FLAG_SCROLLABLE);
    lv_table_set_col_width(table, 0, 50);
    lv_table_set_col_width(table, 1, 120);
    lv_table_set_col_width(table, 2, 120);
}


void tab_mode_create(lv_obj_t* parent)
{
    static lv_coord_t grid_main_col_dsc[] = {
        LV_GRID_CONTENT, 
        LV_GRID_FR(1), 
        LV_GRID_FR(1), 
        LV_GRID_CONTENT, 
        LV_GRID_TEMPLATE_LAST
    };
    static lv_coord_t grid_main_row_dsc[] = {
        LV_GRID_CONTENT, // Mode
        LV_GRID_CONTENT, // Freq
        LV_GRID_CONTENT, // SF
        LV_GRID_CONTENT, // SF
        LV_GRID_TEMPLATE_LAST
    };
    lv_obj_set_grid_dsc_array(parent, grid_main_col_dsc, grid_main_row_dsc);

  // Mode
    lv_obj_t * mode_lbl = lv_label_create(parent);
    lv_label_set_text(mode_lbl, "Режим");
    lv_obj_set_grid_cell(mode_lbl, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    dd_mode = lv_dropdown_create(parent);
    lv_obj_set_grid_cell(dd_mode, LV_GRID_ALIGN_STRETCH, 1, 3, LV_GRID_ALIGN_CENTER, 0, 1);
    tab_modes_load();

  // Freq
    lv_obj_t * freq_lbl = lv_label_create(parent);
    lv_label_set_text(freq_lbl, "Частота, МГц");
    lv_obj_set_grid_cell(freq_lbl, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    /*Create a keyboard*/
    lv_obj_t* kb = lv_keyboard_create(lv_scr_act());
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

    ta_freq_min = lv_textarea_create(parent);
    lv_textarea_set_one_line(ta_freq_min, true);
    lv_textarea_set_placeholder_text(ta_freq_min, "от");
    lv_obj_add_event_cb(ta_freq_min, ta_event_cb, LV_EVENT_ALL, kb);
    lv_obj_set_grid_cell(ta_freq_min, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    ta_freq_max = lv_textarea_create(parent);
    lv_textarea_set_one_line(ta_freq_max, true);
    lv_textarea_set_placeholder_text(ta_freq_max, "до");
    lv_obj_add_event_cb(ta_freq_max, ta_event_cb, LV_EVENT_ALL, kb);
    lv_obj_set_grid_cell(ta_freq_max, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  // SF
    lv_obj_t * sf_lbl = lv_label_create(parent);
    lv_label_set_text(sf_lbl, "SF");
    lv_obj_set_grid_cell(sf_lbl, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 2, 2);

    cb_sf6 = lv_checkbox_create(parent);
    lv_checkbox_set_text(cb_sf6, "6");
    lv_obj_add_state(cb_sf6, LV_STATE_CHECKED);
    lv_obj_set_grid_cell(cb_sf6, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    cb_sf7 = lv_checkbox_create(parent);
    lv_checkbox_set_text(cb_sf7, "7");
    lv_obj_add_state(cb_sf7, LV_STATE_CHECKED);
    lv_obj_set_grid_cell(cb_sf7, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    cb_sf8 = lv_checkbox_create(parent);
    lv_checkbox_set_text(cb_sf8, "8");
    lv_obj_add_state(cb_sf8, LV_STATE_CHECKED);
    lv_obj_set_grid_cell(cb_sf8, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);

    cb_sf9 = lv_checkbox_create(parent);
    lv_checkbox_set_text(cb_sf9, "9");
    lv_obj_add_state(cb_sf9, LV_STATE_CHECKED);
    lv_obj_set_grid_cell(cb_sf9, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_CENTER, 3, 1);

  // Buttons
    load_btn = lv_btn_create(parent);
    lv_obj_add_event_cb(load_btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
    ////lv_obj_add_state(load_btn, LV_STATE_DISABLED);
    lv_obj_set_height(load_btn, LV_SIZE_CONTENT);
    lv_obj_t* label = lv_label_create(load_btn);
    lv_label_set_text(label, "Читать");
    lv_obj_center(label);
    lv_obj_set_grid_cell(load_btn, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    save_btn = lv_btn_create(parent);
    lv_obj_add_event_cb(save_btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
    ////lv_obj_add_state(save_btn, LV_STATE_DISABLED);
    lv_obj_set_height(save_btn, LV_SIZE_CONTENT);
    label = lv_label_create(save_btn);
    lv_label_set_text(label, "Сохр.");
    lv_obj_center(label);
    lv_obj_set_grid_cell(save_btn, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_CENTER, 2, 1);
}


void scr_test(void)
{
    font_large = LV_FONT_DEFAULT;
    font_normal = LV_FONT_DEFAULT;

    font_large = &lv_font_calibri_18;
    font_normal = &lv_font_calibri_14;

    lv_coord_t tab_h;
    tab_h = 35;

    lv_style_init(&style_text_muted);
    lv_style_set_text_opa(&style_text_muted, LV_OPA_50);

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_large);

    lv_style_init(&style_icon);
    lv_style_set_text_color(&style_icon, lv_theme_get_color_primary(NULL));
    lv_style_set_text_font(&style_icon, font_large);

    lv_style_init(&style_bullet);
    lv_style_set_border_width(&style_bullet, 0);
    lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);


    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    tabview = lv_tabview_create(screen, LV_DIR_TOP, tab_h);

//    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);


        lv_obj_t * tab_btns = lv_tabview_get_tab_btns(tabview);
        lv_obj_set_style_pad_left(tab_btns, LV_HOR_RES / 2, 0);

        lv_obj_t * label = lv_label_create(tab_btns);
        lv_obj_add_style(label, &style_title, 0);
        lv_label_set_text(label, "Подавитель");
        lv_obj_align(label, LV_ALIGN_OUT_LEFT_TOP, -LV_HOR_RES / 2 + 25, 0);

        label = lv_label_create(tab_btns);
        lv_obj_add_style(label, &style_text_muted, 0);
        lv_label_set_text(label, "V1.0");
        lv_obj_align(label, LV_ALIGN_OUT_LEFT_TOP, -LV_HOR_RES / 2 + 25, 0);
        lv_obj_set_y(label, 20);
        //lv_obj_align(label, LV_ALIGN_OUT_LEFT_BOTTOM, -LV_HOR_RES / 2 + 25, 0);



    //lv_obj_set_style_text_font(lv_scr_act(), font_normal, 0);
    lv_obj_set_style_text_font(screen, font_normal, 0);

    lv_obj_t * t1 = lv_tabview_add_tab(tabview, "Статус");
    lv_obj_t * t2 = lv_tabview_add_tab(tabview, "Режим");
    lv_obj_t * t3 = lv_tabview_add_tab(tabview, "Настр.");

    lv_disp_load_scr(screen);

    tab_info_create(t1);
    tab_modes_create(t2);
    tab_mode_create(t3);

    //lv_event_send(screen, LV_EVENT_REFRESH, NULL);
}
