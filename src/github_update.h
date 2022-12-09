//
// Created by Jo Uni on 08/12/2022.
//

#pragma once

#include "webserial_monitor.h"

typedef enum{
    GITHUB_UPDATE_ERROR_NO_ERROR        = 0x00,
    GITHUB_UPDATE_ERROR_WIFI            = 0x01,
    GITHUB_UPDATE_ERROR_HTTP            = 0x02,
    GITHUB_UPDATE_ERROR_NO_UPDATE       = 0x03,
    GITHUB_UPDATE_ERROR_UNKNOWN         = 0xFF
} GITHUB_UPDATE_ERROR_t;

GITHUB_UPDATE_ERROR_t github_update_firmwareUpdate();
GITHUB_UPDATE_ERROR_t github_update_firmwareUpdate(const char *desired_version);
GITHUB_UPDATE_ERROR_t github_update_fwVersionCheck(uint8_t fw_major, uint8_t fw_minor, uint8_t fw_patch);

void github_update_init(const char *url_version, const char *url_bin);
