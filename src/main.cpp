#include <lvgl.h>
#include <demos/lv_demos.h>
#include "ui.h"

#include "debug.h"
#include "port.h"
#include "link.h"
#include "lcd_cp28.h"
#include "scr_calibrate.h"
#include "scr_test.h"
#include "data_mode.h"
#include "tab_info.h"

#pragma message TOUCH_CS

void msg_mode_update(char **keys, char **values, int pairs_count)
{
    log("msg mode pairs=", pairs_count);
    int mode_idx = atoi(values[0]);

    if (! mode_idx) return;

    mode_idx--;

    for (int i=1; i<pairs_count; i++) {
        mode_update(mode_idx, keys[i], values[i]);
    }

    tab_modes_load();
}


void msg_info(char **keys, char **values, int pairs_count)
{
    for (int i=0; i<pairs_count; i++) {
        if (strcmp(keys[i], "info") == 0)
            add_info(values[i], true);
    }
}


void msg_cmd_reset(char **keys, char **values, int pairs_count)
{
    mode_clear();
    tab_modes_load();
    link_send_cmd("cmd:get_modes");
}


#define COMMAND_COUNT 3
CMD_ENTRY commands[COMMAND_COUNT] = {
    {"mode", NULL, msg_mode_update},
    {"info", NULL, msg_info},
    {"cmd", "reset", msg_cmd_reset}
};


#ifdef WIN_DEBUG
#define LINK_BOUD 115200
class HostLinkPort : public Port {
    public:
    void init() { }
    uint8_t read() { return Serial.read(); }
    bool is_available() { return Serial.available(); }
    void write(uint8_t data) { Serial.write(data); }
};
#else
#define LINK_BOUD 9600
class HostLinkPort : public Port {
    public:
    void init() { Serial2.begin(LINK_BOUD); }
    uint8_t read() { return Serial2.read(); }
    bool is_available() { return Serial2.available(); }
    void write(uint8_t data) { Serial2.write(data); }
};
#endif


static HostLinkPort host_link_port = HostLinkPort();


void setup()
{
  Serial.begin(LINK_BOUD);
  log_init(&Serial);

  host_link_port.init();
  link_init(&host_link_port, commands, COMMAND_COUNT, false);

  lcd_init();
  //ui_init();
  //scr_calibrate();
  //lcd.fillScreen(TFT_BLACK);

  scr_test();

  log( "Setup done", true);
}


int last_ping_ms;


void loop()
{
    if (millis() - last_ping_ms > 1000) {
        last_ping_ms = millis();

        //char* txt = "ping";
        //Serial2.println(txt);
        //add_info(txt, true);
    
        while (false && host_link_port.is_available()) {
            char buf[2];
            buf[0] = host_link_port.read();
            buf[1] = 0;
            log(buf, false);

            //add_info(buf, false);
        }

    }

    link_process_incom();

    lv_timer_handler();
    delay(5);
}