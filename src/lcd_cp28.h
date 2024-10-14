#include <lvgl.h>
#include <TFT_eSPI.h>


extern const uint16_t screenWidth;
extern const uint16_t screenHeight;

extern TFT_eSPI lcd;

void lcd_init();
void scr_calibrate();