#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "wifi_handler.h"
#include "github_update.h"
#include "webserial_monitor.h"
#include "../../../include/tools/loop_timer.h"

/*
 * This is the main file that needs to be implemented in a running project.
 * From here one can enable/disable adjacent modules such as a github updater, OTA functionality or webserial.
 * This file will create the WiFi and handle all modules that use WiFi
 */

String _ssid;
String _password;

IPAddress _local_IP;
IPAddress _gateway;
IPAddress _subnet;
IPAddress _primaryDNS(8, 8, 8, 8);
IPAddress _secondaryDNS(8, 8, 4, 4);

int timer_wifi_connect = 0;

AsyncWebServer server(80);

WIFI_HANDLER_ERROR_t wifi_handler_init(const char *user_ssid, const char *user_password, const char *user_ip,
                                       const char *user_gateway, const char *user_subnet, const char *url_version,
                                       const char *url_bin) {
    WIFI_HANDLER_ERROR_t retval = WIFI_HANDLER_ERROR_UNKNOWN;

    // get all the user data first
    _local_IP.fromString(user_ip);
    _gateway.fromString(user_gateway);
    _subnet.fromString(user_subnet);
    _ssid = user_ssid;
    _password = user_password;

    // establish connection
    if((retval = wifi_handler_connect()) != WIFI_HANDLER_ERROR_NO_ERROR)
        return retval;

    // initialize modules
#ifdef SYS_CONTROL_AUTO_UPDATE
    github_update_init(url_version, url_bin);
#endif

    webserial_monitor_init();


    return retval;
}

WIFI_HANDLER_ERROR_t wifi_handler_connect() {
    Serial.println("Waiting for WiFi");

#ifdef SYS_CONTROL_STAT_IP
    if (!WiFi.config(_local_IP, _gateway, _subnet, _primaryDNS, _secondaryDNS)) {
        Serial.println("Static IP failed to configure");
    }
#endif

    WiFi.begin(_ssid.c_str(), _password.c_str());
    while ((WiFi.status() != WL_CONNECTED)) {
        delay(500);
        Serial.print(".");
        timer_wifi_connect++;
        if (500 * timer_wifi_connect > TIMEOUT_WIFI_CONNECT_MS) {
            timer_wifi_connect = 0;
            return WIFI_HANDLER_ERROR_WIFI;
        }
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP-address: ");
    Serial.println(WiFi.localIP());

    return WIFI_HANDLER_ERROR_NO_ERROR;
}

bool wifi_handler_is_connected() {
    return WiFi.isConnected();
}