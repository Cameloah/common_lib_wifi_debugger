//
// Created by Cameloah on 31.03.2023.
//

#pragma once

#include "wifi_handler.h"

void wifi_manager_init();
bool wifi_manager_load(wifi_info_t* user_buffer);
void wifi_manager_AP(wifi_info_t* user_buffer);