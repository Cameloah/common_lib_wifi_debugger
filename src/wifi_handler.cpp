#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "wifi_handler.h"
#include "github_update.h"
#include "webserial_monitor.h"
#include "network_ota.h"
#include "wifi_manager.h"
#include "../../../include/tools/loop_timer.h"

/*
 * This is the main file that needs to be implemented in a running project.
 * From here one can enable/disable adjacent modules such as a github updater, OTA functionality or webserial.
 * This file will create the WiFi and handle all modules that use WiFi
 */


wifi_info_t _wifi_info_buffer;

int timer_wifi_connect = 0;
AsyncWebServer server(80);

WIFI_HANDLER_ERROR_t wifi_handler_init(const char *user_ssid, const char *user_password, const char *user_ip,
                                       const char *user_gateway, const char *user_subnet, const char *url_version,
                                       const char *url_bin) {
    WIFI_HANDLER_ERROR_t retval = WIFI_HANDLER_ERROR_UNKNOWN;

    // try to load wifi info from wifi manager
    if (wifi_manager_load(&_wifi_info_buffer)) {
        // we have data, therefore connect normally
        // establish connection
        if ((retval = wifi_handler_connect()) != WIFI_HANDLER_ERROR_NO_ERROR)
            return retval;
    }

    // otherwise we need an access point
    else wifi_manager_AP(&_wifi_info_buffer);

    // initialize modules
#ifdef SYS_CONTROL_AUTO_UPDATE
    github_update_init(url_version, url_bin);
#endif

#ifdef SYS_CONTROL_WEBSERIAL
    webserial_monitor_init();
#endif

    network_ota_init();

    server.begin();
    return retval;
}

void wifi_handler_update() {
    network_ota_update();
}

WIFI_HANDLER_ERROR_t wifi_handler_connect() {
    Serial.println("Waiting for WiFi");

#ifdef SYS_CONTROL_STAT_IP
    if (!WiFi.config(_local_IP, _gateway, _subnet, _primaryDNS, _secondaryDNS)) {
        Serial.println("Static IP failed to configure");
        return WIFI_HANDLER_ERROR_CONFIG;
    }
#endif

    WiFi.begin(_wifi_info_buffer._ssid.c_str(), _wifi_info_buffer._password.c_str());
    while ((WiFi.status() != WL_CONNECTED)) {
        delay(500);
        DualSerial.print(".");
        timer_wifi_connect++;
        if (500 * timer_wifi_connect > TIMEOUT_WIFI_CONNECT_MS) {
            timer_wifi_connect = 0;
            return WIFI_HANDLER_ERROR_CONNECT;
        }
    }

    DualSerial.println("");
    DualSerial.println("WiFi connected");
    DualSerial.println("IP-address: ");
    DualSerial.println(WiFi.localIP());

    return WIFI_HANDLER_ERROR_NO_ERROR;
}

bool wifi_handler_is_connected() {
    return WiFi.isConnected();
}