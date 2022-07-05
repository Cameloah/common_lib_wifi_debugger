//
// Created by koorj on 03.03.2022.
//

#pragma once

typedef enum{
    WIFI_DEBUGGER_ERROR_NO_ERROR        = 0x00,
    WIFI_DEBUGGER_ERROR_WIFI            = 0x01,
    WIFI_DEBUGGER_ERROR_HTTP            = 0x02,
    WIFI_DEBUGGER_ERROR_NO_UPDATE       = 0x02,
    WIFI_DEBUGGER_ERROR_UNKNOWN         = 0xFF
} WIFI_DEBUGGER_ERROR_t;

WIFI_DEBUGGER_ERROR_t connect_wifi(const char* ssid, const char* password);
WIFI_DEBUGGER_ERROR_t wifi_debugger_firmwareUpdate();
WIFI_DEBUGGER_ERROR_t wifi_debugger_firmwareUpdate(const char *desired_version);
WIFI_DEBUGGER_ERROR_t wifi_debugger_fwVersionCheck(uint8_t fw_major, uint8_t fw_minor, uint8_t fw_patch);

WIFI_DEBUGGER_ERROR_t wifi_debugger_init(const char* user_ssid, const char* user_password, const char* url_version, const char* url_bin);
