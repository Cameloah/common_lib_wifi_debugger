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

/// Tries to download the bin file matching the version saved in private buffer
/// \return GITHUB_UPDATE_ERROR_t error code
GITHUB_UPDATE_ERROR_t github_update_firmwareUpdate();

/// Tries to download bin file for a specific firmware version
/// \param desired_version version string in v0.0.0 format
/// \return GITHUB_UPDATE_ERROR_t error code
GITHUB_UPDATE_ERROR_t github_update_firmwareUpdate(const char *desired_version);

/// Checks for the latest release version on a github repository and compares to current version
/// \param fw_major current major
/// \param fw_minor current minor
/// \param fw_patch current patch
/// \return GITHUB_UPDATE_ERROR_t status if new version is available
GITHUB_UPDATE_ERROR_t github_update_fwVersionCheck(uint8_t fw_major, uint8_t fw_minor, uint8_t fw_patch);

/// Initializes the github update module
/// \param url_version url string to the latest github release of that project
/// \param url_bin url to the download server of github to download latest bin
void github_update_init(const char *url_version, const char *url_bin);
