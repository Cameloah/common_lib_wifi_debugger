//
// Created by Cameloah on 03.03.2022.
//

#pragma once

#include "ESPAsyncWebServer.h"

extern AsyncWebServer server;


void project_utils_init(const String& ap_name);

void project_utils_update();