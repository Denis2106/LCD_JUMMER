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
#include "styles.h"

#define LOADING_REMAIN 38
#include "w_loading.h"

#pragma message TOUCH_CS

void msg_mode_update(char **keys, char **values, int pairs_count)
{
    loading_end();

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
        if (strcmp(keys[i], "info") == 0) {
            add_info(values[i], true);
            loading_add_info(values[i]);
        }
    }
}


void reset()
{
    mode_clear();
    link_send_cmd("get_modes", NULL);
    delay(1000);
    tab_modes_load();
}


void msg_cmd_reset(char **keys, char **values, int pairs_count)
{
    loading_end();
    reset();
}

volatile unsigned long last_in_ping_ms;

void msg_cmd_ping(char **keys, char **values, int pairs_count)
{
    return;
    last_in_ping_ms = millis();

    if (loading_is_active())
        msg_cmd_reset(NULL, NULL, 0);
    else
        dlg_close();
}


void msg_cmd_status(char **keys, char **values, int pairs_count)
{
    msg_cmd_ping(keys, values, pairs_count);
}


void loading_tap()
{
    msg_cmd_reset(NULL, NULL, 0);
}


#define COMMAND_COUNT 5
CMD_ENTRY commands[COMMAND_COUNT] = {
    {"mode", NULL, msg_mode_update},
    {"info", NULL, msg_info},
    {"cmd", "reset", msg_cmd_reset},
//    {"cmd", "ping", msg_cmd_ping},
    {"cmd", "status", msg_cmd_status},
    {"cmdres", NULL, msg_cmdres},
};


#ifdef WIN_DEBUG
#define LINK_PORT "USB"
#define LINK_BOUD 115200
class HostLinkPort : public Port {
    public:
    void init() { }
    uint8_t read() { return Serial.read(); }
    bool is_available() { return Serial.available(); }
    void write(uint8_t data) { Serial.write(data); }
};
#else
#define LINK_PORT "UART1"
#define LINK_BOUD 19200
class HostLinkPort : public Port {
    public:
    void init() { Serial2.begin(LINK_BOUD); }
    uint8_t read() { return Serial2.read(); }
    bool is_available() { return Serial2.available(); }
    void write(uint8_t data) { while (!Serial2.availableForWrite()); Serial2.write(data); delay(1); }
};
#endif


static HostLinkPort host_link_port = HostLinkPort();


void setup()
{
  Serial.begin(115200);
  log_init(&Serial);

  host_link_port.init();
  link_init(&host_link_port, commands, COMMAND_COUNT, false);

  lcd_init();
  //ui_init();
  //scr_calibrate();
  //lcd.fillScreen(TFT_BLACK);

  build_screen();
  loading_build(loading_tap);

  log("Setup done", true);
  log("LINK_PORT: ", false); 
  log(LINK_PORT, LINK_BOUD);

  reset();
}


void wait_loop()
{
    link_process_incom();
    lv_timer_handler();
    loading_tick(millis());

    delay(5);
}


void loop()
{
    /*
    if (millis() - last_ping_ms > 1000) {
        last_ping_ms = millis();

        //char* txt = "ping";
        //Serial2.println(txt);
        //log(txt);
        //add_info(txt, true);
    }
    */
    //unsigned long lost_time = (millis()-last_in_ping_ms) / 1000;
    //if (last_in_ping_ms && lost_time > 5 ) { 
        //char buf[20];
        //sprintf(buf, "%d сек", lost_time);
        //dlg("Потеря связи", buf, DLG_BTN_CLOSE);
    //    log("Link_lost");
    //}

    wait_loop();
}