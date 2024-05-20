//
// Created by Cameloah on 31.03.2023.
//

#pragma once

#include "wifi_handler.h"
#include "memory_module.h"


/// \brief get callback for page /wifi. serves page
///
/// \param request incoming request
void webfct_wifi_get(AsyncWebServerRequest *request);

/// \brief post callback for page /wifi. saves and applies incoming user wifi credentials
///
/// \param request incoming request
void webfct_wifi_post(AsyncWebServerRequest *request);

/// \brief loads saved wifi configuration if available. checks for non-meaningful credentials
///
/// \param user_buffer user provided buffer to save wifi configuration
/// \return WIFI_HANDLER_ERROR_t error code
WIFI_HANDLER_ERROR_t wifi_manager_load(MemoryModule* user_config);

/// \brief setup and start an access point for users to log on and set up the device
///
/// /// \param ap_name name of the device when in access point mode
/// \return WIFI_HANDLER_ERROR_t error code
WIFI_HANDLER_ERROR_t wifi_manager_AP(const String& ap_name);

/// \brief needs to be run periodically
void wifi_manager_update();