#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Ticker.h>          //Call the ticker. H Library

#include "ui.h"

extern TFT_eSPI* lcd;
extern uint16_t touchX, touchY;
extern uint16_t calData[5];
