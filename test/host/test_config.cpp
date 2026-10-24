#include "config_manager.h"
#include "test.h"

namespace {

bool sameConfig(const Config& a, const Config& b) {
  return a.mode == b.mode && a.yellowMs == b.yellowMs && a.redMs == b.redMs && a.tailMs == b.tailMs &&
         a.brightness == b.brightness && a.steadyColor == b.steadyColor && a.diagnostic == b.diagnostic &&
         a.idleGlow == b.idleGlow;
}

}  // namespace

TEST("Konfiguration: Standardwerte (01 §9)") {
  ConfigManager m;
  const Config& c = m.get();
  CHECK(c.mode == Mode::Smart);
  CHECK_EQ(c.yellowMs, 3000);
  CHECK_EQ(c.redMs, 6000);
  CHECK_EQ(c.tailMs, 1000);
  CHECK_EQ(c.brightness, 64);
  CHECK(c.steadyColor == (Rgb{255, 160, 60}));
  CHECK(!c.diagnostic);
  CHECK(c.idleGlow);
  CHECK_EQ(m.version(), 0);
}

TEST("Konfiguration: gültiges Teil-Update erhöht configVersion") {
  ConfigManager m;
  ConfigPatch p;
  p.hasMode = true; p.mode = Mode::Steady;
  p.hasBrightness = true; p.brightness = 80;
  p.hasSteadyColor = true; p.steadyR = 0; p.steadyG = 120; p.steadyB = 255;
  p.hasYellowMs = true; p.yellowMs = 2500;
  p.hasRedMs = true; p.redMs = 5000;
  p.hasTailMs = true; p.tailMs = 1200;
  p.hasIdleGlow = true; p.idleGlow = false;
  CHECK(m.apply(p).ok());
  CHECK_EQ(m.version(), 1);
  CHECK(m.get().mode == Mode::Steady);
  CHECK_EQ(m.get().brightness, 80);
  CHECK(m.get().steadyColor == (Rgb{0, 120, 255}));
  CHECK_EQ(m.get().yellowMs, 2500);
  CHECK_EQ(m.get().redMs, 5000);
  CHECK_EQ(m.get().tailMs, 1200);
  CHECK(!m.get().idleGlow);
}

TEST("Konfiguration: leerer Patch") {
  ConfigManager m;
  const ApplyResult r = m.apply(ConfigPatch());
  CHECK(r.error == ConfigError::EmptyPatch);
  CHECK_EQ(m.version(), 0);
}

TEST("Konfiguration: Bereiche und Raster (D-09)") {
  struct Case { ConfigPatch p; ConfigField f; };
  auto make = [](auto set) { ConfigPatch p; set(p); return p; };
  const Case bad[] = {
      {make([](ConfigPatch& p) { p.hasBrightness = true; p.brightness = 300; }), ConfigField::Brightness},
      {make([](ConfigPatch& p) { p.hasBrightness = true; p.brightness = -1; }), ConfigField::Brightness},
      {make([](ConfigPatch& p) { p.hasSteadyColor = true; p.steadyG = 256; }), ConfigField::SteadyColor},
      {make([](ConfigPatch& p) { p.hasYellowMs = true; p.yellowMs = 400; }), ConfigField::YellowMs},
      {make([](ConfigPatch& p) { p.hasYellowMs = true; p.yellowMs = 2550; }), ConfigField::YellowMs},
      {make([](ConfigPatch& p) { p.hasRedMs = true; p.redMs = 60100; }), ConfigField::RedMs},
      {make([](ConfigPatch& p) { p.hasTailMs = true; p.tailMs = 150; }), ConfigField::TailMs},
      {make([](ConfigPatch& p) { p.hasTailMs = true; p.tailMs = 5050; }), ConfigField::TailMs},
      {make([](ConfigPatch& p) { p.hasTailMs = true; p.tailMs = 1025; }), ConfigField::TailMs},
  };
  for (const Case& c : bad) {
    ConfigManager m;
    const ApplyResult r = m.apply(c.p);
    CHECK(r.error == ConfigError::OutOfRange);
    CHECK(r.field == c.f);
    CHECK(sameConfig(m.get(), Config()));
    CHECK_EQ(m.version(), 0);
  }
  ConfigManager m;
  ConfigPatch ok;
  ok.hasYellowMs = true; ok.yellowMs = 500;
  ok.hasRedMs = true; ok.redMs = 600;
  ok.hasTailMs = true; ok.tailMs = 5000;
  ok.hasBrightness = true; ok.brightness = 0;
  CHECK(m.apply(ok).ok());
}

TEST("Konfiguration: Rot ≥ Gelb + 100 mit resultierenden Werten (LED-07)") {
  ConfigManager m;
  ConfigPatch p;
  p.hasRedMs = true; p.redMs = 2000;  // Gelb steht auf 3000
  ApplyResult r = m.apply(p);
  CHECK(r.error == ConfigError::ValidationFailed);
  CHECK(r.field == ConfigField::RedMs);

  ConfigPatch q;
  q.hasYellowMs = true; q.yellowMs = 6000;  // Rot steht auf 6000
  r = m.apply(q);
  CHECK(r.error == ConfigError::ValidationFailed);
  CHECK(r.field == ConfigField::YellowMs);

  ConfigPatch both;  // beide zusammen gültig, obwohl einzeln ungültig
  both.hasYellowMs = true; both.yellowMs = 8000;
  both.hasRedMs = true; both.redMs = 8100;
  CHECK(m.apply(both).ok());
  CHECK_EQ(m.get().yellowMs, 8000);
  CHECK_EQ(m.get().redMs, 8100);
}

TEST("Konfiguration: alles oder nichts (API-02)") {
  ConfigManager m;
  ConfigPatch p;
  p.hasMode = true; p.mode = Mode::Off;
  p.hasBrightness = true; p.brightness = 100;
  p.hasTailMs = true; p.tailMs = 99;  // ungültig
  CHECK(!m.apply(p).ok());
  CHECK(sameConfig(m.get(), Config()));
}

TEST("Konfiguration: Reset je Bereich (06 §5.2)") {
  ConfigManager m;
  ConfigPatch p;
  p.hasMode = true; p.mode = Mode::Off;
  p.hasBrightness = true; p.brightness = 10;
  p.hasIdleGlow = true; p.idleGlow = false;
  p.hasYellowMs = true; p.yellowMs = 1000;
  p.hasRedMs = true; p.redMs = 2000;
  p.hasTailMs = true; p.tailMs = 300;
  p.hasDiagnostic = true; p.diagnostic = true;
  CHECK(m.apply(p).ok());

  m.reset(ResetScope::Lighting);
  CHECK(m.get().mode == Mode::Smart);
  CHECK_EQ(m.get().brightness, 64);
  CHECK(m.get().idleGlow);
  CHECK_EQ(m.get().yellowMs, 1000);  // unberührt
  CHECK_EQ(m.get().tailMs, 300);

  m.reset(ResetScope::Gradient);
  CHECK_EQ(m.get().yellowMs, 3000);
  CHECK_EQ(m.get().redMs, 6000);
  CHECK(m.get().diagnostic);

  m.reset(ResetScope::Sensor);
  CHECK_EQ(m.get().tailMs, 1000);
  CHECK(!m.get().diagnostic);
  CHECK_EQ(m.version(), 4);

  CHECK(m.apply(p).ok());
  m.reset(ResetScope::All);
  CHECK(sameConfig(m.get(), Config()));
}
