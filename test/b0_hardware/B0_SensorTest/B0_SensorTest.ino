// B0 – Sensortest SW-420 (Wegwerfcode, nicht Teil der Firmware)
//
// Prüft: Ruhepegel / Polarität (T-HW-04), Impulsbreiten, und ob Züge durch das
// Glas überhaupt erkannt werden (R-02). Die Zugerkennung entspricht dem Plan
// (04 §4): Latch in der ISR, Auswertung alle 10 ms, Zugende nach TAIL_MS ohne Impuls.
//
// Bedienung über den seriellen Monitor (115200 Baud, Zeilenende egal):
//   s  1-s-Statistik ein/aus (Flanken, Pegel, Impulsbreiten)
//   p  Plotter-Ausgabe ein/aus (für den seriellen Plotter, alle 10 ms)
//   t  Nachlaufzeit umschalten: 500 / 1000 / 1500 / 2000 ms
//   k  Ruhepegel neu kalibrieren (Sensor 3 s nicht berühren)

#include <Arduino.h>

constexpr uint8_t  SENSOR_PIN     = 0;      // GPIO0, siehe 02 §2
constexpr uint32_t EVAL_MS        = 10;
constexpr uint32_t CALIB_MS       = 3000;
constexpr uint32_t STUCK_MS       = 5000;
constexpr uint32_t MAX_PULL_MS    = 60000;
constexpr uint32_t TAIL_OPTIONS[] = {500, 1000, 1500, 2000};

volatile bool     activeLevel   = LOW;      // wird kalibriert
volatile bool     latch         = false;
volatile uint32_t edgeCount     = 0;
volatile uint32_t activeSinceUs = 0;
volatile bool     inPulse       = false;
volatile uint32_t minPulseUs    = UINT32_MAX;
volatile uint32_t maxPulseUs    = 0;

uint8_t  tailIdx   = 1;
bool     showStats = true;
bool     showPlot  = false;

enum class State { Idle, Active };
State    state       = State::Idle;
uint32_t pullStart   = 0;
uint32_t lastPulse   = 0;
uint32_t pullEdges0  = 0;
uint32_t pullNo      = 0;
bool     waitQuiet   = false;   // nach Maximaldauer erst Ruhe abwarten
uint32_t nextEval    = 0;
uint32_t nextStats   = 0;
uint32_t statEdges0  = 0;
uint32_t activeSince = 0;       // Dauer-aktiv-Erkennung
bool     stuckWarned = false;

void IRAM_ATTR onSensorChange() {
  const uint32_t t = micros();
  const bool lvl = digitalRead(SENSOR_PIN);
  edgeCount++;
  if (lvl == activeLevel) {
    latch = true;
    inPulse = true;
    activeSinceUs = t;
  } else if (inPulse) {
    const uint32_t w = t - activeSinceUs;
    if (w < minPulseUs) minPulseUs = w;
    if (w > maxPulseUs) maxPulseUs = w;
    inPulse = false;
  }
}

void calibrate() {
  detachInterrupt(digitalPinToInterrupt(SENSOR_PIN));
  Serial.println(F("\nKalibrierung: Sensor 3 s nicht berühren ..."));
  uint32_t high = 0, low = 0;
  const uint32_t t0 = millis();
  while ((uint32_t)(millis() - t0) < CALIB_MS) {
    if (digitalRead(SENSOR_PIN)) high++; else low++;
    delay(1);
  }
  const bool idleHigh = high >= low;
  activeLevel = idleHigh ? LOW : HIGH;
  Serial.printf("Ruhepegel: %s (HIGH %lu / LOW %lu Abtastungen)\n",
                idleHigh ? "HIGH" : "LOW", (unsigned long)high, (unsigned long)low);
  Serial.printf("=> In app_config.h eintragen: SENSOR_ACTIVE_LOW = %s\n",
                idleHigh ? "true" : "false");
  if (high && low) {
    Serial.println(F("Hinweis: Pegel hat in der Ruhe gewechselt. Poti am SW-420 etwas unempfindlicher drehen."));
  }
  noInterrupts();
  latch = false; inPulse = false; minPulseUs = UINT32_MAX; maxPulseUs = 0;
  interrupts();
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), onSensorChange, CHANGE);
  state = State::Idle;
  waitQuiet = false;
}

