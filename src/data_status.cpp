#include <stdio.h>

#include "debug.h"
#include "link.h"

#include "data_status.h"


static char cmd_buf[100];


void cmd_start(int mode_idx)
{
    sprintf(cmd_buf, "{cmd:start,mode:%d}", mode_idx);
    log(cmd_buf);
    link_send_cmd(cmd_buf);
}


void cmd_stop()
{
    sprintf(cmd_buf, "{cmd:stop}");
    log(cmd_buf);
    link_send_cmd(cmd_buf);
}


void cmd_txgain(int gain)
{
    sprintf(cmd_buf, "{cmd:txgain,gain:%d}", gain);
    log(cmd_buf);
//    link_send_cmd(cmd_buf);
}