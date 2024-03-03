//
// Created by Cameloah on 03.03.2022.
//

#pragma once

#include "ESPAsyncWebServer.h"
#include "webserial_monitor.h"
#include "memory_module.h"

#define SYS_CONTROL_STAT_IP
#define SYS_CONTROL_WEBSERIAL

// set this to any password to protect the AP
#define AP_PASSWORD                     nullptr

// AP verbosity:
// 1 - AP will spawn if esp cannot connect to existing wifi for any reason
// 2 - AP will always spawn, even if connected to existing wifi
#define AP_VERBOSITY                    1

// comment this out to have an always on AP once its spawned
// #define AP_TIMEOUT                      300000      // 5 min

#define TIMEOUT_WIFI_CONNECT_MS         5000

extern AsyncWebServer server;
extern MemoryModule wifi_config;

typedef enum{
    WIFI_HANDLER_ERROR_NO_ERROR        = 0x00,
    WIFI_HANDLER_ERROR_CONFIG          = 0x01,
    WIFI_HANDLER_ERROR_CONNECT         = 0x02,
    WIFI_HANDLER_ERROR_SPIFFS          = 0x03,
    WIFI_HANDLER_ERROR_AP              = 0x04,
    WIFI_HANDLER_ERROR_UNKNOWN         = 0xFF
} WIFI_HANDLER_ERROR_t;

/// Initializes wifi, tries to connect to saved wifi, will setup an access point to input desired
/// wifi credentials otherwise. activates modules such as github update etc
/// \param url_version url to latest github release
/// \param url_bin url to download latest bin file from github
/// \return error code
WIFI_HANDLER_ERROR_t wifi_handler_init(const String& ap_name, const String& device_name);

/// needs to be run periodically to update services
void wifi_handler_update();

/// tries to connect to wifi
/// \return error code
WIFI_HANDLER_ERROR_t wifi_handler_connect();

/// Getter to get wifi connection state
/// \return boolean success
bool wifi_handler_is_connected();

/// Getter to get formatted wifi module mode
/// \return String
String wifi_handler_get_mode();
