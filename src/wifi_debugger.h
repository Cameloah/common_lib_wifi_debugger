//
// Created by koorj on 03.03.2022.
//

#pragma once


void connect_wifi(const char* ssid, const char* password);
void wifi_debugger_firmwareUpdate();
int wifi_debugger_fwVersionCheck(uint8_t fw_major, uint8_t fw_minor, uint8_t fw_patch);

void wifi_debugger_init(const char* user_ssid, const char* user_password, const char* url_version, const char* url_bin);
void wifi_debugger_update();