#include "config_manager.h"

namespace {

bool inRange(int32_t v, int32_t lo, int32_t hi) { return v >= lo && v <= hi; }
bool onGrid(int32_t v, int32_t step) { return v % step == 0; }

ApplyResult fail(ConfigError e, ConfigField f) {
  ApplyResult r;
  r.error = e;
  r.field = f;
  return r;
}

}  // namespace

ApplyResult ConfigManager::apply(const ConfigPatch& p) {
  using namespace app;
  if (p.empty()) return fail(ConfigError::EmptyPatch, ConfigField::None);

  // Einzelwerte
  if (p.hasBrightness && !inRange(p.brightness, 0, 255)) {
    return fail(ConfigError::OutOfRange, ConfigField::Brightness);
  }
  if (p.hasSteadyColor &&
      !(inRange(p.steadyR, 0, 255) && inRange(p.steadyG, 0, 255) && inRange(p.steadyB, 0, 255))) {
    return fail(ConfigError::OutOfRange, ConfigField::SteadyColor);
  }
  if (p.hasYellowMs && !(inRange(p.yellowMs, YELLOW_MIN, YELLOW_MAX) && onGrid(p.yellowMs, YELLOW_STEP))) {
    return fail(ConfigError::OutOfRange, ConfigField::YellowMs);
  }
  if (p.hasRedMs && !(inRange(p.redMs, RED_MIN, RED_MAX) && onGrid(p.redMs, RED_STEP))) {
    return fail(ConfigError::OutOfRange, ConfigField::RedMs);
  }
  if (p.hasTailMs && !(inRange(p.tailMs, TAIL_MIN, TAIL_MAX) && onGrid(p.tailMs, TAIL_STEP))) {
    return fail(ConfigError::OutOfRange, ConfigField::TailMs);
  }

  // Kombination mit den resultierenden Werten
  const int32_t yellow = p.hasYellowMs ? p.yellowMs : cfg_.yellowMs;
  const int32_t red = p.hasRedMs ? p.redMs : cfg_.redMs;
  if (red < yellow + RED_GAP) {
    return fail(ConfigError::ValidationFailed, p.hasRedMs ? ConfigField::RedMs : ConfigField::YellowMs);
  }

  Config next = cfg_;
  if (p.hasMode) next.mode = p.mode;
  if (p.hasBrightness) next.brightness = (uint8_t)p.brightness;
  if (p.hasSteadyColor) next.steadyColor = {(uint8_t)p.steadyR, (uint8_t)p.steadyG, (uint8_t)p.steadyB};
  next.yellowMs = (uint16_t)yellow;
  next.redMs = (uint16_t)red;
  if (p.hasTailMs) next.tailMs = (uint16_t)p.tailMs;
  if (p.hasDiagnostic) next.diagnostic = p.diagnostic;
  if (p.hasIdleGlow) next.idleGlow = p.idleGlow;
  cfg_ = next;
  version_++;
  return ApplyResult();
}

void ConfigManager::reset(ResetScope scope) {
  const Config d;
  switch (scope) {
    case ResetScope::Lighting:
      cfg_.mode = d.mode;
      cfg_.brightness = d.brightness;
      cfg_.steadyColor = d.steadyColor;
      cfg_.idleGlow = d.idleGlow;
      break;
    case ResetScope::Gradient:
      cfg_.yellowMs = d.yellowMs;
      cfg_.redMs = d.redMs;
      break;
    case ResetScope::Sensor:
      cfg_.tailMs = d.tailMs;
      cfg_.diagnostic = d.diagnostic;
      break;
    case ResetScope::All:
      cfg_ = d;
      break;
  }
  version_++;
}

const char* configFieldName(ConfigField f) {
  switch (f) {
    case ConfigField::Mode: return "mode";
    case ConfigField::Brightness: return "brightness";
    case ConfigField::SteadyColor: return "steadyColor";
    case ConfigField::YellowMs: return "yellowMs";
    case ConfigField::RedMs: return "redMs";
    case ConfigField::TailMs: return "tailMs";
    case ConfigField::Diagnostic: return "diagnostic";
    case ConfigField::IdleGlow: return "idleGlow";
    case ConfigField::None: break;
  }
  return "";
}
