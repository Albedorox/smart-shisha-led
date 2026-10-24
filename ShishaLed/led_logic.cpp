#include "led_logic.h"

#include "app_config.h"
#include "time_util.h"

namespace {

LedFrame output(Phase phase, Rgb color, uint8_t brightness) {
  LedFrame f;
  f.phase = phase;
  f.color = color;
  const color::Limited l = color::limitBrightness(color, brightness, app::LED_COUNT,
                                                  app::LED_MA_PER_CHANNEL, app::LED_CURRENT_LIMIT_MA);
  f.brightness = l.brightness;
  f.limited = l.limited;
  f.pixel = color::scale(color, l.brightness);
  return f;
}

}  // namespace

LedFrame LedLogic::update(uint32_t now, const Config& cfg, const PullState& pull) {
  const bool smart = cfg.mode == Mode::Smart;

  if (pull.active) {
    fading_ = false;  // neuer Zug hat Vorrang (LED-09)
    lastColor_ = color::gradient(elapsed(now, pull.startMs), cfg.yellowMs, cfg.redMs);
  } else if (wasActive_) {
    pullEnd_ = now;
    fading_ = smart;
  }
  wasActive_ = pull.active;
  if (!smart) fading_ = false;  // Wechsel weg von Smart bricht den Fade-out ab

  const Phase busy = pull.active ? Phase::Pull : Phase::Idle;
  switch (cfg.mode) {
    case Mode::Steady:
      return output(busy, cfg.steadyColor, cfg.brightness);
    case Mode::Off:
      return output(busy, {0, 0, 0}, cfg.brightness);
    case Mode::Smart:
      break;
  }

  if (pull.active) return output(Phase::Pull, lastColor_, cfg.brightness);

  if (fading_) {
    const uint32_t since = elapsed(now, pullEnd_);
    if (since < app::FADE_MS) {
      LedFrame f = output(Phase::Fade, lastColor_, cfg.brightness);
      const uint8_t k = color::fadeFactor(since, app::FADE_MS);
      f.brightness = (uint8_t)(f.brightness * k / 255);
      f.pixel = color::scale(lastColor_, f.brightness);
      return f;
    }
    fading_ = false;
  }

  LedFrame f = output(Phase::Idle, {0, 0, 0}, cfg.brightness);
  if (cfg.idleGlow) {
    f.glow = true;
    f.pixel = {0, app::IDLE_GLOW_LEVEL, 0};
  }
  return f;
}
