//
// Created by koorj on 03.03.2022.
//

#pragma once

#define SYS_CONTROL_STAT_IP

#define TIMEOUT_WIFI_CONNECT_MS         5000

typedef enum{
    WIFI_DEBUGGER_ERROR_NO_ERROR        = 0x00,
    WIFI_DEBUGGER_ERROR_WIFI            = 0x01,
    WIFI_DEBUGGER_ERROR_HTTP            = 0x02,
    WIFI_DEBUGGER_ERROR_NO_UPDATE       = 0x03,
    WIFI_DEBUGGER_ERROR_UNKNOWN         = 0xFF
} WIFI_DEBUGGER_ERROR_t;

WIFI_DEBUGGER_ERROR_t connect_wifi();
WIFI_DEBUGGER_ERROR_t wifi_debugger_firmwareUpdate();
WIFI_DEBUGGER_ERROR_t wifi_debugger_firmwareUpdate(const char *desired_version);
WIFI_DEBUGGER_ERROR_t wifi_debugger_fwVersionCheck(uint8_t fw_major, uint8_t fw_minor, uint8_t fw_patch);
bool wifi_debugger_is_connected();

WIFI_DEBUGGER_ERROR_t wifi_debugger_init(const char *user_ssid, const char *user_password, const char *user_ip, const char *user_gateway,
                                         const char *user_subnet, const char *url_version, const char *url_bin);
