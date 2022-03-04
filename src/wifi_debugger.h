//
// Created by koorj on 03.03.2022.
//

#pragma once


void connect_wifi();
void wifi_debugger_firmwareUpdate();
int wifi_debugger_fwVersionCheck();

void wifi_debugger_init(const char* user_ssid, const char* user_password, const char* user_url_fw_version, const char* user_url_fw_bin);
void wifi_debugger_update();