#include <stdio.h>

#include "debug.h"
#include "link.h"

#include "data_status.h"


static char cmd_buf[100];


void cmd_start(int mode_idx)
{
    sprintf(cmd_buf, "mode:%d", mode_idx);
    log(cmd_buf);
    link_send_cmd("start", cmd_buf);
}


void cmd_stop()
{
    log(cmd_buf);
    link_send_cmd("stop", NULL);
}

