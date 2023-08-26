//
// Created by Camleoah on 21/12/2022.
//

#pragma once

#include "Arduino.h"

#define RAM_LOG_RINGBUFFER_LEN                  30

typedef enum{
    RAM_LOG_INFO                        = 0x00,
    RAM_LOG_ERROR_WIFI_HANDLER          = 0x01,
    RAM_LOG_ERROR_GITHUB_UPDATE         = 0x02,
    RAM_LOG_ERROR_GPS_MANAGER           = 0x03
} RAM_LOG_ITEM_t;

typedef struct {
    double timestamp;
    RAM_LOG_ITEM_t item_type;
    String payload;
} ram_log_item_t;

/// adds an entry to the ramlog
/// \param itemtype what type of message
/// \param user_payload error code
void ram_log_notify(RAM_LOG_ITEM_t itemtype, uint32_t user_payload);

/// adds an entry to the ramlog that can contain a sting
/// \param itemtype what type of message
/// \param user_payload message string
/// \param flag_print when set to true, message string will also be printed in console
void ram_log_notify(RAM_LOG_ITEM_t itemtype, const char* user_payload, bool flag_print = false);

/// prints the entire ramlog and all entries nicely formatted
void ram_log_print_log();