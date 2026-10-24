// B0 – Machbarkeitstest Zug + Licht (Wegwerfcode, nicht Teil der Firmware)
//
// Der eigentliche Praxistest am Glas (R-02, R-14): Sensor erkennt den Zug, der
// Ring zeigt den Verlauf Grün -> Gelb (3 s) -> Rot (6 s), nach dem Zug 1 s Ausblenden.
// Jeder Zug wird mit Dauer im seriellen Monitor protokolliert.
//
// Vorher B0_SensorTest laufen lassen und SENSOR_ACTIVE_LOW entsprechend setzen.
//
// Serielle Befehle (115200 Baud):
//   + / -  Helligkeit (Schritte von 16, Start 64)
//   t      Nachlaufzeit umschalten: 500 / 1000 / 1500 / 2000 ms
//   g      Bereitschaftsglimmen ein/aus (D-30)

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

constexpr bool     SENSOR_ACTIVE_LOW    = true;  // Ergebnis aus B0_SensorTest eintragen
constexpr uint8_t  SENSOR_PIN           = 0;
constexpr uint8_t  LED_PIN              = 4;
constexpr uint16_t LED_COUNT            = 24;
constexpr uint16_t LED_MA_PER_CHANNEL   = 20;
constexpr uint16_t LED_CURRENT_LIMIT_MA = 500;
constexpr uint32_t EVAL_MS              = 10;
constexpr uint32_t YELLOW_MS            = 3000;
constexpr uint32_t RED_MS               = 6000;
constexpr uint32_t FADE_MS              = 1000;
constexpr uint32_t MAX_PULL_MS          = 60000;
constexpr uint32_t TAIL_OPTIONS[]       = {500, 1000, 1500, 2000};

Adafruit_NeoPixel ring(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

volatile bool latch = false;

struct Rgb { uint8_t r, g, b; };

uint8_t  brightness = 64;
uint8_t  tailIdx    = 1;
bool     idleGlow   = true;
bool     active     = false;
bool     waitQuiet  = false;
bool     fading     = false;
uint32_t pullStart = 0, lastPulse = 0, pullEnd = 0, pullNo = 0;
uint32_t nextEval = 0, lastShow = 0;
Rgb      lastColor = {0, 255, 0};

void IRAM_ATTR onSensorEdge() { latch = true; }

int32_t lerp(int32_t a, int32_t b, int32_t n, int32_t d) { return a + ((b - a) * n + d / 2) / d; }

Rgb gradient(uint32_t t) {   // 04 §6.2
  if (t == 0) return {0, 255, 0};
  if (t < YELLOW_MS) return {(uint8_t)lerp(0, 255, t, YELLOW_MS), 255, 0};
  if (t < RED_MS) return {255, (uint8_t)lerp(255, 0, t - YELLOW_MS, RED_MS - YELLOW_MS), 0};
  return {255, 0, 0};
}

void fillScaled(Rgb c, uint8_t b) {   // 04 §6.4
  const uint32_t ma = (uint32_t)LED_COUNT * LED_MA_PER_CHANNEL * (c.r + c.g + c.b) / 255 * b / 255;
  uint32_t eff = b;
  if (ma > LED_CURRENT_LIMIT_MA) eff = (uint32_t)b * LED_CURRENT_LIMIT_MA / ma;
  ring.fill(ring.Color(c.r * eff / 255, c.g * eff / 255, c.b * eff / 255));
}

void evaluate(uint32_t now) {
  const bool levelActive = digitalRead(SENSOR_PIN) == (SENSOR_ACTIVE_LOW ? LOW : HIGH);
  noInterrupts();
  const bool pulse = levelActive || latch;
  latch = false;
  interrupts();

  const uint32_t tail = TAIL_OPTIONS[tailIdx];
  if (!active) {
    if (waitQuiet) {
      if (pulse) lastPulse = now;
      else if ((uint32_t)(now - lastPulse) >= tail) waitQuiet = false;
    } else if (pulse) {
      active = true;
      fading = false;
      pullStart = lastPulse = now;
      pullNo++;
      Serial.printf("Zug #%lu Start\n", (unsigned long)pullNo);
    }
    return;
  }
  if (pulse) lastPulse = now;
  const bool ended = (uint32_t)(now - lastPulse) >= tail;
  const bool truncated = !ended && (uint32_t)(now - pullStart) >= MAX_PULL_MS;
  if (ended || truncated) {
    active = false;
    fading = true;
    pullEnd = now;
    waitQuiet = truncated;
    Serial.printf("Zug #%lu Ende: %lu ms (davon Nachlauf %lu ms)%s\n", (unsigned long)pullNo,
                  (unsigned long)(now - pullStart), (unsigned long)(now - lastPulse),
                  truncated ? ", MAXIMALDAUER" : "");
  }
}

void render(uint32_t now) {
  if (active) {
    lastColor = gradient(now - pullStart);
    fillScaled(lastColor, brightness);
  } else if (fading && (uint32_t)(now - pullEnd) < FADE_MS) {
    const uint32_t f = 255 - (255 * (now - pullEnd) + 500) / FADE_MS;   // 04 §6.3
    fillScaled(lastColor, brightness * f / 255);
  } else {
    fading = false;
    ring.clear();
    if (idleGlow) ring.setPixelColor(0, 0, 2, 0);   // eine LED, ganz schwach grün
  }
  ring.show();
}

void handleSerial() {
  while (Serial.available()) {
    switch (Serial.read()) {
      case '+': brightness = (uint8_t)min(255, brightness + 16); Serial.printf("Helligkeit %u\n", brightness); break;
      case '-': brightness = (uint8_t)max(0, brightness - 16); Serial.printf("Helligkeit %u\n", brightness); break;
      case 't':
        tailIdx = (tailIdx + 1) % (sizeof(TAIL_OPTIONS) / sizeof(TAIL_OPTIONS[0]));
        Serial.printf("Nachlaufzeit %lu ms\n", (unsigned long)TAIL_OPTIONS[tailIdx]);
        break;
      case 'g': idleGlow = !idleGlow; Serial.printf("Bereitschaftsglimmen %s\n", idleGlow ? "an" : "aus"); break;
      default: break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  ring.begin();
  ring.clear();
  ring.show();
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), onSensorEdge, SENSOR_ACTIVE_LOW ? FALLING : RISING);
  delay(1500);
  Serial.println(F("\n=== B0 Zug + Licht ==="));
  Serial.printf("SENSOR_ACTIVE_LOW = %s, Nachlauf %lu ms, Helligkeit %u\n",
                SENSOR_ACTIVE_LOW ? "true" : "false", (unsigned long)TAIL_OPTIONS[tailIdx], brightness);
  Serial.println(F("Befehle: +/- Helligkeit, t Nachlaufzeit, g Glimmen"));
  nextEval = millis();
}

void loop() {
  handleSerial();
  const uint32_t now = millis();
  if ((int32_t)(now - nextEval) >= 0) {
    evaluate(now);
    nextEval = now + EVAL_MS;
  }
  if ((uint32_t)(now - lastShow) >= 20) {
    lastShow = now;
    render(now);
  }
}
