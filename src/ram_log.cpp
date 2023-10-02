//
// Created by Cameloah on 21/12/2022.
//

#include "ram_log.h"
#include "webserial_monitor.h"

ram_log_item_t _ringbuffer[RAM_LOG_RINGBUFFER_LEN];

uint8_t _ringbuffer_item_index = 0;
uint8_t _ringbuffer_item_count = 0;


void ram_log_notify(RAM_LOG_ITEM_t item_type, uint32_t user_payload) {
    // save timestamp
    _ringbuffer[_ringbuffer_item_index].timestamp = millis();
    // save item type
    _ringbuffer[_ringbuffer_item_index].item_type = item_type;
    // save payload as string
    _ringbuffer[_ringbuffer_item_index].payload = String(user_payload);

    // increase item index
    _ringbuffer_item_index = (_ringbuffer_item_index + 1) % RAM_LOG_RINGBUFFER_LEN;
    _ringbuffer_item_count = (_ringbuffer_item_count > RAM_LOG_RINGBUFFER_LEN) ? RAM_LOG_RINGBUFFER_LEN : _ringbuffer_item_count+1;
}

void ram_log_notify(RAM_LOG_ITEM_t item_type, const char *user_payload, bool flag_print) {
    // save timestamp
    _ringbuffer[_ringbuffer_item_index].timestamp = millis();
    // save item type
    _ringbuffer[_ringbuffer_item_index].item_type = item_type;
    // save payload as string
    _ringbuffer[_ringbuffer_item_index].payload = user_payload;
    // if wanted, also print string
    if (flag_print) DualSerial.println(user_payload);

    // increase item index
    _ringbuffer_item_index = (_ringbuffer_item_index + 1) % RAM_LOG_RINGBUFFER_LEN;
    _ringbuffer_item_count = (_ringbuffer_item_count > RAM_LOG_RINGBUFFER_LEN) ? RAM_LOG_RINGBUFFER_LEN : _ringbuffer_item_count+1;
}

void ram_log_print_log() {
    DualSerial.printf("Log items: %d\n", _ringbuffer_item_count);

    for (int item = 0; item < _ringbuffer_item_count; ++item) {
        unsigned long int seconds = _ringbuffer[item].timestamp / 1000;
        uint16_t ms = (long int) _ringbuffer[item].timestamp % 1000;
        uint8_t sec = seconds % 60;		seconds /= 60;
        uint8_t min = seconds % 60;		seconds /= 60;
        uint8_t hrs = seconds % 24;     seconds /= 24;
        uint16_t d = seconds;
        DualSerial.printf("%dd:%dh:%dm:%ds:%dms ", d, hrs, min, sec, ms);
        if (_ringbuffer[item].item_type == RAM_LOG_INFO)
            DualSerial.print("INFO: ");
        else
            DualSerial.printf("ERROR: %d - ", _ringbuffer[item].item_type);
        DualSerial.println(_ringbuffer[item].payload.c_str());
        delay(500);
    }
}

String ram_log_time_str(unsigned long int sys_ms) {
    unsigned long int seconds = sys_ms / 1000;
    uint16_t ms = sys_ms % 1000;
    uint8_t sec = seconds % 60;		seconds /= 60;
    uint8_t min = seconds % 60;		seconds /= 60;
    uint8_t hrs = seconds % 24;     seconds /= 24;
    uint16_t d = seconds;
    String time_str = String(d) + "d:" + String(hrs) + "h:" + String(min) + "m:" + String(sec) + "s:" + String(ms) + "ms";
    return time_str;
}