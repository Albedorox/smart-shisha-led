// Soll-Anzeige aus Modus, Zugzustand und Fade-out, ohne LED-Treiber (04 §6).
// Der LedController (B2) schreibt das Ergebnis mit Adafruit NeoPixel auf den Ring.
#pragma once

#include <stdint.h>

#include "color_math.h"
#include "config_manager.h"
#include "pull_detector.h"

enum class Phase : uint8_t { Idle, Pull, Fade };

struct LedFrame {
  Phase   phase      = Phase::Idle;
  Rgb     color      = {0, 0, 0};   // logische Farbe vor Helligkeit (für die API)
  Rgb     pixel      = {0, 0, 0};   // Ausgabewert je LED nach Helligkeit und Stromgrenze
  uint8_t brightness = 0;           // wirksame Helligkeit B_eff
  bool    limited    = false;
  bool    glow       = false;       // nur eine LED schwach (Bereitschaftsglimmen, D-30)
};

class LedLogic {
 public:
  LedFrame update(uint32_t now, const Config& cfg, const PullState& pull);

 private:
  bool     wasActive_ = false;
  bool     fading_    = false;
  uint32_t pullEnd_   = 0;
  Rgb      lastColor_ = {0, 255, 0};
};
