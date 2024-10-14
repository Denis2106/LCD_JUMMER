#include <string.h>

#include "data_mode.h"
#include "link.h"

static ModeData mode_data[MAX_MODE_COUNT] = {
    {.freq_min=700, .freq_max=750, .sf_bits=0b00001111},
    {.freq_min=710, .freq_max=740, .sf_bits=0b00000011},
    {.freq_min=720, .freq_max=730, .sf_bits=0b00001000},
};
int mode_count=3;

#define SF_TO_BIT(sf) (1<<(sf-MIN_SF_VALUE))

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
            //sf_list_buf[pos++] = ','; 
            sf_list_buf[pos++] = delimiter;
        }
    }

    if (pos==0) pos=1;  // Если элементов не выводилось
    sf_list_buf[pos-1] = 0;

    return sf_list_buf;
}


#include <stdio.h>

static char tx_cmd_buf[100];

/**
 * Подтверждение изменений для передачи на подавитель
 */
void mode_commit(int idx)
{
    ModeData* mode = &(mode_data[idx]);
    sprintf(tx_cmd_buf, "{cmd:set_mode,mode:%d,freq_min:%.1f,freq_max:%.1f,sf:%s}", idx, mode->freq_min, mode->freq_max, mode_get_sf_list(mode, '|'));
    log(tx_cmd_buf);
    link_send_cmd(tx_cmd_buf);
}