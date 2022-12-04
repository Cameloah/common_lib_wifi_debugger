#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "cert.h"
#include "wifi_debugger.h"
#include "../../../include/tools/loop_timer.h"

String url_fw_version;
String url_fw_bin;
String fw_version;
String ssid;
String password;

IPAddress local_IP;
IPAddress gateway;
IPAddress subnet;
IPAddress primaryDNS;   //optional
IPAddress secondaryDNS; //optional


WIFI_DEBUGGER_ERROR_t wifi_debugger_init(const char *user_ssid, const char *user_password, const char *user_ip,
                                         const char *user_gateway, const char *user_subnet, const char *url_version,
                                         const char *url_bin) {
    // get all the user data first
    local_IP.fromString(user_ip);
    gateway.fromString(user_gateway);
    subnet.fromString(user_subnet);

    url_fw_version = url_version;
    url_fw_bin = url_bin;

    Serial.println(url_fw_version);
    Serial.println(url_fw_bin);

    ssid = user_ssid;
    password = user_password;
    return connect_wifi();
}

int timer_wifi_connect = 0;

WIFI_DEBUGGER_ERROR_t connect_wifi() {
    Serial.println("Warte auf WiFi");

#ifdef SYS_CONTROL_STAT_IP
    if (!WiFi.config(local_IP, gateway, subnet)) {
        Serial.println("STA Failed to configure");
    }
#endif

    WiFi.begin(ssid.c_str(), password.c_str());
    while ((WiFi.status() != WL_CONNECTED)) {
        delay(500);
        Serial.print(".");
        timer_wifi_connect++;
        if (500 * timer_wifi_connect > TIMEOUT_WIFI_CONNECT_MS) {
            timer_wifi_connect = 0;
            return WIFI_DEBUGGER_ERROR_WIFI;
        }
    }

    Serial.println("");
    Serial.println("WiFi verbunden");
    Serial.println("IP-Addresse: ");
    Serial.println(WiFi.localIP());

    return WIFI_DEBUGGER_ERROR_NO_ERROR;
}

WIFI_DEBUGGER_ERROR_t wifi_debugger_firmwareUpdate(void) {
    // initiate wifi update client
    WiFiClientSecure client;
    client.setCACert(rootCACertificate);
    // github redirects you to the lastest version if you access ...\latest
    httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    // lets update the given fw binary url with the latest version
    // TODO: check for proper format of url
    url_fw_bin.replace("<version>", fw_version);
    Serial.printf("\nDownloading and installing Firmware Version %s...\n", fw_version.c_str());
    t_httpUpdate_return ret = httpUpdate.update(client, url_fw_bin);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(),
                          httpUpdate.getLastErrorString().c_str());
            return WIFI_DEBUGGER_ERROR_HTTP;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            return WIFI_DEBUGGER_ERROR_NO_UPDATE;

        case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            return WIFI_DEBUGGER_ERROR_NO_ERROR;
    }
}

WIFI_DEBUGGER_ERROR_t wifi_debugger_firmwareUpdate(const char *desired_version) {
    // TODO: check for proper format
    fw_version = desired_version;
    return wifi_debugger_firmwareUpdate();
}

WIFI_DEBUGGER_ERROR_t wifi_debugger_fwVersionCheck(uint8_t fw_major, uint8_t fw_minor, uint8_t fw_patch) {
    // if wifi not connected, cancel early
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wifi is not connected");
        return WIFI_DEBUGGER_ERROR_WIFI;
    }

    int httpCode;;
    WiFiClientSecure *client = new WiFiClientSecure;

    if (client) {
        client->setCACert(rootCACertificate);

        // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
        HTTPClient https;



        if (https.begin(*client, url_fw_version)) {
            // start connection and send HTTP header
            delay(100);
            // access the url
            httpCode = https.GET();

            if (httpCode == HTTP_CODE_FOUND) // if version received
            {
                // get updated url after redirect
                String location = https.getLocation();
                delay(100);
                // lets extract the FW version from the url
                fw_version = location.substring(location.indexOf("/releases/tag/") + 14);
            } else {
                Serial.print("error in downloading version file:");
                Serial.println(httpCode);
                return WIFI_DEBUGGER_ERROR_HTTP;
            }
            https.end();
        }
        delete client;
    }

    if (httpCode == HTTP_CODE_FOUND) // if version received
    {
        fw_version.trim();
        // extract firmware numbers
        String fw_version_latest = fw_version;
        uint8_t fw_latest_major = atoi(strtok((char *) fw_version_latest.c_str(), "v.\n"));
        uint8_t fw_latest_minor = atoi(strtok(nullptr, "v.\n"));
        uint8_t fw_latest_patch = atoi(strtok(nullptr, "v.\n"));

        if (fw_latest_major > fw_major) {
            Serial.printf("\nNew Firmware detected! Major update available: v%d.%d.%d\n",
                          fw_latest_major, fw_latest_minor, fw_latest_patch);
        } else if (fw_latest_major == fw_major && fw_latest_minor > fw_minor) {
            Serial.printf("\nNew Firmware detected! Minor update available: v%d.%d.%d\n",
                          fw_latest_major, fw_latest_minor, fw_latest_patch);
        } else if (fw_latest_major == fw_major && fw_latest_minor == fw_minor && fw_latest_patch > fw_patch) {
            Serial.printf("\nNew Firmware detected! New Patch available: v%d.%d.%d\n",
                          fw_latest_major, fw_latest_minor, fw_latest_patch);
        } else {
            Serial.printf("\nDevice running on latest firmware version: v%d.%d.%d\n",
                          fw_major, fw_minor, fw_patch);
            return WIFI_DEBUGGER_ERROR_NO_UPDATE;
        }
        return WIFI_DEBUGGER_ERROR_NO_ERROR;
    }
    return WIFI_DEBUGGER_ERROR_HTTP;
}

bool wifi_debugger_is_connected() {
    return WiFi.isConnected();
}