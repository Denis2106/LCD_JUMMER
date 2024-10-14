#include "debug.h"
#include "port.h"

/**
 * Структура описания команд для вызова из обработчика сообщений
 */
typedef struct {
    char* attr;
    char* value;
    void (*func)(char **keys, char **values, int key_count);
} CMD_ENTRY;


void link_init(Port* link_port, CMD_ENTRY *commands, int command_count, bool check_crc=true);

int link_process_incom();

void link_send_cmd(char *cmd);