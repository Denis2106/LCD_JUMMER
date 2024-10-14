#include "link.h"
#include "debug.h"

#define SERIAL_BUF_LEN 150
char serial_buf[SERIAL_BUF_LEN];
int serial_write_pos = 0;


Port *_link_port;
bool _check_crc;
CMD_ENTRY* _commands;
int _command_count;

/**
 * Инициализация модуля обработки сообщений
 */
void link_init(Port *link_port, CMD_ENTRY *commands, int command_count, bool check_crc)
{
    _link_port = link_port;
    _commands = commands;
    _command_count = command_count;
    _check_crc = check_crc;
}


/**
 * Глобальный буфер элементов сообщения
 */
#define MAX_PAIRS_COUNT 20
char *key_pos[MAX_PAIRS_COUNT];     // Массив указаметей на начало ключений
char *val_pos[MAX_PAIRS_COUNT];     // Массив указателей на начало значений
int pairs_count;


/**
 * Выделяет сообщение из переданных данных.
 * размещает ссылки на элементы сообщения в глобальном буфере
 */
void link_parse_msg(char *data, int data_size)
{
    enum PARSE_STATE {
        PS_LOOK_MSG_START,
        PS_LOOK_KEY_START,
        PS_LOOK_KEY_END,
        PS_LOOK_VAL_START,
        PS_LOOK_VAL_END
    };

    PARSE_STATE stage = PS_LOOK_MSG_START;  
    // Этапы 
    // 0 - поиск начала сообщения
    // 1 - поиск начала ключа
    // 2 - поиск конца ключа
    // 3 - поиск начала значения
    // 4 - поиск конца значения
    int pos = 0;   // № позиции в исходной строке
    pairs_count = 0;

    while (1) {
        char ch = data[pos]; 
        //log("parsing: ", ch);

        // Обнаружение начала сообщения
        if (stage == PS_LOOK_MSG_START && ch == '{') {
            pairs_count = 0;
            stage = PS_LOOK_KEY_START;
        }

        // Поиск начала ключа
        else if (stage == PS_LOOK_KEY_START && ch !=' ') {
            key_pos[pairs_count] = &(data[pos]);
            stage = PS_LOOK_KEY_END;
        }

        // Поиск конца ключа
        else if (stage == PS_LOOK_KEY_END && ch == ':') {
            data[pos] = 0;
            stage = PS_LOOK_VAL_START;
        }

        // Поиск начала значения
        else if (stage == PS_LOOK_VAL_START && ch != ' ') {
            val_pos[pairs_count++] =&(data[pos]);
            stage = PS_LOOK_VAL_END;
        }

        // Поиск конца значения
        else if (stage == PS_LOOK_VAL_END && ch == ',' || ch == '}') {
            data[pos] = 0;
            stage = PS_LOOK_KEY_START;
        }

        // Завершение парсинга
        if (pairs_count == MAX_PAIRS_COUNT || pos == data_size || ch == '}' || ch == 0) 
            break;

        pos++;
    }    

    // Принудительное завершение строки последнего ключа или значения
    data[pos] = 0;
}


/**
 * Обрабатывает сообщение в глобальном буфере
 *  - находит обработчик команды и вызывает его
 */
void link_process_msg()
{
    for (int i=0; i<_command_count; i++) {
        if (strcmp(key_pos[0], _commands[i].attr)==0) {
            if (_commands[i].value == NULL || strcmp(_commands[i].value, val_pos[0])==0)
                _commands[i].func(key_pos, val_pos, pairs_count);
                return;
        }
    }

    log("Undefined msg key=", key_pos[0]);
}


uint8_t link_calc_crc(char *buf, int size) 
{
    uint8_t res = 0;
    for (int i=0; i<size; i++)
        res ^= buf[i];

    return res;
}

/**
 * Обрабатывает данные поступающие по последовательному порту
 * Возвращает 1, если данные получены и обработаны
 */
int link_process_incom()
{
    int t_end = millis() + 100;

    while (_link_port->is_available() && millis() < t_end) {
        char ch = _link_port->read();
        if (ch < ' ') continue;
        // Эхо вывод, для отладки
        _link_port->write(ch);
        //char buf[2];
        //buf[0] = ch;
        //buf[1] = 0;
        //log(buf, false);

        if (ch == '{') serial_write_pos = 0;

        serial_buf[serial_write_pos++] = ch;

        // Принудительно заканчиваем прием если буфер заполнен
        if (serial_write_pos >= SERIAL_BUF_LEN-1) {
            serial_buf[SERIAL_BUF_LEN-1] = 0;
            log("To long message: ", serial_buf);

            ch = '}';
            serial_buf[SERIAL_BUF_LEN-2] = ch;
        }

        if (ch == '}') {
            uint8_t crc = _check_crc ? _link_port->read() : 0;
            log("serial_buf=", false);
            log(serial_buf);

            if (crc == link_calc_crc(serial_buf, serial_write_pos) || !_check_crc) {
                log("serial_write_pos=", serial_write_pos);
                //log_hex(serial_buf, serial_write_pos);
                link_parse_msg(serial_buf, serial_write_pos);
                log("pairs_count ", pairs_count);
                for (int i=0; i<pairs_count; i++)
                    log_kv("  ", key_pos[i], val_pos[i]);

                if (pairs_count == 0) return 1;

                link_process_msg();
                serial_write_pos = 0;
                return 1;
            } else {
                log("Bad CRC ", serial_buf);
            }
        }
    }

    return 0;
}


void link_send_cmd(char *cmd)
{
    int pos = 0;
    while (cmd[pos]) {
        char ch = cmd[pos++];
        _link_port->write(ch);
    }
}
