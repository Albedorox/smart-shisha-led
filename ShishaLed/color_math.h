// Reine Farbfunktionen: Verlauf, Fade-out, Helligkeit und Stromgrenze (04 §6)
#pragma once

#include <stdint.h>

struct Rgb {
  uint8_t r, g, b;
};

inline bool operator==(const Rgb& a, const Rgb& b) { return a.r == b.r && a.g == b.g && a.b == b.b; }

namespace color {

// a + ((b − a)·n + d/2) / d in Ganzzahlarithmetik (04 §6.2)
int32_t lerp(int32_t a, int32_t b, int32_t n, int32_t d);

// Grün (0) → Gelb (yellowMs) → Rot (redMs), danach Rot. Erwartet redMs > yellowMs > 0.
Rgb gradient(uint32_t t, uint32_t yellowMs, uint32_t redMs);

// 255 bei 0 ms, linear auf 0 bei fadeMs (04 §6.3)
uint8_t fadeFactor(uint32_t sinceEndMs, uint32_t fadeMs);

// Kanal · factor / 255
Rgb scale(Rgb c, uint8_t factor);

// Geschätzter Strom, wenn alle `ledCount` LEDs `c` mit Helligkeit `brightness` zeigen
uint32_t estimateCurrentMa(Rgb c, uint8_t brightness, uint16_t ledCount, uint16_t maPerChannel);

struct Limited {
  uint8_t brightness;  // wirksame Helligkeit B_eff
  bool    limited;     // B_eff < B
};

// Helligkeit so weit senken, dass der geschätzte Strom ≤ limitMa bleibt (04 §6.4)
Limited limitBrightness(Rgb c, uint8_t brightness, uint16_t ledCount, uint16_t maPerChannel,
                        uint16_t limitMa);

}  // namespace color
