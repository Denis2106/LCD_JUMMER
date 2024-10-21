#include "scr_test.h"
#include <lvgl.h>

#include "debug.h"
#include "styles.h"
#include "tab_status.h"
#include "tab_modes.h"
#include "tab_info.h"


lv_obj_t * tabview;


void scr_test(void)
{
    styles_init();

    lv_coord_t tab_h;
    tab_h = 35;

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
    lv_obj_t * t4 = lv_tabview_add_tab(tabview, "Инф.");

    lv_disp_load_scr(screen);

    tab_status_create(t1);
    tab_modes_create(t2);
    tab_mode_create(t3);
    tab_info_create(t4);

    //lv_event_send(screen, LV_EVENT_REFRESH, NULL);
}
