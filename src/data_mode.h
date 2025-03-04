#include <stdint.h>

#define MODES_COUNT 8
#define MODE_STR_SIZE 30

typedef struct {
    float freq_min;
    float freq_max;
    uint8_t sf_bits;
} ModeData;

#define MIN_SF_VALUE 6
#define MAX_SF_VALUE 9

extern int mode_count;
extern char btn_modes_map[MODES_COUNT][MODE_STR_SIZE];

void mode_clear();
ModeData* mode_get(int idx);
int mode_get_sf_bit(ModeData* mode, int sf);
void mode_set_sf_bit(ModeData* mode, int sf, int value);
char* mode_get_sf_list(ModeData* mode, char  delimiter);
void mode_update(int idx, char* key, char* value);
bool mode_commit(int idx);

char* modes_get_dropdown_options();
void modes_update_map();
