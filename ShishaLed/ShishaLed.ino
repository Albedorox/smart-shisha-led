// Smart Shisha LED-Untersetzer – Einstieg (übernimmt die Rolle von main.cpp, D-19)
//
// Rohfassung B2/B3 (2026-10-24): Sensor (ISR-Latch), LED-Ring (Adafruit NeoPixel),
// offenes WLAN mit Web-App und HTTP-API, Konfiguration im Flash (Preferences).
// Die Kernlogik (Zugerkennung, Rangliste, LED-Logik) ist per Host-Tests geprüft;
// dieser Sketch verbindet sie nur mit der Hardware. Offene Messwerte (B0) sind
// als Annahmen in app_config.h hinterlegt.
//
// Bibliotheken: Adafruit NeoPixel (Bibliotheksverwalter); WiFi, WebServer und
// Preferences gehören zum ESP32-Arduino-Core.

#include <Adafruit_NeoPixel.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>

#include "app_config.h"
#include "config_manager.h"
#include "led_logic.h"
#include "pull_detector.h"
#include "session_manager.h"
#include "time_util.h"
#include "web_page.h"

namespace {

Adafruit_NeoPixel ring(app::LED_COUNT, app::PIN_LED, NEO_GRB + NEO_KHZ800);
WebServer         server(80);
Preferences       prefs;

ConfigManager  cfg;
PullDetector   det;
SessionManager ses;
LedLogic       led;
LedFrame       frame;
Uptime         up;
IntervalTimer  evalTimer(app::SENSOR_EVAL_MS);
IntervalTimer  frameTimer(app::LED_FRAME_MS);

volatile bool     sensorLatch = false;
volatile uint32_t sensorEdges = 0;
uint32_t          savedVersion = 0;
uint32_t          saveDueMs    = 0;
bool              savePending  = false;
LedFrame          shown;
bool              hasShown = false;

void IRAM_ATTR onSensorEdge() {
  sensorLatch = true;
  sensorEdges = sensorEdges + 1;
}

bool sensorLevelActive() {
  const bool high = digitalRead(app::PIN_SENSOR) == HIGH;
  return app::SENSOR_ACTIVE_LOW ? !high : high;
}

// ---- Konfiguration im Flash ----------------------------------------------

void loadConfig() {
  prefs.begin("shisha", true);
  ConfigPatch p;
  if (prefs.isKey("mode")) { p.hasMode = true; p.mode = (Mode)prefs.getUChar("mode"); }
  if (prefs.isKey("bri")) { p.hasBrightness = true; p.brightness = prefs.getUChar("bri"); }
  if (prefs.isKey("col")) {
    const uint32_t c = prefs.getULong("col");
    p.hasSteadyColor = true;
    p.steadyR = (c >> 16) & 0xFF; p.steadyG = (c >> 8) & 0xFF; p.steadyB = c & 0xFF;
  }
  if (prefs.isKey("yel")) { p.hasYellowMs = true; p.yellowMs = prefs.getUShort("yel"); }
  if (prefs.isKey("red")) { p.hasRedMs = true; p.redMs = prefs.getUShort("red"); }
  if (prefs.isKey("tail")) { p.hasTailMs = true; p.tailMs = prefs.getUShort("tail"); }
  if (prefs.isKey("glow")) { p.hasIdleGlow = true; p.idleGlow = prefs.getBool("glow"); }
  prefs.end();
  // Ungültige oder veraltete Werte verwerfen: dann gelten die Standardwerte.
  if (!p.empty() && !cfg.apply(p).ok()) Serial.println("Gespeicherte Konfiguration ungültig, Standardwerte");
  savedVersion = cfg.version();
}

// Diagnosemodus wird bewusst nicht gespeichert: nach dem Neustart zählt jeder Zug wieder.
void saveConfig() {
  const Config& c = cfg.get();
  prefs.begin("shisha", false);
  prefs.putUChar("mode", (uint8_t)c.mode);
  prefs.putUChar("bri", c.brightness);
  prefs.putULong("col", ((uint32_t)c.steadyColor.r << 16) | ((uint32_t)c.steadyColor.g << 8) | c.steadyColor.b);
  prefs.putUShort("yel", c.yellowMs);
  prefs.putUShort("red", c.redMs);
  prefs.putUShort("tail", c.tailMs);
  prefs.putBool("glow", c.idleGlow);
  prefs.end();
  savedVersion = cfg.version();
}

// Flash schonen: erst 2 s nach der letzten Änderung schreiben.
void scheduleSave(uint32_t now) {
  savePending = true;
  saveDueMs = now + 2000;
}

// ---- JSON ------------------------------------------------------------------

void jsonString(String& out, const char* s) {
  out += '"';
  for (const char* p = s; *p; p++) {
    const char ch = *p;
    if (ch == '"' || ch == '\\') { out += '\\'; out += ch; }
    else if ((uint8_t)ch < 0x20) { char buf[8]; snprintf(buf, sizeof(buf), "\\u%04x", ch); out += buf; }
    else out += ch;
  }
  out += '"';
}

const char* modeName(Mode m) {
  switch (m) {
    case Mode::Smart: return "smart";
    case Mode::Steady: return "steady";
    case Mode::Off: return "off";
  }
  return "smart";
}

void appendEntry(String& out, const LeaderboardEntry& e) {
  out += "{\"name\":";
  jsonString(out, e.name);
  out += ",\"durationMs\":"; out += e.durationMs;
  out += ",\"truncated\":"; out += e.truncated ? "true" : "false";
  out += ",\"diagnostic\":"; out += e.diagnostic ? "true" : "false";
  out += '}';
}

String stateJson() {
  const uint32_t now = millis();
  const Config& c = cfg.get();
  const PullState& ps = det.state();
  const SensorDiag d = det.diag(now, up.ms());
  String out;
  out.reserve(1024);
  out += "{\"fw\":"; jsonString(out, app::FW_VERSION);
  out += ",\"name\":"; jsonString(out, ses.currentName());
  out += ",\"pull\":{\"active\":"; out += ps.active ? "true" : "false";
  out += ",\"elapsedMs\":"; out += ps.active ? elapsed(now, ps.startMs) : 0;
  out += "},\"color\":[";
  out += frame.color.r; out += ','; out += frame.color.g; out += ','; out += frame.color.b;
  out += "],\"limited\":"; out += frame.limited ? "true" : "false";
  out += ",\"config\":{\"mode\":\""; out += modeName(c.mode);
  out += "\",\"brightness\":"; out += c.brightness;
  out += ",\"steadyColor\":[";
  out += c.steadyColor.r; out += ','; out += c.steadyColor.g; out += ','; out += c.steadyColor.b;
  out += "],\"yellowMs\":"; out += c.yellowMs;
  out += ",\"redMs\":"; out += c.redMs;
  out += ",\"tailMs\":"; out += c.tailMs;
  out += ",\"diagnostic\":"; out += c.diagnostic ? "true" : "false";
  out += ",\"idleGlow\":"; out += c.idleGlow ? "true" : "false";
  out += "},\"pullCount\":"; out += ses.pullCount();
  out += ",\"lastPull\":";
  if (ses.hasLastPull()) appendEntry(out, ses.lastPull()); else out += "null";
  out += ",\"leaderboard\":[";
  for (uint8_t i = 0; i < ses.entryCount(); i++) {
    if (i) out += ',';
    appendEntry(out, ses.entry(i));
  }
  out += "],\"sensor\":{\"level\":"; out += d.levelActive ? "true" : "false";
  out += ",\"pulses10s\":"; out += d.pulsesInWindow;
  out += ",\"fault\":"; out += d.fault == SensorFault::StuckActive ? "\"stuckActive\"" : "null";
  out += "}}";
  return out;
}

// ---- HTTP ------------------------------------------------------------------

void sendError(int code, const char* msg) {
  String out = "{\"error\":";
  jsonString(out, msg);
  out += '}';
  server.send(code, "application/json", out);
}

bool argInt(const char* name, int32_t* v) {
  if (!server.hasArg(name)) return false;
  *v = server.arg(name).toInt();
  return true;
}

bool argBool(const char* name, bool* v) {
  if (!server.hasArg(name)) return false;
  const String s = server.arg(name);
  *v = s == "1" || s == "true" || s == "on";
  return true;
}

// POST /api/config mit Formularfeldern: mode, brightness, r, g, b, yellowMs, redMs,
// tailMs, diagnostic, idleGlow – jeweils optional, alles oder nichts.
void handleConfig() {
  ConfigPatch p;
  if (server.hasArg("mode")) {
    const String m = server.arg("mode");
    p.hasMode = true;
    if (m == "smart") p.mode = Mode::Smart;
    else if (m == "steady") p.mode = Mode::Steady;
    else if (m == "off") p.mode = Mode::Off;
    else return sendError(400, "mode");
  }
  p.hasBrightness = argInt("brightness", &p.brightness);
  int32_t r, g, b;
  if (argInt("r", &r) && argInt("g", &g) && argInt("b", &b)) {
    p.hasSteadyColor = true;
    p.steadyR = r; p.steadyG = g; p.steadyB = b;
  }
  p.hasYellowMs = argInt("yellowMs", &p.yellowMs);
  p.hasRedMs = argInt("redMs", &p.redMs);
  p.hasTailMs = argInt("tailMs", &p.tailMs);
  p.hasDiagnostic = argBool("diagnostic", &p.diagnostic);
  p.hasIdleGlow = argBool("idleGlow", &p.idleGlow);
  const ApplyResult res = cfg.apply(p);
  if (!res.ok()) return sendError(400, configFieldName(res.field));
  scheduleSave(millis());
  server.send(200, "application/json", stateJson());
}

void handleName() {
  const String n = server.arg("name");
  if (ses.setName(n.c_str(), n.length()) == utf8::NameResult::Invalid) return sendError(400, "name");
  server.send(200, "application/json", stateJson());
}

void handleReset() {
  const String what = server.arg("scope");
  if (what == "leaderboard") ses.resetLeaderboard();
  else if (what == "lighting") cfg.reset(ResetScope::Lighting);
  else if (what == "gradient") cfg.reset(ResetScope::Gradient);
  else if (what == "sensor") cfg.reset(ResetScope::Sensor);
  else if (what == "all") cfg.reset(ResetScope::All);
  else return sendError(400, "scope");
  if (what != "leaderboard") scheduleSave(millis());
  server.send(200, "application/json", stateJson());
}

void setupWeb() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(app::AP_SSID);  // offenes Netz (Lastenheft), Adresse 192.168.4.1
  server.on("/", HTTP_GET, [] { server.send_P(200, "text/html; charset=utf-8", WEB_PAGE); });
  server.on("/api/state", HTTP_GET, [] { server.send(200, "application/json", stateJson()); });
  server.on("/api/config", HTTP_POST, handleConfig);
  server.on("/api/name", HTTP_POST, handleName);
  server.on("/api/reset", HTTP_POST, handleReset);
  server.onNotFound([] { sendError(404, "not found"); });
  server.begin();
}

