// Zug-Zustandsmaschine und Sensordiagnose ohne Hardwarezugriff (04 §4).
// Der SensorManager (B2) liefert alle 10 ms Impuls und Pegel aus ISR-Latch und Pin.
#pragma once

#include <stdint.h>

#include "app_config.h"

struct PullState {
  bool     active      = false;
  uint32_t startMs     = 0;   // millis() des ersten Impulses
  uint32_t lastPulseMs = 0;
  uint32_t seq         = 0;   // laufende Zugnummer seit Start
};

struct PullRecord {
  uint32_t seq        = 0;
  uint32_t durationMs = 0;
  uint64_t endedAtMs  = 0;    // Uptime 64 bit
  bool     truncated  = false;
};

enum class SensorFault : uint8_t { None, StuckActive };

struct SensorDiag {
  bool        levelActive;
  bool        hasPulse;
  uint32_t    lastPulseAgoMs;   // nur gültig mit hasPulse
  uint16_t    pulsesInWindow;   // Flanken in den letzten 10 s
  SensorFault fault;
};

class PullDetector {
 public:
  // Eine Auswertung im 10-ms-Raster. impulse = Pegel aktiv ODER Latch gesetzt.
  void evaluate(uint32_t now, uint64_t uptimeMs, bool impulse, bool levelActive, uint32_t tailMs,
                uint32_t maxPullMs = app::MAX_PULL_MS);

  // Flanken aus dem ISR-Zähler für das Diagnosefenster
  void addEdges(uint32_t count, uint64_t uptimeMs);

  const PullState& state() const { return state_; }

  // Ereignis „Zug gestartet“, genau einmal je Zug
  bool takeStartedPull(uint32_t* seq);
  // Abgeschlossener Zug, genau einmal je Zug
  bool takeCompletedPull(PullRecord* rec);

  SensorDiag diag(uint32_t now, uint64_t uptimeMs) const;

 private:
  static constexpr uint8_t QUEUE = 4;

  void finish(uint32_t now, uint64_t uptimeMs, bool truncated);

  PullState  state_;
  bool       waitQuiet_    = false;   // nach Maximaldauer erst Ruhe abwarten
  bool       startPending_ = false;
  PullRecord queue_[QUEUE];
  uint8_t    qHead_ = 0, qCount_ = 0;

  bool        hasPulse_     = false;
  uint32_t    lastPulseAny_ = 0;
  bool        levelActive_  = false;
  bool        stuckRun_     = false;   // Pegel ununterbrochen aktiv
  uint32_t    activeSince_  = 0;
  uint32_t    inactiveSince_ = 0;
  SensorFault fault_        = SensorFault::None;

  uint32_t bucketCount_[app::DIAG_WINDOW_S] = {};
  uint64_t bucketSecond_[app::DIAG_WINDOW_S] = {};
};
