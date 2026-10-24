#include "color_math.h"

namespace color {

int32_t lerp(int32_t a, int32_t b, int32_t n, int32_t d) { return a + ((b - a) * n + d / 2) / d; }

Rgb gradient(uint32_t t, uint32_t yellowMs, uint32_t redMs) {
  if (t == 0) return {0, 255, 0};
  if (t < yellowMs) return {(uint8_t)lerp(0, 255, (int32_t)t, (int32_t)yellowMs), 255, 0};
  if (t < redMs) {
    return {255, (uint8_t)lerp(255, 0, (int32_t)(t - yellowMs), (int32_t)(redMs - yellowMs)), 0};
  }
  return {255, 0, 0};
}

uint8_t fadeFactor(uint32_t sinceEndMs, uint32_t fadeMs) {
  if (sinceEndMs >= fadeMs) return 0;
  return (uint8_t)(255 - (255 * sinceEndMs + fadeMs / 2) / fadeMs);
}

Rgb scale(Rgb c, uint8_t factor) {
  return {(uint8_t)(c.r * factor / 255), (uint8_t)(c.g * factor / 255), (uint8_t)(c.b * factor / 255)};
}

uint32_t estimateCurrentMa(Rgb c, uint8_t brightness, uint16_t ledCount, uint16_t maPerChannel) {
  const uint32_t sum = (uint32_t)c.r + c.g + c.b;
  return (uint32_t)ledCount * maPerChannel * sum * brightness / (255u * 255u);
}

Limited limitBrightness(Rgb c, uint8_t brightness, uint16_t ledCount, uint16_t maPerChannel,
                        uint16_t limitMa) {
  const uint32_t ma = estimateCurrentMa(c, brightness, ledCount, maPerChannel);
  if (ma <= limitMa) return {brightness, false};
  return {(uint8_t)((uint32_t)brightness * limitMa / ma), true};
}

}  // namespace color
