#include <stdint.h>

typedef struct {
    float freq_min;
    float freq_max;
    uint8_t sf_bits;
} ModeData;

#define MAX_MODE_COUNT 10
#define MIN_SF_VALUE 6
#define MAX_SF_VALUE 9

extern int mode_count;

ModeData* mode_get(int idx);
int mode_get_sf_bit(ModeData* mode, int sf);
void mode_set_sf_bit(ModeData* mode, int sf, int value);
char* mode_get_sf_list(ModeData* mode, char  delimiter);
void mode_update(int idx, char* key, char* value);
void mode_commit(int idx);

char* modes_get_dropdown_options(bool include_off=false);