// ---- LED-Ausgabe ------------------------------------------------------------

bool sameFrame(const LedFrame& a, const LedFrame& b) {
  return a.pixel == b.pixel && a.glow == b.glow;
}

void showFrame(const LedFrame& f) {
  if (hasShown && sameFrame(f, shown)) return;
  ring.clear();
  if (f.glow) {
    ring.setPixelColor(0, f.pixel.r, f.pixel.g, f.pixel.b);
  } else {
    for (uint16_t i = 0; i < app::LED_COUNT; i++) ring.setPixelColor(i, f.pixel.r, f.pixel.g, f.pixel.b);
  }
  ring.show();
  shown = f;
  hasShown = true;
}

}  // namespace

void setup() {
  Serial.begin(115200);
  pinMode(app::PIN_SENSOR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(app::PIN_SENSOR), onSensorEdge,
                  app::SENSOR_ACTIVE_LOW ? FALLING : RISING);
  ring.begin();
  ring.clear();
  ring.show();

  loadConfig();
  setupWeb();

  const uint32_t now = millis();
  up.begin(now);
  evalTimer.begin(now);
  frameTimer.begin(now);
  Serial.printf("ShishaLed %s – WLAN \"%s\", http://192.168.4.1\n", app::FW_VERSION, app::AP_SSID);
}

void loop() {
  const uint32_t now = millis();

  if (evalTimer.due(now)) {
    const uint64_t u = up.update(now);
    noInterrupts();
    const bool latched = sensorLatch;
    const uint32_t edges = sensorEdges;
    sensorLatch = false;
    sensorEdges = 0;
    interrupts();
    const bool level = sensorLevelActive();
    det.addEdges(edges, u);
    det.evaluate(now, u, latched || level, level, cfg.get().tailMs);

    uint32_t seq;
    if (det.takeStartedPull(&seq)) ses.onPullStarted(seq);
    PullRecord rec;
    while (det.takeCompletedPull(&rec)) {
      ses.onPullCompleted(rec, cfg.get().diagnostic);
      Serial.printf("Zug %lu: %lu ms%s\n", (unsigned long)rec.seq, (unsigned long)rec.durationMs,
                    rec.truncated ? " (abgeschnitten)" : "");
    }
  }

  if (frameTimer.due(now)) {
    frame = led.update(now, cfg.get(), det.state());
    showFrame(frame);
  }

  server.handleClient();

  if (savePending && (int32_t)(now - saveDueMs) >= 0) {
    savePending = false;
    if (cfg.version() != savedVersion) saveConfig();
  }
}
