#include "sim.h"
#include "test.h"

TEST("Smart: Ruhe mit Bereitschaftsglimmen bzw. aus (D-30)") {
  Sim s;
  s.idle(100);
  CHECK(s.frame.phase == Phase::Idle);
  CHECK(s.frame.glow);
  CHECK(s.frame.pixel == (Rgb{0, app::IDLE_GLOW_LEVEL, 0}));
  ConfigPatch p;
  p.hasIdleGlow = true;
  p.idleGlow = false;
  CHECK(s.cfg.apply(p).ok());
  s.idle(10);
  CHECK(!s.frame.glow);
  CHECK(s.frame.pixel == (Rgb{0, 0, 0}));
}

TEST("LED-02 Smart: Verlauf im Zug, danach Fade-out") {
  Sim s;
  s.eval(true);
  CHECK(s.frame.phase == Phase::Pull);
  CHECK(s.frame.color == (Rgb{0, 255, 0}));
  CHECK(s.frame.pixel == (Rgb{0, 64, 0}));
  for (uint32_t t = 10; t <= 3000; t += 10) s.eval(t % 500 == 0);  // letzter Impuls bei 3000
  CHECK(s.frame.color == (Rgb{255, 255, 0}));
  s.idle(990);
  CHECK(s.frame.phase == Phase::Pull);
  s.eval(false);  // t = 4000: Zugende
  CHECK(s.frame.phase == Phase::Fade);
  CHECK_EQ(s.frame.brightness, 64);
  s.idle(500);
  CHECK(s.frame.phase == Phase::Fade);
  CHECK_NEAR(s.frame.brightness, 64 * 127 / 255, 1);
  CHECK(s.frame.color == color::gradient(3990, 3000, 6000));  // letzte Zugfarbe
  s.idle(500);
  CHECK(s.frame.phase == Phase::Idle);
}

TEST("T-FW-11 Moduswechsel im Zug ändert die Dauer nicht") {
  Sim a, b;
  for (Sim* s : {&a, &b}) {
    for (uint32_t t = 0; t <= 4000; t += 10) {
      if (s == &b && t == 2000) s->setMode(Mode::Steady);
      if (s == &b && t == 2500) s->setMode(Mode::Off);
      if (s == &b && t == 3000) s->setMode(Mode::Smart);
      s->eval(t % 400 == 0);
      if (s == &b && t == 2200) {
        CHECK(s->frame.phase == Phase::Pull);
        CHECK(s->frame.color == (Rgb{255, 160, 60}));
      }
      if (s == &b && t == 2700) {
        CHECK(s->frame.phase == Phase::Pull);
        CHECK(s->frame.pixel == (Rgb{0, 0, 0}));
      }
      if (s == &b && t == 3500) CHECK(s->frame.color == color::gradient(3500, 3000, 6000));
    }
    while (s->done.empty()) s->eval(false);
  }
  CHECK_EQ(a.done[0].durationMs, b.done[0].durationMs);
}

TEST("T-FW-13 neuer Zug im Fade-out startet wieder bei Grün") {
  Sim s;
  s.pullFor(5000);
  CHECK(s.frame.phase == Phase::Fade);
  s.idle(390);
  s.eval(true);  // 400 ms nach Zugende
  CHECK(s.frame.phase == Phase::Pull);
  CHECK(s.frame.color == (Rgb{0, 255, 0}));
}

TEST("Wechsel weg von Smart bricht den Fade-out ab") {
  Sim s;
  s.pullFor(2000);
  CHECK(s.frame.phase == Phase::Fade);
  s.setMode(Mode::Steady);
  s.eval(false);
  CHECK(s.frame.phase == Phase::Idle);
  CHECK(s.frame.color == (Rgb{255, 160, 60}));
  s.setMode(Mode::Smart);
  s.eval(false);
  CHECK(s.frame.phase == Phase::Idle);  // Fade kommt nicht zurück
}

TEST("Off: LEDs aus, Züge werden weiter erfasst (LED-04)") {
  Sim s;
  s.setMode(Mode::Off);
  s.pullFor(2500);
  CHECK(s.frame.pixel == (Rgb{0, 0, 0}));
  CHECK(!s.frame.glow);
  CHECK_EQ(s.ses.pullCount(), 1);
}

TEST("Dauerlicht mit Stromgrenze") {
  Sim s;
  ConfigPatch p;
  p.hasMode = true; p.mode = Mode::Steady;
  p.hasSteadyColor = true; p.steadyR = 255; p.steadyG = 255; p.steadyB = 255;
  p.hasBrightness = true; p.brightness = 255;
  CHECK(s.cfg.apply(p).ok());
  s.eval(false);
  CHECK(s.frame.limited);
  CHECK_NEAR(s.frame.brightness, 88, 1);
  CHECK(s.frame.pixel == (Rgb{88, 88, 88}));
}
