//
// Created by Cameloah on 31.03.2023.
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

const byte DNS_PORT = 53;
DNSServer dnsServer;

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

bool bool_test = false;
String ledstate = "OFF";
String ip_default = "192.168.2.73";

//Variables to save values from HTML form
char ssid[30];
char pass[20];
char ip[20];
char gateway[20];

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";

bool flag_ap_active = false;

// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path){
    DualSerial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        DualSerial.println("- failed to open file for reading");
        return {};
    }

    String fileContent;
    while(file.available()){
        fileContent = file.readStringUntil('\n');
        break;
    }
    return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message){
    DualSerial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        DualSerial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        DualSerial.println("- file written");
    } else {
        DualSerial.println("- write failed");
    }
}

void read_data_from_eeprom() {
    EEPROM_readAnything(20,ip);
    EEPROM_readAnything(40,gateway);
    EEPROM_readAnything(60,ssid);
    EEPROM_readAnything(90,pass);
}

void save_data_to_eeprom() {
    EEPROM_writeAnything(20, ip);
    EEPROM_writeAnything(40, gateway);
    EEPROM_writeAnything(60, ssid);
    EEPROM_writeAnything(90, pass);

    EEPROM.commit();
}

void webfct_wifi_get(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/wifimanager.html", "text/html");
}

void webfct_wifi_post(AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
            // HTTP POST ssid value
            if (p->name() == PARAM_INPUT_1) {
                strcpy(ssid, p->value().c_str()); //ssid = p->value().c_str();
                DualSerial.print("SSID set to: ");
                DualSerial.println(ssid);
                // Write file to save value
                // writeFile(SPIFFS, ssidPath, ssid.c_str());
            }
            // HTTP POST pass value
            if (p->name() == PARAM_INPUT_2) {
                strcpy(pass, p->value().c_str()); //pass = p->value().c_str();
                DualSerial.print("Password set to: ");
                DualSerial.println(pass);
                // Write file to save value
                // writeFile(SPIFFS, passPath, pass.c_str());
            }
            // HTTP POST ip value
            if (p->name() == PARAM_INPUT_3) {
                strcpy(ip, p->value().c_str()); //ip = p->value().c_str();
                DualSerial.print("IP Address set to: ");
                DualSerial.println(ip);
                // Write file to save value
                // writeFile(SPIFFS, ipPath, ip.c_str());
            }
            // HTTP POST gateway value
            if (p->name() == PARAM_INPUT_4) {
                strcpy(gateway, p->value().c_str()); //gateway = p->value().c_str();
                DualSerial.print("Gateway set to: ");
                DualSerial.println(gateway);
                // Write file to save value
                // writeFile(SPIFFS, gatewayPath, gateway.c_str());
            }
            //DualSerial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            // save to eeprom
            save_data_to_eeprom();
        }
    }
    request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + String(ip));
    delay(3000);
    ESP.restart();
}

WIFI_HANDLER_ERROR_t wifi_manager_load(wifi_info_t* user_buffer) {
    if (!SPIFFS.begin(true)) {
        return WIFI_HANDLER_ERROR_SPIFFS;
    }

    // read from eeprom
    read_data_from_eeprom();

    // Load values saved in SPIFFS
    user_buffer->_ssid = ssid; //= readFile(SPIFFS, ssidPath);
    user_buffer->_password = pass; //= readFile(SPIFFS, passPath);
    user_buffer->_subnet.fromString("255.255.255.0");
    user_buffer->_primaryDNS.fromString("8.8.8.8");
    user_buffer->_secondaryDNS.fromString("8.8.4.4");


    if(user_buffer->_ssid=="" || user_buffer->_password==""){
        // there were no configs stored, so return false
        return WIFI_HANDLER_ERROR_CONFIG;
    }

    // do not populate these if fails
    user_buffer->_local_IP.fromString(ip); //fromString(readFile(SPIFFS, ipPath));
    user_buffer->_gateway.fromString(gateway); //fromString(readFile (SPIFFS, gatewayPath));

    return WIFI_HANDLER_ERROR_NO_ERROR;
}

WIFI_HANDLER_ERROR_t wifi_manager_AP(String ap_name) {
    // Connect to Wi-Fi network with SSID and password
    ram_log_notify(RAM_LOG_INFO, "Starting Access Point", true);
    // nullptr sets an open Access Point

    if(!WiFi.softAP(ap_name.c_str(), AP_PASSWORD))
        return WIFI_HANDLER_ERROR_AP;

    DualSerial.print("AP IP address: ");
    DualSerial.println(WiFi.softAPIP());

    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    // Web Server Root URL
    server.onNotFound(webfct_wifi_get);

    flag_ap_active = true;
    return WIFI_HANDLER_ERROR_NO_ERROR;
}

void wifi_manager_update() {
    if (flag_ap_active)
        dnsServer.processNextRequest();

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