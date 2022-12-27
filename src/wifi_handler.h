//
// Created by koorj on 03.03.2022.
//

#pragma once

#include "ESPAsyncWebServer.h"
#include "webserial_monitor.h"

#define SYS_CONTROL_STAT_IP
#define SYS_CONTROL_AUTO_UPDATE
#define SYS_CONTROL_WEBSERIAL

#define TIMEOUT_WIFI_CONNECT_MS         5000

extern AsyncWebServer server;

typedef enum{
    WIFI_HANDLER_ERROR_NO_ERROR        = 0x00,
    WIFI_HANDLER_ERROR_WIFI            = 0x01,
    WIFI_HANDLER_ERROR_HTTP            = 0x02,
    WIFI_HANDLER_ERROR_NO_UPDATE       = 0x03,
    WIFI_HANDLER_ERROR_UNKNOWN         = 0xFF
} WIFI_HANDLER_ERROR_t;

WIFI_HANDLER_ERROR_t wifi_handler_init(const char *user_ssid, const char *user_password, const char *user_ip, const char *user_gateway,
                                       const char *user_subnet, const char *url_version, const char *url_bin);

WIFI_HANDLER_ERROR_t wifi_handler_connect();
bool wifi_handler_is_connected();
