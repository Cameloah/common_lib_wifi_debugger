//
// Created by Jo Uni on 08/12/2022.
//

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>

#include "github_update.h"
#include "cert.h"
#include "../../../include/tools/loop_timer.h"

String url_fw_version;
String url_fw_bin;
String fw_version;


void github_update_init(const char *url_version, const char *url_bin) {
    url_fw_version = url_version;
    url_fw_bin = url_bin;
}

GITHUB_UPDATE_ERROR_t github_update_firmwareUpdate(void) {
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
            return GITHUB_UPDATE_ERROR_HTTP;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            return GITHUB_UPDATE_ERROR_NO_UPDATE;

        case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            return GITHUB_UPDATE_ERROR_NO_ERROR;
    }
}

GITHUB_UPDATE_ERROR_t github_update_firmwareUpdate(const char *desired_version) {
    // TODO: check for proper format
    fw_version = desired_version;
    return github_update_firmwareUpdate();
}

GITHUB_UPDATE_ERROR_t github_update_fwVersionCheck(uint8_t fw_major, uint8_t fw_minor, uint8_t fw_patch) {
    // if wifi not connected, cancel early
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wifi is not connected");
        return GITHUB_UPDATE_ERROR_WIFI;
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
                return GITHUB_UPDATE_ERROR_HTTP;
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
            return GITHUB_UPDATE_ERROR_NO_UPDATE;
        }
        return GITHUB_UPDATE_ERROR_NO_ERROR;
    }
    return GITHUB_UPDATE_ERROR_HTTP;
}