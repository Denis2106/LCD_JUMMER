#include <string.h>

#include "data_mode.h"
#include "link.h"

static ModeData mode_data[MODES_COUNT] = {
    {.freq_min=700, .freq_max=750, .sf_bits=0b00001111},
    {.freq_min=710, .freq_max=740, .sf_bits=0b00000011},
    {.freq_min=720, .freq_max=730, .sf_bits=0b00001000},
};

static char mode_buf[MODE_STR_SIZE];

char btn_modes_map[MODES_COUNT][MODE_STR_SIZE] = {
    "0+1: ---",
    "0+2: ---",
    "1+0: ---",
    "1+1: ---",
    "1+2: ---",
    "2+0: ---",
    "2+1: ---",
    "2+2: ---",
};


#define SF_TO_BIT(sf) (1<<(sf-MIN_SF_VALUE))

void mode_clear()
{
    for (int i=0; i<MODES_COUNT; i++) {
        mode_data[i].freq_min = 0;
        mode_data[i].freq_max = 0;
        mode_data[i].sf_bits = 0;
    }
}


void mode_set_sf_bit(ModeData* mode, int sf, int value)
{
    if (value == 0)
        mode->sf_bits &= ~SF_TO_BIT(sf);
    else
        mode->sf_bits |= SF_TO_BIT(sf);
}


int mode_get_sf_bit(ModeData* mode, int sf) 
{
    return mode->sf_bits & SF_TO_BIT(sf);
}


void mode_update(int idx, char* key, char* value)
{
    if (idx < 0 || idx >= MODES_COUNT)
        return;

    ModeData* cur_mode = &(mode_data[idx]);

    if (strcmp(key, "freq_min")==0)
        cur_mode->freq_min = atof(value);

    else if (strcmp(key, "freq_max")==0)
        cur_mode->freq_max = atof(value);

    else if (key[0]=='s' && key[1]=='f') {
        // Сбрасываем все значения
        for (int sf=MIN_SF_VALUE; sf<=MAX_SF_VALUE; sf++)
            mode_set_sf_bit(cur_mode, sf, 0);

        for (int i=0; value[i]; i++) {
            char sf = value[i] - '0';
            if (sf>=MIN_SF_VALUE && sf<=MAX_SF_VALUE) {
                mode_set_sf_bit(cur_mode, sf, 1);
            }
        }
    }
}


ModeData* mode_get(int idx)
{
    return &(mode_data[idx]);
}


#define SF_LIST_BUF_SIZE 20
static char sf_list_buf[SF_LIST_BUF_SIZE];

char* mode_get_sf_list(ModeData* mode, char  delimiter)
{
    int pos = 0;

    for (int sf=MIN_SF_VALUE; sf<=MAX_SF_VALUE; sf++) {
        if (mode_get_sf_bit(mode, sf)) {
            sf_list_buf[pos++] = '0'+sf;
            sf_list_buf[pos++] = delimiter;
        }
    }

    if (pos==0) pos=1;  // Если элементов не выводилось
    sf_list_buf[pos-1] = 0;

    return sf_list_buf;
}


void modes_update_map()
{
    for (int i=0; i<MODES_COUNT; i++) {        
        ModeData* mode = mode_get(i);
        char *buf = &(btn_modes_map[i][4]);
        sprintf(buf, "%4.0f-%4.0f", mode->freq_min, mode->freq_max);
    }
}


#define RES_BUF_SIZE ((MODE_STR_SIZE+1)*MODES_COUNT) 
static char res_buf[RES_BUF_SIZE] = "1/n2/n3/n";

// Формирование строки для DropDown
char* modes_get_dropdown_options()
{
    int pos = 0;

    for (int i=0; i<MODES_COUNT; i++) {        
        sprintf(&(res_buf[pos]), "%s", btn_modes_map[i]);
        pos = strlen(res_buf);

        if (pos >= RES_BUF_SIZE) break;

        res_buf[pos++] = '\n';
        res_buf[pos] = 0;
    }

    return res_buf;
}

#include <stdio.h>
#include "w_loading.h"

static char cmd_data[100];

/**
 * Подтверждение изменений для передачи на подавитель
 */
bool mode_commit(int idx)
{
    dlg("Сохранение", "Идет сохранение ...", 0);

    ModeData* mode = &(mode_data[idx]);
    sprintf(cmd_data, "mode:%d,freq_min:%.1f,freq_max:%.1f", idx+1, mode->freq_min, mode->freq_max, mode_get_sf_list(mode, '|'));

    int tries = 5;
    while (tries--) {
        link_send_cmd("set_mode", cmd_data);
        int reply = link_wait_reply(1000);
        if (reply == 0) {
            dlg("Сохранение", "Режим обновлен", DLG_BTN_CLOSE);
            dlg_set_color(LV_PALETTE_LIGHT_GREEN);

            return true;
        }
    }

    log("mode_commit error");
    dlg("Сохранение", "Ошибка при сохранении", DLG_BTN_CLOSE);
    dlg_set_color(LV_PALETTE_RED);

    return false;
}