//
// Created by Cameloah on 31.03.2023.
//

#pragma once

#include "wifi_handler.h"

WIFI_HANDLER_ERROR_t wifi_manager_load(wifi_info_t* user_buffer);
WIFI_HANDLER_ERROR_t wifi_manager_AP();
void wifi_manager_update();