void handleSerial() {
  while (Serial.available()) {
    switch (Serial.read()) {
      case 's': showStats = !showStats; break;
      case 'p': showPlot = !showPlot; break;
      case 't':
        tailIdx = (tailIdx + 1) % (sizeof(TAIL_OPTIONS) / sizeof(TAIL_OPTIONS[0]));
        Serial.printf("Nachlaufzeit: %lu ms\n", (unsigned long)TAIL_OPTIONS[tailIdx]);
        break;
      case 'k': calibrate(); break;
      default: break;
    }
  }
}

void endPull(uint32_t now, bool truncated) {
  const uint32_t edges = edgeCount - pullEdges0;
  Serial.printf("Zug #%lu Ende: Dauer %lu ms (Signal %lu ms + Nachlauf %lu ms), %lu Flanken%s\n",
                (unsigned long)pullNo, (unsigned long)(now - pullStart),
                (unsigned long)(lastPulse - pullStart), (unsigned long)(now - lastPulse),
                (unsigned long)edges, truncated ? ", MAXIMALDAUER erreicht" : "");
  state = State::Idle;
  waitQuiet = truncated;
}

void evaluate(uint32_t now) {
  const bool levelActive = digitalRead(SENSOR_PIN) == activeLevel;
  noInterrupts();
  const bool pulse = levelActive || latch;
  latch = false;
  interrupts();

  // Dauer-aktiv-Erkennung (04 §4.3)
  if (levelActive) {
    if (!activeSince) activeSince = now ? now : 1;
    if (!stuckWarned && (uint32_t)(now - activeSince) >= STUCK_MS) {
      Serial.println(F("WARNUNG: Sensor seit 5 s dauerhaft aktiv. Poti unempfindlicher drehen oder Polarität prüfen."));
      stuckWarned = true;
    }
  } else {
    activeSince = 0;
    stuckWarned = false;
  }

  const uint32_t tail = TAIL_OPTIONS[tailIdx];
  if (state == State::Idle) {
    if (waitQuiet) {
      if (pulse) lastPulse = now;
      else if ((uint32_t)(now - lastPulse) >= tail) waitQuiet = false;
    } else if (pulse) {
      state = State::Active;
      pullStart = lastPulse = now;
      pullEdges0 = edgeCount;
      pullNo++;
      Serial.printf("Zug #%lu Start\n", (unsigned long)pullNo);
    }
  } else {
    if (pulse) lastPulse = now;
    if ((uint32_t)(now - lastPulse) >= tail) endPull(now, false);
    else if ((uint32_t)(now - pullStart) >= MAX_PULL_MS) endPull(now, true);
  }

  if (showPlot) {
    Serial.printf("pegel:%d,impuls:%d,zug:%d\n", levelActive ? 1 : 0, pulse ? 1 : 0,
                  state == State::Active ? 2 : 0);
  }
}

void printStats(uint32_t now) {
  noInterrupts();
  const uint32_t edges = edgeCount;
  const uint32_t mn = minPulseUs, mx = maxPulseUs;
  minPulseUs = UINT32_MAX; maxPulseUs = 0;
  interrupts();
  const uint32_t perSec = edges - statEdges0;
  statEdges0 = edges;
  if (perSec == 0 && state == State::Idle) return;   // Ruhe nicht protokollieren
  Serial.printf("[%6lu ms] Flanken/s %3lu | Impulsbreite min %lu µs, max %lu µs | Zug %s\n",
                (unsigned long)now, (unsigned long)perSec,
                (unsigned long)(mn == UINT32_MAX ? 0 : mn), (unsigned long)mx,
                state == State::Active ? "aktiv" : "-");
}

void setup() {
  Serial.begin(115200);
  delay(1500);   // USB-CDC des ESP32-C3 braucht einen Moment
  Serial.println(F("\n=== B0 Sensortest SW-420 an GPIO0 ==="));
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  calibrate();
  Serial.println(F("Befehle: s Statistik, p Plotter, t Nachlaufzeit, k Kalibrieren"));
  nextEval = nextStats = millis();
}

void loop() {
  handleSerial();
  const uint32_t now = millis();
  if ((int32_t)(now - nextEval) >= 0) {
    evaluate(now);
    nextEval = now + EVAL_MS;   // verspätet: keine Nachholzyklen
  }
  if (showStats && !showPlot && (int32_t)(now - nextStats) >= 0) {
    printStats(now);
    nextStats = now + 1000;
  }
}
