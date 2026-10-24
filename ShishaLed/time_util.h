// Zeitrechnung mit millis(): überlaufsichere Differenzen, 64-bit-Uptime, 10-ms-Raster
#pragma once

#include <stdint.h>

// Vergangene Zeit seit `since`, korrekt über den uint32-Überlauf (≈ 49,7 Tage) hinweg.
inline uint32_t elapsed(uint32_t now, uint32_t since) { return (uint32_t)(now - since); }

// Laufzeit in 64 bit; update() muss mindestens einmal je 49 Tage aufgerufen werden.
class Uptime {
 public:
  void begin(uint32_t now) { last_ = now; total_ = 0; }
  uint64_t update(uint32_t now) {
    total_ += elapsed(now, last_);
    last_ = now;
    return total_;
  }
  uint64_t ms() const { return total_; }

 private:
  uint32_t last_  = 0;
  uint64_t total_ = 0;
};

// Festes Raster ohne Nachholen: Nach einer Verspätung folgt genau eine Auswertung,
// der nächste Termin ist dann now + interval (04 §4.1).
class IntervalTimer {
 public:
  explicit IntervalTimer(uint32_t intervalMs) : interval_(intervalMs) {}
  void begin(uint32_t now) { next_ = now; }
  bool due(uint32_t now) {
    if ((int32_t)(now - next_) < 0) return false;
    next_ = now + interval_;
    return true;
  }

 private:
  uint32_t interval_;
  uint32_t next_ = 0;
};
