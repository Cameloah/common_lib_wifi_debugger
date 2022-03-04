#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "cert.h"
#include "wifi_debugger.h"

const char* ssid = "";
const char* password = "";

const char* url_fw_version = "";
const char* url_fw_bin = "";

String FirmwareVer = {
        "2.2"
};
#define URL_fw_Version "https://github.com/Cameloah/common_lib_wifi_debugger/blob/master/bin_version.txt"
#define URL_fw_Bin "https://github.com/Cameloah/common_lib_wifi_debugger/blob/master/.pio/build/esp32dev/firmware.bin"

//#define URL_fw_Version "http://cade-make.000webhostapp.com/version.txt"
//#define URL_fw_Bin "http://cade-make.000webhostapp.com/firmware.bin"


unsigned long previousMillis = 0; // will store last time LED was updated
unsigned long previousMillis_2 = 0;
const long interval = 60000;
const long mini_interval = 1000;
void repeatedCall() {
    static int num=0;
    unsigned long currentMillis = millis();
    if ((currentMillis - previousMillis) >= interval) {
        // save the last time you blinked the LED
        previousMillis = currentMillis;
        if (wifi_debugger_fwVersionCheck()) {
            wifi_debugger_firmwareUpdate();
        }
    }
    if ((currentMillis - previousMillis_2) >= mini_interval) {
        previousMillis_2 = currentMillis;
        Serial.print("idle loop...");
        Serial.print(num++);
        Serial.print(" Active fw version:");
        Serial.println(FirmwareVer);
        if(WiFi.status() == WL_CONNECTED)
        {
            Serial.println("wifi connected");
        }
        else
        {
            connect_wifi();
        }
    }
}

struct Button {
    const uint8_t PIN;
    uint32_t numberKeyPresses;
    bool pressed;
};

Button button_boot = {
        0,
        0,
        false
};
/*void IRAM_ATTR isr(void* arg) {
    Button* s = static_cast<Button*>(arg);
    s->numberKeyPresses += 1;
    s->pressed = true;
}*/

void IRAM_ATTR isr() {
    button_boot.numberKeyPresses += 1;
    button_boot.pressed = true;
}


void wifi_debugger_init(const char* user_ssid, const char* user_password, const char* user_url_fw_version, const char* user_url_fw_bin) {
    // get all the user data first
    ssid = user_ssid;
    password = user_password;
    url_fw_version = user_url_fw_version;
    url_fw_bin = user_url_fw_bin;

    pinMode(button_boot.PIN, INPUT);
    attachInterrupt(button_boot.PIN, isr, RISING);
    Serial.print("Active firmware version:");
    Serial.println(FirmwareVer);
    connect_wifi();
}

void wifi_debugger_update() {
    if (button_boot.pressed) { //to connect wifi via Android esp touch app
        Serial.println("Firmware update Starting..");
        wifi_debugger_firmwareUpdate();
        button_boot.pressed = false;
    }
    repeatedCall();
}

int timer_wifi_connect = 0;
void connect_wifi() {
    Serial.println("Waiting for WiFi");
    WiFi.begin(ssid, password);
    while ((WiFi.status() != WL_CONNECTED)) {
        delay(500);
        Serial.print(".");
        timer_wifi_connect++;
        if(timer_wifi_connect > 10) {
            Serial.println("Wifi timeout.");
            return;
        }
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}


void wifi_debugger_firmwareUpdate(void) {
    WiFiClientSecure client;
    client.setCACert(rootCACertificate);
    t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;

        case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            break;
    }
}
int wifi_debugger_fwVersionCheck(void) {
    // if wifi not connected, cancel early
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wifi is not connected");
        return 0;
    }

    String payload;
    int httpCode;
    String fwurl = "";
    fwurl += URL_fw_Version;
    fwurl += "?";
    fwurl += String(rand());
    Serial.println(fwurl);
    WiFiClientSecure * client = new WiFiClientSecure;

    if (client)
    {
        client -> setCACert(rootCACertificate);

        // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
        HTTPClient https;

        if (https.begin( * client, fwurl))
        { // HTTPS
            Serial.print("[HTTPS] GET...\n");
            // start connection and send HTTP header
            delay(100);
            httpCode = https.GET();
            delay(100);
            if (httpCode == HTTP_CODE_OK) // if version received
            {
                payload = https.getString(); // save received version
            } else {
                Serial.print("error in downloading version file:");
                Serial.println(httpCode);
            }
            https.end();
        }
        delete client;
    }

    if (httpCode == HTTP_CODE_OK) // if version received
    {
        payload.trim();
        if (payload.equals(FirmwareVer)) {
            Serial.printf("\nDevice already on latest firmware version:%s\n", FirmwareVer.c_str());
            return 0;
        }
        else
        {
            Serial.println(payload);
            Serial.println("New firmware detected");
            return 1;
        }
    }
    return 0;
}