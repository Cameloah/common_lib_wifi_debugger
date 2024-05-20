//
// Created by Cameloah on 31.03.2023.
// The wifi manager provides an interface to setup and configure the wifi network, the esp is connecting to. 
// In the future, it should be able to store multipe sets of wifi credentials, show available networks and give the option for static or dynamic IP addresses. 
//

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include <DNSServer.h>
#include "EEPROM.h"

#include "wifi_manager.h"
#include "wifi_handler.h"
#include "webserial_monitor.h"
#include "ram_log.h"
#include "memory_module.h"
#include "lib_tools.h"

const byte DNS_PORT = 53;
DNSServer dnsServer;

// Search for parameter in HTTP POST request
const char* GENERAL_INPUT_1 = "Device Name";

const char* WIFI_INPUT_1 = "ssid";
const char* WIFI_INPUT_2 = "pass";
const char* WIFI_INPUT_3 = "ip";
const char* WIFI_INPUT_4 = "gateway";

String ip_default = "192.168.2.73";

bool flag_ap_active = false;
bool flag_wifi_config_loaded = false;

void webfct_wifi_get(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/wifimanager.html", "text/html");
}

void webfct_wifi_post(AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
            // save general settings
            if (p->name() == GENERAL_INPUT_1) {
                wifi_info.set("deviceName", p->value(), true);
                DualSerial.print("Device name set to: ");
                DualSerial.println(p->value());
            }
            // save wifi settings
            if (p->name() == WIFI_INPUT_1) {
                wifi_config.set("ssid", p->value(), true);
                DualSerial.print("SSID set to: ");
                DualSerial.println(p->value());
            }
    
            if (p->name() == WIFI_INPUT_2) {
                wifi_config.set("password", p->value(), true);
                DualSerial.print("Password set to: ");
                DualSerial.println(p->value());
            }
            
            if (p->name() == WIFI_INPUT_3) {
                wifi_config.set("localIP", p->value(), true);
                DualSerial.print("IP Address set to: ");
                DualSerial.println(p->value());
            }

            if (p->name() == WIFI_INPUT_4) {
                wifi_config.set("gateway", p->value(), true);
                DualSerial.print("Gateway set to: ");
                DualSerial.println(p->value());
            }
        }
    }
    request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + wifi_config.getString("localIP"));
    delay(3000);
    ESP.restart();
}

WIFI_HANDLER_ERROR_t wifi_manager_load(MemoryModule* user_config) {

    // Load values from NVS
    if (user_config->loadAllStrict() != ESP_OK)
        return WIFI_HANDLER_ERROR_CONFIG;

    // lets check for meaningful credentials
    if(!otherThanWhitespace(user_config->getString("ssid")) || !otherThanWhitespace(user_config->getString("password")))
        return WIFI_HANDLER_ERROR_CONFIG;

    flag_wifi_config_loaded = true;
    return WIFI_HANDLER_ERROR_NO_ERROR;
}

//TODO: does not belong here. move to wifi handler
WIFI_HANDLER_ERROR_t wifi_manager_AP(const String& ap_name) {
    // Connect to Wi-Fi network with SSID and password
    String payload = "Starting Access Point: " + ap_name;
    ram_log_notify(RAM_LOG_INFO, payload.c_str(), true);
    // nullptr sets an open Access Point

    WiFi.enableAP(true);
    if(!WiFi.softAP(ap_name.c_str(), AP_PASSWORD))
        return WIFI_HANDLER_ERROR_AP;

    payload = "AP active. IP: " + WiFi.softAPIP().toString() + ", Wifi mode: " + wifi_handler_get_mode();
    ram_log_notify(RAM_LOG_INFO, payload.c_str(), true);

    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request

    //TODO: dns server causes crash on esp32 devkit v1
    // dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    // Web Server Root URL
    server.onNotFound(webfct_wifi_get);

    flag_ap_active = true;
    return WIFI_HANDLER_ERROR_NO_ERROR;
}

void wifi_manager_update() {
    if (flag_ap_active)
        return;
        //dnsServer.processNextRequest();

#ifdef AP_TIMEOUT
    if (flag_ap_active && millis() > AP_TIMEOUT) {
        // Mr. Gorbatschow, tear down that Access Point
        ram_log_notify(RAM_LOG_INFO, "Stopping access point", true);
        dnsServer.stop();
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        flag_ap_active = false;
        wifi_handler_connect();
    }
#endif

}