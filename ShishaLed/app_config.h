// Kompilierkonstanten: Pins, Polarität, Grenzen, Standardwerte (04 §2.2, 01 §9)
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace app {

constexpr const char* FW_VERSION  = "0.3.0";
constexpr const char* DEVICE_NAME = "Smart Shisha LED-Untersetzer";
constexpr const char* AP_SSID     = "ShishaLed";  // offenes WLAN (Nutzer 2026-10-25, statt 06: Shisha-LED-Control)

// Hardware (02 §2). SENSOR_ACTIVE_LOW nach B0 / T-HW-04 bestätigen.
constexpr uint8_t PIN_SENSOR        = 0;
constexpr uint8_t PIN_LED           = 4;
constexpr bool    SENSOR_ACTIVE_LOW = true;

// LEDs (CR-01, D-05)
constexpr uint16_t LED_COUNT            = 24;
constexpr uint16_t LED_MA_PER_CHANNEL   = 20;    // Annahme, nach M-05 anpassen
constexpr uint16_t LED_CURRENT_LIMIT_MA = 500;
constexpr uint8_t  IDLE_GLOW_LEVEL      = 2;     // Bereitschaftsglimmen, eine LED (D-30)
constexpr uint32_t LED_FRAME_MS         = 20;    // höchstens 50 Hz
constexpr uint32_t FADE_MS              = 1000;  // fest (D-10)

// Sensor (04 §4)
constexpr uint32_t SENSOR_EVAL_MS  = 10;
constexpr uint32_t MAX_PULL_MS     = 60000;      // D-08
constexpr uint32_t STUCK_ACTIVE_MS = 5000;
constexpr uint32_t STUCK_CLEAR_MS  = 1000;
constexpr uint8_t  DIAG_WINDOW_S   = 10;

// Sitzung (01 §6)
constexpr uint8_t     LEADERBOARD_SIZE    = 5;
constexpr uint8_t     NAME_MAX_CODEPOINTS = 24;
constexpr size_t      NAME_BUF            = NAME_MAX_CODEPOINTS * 4 + 1;
constexpr const char* DEFAULT_NAME        = "Unbekannt";

// Konfiguration: Standardwerte und Bereiche (01 §9, D-09)
constexpr uint16_t DEFAULT_YELLOW_MS  = 3000;
constexpr uint16_t DEFAULT_RED_MS     = 6000;
constexpr uint16_t DEFAULT_TAIL_MS    = 1000;
constexpr uint8_t  DEFAULT_BRIGHTNESS = 64;
constexpr uint8_t  DEFAULT_STEADY_R   = 255;     // Warmweiß (D-07)
constexpr uint8_t  DEFAULT_STEADY_G   = 160;
constexpr uint8_t  DEFAULT_STEADY_B   = 60;
constexpr bool     DEFAULT_IDLE_GLOW  = true;    // D-30

constexpr int32_t YELLOW_MIN = 500,  YELLOW_MAX = 30000, YELLOW_STEP = 100;
constexpr int32_t RED_MIN    = 600,  RED_MAX    = 60000, RED_STEP    = 100;
constexpr int32_t RED_GAP    = 100;              // redMs >= yellowMs + RED_GAP
constexpr int32_t TAIL_MIN   = 200,  TAIL_MAX   = 5000,  TAIL_STEP   = 50;

}  // namespace app
