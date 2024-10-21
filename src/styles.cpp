#include "lvgl_fonts/font_rus.h"
#include "styles.h"

const lv_font_t * font_large;
const lv_font_t * font_normal;

lv_style_t style_text_muted;
lv_style_t style_title;
lv_style_t style_icon;
lv_style_t style_bullet;


void styles_init()
{
    font_large = LV_FONT_DEFAULT;
    font_normal = LV_FONT_DEFAULT;

    font_large = &lv_font_calibri_18;
    font_normal = &lv_font_calibri_14;

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
}