// Simulation der Hauptschleife (04 §5) im 10-ms-Raster für die Host-Tests
#pragma once

#include <vector>

#include "config_manager.h"
#include "led_logic.h"
#include "pull_detector.h"
#include "session_manager.h"
#include "time_util.h"

struct Sim {
  uint32_t        now;
  Uptime          up;
  PullDetector    det;
  SessionManager  ses;
  ConfigManager   cfg;
  LedLogic        led;
  LedFrame        frame;
  std::vector<PullRecord> done;
  int             starts = 0;

  explicit Sim(uint32_t start = 0) : now(start) { up.begin(start); }

  // Eine Auswertung zum Zeitpunkt now, danach 10 ms weiter
  void eval(bool impulse, bool level = false) {
    const uint64_t u = up.update(now);
    det.evaluate(now, u, impulse, level, cfg.get().tailMs);
    uint32_t seq;
    if (det.takeStartedPull(&seq)) {
      ses.onPullStarted(seq);
      starts++;
    }
    PullRecord r;
    while (det.takeCompletedPull(&r)) {
      ses.onPullCompleted(r, cfg.get().diagnostic);
      done.push_back(r);
    }
    frame = led.update(now, cfg.get(), det.state());
    now += 10;
  }

  void idle(uint32_t ms) {
    for (uint32_t t = 0; t < ms; t += 10) eval(false);
  }

  // Zug mit genau durationMs (≥ tailMs): Impulse alle 500 ms bis durationMs − tailMs,
  // dann Ruhe bis zur Zugende-Erkennung.
  PullRecord pullFor(uint32_t durationMs) {
    const uint32_t span = durationMs - cfg.get().tailMs;
    const size_t before = done.size();
    for (uint32_t t = 0; done.size() == before; t += 10) {
      eval(t == 0 || (t <= span && t % 500 == 0) || t == span);
    }
    return done.back();
  }

  void setMode(Mode m) {
    ConfigPatch p;
    p.hasMode = true;
    p.mode = m;
    cfg.apply(p);
  }
};
