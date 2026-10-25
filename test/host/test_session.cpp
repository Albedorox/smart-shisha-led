#include <cstring>
#include <string>

#include "sim.h"
#include "test.h"

namespace {

void name(Sim& s, const char* n) { s.ses.setName(n, strlen(n)); }

}  // namespace

TEST("T-FW-08 Diagnosemodus: Rangliste und Zähler unverändert") {
  Sim s;
  s.pullFor(2000);
  const uint32_t v = s.ses.lbVersion();
  ConfigPatch p;
  p.hasDiagnostic = true;
  p.diagnostic = true;
  CHECK(s.cfg.apply(p).ok());
  for (int i = 0; i < 3; i++) s.pullFor(5000);
  CHECK_EQ(s.ses.entryCount(), 1);
  CHECK_EQ(s.ses.pullCount(), 1);
  CHECK_EQ(s.ses.lbVersion(), v);
  CHECK(s.ses.hasLastPull());
  CHECK(s.ses.lastPull().diagnostic);
  CHECK_EQ(s.ses.lastPull().durationMs, 5000);
}

TEST("T-FW-15 Kapazität: 8 Züge → Top 5, pullCount 8") {
  Sim s;
  for (uint32_t d = 1; d <= 8; d++) s.pullFor(d * 1000);
  CHECK_EQ(s.ses.entryCount(), 5);
  CHECK_EQ(s.ses.pullCount(), 8);
  for (uint8_t i = 0; i < 5; i++) CHECK_EQ(s.ses.entry(i).durationMs, (8 - i) * 1000);
  CHECK_EQ(s.ses.lastPull().durationMs, 8000);
}

TEST("T-FW-15 kurzer Zug bei voller Rangliste wird verworfen, zählt aber") {
  Sim s;
  for (uint32_t d = 2; d <= 6; d++) s.pullFor(d * 1000);
  const uint32_t v = s.ses.lbVersion();
  s.pullFor(1000);
  CHECK_EQ(s.ses.entryCount(), 5);
  CHECK_EQ(s.ses.entry(4).durationMs, 2000);
  CHECK_EQ(s.ses.pullCount(), 6);
  CHECK(s.ses.lbVersion() > v);  // Zähler geändert
}

TEST("T-FW-16 Gleichstand: früher beendeter Zug zuerst") {
  Sim s;
  name(s, "A"); s.pullFor(3000);
  name(s, "B"); s.pullFor(3000);
  name(s, "C"); s.pullFor(3000);
  CHECK_EQ(s.ses.entryCount(), 3);
  CHECK(std::string(s.ses.entry(0).name) == "A");
  CHECK(std::string(s.ses.entry(1).name) == "B");
  CHECK(std::string(s.ses.entry(2).name) == "C");

  // Rangliste voll mit 3000ern: ein weiterer 3000er bleibt draußen
  name(s, "D"); s.pullFor(3000);
  name(s, "E"); s.pullFor(3000);
  name(s, "F"); s.pullFor(3000);
  CHECK(std::string(s.ses.entry(4).name) == "E");
}

TEST("T-FW-17 Reset während eines Zuges") {
  Sim s;
  s.pullFor(4000);
  s.eval(true);  // neuer Zug läuft
  s.ses.resetLeaderboard();
  CHECK_EQ(s.ses.entryCount(), 0);
  CHECK_EQ(s.ses.pullCount(), 0);
  CHECK(s.ses.hasLastPull());  // letzter Zug bleibt (D-11)
  CHECK(s.det.state().active);
  while (s.done.size() < 2) s.eval(false);
  CHECK_EQ(s.ses.entryCount(), 1);
  CHECK_EQ(s.ses.pullCount(), 1);
  CHECK_EQ(s.ses.entry(0).durationMs, 1000);
}

TEST("T-FW-18 Name wird beim Zugbeginn übernommen") {
  Sim s;
  name(s, "A");
  s.eval(true);
  name(s, "B");
  CHECK(std::string(s.ses.pullName()) == "A");
  CHECK(std::string(s.ses.currentName()) == "B");
  while (s.done.empty()) s.eval(false);
  CHECK(std::string(s.ses.entry(0).name) == "A");
  s.pullFor(1500);
  CHECK(std::string(s.ses.entry(0).name) == "B");
}

TEST("Standardname und Normalisierung") {
  Sim s;
  CHECK(std::string(s.ses.currentName()) == "Unbekannt");
  s.pullFor(1000);
  CHECK(std::string(s.ses.entry(0).name) == "Unbekannt");
  CHECK(s.ses.setName("  ", 2) == utf8::NameResult::Normalized);
  CHECK(s.ses.setName("Name\x01", 5) == utf8::NameResult::Invalid);
  name(s, "Mia");
  CHECK(s.ses.setName("\xC0\x80", 2) == utf8::NameResult::Invalid);
  CHECK(std::string(s.ses.currentName()) == "Mia");  // ungültig ändert nichts
}
