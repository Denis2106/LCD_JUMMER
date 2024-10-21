
#include "tab_modes.h"
#include "data_mode.h"

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

    for (int i=0; i<mode_count; i++) {        
        ModeData* mode = mode_get(i);

        // TODO отображение списка в таблицу
        modes_table_fill(mode, i);
    }

    char* modes_str = modes_get_dropdown_options();
    lv_dropdown_set_options_static(dd_mode, modes_str);
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
void on_mode_load()
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


void on_mode_save()
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
        on_mode_load();

    else if (btn == save_btn)
        on_mode_save();

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


void tab_modes_create(lv_obj_t* parent)
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
