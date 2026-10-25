// Smart Shisha LED-Untersetzer – Einstieg (übernimmt die Rolle von main.cpp, D-19)
//
// Sensor (ISR-Latch), LED-Ring (Adafruit NeoPixel), offenes WLAN mit Web-App und
// HTTP-API (web_api.cpp, 06_api_contracts.md). Die Kernlogik (Zugerkennung,
// Rangliste, LED-Logik) ist per Host-Tests geprüft; dieser Sketch verbindet sie
// nur mit der Hardware. Alle Daten sind flüchtig (SYS-03): nach dem Neustart
// gelten wieder die Standardwerte.
//
// Bibliotheken: Adafruit NeoPixel, ArduinoJson (Bibliotheksverwalter); WiFi und
// WebServer gehören zum ESP32-Arduino-Core.

#include <Adafruit_NeoPixel.h>

#include "app_config.h"
#include "config_manager.h"
#include "led_logic.h"
#include "pull_detector.h"
#include "session_manager.h"
#include "time_util.h"
#include "web_api.h"

namespace {

Adafruit_NeoPixel ring(app::LED_COUNT, app::PIN_LED, NEO_GRB + NEO_KHZ800);

ConfigManager  cfg;
PullDetector   det;
SessionManager ses;
LedLogic       led;
LedFrame       frame;
Uptime         up;
IntervalTimer  evalTimer(app::SENSOR_EVAL_MS);
IntervalTimer  frameTimer(app::LED_FRAME_MS);
WebContext     web{cfg, det, ses, frame, up};

volatile bool     sensorLatch = false;
volatile uint32_t sensorEdges = 0;
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

// Schreibt den Ring nur, wenn sich die Ausgabe geändert hat.
void showFrame(const LedFrame& f) {
  if (hasShown && f.pixel == shown.pixel && f.glow == shown.glow) return;
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

  const uint32_t now = millis();
  up.begin(now);
  evalTimer.begin(now);
  frameTimer.begin(now);
  webBegin(&web);
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

  webLoop();
}
