// WLAN-Zugangspunkt, Weboberfläche und HTTP-API nach 06_api_contracts.md.
// Nur für den ESP32 (WebServer, ArduinoJson); die Logikmodule bleiben hardwarefrei.
#pragma once

#include <stdint.h>

#include "config_manager.h"
#include "led_logic.h"
#include "pull_detector.h"
#include "session_manager.h"
#include "time_util.h"

struct WebContext {
  ConfigManager&  cfg;
  PullDetector&   det;
  SessionManager& ses;
  const LedFrame& frame;
  const Uptime&   up;
};

void webBegin(WebContext* ctx);
void webLoop();
