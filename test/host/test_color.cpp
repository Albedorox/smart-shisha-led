#include "color_math.h"
#include "test.h"

using color::gradient;

TEST("T-FW-10 Farbverlauf Referenzwerte (04 §6.2)") {
  struct { uint32_t t; Rgb c; } ref[] = {
      {0, {0, 255, 0}},       {1500, {128, 255, 0}}, {3000, {255, 255, 0}},
      {4500, {255, 128, 0}},  {6000, {255, 0, 0}},   {10000, {255, 0, 0}},
  };
  for (auto& r : ref) {
    const Rgb c = gradient(r.t, 3000, 6000);
    CHECK_NEAR(c.r, r.c.r, 1);
    CHECK_NEAR(c.g, r.c.g, 1);
    CHECK_EQ(c.b, 0);
  }
}

TEST("T-FW-10 Farbverlauf monoton, auch bei Grenzwerten") {
  const uint32_t cases[][2] = {{3000, 6000}, {500, 600}, {30000, 60000}};
  for (auto& yr : cases) {
    Rgb prev = gradient(0, yr[0], yr[1]);
    CHECK(prev == (Rgb{0, 255, 0}));
    for (uint32_t t = 10; t <= yr[1] + 1000; t += 10) {
      const Rgb c = gradient(t, yr[0], yr[1]);
      CHECK(c.r >= prev.r);  // Rot steigt bis Gelb
      CHECK(c.g <= prev.g);  // Grün fällt ab Gelb
      if (t < yr[0]) CHECK_EQ(c.g, 255);
      if (t >= yr[0]) CHECK_EQ(c.r, 255);
      prev = c;
    }
    CHECK(gradient(yr[0], yr[0], yr[1]) == (Rgb{255, 255, 0}));
    CHECK(gradient(yr[1], yr[0], yr[1]) == (Rgb{255, 0, 0}));
  }
}

TEST("T-FW-12 Fade-out-Faktor") {
  CHECK_NEAR(color::fadeFactor(0, 1000), 255, 3);
  CHECK_NEAR(color::fadeFactor(250, 1000), 191, 3);
  CHECK_NEAR(color::fadeFactor(500, 1000), 127, 3);
  CHECK_NEAR(color::fadeFactor(750, 1000), 64, 3);
  CHECK_EQ(color::fadeFactor(1000, 1000), 0);
  CHECK_EQ(color::fadeFactor(5000, 1000), 0);
}

TEST("T-FW-14 Stromgrenze Weiß, B = 255, 24 LEDs, 500 mA") {
  const Rgb white = {255, 255, 255};
  CHECK_EQ(color::estimateCurrentMa(white, 255, 24, 20), 1440);
  const color::Limited l = color::limitBrightness(white, 255, 24, 20, 500);
  CHECK_NEAR(l.brightness, 88, 1);
  CHECK(l.limited);
}

TEST("Stromgrenze greift nicht unterhalb des Limits") {
  const color::Limited l = color::limitBrightness({255, 255, 0}, 64, 24, 20, 500);  // ≈ 240 mA
  CHECK_EQ(l.brightness, 64);
  CHECK(!l.limited);
  CHECK_NEAR(color::estimateCurrentMa({255, 255, 0}, 64, 24, 20), 240, 1);
}

TEST("Skalierung") {
  CHECK(color::scale({255, 128, 0}, 255) == (Rgb{255, 128, 0}));
  CHECK(color::scale({255, 128, 0}, 0) == (Rgb{0, 0, 0}));
  CHECK(color::scale({255, 255, 255}, 64) == (Rgb{64, 64, 64}));
}
