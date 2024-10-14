#include "lcd_cp28.h"

//Пины подключения дисплея 2.8
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK 18
#define SD_CS 5


const uint16_t screenWidth  = 320;
const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[ screenWidth * screenHeight / 10 ];

TFT_eSPI lcd = TFT_eSPI(); 


void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );

  lcd.startWrite();
  lcd.setAddrWindow( area->x1, area->y1, w, h );
  lcd.pushColors( ( uint16_t * )&color_p->full, w * h, true );
  lcd.endWrite();

  lv_disp_flush_ready( disp );
}

uint16_t touchX, touchY;

void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
  bool touched = lcd.getTouch( &touchX, &touchY, 600);
  if ( !touched )
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;

    data->point.x = touchX;
    data->point.y = touchY;

    //Serial.print( "Data x " );
    //Serial.println( touchX );

    //Serial.print( "Data y " );
    //Serial.println( touchY );
  }
}

uint16_t calData[5] = { 189, 3416, 359, 3439, 1 };

void lcd_init()
{
  lcd.begin();          
  lcd.setRotation(1);  
  lcd.fillScreen(TFT_BLACK);
  delay(200);

  pinMode(27, OUTPUT);
  digitalWrite(27, HIGH);
  delay(100);

  lcd.setTouch( calData );

  lv_disp_draw_buf_init( &draw_buf, buf1, NULL, screenWidth * screenHeight / 10 );

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register( &disp_drv );

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register( &indev_drv );
}

