#include "tab_info.h"

static lv_obj_t* textarea;


void add_info(const char *txt, bool add_CR)
{
    lv_textarea_add_text(textarea, txt);

    if (add_CR)
        lv_textarea_add_text(textarea, "\n");
}


void tab_info_create(lv_obj_t* parent)
{
    textarea = lv_textarea_create(parent);
}