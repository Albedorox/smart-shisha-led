#include "sim.h"
#include "test.h"

TEST("T-FW-02 Zugstart: Impuls bei t = 100 → aktiv ab Auswertung ≤ 110") {
  Sim s;
  s.idle(100);  // Auswertungen bei 0 … 90
  CHECK(!s.det.state().active);
  s.eval(true);  // t = 100
  CHECK(s.det.state().active);
  CHECK_EQ(s.det.state().startMs, 100);
  CHECK_EQ(s.starts, 1);
}

TEST("T-FW-03 Impulse alle 900 ms über 10 s ergeben einen Zug") {
  Sim s;
  uint32_t last = 0;
  for (uint32_t t = 0; t <= 10000; t += 10) {
    const bool imp = t % 900 == 0;
    if (imp) last = t;
    s.eval(imp);
    CHECK(s.det.state().active);
  }
  while (s.done.empty()) s.eval(false);
  CHECK_EQ(s.starts, 1);
  CHECK_EQ(s.done.size(), 1);
  CHECK_EQ(s.done[0].durationMs, last + 1000);
}

TEST("T-FW-04 Zugende bei letztem Impuls + tailMs") {
  Sim s;
  for (uint32_t t = 0; t <= 2000; t += 10) s.eval(t % 500 == 0);  // L = 2000
  while (s.done.empty()) s.eval(false);
  const uint32_t endAt = s.now - 10;  // Zeitpunkt der erkennenden Auswertung
  CHECK(endAt >= 3000 && endAt <= 3010);
}

TEST("T-FW-05 Einzelimpuls: Dauer = tailMs ± 10") {
  Sim s;
  s.eval(true);
  s.idle(1500);
  CHECK_EQ(s.done.size(), 1);
  CHECK_NEAR(s.done[0].durationMs, 1000, 10);
  CHECK(!s.done[0].truncated);
}

TEST("T-FW-05 Nachlaufzeit ändern wirkt sofort, auch im laufenden Zug") {
  Sim s;
  s.eval(true);
  s.idle(300);
  ConfigPatch p;
  p.hasTailMs = true;
  p.tailMs = 200;
  CHECK(s.cfg.apply(p).ok());
  s.eval(false);
  CHECK_EQ(s.done.size(), 1);
  CHECK_EQ(s.done[0].durationMs, 310);
}

TEST("T-FW-06 100 Züge, jeder genau einmal") {
  Sim s;
  for (int i = 0; i < 100; i++) {
    s.pullFor(1000 + (uint32_t)i * 10);
    s.idle(200);
    PullRecord r;
    CHECK(!s.det.takeCompletedPull(&r));  // nichts doppelt im Puffer
  }
  CHECK_EQ(s.done.size(), 100);
  CHECK_EQ(s.ses.pullCount(), 100);
  for (size_t i = 0; i < s.done.size(); i++) CHECK_EQ(s.done[i].seq, i + 1);
  CHECK_EQ(s.ses.entryCount(), 5);
  CHECK_EQ(s.ses.entry(0).durationMs, 1990);
}

TEST("T-FW-07 Dauersignal: Fehler nach 5 s, Zugende bei 60 s, danach erst Ruhe") {
  Sim s;
  bool faultAt5 = false;
  for (uint32_t t = 0; t < 70000; t += 10) {
    s.eval(true, true);
    if (t == 4990) CHECK(s.det.diag(s.now, s.up.ms()).fault == SensorFault::None);
    if (t == 5000) faultAt5 = s.det.diag(s.now, s.up.ms()).fault == SensorFault::StuckActive;
  }
  CHECK(faultAt5);
  CHECK_EQ(s.done.size(), 1);
  CHECK(s.done[0].truncated);
  CHECK_EQ(s.done[0].durationMs, 60000);
  CHECK(!s.det.state().active);  // kein neuer Zug trotz Dauersignal
  CHECK_EQ(s.starts, 1);

  s.idle(500);  // Pegel inaktiv, aber kürzer als tailMs
  s.eval(true);
  CHECK(!s.det.state().active);
  s.idle(1000);  // jetzt ≥ tailMs Ruhe
  CHECK(s.det.diag(s.now, s.up.ms()).fault == SensorFault::None);
  s.eval(true);
  CHECK(s.det.state().active);
  CHECK_EQ(s.starts, 2);
}

TEST("T-FW-22 millis()-Überlauf während Zug und Fade") {
  Sim s(0xFFFFFFFFu - 3000 + 1);  // Start 2^32 − 3000
  s.idle(2000);
  s.eval(true);                   // Zugstart 1 s vor dem Überlauf
  for (int i = 0; i < 250; i++) s.eval(i % 50 == 0);  // letzter Impuls 2010 ms nach Start
  while (s.done.empty()) s.eval(false);
  CHECK(s.now < 10000);  // Überlauf liegt hinter uns
  CHECK_EQ(s.done[0].durationMs, 2010 + 1000);
  CHECK(s.frame.phase == Phase::Fade);
  s.idle(500);
  CHECK(s.frame.phase == Phase::Fade);
  s.idle(500);
  CHECK(s.frame.phase == Phase::Idle);
  CHECK_EQ(s.ses.entry(0).durationMs, 3010);
  CHECK(s.up.ms() > 5000 && s.up.ms() < 7000);
}

TEST("Auswerteraster: kein Nachholen nach Verspätung (SEN-01)") {
  IntervalTimer t(10);
  t.begin(1000);
  CHECK(t.due(1000));
  CHECK(!t.due(1005));
  CHECK(t.due(1010));
  CHECK(t.due(1075));   // 55 ms zu spät: genau eine Auswertung
  CHECK(!t.due(1080));
  CHECK(t.due(1085));
  t.begin(0xFFFFFFFAu);  // über den Überlauf
  CHECK(t.due(0xFFFFFFFAu));
  CHECK(!t.due(0xFFFFFFFFu));
  CHECK(t.due(4));
}

TEST("Diagnose: Flanken im 10-s-Fenster, letzter Impuls") {
  PullDetector d;
  CHECK(!d.diag(0, 0).hasPulse);
  d.addEdges(3, 500);
  d.addEdges(2, 4500);
  d.evaluate(4500, 4500, true, false, 1000);
  SensorDiag g = d.diag(5000, 5000);
  CHECK_EQ(g.pulsesInWindow, 5);
  CHECK(g.hasPulse);
  CHECK_EQ(g.lastPulseAgoMs, 500);
  g = d.diag(10600, 10600);  // Sekunde 0 fällt heraus
  CHECK_EQ(g.pulsesInWindow, 2);
  g = d.diag(15000, 15000);
  CHECK_EQ(g.pulsesInWindow, 0);
}
