// Smart Shisha LED-Untersetzer – Einstieg (übernimmt die Rolle von main.cpp, D-19)
//
// Stand B1: nur die Kernlogik (Konfiguration, Zugerkennung, Rangliste, LED-Logik)
// ist umgesetzt und per Host-Tests geprüft (test/host). Sensor- und LED-Anbindung
// folgen in B2, WLAN und API in B3.

#include "app_config.h"
#include "config_manager.h"
#include "led_logic.h"
#include "pull_detector.h"
#include "session_manager.h"

void setup() {
  Serial.begin(115200);
  Serial.printf("ShishaLed %s – Kernlogik (B1), Hardwareanbindung folgt in B2\n", app::FW_VERSION);
}

void loop() {}
