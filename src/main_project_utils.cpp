#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

#include "main_project_utils.h"
#include "webserial_monitor.h"
#include "network_ota.h"
#include "network_manager.h"
#include "memory_module.h"
#include "../../../include/tools/loop_timer.h"

/*
 * This is the main file that needs to be implemented in a running project.
 * From here one can enable/disable adjacent modules such as a github updater, OTA functionality or webserial.
 */


// -------------- server -------------- //

AsyncWebServer server(80);



// ---------- http parameters --------- //

const char* GENERAL_INPUT_1 = "Device Name";

const char* WIFI_INPUT_1 = "ssid";
const char* WIFI_INPUT_2 = "pass";
const char* WIFI_INPUT_3 = "ip";
const char* WIFI_INPUT_4 = "gateway";



// ---------- web functions ----------- //

void webfct_get_wifi_cred(AsyncWebServerRequest *request) {
    String pw = wifi_config.getString("password");

    // hide the password if access point is up. TODO: only hide when request comes via access point, make sure browser cannot send only asterix pw
    if (WiFi.getMode() != 1)
        pw = "**********";

    String payload = wifi_config.getString("ssid") + "\n"
                    + pw + "\n"
                    + wifi_config.getString("localIP") + "\n"
                    + wifi_config.getString("gateway");
    
    request->send(200, "text/plain", payload);
}

void webfct_get_general_config(AsyncWebServerRequest *request) {

    String payload = wifi_info.getString("deviceName") + "\n";
    request->send(200, "text/plain", payload);
}

void webfct_get_settings(AsyncWebServerRequest *request) {request->send(SPIFFS, "/settings.html", "text/html");}

void webfct_settings_post(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len,  size_t total) {
    String payload = String((char*)data);
    int pos = payload.indexOf('=');
    if (pos != -1) { // Make sure '=' is found in the string
        String valueStr = payload.substring(pos + 1);

        // Step 3: Convert the value to a boolean
        bool value = (valueStr == "true");  // Assumes the string is exactly "true" or anything else is false

        // Use the boolean value as needed
        DualSerial.print("Boolean value: ");
        DualSerial.println(value);
    }
}

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



// --------- member functions ----------- //

void project_utils_init(const String& ap_name = "New ESP-Device") {
    uint8_t retval;

    // establish connection
    if((retval = network_manager_init(ap_name)) != NETWORK_MANAGER_ERROR_NO_ERROR) 
        ram_log_notify(RAM_LOG_ERROR_NETWORK_MANAGER, retval);

    // *****----- SERVER MODULES -----*****

    // Systems that do not need SPIFFS
#ifdef SYS_CONTROL_WEBSERIAL
    webserial_monitor_init();
#endif

    network_ota_init();

    // now lets initialize spiffs
    if(!SPIFFS.begin(true)){
        ram_log_notify(RAM_LOG_INFO, "SPIFFS failed to initialize!");
        return;
    }

    server.serveStatic("/", SPIFFS, "/");                                                                                               // makes everything available in the browser
    server.on("/wifi", HTTP_GET, webfct_wifi_get);                                                                                      // serves /wifi config page
    server.on("/", HTTP_POST, webfct_wifi_post);                                                                                        // listens for POST requests and saves user wifi credentials
    server.on("/wifi-config", HTTP_GET, webfct_get_wifi_cred);                                                                          // serves wifi credentials requested by /wifi config page
    server.on("/general-config", HTTP_GET, webfct_get_general_config);                                                                  // serves general configuration
    server.on("/settings", HTTP_GET, webfct_get_settings);
    server.on("/settings", HTTP_POST, [](AsyncWebServerRequest *request) {}, webfct_settings_post);

    server.begin();
}


void project_utils_update() {
    network_ota_update();
    network_manager_update();
}