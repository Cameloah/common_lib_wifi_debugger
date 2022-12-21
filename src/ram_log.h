//
// Created by Jo Uni on 21/12/2022.
//

#pragma once

#include "Arduino.h"

#define RAM_LOG_RINGBUFFER_LEN                  30
#define RAM_LOG_PAYLOAD_SIZE                    4

typedef enum{
    RAM_LOG_INFO                        = 0x00,
} RAM_LOG_ITEM_t;

typedef struct {
    double timestamp;
    RAM_LOG_ITEM_t item_type;
    String payload;
} ram_log_item_t;

void ram_log_init();

void ram_log_notify(RAM_LOG_ITEM_t itemtype, uint32_t user_payload);
void ram_log_notify(RAM_LOG_ITEM_t itemtype, const char* user_payload);
void ram_log_print_log();