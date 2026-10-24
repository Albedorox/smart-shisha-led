// RAM-Konfiguration mit Validierung (alles oder nichts), Reset und Versionszähler (06 §5)
#pragma once

#include <stdint.h>

#include "app_config.h"
#include "color_math.h"

enum class Mode : uint8_t { Smart, Steady, Off };

struct Config {
  Mode     mode        = Mode::Smart;
  uint16_t yellowMs    = app::DEFAULT_YELLOW_MS;
  uint16_t redMs       = app::DEFAULT_RED_MS;
  uint16_t tailMs      = app::DEFAULT_TAIL_MS;
  uint8_t  brightness  = app::DEFAULT_BRIGHTNESS;
  Rgb      steadyColor = {app::DEFAULT_STEADY_R, app::DEFAULT_STEADY_G, app::DEFAULT_STEADY_B};
  bool     diagnostic  = false;
  bool     idleGlow    = app::DEFAULT_IDLE_GLOW;
};

// Teil-Update: nur Felder mit has* = true werden geändert. Zahlen kommen ungeprüft
// als int32 an, damit die Bereichsprüfung hier und nicht beim Einlesen passiert.
struct ConfigPatch {
  bool    hasMode = false;        Mode    mode = Mode::Smart;
  bool    hasBrightness = false;  int32_t brightness = 0;
  bool    hasSteadyColor = false; int32_t steadyR = 0, steadyG = 0, steadyB = 0;
  bool    hasYellowMs = false;    int32_t yellowMs = 0;
  bool    hasRedMs = false;       int32_t redMs = 0;
  bool    hasTailMs = false;      int32_t tailMs = 0;
  bool    hasDiagnostic = false;  bool    diagnostic = false;
  bool    hasIdleGlow = false;    bool    idleGlow = false;

  bool empty() const {
    return !(hasMode || hasBrightness || hasSteadyColor || hasYellowMs || hasRedMs || hasTailMs ||
             hasDiagnostic || hasIdleGlow);
  }
};

enum class ConfigError : uint8_t { None, EmptyPatch, OutOfRange, ValidationFailed };

enum class ConfigField : uint8_t {
  None, Mode, Brightness, SteadyColor, YellowMs, RedMs, TailMs, Diagnostic, IdleGlow
};

struct ApplyResult {
  ConfigError error = ConfigError::None;
  ConfigField field = ConfigField::None;
  bool ok() const { return error == ConfigError::None; }
};

enum class ResetScope : uint8_t {
  Lighting,  // mode, brightness, steadyColor, idleGlow
  Gradient,  // yellowMs, redMs
  Sensor,    // tailMs, diagnostic
  All,
};

class ConfigManager {
 public:
  const Config& get() const { return cfg_; }
  uint32_t version() const { return version_; }

  // Prüft den ganzen Patch gegen die resultierenden Werte; ändert nur bei Erfolg.
  ApplyResult apply(const ConfigPatch& patch);
  void reset(ResetScope scope);

 private:
  Config   cfg_;
  uint32_t version_ = 0;
};

const char* configFieldName(ConfigField f);
