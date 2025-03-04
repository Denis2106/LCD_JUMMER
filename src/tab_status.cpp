/*
Наполнение страницы статуса
 - текущий режим (выбор из dropdown)
 - включение усилителей
 - усиление (слайдер)
 - кнопки пуск/стоп
*/

#include "styles.h"
#include "tab_status.h"
#include "data_status.h"
#include "data_mode.h"

static lv_obj_t* dd_mode;
static lv_obj_t* cb_ch1;
static lv_obj_t* cb_ch2;
static lv_obj_t* btn_start;
static lv_obj_t* btn_stop;


static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);

    if (btn == btn_start) {
        int cur_mode_idx = lv_dropdown_get_selected(dd_mode);
        cmd_start(cur_mode_idx);
    }
    else if (btn == btn_stop)
        cmd_stop();

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


void load_modes()
{
    char* modes_str = modes_get_dropdown_options();
    lv_dropdown_set_options_static(dd_mode, "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n");
}


void tab_status_create(lv_obj_t * parent)
{
    static lv_coord_t grid_main_col_dsc[] = {
        LV_GRID_CONTENT, 
        LV_GRID_FR(1), 
        LV_GRID_FR(1), 
        LV_GRID_TEMPLATE_LAST
    };
    static lv_coord_t grid_main_row_dsc[] = {
        LV_GRID_CONTENT, // Режим
        LV_GRID_CONTENT, // Каналы
        LV_GRID_CONTENT,  // Усиление
        LV_GRID_CONTENT,  // Кнопки
        LV_GRID_TEMPLATE_LAST 
    };
    lv_obj_set_grid_dsc_array(parent, grid_main_col_dsc, grid_main_row_dsc);

  // Mode
    lv_obj_t * mode_lbl = lv_label_create(parent);
    lv_label_set_text(mode_lbl, "Режим");
    lv_obj_set_grid_cell(mode_lbl, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    dd_mode = lv_dropdown_create(parent);
    lv_obj_set_grid_cell(dd_mode, LV_GRID_ALIGN_STRETCH, 1, 2, LV_GRID_ALIGN_CENTER, 0, 1);
    load_modes();

    lv_obj_t * sf_lbl = lv_label_create(parent);
    lv_label_set_text(sf_lbl, "Каналы");
    lv_obj_set_grid_cell(sf_lbl, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    cb_ch1 = lv_checkbox_create(parent);
    lv_checkbox_set_text(cb_ch1, "1");
    lv_obj_add_state(cb_ch1, LV_STATE_CHECKED);
    lv_obj_set_grid_cell(cb_ch1, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    cb_ch2 = lv_checkbox_create(parent);
    lv_checkbox_set_text(cb_ch2, "2");
    lv_obj_add_state(cb_ch2, LV_STATE_CHECKED);
    lv_obj_set_grid_cell(cb_ch2, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);


  // Gain
    lv_obj_t * gain_lbl = lv_label_create(parent);
    lv_label_set_text(gain_lbl, "Мощность");
    lv_obj_set_grid_cell(gain_lbl, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    lv_obj_t * slider1 = lv_slider_create(parent);
    lv_obj_set_width(slider1, LV_PCT(95));
    lv_obj_add_event_cb(slider1, slider_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_refresh_ext_draw_size(slider1);
    lv_obj_set_grid_cell(slider1, LV_GRID_ALIGN_STRETCH, 1, 2, LV_GRID_ALIGN_CENTER, 2, 1);


  // Buttons
    btn_start = lv_btn_create(parent);
    lv_obj_add_event_cb(btn_start, btn_event_cb, LV_EVENT_CLICKED, NULL);
    ////lv_obj_add_state(btn_start, LV_STATE_DISABLED);
    lv_obj_set_height(btn_start, LV_SIZE_CONTENT);
    lv_obj_t* label = lv_label_create(btn_start);
    lv_label_set_text(label, "Старт");
    lv_obj_center(label);
    lv_obj_set_grid_cell(btn_start, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);

    btn_stop = lv_btn_create(parent);
    lv_obj_set_height(btn_stop, LV_SIZE_CONTENT);
    label = lv_label_create(btn_stop);
    lv_label_set_text(label, "Стоп");
    lv_obj_center(label);
    lv_obj_set_grid_cell(btn_stop, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_CENTER, 3, 1);

}


