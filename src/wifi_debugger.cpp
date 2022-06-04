#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "cert.h"
#include "wifi_debugger.h"

char url_fw_version[200] = "";
char url_fw_bin[200] = "";

String FirmwareVer = {
        "2.1"
};


void wifi_debugger_init(const char* user_ssid, const char* user_password, const char* url_version, const char* url_bin) {
    // get all the user data first
    strcpy(url_fw_version, url_version);
    strcpy(url_fw_bin, url_bin);
    Serial.print("Active firmware version:");
    Serial.println(FirmwareVer);
    connect_wifi(user_ssid, user_password);
}

void wifi_debugger_update() {
}

int timer_wifi_connect = 0;
void connect_wifi(const char* ssid, const char* password) {
    Serial.println("Warte auf WiFi");
    WiFi.begin(ssid, password);
    while ((WiFi.status() != WL_CONNECTED)) {
        delay(500);
        Serial.print(".");
        timer_wifi_connect++;
        if(timer_wifi_connect > 60) {
            Serial.println("Gespeichertes WiFi nicht gefunden.");
            return;
        }
    }

    Serial.println("");
    Serial.println("WiFi verbunden");
    Serial.println("IP-Addresse: ");
    Serial.println(WiFi.localIP());
}


void wifi_debugger_firmwareUpdate(void) {
    WiFiClientSecure client;
    client.setCACert(rootCACertificate);
    httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    t_httpUpdate_return ret = httpUpdate.update(client, "https://github.com/Cameloah/Kraeng-o-meter/releases/download/v1.0.1/kraengometer_v1_0_1.bin");

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
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
    fwurl += "https://raw.githubusercontent.com/Cameloah/Kraeng-o-meter/master/bin_version.txt";
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

        Serial.print("Firmware from Payload:");
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