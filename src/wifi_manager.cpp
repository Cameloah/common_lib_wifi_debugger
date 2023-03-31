//
// Created by Cameloah on 31.03.2023.
//

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"

#include "wifi_manager.h"
#include "wifi_handler.h"
#include "webserial_monitor.h"

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

bool bool_test = false;
String ledstate = "OFF";

//Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";

IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)

// Initialize SPIFFS
void initSPIFFS() {
    if (!SPIFFS.begin(true)) {
        Serial.println("An error has occurred while mounting SPIFFS");
    }
    DualSerial.println("SPIFFS mounted successfully");
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path){
    DualSerial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        DualSerial.println("- failed to open file for reading");
        return String();
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
        DualSerial.println("- frite failed");
    }
}

// Initialize WiFi
bool initWiFi() {
    if(ssid=="" || ip==""){
        DualSerial.println("Undefined SSID or IP address.");
        return false;
    }

    WiFi.mode(WIFI_STA);
    localIP.fromString(ip.c_str());
    localGateway.fromString(gateway.c_str());


    if (!WiFi.config(localIP, localGateway, subnet)){
        DualSerial.println("STA Failed to configure");
        return false;
    }
    WiFi.begin(ssid.c_str(), pass.c_str());
    DualSerial.println("Connecting to WiFi...");

    unsigned long currentMillis = millis();
    previousMillis = currentMillis;

    while(WiFi.status() != WL_CONNECTED) {
        currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            DualSerial.println("Failed to connect.");
            return false;
        }
    }

    DualSerial.println(WiFi.localIP());
    return true;
}

// Replaces placeholder with LED state value
String processor(const String& var) {
    if(var == "STATE") {
        if(bool_test) {
            ledstate = "ON";
        }
        else {
            ledstate = "OFF";
        }
        return ledstate;
    }
    return String();
}

bool wifi_manager_load(wifi_info_t* user_buffer) {
    initSPIFFS();
    // Load values saved in SPIFFS
    user_buffer->_ssid = readFile(SPIFFS, ssidPath);
    user_buffer->_password = readFile(SPIFFS, passPath);
    user_buffer->_subnet.fromString("255.255.255.0");
    user_buffer->_primaryDNS.fromString("8.8.8.8");
    user_buffer->_secondaryDNS.fromString("8.8.4.4");


    if(user_buffer->_ssid=="" || user_buffer->_password==""){
        // there were no configs stored, so return false
        return false;
    }

    // do not populate these if fails
    user_buffer->_local_IP.fromString(readFile(SPIFFS, ipPath));
    user_buffer->_gateway.fromString(readFile (SPIFFS, gatewayPath));

    return true;
}

void wifi_manager_AP(wifi_info_t* user_buffer) {
    // Connect to Wi-Fi network with SSID and password
    DualSerial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("New ESP-Device", NULL);

    IPAddress IP = WiFi.softAPIP();
    DualSerial.print("AP IP address: ");
    DualSerial.println(IP);

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/wifimanager.html", "text/html");
    });

    server.serveStatic("/", SPIFFS, "/");

    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
        int params = request->params();
        for(int i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isPost()){
                // HTTP POST ssid value
                if (p->name() == PARAM_INPUT_1) {
                    ssid = p->value().c_str();
                    DualSerial.print("SSID set to: ");
                    DualSerial.println(ssid);
                    // Write file to save value
                    writeFile(SPIFFS, ssidPath, ssid.c_str());
                }
                // HTTP POST pass value
                if (p->name() == PARAM_INPUT_2) {
                    pass = p->value().c_str();
                    DualSerial.print("Password set to: ");
                    DualSerial.println(pass);
                    // Write file to save value
                    writeFile(SPIFFS, passPath, pass.c_str());
                }
                // HTTP POST ip value
                if (p->name() == PARAM_INPUT_3) {
                    ip = p->value().c_str();
                    DualSerial.print("IP Address set to: ");
                    DualSerial.println(ip);
                    // Write file to save value
                    writeFile(SPIFFS, ipPath, ip.c_str());
                }
                // HTTP POST gateway value
                if (p->name() == PARAM_INPUT_4) {
                    gateway = p->value().c_str();
                    DualSerial.print("Gateway set to: ");
                    DualSerial.println(gateway);
                    // Write file to save value
                    writeFile(SPIFFS, gatewayPath, gateway.c_str());
                }
                //DualSerial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
        }
        request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
        delay(3000);
        ESP.restart();
    });
    server.begin();
}

void wifi_manager_init() {
    initSPIFFS();

    DualSerial.println(ssid);
    DualSerial.println(pass);
    DualSerial.println(ip);
    DualSerial.println(gateway);

    if(initWiFi()) {
        // Route for root / web page
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/index.html", "text/html", false, processor);
        });
        server.serveStatic("/", SPIFFS, "/");

        // Route to set GPIO state to HIGH
        server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
            DualSerial.println(ledstate);
            request->send(SPIFFS, "/index.html", "text/html", false, processor);
        });

        // Route to set GPIO state to LOW
        server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
            DualSerial.println(ledstate);
            request->send(SPIFFS, "/index.html", "text/html", false, processor);
        });
        server.begin();
    }
    else {
        // Connect to Wi-Fi network with SSID and password
        DualSerial.println("Setting AP (Access Point)");
        // NULL sets an open Access Point
        WiFi.softAP("ESP-WIFI-MANAGER", NULL);

        IPAddress IP = WiFi.softAPIP();
        DualSerial.print("AP IP address: ");
        DualSerial.println(IP);

        // Web Server Root URL
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(SPIFFS, "/wifimanager.html", "text/html");
        });

        server.serveStatic("/", SPIFFS, "/");

        server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
            int params = request->params();
            for(int i=0;i<params;i++){
                AsyncWebParameter* p = request->getParam(i);
                if(p->isPost()){
                    // HTTP POST ssid value
                    if (p->name() == PARAM_INPUT_1) {
                        ssid = p->value().c_str();
                        DualSerial.print("SSID set to: ");
                        DualSerial.println(ssid);
                        // Write file to save value
                        writeFile(SPIFFS, ssidPath, ssid.c_str());
                    }
                    // HTTP POST pass value
                    if (p->name() == PARAM_INPUT_2) {
                        pass = p->value().c_str();
                        DualSerial.print("Password set to: ");
                        DualSerial.println(pass);
                        // Write file to save value
                        writeFile(SPIFFS, passPath, pass.c_str());
                    }
                    // HTTP POST ip value
                    if (p->name() == PARAM_INPUT_3) {
                        ip = p->value().c_str();
                        DualSerial.print("IP Address set to: ");
                        DualSerial.println(ip);
                        // Write file to save value
                        writeFile(SPIFFS, ipPath, ip.c_str());
                    }
                    // HTTP POST gateway value
                    if (p->name() == PARAM_INPUT_4) {
                        gateway = p->value().c_str();
                        DualSerial.print("Gateway set to: ");
                        DualSerial.println(gateway);
                        // Write file to save value
                        writeFile(SPIFFS, gatewayPath, gateway.c_str());
                    }
                    //DualSerial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
                }
            }
            request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
            delay(3000);
            ESP.restart();
        });
        server.begin();
    }
}
