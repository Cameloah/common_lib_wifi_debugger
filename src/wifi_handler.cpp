#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

#include "wifi_handler.h"
#include "webserial_monitor.h"
#include "network_ota.h"
#include "wifi_manager.h"
#include "../../../include/tools/loop_timer.h"

/*
 * This is the main file that needs to be implemented in a running project.
 * From here one can enable/disable adjacent modules such as a github updater, OTA functionality or webserial.
 * This file will create the WiFi and handle all modules that use WiFi
 */

// object that will contain the credential data for the current wifi
wifi_info_t wifi_info_buffer;

int timer_wifi_connect = 0;
AsyncWebServer server(80);

WIFI_HANDLER_ERROR_t wifi_handler_init() {
    WIFI_HANDLER_ERROR_t retval = WIFI_HANDLER_ERROR_UNKNOWN;

    // try to load wifi info from wifi manager
    if ((retval = wifi_manager_load(&wifi_info_buffer)) == WIFI_HANDLER_ERROR_NO_ERROR) {
        // we have data, therefore connect normally
        // establish connection
        if ((retval = wifi_handler_connect()) == WIFI_HANDLER_ERROR_CONNECT)
            if((retval = wifi_manager_AP()) != WIFI_HANDLER_ERROR_NO_ERROR) return retval; // we couldnt connect, use AP

    }
    // if no config we need an access point
    else if (retval == WIFI_HANDLER_ERROR_CONFIG) {
        if((retval = wifi_manager_AP()) != WIFI_HANDLER_ERROR_NO_ERROR) return retval;
    }

    else return retval;

    // initialize modules
#ifdef SYS_CONTROL_WEBSERIAL
    webserial_monitor_init();
#endif

    network_ota_init();

    server.serveStatic("/", SPIFFS, "/");
    server.on("/wifi", HTTP_GET, webfct_wifi_get);
    server.on("/", HTTP_POST, webfct_wifi_post);

    server.begin();
    return retval;
}

void wifi_handler_update() {
    network_ota_update();
    wifi_manager_update();
}

WIFI_HANDLER_ERROR_t wifi_handler_connect() {
    Serial.println("Waiting for WiFi");

#ifdef SYS_CONTROL_STAT_IP
    if (!WiFi.config(wifi_info_buffer._local_IP, wifi_info_buffer._gateway, wifi_info_buffer._subnet, wifi_info_buffer._primaryDNS, wifi_info_buffer._secondaryDNS)) {
        Serial.println("Static IP failed to configure");
        return WIFI_HANDLER_ERROR_CONFIG;
    }
#endif

    WiFi.begin(wifi_info_buffer._ssid.c_str(), wifi_info_buffer._password.c_str());
    while ((WiFiClass::status() != WL_CONNECTED)) {
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

String wifi_handler_get_mode() {
    switch (WiFi.getMode()) {
        case 1:
            return "STA Mode";
        case 2:
            return "AP Mode";
        case 3:
            return "STA and AP Mode";
        default:
            return "unknown";
    }
}