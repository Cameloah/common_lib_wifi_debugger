#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

#include "wifi_handler.h"
#include "webserial_monitor.h"
#include "network_ota.h"
#include "wifi_manager.h"
#include "memory_module.h"
#include "../../../include/tools/loop_timer.h"

/*
 * This is the main file that needs to be implemented in a running project.
 * From here one can enable/disable adjacent modules such as a github updater, OTA functionality or webserial.
 * This file will create the WiFi and handle all modules that use WiFi
 */

// object that will contain the credential data for the current wifi
MemoryModule wifi_config;
MemoryModule wifi_info;

int timer_wifi_connect = 0;
AsyncWebServer server(80);

WIFI_HANDLER_ERROR_t wifi_handler_init(const String& ap_name = "New ESP-Device", const String& device_name = "ESP-Device No. ") {
    WIFI_HANDLER_ERROR_t retval = WIFI_HANDLER_ERROR_UNKNOWN;

    // lets populate the wifi_config object with empty parameters
    wifi_config.addParameter("ssid");
    wifi_config.addParameter("password");
    wifi_config.addParameter("localIP");
    wifi_config.addParameter("gateway");

    // the wifi_info object will contain general information such as identification
    wifi_info.addParameter("APname");
    wifi_info.addParameter("deviceName");
    wifi_info.addParameter("subnet", (String) "255.255.255.0");
    wifi_info.addParameter("primaryDNS", (String) "8.8.8.8");
    wifi_info.addParameter("secondaryDNS", (String) "8.8.4.4");

    wifi_info.set("APname", ap_name);
    wifi_info.set("deviceName", device_name);

    //DualSerial.println(*static_cast<String*>(wifi_info.get("subnet")));
    //DualSerial.println(wifi_info.getString("APname"));
    //DualSerial.println(wifi_info.getString("deviceName"));



    // try to load wifi config and credentials from wifi manager
    if ((retval = wifi_manager_load(&wifi_config)) == WIFI_HANDLER_ERROR_NO_ERROR) {
        // we have data, therefore connect normally
        String output = "Loaded WiFi: '" + wifi_config.getString("ssid") + "'\n" +
                "With Password: '" + wifi_config.getString("password") + "'\n" +
                "IP-Address: " + wifi_config.getString("localIP") + "\n";
        DualSerial.println(output);

        // redefine device name to add a unique number
        std::string device_ip = wifi_config.getString("localIP").c_str();
        std::string identifier = device_ip.substr(device_ip.rfind('.') + 1);

        wifi_info.getString("deviceName").concat(identifier.c_str());

        // establish connection, spawn AP anyway if wanted
        if ((retval = wifi_handler_connect()) == WIFI_HANDLER_ERROR_CONNECT || AP_VERBOSITY == 2)
            // we couldn't connect, use AP
            if((retval = wifi_manager_AP(wifi_config.getString("APname"))) != WIFI_HANDLER_ERROR_NO_ERROR) return retval;

    }

    // if no config we need an access point
    else if (retval == WIFI_HANDLER_ERROR_CONFIG) {
        if((retval = wifi_manager_AP(wifi_info.getString("APname"))) != WIFI_HANDLER_ERROR_NO_ERROR) return retval;
    }

    else return retval;


    // *****----- SERVER MODULES -----*****

    // TODO: put all server services in extra function to also call properly in main loop
    // Systems that do not need SPIFFS
#ifdef SYS_CONTROL_WEBSERIAL
    webserial_monitor_init();
#endif

    network_ota_init();

    // now lets initialize spiffs
    if(!SPIFFS.begin(true)){
        return WIFI_HANDLER_ERROR_SPIFFS;
    }

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

    // if we are in access point mode we don't look for Wi-Fi
    if (WiFi.getMode() == 2)
        return WIFI_HANDLER_ERROR_CONNECT;

    DualSerial.println("Waiting for WiFi");

#ifdef SYS_CONTROL_STAT_IP

    IPAddress local_IP;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress primaryDNS;
    IPAddress secondaryDNS;

    local_IP.fromString(wifi_config.getString("localIP"));
    gateway.fromString(wifi_config.getString("gateway"));
    subnet.fromString(wifi_info.getString("subnet"));
    primaryDNS.fromString(wifi_info.getString("primaryDNS"));
    secondaryDNS.fromString(wifi_info.getString("secondaryDNS"));

    DualSerial.println("Using static IP...");

    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
        DualSerial.println("Static IP failed to configure");
        return WIFI_HANDLER_ERROR_CONFIG;
    }
#endif

    WiFi.begin(wifi_config.getString("ssid").c_str(), wifi_config.getString("password").c_str());

    while ((WiFiClass::status() != WL_CONNECTED)) {
        delay(500);
        DualSerial.print(".");
        timer_wifi_connect++;
        if (500 * timer_wifi_connect > TIMEOUT_WIFI_CONNECT_MS) {
            timer_wifi_connect = 0;
            return WIFI_HANDLER_ERROR_CONNECT;
        }
    }

#ifdef SYS_CONTROL_STAT_IP
    if(wifi_config.getString("localIP") != WiFi.localIP().toString()) {
        DualSerial.println("Static IP failed to configure");
        return WIFI_HANDLER_ERROR_CONFIG;
    }
#endif

    DualSerial.println("");
    DualSerial.println("WiFi connected");
    DualSerial.println("IP-address: ");
    DualSerial.println(WiFi.localIP());

#ifndef SYS_CONTROL_STAT_IP
    // if we got the IP from the DNS, lets save the ip in the memory buffer
    // wifi_config.set("localIP", WiFi.localIP().toString());
#endif

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