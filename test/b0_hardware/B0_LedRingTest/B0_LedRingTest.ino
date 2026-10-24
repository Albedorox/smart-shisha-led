// B0 – LED-Ring-Test (Wegwerfcode, nicht Teil der Firmware)
//
// Prüft: Einzeladressierung und Reihenfolge (T-HW-06), Lichtwirkung unter dem
// Glas (R-14), Stromaufnahme je Helligkeit für die Strommessung (T-HW-05, M-05).
//
// Bedienung über den seriellen Monitor (115200 Baud):
//   1  Einzeltest: jede LED nacheinander Rot, Grün, Blau (LED 0 = am DI-Pad)
//   2  Zug-Demo: Grün -> Gelb -> Rot über 8 s, dann 1 s Ausblenden, Pause
//   3  Strommessung: ganzer Ring Weiß mit Helligkeit 32, 64, 128 (je 10 s)
//   4  Dauerfarben: Grün, Gelb, Rot, Blau, Weiß im 3-s-Takt
//   0  alles aus
//   + / -  Helligkeit für 2 und 4 (Schritte von 16, Start 64)

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

constexpr uint8_t  LED_PIN              = 4;     // GPIO4, siehe 02 §2
constexpr uint16_t LED_COUNT            = 24;
constexpr uint16_t LED_MA_PER_CHANNEL   = 20;    // Annahme, wird mit Modus 3 gemessen
constexpr uint16_t LED_CURRENT_LIMIT_MA = 500;   // D-05
constexpr uint32_t YELLOW_MS            = 3000;
constexpr uint32_t RED_MS               = 6000;

Adafruit_NeoPixel ring(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t  mode       = 1;
uint8_t  brightness = 64;
uint32_t modeStart  = 0;
uint32_t lastShow   = 0;
int32_t  phaseShown = -1;   // zuletzt gemeldeter Schritt im aktuellen Modus

struct Rgb { uint8_t r, g, b; };

int32_t lerp(int32_t a, int32_t b, int32_t n, int32_t d) { return a + ((b - a) * n + d / 2) / d; }

Rgb gradient(uint32_t t) {   // 04 §6.2
  if (t == 0) return {0, 255, 0};
  if (t < YELLOW_MS) return {(uint8_t)lerp(0, 255, t, YELLOW_MS), 255, 0};
  if (t < RED_MS) return {255, (uint8_t)lerp(255, 0, t - YELLOW_MS, RED_MS - YELLOW_MS), 0};
  return {255, 0, 0};
}

// Ganzer Ring in einer Farbe, mit Helligkeit und Stromgrenze (04 §6.4)
void fillScaled(Rgb c, uint8_t b) {
  const uint32_t ma = (uint32_t)LED_COUNT * LED_MA_PER_CHANNEL * (c.r + c.g + c.b) / 255 * b / 255;
  uint32_t eff = b;
  if (ma > LED_CURRENT_LIMIT_MA) eff = (uint32_t)b * LED_CURRENT_LIMIT_MA / ma;
  ring.fill(ring.Color(c.r * eff / 255, c.g * eff / 255, c.b * eff / 255));
}

void setMode(uint8_t m) {
  mode = m;
  modeStart = millis();
  phaseShown = -1;
  ring.clear();
  ring.show();
  static const char* const names[] = {"aus", "Einzeltest", "Zug-Demo", "Strommessung", "Dauerfarben"};
  Serial.printf("Modus %u: %s (Helligkeit %u)\n", m, names[m], brightness);
  if (m == 3) {
    Serial.println(F("Strommessung: USB-Messgerät ablesen. Geschätzt: 32 ≈ 180 mA, 64 ≈ 360 mA, 128 ≈ 720 mA (nur LEDs)."));
    Serial.println(F("Helligkeit 255 ist absichtlich nicht dabei (≈ 1,4 A)."));
  }
}

void runSingle(uint32_t t) {
  // 24 LEDs × 3 Farben × 250 ms = 18 s, danach automatisch Zug-Demo
  const uint32_t step = t / 250;
  if (step >= (uint32_t)LED_COUNT * 3) { setMode(2); return; }
  const uint16_t idx = step / 3;
  const uint8_t ch = step % 3;
  if ((int32_t)step == phaseShown) return;
  phaseShown = step;
  ring.clear();
  ring.setPixelColor(idx, ch == 0 ? 64 : 0, ch == 1 ? 64 : 0, ch == 2 ? 64 : 0);
  ring.show();
  if (ch == 0) Serial.printf("LED %u\n", idx);
}

void runDemo(uint32_t t) {
  constexpr uint32_t PULL = 8000, FADE = 1000, PAUSE = 2000;
  const uint32_t p = t % (PULL + FADE + PAUSE);
  if (p < PULL) {
    fillScaled(gradient(p), brightness);
  } else if (p < PULL + FADE) {
    const uint32_t f = 255 - (255 * (p - PULL) + 500) / 1000;   // 04 §6.3
    fillScaled(gradient(PULL), brightness * f / 255);
  } else {
    ring.clear();
  }
}

void runCurrent(uint32_t t) {
  static const uint8_t levels[] = {32, 64, 128};
  const uint32_t i = t / 10000;
  if (i >= sizeof(levels)) { setMode(0); return; }
  const uint8_t b = levels[i];
  ring.fill(ring.Color(b, b, b));   // ohne Stromgrenze, um echte Werte zu messen
  if ((int32_t)i != phaseShown) {
    phaseShown = i;
    Serial.printf("Weiß, Helligkeit %u, geschätzt %lu mA\n", b,
                  (unsigned long)(LED_COUNT * LED_MA_PER_CHANNEL * 3UL * b / 255));
  }
}

void runColors(uint32_t t) {
  static const Rgb colors[] = {{0, 255, 0}, {255, 255, 0}, {255, 0, 0}, {0, 0, 255}, {255, 255, 255}};
  fillScaled(colors[(t / 3000) % 5], brightness);
}

void handleSerial() {
  while (Serial.available()) {
    const char c = Serial.read();
    if (c >= '0' && c <= '4') setMode(c - '0');
    else if (c == '+' || c == '-') {
      const int b = brightness + (c == '+' ? 16 : -16);
      brightness = (uint8_t)constrain(b, 0, 255);
      Serial.printf("Helligkeit %u\n", brightness);
    }
  }
}

void setup() {
  Serial.begin(115200);
  ring.begin();
  ring.clear();   // WS2812 zeigen nach dem Einschalten teils Zufallsfarben
  ring.show();
  delay(1500);
  Serial.println(F("\n=== B0 LED-Ring-Test, 24 LEDs an GPIO4 ==="));
  Serial.println(F("Befehle: 1 Einzeltest, 2 Zug-Demo, 3 Strom, 4 Farben, 0 aus, +/- Helligkeit"));
  setMode(1);
}

void loop() {
  handleSerial();
  const uint32_t now = millis();
  if ((uint32_t)(now - lastShow) < 20) return;   // höchstens 50 Hz
  lastShow = now;
  const uint32_t t = now - modeStart;
  switch (mode) {
    case 1: runSingle(t); return;   // zeigt selbst an
    case 2: runDemo(t); break;
    case 3: runCurrent(t); break;
    case 4: runColors(t); break;
    default: ring.clear(); break;
  }
  ring.show();
}
