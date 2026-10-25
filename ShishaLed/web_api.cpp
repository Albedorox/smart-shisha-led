#include "web_api.h"

#include <ArduinoJson.h>
#include <WebServer.h>
#include <WiFi.h>

#include "app_config.h"
#include "web_page.h"

namespace {

constexpr size_t MAX_BODY = 512;

WebServer   server(80);
WebContext* ctx = nullptr;

// ---- Antworten ---------------------------------------------------------------

void sendJson(int code, JsonDocument& doc) {
  String out;
  serializeJson(doc, out);
  server.sendHeader("Cache-Control", "no-store");
  server.send(code, "application/json; charset=utf-8", out);
}

void sendError(int code, const char* errCode, const char* message, const char* field = nullptr) {
  JsonDocument doc;
  doc["ok"] = false;
  JsonObject e = doc["error"].to<JsonObject>();
  e["code"] = errCode;
  if (field) e["field"] = field;
  e["message"] = message;
  sendJson(code, doc);
}

const char* modeName(Mode m) {
  switch (m) {
    case Mode::Smart: return "smart";
    case Mode::Steady: return "steady";
    case Mode::Off: return "off";
  }
  return "smart";
}

const char* phaseName(Phase p) {
  switch (p) {
    case Phase::Idle: return "idle";
    case Phase::Pull: return "pull";
    case Phase::Fade: return "fade";
  }
  return "idle";
}

void fillConfig(JsonObject o) {
  const Config& c = ctx->cfg.get();
  o["mode"] = modeName(c.mode);
  o["yellowMs"] = c.yellowMs;
  o["redMs"] = c.redMs;
  o["tailMs"] = c.tailMs;
  o["brightness"] = c.brightness;
  JsonObject sc = o["steadyColor"].to<JsonObject>();
  sc["r"] = c.steadyColor.r;
  sc["g"] = c.steadyColor.g;
  sc["b"] = c.steadyColor.b;
  o["diagnostic"] = c.diagnostic;
  o["idleGlow"] = c.idleGlow;
}

// ---- Prüfung der Anfrage (06 §1) ---------------------------------------------

bool requireMethod(HTTPMethod m, const char* allow) {
  if (server.method() == m) return true;
  server.sendHeader("Allow", allow);
  sendError(405, "method_not_allowed", "Methode nicht erlaubt.");
  return false;
}

// Liest den JSON-Body in doc; bei Fehler ist die Antwort bereits gesendet.
bool readBody(JsonDocument& doc) {
  if (!requireMethod(HTTP_POST, "POST")) return false;
  const String ct = server.header("Content-Type");
  if (!ct.startsWith("application/json")) {
    sendError(415, "unsupported_media_type", "Content-Type muss application/json sein.");
    return false;
  }
  const String& body = server.arg("plain");
  if (body.length() > MAX_BODY) {
    sendError(413, "payload_too_large", "Anfrage zu groß (max. 512 Byte).");
    return false;
  }
  if (deserializeJson(doc, body) != DeserializationError::Ok || !doc.is<JsonObject>()) {
    sendError(400, "invalid_json", "Ungültiges JSON.");
    return false;
  }
  return true;
}

bool readInt(JsonVariant v, const char* field, int32_t* out) {
  if (!v.is<int32_t>()) {
    sendError(400, "invalid_type", "Ganze Zahl erwartet.", field);
    return false;
  }
  *out = v.as<int32_t>();
  return true;
}

bool readBool(JsonVariant v, const char* field, bool* out) {
  if (!v.is<bool>()) {
    sendError(400, "invalid_type", "true oder false erwartet.", field);
    return false;
  }
  *out = v.as<bool>();
  return true;
}

const char* rangeMessage(ConfigField f) {
  switch (f) {
    case ConfigField::Brightness: return "Helligkeit zwischen 0 und 255 eingeben.";
    case ConfigField::SteadyColor: return "Farbwerte zwischen 0 und 255 eingeben.";
    case ConfigField::YellowMs: return "Zeit bis Gelb: 0,5 bis 30,0 s in Schritten von 0,1 s.";
    case ConfigField::RedMs: return "Zeit bis Rot: 0,6 bis 60,0 s in Schritten von 0,1 s.";
    case ConfigField::TailMs: return "Nachlaufzeit: 0,20 bis 5,00 s in Schritten von 0,05 s.";
    default: return "Wert außerhalb des Bereichs.";
  }
}

// ---- Handler -----------------------------------------------------------------

void handleRoot() {
  if (!requireMethod(HTTP_GET, "GET")) return;
  if (server.header("If-None-Match") == WEB_PAGE_ETAG) {
    server.send(304);
    return;
  }
  server.sendHeader("Content-Encoding", "gzip");
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("ETag", WEB_PAGE_ETAG);
  server.send_P(200, "text/html; charset=utf-8", (const char*)WEB_PAGE_GZ, WEB_PAGE_GZ_LEN);
}

void handleStatus() {
  if (!requireMethod(HTTP_GET, "GET")) return;
  const uint32_t now = millis();
  const uint64_t up = ctx->up.ms();
  const PullState& ps = ctx->det.state();
  const LedFrame& f = ctx->frame;
  const Config& c = ctx->cfg.get();
  const SensorDiag d = ctx->det.diag(now, up);

  JsonDocument doc;
  doc["ok"] = true;
  doc["uptimeMs"] = up;
  doc["mode"] = modeName(c.mode);
  doc["phase"] = phaseName(f.phase);

  JsonObject pull = doc["pull"].to<JsonObject>();
  pull["active"] = ps.active;
  pull["elapsedMs"] = ps.active ? elapsed(now, ps.startMs) : 0;
  if (ps.active) pull["player"] = ctx->ses.pullName(); else pull["player"] = nullptr;
  pull["seq"] = ps.seq;

  JsonObject led = doc["led"].to<JsonObject>();
  led["r"] = f.color.r;
  led["g"] = f.color.g;
  led["b"] = f.color.b;
  led["brightness"] = c.brightness;
  led["effectiveBrightness"] = f.brightness;
  led["limited"] = f.limited;
  led["on"] = !(f.pixel == Rgb{0, 0, 0});
  led["glow"] = f.glow;

  fillConfig(doc["config"].to<JsonObject>());
  doc["configVersion"] = ctx->cfg.version();

  JsonObject s = doc["sensor"].to<JsonObject>();
  s["levelActive"] = d.levelActive;
  s["hasPulse"] = d.hasPulse;
  if (d.hasPulse) s["lastPulseAgoMs"] = d.lastPulseAgoMs; else s["lastPulseAgoMs"] = nullptr;
  s["pulsesInWindow"] = d.pulsesInWindow;
  s["windowMs"] = (uint32_t)app::DIAG_WINDOW_S * 1000;
  s["fault"] = d.fault == SensorFault::StuckActive ? "stuck_active" : "none";
  s["sampleIntervalMs"] = app::SENSOR_EVAL_MS;

  doc["player"]["current"] = ctx->ses.currentName();

  JsonObject ses = doc["session"].to<JsonObject>();
  ses["pullCount"] = ctx->ses.pullCount();
  ses["lbVersion"] = ctx->ses.lbVersion();
  if (ctx->ses.hasLastPull()) {
    const LeaderboardEntry& e = ctx->ses.lastPull();
    JsonObject lp = ses["lastPull"].to<JsonObject>();
    lp["player"] = e.name;
    lp["durationMs"] = e.durationMs;
    lp["endedAtMs"] = e.endedAtMs;
    lp["truncated"] = e.truncated;
    lp["diagnostic"] = e.diagnostic;
  } else {
    ses["lastPull"] = nullptr;
  }

  JsonObject sys = doc["system"].to<JsonObject>();
  sys["deviceName"] = app::DEVICE_NAME;
  sys["fwVersion"] = app::FW_VERSION;
  sys["build"] = __DATE__;
  sys["ssid"] = app::AP_SSID;
  sys["ip"] = WiFi.softAPIP().toString();
  sys["ledCount"] = app::LED_COUNT;
  sys["fadeMs"] = app::FADE_MS;
  sys["clients"] = WiFi.softAPgetStationNum();

  sendJson(200, doc);
}

void handleLeaderboard() {
  if (!requireMethod(HTTP_GET, "GET")) return;
  JsonDocument doc;
  doc["ok"] = true;
  doc["lbVersion"] = ctx->ses.lbVersion();
  doc["uptimeMs"] = ctx->up.ms();
  doc["pullCount"] = ctx->ses.pullCount();
  JsonArray arr = doc["entries"].to<JsonArray>();
  for (uint8_t i = 0; i < ctx->ses.entryCount(); i++) {
    const LeaderboardEntry& e = ctx->ses.entry(i);
    JsonObject o = arr.add<JsonObject>();
    o["rank"] = i + 1;
    o["player"] = e.name;
    o["durationMs"] = e.durationMs;
    o["endedAtMs"] = e.endedAtMs;
    o["truncated"] = e.truncated;
  }
  sendJson(200, doc);
}

void sendConfig() {
  JsonDocument doc;
  doc["ok"] = true;
  doc["configVersion"] = ctx->cfg.version();
  fillConfig(doc["config"].to<JsonObject>());
  sendJson(200, doc);
}

void handleConfig() {
  JsonDocument doc;
  if (!readBody(doc)) return;
  JsonObject obj = doc.as<JsonObject>();

  if (!obj["reset"].isNull()) {
    if (obj.size() != 1) {
      return sendError(400, "validation_failed", "„reset“ darf nicht mit anderen Feldern kombiniert werden.", "reset");
    }
    const char* r = obj["reset"].is<const char*>() ? obj["reset"].as<const char*>() : "";
    if (!strcmp(r, "lighting")) ctx->cfg.reset(ResetScope::Lighting);
    else if (!strcmp(r, "gradient")) ctx->cfg.reset(ResetScope::Gradient);
    else if (!strcmp(r, "sensor")) ctx->cfg.reset(ResetScope::Sensor);
    else if (!strcmp(r, "all")) ctx->cfg.reset(ResetScope::All);
    else return sendError(400, "out_of_range", "Unbekannter Bereich für „reset“.", "reset");
    return sendConfig();
  }

  ConfigPatch p;
  for (JsonPair kv : obj) {
    const char* k = kv.key().c_str();
    JsonVariant v = kv.value();
    if (!strcmp(k, "mode")) {
      const char* m = v.is<const char*>() ? v.as<const char*>() : nullptr;
      if (!m) return sendError(400, "invalid_type", "Text erwartet.", "mode");
      p.hasMode = true;
      if (!strcmp(m, "smart")) p.mode = Mode::Smart;
      else if (!strcmp(m, "steady")) p.mode = Mode::Steady;
      else if (!strcmp(m, "off")) p.mode = Mode::Off;
      else return sendError(400, "out_of_range", "Modus muss smart, steady oder off sein.", "mode");
    } else if (!strcmp(k, "brightness")) {
      if (!readInt(v, k, &p.brightness)) return;
      p.hasBrightness = true;
    } else if (!strcmp(k, "steadyColor")) {
      if (!v.is<JsonObject>()) return sendError(400, "invalid_type", "Objekt {r,g,b} erwartet.", k);
      JsonObject c = v.as<JsonObject>();
      if (c.size() != 3 || !c["r"].is<int32_t>() || !c["g"].is<int32_t>() || !c["b"].is<int32_t>()) {
        return sendError(400, "invalid_type", "Objekt {r,g,b} mit ganzen Zahlen erwartet.", k);
      }
      p.steadyR = c["r"].as<int32_t>();
      p.steadyG = c["g"].as<int32_t>();
      p.steadyB = c["b"].as<int32_t>();
      p.hasSteadyColor = true;
    } else if (!strcmp(k, "yellowMs")) {
      if (!readInt(v, k, &p.yellowMs)) return;
      p.hasYellowMs = true;
    } else if (!strcmp(k, "redMs")) {
      if (!readInt(v, k, &p.redMs)) return;
      p.hasRedMs = true;
    } else if (!strcmp(k, "tailMs")) {
      if (!readInt(v, k, &p.tailMs)) return;
      p.hasTailMs = true;
    } else if (!strcmp(k, "diagnostic")) {
      if (!readBool(v, k, &p.diagnostic)) return;
      p.hasDiagnostic = true;
    } else if (!strcmp(k, "idleGlow")) {
      if (!readBool(v, k, &p.idleGlow)) return;
      p.hasIdleGlow = true;
    } else {
      return sendError(400, "unknown_field", "Unbekanntes Feld.", k);
    }
  }

  const ApplyResult res = ctx->cfg.apply(p);
  switch (res.error) {
    case ConfigError::None: return sendConfig();
    case ConfigError::EmptyPatch: return sendError(400, "empty_patch", "Keine änderbaren Felder übergeben.");
    case ConfigError::OutOfRange:
      return sendError(400, "out_of_range", rangeMessage(res.field), configFieldName(res.field));
    case ConfigError::ValidationFailed:
      return sendError(400, "validation_failed", "Rot muss mindestens 0,1 s nach Gelb erreicht werden.",
                       configFieldName(res.field));
  }
}

void handleName() {
  JsonDocument doc;
  if (!readBody(doc)) return;
  JsonObject obj = doc.as<JsonObject>();
  for (JsonPair kv : obj) {
    if (strcmp(kv.key().c_str(), "name")) return sendError(400, "unknown_field", "Unbekanntes Feld.", kv.key().c_str());
  }
  JsonVariant v = obj["name"];
  const char* s = "";
  size_t len = 0;
  if (!v.isNull()) {
    if (!v.is<const char*>()) return sendError(400, "invalid_type", "Text erwartet.", "name");
    JsonString js = v.as<JsonString>();
    s = js.c_str();
    len = js.size();
  }
  const utf8::NameResult r = ctx->ses.setName(s, len);
  if (r == utf8::NameResult::Invalid) {
    return sendError(400, "invalid_name", "Maximal 24 Zeichen, keine Steuerzeichen.", "name");
  }
  JsonDocument out;
  out["ok"] = true;
  out["player"] = ctx->ses.currentName();
  out["normalized"] = r == utf8::NameResult::Normalized;
  out["appliesFromNextPull"] = ctx->det.state().active;
  sendJson(200, out);
}

void handleLeaderboardReset() {
  JsonDocument doc;
  if (!readBody(doc)) return;
  JsonObject obj = doc.as<JsonObject>();
  if (!(obj["confirm"].is<bool>() && obj["confirm"].as<bool>()) || obj.size() != 1) {
    return sendError(400, "confirmation_required", "Bestätigung fehlt ({\"confirm\": true}).", "confirm");
  }
  ctx->ses.resetLeaderboard();
  JsonDocument out;
  out["ok"] = true;
  out["lbVersion"] = ctx->ses.lbVersion();
  out["pullCount"] = ctx->ses.pullCount();
  sendJson(200, out);
}

}  // namespace

void webBegin(WebContext* c) {
  ctx = c;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(app::AP_SSID);  // offen, ohne Passwort (01 SAF-01)

  static const char* headers[] = {"Content-Type", "If-None-Match"};
  server.collectHeaders(headers, 2);
  server.on("/", HTTP_ANY, handleRoot);
  server.on("/api/status", HTTP_ANY, handleStatus);
  server.on("/api/leaderboard", HTTP_ANY, handleLeaderboard);
  server.on("/api/config", HTTP_ANY, handleConfig);
  server.on("/api/name", HTTP_ANY, handleName);
  server.on("/api/leaderboard/reset", HTTP_ANY, handleLeaderboardReset);
  server.onNotFound([] { sendError(404, "not_found", "Pfad unbekannt."); });
  server.begin();
}

void webLoop() { server.handleClient(); }
